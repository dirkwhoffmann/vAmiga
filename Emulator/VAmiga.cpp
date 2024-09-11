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
#include "Emulator.h"
#include "GuardList.h"

namespace vamiga {

DefaultsAPI VAmiga::defaults(&Emulator::defaults);

//
// API
//

void
API::suspend()
{
    emu->suspend();
}

void
API::resume()
{
    emu->resume();
}

bool
API::isUserThread() const
{
    return !emu->isEmulatorThread();
}


//
// Components (Amiga)
//

const AmigaConfig &
AmigaAPI::getConfig() const
{
    return amiga->getConfig();
}

const AmigaInfo &
AmigaAPI::getInfo() const
{
    return amiga->getInfo();
}

const AmigaInfo &
AmigaAPI::getCachedInfo() const
{
    return amiga->getCachedInfo();
}


//
// Components (Agnus)
//

const DmaDebuggerConfig &
DmaDebuggerAPI::getConfig() const
{
    return dmaDebugger->getConfig();
}

const DmaDebuggerInfo &
DmaDebuggerAPI::getInfo() const
{
    return dmaDebugger->getInfo();
}

const DmaDebuggerInfo &
DmaDebuggerAPI::getCachedInfo() const
{
    return dmaDebugger->getCachedInfo();
}

const AgnusConfig &
AgnusAPI::getConfig() const
{
    return agnus->getConfig();
}

const AgnusInfo &
AgnusAPI::getInfo() const
{
    return agnus->getInfo();
}

const AgnusInfo &
AgnusAPI::getCachedInfo() const
{
    return agnus->getCachedInfo();
}

const AgnusStats &
AgnusAPI::getStats() const
{
    return agnus->getStats();
}

const AgnusTraits
AgnusAPI::getTraits() const
{
    return agnus->getTraits();
}


//
// Components (Blitter)
//

const BlitterInfo &
BlitterAPI::getInfo() const
{
    return blitter->getInfo();
}

const BlitterInfo &
BlitterAPI::getCachedInfo() const
{
    return blitter->getCachedInfo();
}


//
// Components (CIA)
//

const CIAConfig &
CIAAPI::getConfig() const
{
    return cia->getConfig();
}

const CIAInfo &
CIAAPI::getInfo() const
{
    return cia->getInfo();
}

const CIAInfo &
CIAAPI::getCachedInfo() const
{
    return cia->getCachedInfo();
}

CIAStats
CIAAPI::getStats() const
{
    return cia->getStats();
}


//
// Components (Copper)
//

const CopperInfo &
CopperAPI::getInfo() const
{
    return copper->getInfo();
}

const CopperInfo &
CopperAPI::getCachedInfo() const
{
    return copper->getCachedInfo();
}

string 
CopperAPI::disassemble(isize list, isize offset, bool symbolic) const
{
    return copper->debugger.disassemble(list, offset, symbolic);
}

string
CopperAPI::disassemble(u32 addr, bool symbolic) const
{
    return copper->debugger.disassemble(addr, symbolic);
}

bool 
CopperAPI::isIllegalInstr(u32 addr) const
{
    return copper->isIllegalInstr(addr);
}


//
// Components (CPU)
//

isize 
GuardsAPI::elements() const
{
    return guards->elements();
}

std::optional<GuardInfo>
GuardsAPI::guardNr(long nr) const
{
    return guards->guardNr(nr);
}

std::optional<GuardInfo>
GuardsAPI::guardAt(u32 target) const
{
    return guards->guardAt(target);
}

void
GuardsAPI::setAt(u32 target, isize ignores)
{
    emu->put(Cmd(CMD_GUARD_SET_AT, (void *)guards, target, ignores));
}

void 
GuardsAPI::moveTo(isize nr, u32 newTarget)
{
    emu->put(Cmd(CMD_GUARD_MOVE_NR, (void *)guards, nr, newTarget));
}

void 
GuardsAPI::remove(isize nr)
{
    emu->put(Cmd(CMD_GUARD_REMOVE_NR, (void *)guards, nr));
}

void 
GuardsAPI::removeAt(u32 target)
{
    emu->put(Cmd(CMD_GUARD_REMOVE_AT, (void *)guards, target));
}

void 
GuardsAPI::removeAll()
{
    emu->put(Cmd(CMD_GUARD_REMOVE_ALL, (void *)guards));
}

void 
GuardsAPI::enable(isize nr)
{
    emu->put(Cmd(CMD_GUARD_ENABLE_NR, (void *)guards, nr));
}

void 
GuardsAPI::enableAt(u32 target)
{
    emu->put(Cmd(CMD_GUARD_ENABLE_AT, (void *)guards, target));
}

void 
GuardsAPI::enableAll()
{
    emu->put(Cmd(CMD_GUARD_ENABLE_ALL, (void *)guards));
}

void 
GuardsAPI::disable(isize nr)
{
    emu->put(Cmd(CMD_GUARD_DISABLE_NR, (void *)guards, nr));
}

void 
GuardsAPI::disableAt(u32 target)
{
    guards->disableAt(target);
}

void 
GuardsAPI::disableAll()
{
    guards->disableAll();
}

void 
GuardsAPI::toggle(isize nr)
{
    guards->toggle(nr);
}

isize
CPUDebuggerAPI::loggedInstructions() const
{
    return cpu->debugger.loggedInstructions();
}

void
CPUDebuggerAPI::clearLog()
{
    return cpu->debugger.clearLog();
}

const char *
CPUDebuggerAPI::disassembleRecordedInstr(isize i, isize *len)
{
    return cpu->disassembleRecordedInstr(i, len);
}

const char *
CPUDebuggerAPI::disassembleRecordedWords(isize i, isize len)
{
    return cpu->disassembleRecordedWords(i, len);
}

const char *
CPUDebuggerAPI::disassembleRecordedFlags(isize i)
{
    return cpu->disassembleRecordedFlags(i);
}

const char *
CPUDebuggerAPI::disassembleRecordedPC(isize i)
{
    return cpu->disassembleRecordedPC(i);
}

const char *
CPUDebuggerAPI::disassembleWord(u16 value)
{
    return cpu->disassembleWord(value);
}

const char *
CPUDebuggerAPI::disassembleAddr(u32 addr)
{
    return cpu->disassembleAddr(addr);
}

const char *
CPUDebuggerAPI::disassembleInstr(u32 addr, isize *len)
{
    return cpu->disassembleInstr(addr, len);
}

const char *
CPUDebuggerAPI::disassembleWords(u32 addr, isize len)
{
    return cpu->disassembleWords(addr, len);
}

string
CPUDebuggerAPI::vectorName(isize i)
{
    return cpu->debugger.vectorName(u8(i));
}

const CPUConfig &
CPUAPI::getConfig() const
{
    return cpu->getConfig();
}

const CPUInfo &
CPUAPI::getInfo() const
{
    return cpu->getInfo();
}

const CPUInfo &
CPUAPI::getCachedInfo() const
{
    return cpu->getCachedInfo();
}


//
// Components (Denise)
//

const DeniseConfig &
DeniseAPI::getConfig() const
{
    return denise->getConfig();
}

const DeniseInfo &
DeniseAPI::getInfo() const
{
    return denise->getInfo();
}

const DeniseInfo &
DeniseAPI::getCachedInfo() const
{
    return denise->getCachedInfo();
}


//
// Components (Memory)
//

MemorySource 
MemoryDebuggerAPI::getMemSrc(Accessor acc, u32 addr) const
{
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
    assert(isUserThread());

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
    assert(isUserThread());

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
    assert(isUserThread());

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
    assert(isUserThread());
    return mem->getConfig();
}

const MemInfo &
MemoryAPI::getInfo() const
{
    assert(isUserThread());
    return mem->getInfo();
}

const MemInfo &
MemoryAPI::getCachedInfo() const
{
    assert(isUserThread());
    return mem->getCachedInfo();
}

const MemStats &
MemoryAPI::getStats() const
{
    assert(isUserThread());
    return mem->getStats();
}

const RomTraits &
MemoryAPI::getRomTraits() const
{
    return mem->getRomTraits();
}

const RomTraits &
MemoryAPI::getWomTraits() const
{
    return mem->getWomTraits();
}

const RomTraits &
MemoryAPI::getExtTraits() const
{
    return mem->getExtTraits();
}

void 
MemoryAPI::loadRom(const fs::path &path)
{
    mem->loadRom(path);
    emu->isDirty = true;
}

void
MemoryAPI::loadExt(const fs::path &path)
{
    mem->loadExt(path);
    emu->isDirty = true;
}

void
MemoryAPI::loadRom(MediaFile &file)
{
    mem->loadRom(file);
    emu->isDirty = true;
}

void
MemoryAPI::loadExt(MediaFile &file)
{
    mem->loadExt(file);
    emu->isDirty = true;
}

void
MemoryAPI::loadRom(const u8 *buf, isize len)
{
    mem->loadRom(buf, len);
    emu->isDirty = true;
}

void
MemoryAPI::loadExt(const u8 *buf, isize len)
{
    mem->loadExt(buf, len);
    emu->isDirty = true;
}

void 
MemoryAPI::saveRom(const std::filesystem::path &path)
{
    mem->saveRom(path);
}

void 
MemoryAPI::saveWom(const std::filesystem::path &path)
{
    mem->saveWom(path);
}

void 
MemoryAPI::saveExt(const std::filesystem::path &path)
{
    mem->saveExt(path);
}

void
MemoryAPI::deleteRom()
{
    mem->deleteRom();
    emu->isDirty = true;
}

void 
MemoryAPI::deleteWom()
{
    mem->deleteWom();
    emu->isDirty = true;
}

void 
MemoryAPI::deleteExt()
{
    mem->deleteExt();
    emu->isDirty = true;
}


//
// Components (Paula)
//

const StateMachineInfo &
AudioChannelAPI::getInfo() const
{
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
    return diskController->getConfig();
}

const DiskControllerInfo &
DiskControllerAPI::getInfo() const
{
    return diskController->getInfo();
}

const DiskControllerInfo &
DiskControllerAPI::getCachedInfo() const
{
    return diskController->getCachedInfo();
}

const UARTInfo &
UARTAPI::getInfo() const
{
    return uart->getInfo();
}

const UARTInfo &
UARTAPI::getCachedInfo() const
{
    return uart->getCachedInfo();
}

const PaulaInfo &
PaulaAPI::getInfo() const
{
    return paula->getInfo();
}

const PaulaInfo &
PaulaAPI::getCachedInfo() const
{
    return paula->getCachedInfo();
}


//
// Components (RTC)
//

const RTCConfig &
RTCAPI::getConfig() const
{
    return rtc->getConfig();
}

void
RTCAPI::update()
{
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
    return port->getConfig();
}

const AudioPortStats &
AudioPortAPI::getStats() const
{
    return port->getStats();
}

isize
AudioPortAPI::copyMono(float *buffer, isize n)
{
    return port->copyMono(buffer, n);
}

isize
AudioPortAPI::copyStereo(float *left, float *right, isize n)
{
    return port->copyStereo(left, right, n);
}

isize
AudioPortAPI::copyInterleaved(float *buffer, isize n)
{
    return port->copyInterleaved(buffer, n);
}

void 
AudioPortAPI::drawL(u32 *buffer, isize width, isize height, u32 color) const
{
    port->stream.drawL(buffer, width, height, color);
}

void
AudioPortAPI::drawR(u32 *buffer, isize width, isize height, u32 color) const
{
    port->stream.drawR(buffer, width, height, color);
}


//
// Ports (ControlPort)
//

const ControlPortInfo &
ControlPortAPI::getInfo() const
{
    return controlPort->getInfo();
}

const ControlPortInfo &
ControlPortAPI::getCachedInfo() const
{
    return controlPort->getCachedInfo();
}


//
// Ports (SerialPort)
//

const SerialPortConfig &
SerialPortAPI::getConfig() const
{
    return serialPort->getConfig();
}

const SerialPortInfo &
SerialPortAPI::getInfo() const
{
    return serialPort->getInfo();
}

const SerialPortInfo &
SerialPortAPI::getCachedInfo() const
{
    return serialPort->getCachedInfo();
}

int 
SerialPortAPI::readIncomingPrintableByte() const
{
    return serialPort->readIncomingPrintableByte();
}

int 
SerialPortAPI::readOutgoingPrintableByte() const
{
    return serialPort->readOutgoingPrintableByte();
}


//
// Ports (VideoPort)
//

/*
const class FrameBuffer &
VideoPortAPI::getTexture() const
{
    return videoPort->getTexture();
}
*/

const u32 *
VideoPortAPI::getTexture() const
{
    return emu->getTexture().pixels.ptr;
}

const u32 *
VideoPortAPI::getTexture(isize *nr, bool *lof, bool *prevlof) const
{
    auto &frameBuffer = emu->getTexture();

    *nr = isize(frameBuffer.nr);
    *lof = frameBuffer.lof;
    *prevlof = frameBuffer.prevlof;

    return frameBuffer.pixels.ptr;
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
    return keyboard->isPressed(key);
}

void
KeyboardAPI::press(KeyCode key, double delay, double duration)
{
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
    keyboard->abortAutoTyping();
}


//
// Peripherals (FloppyDrive)
//

const FloppyDriveConfig &
FloppyDriveAPI::getConfig() const
{
    return drive->getConfig();
}

const FloppyDriveInfo &
FloppyDriveAPI::getInfo() const
{
    return drive->getInfo();
}

const FloppyDriveInfo &
FloppyDriveAPI::getCachedInfo() const
{
    return drive->getCachedInfo();
}

FloppyDisk &
FloppyDriveAPI::getDisk()
{
    return *(drive->disk);
}

bool
FloppyDriveAPI::getFlag(DiskFlags mask) const
{
    return drive->getFlag(mask);
}

void
FloppyDriveAPI::setFlag(DiskFlags mask, bool value)
{
    drive->setFlag(mask, value);
    emu->isDirty = true;
}

bool 
FloppyDriveAPI::isInsertable(Diameter t, Density d) const
{
    return drive->isInsertable(t, d);
}

void
FloppyDriveAPI::insertBlankDisk(FSVolumeType fstype, BootBlockId bb, string name)
{
    drive->insertNew(fstype, bb, name);
    emu->isDirty = true;
}

void
FloppyDriveAPI::insertMedia(MediaFile &file, bool wp)
{
    drive->insertMediaFile(file, wp);
    emu->isDirty = true;
}

/*
void
FloppyDriveAPI::insertFileSystem(const class MutableFileSystem &fs, bool wp);
{

    // NOT IMPLEMENTED YET
    assert(false);
    // drive->insertFileSystem(device, wp);
}
*/

void
FloppyDriveAPI::ejectDisk()
{
    drive->ejectDisk();
}

class MediaFile *
FloppyDriveAPI::exportDisk(FileType type)
{
    return drive->exportDisk(type);
}

string
FloppyDriveAPI::readTrackBits(isize track)
{
    return drive->readTrackBits(track);
}


//
// Peripherals (HardDrive)
//

class HardDrive &
HardDriveAPI::getDrive()
{
    return *drive;
}

const HardDriveConfig &
HardDriveAPI::getConfig() const
{
    return drive->getConfig();
}

const HardDriveInfo &
HardDriveAPI::getInfo() const
{
    return drive->getInfo();
}

const HardDriveInfo &
HardDriveAPI::getCachedInfo() const
{
    return drive->getCachedInfo();
}

const HardDriveTraits &
HardDriveAPI::getTraits() const
{
    return drive->getTraits();
}

const PartitionTraits &
HardDriveAPI::getPartitionTraits(isize nr) const
{
    return drive->getPartitionTraits(nr);
}

bool
HardDriveAPI::getFlag(DiskFlags mask)
{
    return drive->getFlag(mask);
}

void
HardDriveAPI::setFlag(DiskFlags mask, bool value)
{
    drive->setFlag(mask, value);
}

std::vector<std::tuple<isize,isize,isize>>
HardDriveAPI::geometries(isize numBlocks)
{
    return GeometryDescriptor::driveGeometries(numBlocks);
}

void 
HardDriveAPI::changeGeometry(isize c, isize h, isize s, isize b)
{
    return drive->changeGeometry(c, h, s, b);
}

void
HardDriveAPI::attach(const std::filesystem::path &path)
{
    drive->init(path.string());
}

void 
HardDriveAPI::attach(const MediaFile &file)
{
    drive->init(file);
}

void
HardDriveAPI::attach(isize c, isize h, isize s, isize b)
{
    auto geometry = GeometryDescriptor(c, h, s, b);
    drive->init(geometry);
}

void 
HardDriveAPI::format(FSVolumeType fs, const string &name)
{
    drive->format(fs, name);
}

void 
HardDriveAPI::writeToFile(std::filesystem::path path)
{
    drive->writeToFile(path);
}

MediaFile *
HardDriveAPI::createHDF()
{
    return new HDFFile(*drive);
}


//
// Peripherals (HdController)
//

const HdcInfo &
HdControllerAPI::getInfo() const
{
    return controller->getInfo();
}

const HdcInfo &
HdControllerAPI::getCachedInfo() const
{
    return controller->getCachedInfo();
}

const HdcStats &
HdControllerAPI::getStats() const
{
    return controller->getStats();
}


//
// Peripherals (Joystick)
//

const JoystickInfo &
JoystickAPI::getInfo() const
{
    return joystick->getInfo();
}

const JoystickInfo &
JoystickAPI::getCachedInfo() const
{
    return joystick->getCachedInfo();
}

void 
JoystickAPI::trigger(GamePadAction event)
{
    emu->put(CMD_JOY_EVENT, GamePadCmd { .port = joystick->objid, .action = event });
}


//
// Mouse
//

bool 
MouseAPI::detectShakeXY(double x, double y)
{
    return mouse->detectShakeXY(x, y);
}

bool 
MouseAPI::detectShakeDxDy(double dx, double dy)
{
    return mouse->detectShakeDxDy(dx, dy);
}

void 
MouseAPI::setXY(double x, double y)
{
    emu->put(Cmd(CMD_MOUSE_MOVE_ABS, CoordCmd { .port = mouse->objid, .x = x, .y = y }));
}

void 
MouseAPI::setDxDy(double dx, double dy)
{
    emu->put(Cmd(CMD_MOUSE_MOVE_REL, CoordCmd { .port = mouse->objid, .x = dx, .y = dy }));
}

void 
MouseAPI::trigger(GamePadAction action)
{
    emu->put(Cmd(CMD_MOUSE_EVENT, GamePadCmd { .port = mouse->objid, .action = action }));
}


//
// DefaultsAPI
//

void
DefaultsAPI::load(const fs::path &path)
{
    defaults->load(path);
}

void
DefaultsAPI::load(std::ifstream &stream)
{
    defaults->load(stream);
}

void
DefaultsAPI::load(std::stringstream &stream)
{
    defaults->load(stream);
}

void
DefaultsAPI::save(const fs::path &path)
{
    defaults->save(path);
}

void
DefaultsAPI::save(std::ofstream &stream)
{
    defaults->save(stream);
}

void
DefaultsAPI::save(std::stringstream &stream)
{
    defaults->save(stream);
}

string
DefaultsAPI::getRaw(const string &key) const
{
    return defaults->getRaw(key);
}

i64
DefaultsAPI::get(const string &key) const
{
    return defaults->get(key);
}

i64
DefaultsAPI::get(Option option, isize nr) const
{
    return defaults->get(option, nr);
}

string
DefaultsAPI::getFallbackRaw(const string &key) const
{
    return defaults->getFallbackRaw(key);
}

i64
DefaultsAPI::getFallback(const string &key) const
{
    return defaults->getFallback(key);
}

i64
DefaultsAPI::getFallback(Option option, isize nr) const
{
    return defaults->getFallback(option, nr);
}

void
DefaultsAPI::set(const string &key, const string &value)
{
    defaults->set(key, value);
}

void
DefaultsAPI::set(Option opt, const string &value)
{
    defaults->set(opt, value);
}

void
DefaultsAPI::set(Option opt, const string &value, std::vector<isize> objids)
{
    defaults->set(opt, value, objids);
}

void
DefaultsAPI::set(Option opt, i64 value)
{
    defaults->set(opt, value);
}

void
DefaultsAPI::set(Option opt, i64 value, std::vector<isize> objids)
{
    defaults->set(opt, value, objids);
}

void
DefaultsAPI::setFallback(const string &key, const string &value)
{
    defaults->setFallback(key, value);
}

void
DefaultsAPI::setFallback(Option opt, const string &value)
{
    defaults->setFallback(opt, value);
}

void
DefaultsAPI::setFallback(Option opt, const string &value, std::vector<isize> objids)
{
    defaults->setFallback(opt, value, objids);
}

void
DefaultsAPI::setFallback(Option opt, i64 value)
{
    defaults->setFallback(opt, value);
}

void
DefaultsAPI::setFallback(Option opt, i64 value, std::vector<isize> objids)
{
    defaults->setFallback(opt, value, objids);
}

void
DefaultsAPI::remove()
{
    defaults->remove();
}

void
DefaultsAPI::remove(const string &key)
{
    defaults->remove(key);
}

void
DefaultsAPI::remove(Option option)
{
    defaults->remove(option);
}

void
DefaultsAPI::remove(Option option, std::vector <isize> objids)
{
    defaults->remove(option, objids);
}


//
// RecorderAPI
//

/*
const RecorderConfig &
RecorderAPI::getConfig() const
{
    return recorder->getConfig();
}

const RecorderInfo &
RecorderAPI::getInfo() const
{
    return recorder->getInfo();
}

const RecorderInfo &
RecorderAPI::getCachedInfo() const
{
    return recorder->getCachedInfo();
}
*/

double RecorderAPI::getDuration() const { return recorder->getDuration().asSeconds(); }
isize RecorderAPI::getFrameRate() const { return recorder->getFrameRate(); }
isize RecorderAPI::getBitRate() const { return recorder->getBitRate(); }
isize RecorderAPI::getSampleRate() const { return recorder->getSampleRate(); }
bool RecorderAPI::isRecording() const { return recorder->isRecording(); }

const std::vector<std::filesystem::path> &
RecorderAPI::paths() const
{
    return FFmpeg::paths;
}

bool 
RecorderAPI::hasFFmpeg() const
{
    return FFmpeg::available();
}

const fs::path
RecorderAPI::getExecPath() const
{
    return FFmpeg::getExecPath();
}

void RecorderAPI::setExecPath(const std::filesystem::path &path)
{
    FFmpeg::setExecPath(path);
}

void
RecorderAPI::startRecording(isize x1, isize y1, isize x2, isize y2,
                            isize bitRate,
                            isize aspectX, isize aspectY)
{
    recorder->startRecording(x1, y1, x2, y2, bitRate, aspectX, aspectY);
}

void
RecorderAPI::stopRecording()
{
    recorder->stopRecording();
}

bool
RecorderAPI::exportAs(const std::filesystem::path &path)
{
    return recorder->exportAs(path);
}


//
// RemoteManagerAPI
//

const RemoteManagerInfo &
RemoteManagerAPI::getInfo() const
{
    return remoteManager->getInfo();
}

const RemoteManagerInfo &
RemoteManagerAPI::getCachedInfo() const
{
    return remoteManager->getCachedInfo();
}


// RetroShellAPI
//

const char *
RetroShellAPI::text()
{
    return retroShell->text();
}

isize
RetroShellAPI::cursorRel()
{
    return retroShell->cursorRel();
}

void
RetroShellAPI::press(RetroShellKey key, bool shift)
{
    retroShell->press(key, shift);
}

void
RetroShellAPI::press(char c)
{
    retroShell->press(c);
}

void
RetroShellAPI::press(const string &s)
{
    retroShell->press(s);
}

void
RetroShellAPI::execScript(std::stringstream &ss)
{
    retroShell->asyncExecScript(ss);
}

void
RetroShellAPI::execScript(const std::ifstream &fs)
{
    retroShell->asyncExecScript(fs);
}

void
RetroShellAPI::execScript(const string &contents)
{
    retroShell->asyncExecScript(contents);
}

void
RetroShellAPI::execScript(const MediaFile &file)
{
    retroShell->asyncExecScript(file);
}

void
RetroShellAPI::setStream(std::ostream &os)
{
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
    agnus.dma.emu = emu;
    agnus.dma.debugger.emu = emu;
    agnus.dma.debugger.dmaDebugger = &emu->main.agnus.dmaDebugger;
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

    host.emu = emu;
    host.host = &emu->main.host;

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
    emu->halt();
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
    return emu->getInfo();
}

const EmulatorInfo &
VAmiga::getCachedInfo() const
{
    return emu->getCachedInfo();
}

const EmulatorStats &
VAmiga::getStats() const
{
    return emu->getStats();
}

bool
VAmiga::isPoweredOn() const
{
    return emu->main.isPoweredOn();
}

bool
VAmiga::isPoweredOff() const
{
    return emu->main.isPoweredOff();
}

bool
VAmiga::isPaused() const
{
    return emu->main.isPaused();
}

bool
VAmiga::isRunning() const
{
    return emu->main.isRunning();
}

bool
VAmiga::isSuspended() const
{
    return emu->main.isSuspended();
}

bool
VAmiga::isHalted() const
{
    return emu->main.isHalted();
}

bool
VAmiga::isWarping() const
{
    return emu->isWarping();
}

bool
VAmiga::isTracking() const
{
    return emu->isTracking();
}

void
VAmiga::isReady() const
{
    return emu->isReady();
}

void
VAmiga::powerOn()
{
    emu->Thread::powerOn();
    emu->isDirty = true;
}

void
VAmiga::powerOff()
{
    emu->Thread::powerOff();
    emu->isDirty = true;
}

void
VAmiga::run()
{
    emu->run();
    emu->isDirty = true;
}

void
VAmiga::pause()
{
    emu->pause();
    emu->isDirty = true;
}

void 
VAmiga::hardReset()
{
    emu->hardReset();
    emu->isDirty = true;
}

void
VAmiga::softReset()
{
    emu->softReset();
    emu->isDirty = true;
}

void
VAmiga::halt()
{
    emu->halt();
    emu->isDirty = true;
}

void
VAmiga::suspend()
{
    emu->suspend();
}

void
VAmiga::resume()
{
    emu->resume();
}

void
VAmiga::warpOn(isize source)
{
    emu->warpOn(source);
}

void
VAmiga::warpOff(isize source)
{
    emu->warpOff(source);
}

void
VAmiga::trackOn(isize source)
{
    emu->trackOn(source);
}

void
VAmiga::trackOff(isize source)
{
    emu->trackOff(source);
}

void
VAmiga::stepInto()
{
    emu->stepInto();
}

void
VAmiga::stepOver()
{
    emu->stepOver();
}

void
VAmiga::wakeUp()
{
    emu->wakeUp();
}

void
VAmiga::launch(const void *listener, Callback *func)
{
    assert(isUserThread());
    emu->launch(listener, func);
}

bool
VAmiga::isLaunched() const
{
    return emu->isLaunched();
}

i64
VAmiga::get(Option option) const
{
    assert(isUserThread());
    return emu->get(option);
}

i64
VAmiga::get(Option option, long id) const
{
    assert(isUserThread());
    return emu->get(option, id);
}

void
VAmiga::set(ConfigScheme model)
{
    assert(isUserThread());
    emu->set(model);
    emu->isDirty = true;
}

void
VAmiga::set(Option opt, i64 value) throws
{
    assert(isUserThread());

    emu->check(opt, value);
    put(CMD_CONFIG_ALL, ConfigCmd { .option = opt, .value = value });
    emu->isDirty = true;
}

void
VAmiga::set(Option opt, i64 value, long id)
{
    assert(isUserThread());

    emu->check(opt, value, { id });
    put(CMD_CONFIG, ConfigCmd { .option = opt, .value = value, .id = id });
    emu->isDirty = true;
}

void
VAmiga::exportConfig(const fs::path &path, bool diff) const
{
    assert(isUserThread());
    emu->main.exportConfig(path, diff);
}

void
VAmiga::exportConfig(std::ostream& stream, bool diff) const
{
    emu->main.exportConfig(stream, diff);
}

void
VAmiga::put(const Cmd &cmd)
{
    emu->put(cmd);
}


//
// AmigaAPI
//

MediaFile *
AmigaAPI::takeSnapshot()
{
    return amiga->takeSnapshot();
}

void 
AmigaAPI::loadSnapshot(const MediaFile &snapshot)
{
    amiga->loadSnapshot(snapshot);
    emu->isDirty = true;
}
    
u64
AmigaAPI::getAutoInspectionMask() const
{
    return amiga->getAutoInspectionMask();
}

void
AmigaAPI::setAutoInspectionMask(u64 mask)
{
    amiga->setAutoInspectionMask(mask);
}


}
