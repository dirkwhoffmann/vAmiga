// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "HdrController.h"
#include "HdrControllerRom.h"
#include "HardDrive.h"
#include "HDFFile.h"
#include "Memory.h"
#include "OSDebugger.h"

HdrController::HdrController(Amiga& ref, HardDrive& hdr) : ZorroBoard(ref), drive(hdr)
{
    nr = drive.getNr();
    
    // Copy Rom code
    rom = new u8[EXPROM_SIZE];
    std::memcpy((u8 *)rom, exprom, EXPROM_SIZE);
    
    // Setup the device and dos name for this device
    char devName[] = "DH?";             devName[2] = char('0' + nr);
    char dosName[] = "hdrv?.device";    dosName[4] = char('0' + nr);
    
    // Patch Rom
    util::replace((char *)rom, EXPROM_SIZE, "DH0", devName);
    util::replace((char *)rom, EXPROM_SIZE, "hello.device", dosName);
}

const char *
HdrController::getDescription() const
{
    switch (nr) {
            
        case 0: return "HdrController0";
        case 1: return "HdrController1";
        case 2: return "HdrController2";
        case 3: return "HdrController3";

        default:
            fatalError;
    }
}

void
HdrController::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
        
    ZorroBoard::_dump(category, os);
    
    if (category & dump::State) {
    
    }
}

void
HdrController::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    if (hard) {

        if (pluggedIn())  {

            state = STATE_AUTOCONF;
            debug(true, "Hard drive emulation enabled.\n");

        } else {
            
            state = STATE_SHUTUP;
            debug(true, "Hard drive emulation disabled. No HDF.\n");
        }
    }
}

bool
HdrController::pluggedIn() const
{
    return drive.isConnected();
}

void
HdrController::updateMemSrcTables()
{
    // Only proceed if this board has been configured
    if (baseAddr == 0) return;
    
    // Map in this device
    mem.cpuMemSrc[firstPage()] = MEM_ZOR;
}

u8
HdrController::peek8(u32 addr)
{
    auto result = spypeek8(addr);

    trace(ZOR_DEBUG, "peek8(%06x) = %02x\n", addr, result);
    return result;
}

u16
HdrController::peek16(u32 addr)
{
    auto result = spypeek16(addr);

    trace(ZOR_DEBUG, "peek16(%06x) = %04x\n", addr, result);
    return result;
}

u8
HdrController::spypeek8(u32 addr) const
{
    isize offset = (isize)(addr & 0xFFFF) - (isize)initDiagVec();
    return offset < EXPROM_SIZE ? rom[offset] : 0;
}

u16
HdrController::spypeek16(u32 addr) const
{
    return HI_LO(spypeek8(addr), spypeek8(addr + 1));
}

void
HdrController::poke8(u32 addr, u8 value)
{
    trace(ZOR_DEBUG, "poke8(%06x,%02x)\n", addr, value);
}

void
HdrController::poke16(u32 addr, u16 value)
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
                    
                case 0xfede:
                    
                    processCmd();
                    break;
                    
                case 0xfedf:
                    
                    processInit();
                    break;
                    
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
HdrController::processInit()
{
    trace(HDR_DEBUG, "processInit()\n");

    // Collect hard drive information
    auto geometry = drive.getGeometry();
    u32 sizeBlock = (u32)(geometry.bsize / 4);
    u32 numHeads = (u32)(geometry.heads);
    u32 blkTrack = (u32)(geometry.sectors);
    u32 upperCyl = (u32)(geometry.cylinders - 1);
    
    constexpr uint16_t devn_sizeBlock = 0x14; // number of longwords in a block
    constexpr uint16_t devn_numHeads  = 0x1C; // number of surfaces
    constexpr uint16_t devn_blkTrack  = 0x24; // secs per track
    constexpr uint16_t devn_upperCyl  = 0x38; // upper cylinder
    // constexpr uint16_t devn_dName     = 0x44;
                
    debug(true, "sizeBlock = %d\n", sizeBlock);
    debug(true, "numHeads = %d\n", numHeads);
    debug(true, "blkTrack = %d\n", blkTrack);
    debug(true, "upperCyl = %d\n", upperCyl);

    auto rom = pointer;
    mem.patch(rom + devn_sizeBlock, u32(sizeBlock));
    mem.patch(rom + devn_numHeads, u32(numHeads));
    mem.patch(rom + devn_blkTrack, u32(blkTrack));
    mem.patch(rom + devn_upperCyl, u32(upperCyl));
}

void
HdrController::processCmd()
{
    os::IOStdReq stdReq;
    osDebugger.read(pointer, &stdReq);
    
    auto cmd = IoCommand(stdReq.io_Command);
    auto offset = isize(stdReq.io_Offset);
    auto length = isize(stdReq.io_Length);
    auto addr = u32(stdReq.io_Data);

    debug(HDR_DEBUG, "Processing command %s\n", IoCommandEnum::key(cmd));

    switch (cmd) {
            
        case CMD_READ:
        {
            // Perform the operation
            auto error = drive.read(offset, length, addr);
                
            // Check for errors
            if (error) mem.patch(pointer + IO_ERROR, u8(error));
            break;
        }
        case CMD_WRITE:
        case CMD_TD_FORMAT:
        {
            // Perform the operation
            auto error = drive.write(offset, length, addr);
                
            // Check for errors
            if (error) mem.patch(pointer + IO_ERROR, u8(error));
            break;
        }
  
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
            
            mem.patch(pointer + IO_ACTUAL, u32(0));
            mem.patch(pointer + IO_ERROR, u8(0));
            break;
            
        default:
            
            warn("Unsupported command: %lx\n", cmd);
            mem.patch(pointer + IO_ERROR, u8(IOERR_NOCMD));
    }
}
