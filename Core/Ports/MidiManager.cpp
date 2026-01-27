// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "MidiManager.h"
#include "Amiga.h"
#include "Agnus.h"
#include "IOUtils.h"

#ifdef __APPLE__
#define MIDI_SUPPORT
#endif

namespace vamiga {

#ifdef MIDI_SUPPORT

//
// MidiRingBuffer
//

bool
MidiRingBuffer::push(uint8_t byte)
{
    size_t currentWrite = writePos.load(std::memory_order_relaxed);
    size_t nextWrite = (currentWrite + 1) % CAPACITY;
    
    if (nextWrite == readPos.load(std::memory_order_acquire)) {
        return false; // Buffer full
    }
    
    data[currentWrite] = byte;
    writePos.store(nextWrite, std::memory_order_release);
    return true;
}

bool
MidiRingBuffer::pop(uint8_t *byte)
{
    size_t currentRead = readPos.load(std::memory_order_relaxed);
    
    if (currentRead == writePos.load(std::memory_order_acquire)) {
        return false; // Buffer empty
    }
    
    *byte = data[currentRead];
    readPos.store((currentRead + 1) % CAPACITY, std::memory_order_release);
    return true;
}

bool
MidiRingBuffer::isEmpty() const
{
    return readPos.load(std::memory_order_acquire) == 
           writePos.load(std::memory_order_acquire);
}

//
// MidiManager
//

void
MidiManager::_dump(Category category, std::ostream &os) const
{
    using namespace util;
    
    if (category == Category::State) {
        os << tab("Client Created") << bol(clientCreated) << std::endl;
        os << tab("Output Endpoint") << (currentOutputEndpoint ? "Connected" : "None") << std::endl;
        os << tab("Input Endpoint") << (currentInputEndpoint ? "Connected" : "None") << std::endl;
    }
}

void
MidiManager::_didReset(bool hard)
{
    // Reset parser state
    outputStatus = 0;
    outputBytesExpected = 0;
    outputBytesReceived = 0;
    inSysEx = false;
    sysExBuffer.clear();
}

void
MidiManager::_powerOff()
{
    shutdownMidi();
}

i64
MidiManager::getOption(Opt option) const
{
    switch (option) {
        case Opt::MIDI_DEVICE_OUT:  return selectedOutputDevice;
        case Opt::MIDI_DEVICE_IN:   return selectedInputDevice;
        default:                    fatalError;
    }
}

void
MidiManager::checkOption(Opt opt, i64 value)
{
    switch (opt) {
        case Opt::MIDI_DEVICE_OUT:
        case Opt::MIDI_DEVICE_IN:
            return;
        default:
            throw(Fault::OPT_UNSUPPORTED);
    }
}

void
MidiManager::setOption(Opt option, i64 value)
{
    switch (option) {
        case Opt::MIDI_DEVICE_OUT:
            selectedOutputDevice = (int)value;
            if (value >= 0 && value < (i64)getOutputCount()) {
                initMidi();
                openOutput(getOutputEndpoint((ItemCount)value));
            } else {
                closeOutput();
            }
            return;
            
        case Opt::MIDI_DEVICE_IN:
            selectedInputDevice = (int)value;
            if (value >= 0 && value < (i64)getInputCount()) {
                initMidi();
                openInput(getInputEndpoint((ItemCount)value));
                // Start the receive event loop
                amiga.agnus.scheduleImm<SLOT_SER>(SER_RECEIVE);
            } else {
                closeInput();
            }
            return;

        default:
            fatalError;
    }
}

//
// CoreMIDI Setup
//

// Static callback wrappers for CoreMIDI C API
static void staticMidiNotifyCallback(const MIDINotification *notification, void *refCon)
{
    MidiManager *self = static_cast<MidiManager *>(refCon);
    if (notification->messageID == kMIDIMsgSetupChanged) {
        // MIDI setup changed - devices connected/disconnected
        // Close connections to be safe - user can reconnect via GUI
        self->closeOutput();
        self->closeInput();
    }
}

static void staticMidiInputCallback(const MIDIPacketList *pktlist,
                                     void *readProcRefCon,
                                     void *srcConnRefCon)
{
    MidiManager *self = static_cast<MidiManager *>(readProcRefCon);
    self->handleMidiInput(pktlist);
}

bool
MidiManager::initMidi()
{
    if (clientCreated) return true;
    
    OSStatus status = MIDIClientCreate(
        CFSTR("vAmiga"),
        staticMidiNotifyCallback,
        this,
        &midiClient
    );
    
    if (status != noErr) {
        warn("Failed to create MIDI client: %d\n", (int)status);
        return false;
    }
    
    // Create input port
    status = MIDIInputPortCreate(
        midiClient,
        CFSTR("vAmiga Input"),
        staticMidiInputCallback,
        this,
        &inputPort
    );
    
    if (status != noErr) {
        warn("Failed to create MIDI input port: %d\n", (int)status);
        MIDIClientDispose(midiClient);
        midiClient = 0;
        return false;
    }
    
    // Create output port
    status = MIDIOutputPortCreate(
        midiClient,
        CFSTR("vAmiga Output"),
        &outputPort
    );
    
    if (status != noErr) {
        warn("Failed to create MIDI output port: %d\n", (int)status);
        MIDIPortDispose(inputPort);
        inputPort = 0;
        MIDIClientDispose(midiClient);
        midiClient = 0;
        return false;
    }
    
    clientCreated = true;
   // Useful to debug CoreMIDI 
   // printf("MIDI client initialized successfully - clientCreated=%d\n", clientCreated);
    debug(SER_DEBUG, "MIDI client initialized successfully\n");
    return true;
}

void
MidiManager::shutdownMidi()
{
    closeInput();
    closeOutput();
    
    if (inputPort) {
        MIDIPortDispose(inputPort);
        inputPort = 0;
    }
    
    if (outputPort) {
        MIDIPortDispose(outputPort);
        outputPort = 0;
    }
    
    if (midiClient) {
        MIDIClientDispose(midiClient);
        midiClient = 0;
    }
    
    clientCreated = false;
    debug(SER_DEBUG, "MIDI client shut down\n");
}

bool
MidiManager::openOutput(MIDIEndpointRef endpoint)
{
    if (!clientCreated && !initMidi()) return false;

    closeOutput();
    currentOutputEndpoint = endpoint;

    debug(SER_DEBUG, "MIDI output opened\n");
    return true;
}

bool
MidiManager::openInput(MIDIEndpointRef endpoint)
{
    if (!clientCreated && !initMidi()) return false;
    
    closeInput();
    
    OSStatus status = MIDIPortConnectSource(inputPort, endpoint, nullptr);
    if (status != noErr) {
        warn("Failed to connect MIDI input source: %d\n", (int)status);
        return false;
    }
    
    currentInputEndpoint = endpoint;
    debug(SER_DEBUG, "MIDI input opened\n");
    return true;
}

void
MidiManager::closeOutput()
{
    if (currentOutputEndpoint) {
        // Flush any pending SysEx
        if (inSysEx) {
            flushSysEx();
        }
        currentOutputEndpoint = 0;
    }
}

void
MidiManager::closeInput()
{
    if (currentInputEndpoint && inputPort) {
        MIDIPortDisconnectSource(inputPort, currentInputEndpoint);
        currentInputEndpoint = 0;
    }
}

//
// Device Enumeration
//

ItemCount
MidiManager::getOutputCount()
{
    return MIDIGetNumberOfDestinations();
}

ItemCount
MidiManager::getInputCount()
{
    return MIDIGetNumberOfSources();
}

std::string
MidiManager::getOutputName(ItemCount index)
{
    MIDIEndpointRef endpoint = MIDIGetDestination(index);
    if (!endpoint) return "";
    
    CFStringRef name = nullptr;
    MIDIObjectGetStringProperty(endpoint, kMIDIPropertyDisplayName, &name);

    if (!name) return "";
    
    char buffer[256];
    CFStringGetCString(name, buffer, sizeof(buffer), kCFStringEncodingUTF8);
    CFRelease(name);
    
    return std::string(buffer);
}

std::string
MidiManager::getInputName(ItemCount index)
{
    MIDIEndpointRef endpoint = MIDIGetSource(index);
    if (!endpoint) return "";
    
    CFStringRef name = nullptr;
    MIDIObjectGetStringProperty(endpoint, kMIDIPropertyDisplayName, &name);

    if (!name) return "";
    
    char buffer[256];
    CFStringGetCString(name, buffer, sizeof(buffer), kCFStringEncodingUTF8);
    CFRelease(name);
    
    return std::string(buffer);
}

MIDIEndpointRef
MidiManager::getOutputEndpoint(ItemCount index)
{
    return MIDIGetDestination(index);
}

MIDIEndpointRef
MidiManager::getInputEndpoint(ItemCount index)
{
    return MIDIGetSource(index);
}

//
// MIDI I/O
//

void
MidiManager::sendByte(uint8_t byte)
{
    if (!currentOutputEndpoint || !clientCreated) return;

    processOutputByte(byte);
}

bool
MidiManager::hasInput() const
{
    return !inputBuffer.isEmpty();
}

bool
MidiManager::receiveByte(uint8_t *byte)
{
    return inputBuffer.pop(byte);
}

void
MidiManager::handleMidiInput(const MIDIPacketList *pktlist)
{
    // Safety check - ignore input if not connected
    if (!currentInputEndpoint || !clientCreated) return;
    
    const MIDIPacket *packet = &pktlist->packet[0];
    for (UInt32 i = 0; i < pktlist->numPackets; ++i) {
        for (UInt16 j = 0; j < packet->length; ++j) {
            inputBuffer.push(packet->data[j]);
        }
        packet = MIDIPacketNext(packet);
    }
}

//
// Internal helpers
//

void
MidiManager::processOutputByte(uint8_t byte)
{
    // Handle real-time messages (can occur anywhere)
    if (byte >= 0xF8) {
        sendMidiMessage(&byte, 1);
        return;
    }
    
    // Handle SysEx
    if (byte == 0xF0) {
        // Start SysEx
        inSysEx = true;
        sysExBuffer.clear();
        sysExBuffer.push_back(byte);
        return;
    }
    
    if (inSysEx) {
        sysExBuffer.push_back(byte);
        if (byte == 0xF7) {
            // End of SysEx
            flushSysEx();
            inSysEx = false;
        }
        return;
    }
    
    // Handle status byte
    if (byte & 0x80) {
        outputStatus = byte;
        outputBuffer[0] = byte;
        outputBytesReceived = 1;
        outputBytesExpected = getMessageLength(byte);
        
        // Single byte messages
        if (outputBytesExpected == 1) {
            sendMidiMessage(outputBuffer, 1);
            outputBytesReceived = 0;
        }
        return;
    }
    
    // Handle data byte
    if (outputBytesReceived == 0) {
        // Running status - reuse last status byte
        outputBuffer[0] = outputStatus;
        outputBytesReceived = 1;
        outputBytesExpected = getMessageLength(outputStatus);
    }
    
    outputBuffer[outputBytesReceived++] = byte;
    
    if (outputBytesReceived >= outputBytesExpected) {
        sendMidiMessage(outputBuffer, outputBytesExpected);
        outputBytesReceived = 0;
    }
}

void
MidiManager::sendMidiMessage(const uint8_t *data, size_t length)
{
    if (!currentOutputEndpoint || !clientCreated || length == 0) return;

    // Prepare packet list
    Byte buffer[256];
    MIDIPacketList *packetList = (MIDIPacketList *)buffer;
    MIDIPacket *packet = MIDIPacketListInit(packetList);

    packet = MIDIPacketListAdd(packetList, sizeof(buffer), packet,
                                0, // timestamp 0 = now
                                length, data);

    if (packet) {
        OSStatus status = MIDISend(outputPort, currentOutputEndpoint, packetList);
        if (status != noErr) {
            warn("MIDI send failed: %d\n", (int)status);
            closeOutput();
        }
    }
}

void
MidiManager::flushSysEx()
{
    if (sysExBuffer.empty()) return;
    
    sendMidiMessage(sysExBuffer.data(), sysExBuffer.size());
    sysExBuffer.clear();
}

int
MidiManager::getMessageLength(uint8_t status)
{
    if (status < 0x80) return 0; // Not a status byte
    
    switch (status & 0xF0) {
        case 0x80: return 3; // Note Off
        case 0x90: return 3; // Note On
        case 0xA0: return 3; // Aftertouch
        case 0xB0: return 3; // Control Change
        case 0xC0: return 2; // Program Change
        case 0xD0: return 2; // Channel Pressure
        case 0xE0: return 3; // Pitch Bend
        case 0xF0:
            switch (status) {
                case 0xF0: return 0; // SysEx (variable)
                case 0xF1: return 2; // MTC Quarter Frame
                case 0xF2: return 3; // Song Position
                case 0xF3: return 2; // Song Select
                case 0xF6: return 1; // Tune Request
                case 0xF7: return 1; // End of SysEx
                case 0xF8: return 1; // Timing Clock
                case 0xFA: return 1; // Start
                case 0xFB: return 1; // Continue
                case 0xFC: return 1; // Stop
                case 0xFE: return 1; // Active Sensing
                case 0xFF: return 1; // Reset
                default: return 1;
            }
    }
    return 1;
}

#else // MIDI_SUPPORT

bool MidiRingBuffer::push(uint8_t byte) { return false; }
bool MidiRingBuffer::pop(uint8_t *byte) { return false; }
bool MidiRingBuffer::isEmpty() const { return true; }

void MidiManager::_dump(Category category, std::ostream &os) const { }
void MidiManager::_didReset(bool hard) { }
void MidiManager::_powerOff() { }
i64 MidiManager::getOption(Opt option) const { return 0; }
void MidiManager::checkOption(Opt opt, i64 value) { }
void MidiManager::setOption(Opt option, i64 value) { }
bool MidiManager::initMidi() { return false; }
void MidiManager::shutdownMidi() { }
bool MidiManager::openOutput(MIDIEndpointRef endpoint) { return false; }
bool MidiManager::openInput(MIDIEndpointRef endpoint) { return false; }
void MidiManager::closeOutput() { }
void MidiManager::closeInput() { }
ItemCount MidiManager::getOutputCount() { return 0; }
ItemCount MidiManager::getInputCount() { return 0; }
string MidiManager::getOutputName(ItemCount index) { return ""; }
std::string MidiManager::getInputName(ItemCount index) { return ""; }
MIDIEndpointRef MidiManager::getOutputEndpoint(ItemCount index) { return 0; }
MIDIEndpointRef MidiManager::getInputEndpoint(ItemCount index)  { return 0; }
void MidiManager::sendByte(uint8_t byte) { }
bool MidiManager::hasInput() const { return false; }
bool MidiManager::receiveByte(uint8_t *byte) { return false; }
void MidiManager::handleMidiInput(const MIDIPacketList *pktlist) { }
void MidiManager::processOutputByte(uint8_t byte) { }
void MidiManager::sendMidiMessage(const uint8_t *data, size_t length) { }
void MidiManager::flushSysEx() { }
int MidiManager::getMessageLength(uint8_t status) { return 0; }

#endif
}
