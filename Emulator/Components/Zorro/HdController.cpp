// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "HdController.h"
#include "HdControllerRom.h"
#include "Amiga.h"
#include "HDFFile.h"
#include "FloppyDrive.h"
#include "OSDebugger.h"
#include "OSDescriptors.h"

namespace vamiga {

HdController::HdController(Amiga& ref, HardDrive& hdr) : ZorroBoard(ref), drive(hdr)
{

}

const char *
HdController::getDescription() const
{
    switch (nr) {
            
        case 0: return "Hd0Con";
        case 1: return "Hd1Con";
        case 2: return "Hd2Con";
        case 3: return "Hd3Con";

        default:
            fatalError;
    }
}

void
HdController::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    ZorroBoard::_dump(category, os);
    
    if (category == Category::Config) {
        
        os << tab("Connected");
        os << bol(config.connected) << std::endl;
    }
    
    if (category == Category::Stats) {
        
        for (isize i = 0; IoCommandEnum::isValid(i); i++) {
            
            os << tab(IoCommandEnum::key(i));
            os << stats.cmdCount[i] << std::endl;
        }
    }
}

void
HdController::_initialize()
{
    CoreComponent::_initialize();

    nr = drive.getNr();
}

void
HdController::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    if (hard) {
        
        // Burn Expansion Rom
        rom.init(exprom, EXPROM_SIZE);
        
        // Make the device name unique
        char dosName[] = "hrddrive?.device"; dosName[8] = char('0' + nr);
        rom.patch("virtualhd.device", dosName);

        // Patch Kickstart Rom (1.2 only)
        mem.patchExpansionLib();

        // Set initial state
        state = pluggedIn() ? STATE_AUTOCONF : STATE_SHUTUP;
        resetHdcState();
        
        // Wipe out previously recorded usage information
        clearStats();
    }
}

void
HdController::resetConfig()
{
    assert(isPoweredOff());
    auto &defaults = amiga.defaults;

    nr = drive.getNr();
    
    std::vector <Option> options = {
        
        OPT_HDC_CONNECT
    };

    for (auto &option : options) {
        setConfigItem(option, defaults.get(option, nr));
    }
}

i64
HdController::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_HDC_CONNECT:       return (long)config.connected;

        default:
            fatalError;
    }
}

void
HdController::setConfigItem(Option option, i64 value)
{
    switch (option) {

        case OPT_HDC_CONNECT:
            
            if (!isPoweredOff()) {
                throw VAError(ERROR_OPT_LOCKED);
            }
            
            if (bool(value) == config.connected) {
                break;
            }

            if (value) {
                
                config.connected = true;
                drive.connect();
                msgQueue.put(MSG_HDC_CONNECT, DriveMsg { i16(nr), true, 0, 0 } );

            } else {
                
                config.connected = false;
                drive.disconnect();
                msgQueue.put(MSG_HDC_CONNECT, DriveMsg { i16(nr), false, 0, 0 } );
            }
            return;

        default:
            fatalError;
    }
}

bool
HdController::pluggedIn() const
{
    return drive.isConnected() && !drive.data.empty();
}

void
HdController::updateMemSrcTables()
{
    // Only proceed if this board has been configured
    if (baseAddr == 0) return;
    
    // Map in this device
    mem.cpuMemSrc[firstPage()] = MEM_ZOR;
}

bool
HdController::isCompatible(u32 crc32)
{
    switch (crc32) {
            
        case CRC32_KICK13_34_005_A500:
        case CRC32_KICK13_34_005_A3000:
            
        case CRC32_KICK20_36_028:
        case CRC32_KICK202_36_207_A3000:
        case CRC32_KICK204_37_175_A500:
        case CRC32_KICK204_37_175_A3000:
        case CRC32_KICK205_37_299_A600:
        case CRC32_KICK205_37_300_A600HD:
        case CRC32_KICK205_37_350_A600HD:
            
        case CRC32_KICK30_39_106_A1200:
        case CRC32_KICK30_39_106_A4000:
        case CRC32_KICK31_40_063_A500:
        case CRC32_KICK31_40_068_A1200:
        case CRC32_KICK31_40_068_A3000:
        case CRC32_KICK31_40_068_A4000:
        case CRC32_KICK31_40_070_A4000T:
            return true;
            
        default:
            return false;
    }
}

bool
HdController::isCompatible()
{
    return isCompatible(mem.romFingerprint());
}

void
HdController::resetHdcState()
{
    hdcState = HDC_UNDETECTED;
    msgQueue.put(MSG_HDC_STATE, HdcMsg { i16(nr), hdcState });
}

void
HdController::changeHdcState(HdcState newState)
{
    if (hdcState != newState) {
        
        debug(HDR_DEBUG, "Changing state to %s\n", HdcStateEnum::key(newState));
        
        hdcState = newState;
        msgQueue.put(MSG_HDC_STATE, HdcMsg { i16(nr), hdcState });
    }
}

u8
HdController::peek8(u32 addr)
{
    auto result = spypeek8(addr);

    trace(ZOR_DEBUG, "peek8(%06x) = %02x\n", addr, result);
    return result;
}

u16
HdController::peek16(u32 addr)
{
    auto result = spypeek16(addr);

    trace(ZOR_DEBUG, "peek16(%06x) = %04x\n", addr, result);
    return result;
}

u8
HdController::spypeek8(u32 addr) const
{
    isize offset = (isize)(addr & 0xFFFF) - (isize)initDiagVec();
    return offset < rom.size ? rom[offset] : 0;
}

u16
HdController::spypeek16(u32 addr) const
{
    isize offset = (isize)(addr & 0xFFFF) - (isize)initDiagVec();
    
    switch (offset) {
            
        case EXPROM_SIZE:
            
            // Return the number of partitions
            debug(HDR_DEBUG, "Partitions: %ld\n", drive.numPartitions());
            return u16(drive.numPartitions());
            
        case EXPROM_SIZE + 2:
            
            // Number of filesystem drivers to add
            debug(HDR_DEBUG, "Filesystem drivers: %ld\n", drive.numDrivers());
            return u16(drive.numDrivers());
            
        case EXPROM_SIZE + 4:

            // Should auto boot be disabled?
            if (df0.hasDisk() || drive.bootable == false) {

                debug(HDR_DEBUG, "Disabling auto boot\n");
                return u16(true);
            }

            return u16(false);

        case EXPROM_SIZE + 6:
            
            // Number of shared folders (not supported yet)
            debug(HDR_DEBUG, "Shared folders: 0\n");
            return 0;
            
        default:
            
            // Return Rom code
            return offset < rom.size ? HI_LO(rom[offset], rom[offset + 1]) : 0;
    }
}

void
HdController::poke8(u32 addr, u8 value)
{
    trace(ZOR_DEBUG, "poke8(%06x,%02x)\n", addr, value);
}

void
HdController::poke16(u32 addr, u16 value)
{
    trace(ZOR_DEBUG, "poke16(%06x,%04x)\n", addr, value);
    
    isize offset = (isize)(addr & 0xFFFF) - (isize)initDiagVec();

    switch (offset) {
            
        case EXPROM_SIZE:
            
            pointer = REPLACE_HI_WORD(pointer, value);
            break;
            
        case EXPROM_SIZE + 2:

            pointer = REPLACE_LO_WORD(pointer, value);
            break;

        case EXPROM_SIZE + 4:

            switch (value) {
                    
                case 0xfede: processCmd(pointer); break;
                case 0xfedf: processInit(pointer); break;
                case 0xfee0: processResource(pointer); break;
                case 0xfee1: processInfoReq(pointer); break;
                case 0xfee2: processInitSeg(pointer); break;
                    
                default:
                    warn("Invalid value: %x\n", value);
                    break;
            }
            break;

        default:

            warn("Invalid addr: %x\n", addr);
            break;
    }
}

void
HdController::processCmd(u32 ptr)
{
    u8 error = 0;
    u32 actual = 0;
    
    // Read the IOStdReq struct from memory
    os::IOStdReq stdReq;
    osDebugger.read(ptr, &stdReq);
    
    // Extract information
    auto cmd = IoCommand(stdReq.io_Command);
    auto offset = isize(stdReq.io_Offset);
    auto length = isize(stdReq.io_Length);
    auto addr = u32(stdReq.io_Data);
    
    if (HDR_DEBUG) {

        [[maybe_unused]] auto unit = mem.spypeek32 <ACCESSOR_CPU> (stdReq.io_Unit + 0x2A);
        [[maybe_unused]] auto blck = offset / 512;
        
        debug(HDR_DEBUG, "%d.%ld: %s\n", unit, blck, IoCommandEnum::key(cmd));
    }
    
    // Update the usage profile
    if (IoCommandEnum::isValid(cmd)) stats.cmdCount[cmd]++;
    
    switch (cmd) {
            
        case CMD_READ:

            if (offset) changeHdcState(HDC_READY);
            
            error = drive.read(offset, length, addr);
            actual = u32(length);
            break;

        case CMD_WRITE:
        case CMD_TD_FORMAT:

            error = drive.write(offset, length, addr);
            actual = u32(length);
            break;

        case CMD_RESET:
        case CMD_UPDATE:
        case CMD_CLEAR:
        case CMD_STOP:
        case CMD_START:
        case CMD_FLUSH:
        case CMD_TD_MOTOR:
        case CMD_TD_SEEK:
        case CMD_TD_REMOVE:
        case CMD_TD_CHANGENUM:
        case CMD_TD_CHANGESTATE:
        case CMD_TD_PROTSTATUS:
        case CMD_TD_ADDCHANGEINT:
        case CMD_TD_REMCHANGEINT:
            
            break;
            
        default:
            
            debug(HDR_DEBUG, "Unsupported cmd: %ld (%s)\n", cmd, IoCommandEnum::key(cmd));
            error = u8(IOERR_NOCMD);
    }
    
    // Write back the return code
    mem.patch(ptr + IO_ERROR, error);
    
    // On success, report the number of processed bytes
    if (!error) mem.patch(ptr + IO_ACTUAL, actual);
}

void
HdController::processInit(u32 ptr)
{
    debug(HDR_DEBUG, "processInit(%x)\n", ptr);

    auto assignDosName = [&](isize partition, char *name) {

        name[0] = 'D';
        name[1] = 'H';

        if (nr == 0) {
            
            name[2] = '0' + char(partition);
            name[3] = 0;
            name[4] = 0;

        } else {

            name[2] = '0' + char(nr);
            name[3] = '0' + char(partition);
            name[4] = 0;

        }
    };

    // Keep in check with exprom.asm
    constexpr u16 devn_dosName      = 0x00;  // APTR  Pointer to DOS file handler name
    constexpr u16 devn_unit         = 0x08;  // ULONG Unit number
    constexpr u16 devn_flags        = 0x0C;  // ULONG OpenDevice flags
    constexpr u16 devn_sizeBlock    = 0x14;  // ULONG # longwords in a block
    constexpr u16 devn_secOrg       = 0x18;  // ULONG sector origin -- unused
    constexpr u16 devn_numHeads     = 0x1C;  // ULONG number of surfaces
    constexpr u16 devn_secsPerBlk   = 0x20;  // ULONG secs per logical block
    constexpr u16 devn_blkTrack     = 0x24;  // ULONG secs per track
    constexpr u16 devn_resBlks      = 0x28;  // ULONG reserved blocks -- MUST be at least 1!
    constexpr u16 devn_interleave   = 0x30;  // ULONG interleave
    constexpr u16 devn_lowCyl       = 0x34;  // ULONG lower cylinder
    constexpr u16 devn_upperCyl     = 0x38;  // ULONG upper cylinder
    constexpr u16 devn_numBuffers   = 0x3C;  // ULONG number of buffers
    constexpr u16 devn_memBufType   = 0x40;  // ULONG Type of memory for AmigaDOS buffers
    constexpr u16 devn_transferSize = 0x44;  // LONG  largest transfer size (largest signed #)
    constexpr u16 devn_addMask      = 0x48;  // ULONG address mask
    constexpr u16 devn_bootPrio     = 0x4c;  // ULONG boot priority
    constexpr u16 devn_dName        = 0x50;  // char[4] DOS file handler name
    constexpr u16 devn_bootflags    = 0x54;  // boot flags (not part of DOS packet)
    constexpr u16 devn_segList      = 0x58;  // filesystem segment list (not part of DOS packet)
    
    u32 unit = mem.spypeek32 <ACCESSOR_CPU> (ptr + devn_unit);
    
    if (unit < drive.ptable.size()) {

        debug(HDR_DEBUG, "Initializing partition %d\n", unit);
        changeHdcState(HDC_INITIALIZING);
        
        // Collect hard drive information
        auto &geometry = drive.geometry;
        auto &part = drive.ptable[unit];
        char dosName[5];
        assignDosName(unit, dosName);

        u32 name_ptr = mem.spypeek32 <ACCESSOR_CPU> (ptr + devn_dosName);
        for (isize i = 0; i < isizeof(dosName); i++) {
            mem.patch(u32(name_ptr + i), u8(dosName[i]));
        }

        u32 segList = 0;
        for (auto &driver : drive.drivers) {
            if (driver.dosType == part.dosType) {
                segList = driver.segList;
                debug(HDR_DEBUG, "Using seglist at BPTR %x\n", segList);
            }
        }
        
        mem.patch(ptr + devn_flags,         u32(part.flags));
        mem.patch(ptr + devn_sizeBlock,     u32(part.sizeBlock));
        mem.patch(ptr + devn_secOrg,        u32(0));
        mem.patch(ptr + devn_numHeads,      u32(geometry.heads));
        mem.patch(ptr + devn_secsPerBlk,    u32(1));
        mem.patch(ptr + devn_blkTrack,      u32(geometry.sectors));
        mem.patch(ptr + devn_interleave,    u32(0));
        mem.patch(ptr + devn_resBlks,       u32(part.reserved));
        mem.patch(ptr + devn_lowCyl,        u32(part.lowCyl));
        mem.patch(ptr + devn_upperCyl,      u32(part.highCyl));
        mem.patch(ptr + devn_numBuffers,    u32(30));
        mem.patch(ptr + devn_memBufType,    u32(0));
        mem.patch(ptr + devn_transferSize,  u32(0x7FFFFFFF));
        mem.patch(ptr + devn_addMask,       u32(0xFFFFFFFE));
        mem.patch(ptr + devn_bootPrio,      u32(0));
        mem.patch(ptr + devn_dName,         u32(part.dosType));
        mem.patch(ptr + devn_bootflags,     u32(part.flags & 1));
        mem.patch(ptr + devn_segList,       u32(segList));
        
        if ((part.dosType & 0xFFFFFFF0) != 0x444f5300) {
            debug(HDR_DEBUG, "Unusual DOS type %x\n", part.dosType);
        }
        
        numPartitions = std::max(isize(unit), numPartitions);

    } else {

        debug(HDR_DEBUG, "Partition %d does not exist\n", unit);
    }
}

void
HdController::processResource(u32 ptr)
{
    debug(HDR_DEBUG, "processResource(%x)\n", ptr);

    // Read the file system resource
    os::FileSysResource fsResource;
    osDebugger.read(ptr, &fsResource);

    // Read file system entries
    std::vector <os::FileSysEntry> entries;
    osDebugger.read(fsResource.fsr_FileSysEntries.lh_Head, entries);

    auto &drivers = drive.drivers;
    
    for (const auto &fse : entries) {
        
        debug(HDR_DEBUG, "Providing %s %s\n",
              OSDebugger::dosTypeStr(fse.fse_DosType).c_str(),
              OSDebugger::dosVersionStr(fse.fse_Version).c_str());
        
        for (auto it = drivers.begin(); it != drivers.end(); ) {

            if (HDR_FS_LOAD_ALL) {

                it++;
                continue;
            }

            if (fse.fse_DosType == it->dosType && fse.fse_Version >= it->dosVersion) {
                
                debug(HDR_DEBUG, "Not needed: %s %s\n",
                      OSDebugger::dosTypeStr(it->dosType).c_str(),
                      OSDebugger::dosVersionStr(it->dosVersion).c_str());

                it = drivers.erase(it);

            } else {
                
                it++;
            }
        }
    }
    
    debug(HDR_DEBUG, "Remaining drivers: %zu\n", drivers.size());
}

void
HdController::processInfoReq(u32 ptr)
{
    debug(HDR_DEBUG, "processInfoReq(%x)\n", ptr);
    
    // Keep in sync with exprom.asm
    static constexpr u16 fsinfo_num = 0x00;
    static constexpr u16 fsinfo_dosType = 0x02;
    static constexpr u16 fsinfo_version = 0x06;
    static constexpr u16 fsinfo_numHunks = 0x0a;
    static constexpr u16 fsinfo_hunk = 0x0e;

    try {

        // Read driver number
        u16 num = mem.spypeek16 <ACCESSOR_CPU> (ptr + fsinfo_num);
        debug(HDR_DEBUG, "Requested info for driver %d\n", num);

        if (num >= drive.drivers.size()) {
            throw VAError(ERROR_HDC_INIT, "Invalid driver number: " + std::to_string(num));
        }
        auto &driver = drive.drivers[num];

        // Read driver
        Buffer<u8> code;
        drive.readDriver(num, code);
        ProgramUnitDescriptor descr(code);
        descr.dump(Category::Sections);
        
        // We accept up to three hunks
        auto numHunks = descr.numHunks();
        if (numHunks == 0 || numHunks > 3) {
            throw VAError(ERROR_HUNK_CORRUPTED);
        }
        
        // Pass the hunk information back to the driver
        mem.patch(ptr + fsinfo_dosType, u32(driver.dosType));
        mem.patch(ptr + fsinfo_version, u32(driver.dosVersion));
        mem.patch(ptr + fsinfo_numHunks, u32(numHunks));
        for (isize i = 0; i < numHunks; i++) {
            mem.patch(u32(ptr + fsinfo_hunk + 4 * i), descr.hunks[i].memRaw);
        }

    } catch (VAError &e) {

        warn("processInfoReq: %s\n", e.what());
    }
}

void
HdController::processInitSeg(u32 ptr)
{
    debug(HDR_DEBUG, "processInitSeg(%x)\n", ptr);
    
    static constexpr u16 fsinitseg_hunk = 0x00;
    static constexpr u16 fsinitseg_num = 0x0c;
    
    try {
        
        // Read driver number
        u32 num = mem.spypeek32 <ACCESSOR_CPU> (ptr + fsinitseg_num);
        debug(HDR_DEBUG, "Processing driver %d\n", num);

        if (num >= drive.drivers.size()) {
            throw VAError(ERROR_HDC_INIT, "Invalid driver number: " + std::to_string(num));
        }

        // Read driver
        Buffer<u8> code;
        drive.readDriver(num, code);
        ProgramUnitDescriptor descr(code);

        // We accept up to three hunks
        auto numHunks = descr.numHunks();
        if (numHunks == 0 || numHunks > 3) {
            throw VAError(ERROR_HUNK_CORRUPTED);
        }
        
        // Extract pointers to the allocated memory
        std::vector<u32> segPtrs;
        for (isize i = 0; i < numHunks; i++) {
            
            auto segPtrAddr = u32(ptr + fsinitseg_hunk + 4 * i);
            auto segPtr = mem.spypeek32 <ACCESSOR_CPU> (segPtrAddr);
            
            if (segPtr == 0) {
                throw VAError(ERROR_HDC_INIT, "Memory allocation failed inside AmigaOS");
            }
            debug(HDR_DEBUG, "Allocated memory at %x\n", segPtr);
            segPtrs.push_back(segPtr);
        }
        
        // Build seglist
        for (isize i = 0; i < numHunks; i++) {

            bool last = (i == numHunks - 1);

            for (auto &s : descr.hunks[i].sections) {
                
                if (s.type == HUNK_CODE || s.type == HUNK_DATA) {
                    
                    // Write hunk size
                    mem.patch(segPtrs[i], u32(descr.hunks[i].memSize + 8));

                    // Add a BPTR to the next hunk in the list
                    mem.patch(segPtrs[i] + 4, last ? 0 : (segPtrs[i + 1] + 4) >> 2);

                    // Copy data
                    debug(HDR_DEBUG, "Copying %d bytes from %d\n", s.size, s.offset + 8);
                    mem.patch(segPtrs[i] + 8, code.ptr + s.offset + 8, s.size);
                }
            }
            
            // Apply relocations
            for (auto &s : descr.hunks[i].sections) {
                
                if (s.type == HUNK_RELOC32) {
                    
                    if (s.target >= numHunks) {
                        throw VAError(ERROR_HDC_INIT, "Invalid relocation target");
                    }
                    debug(HDR_DEBUG, "Relocation target: %ld\n", s.target);
                    
                    for (auto &offset : s.relocations) {
                        
                        auto addr = segPtrs[i] + 8 + offset;
                        auto value = mem.spypeek32 <ACCESSOR_CPU> (addr);
                        debug(HDR_DEBUG, "%x: %x -> %x\n",
                              addr, value, value + segPtrs[s.target] + 8)
                        mem.patch(addr, value + segPtrs[s.target] + 8);
                    }
                }
            }
        }
        
        // Remember a BPTR to the seglist
        drive.drivers[num].segList = (segPtrs[0] + 4) >> 2;
        
    } catch (VAError &e) {

        warn("processInitSeg: %s\n", e.what());
    }
    
    debug(HDR_DEBUG, "processInitSeg completed\n");
}

}
