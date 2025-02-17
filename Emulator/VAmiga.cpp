// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "VAmiga.h"
#include "Concurrency.h"
#include "Emulator.h"
#include "GuardList.h"

namespace vamiga {

/* All main API functions are annotated with one of the following keywords:
 *
 *   VAMIGA_PUBLIC:
 *
 *     This macro performs a sanity check in debug builds by assuring
 *     that the function is called from outside the emulator thread.
 *
 *   VAMIGA_PUBLIC_SUSPEND:
 *
 *     The macro additionally ensures that the emulator is in suspended
 *     state before the function body is executed. The emulator can suspend
 *     only after the current frame has been completed. Thus, calling
 *     an API function with this annotation may cause a noticable lag.
 */

#define VAMIGA_PUBLIC assert(!emu || emu->isUserThread());

DefaultsAPI VAmiga::defaults(&Emulator::defaults);
struct SuspendResume {
    const API *api;
    SuspendResume(const API *api) : api(api) {
        assert(!api->emu || api->emu->isUserThread());
        api->suspend();
    }
    ~SuspendResume() { api->resume(); }
};

#define VAMIGA_PUBLIC_SUSPEND VAMIGA_PUBLIC SuspendResume _sr(this);
// #define VAMIGA_SUSPENDED printf("%d: SUSPEND\n", __LINE__); SuspendResume _sr(this);


//
// API
//

void
API::suspend() const
{
    VAMIGA_PUBLIC
    emu->suspend();
}

void
API::resume() const
{
    VAMIGA_PUBLIC
    emu->resume();
}


//
// Components (Amiga)
//

const AmigaConfig &
AmigaAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return amiga->getConfig();
}

const AmigaInfo &
AmigaAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return amiga->getInfo();
}

const AmigaInfo &
AmigaAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return amiga->getCachedInfo();
}

void
AmigaAPI::dump(Category category, std::ostream& os) const
{
    VAMIGA_PUBLIC_SUSPEND
    amiga->dump(category, os);
}


//
// Components (Agnus)
//

const LogicAnalyzerConfig &
LogicAnalyzerAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return logicAnalyzer->getConfig();
}

const LogicAnalyzerInfo &
LogicAnalyzerAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return logicAnalyzer->getInfo();
}

const LogicAnalyzerInfo &
LogicAnalyzerAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return logicAnalyzer->getCachedInfo();
}

const DmaDebuggerConfig &
DmaDebuggerAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return dmaDebugger->getConfig();
}

const DmaDebuggerInfo &
DmaDebuggerAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return dmaDebugger->getInfo();
}

const DmaDebuggerInfo &
DmaDebuggerAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return dmaDebugger->getCachedInfo();
}

const AgnusConfig &
AgnusAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return agnus->getConfig();
}

const AgnusInfo &
AgnusAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return agnus->getInfo();
}

const AgnusInfo &
AgnusAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return agnus->getCachedInfo();
}

const AgnusStats &
AgnusAPI::getStats() const
{
    VAMIGA_PUBLIC
    return agnus->getStats();
}

const AgnusTraits
AgnusAPI::getTraits() const
{
    VAMIGA_PUBLIC
    return agnus->getTraits();
}


//
// Components (Blitter)
//

const BlitterInfo &
BlitterAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return blitter->getInfo();
}

const BlitterInfo &
BlitterAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return blitter->getCachedInfo();
}


//
// Components (CIA)
//

const CIAConfig &
CIAAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return cia->getConfig();
}

const CIAInfo &
CIAAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return cia->getInfo();
}

const CIAInfo &
CIAAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return cia->getCachedInfo();
}

CIAStats
CIAAPI::getStats() const
{
    VAMIGA_PUBLIC
    return cia->getStats();
}


//
// Components (Copper)
//

const CopperInfo &
CopperAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return copper->getInfo();
}

const CopperInfo &
CopperAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return copper->getCachedInfo();
}

string
CopperAPI::disassemble(isize list, isize offset, bool symbolic) const
{
    VAMIGA_PUBLIC_SUSPEND
    return copper->debugger.disassemble(list, offset, symbolic);
}

string
CopperAPI::disassemble(u32 addr, bool symbolic) const
{
    VAMIGA_PUBLIC_SUSPEND
    return copper->debugger.disassemble(addr, symbolic);
}

bool
CopperAPI::isIllegalInstr(u32 addr) const
{
    VAMIGA_PUBLIC_SUSPEND
    return copper->isIllegalInstr(addr);
}


//
// Components (CPU)
//

isize
GuardsAPI::elements() const
{
    VAMIGA_PUBLIC
    return guards->elements();
}

std::optional<GuardInfo>
GuardsAPI::guardNr(long nr) const
{
    VAMIGA_PUBLIC
    return guards->guardNr(nr);
}

std::optional<GuardInfo>
GuardsAPI::guardAt(u32 target) const
{
    VAMIGA_PUBLIC
    return guards->guardAt(target);
}

void
GuardsAPI::setAt(u32 target, isize ignores)
{
    VAMIGA_PUBLIC_SUSPEND
    emu->put(Command(Cmd::GUARD_SET_AT, (void *)guards, target, ignores));
}

void
GuardsAPI::moveTo(isize nr, u32 newTarget)
{
    VAMIGA_PUBLIC_SUSPEND
    emu->put(Command(Cmd::GUARD_MOVE_NR, (void *)guards, nr, newTarget));
}

void
GuardsAPI::remove(isize nr)
{
    VAMIGA_PUBLIC_SUSPEND
    emu->put(Command(Cmd::GUARD_REMOVE_NR, (void *)guards, nr));
}

void
GuardsAPI::removeAt(u32 target)
{
    VAMIGA_PUBLIC_SUSPEND
    emu->put(Command(Cmd::GUARD_REMOVE_AT, (void *)guards, target));
}

void
GuardsAPI::removeAll()
{
    VAMIGA_PUBLIC_SUSPEND
    emu->put(Command(Cmd::GUARD_REMOVE_ALL, (void *)guards));
}

void
GuardsAPI::enable(isize nr)
{
    VAMIGA_PUBLIC_SUSPEND
    emu->put(Command(Cmd::GUARD_ENABLE_NR, (void *)guards, nr));
}

void
GuardsAPI::enableAt(u32 target)
{
    VAMIGA_PUBLIC_SUSPEND
    emu->put(Command(Cmd::GUARD_ENABLE_AT, (void *)guards, target));
}

void
GuardsAPI::enableAll()
{
    VAMIGA_PUBLIC_SUSPEND
    emu->put(Command(Cmd::GUARD_ENABLE_ALL, (void *)guards));
}

void
GuardsAPI::disable(isize nr)
{
    VAMIGA_PUBLIC_SUSPEND
    emu->put(Command(Cmd::GUARD_DISABLE_NR, (void *)guards, nr));
}

void
GuardsAPI::disableAt(u32 target)
{
    VAMIGA_PUBLIC_SUSPEND
    emu->put(Command(Cmd::GUARD_DISABLE_AT, (void *)guards, target));
}

void
GuardsAPI::disableAll()
{
    VAMIGA_PUBLIC_SUSPEND
    emu->put(Command(Cmd::GUARD_DISABLE_ALL));
}

void
GuardsAPI::toggle(isize nr)
{
    VAMIGA_PUBLIC_SUSPEND
    guards->toggle(nr);
}

isize
CPUDebuggerAPI::loggedInstructions() const
{
    VAMIGA_PUBLIC
    return cpu->debugger.loggedInstructions();
}

void
CPUDebuggerAPI::clearLog()
{
    VAMIGA_PUBLIC_SUSPEND
    return cpu->debugger.clearLog();
}

const char *
CPUDebuggerAPI::disassembleRecordedInstr(isize i, isize *len)
{
    VAMIGA_PUBLIC_SUSPEND
    return cpu->disassembleRecordedInstr(i, len);
}

const char *
CPUDebuggerAPI::disassembleRecordedWords(isize i, isize len)
{
    VAMIGA_PUBLIC_SUSPEND
    return cpu->disassembleRecordedWords(i, len);
}

const char *
CPUDebuggerAPI::disassembleRecordedFlags(isize i)
{
    VAMIGA_PUBLIC_SUSPEND
    return cpu->disassembleRecordedFlags(i);
}

const char *
CPUDebuggerAPI::disassembleRecordedPC(isize i)
{
    VAMIGA_PUBLIC_SUSPEND
    return cpu->disassembleRecordedPC(i);
}

const char *
CPUDebuggerAPI::disassembleWord(u16 value)
{
    VAMIGA_PUBLIC_SUSPEND
    return cpu->disassembleWord(value);
}

const char *
CPUDebuggerAPI::disassembleAddr(u32 addr)
{
    VAMIGA_PUBLIC_SUSPEND
    return cpu->disassembleAddr(addr);
}

const char *
CPUDebuggerAPI::disassembleInstr(u32 addr, isize *len)
{
    VAMIGA_PUBLIC_SUSPEND
    return cpu->disassembleInstr(addr, len);
}

const char *
CPUDebuggerAPI::disassembleWords(u32 addr, isize len)
{
    VAMIGA_PUBLIC_SUSPEND
    return cpu->disassembleWords(addr, len);
}

string
CPUDebuggerAPI::vectorName(isize i)
{
    VAMIGA_PUBLIC_SUSPEND
    return cpu->debugger.vectorName(u8(i));
}

const CPUConfig &
CPUAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return cpu->getConfig();
}

const CPUInfo &
CPUAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return cpu->getInfo();
}

const CPUInfo &
CPUAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return cpu->getCachedInfo();
}


//
// Components (Denise)
//

const DeniseConfig &
DeniseAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return denise->getConfig();
}

const DeniseInfo &
DeniseAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return denise->getInfo();
}

const DeniseInfo &
DeniseAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return denise->getCachedInfo();
}


//
// Components (Memory)
//

MemSrc
MemoryDebuggerAPI::getMemSrc(Accessor acc, u32 addr) const
{
    VAMIGA_PUBLIC
    
    switch (acc) {
            
        case Accessor::CPU:      return mem->getMemSrc<Accessor::CPU>(addr);
        case Accessor::AGNUS:    return mem->getMemSrc<Accessor::AGNUS>(addr);
            
        default:
            fatalError;
    }
}

u8
MemoryDebuggerAPI::spypeek8(Accessor acc, u32 addr) const
{
    VAMIGA_PUBLIC
    
    switch (acc) {
            
        case Accessor::CPU:      return mem->spypeek8<Accessor::CPU>(addr);
        case Accessor::AGNUS:    return mem->spypeek8<Accessor::AGNUS>(addr);
            
        default:
            fatalError;
    }
}

u16
MemoryDebuggerAPI::spypeek16(Accessor acc, u32 addr) const
{
    VAMIGA_PUBLIC
    
    switch (acc) {
            
        case Accessor::CPU:      return mem->spypeek16<Accessor::CPU>(addr);
        case Accessor::AGNUS:    return mem->spypeek16<Accessor::AGNUS>(addr);
            
        default:
            fatalError;
    }
}

string
MemoryDebuggerAPI::ascDump(Accessor acc, u32 addr, isize bytes) const
{
    VAMIGA_PUBLIC
    
    switch (acc) {
            
        case Accessor::CPU:      return mem->debugger.ascDump<Accessor::CPU>(addr, bytes);
        case Accessor::AGNUS:    return mem->debugger.ascDump<Accessor::AGNUS>(addr, bytes);
            
        default:
            fatalError;
    }
}

string
MemoryDebuggerAPI::hexDump(Accessor acc, u32 addr, isize bytes, isize sz) const
{
    VAMIGA_PUBLIC
    
    switch (acc) {
            
        case Accessor::CPU:      return mem->debugger.hexDump<Accessor::CPU>(addr, bytes, sz);
        case Accessor::AGNUS:    return mem->debugger.hexDump<Accessor::AGNUS>(addr, bytes, sz);
            
        default:
            fatalError;
    }
}

string
MemoryDebuggerAPI::memDump(Accessor acc, u32 addr, isize bytes, isize sz) const
{
    VAMIGA_PUBLIC
    
    switch (acc) {
            
        case Accessor::CPU:      return mem->debugger.memDump<Accessor::CPU>(addr, bytes, sz);
        case Accessor::AGNUS:    return mem->debugger.memDump<Accessor::AGNUS>(addr, bytes, sz);
            
        default:
            fatalError;
    }
}

const MemConfig &
MemoryAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return mem->getConfig();
}

const MemInfo &
MemoryAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return mem->getInfo();
}

const MemInfo &
MemoryAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return mem->getCachedInfo();
}

const MemStats &
MemoryAPI::getStats() const
{
    VAMIGA_PUBLIC
    return mem->getStats();
}

const RomTraits &
MemoryAPI::getRomTraits() const
{
    VAMIGA_PUBLIC
    return mem->getRomTraits();
}

const RomTraits &
MemoryAPI::getWomTraits() const
{
    VAMIGA_PUBLIC
    return mem->getWomTraits();
}

const RomTraits &
MemoryAPI::getExtTraits() const
{
    VAMIGA_PUBLIC
    return mem->getExtTraits();
}

void
MemoryAPI::loadRom(const fs::path &path)
{
    VAMIGA_PUBLIC_SUSPEND
    mem->loadRom(path);
    emu->isDirty = true;
}

void
MemoryAPI::loadExt(const fs::path &path)
{
    VAMIGA_PUBLIC_SUSPEND
    mem->loadExt(path);
    emu->isDirty = true;
}

void
MemoryAPI::loadRom(MediaFile &file)
{
    VAMIGA_PUBLIC_SUSPEND
    mem->loadRom(file);
    emu->isDirty = true;
}

void
MemoryAPI::loadExt(MediaFile &file)
{
    VAMIGA_PUBLIC_SUSPEND
    mem->loadExt(file);
    emu->isDirty = true;
}

void
MemoryAPI::loadRom(const u8 *buf, isize len)
{
    VAMIGA_PUBLIC_SUSPEND
    mem->loadRom(buf, len);
    emu->isDirty = true;
}

void
MemoryAPI::loadExt(const u8 *buf, isize len)
{
    VAMIGA_PUBLIC_SUSPEND
    mem->loadExt(buf, len);
    emu->isDirty = true;
}

void
MemoryAPI::saveRom(const fs::path &path)
{
    VAMIGA_PUBLIC_SUSPEND
    mem->saveRom(path);
}

void
MemoryAPI::saveWom(const fs::path &path)
{
    VAMIGA_PUBLIC_SUSPEND
    mem->saveWom(path);
}

void
MemoryAPI::saveExt(const fs::path &path)
{
    VAMIGA_PUBLIC_SUSPEND
    mem->saveExt(path);
}

void
MemoryAPI::deleteRom()
{
    VAMIGA_PUBLIC_SUSPEND
    mem->deleteRom();
    emu->isDirty = true;
}

void
MemoryAPI::deleteWom()
{
    VAMIGA_PUBLIC_SUSPEND
    mem->deleteWom();
    emu->isDirty = true;
}

void
MemoryAPI::deleteExt()
{
    VAMIGA_PUBLIC_SUSPEND
    mem->deleteExt();
    emu->isDirty = true;
}


//
// Components (Paula)
//

const StateMachineInfo &
AudioChannelAPI::getInfo() const
{
    VAMIGA_PUBLIC
    
    switch (channel) {
            
        case 0:     return paula->channel0.getInfo();
        case 1:     return paula->channel1.getInfo();
        case 2:     return paula->channel2.getInfo();
        default:    return paula->channel3.getInfo();
    }
}

const StateMachineInfo &
AudioChannelAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    
    switch (channel) {
            
        case 0:     return paula->channel0.getCachedInfo();
        case 1:     return paula->channel1.getCachedInfo();
        case 2:     return paula->channel2.getCachedInfo();
        default:    return paula->channel3.getCachedInfo();
    }
}

const DiskControllerConfig &
DiskControllerAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return diskController->getConfig();
}

const DiskControllerInfo &
DiskControllerAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return diskController->getInfo();
}

const DiskControllerInfo &
DiskControllerAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return diskController->getCachedInfo();
}

const UARTInfo &
UARTAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return uart->getInfo();
}

const UARTInfo &
UARTAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return uart->getCachedInfo();
}

const PaulaInfo &
PaulaAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return paula->getInfo();
}

const PaulaInfo &
PaulaAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return paula->getCachedInfo();
}


//
// Components (RTC)
//

const RTCConfig &
RTCAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return rtc->getConfig();
}

void
RTCAPI::update()
{
    VAMIGA_PUBLIC_SUSPEND
    rtc->update();
    emu->isDirty = true;
}


//
// Ports
//


//
// Ports (AudioPort)
//

const AudioPortConfig &
AudioPortAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return port->getConfig();
}

const AudioPortStats &
AudioPortAPI::getStats() const
{
    VAMIGA_PUBLIC
    return port->getStats();
}

isize
AudioPortAPI::copyMono(float *buffer, isize n)
{
    VAMIGA_PUBLIC
    return port->copyMono(buffer, n);
}

isize
AudioPortAPI::copyStereo(float *left, float *right, isize n)
{
    VAMIGA_PUBLIC
    return port->copyStereo(left, right, n);
}

isize
AudioPortAPI::copyInterleaved(float *buffer, isize n)
{
    VAMIGA_PUBLIC
    return port->copyInterleaved(buffer, n);
}

void
AudioPortAPI::drawL(u32 *buffer, isize width, isize height, u32 color) const
{
    VAMIGA_PUBLIC
    port->stream.drawL(buffer, width, height, color);
}

void
AudioPortAPI::drawR(u32 *buffer, isize width, isize height, u32 color) const
{
    VAMIGA_PUBLIC
    port->stream.drawR(buffer, width, height, color);
}


//
// Ports (ControlPort)
//

const ControlPortInfo &
ControlPortAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return controlPort->getInfo();
}

const ControlPortInfo &
ControlPortAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return controlPort->getCachedInfo();
}


//
// Ports (SerialPort)
//

const SerialPortConfig &
SerialPortAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return serialPort->getConfig();
}

const SerialPortInfo &
SerialPortAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return serialPort->getInfo();
}

const SerialPortInfo &
SerialPortAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return serialPort->getCachedInfo();
}

int
SerialPortAPI::readIncomingPrintableByte() const
{
    VAMIGA_PUBLIC
    return serialPort->readIncomingPrintableByte();
}

int
SerialPortAPI::readOutgoingPrintableByte() const
{
    VAMIGA_PUBLIC
    return serialPort->readOutgoingPrintableByte();
}


//
// Ports (VideoPort)
//

void
VideoPortAPI::lockTexture()
{
    VAMIGA_PUBLIC
    emu->lockTexture();
}

void
VideoPortAPI::unlockTexture()
{
    VAMIGA_PUBLIC
    emu->unlockTexture();
}

const u32 *
VideoPortAPI::getTexture() const
{
    VAMIGA_PUBLIC
    return (u32 *)emu->getTexture().pixels.ptr;
}

const u32 *
VideoPortAPI::getTexture(isize *nr, bool *lof, bool *prevlof) const
{
    VAMIGA_PUBLIC
    
    auto &frameBuffer = emu->getTexture();
    
    *nr = isize(frameBuffer.nr);
    *lof = frameBuffer.lof;
    *prevlof = frameBuffer.prevlof;
    
    return (u32 *)frameBuffer.pixels.ptr;
}

void
VideoPortAPI::findInnerArea(isize &x1, isize &x2, isize &y1, isize &y2) const
{
    VAMIGA_PUBLIC_SUSPEND
    videoPort->findInnerArea(x1, x2, y1, y2);
}

void
VideoPortAPI::findInnerAreaNormalized(double &x1, double &x2, double &y1, double &y2) const
{
    VAMIGA_PUBLIC_SUSPEND
    videoPort->findInnerAreaNormalized(x1, x2, y1, y2);
}

//
// Peripherals
//

//
// Peripherals (Keyboard)
//

const KeyboardInfo &
KeyboardAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return keyboard->getInfo();
}

const KeyboardInfo &
KeyboardAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return keyboard->getCachedInfo();
}

bool
KeyboardAPI::isPressed(KeyCode key) const
{
    VAMIGA_PUBLIC
    return keyboard->isPressed(key);
}

void
KeyboardAPI::press(KeyCode key, double delay, double duration)
{
    VAMIGA_PUBLIC
    if (delay == 0.0) {

        keyboard->press(key);
        emu->isDirty = true;

    } else {
        
        emu->put(Command(Cmd::KEY_PRESS, KeyCommand { .keycode = key, .delay = delay }));
    }
    if (duration != 0.0) {
        
        emu->put(Command(Cmd::KEY_RELEASE, KeyCommand { .keycode = key, .delay = delay + duration }));
    }
}

void
KeyboardAPI::toggle(KeyCode key, double delay, double duration)
{
    VAMIGA_PUBLIC
    if (delay == 0.0) {
        
        keyboard->toggle(key);
        emu->isDirty = true;
        
    } else {
        
        emu->put(Command(Cmd::KEY_TOGGLE, KeyCommand { .keycode = key, .delay = delay }));
    }
    if (duration != 0.0) {
        
        emu->put(Command(Cmd::KEY_TOGGLE, KeyCommand { .keycode = key, .delay = delay + duration }));
    }
}

void
KeyboardAPI::release(KeyCode key, double delay)
{
    VAMIGA_PUBLIC
    if (delay == 0.0) {
        
        keyboard->release(key);
        emu->isDirty = true;
        
    } else {
        
        emu->put(Command(Cmd::KEY_RELEASE, KeyCommand { .keycode = key, .delay = delay }));
    }
}

void
KeyboardAPI::releaseAll()
{
    VAMIGA_PUBLIC
    emu->put(Command(Cmd::KEY_RELEASE_ALL));
}

void KeyboardAPI::abortTyping()
{
    VAMIGA_PUBLIC
    keyboard->abortTyping();
}


//
// Peripherals (FloppyDrive)
//

const FloppyDriveConfig &
FloppyDriveAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return drive->getConfig();
}

const FloppyDriveInfo &
FloppyDriveAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return drive->getInfo();
}

const FloppyDriveInfo &
FloppyDriveAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return drive->getCachedInfo();
}

FloppyDisk &
FloppyDriveAPI::getDisk() const
{
    VAMIGA_PUBLIC_SUSPEND
    return *(drive->disk);
}

bool
FloppyDriveAPI::getFlag(DiskFlags mask) const
{
    VAMIGA_PUBLIC
    return drive->getFlag(mask);
}

void
FloppyDriveAPI::setFlag(DiskFlags mask, bool value)
{
    VAMIGA_PUBLIC_SUSPEND
    drive->setFlag(mask, value);
    emu->isDirty = true;
}

bool 
FloppyDriveAPI::isInsertable(Diameter t, Density d) const
{
    VAMIGA_PUBLIC
    return drive->isInsertable(t, d);
}

void
FloppyDriveAPI::insertBlankDisk(FSVolumeType fstype, BootBlockId bb, string name)
{
    VAMIGA_PUBLIC_SUSPEND
    drive->insertNew(fstype, bb, name);
    emu->isDirty = true;
}

void
FloppyDriveAPI::insertMedia(MediaFile &file, bool wp)
{
    VAMIGA_PUBLIC_SUSPEND
    drive->insertMediaFile(file, wp);
    emu->isDirty = true;
}

void
FloppyDriveAPI::ejectDisk()
{
    VAMIGA_PUBLIC_SUSPEND
    drive->ejectDisk();
}

class MediaFile *
FloppyDriveAPI::exportDisk(FileType type)
{
    VAMIGA_PUBLIC_SUSPEND
    return drive->exportDisk(type);
}

string
FloppyDriveAPI::readTrackBits(isize track)
{
    VAMIGA_PUBLIC_SUSPEND
    return drive->readTrackBits(track);
}


//
// Peripherals (HardDrive)
//

class HardDrive &
HardDriveAPI::getDrive()
{
    VAMIGA_PUBLIC
    return *drive;
}

const HardDriveConfig &
HardDriveAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return drive->getConfig();
}

const HardDriveInfo &
HardDriveAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return drive->getInfo();
}

const HardDriveInfo &
HardDriveAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return drive->getCachedInfo();
}

const HardDriveTraits &
HardDriveAPI::getTraits() const
{
    VAMIGA_PUBLIC
    return drive->getTraits();
}

const PartitionTraits &
HardDriveAPI::getPartitionTraits(isize nr) const
{
    VAMIGA_PUBLIC
    return drive->getPartitionTraits(nr);
}

bool
HardDriveAPI::getFlag(DiskFlags mask)
{
    VAMIGA_PUBLIC
    return drive->getFlag(mask);
}

void
HardDriveAPI::setFlag(DiskFlags mask, bool value)
{
    VAMIGA_PUBLIC_SUSPEND
    drive->setFlag(mask, value);
}

std::vector<std::tuple<isize,isize,isize>>
HardDriveAPI::geometries(isize numBlocks) const
{
    VAMIGA_PUBLIC_SUSPEND
    return GeometryDescriptor::driveGeometries(numBlocks);
}

void 
HardDriveAPI::changeGeometry(isize c, isize h, isize s, isize b)
{
    VAMIGA_PUBLIC_SUSPEND
    return drive->changeGeometry(c, h, s, b);
}

void
HardDriveAPI::attach(const fs::path &path)
{
    VAMIGA_PUBLIC_SUSPEND
    drive->init(path.string());
}

void 
HardDriveAPI::attach(const MediaFile &file)
{
    VAMIGA_PUBLIC_SUSPEND
    drive->init(file);
}

void
HardDriveAPI::attach(isize c, isize h, isize s, isize b)
{
    VAMIGA_PUBLIC_SUSPEND
    auto geometry = GeometryDescriptor(c, h, s, b);
    drive->init(geometry);
}

void 
HardDriveAPI::format(FSVolumeType fs, const string &name)
{
    VAMIGA_PUBLIC_SUSPEND
    drive->format(fs, name);
}

void 
HardDriveAPI::writeToFile(fs::path path)
{
    VAMIGA_PUBLIC_SUSPEND
    drive->writeToFile(path);
}

MediaFile *
HardDriveAPI::createHDF()
{
    VAMIGA_PUBLIC_SUSPEND
    return new HDFFile(*drive);
}


//
// Peripherals (HdController)
//

const HdcInfo &
HdControllerAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return controller->getInfo();
}

const HdcInfo &
HdControllerAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return controller->getCachedInfo();
}

const HdcStats &
HdControllerAPI::getStats() const
{
    VAMIGA_PUBLIC
    return controller->getStats();
}


//
// Peripherals (Joystick)
//

const JoystickInfo &
JoystickAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return joystick->getInfo();
}

const JoystickInfo &
JoystickAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return joystick->getCachedInfo();
}

void 
JoystickAPI::trigger(GamePadAction event)
{
    VAMIGA_PUBLIC
    emu->put(Cmd::JOY_EVENT, GamePadCommand { .port = joystick->objid, .action = event });
}


//
// Mouse
//

bool 
MouseAPI::detectShakeXY(double x, double y)
{
    VAMIGA_PUBLIC
    return mouse->detectShakeXY(x, y);
}

bool 
MouseAPI::detectShakeDxDy(double dx, double dy)
{
    VAMIGA_PUBLIC
    return mouse->detectShakeDxDy(dx, dy);
}

void 
MouseAPI::setXY(double x, double y)
{
    VAMIGA_PUBLIC
    emu->put(Command(Cmd::MOUSE_MOVE_ABS, CoordCommand { .port = mouse->objid, .x = x, .y = y }));
}

void 
MouseAPI::setDxDy(double dx, double dy)
{
    VAMIGA_PUBLIC
    emu->put(Command(Cmd::MOUSE_MOVE_REL, CoordCommand { .port = mouse->objid, .x = dx, .y = dy }));
}

void 
MouseAPI::trigger(GamePadAction action)
{
    VAMIGA_PUBLIC
    emu->put(Command(Cmd::MOUSE_BUTTON, GamePadCommand { .port = mouse->objid, .action = action }));
}


//
// DefaultsAPI
//

void
DefaultsAPI::load(const fs::path &path)
{
    VAMIGA_PUBLIC
    defaults->load(path);
}

void
DefaultsAPI::load(std::ifstream &stream)
{
    VAMIGA_PUBLIC
    defaults->load(stream);
}

void
DefaultsAPI::load(std::stringstream &stream)
{
    VAMIGA_PUBLIC
    defaults->load(stream);
}

void
DefaultsAPI::save(const fs::path &path)
{
    VAMIGA_PUBLIC
    defaults->save(path);
}

void
DefaultsAPI::save(std::ofstream &stream)
{
    VAMIGA_PUBLIC
    defaults->save(stream);
}

void
DefaultsAPI::save(std::stringstream &stream)
{
    VAMIGA_PUBLIC
    defaults->save(stream);
}

string
DefaultsAPI::getRaw(const string &key) const
{
    VAMIGA_PUBLIC
    return defaults->getRaw(key);
}

i64
DefaultsAPI::get(const string &key) const
{
    VAMIGA_PUBLIC
    return defaults->get(key);
}

i64
DefaultsAPI::get(Opt option, isize nr) const
{
    VAMIGA_PUBLIC
    return defaults->get(option, nr);
}

string
DefaultsAPI::getFallbackRaw(const string &key) const
{
    VAMIGA_PUBLIC
    return defaults->getFallbackRaw(key);
}

i64
DefaultsAPI::getFallback(const string &key) const
{
    VAMIGA_PUBLIC
    return defaults->getFallback(key);
}

i64
DefaultsAPI::getFallback(Opt option, isize nr) const
{
    VAMIGA_PUBLIC
    return defaults->getFallback(option, nr);
}

void
DefaultsAPI::set(const string &key, const string &value)
{
    VAMIGA_PUBLIC
    defaults->set(key, value);
}

void
DefaultsAPI::set(Opt opt, const string &value)
{
    VAMIGA_PUBLIC
    defaults->set(opt, value);
}

void
DefaultsAPI::set(Opt opt, const string &value, std::vector<isize> objids)
{
    VAMIGA_PUBLIC
    defaults->set(opt, value, objids);
}

void
DefaultsAPI::set(Opt opt, i64 value)
{
    VAMIGA_PUBLIC
    defaults->set(opt, value);
}

void
DefaultsAPI::set(Opt opt, i64 value, std::vector<isize> objids)
{
    VAMIGA_PUBLIC
    defaults->set(opt, value, objids);
}

void
DefaultsAPI::setFallback(const string &key, const string &value)
{
    VAMIGA_PUBLIC
    defaults->setFallback(key, value);
}

void
DefaultsAPI::setFallback(Opt opt, const string &value)
{
    VAMIGA_PUBLIC
    defaults->setFallback(opt, value);
}

void
DefaultsAPI::setFallback(Opt opt, const string &value, std::vector<isize> objids)
{
    VAMIGA_PUBLIC
    defaults->setFallback(opt, value, objids);
}

void
DefaultsAPI::setFallback(Opt opt, i64 value)
{
    VAMIGA_PUBLIC
    defaults->setFallback(opt, value);
}

void
DefaultsAPI::setFallback(Opt opt, i64 value, std::vector<isize> objids)
{
    VAMIGA_PUBLIC
    defaults->setFallback(opt, value, objids);
}

void
DefaultsAPI::remove()
{
    VAMIGA_PUBLIC
    defaults->remove();
}

void
DefaultsAPI::remove(const string &key)
{
    VAMIGA_PUBLIC
    defaults->remove(key);
}

void
DefaultsAPI::remove(Opt option)
{
    VAMIGA_PUBLIC
    defaults->remove(option);
}

void
DefaultsAPI::remove(Opt option, std::vector <isize> objids)
{
    VAMIGA_PUBLIC
    defaults->remove(option, objids);
}


//
// RecorderAPI
//

const RecorderConfig &
RecorderAPI::getConfig() const
{
    VAMIGA_PUBLIC
    return recorder->getConfig();
}

/*
const RecorderInfo &
RecorderAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return recorder->getInfo();
}

const RecorderInfo &
RecorderAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return recorder->getCachedInfo();
}
*/

double RecorderAPI::getDuration() const { VAMIGA_PUBLIC_SUSPEND return recorder->getDuration().asSeconds(); }
/*
isize RecorderAPI::getFrameRate() const { VAMIGA_PUBLIC VAMIGA_SUSPEND return recorder->getFrameRate(); }
isize RecorderAPI::getBitRate() const { VAMIGA_PUBLIC VAMIGA_SUSPEND return recorder->getBitRate(); }
isize RecorderAPI::getSampleRate() const { VAMIGA_PUBLIC VAMIGA_SUSPEND return recorder->getSampleRate(); }
*/
bool RecorderAPI::isRecording() const { VAMIGA_PUBLIC_SUSPEND return recorder->isRecording(); }

const std::vector<fs::path> &
RecorderAPI::paths() const
{
    VAMIGA_PUBLIC
    return FFmpeg::paths;
}

bool 
RecorderAPI::hasFFmpeg() const
{
    VAMIGA_PUBLIC
    return FFmpeg::available();
}

const fs::path
RecorderAPI::getExecPath() const
{
    VAMIGA_PUBLIC
    return FFmpeg::getExecPath();
}

void RecorderAPI::setExecPath(const fs::path &path)
{
    VAMIGA_PUBLIC
    FFmpeg::setExecPath(path);
}

void
RecorderAPI::startRecording(isize x1, isize y1, isize x2, isize y2,
                            isize bitRate,
                            isize aspectX, isize aspectY)
{
    VAMIGA_PUBLIC_SUSPEND
    recorder->startRecording(x1, y1, x2, y2, bitRate, aspectX, aspectY);
}

void
RecorderAPI::stopRecording()
{
    VAMIGA_PUBLIC_SUSPEND
    recorder->stopRecording();
}

bool
RecorderAPI::exportAs(const fs::path &path)
{
    VAMIGA_PUBLIC_SUSPEND
    return recorder->exportAs(path);
}


//
// RemoteManagerAPI
//

const RemoteManagerInfo &
RemoteManagerAPI::getInfo() const
{
    VAMIGA_PUBLIC
    return remoteManager->getInfo();
}

const RemoteManagerInfo &
RemoteManagerAPI::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return remoteManager->getCachedInfo();
}


// RetroShellAPI
//

const char *
RetroShellAPI::text()
{
    VAMIGA_PUBLIC_SUSPEND
    return retroShell->text();
}

isize
RetroShellAPI::cursorRel()
{
    VAMIGA_PUBLIC_SUSPEND
    return retroShell->cursorRel();
}

void
RetroShellAPI::press(RetroShellKey key, bool shift)
{
    VAMIGA_PUBLIC_SUSPEND
    retroShell->press(key, shift);
}

void
RetroShellAPI::press(char c)
{
    VAMIGA_PUBLIC_SUSPEND
    retroShell->press(c);
}

void
RetroShellAPI::press(const string &s)
{
    VAMIGA_PUBLIC_SUSPEND
    retroShell->press(s);
}

void
RetroShellAPI::execScript(std::stringstream &ss)
{
    VAMIGA_PUBLIC_SUSPEND
    retroShell->asyncExecScript(ss);
}

void
RetroShellAPI::execScript(const std::ifstream &fs)
{
    VAMIGA_PUBLIC_SUSPEND
    retroShell->asyncExecScript(fs);
}

void
RetroShellAPI::execScript(const string &contents)
{
    VAMIGA_PUBLIC_SUSPEND
    retroShell->asyncExecScript(contents);
}

void
RetroShellAPI::execScript(const MediaFile &file)
{
    VAMIGA_PUBLIC_SUSPEND
    retroShell->asyncExecScript(file);
}

void
RetroShellAPI::setStream(std::ostream &os)
{
    VAMIGA_PUBLIC_SUSPEND
    retroShell->setStream(os);
}


//
// VAmiga API
//

VAmiga::VAmiga() {

    emu = new Emulator();

    // Wire all APIs...

    // Components
    amiga.emu = emu;
    amiga.amiga = &emu->main;

    agnus.emu = emu;
    agnus.agnus = &emu->main.agnus;
    agnus.logicAnalyzer.emu = emu;
    agnus.logicAnalyzer.logicAnalyzer = &emu->main.logicAnalyzer;
    agnus.dmaDebugger.emu = emu;
    agnus.dmaDebugger.dmaDebugger = &emu->main.agnus.dmaDebugger;
    agnus.copper.emu = emu;
    agnus.copper.copper = &emu->main.agnus.copper;
    agnus.blitter.emu = emu;
    agnus.blitter.blitter = &emu->main.agnus.blitter;

    ciaA.emu = emu;
    ciaA.cia = &emu->main.ciaA;

    ciaB.emu = emu;
    ciaB.cia = &emu->main.ciaB;

    // Ports
    audioPort.emu = emu;
    audioPort.port = &emu->main.audioPort;

    controlPort1.emu = emu;
    controlPort1.controlPort = &emu->main.controlPort1;
    controlPort1.joystick.emu = emu;
    controlPort1.joystick.joystick = &emu->main.controlPort1.joystick;
    controlPort1.mouse.emu = emu;
    controlPort1.mouse.mouse = &emu->main.controlPort1.mouse;

    controlPort2.emu = emu;
    controlPort2.controlPort = &emu->main.controlPort2;
    controlPort2.joystick.emu = emu;
    controlPort2.joystick.joystick = &emu->main.controlPort2.joystick;
    controlPort2.mouse.emu = emu;
    controlPort2.mouse.mouse = &emu->main.controlPort2.mouse;

    copperBreakpoints.emu = emu;
    copperBreakpoints.guards = &emu->main.agnus.copper.debugger.breakpoints;

    cpu.emu = emu;
    cpu.cpu = &emu->main.cpu;
    cpu.debugger.emu = emu;
    cpu.debugger.cpu = &emu->main.cpu;
    cpu.breakpoints.emu = emu;
    cpu.breakpoints.guards = &emu->main.cpu.breakpoints;
    cpu.watchpoints.emu = emu;
    cpu.watchpoints.guards = &emu->main.cpu.watchpoints;

    denise.emu = emu;
    denise.denise = &emu->main.denise;

    df0.emu = emu;
    df0.drive = &emu->main.df0;

    df1.emu = emu;
    df1.drive = &emu->main.df1;

    df2.emu = emu;
    df2.drive = &emu->main.df2;

    df3.emu = emu;
    df3.drive = &emu->main.df3;

    hd0.emu = emu;
    hd0.drive = &emu->main.hd0;
    hd0.controller.emu = emu;
    hd0.controller.controller = &emu->main.hd0con;

    hd1.emu = emu;
    hd1.drive = &emu->main.hd1;
    hd1.controller.emu = emu;
    hd1.controller.controller = &emu->main.hd1con;

    hd2.emu = emu;
    hd2.drive = &emu->main.hd2;
    hd2.controller.emu = emu;
    hd2.controller.controller = &emu->main.hd2con;

    hd3.emu = emu;
    hd3.drive = &emu->main.hd3;
    hd3.controller.emu = emu;
    hd3.controller.controller = &emu->main.hd3con;
    
    keyboard.emu = emu;
    keyboard.keyboard = &emu->main.keyboard;

    mem.emu = emu;
    mem.mem = &emu->main.mem;
    mem.debugger.emu = emu;
    mem.debugger.mem = &emu->main.mem;

    paula.emu = emu;
    paula.paula = &emu->main.paula;
    paula.audioChannel0.emu = emu;
    paula.audioChannel0.paula = &emu->main.paula;
    paula.audioChannel1.emu = emu;
    paula.audioChannel1.paula = &emu->main.paula;
    paula.audioChannel2.emu = emu;
    paula.audioChannel2.paula = &emu->main.paula;
    paula.audioChannel3.emu = emu;
    paula.audioChannel3.paula = &emu->main.paula;
    paula.diskController.emu = emu;
    paula.diskController.diskController = &emu->main.paula.diskController;
    paula.uart.emu = emu;
    paula.uart.uart = &emu->main.paula.uart;

    retroShell.emu = emu;
    retroShell.retroShell = &emu->main.retroShell;

    rtc.emu = emu;
    rtc.rtc = &emu->main.rtc;

    recorder.emu = emu;
    recorder.recorder = &emu->main.denise.screenRecorder;

    remoteManager.emu = emu;
    remoteManager.remoteManager = &emu->main.remoteManager;

    serialPort.emu = emu;
    serialPort.serialPort = &emu->main.serialPort;

    videoPort.emu = emu;
    videoPort.videoPort = &emu->main.videoPort;
    
    // watchpoints.emu = emu;
    // watchpoints.guards = &emu->main.cpu.debugger.watchpoints;
}

VAmiga::~VAmiga()
{
    VAMIGA_PUBLIC
    halt();
    delete emu;
}

string
VAmiga::version()
{
    return Amiga::version();
}

string
VAmiga::build()
{
    return Amiga::build();
}

const EmulatorInfo &
VAmiga::getInfo() const
{
    VAMIGA_PUBLIC
    return emu->getInfo();
}

const EmulatorInfo &
VAmiga::getCachedInfo() const
{
    VAMIGA_PUBLIC
    return emu->getCachedInfo();
}

const EmulatorStats &
VAmiga::getStats() const
{
    VAMIGA_PUBLIC
    return emu->getStats();
}

bool
VAmiga::isPoweredOn() const
{
    VAMIGA_PUBLIC
    return emu->main.isPoweredOn();
}

bool
VAmiga::isPoweredOff() const
{
    VAMIGA_PUBLIC
    return emu->main.isPoweredOff();
}

bool
VAmiga::isPaused() const
{
    VAMIGA_PUBLIC
    return emu->main.isPaused();
}

bool
VAmiga::isRunning() const
{
    VAMIGA_PUBLIC
    return emu->main.isRunning();
}

bool
VAmiga::isSuspended() const
{
    VAMIGA_PUBLIC
    return emu->isSuspended();
}

bool
VAmiga::isHalted() const
{
    VAMIGA_PUBLIC
    return emu->main.isHalted();
}

bool
VAmiga::isWarping() const
{
    VAMIGA_PUBLIC
    return emu->isWarping();
}

bool
VAmiga::isTracking() const
{
    VAMIGA_PUBLIC
    return emu->isTracking();
}

void
VAmiga::isReady() const
{
    VAMIGA_PUBLIC
    return emu->isReady();
}

void
VAmiga::powerOn()
{
    VAMIGA_PUBLIC
    emu->put(Cmd::POWER_ON);
}

void
VAmiga::powerOff()
{
    VAMIGA_PUBLIC
    emu->put(Cmd::POWER_OFF);
}

void
VAmiga::run()
{
    VAMIGA_PUBLIC
    
    // Throw an exception if the emulator is not ready to run
    isReady();
    
    emu->put(Cmd::RUN);
}

void
VAmiga::pause()
{
    VAMIGA_PUBLIC
    emu->put(Cmd::PAUSE);
}

void 
VAmiga::hardReset()
{
    VAMIGA_PUBLIC
    emu->put(Cmd::HARD_RESET);
}

void
VAmiga::softReset()
{
    VAMIGA_PUBLIC
    emu->put(Cmd::SOFT_RESET);
}

void
VAmiga::halt()
{
    // Signal the emulator to halt
    emu->put(Cmd::HALT);
    
    // Wait for the thread to terminate
    emu->join();
}

void
VAmiga::suspend() const
{
    VAMIGA_PUBLIC
    emu->suspend();
}

void
VAmiga::resume() const
{
    VAMIGA_PUBLIC
    emu->resume();
}

void
VAmiga::warpOn(isize source)
{
    VAMIGA_PUBLIC
    emu->put(Cmd::WARP_ON, source);
}

void
VAmiga::warpOff(isize source)
{
    VAMIGA_PUBLIC
    emu->put(Cmd::WARP_OFF, source);
}

void
VAmiga::trackOn(isize source)
{
    VAMIGA_PUBLIC_SUSPEND
    emu->trackOn(source);
}

void
VAmiga::trackOff(isize source)
{
    VAMIGA_PUBLIC_SUSPEND
    emu->trackOff(source);
}

void
VAmiga::stepInto()
{
    VAMIGA_PUBLIC
    emu->stepInto();
}

void
VAmiga::stepOver()
{
    VAMIGA_PUBLIC
    emu->stepOver();
}

void
VAmiga::finishLine()
{
    VAMIGA_PUBLIC
    emu->finishLine();
}

void
VAmiga::finishFrame()
{
    VAMIGA_PUBLIC
    emu->finishFrame();
}

void
VAmiga::wakeUp()
{
    VAMIGA_PUBLIC
    emu->wakeUp();
}

void
VAmiga::launch(const void *listener, Callback *func)
{
    VAMIGA_PUBLIC
    emu->launch(listener, func);
}

bool
VAmiga::isLaunched() const
{
    VAMIGA_PUBLIC
    return emu->isLaunched();
}

i64
VAmiga::get(Opt option) const
{
    VAMIGA_PUBLIC
    return emu->get(option);
}

i64
VAmiga::get(Opt option, long id) const
{
    VAMIGA_PUBLIC
    return emu->get(option, id);
}

void
VAmiga::set(ConfigScheme model)
{
    VAMIGA_PUBLIC_SUSPEND
    emu->set(model);
    emu->isDirty = true;
}

void
VAmiga::set(Opt opt, i64 value)
{
    VAMIGA_PUBLIC
    emu->check(opt, value);
    put(Cmd::CONFIG_ALL, ConfigCommand { .option = opt, .value = value });
    emu->isDirty = true;
}

void
VAmiga::set(Opt opt, i64 value, long id)
{
    VAMIGA_PUBLIC
    emu->check(opt, value, { id });
    put(Cmd::CONFIG, ConfigCommand { .option = opt, .value = value, .id = id });
    emu->isDirty = true;
}

void
VAmiga::exportConfig(const fs::path &path, bool diff) const
{
    VAMIGA_PUBLIC_SUSPEND
    emu->main.exportConfig(path, diff);
}

void
VAmiga::exportConfig(std::ostream& stream, bool diff) const
{
    VAMIGA_PUBLIC_SUSPEND
    emu->main.exportConfig(stream, diff);
}

void
VAmiga::put(const Command &cmd)
{
    VAMIGA_PUBLIC
    emu->put(cmd);
}


//
// AmigaAPI
//

MediaFile *
AmigaAPI::takeSnapshot()
{
    VAMIGA_PUBLIC_SUSPEND
    return amiga->takeSnapshot();
}

void 
AmigaAPI::loadSnapshot(const MediaFile &snapshot)
{
    VAMIGA_PUBLIC_SUSPEND
    
    emu->isDirty = true;
    
    try {
        
        // Restore the saved state
        amiga->loadSnapshot(snapshot);
        
    } catch (CoreError &error) {
        
        /* If we reach this point, the emulator has been put into an
         * inconsistent state due to corrupted snapshot data. We cannot
         * continue emulation, because it would likely crash the
         * application. Because we cannot revert to the old state either,
         * we perform a hard reset to eliminate the inconsistency.
         */
        emu->put(Cmd::HARD_RESET);
        throw;
    }
}
 
void
AmigaAPI::loadSnapshot(const fs::path &path)
{
    VAMIGA_PUBLIC_SUSPEND
    amiga->loadSnapshot(path);
}

void
AmigaAPI::saveSnapshot(const fs::path &path) const
{
    VAMIGA_PUBLIC_SUSPEND
    amiga->saveSnapshot(path);
    
}

void
AmigaAPI::loadWorkspace(const fs::path &path)
{
    VAMIGA_PUBLIC_SUSPEND
    amiga->loadWorkspace(path);
}

void
AmigaAPI::saveWorkspace(const fs::path &path) const
{
    VAMIGA_PUBLIC_SUSPEND
    amiga->saveWorkspace(path);
}

u64
AmigaAPI::getAutoInspectionMask() const
{
    VAMIGA_PUBLIC
    return amiga->getAutoInspectionMask();
}

void
AmigaAPI::setAutoInspectionMask(u64 mask)
{
    VAMIGA_PUBLIC_SUSPEND
    amiga->setAutoInspectionMask(mask);
}

bool
AmigaAPI::getMsg(Message &msg)
{
    VAMIGA_PUBLIC
    return amiga->msgQueue.get(msg);
}

}
