// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "VAmiga.h"
#include "Concurrency.h"
#include "Emulator.h"
#include "GuardList.h"

namespace vamiga {

/* All main API functions are annotated with one or more of the following
 * keywords:
 *
 * PUBLIC_API:   This macro performs a sanity check in debug builds by assuring
 *               that the function is called from outside the emulator thread.
 *
 * SUSPENDED:    The macro ensures that the emulator is in suspended state
 *               before the function body is executed. The emulator can suspend
 *               only after the current frame has been completed. Thus, calling
 *               an API function with this annotation may cause a noticable lag.
 *
 * SYNCHRONIZE:  The macro locks a component prior to calling a function, thus
 *               eliminating race-conditions with functions that acquire the
 *               same lock internally.
 */

#define PUBLIC_API assert(!emu || emu->isUserThread());

DefaultsAPI VAmiga::defaults(&Emulator::defaults);
struct SuspendResume {
    const API *api;
    SuspendResume(const API *api) : api(api) {
        assert(!api->emu || api->emu->isUserThread());
        api->suspend();
    }
    ~SuspendResume() { api->resume(); }
};

#define SUSPENDED SuspendResume _sr(this);
// #define SUSPENDED printf("%d: SUSPEND\n", __LINE__); SuspendResume _sr(this);

#define SYNCHRONIZE(obj) util::AutoMutex _am(obj->mutex);

//
// API
//

void
API::suspend() const
{
    PUBLIC_API
    emu->suspend();
}

void
API::resume() const
{
    PUBLIC_API
    emu->resume();
}


//
// Components (Amiga)
//

const AmigaConfig &
AmigaAPI::getConfig() const
{
    PUBLIC_API
    return amiga->getConfig();
}

const AmigaInfo &
AmigaAPI::getInfo() const
{
    PUBLIC_API
    return amiga->getInfo();
}

const AmigaInfo &
AmigaAPI::getCachedInfo() const
{
    PUBLIC_API
    return amiga->getCachedInfo();
}

void
AmigaAPI::dump(Category category, std::ostream& os) const
{
    PUBLIC_API SUSPENDED
    amiga->dump(category, os);
}


//
// Components (Agnus)
//

const LogicAnalyzerConfig &
LogicAnalyzerAPI::getConfig() const
{
    PUBLIC_API
    return logicAnalyzer->getConfig();
}

const LogicAnalyzerInfo &
LogicAnalyzerAPI::getInfo() const
{
    PUBLIC_API
    return logicAnalyzer->getInfo();
}

const LogicAnalyzerInfo &
LogicAnalyzerAPI::getCachedInfo() const
{
    PUBLIC_API
    return logicAnalyzer->getCachedInfo();
}

const DmaDebuggerConfig &
DmaDebuggerAPI::getConfig() const
{
    PUBLIC_API
    return dmaDebugger->getConfig();
}

const DmaDebuggerInfo &
DmaDebuggerAPI::getInfo() const
{
    PUBLIC_API
    return dmaDebugger->getInfo();
}

const DmaDebuggerInfo &
DmaDebuggerAPI::getCachedInfo() const
{
    PUBLIC_API
    return dmaDebugger->getCachedInfo();
}

const AgnusConfig &
AgnusAPI::getConfig() const
{
    PUBLIC_API
    return agnus->getConfig();
}

const AgnusInfo &
AgnusAPI::getInfo() const
{
    PUBLIC_API
    return agnus->getInfo();
}

const AgnusInfo &
AgnusAPI::getCachedInfo() const
{
    PUBLIC_API
    return agnus->getCachedInfo();
}

const AgnusStats &
AgnusAPI::getStats() const
{
    PUBLIC_API
    return agnus->getStats();
}

const AgnusTraits
AgnusAPI::getTraits() const
{
    PUBLIC_API
    return agnus->getTraits();
}


//
// Components (Blitter)
//

const BlitterInfo &
BlitterAPI::getInfo() const
{
    PUBLIC_API
    return blitter->getInfo();
}

const BlitterInfo &
BlitterAPI::getCachedInfo() const
{
    PUBLIC_API
    return blitter->getCachedInfo();
}


//
// Components (CIA)
//

const CIAConfig &
CIAAPI::getConfig() const
{
    PUBLIC_API
    return cia->getConfig();
}

const CIAInfo &
CIAAPI::getInfo() const
{
    PUBLIC_API
    return cia->getInfo();
}

const CIAInfo &
CIAAPI::getCachedInfo() const
{
    PUBLIC_API
    return cia->getCachedInfo();
}

CIAStats
CIAAPI::getStats() const
{
    PUBLIC_API
    return cia->getStats();
}


//
// Components (Copper)
//

const CopperInfo &
CopperAPI::getInfo() const
{
    PUBLIC_API
    return copper->getInfo();
}

const CopperInfo &
CopperAPI::getCachedInfo() const
{
    PUBLIC_API
    return copper->getCachedInfo();
}

string 
CopperAPI::disassemble(isize list, isize offset, bool symbolic) const
{
    PUBLIC_API SUSPENDED
    return copper->debugger.disassemble(list, offset, symbolic);
}

string
CopperAPI::disassemble(u32 addr, bool symbolic) const
{
    PUBLIC_API SUSPENDED
    return copper->debugger.disassemble(addr, symbolic);
}

bool 
CopperAPI::isIllegalInstr(u32 addr) const
{
    PUBLIC_API SUSPENDED
    return copper->isIllegalInstr(addr);
}


//
// Components (CPU)
//

isize 
GuardsAPI::elements() const
{
    PUBLIC_API SUSPENDED
    return guards->elements();
}

std::optional<GuardInfo>
GuardsAPI::guardNr(long nr) const
{
    PUBLIC_API SUSPENDED
    return guards->guardNr(nr);
}

std::optional<GuardInfo>
GuardsAPI::guardAt(u32 target) const
{
    PUBLIC_API SUSPENDED
    return guards->guardAt(target);
}

void
GuardsAPI::setAt(u32 target, isize ignores)
{
    PUBLIC_API SUSPENDED
    emu->put(Cmd(CMD_GUARD_SET_AT, (void *)guards, target, ignores));
}

void 
GuardsAPI::moveTo(isize nr, u32 newTarget)
{
    PUBLIC_API SUSPENDED
    emu->put(Cmd(CMD_GUARD_MOVE_NR, (void *)guards, nr, newTarget));
}

void 
GuardsAPI::remove(isize nr)
{
    PUBLIC_API SUSPENDED
    emu->put(Cmd(CMD_GUARD_REMOVE_NR, (void *)guards, nr));
}

void 
GuardsAPI::removeAt(u32 target)
{
    PUBLIC_API SUSPENDED
    emu->put(Cmd(CMD_GUARD_REMOVE_AT, (void *)guards, target));
}

void 
GuardsAPI::removeAll()
{
    PUBLIC_API SUSPENDED
    emu->put(Cmd(CMD_GUARD_REMOVE_ALL, (void *)guards));
}

void 
GuardsAPI::enable(isize nr)
{
    PUBLIC_API SUSPENDED
    emu->put(Cmd(CMD_GUARD_ENABLE_NR, (void *)guards, nr));
}

void 
GuardsAPI::enableAt(u32 target)
{
    PUBLIC_API SUSPENDED
    emu->put(Cmd(CMD_GUARD_ENABLE_AT, (void *)guards, target));
}

void 
GuardsAPI::enableAll()
{
    PUBLIC_API SUSPENDED
    emu->put(Cmd(CMD_GUARD_ENABLE_ALL, (void *)guards));
}

void 
GuardsAPI::disable(isize nr)
{
    PUBLIC_API SUSPENDED
    emu->put(Cmd(CMD_GUARD_DISABLE_NR, (void *)guards, nr));
}

void 
GuardsAPI::disableAt(u32 target)
{
    PUBLIC_API SUSPENDED
    emu->put(Cmd(CMD_GUARD_DISABLE_AT, (void *)guards, target));
}

void 
GuardsAPI::disableAll()
{
    PUBLIC_API SUSPENDED
    emu->put(Cmd(CMD_GUARD_DISABLE_ALL));
}

void 
GuardsAPI::toggle(isize nr)
{
    PUBLIC_API SUSPENDED
    guards->toggle(nr);
}

isize
CPUDebuggerAPI::loggedInstructions() const
{
    PUBLIC_API SUSPENDED
    return cpu->debugger.loggedInstructions();
}

void
CPUDebuggerAPI::clearLog()
{
    PUBLIC_API SUSPENDED
    return cpu->debugger.clearLog();
}

const char *
CPUDebuggerAPI::disassembleRecordedInstr(isize i, isize *len)
{
    PUBLIC_API SUSPENDED
    return cpu->disassembleRecordedInstr(i, len);
}

const char *
CPUDebuggerAPI::disassembleRecordedWords(isize i, isize len)
{
    PUBLIC_API SUSPENDED
    return cpu->disassembleRecordedWords(i, len);
}

const char *
CPUDebuggerAPI::disassembleRecordedFlags(isize i)
{
    PUBLIC_API SUSPENDED
    return cpu->disassembleRecordedFlags(i);
}

const char *
CPUDebuggerAPI::disassembleRecordedPC(isize i)
{
    PUBLIC_API SUSPENDED
    return cpu->disassembleRecordedPC(i);
}

const char *
CPUDebuggerAPI::disassembleWord(u16 value)
{
    PUBLIC_API SUSPENDED
    return cpu->disassembleWord(value);
}

const char *
CPUDebuggerAPI::disassembleAddr(u32 addr)
{
    PUBLIC_API SUSPENDED
    return cpu->disassembleAddr(addr);
}

const char *
CPUDebuggerAPI::disassembleInstr(u32 addr, isize *len)
{
    PUBLIC_API SUSPENDED
    return cpu->disassembleInstr(addr, len);
}

const char *
CPUDebuggerAPI::disassembleWords(u32 addr, isize len)
{
    PUBLIC_API SUSPENDED
    return cpu->disassembleWords(addr, len);
}

string
CPUDebuggerAPI::vectorName(isize i)
{
    PUBLIC_API SUSPENDED
    return cpu->debugger.vectorName(u8(i));
}

const CPUConfig &
CPUAPI::getConfig() const
{
    PUBLIC_API
    return cpu->getConfig();
}

const CPUInfo &
CPUAPI::getInfo() const
{
    PUBLIC_API
    return cpu->getInfo();
}

const CPUInfo &
CPUAPI::getCachedInfo() const
{
    PUBLIC_API
    return cpu->getCachedInfo();
}


//
// Components (Denise)
//

const DeniseConfig &
DeniseAPI::getConfig() const
{
    PUBLIC_API
    return denise->getConfig();
}

const DeniseInfo &
DeniseAPI::getInfo() const
{
    PUBLIC_API
    return denise->getInfo();
}

const DeniseInfo &
DeniseAPI::getCachedInfo() const
{
    PUBLIC_API
    return denise->getCachedInfo();
}


//
// Components (Memory)
//

MemorySource 
MemoryDebuggerAPI::getMemSrc(Accessor acc, u32 addr) const
{
    PUBLIC_API
    
    switch (acc) {

        case ACCESSOR_CPU:      return mem->getMemSrc<ACCESSOR_CPU>(addr);
        case ACCESSOR_AGNUS:    return mem->getMemSrc<ACCESSOR_AGNUS>(addr);

        default:
            fatalError;
    }
}

u8
MemoryDebuggerAPI::spypeek8(Accessor acc, u32 addr) const
{
    PUBLIC_API
    
    switch (acc) {

        case ACCESSOR_CPU:      return mem->spypeek8<ACCESSOR_CPU>(addr);
        case ACCESSOR_AGNUS:    return mem->spypeek8<ACCESSOR_AGNUS>(addr);

        default:
            fatalError;
    }
}

u16 
MemoryDebuggerAPI::spypeek16(Accessor acc, u32 addr) const
{
    PUBLIC_API
    
    switch (acc) {

        case ACCESSOR_CPU:      return mem->spypeek16<ACCESSOR_CPU>(addr);
        case ACCESSOR_AGNUS:    return mem->spypeek16<ACCESSOR_AGNUS>(addr);

        default:
            fatalError;
    }
}

string
MemoryDebuggerAPI::ascDump(Accessor acc, u32 addr, isize bytes) const
{
    PUBLIC_API
    
    switch (acc) {

        case ACCESSOR_CPU:      return mem->debugger.ascDump<ACCESSOR_CPU>(addr, bytes);
        case ACCESSOR_AGNUS:    return mem->debugger.ascDump<ACCESSOR_AGNUS>(addr, bytes);

        default:
            fatalError;
    }
}

string
MemoryDebuggerAPI::hexDump(Accessor acc, u32 addr, isize bytes, isize sz) const
{
    PUBLIC_API
    
    switch (acc) {

        case ACCESSOR_CPU:      return mem->debugger.hexDump<ACCESSOR_CPU>(addr, bytes, sz);
        case ACCESSOR_AGNUS:    return mem->debugger.hexDump<ACCESSOR_AGNUS>(addr, bytes, sz);

        default:
            fatalError;
    }
}

string
MemoryDebuggerAPI::memDump(Accessor acc, u32 addr, isize bytes, isize sz) const
{
    PUBLIC_API
    
    switch (acc) {

        case ACCESSOR_CPU:      return mem->debugger.memDump<ACCESSOR_CPU>(addr, bytes, sz);
        case ACCESSOR_AGNUS:    return mem->debugger.memDump<ACCESSOR_AGNUS>(addr, bytes, sz);

        default:
            fatalError;
    }
}

const MemConfig &
MemoryAPI::getConfig() const
{
    PUBLIC_API
    return mem->getConfig();
}

const MemInfo &
MemoryAPI::getInfo() const
{
    PUBLIC_API
    return mem->getInfo();
}

const MemInfo &
MemoryAPI::getCachedInfo() const
{
    PUBLIC_API
    return mem->getCachedInfo();
}

const MemStats &
MemoryAPI::getStats() const
{
    PUBLIC_API
    return mem->getStats();
}

const RomTraits &
MemoryAPI::getRomTraits() const
{
    PUBLIC_API
    return mem->getRomTraits();
}

const RomTraits &
MemoryAPI::getWomTraits() const
{
    PUBLIC_API
    return mem->getWomTraits();
}

const RomTraits &
MemoryAPI::getExtTraits() const
{
    PUBLIC_API
    return mem->getExtTraits();
}

void 
MemoryAPI::loadRom(const fs::path &path)
{
    PUBLIC_API SUSPENDED
    mem->loadRom(path);
    emu->isDirty = true;
}

void
MemoryAPI::loadExt(const fs::path &path)
{
    PUBLIC_API SUSPENDED
    mem->loadExt(path);
    emu->isDirty = true;
}

void
MemoryAPI::loadRom(MediaFile &file)
{
    PUBLIC_API SUSPENDED
    mem->loadRom(file);
    emu->isDirty = true;
}

void
MemoryAPI::loadExt(MediaFile &file)
{
    PUBLIC_API SUSPENDED
    mem->loadExt(file);
    emu->isDirty = true;
}

void
MemoryAPI::loadRom(const u8 *buf, isize len)
{
    PUBLIC_API SUSPENDED
    mem->loadRom(buf, len);
    emu->isDirty = true;
}

void
MemoryAPI::loadExt(const u8 *buf, isize len)
{
    PUBLIC_API SUSPENDED
    mem->loadExt(buf, len);
    emu->isDirty = true;
}

void 
MemoryAPI::saveRom(const std::filesystem::path &path)
{
    PUBLIC_API SUSPENDED
    mem->saveRom(path);
}

void 
MemoryAPI::saveWom(const std::filesystem::path &path)
{
    PUBLIC_API SUSPENDED
    mem->saveWom(path);
}

void 
MemoryAPI::saveExt(const std::filesystem::path &path)
{
    PUBLIC_API SUSPENDED
    mem->saveExt(path);
}

void
MemoryAPI::deleteRom()
{
    PUBLIC_API SUSPENDED
    mem->deleteRom();
    emu->isDirty = true;
}

void 
MemoryAPI::deleteWom()
{
    PUBLIC_API  SUSPENDED
    mem->deleteWom();
    emu->isDirty = true;
}

void 
MemoryAPI::deleteExt()
{
    PUBLIC_API SUSPENDED
    mem->deleteExt();
    emu->isDirty = true;
}


//
// Components (Paula)
//

const StateMachineInfo &
AudioChannelAPI::getInfo() const
{
    PUBLIC_API
    
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
    PUBLIC_API
    
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
    PUBLIC_API
    return diskController->getConfig();
}

const DiskControllerInfo &
DiskControllerAPI::getInfo() const
{
    PUBLIC_API
    return diskController->getInfo();
}

const DiskControllerInfo &
DiskControllerAPI::getCachedInfo() const
{
    PUBLIC_API
    return diskController->getCachedInfo();
}

const UARTInfo &
UARTAPI::getInfo() const
{
    PUBLIC_API
    return uart->getInfo();
}

const UARTInfo &
UARTAPI::getCachedInfo() const
{
    PUBLIC_API
    return uart->getCachedInfo();
}

const PaulaInfo &
PaulaAPI::getInfo() const
{
    PUBLIC_API
    return paula->getInfo();
}

const PaulaInfo &
PaulaAPI::getCachedInfo() const
{
    PUBLIC_API
    return paula->getCachedInfo();
}


//
// Components (RTC)
//

const RTCConfig &
RTCAPI::getConfig() const
{
    PUBLIC_API
    return rtc->getConfig();
}

void
RTCAPI::update()
{
    PUBLIC_API SUSPENDED
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
    PUBLIC_API
    return port->getConfig();
}

const AudioPortStats &
AudioPortAPI::getStats() const
{
    PUBLIC_API
    return port->getStats();
}

isize
AudioPortAPI::copyMono(float *buffer, isize n)
{
    PUBLIC_API
    return port->copyMono(buffer, n);
}

isize
AudioPortAPI::copyStereo(float *left, float *right, isize n)
{
    PUBLIC_API
    return port->copyStereo(left, right, n);
}

isize
AudioPortAPI::copyInterleaved(float *buffer, isize n)
{
    PUBLIC_API
    return port->copyInterleaved(buffer, n);
}

void 
AudioPortAPI::drawL(u32 *buffer, isize width, isize height, u32 color) const
{
    PUBLIC_API
    port->stream.drawL(buffer, width, height, color);
}

void
AudioPortAPI::drawR(u32 *buffer, isize width, isize height, u32 color) const
{
    PUBLIC_API
    port->stream.drawR(buffer, width, height, color);
}


//
// Ports (ControlPort)
//

const ControlPortInfo &
ControlPortAPI::getInfo() const
{
    PUBLIC_API
    return controlPort->getInfo();
}

const ControlPortInfo &
ControlPortAPI::getCachedInfo() const
{
    PUBLIC_API
    return controlPort->getCachedInfo();
}


//
// Ports (SerialPort)
//

const SerialPortConfig &
SerialPortAPI::getConfig() const
{
    PUBLIC_API
    return serialPort->getConfig();
}

const SerialPortInfo &
SerialPortAPI::getInfo() const
{
    PUBLIC_API
    return serialPort->getInfo();
}

const SerialPortInfo &
SerialPortAPI::getCachedInfo() const
{
    PUBLIC_API
    return serialPort->getCachedInfo();
}

int 
SerialPortAPI::readIncomingPrintableByte() const
{
    PUBLIC_API SYNCHRONIZE(serialPort)
    return serialPort->readIncomingPrintableByte();
}

int 
SerialPortAPI::readOutgoingPrintableByte() const
{
    PUBLIC_API SYNCHRONIZE(serialPort)
    return serialPort->readOutgoingPrintableByte();
}


//
// Ports (VideoPort)
//

void
VideoPortAPI::lockTexture()
{
    PUBLIC_API
    emu->lockTexture();
}

void
VideoPortAPI::unlockTexture()
{
    PUBLIC_API
    emu->unlockTexture();
}

const u32 *
VideoPortAPI::getTexture() const
{
    PUBLIC_API
    return (u32 *)emu->getTexture().pixels.ptr;
}

const u32 *
VideoPortAPI::getTexture(isize *nr, bool *lof, bool *prevlof) const
{
    PUBLIC_API
    
    auto &frameBuffer = emu->getTexture();

    *nr = isize(frameBuffer.nr);
    *lof = frameBuffer.lof;
    *prevlof = frameBuffer.prevlof;

    return (u32 *)frameBuffer.pixels.ptr;
}


//
// Peripherals
//

//
// Peripherals (Keyboard)
//

bool
KeyboardAPI::isPressed(KeyCode key) const
{
    PUBLIC_API
    return keyboard->isPressed(key);
}

void
KeyboardAPI::press(KeyCode key, double delay, double duration)
{
    PUBLIC_API
    if (delay == 0.0) {

        keyboard->press(key);
        emu->isDirty = true;

    } else {
        
        emu->put(Cmd(CMD_KEY_PRESS, KeyCmd { .keycode = key, .delay = delay }));
    }
    if (duration != 0.0) {
        
        emu->put(Cmd(CMD_KEY_RELEASE, KeyCmd { .keycode = key, .delay = delay + duration }));
    }
}

void
KeyboardAPI::toggle(KeyCode key, double delay, double duration)
{
    PUBLIC_API
    if (delay == 0.0) {
        
        keyboard->toggle(key);
        emu->isDirty = true;
        
    } else {
        
        emu->put(Cmd(CMD_KEY_TOGGLE, KeyCmd { .keycode = key, .delay = delay }));
    }
    if (duration != 0.0) {
        
        emu->put(Cmd(CMD_KEY_TOGGLE, KeyCmd { .keycode = key, .delay = delay + duration }));
    }
}

void
KeyboardAPI::release(KeyCode key, double delay)
{
    PUBLIC_API
    if (delay == 0.0) {
        
        keyboard->release(key);
        emu->isDirty = true;
        
    } else {
        
        emu->put(Cmd(CMD_KEY_RELEASE, KeyCmd { .keycode = key, .delay = delay }));
    }
}

void
KeyboardAPI::releaseAll()
{
    PUBLIC_API
    emu->put(Cmd(CMD_KEY_RELEASE_ALL));
}

/*
void KeyboardAPI::autoType(const string &text)
{
    keyboard->autoType(text);
}
*/

void KeyboardAPI::abortAutoTyping()
{
    PUBLIC_API
    keyboard->abortAutoTyping();
}


//
// Peripherals (FloppyDrive)
//

const FloppyDriveConfig &
FloppyDriveAPI::getConfig() const
{
    PUBLIC_API
    return drive->getConfig();
}

const FloppyDriveInfo &
FloppyDriveAPI::getInfo() const
{
    PUBLIC_API
    return drive->getInfo();
}

const FloppyDriveInfo &
FloppyDriveAPI::getCachedInfo() const
{
    PUBLIC_API
    return drive->getCachedInfo();
}

FloppyDisk &
FloppyDriveAPI::getDisk()
{
    PUBLIC_API SUSPENDED
    return *(drive->disk);
}

bool
FloppyDriveAPI::getFlag(DiskFlags mask) const
{
    PUBLIC_API SUSPENDED
    return drive->getFlag(mask);
}

void
FloppyDriveAPI::setFlag(DiskFlags mask, bool value)
{
    PUBLIC_API SUSPENDED
    drive->setFlag(mask, value);
    emu->isDirty = true;
}

bool 
FloppyDriveAPI::isInsertable(Diameter t, Density d) const
{
    PUBLIC_API SUSPENDED
    return drive->isInsertable(t, d);
}

void
FloppyDriveAPI::insertBlankDisk(FSVolumeType fstype, BootBlockId bb, string name)
{
    PUBLIC_API SUSPENDED
    drive->insertNew(fstype, bb, name);
    emu->isDirty = true;
}

void
FloppyDriveAPI::insertMedia(MediaFile &file, bool wp)
{
    PUBLIC_API SUSPENDED
    drive->insertMediaFile(file, wp);
    emu->isDirty = true;
}

void
FloppyDriveAPI::ejectDisk()
{
    PUBLIC_API SUSPENDED
    drive->ejectDisk();
}

class MediaFile *
FloppyDriveAPI::exportDisk(FileType type)
{
    PUBLIC_API SUSPENDED
    return drive->exportDisk(type);
}

string
FloppyDriveAPI::readTrackBits(isize track)
{
    PUBLIC_API SUSPENDED
    return drive->readTrackBits(track);
}


//
// Peripherals (HardDrive)
//

class HardDrive &
HardDriveAPI::getDrive()
{
    PUBLIC_API
    return *drive;
}

const HardDriveConfig &
HardDriveAPI::getConfig() const
{
    PUBLIC_API
    return drive->getConfig();
}

const HardDriveInfo &
HardDriveAPI::getInfo() const
{
    PUBLIC_API
    return drive->getInfo();
}

const HardDriveInfo &
HardDriveAPI::getCachedInfo() const
{
    PUBLIC_API
    return drive->getCachedInfo();
}

const HardDriveTraits &
HardDriveAPI::getTraits() const
{
    PUBLIC_API
    return drive->getTraits();
}

const PartitionTraits &
HardDriveAPI::getPartitionTraits(isize nr) const
{
    PUBLIC_API
    return drive->getPartitionTraits(nr);
}

bool
HardDriveAPI::getFlag(DiskFlags mask)
{
    PUBLIC_API SUSPENDED
    return drive->getFlag(mask);
}

void
HardDriveAPI::setFlag(DiskFlags mask, bool value)
{
    PUBLIC_API SUSPENDED
    drive->setFlag(mask, value);
}

std::vector<std::tuple<isize,isize,isize>>
HardDriveAPI::geometries(isize numBlocks)
{
    PUBLIC_API SUSPENDED
    return GeometryDescriptor::driveGeometries(numBlocks);
}

void 
HardDriveAPI::changeGeometry(isize c, isize h, isize s, isize b)
{
    PUBLIC_API SUSPENDED
    return drive->changeGeometry(c, h, s, b);
}

void
HardDriveAPI::attach(const std::filesystem::path &path)
{
    PUBLIC_API SUSPENDED
    drive->init(path.string());
}

void 
HardDriveAPI::attach(const MediaFile &file)
{
    PUBLIC_API SUSPENDED
    drive->init(file);
}

void
HardDriveAPI::attach(isize c, isize h, isize s, isize b)
{
    PUBLIC_API SUSPENDED
    auto geometry = GeometryDescriptor(c, h, s, b);
    drive->init(geometry);
}

void 
HardDriveAPI::format(FSVolumeType fs, const string &name)
{
    PUBLIC_API SUSPENDED
    drive->format(fs, name);
}

void 
HardDriveAPI::writeToFile(std::filesystem::path path)
{
    PUBLIC_API SUSPENDED
    drive->writeToFile(path);
}

MediaFile *
HardDriveAPI::createHDF()
{
    PUBLIC_API SUSPENDED
    return new HDFFile(*drive);
}


//
// Peripherals (HdController)
//

const HdcInfo &
HdControllerAPI::getInfo() const
{
    PUBLIC_API
    return controller->getInfo();
}

const HdcInfo &
HdControllerAPI::getCachedInfo() const
{
    PUBLIC_API
    return controller->getCachedInfo();
}

const HdcStats &
HdControllerAPI::getStats() const
{
    PUBLIC_API
    return controller->getStats();
}


//
// Peripherals (Joystick)
//

const JoystickInfo &
JoystickAPI::getInfo() const
{
    PUBLIC_API
    return joystick->getInfo();
}

const JoystickInfo &
JoystickAPI::getCachedInfo() const
{
    PUBLIC_API
    return joystick->getCachedInfo();
}

void 
JoystickAPI::trigger(GamePadAction event)
{
    PUBLIC_API
    emu->put(CMD_JOY_EVENT, GamePadCmd { .port = joystick->objid, .action = event });
}


//
// Mouse
//

bool 
MouseAPI::detectShakeXY(double x, double y)
{
    PUBLIC_API
    return mouse->detectShakeXY(x, y);
}

bool 
MouseAPI::detectShakeDxDy(double dx, double dy)
{
    PUBLIC_API
    return mouse->detectShakeDxDy(dx, dy);
}

void 
MouseAPI::setXY(double x, double y)
{
    PUBLIC_API
    emu->put(Cmd(CMD_MOUSE_MOVE_ABS, CoordCmd { .port = mouse->objid, .x = x, .y = y }));
}

void 
MouseAPI::setDxDy(double dx, double dy)
{
    PUBLIC_API
    emu->put(Cmd(CMD_MOUSE_MOVE_REL, CoordCmd { .port = mouse->objid, .x = dx, .y = dy }));
}

void 
MouseAPI::trigger(GamePadAction action)
{
    PUBLIC_API
    emu->put(Cmd(CMD_MOUSE_EVENT, GamePadCmd { .port = mouse->objid, .action = action }));
}


//
// DefaultsAPI
//

void
DefaultsAPI::load(const fs::path &path)
{
    PUBLIC_API
    defaults->load(path);
}

void
DefaultsAPI::load(std::ifstream &stream)
{
    PUBLIC_API
    defaults->load(stream);
}

void
DefaultsAPI::load(std::stringstream &stream)
{
    PUBLIC_API
    defaults->load(stream);
}

void
DefaultsAPI::save(const fs::path &path)
{
    PUBLIC_API
    defaults->save(path);
}

void
DefaultsAPI::save(std::ofstream &stream)
{
    PUBLIC_API
    defaults->save(stream);
}

void
DefaultsAPI::save(std::stringstream &stream)
{
    PUBLIC_API
    defaults->save(stream);
}

string
DefaultsAPI::getRaw(const string &key) const
{
    PUBLIC_API
    return defaults->getRaw(key);
}

i64
DefaultsAPI::get(const string &key) const
{
    PUBLIC_API
    return defaults->get(key);
}

i64
DefaultsAPI::get(Option option, isize nr) const
{
    PUBLIC_API
    return defaults->get(option, nr);
}

string
DefaultsAPI::getFallbackRaw(const string &key) const
{
    PUBLIC_API
    return defaults->getFallbackRaw(key);
}

i64
DefaultsAPI::getFallback(const string &key) const
{
    PUBLIC_API
    return defaults->getFallback(key);
}

i64
DefaultsAPI::getFallback(Option option, isize nr) const
{
    PUBLIC_API
    return defaults->getFallback(option, nr);
}

void
DefaultsAPI::set(const string &key, const string &value)
{
    PUBLIC_API
    defaults->set(key, value);
}

void
DefaultsAPI::set(Option opt, const string &value)
{
    PUBLIC_API
    defaults->set(opt, value);
}

void
DefaultsAPI::set(Option opt, const string &value, std::vector<isize> objids)
{
    PUBLIC_API
    defaults->set(opt, value, objids);
}

void
DefaultsAPI::set(Option opt, i64 value)
{
    PUBLIC_API
    defaults->set(opt, value);
}

void
DefaultsAPI::set(Option opt, i64 value, std::vector<isize> objids)
{
    PUBLIC_API
    defaults->set(opt, value, objids);
}

void
DefaultsAPI::setFallback(const string &key, const string &value)
{
    PUBLIC_API
    defaults->setFallback(key, value);
}

void
DefaultsAPI::setFallback(Option opt, const string &value)
{
    PUBLIC_API
    defaults->setFallback(opt, value);
}

void
DefaultsAPI::setFallback(Option opt, const string &value, std::vector<isize> objids)
{
    PUBLIC_API
    defaults->setFallback(opt, value, objids);
}

void
DefaultsAPI::setFallback(Option opt, i64 value)
{
    PUBLIC_API
    defaults->setFallback(opt, value);
}

void
DefaultsAPI::setFallback(Option opt, i64 value, std::vector<isize> objids)
{
    PUBLIC_API
    defaults->setFallback(opt, value, objids);
}

void
DefaultsAPI::remove()
{
    PUBLIC_API
    defaults->remove();
}

void
DefaultsAPI::remove(const string &key)
{
    PUBLIC_API
    defaults->remove(key);
}

void
DefaultsAPI::remove(Option option)
{
    PUBLIC_API
    defaults->remove(option);
}

void
DefaultsAPI::remove(Option option, std::vector <isize> objids)
{
    PUBLIC_API
    defaults->remove(option, objids);
}


//
// RecorderAPI
//

/*
const RecorderConfig &
RecorderAPI::getConfig() const
{
    PUBLIC_API
    return recorder->getConfig();
}

const RecorderInfo &
RecorderAPI::getInfo() const
{
    PUBLIC_API
    return recorder->getInfo();
}

const RecorderInfo &
RecorderAPI::getCachedInfo() const
{
    PUBLIC_API
    return recorder->getCachedInfo();
}
*/

double RecorderAPI::getDuration() const { PUBLIC_API SUSPENDED return recorder->getDuration().asSeconds(); }
isize RecorderAPI::getFrameRate() const { PUBLIC_API SUSPENDED return recorder->getFrameRate(); }
isize RecorderAPI::getBitRate() const { PUBLIC_API SUSPENDED return recorder->getBitRate(); }
isize RecorderAPI::getSampleRate() const { PUBLIC_API SUSPENDED return recorder->getSampleRate(); }
bool RecorderAPI::isRecording() const { PUBLIC_API SUSPENDED return recorder->isRecording(); }

const std::vector<std::filesystem::path> &
RecorderAPI::paths() const
{
    PUBLIC_API
    return FFmpeg::paths;
}

bool 
RecorderAPI::hasFFmpeg() const
{
    PUBLIC_API
    return FFmpeg::available();
}

const fs::path
RecorderAPI::getExecPath() const
{
    PUBLIC_API
    return FFmpeg::getExecPath();
}

void RecorderAPI::setExecPath(const std::filesystem::path &path)
{
    PUBLIC_API
    FFmpeg::setExecPath(path);
}

void
RecorderAPI::startRecording(isize x1, isize y1, isize x2, isize y2,
                            isize bitRate,
                            isize aspectX, isize aspectY)
{
    PUBLIC_API SUSPENDED
    recorder->startRecording(x1, y1, x2, y2, bitRate, aspectX, aspectY);
}

void
RecorderAPI::stopRecording()
{
    PUBLIC_API SUSPENDED
    recorder->stopRecording();
}

bool
RecorderAPI::exportAs(const std::filesystem::path &path)
{
    PUBLIC_API SUSPENDED
    return recorder->exportAs(path);
}


//
// RemoteManagerAPI
//

const RemoteManagerInfo &
RemoteManagerAPI::getInfo() const
{
    PUBLIC_API
    return remoteManager->getInfo();
}

const RemoteManagerInfo &
RemoteManagerAPI::getCachedInfo() const
{
    PUBLIC_API
    return remoteManager->getCachedInfo();
}


// RetroShellAPI
//

const char *
RetroShellAPI::text()
{
    PUBLIC_API SUSPENDED
    return retroShell->text();
}

isize
RetroShellAPI::cursorRel()
{
    PUBLIC_API SUSPENDED
    return retroShell->cursorRel();
}

void
RetroShellAPI::press(RetroShellKey key, bool shift)
{
    PUBLIC_API SUSPENDED
    retroShell->press(key, shift);
}

void
RetroShellAPI::press(char c)
{
    PUBLIC_API SUSPENDED
    retroShell->press(c);
}

void
RetroShellAPI::press(const string &s)
{
    PUBLIC_API SUSPENDED
    retroShell->press(s);
}

void
RetroShellAPI::execScript(std::stringstream &ss)
{
    PUBLIC_API SUSPENDED
    retroShell->asyncExecScript(ss);
}

void
RetroShellAPI::execScript(const std::ifstream &fs)
{
    PUBLIC_API SUSPENDED
    retroShell->asyncExecScript(fs);
}

void
RetroShellAPI::execScript(const string &contents)
{
    PUBLIC_API SUSPENDED
    retroShell->asyncExecScript(contents);
}

void
RetroShellAPI::execScript(const MediaFile &file)
{
    PUBLIC_API SUSPENDED
    retroShell->asyncExecScript(file);
}

void
RetroShellAPI::setStream(std::ostream &os)
{
    PUBLIC_API SUSPENDED
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
    PUBLIC_API
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
    PUBLIC_API
    return emu->getInfo();
}

const EmulatorInfo &
VAmiga::getCachedInfo() const
{
    PUBLIC_API
    return emu->getCachedInfo();
}

const EmulatorStats &
VAmiga::getStats() const
{
    PUBLIC_API
    return emu->getStats();
}

bool
VAmiga::isPoweredOn() const
{
    PUBLIC_API
    return emu->main.isPoweredOn();
}

bool
VAmiga::isPoweredOff() const
{
    PUBLIC_API
    return emu->main.isPoweredOff();
}

bool
VAmiga::isPaused() const
{
    PUBLIC_API
    return emu->main.isPaused();
}

bool
VAmiga::isRunning() const
{
    PUBLIC_API
    return emu->main.isRunning();
}

bool
VAmiga::isSuspended() const
{
    PUBLIC_API
    return emu->isSuspended();
}

bool
VAmiga::isHalted() const
{
    PUBLIC_API
    return emu->main.isHalted();
}

bool
VAmiga::isWarping() const
{
    PUBLIC_API
    return emu->isWarping();
}

bool
VAmiga::isTracking() const
{
    PUBLIC_API
    return emu->isTracking();
}

void
VAmiga::isReady() const
{
    PUBLIC_API
    return emu->isReady();
}

void
VAmiga::powerOn()
{
    PUBLIC_API SUSPENDED
    
    emu->Thread::powerOn();
    emu->isDirty = true;
}

void
VAmiga::powerOff()
{
    PUBLIC_API SUSPENDED
    
    emu->Thread::powerOff();
    emu->isDirty = true;
}

void
VAmiga::run()
{
    PUBLIC_API SUSPENDED
    
    emu->run();
    emu->isDirty = true;
}

void
VAmiga::pause()
{
    PUBLIC_API SUSPENDED
    
    emu->pause();
    emu->isDirty = true;
}

void 
VAmiga::hardReset()
{
    PUBLIC_API SUSPENDED
    
    emu->hardReset();
    emu->isDirty = true;
}

void
VAmiga::softReset()
{
    PUBLIC_API SUSPENDED
    
    emu->softReset();
    emu->isDirty = true;
}

void
VAmiga::halt()
{
    PUBLIC_API
    
    {   SUSPENDED
        
        // Signal the emulator to halt
        emu->halt();
        
    }   // Leave suspended state
    
    // Wait for the thread to terminate
    emu->join();
}

void
VAmiga::suspend() const
{
    PUBLIC_API
    emu->suspend();
}

void
VAmiga::resume() const
{
    PUBLIC_API
    emu->resume();
}

void
VAmiga::warpOn(isize source)
{
    PUBLIC_API SUSPENDED
    emu->warpOn(source);
}

void
VAmiga::warpOff(isize source)
{
    PUBLIC_API SUSPENDED
    emu->warpOff(source);
}

void
VAmiga::trackOn(isize source)
{
    PUBLIC_API SUSPENDED
    emu->trackOn(source);
}

void
VAmiga::trackOff(isize source)
{
    PUBLIC_API SUSPENDED
    emu->trackOff(source);
}

void
VAmiga::stepInto()
{
    PUBLIC_API SUSPENDED
    emu->stepInto();
}

void
VAmiga::stepOver()
{
    PUBLIC_API SUSPENDED
    emu->stepOver();
}

void
VAmiga::finishLine()
{
    PUBLIC_API SUSPENDED
    emu->finishLine();
}

void
VAmiga::finishFrame()
{
    PUBLIC_API SUSPENDED
    emu->finishFrame();
}

void
VAmiga::wakeUp()
{
    PUBLIC_API
    emu->wakeUp();
}

void
VAmiga::launch(const void *listener, Callback *func)
{
    PUBLIC_API
    emu->launch(listener, func);
}

bool
VAmiga::isLaunched() const
{
    PUBLIC_API
    return emu->isLaunched();
}

i64
VAmiga::get(Option option) const
{
    PUBLIC_API
    return emu->get(option);
}

i64
VAmiga::get(Option option, long id) const
{
    PUBLIC_API
    return emu->get(option, id);
}

void
VAmiga::set(ConfigScheme model)
{
    PUBLIC_API SUSPENDED
    emu->set(model);
    emu->isDirty = true;
}

void
VAmiga::set(Option opt, i64 value) throws
{
    PUBLIC_API
    emu->check(opt, value);
    put(CMD_CONFIG_ALL, ConfigCmd { .option = opt, .value = value });
    emu->isDirty = true;
}

void
VAmiga::set(Option opt, i64 value, long id)
{
    PUBLIC_API
    emu->check(opt, value, { id });
    put(CMD_CONFIG, ConfigCmd { .option = opt, .value = value, .id = id });
    emu->isDirty = true;
}

void
VAmiga::exportConfig(const fs::path &path, bool diff) const
{
    PUBLIC_API SUSPENDED
    emu->main.exportConfig(path, diff);
}

void
VAmiga::exportConfig(std::ostream& stream, bool diff) const
{
    PUBLIC_API SUSPENDED
    emu->main.exportConfig(stream, diff);
}

void
VAmiga::put(const Cmd &cmd)
{
    PUBLIC_API
    emu->put(cmd);
}


//
// AmigaAPI
//

MediaFile *
AmigaAPI::takeSnapshot()
{
    PUBLIC_API SUSPENDED
    return amiga->takeSnapshot();
}

void 
AmigaAPI::loadSnapshot(const MediaFile &snapshot)
{
    PUBLIC_API SUSPENDED
    amiga->loadSnapshot(snapshot);
    emu->isDirty = true;
}
    
u64
AmigaAPI::getAutoInspectionMask() const
{
    PUBLIC_API SUSPENDED
    return amiga->getAutoInspectionMask();
}

void
AmigaAPI::setAutoInspectionMask(u64 mask)
{
    PUBLIC_API SUSPENDED
    amiga->setAutoInspectionMask(mask);
}

}
