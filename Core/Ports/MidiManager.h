// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include <atomic>
#include <vector>
#include <string>

#ifdef __APPLE__
#include <CoreMIDI/CoreMIDI.h>
#else

// namespace vamiga {

// No MIDI support. Declare dummy types to make the compile happy
using MIDIClientRef = long;
using MIDIEndpointRef = long;
using MIDIEndpointRef = long;
using MIDINotification = long;
using MIDIPacketList = long;
using MIDIPortRef = long;
using MIDIPortRef = long;
using ItemCount = unsigned long;
// }

#endif

namespace vamiga {

// Lock-free ring buffer for MIDI input (from external device to Amiga)
struct MidiRingBuffer {
    // Ring Buffer Size
    static constexpr size_t CAPACITY = 512;
    uint8_t data[CAPACITY];
    std::atomic<size_t> writePos{0};
    std::atomic<size_t> readPos{0};
    
    bool push(uint8_t byte);
    bool pop(uint8_t *byte);
    bool isEmpty() const;
};

class MidiManager : public SubComponent {

    Descriptions descriptions = {{
        .type           = Class::MidiManager,
        .name           = "MidiManager",
        .description    = "MIDI Manager",
        .shell          = "midi"
    }};

    Options options = {
        Opt::MIDI_DEVICE_OUT,
        Opt::MIDI_DEVICE_IN
    };

    // CoreMIDI handles (not serialized - runtime state only)
    MIDIClientRef midiClient = 0;
    MIDIPortRef inputPort = 0;
    MIDIPortRef outputPort = 0;
    MIDIEndpointRef currentInputEndpoint = 0;
    MIDIEndpointRef currentOutputEndpoint = 0;
    
    // Connection state
    bool clientCreated = false;
    
    // Selected device indices (-1 = none)
    int selectedOutputDevice = -1;
    int selectedInputDevice = -1;

    // Lock-free buffer for incoming MIDI data
    MidiRingBuffer inputBuffer;
    
    // MIDI output message parser state
    uint8_t outputStatus = 0;        // Running status byte
    uint8_t outputBuffer[3];         // Message buffer
    int outputBytesExpected = 0;     // Bytes remaining for current message
    int outputBytesReceived = 0;     // Bytes received so far
    bool inSysEx = false;            // Currently receiving SysEx
    std::vector<uint8_t> sysExBuffer; // SysEx accumulator

public:

    using SubComponent::SubComponent;
    
    MidiManager& operator= (const MidiManager& other) {
        return *this;
    }

    //
    // Methods from Serializable
    //

private:

    template <class T>
    void serialize(T& worker)
    {
        // MidiManager has no persistent state to serialize
        // All CoreMIDI handles are runtime-only
        if (isResetter(worker)) return;

    } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }
    const Options &getOptions() const override { return options; }
    i64 getOption(Opt option) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt option, i64 value) override;

private:

    void _dump(Category category, std::ostream &os) const override;
    void _didReset(bool hard) override;
    void _powerOff() override;



public:

    //
    // CoreMIDI Setup
    //
    
    // Initialize CoreMIDI client (call once at startup)
    bool initMidi();
    
    // Shutdown CoreMIDI (call at cleanup)
    void shutdownMidi();
    
    // Open/close connections to specific endpoints
    bool openOutput(MIDIEndpointRef endpoint);
    bool openInput(MIDIEndpointRef endpoint);
    void closeOutput();
    void closeInput();
    
    //
    // Device Enumeration
    //
    
    // Get number of available MIDI destinations (outputs)
    static ItemCount getOutputCount();
    
    // Get number of available MIDI sources (inputs)
    static ItemCount getInputCount();
    
    // Get name of a MIDI destination
    static std::string getOutputName(ItemCount index);
    
    // Get name of a MIDI source
    static std::string getInputName(ItemCount index);
    
    // Get endpoint reference by index
    static MIDIEndpointRef getOutputEndpoint(ItemCount index);
    static MIDIEndpointRef getInputEndpoint(ItemCount index);
    
    //
    // MIDI I/O (called from UART)
    //
    
    // Send a byte from the Amiga to external MIDI device
    void sendByte(uint8_t byte);
    
    // Check if there's incoming MIDI data available
    bool hasInput() const;
    
    // Receive a byte from external MIDI device to Amiga
    bool receiveByte(uint8_t *byte);
    
    // Called from CoreMIDI input callback
    void handleMidiInput(const MIDIPacketList *pktlist);

private:

    //
    // Internal helpers
    //
    
    // Parse outgoing byte stream and send complete MIDI messages
    void processOutputByte(uint8_t byte);
    
    // Send a complete MIDI message via CoreMIDI
    void sendMidiMessage(const uint8_t *data, size_t length);
    
    // Send accumulated SysEx data
    void flushSysEx();
    
    // Get expected message length for a status byte
    static int getMessageLength(uint8_t status);
};

}
