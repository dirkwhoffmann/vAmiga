// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Copper.h"
#include "Amiga.h"
#include "CopperDebugger.h"
#include "Checksum.h"
#include "IO.h"
#include "PixelEngine.h"

Copper::Copper(Amiga& ref) : SubComponent(ref)
{
    subComponents = std::vector<AmigaComponent *> {
        
        &debugger
    };
}

void
Copper::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
}

void
Copper::_inspect() const
{    
    synchronized {
        
        info.copList = copList;
        info.copList1Start = debugger.startOfCopperList(1);
        info.copList1End = debugger.endOfCopperList(1);
        info.copList2Start = debugger.startOfCopperList(2);
        info.copList2End = debugger.endOfCopperList(2);
        info.active = scheduler.isPending<SLOT_COP>();
        info.cdang = cdang;
        info.coppc = coppc & agnus.ptrMask;
        info.cop1lc = cop1lc & agnus.ptrMask;
        info.cop2lc = cop2lc & agnus.ptrMask;
        info.cop1ins = cop1ins;
        info.cop2ins = cop2ins;
        info.length1 = (cop1end - cop1lc) / 4;
        info.length2 = (cop2end - cop2lc) / 4;
    }
}

void
Copper::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::State) {
        
        os << tab("Active Copper list");
        os << dec(copList) << std::endl;
        os << tab("Skip flag");
        os << bol(skip) << std::endl;        
    }
    
    if (category & dump::Registers) {
        
        os << tab("COPPC");
        os << hex(coppc) << std::endl;
        os << tab("COP1LC");
        os << hex(cop1lc) << std::endl;
        os << tab("COP1LC");
        os << hex(cop2lc) << std::endl;
        os << tab("COPINS1");
        os << hex(cop1ins) << std::endl;
        os << tab("COPINS2");
        os << hex(cop2ins) << std::endl;
        os << tab("CDANG");
        os << bol(cdang) << std::endl;
    }
    
    if ((category & dump::List1) || (category & dump::List2)) {
        
        debugger.dump(category, os);
    }
}

void
Copper::setPC(u32 addr)
{
    coppc = addr;
    
    // Notify the debugger
    if (amiga.inDebugMode()) { debugger.jumped(); }
}

void
Copper::advancePC()
{
    coppc += 2;
    
    // Notify the debugger
    if (amiga.inDebugMode()) { debugger.advanced(); }
}

void
Copper::switchToCopperList(isize nr)
{
    assert(nr == 1 || nr == 2);

    copList = nr;
    setPC(nr == 1 ? cop1lc : cop2lc);
    agnus.scheduleRel<SLOT_COP>(0, COP_REQ_DMA);
}

bool
Copper::findMatch(Beam &match) const
{
    // Start searching at the current beam position
    u32 beam = (u32)(agnus.pos.v << 8 | agnus.pos.h);

    // Get the comparison position and the comparison mask
    u32 comp = getVPHP();
    u32 mask = getVMHM();

    // Iterate through all lines starting from the current position
    isize numLines = agnus.frame.numLines();
    while ((beam >> 8) < numLines) {

        // Check if the vertical components are equal
        if ((beam & mask & ~0xFF) == (comp & mask & ~0xFF)) {

            // debug("Matching vertically: beam = %X comp = %X mask = %X\n", beam, comp, mask);

            // Try to match the horizontal coordinate as well
            if (findHorizontalMatch(beam, comp, mask)) {

                // Success
                match.v = beam >> 8;
                match.h = beam & 0xFF;
                return true;
            }
        }

        // Check if the vertical beam position is greater
        else if ((beam & mask & ~0xFF) > (comp & mask & ~0xFF)) {

            // Success
            match.v = beam >> 8;
            match.h = beam & 0xFF;
            return true;
        }

        // Jump to the beginning of the next line
        beam = (beam & ~0xFF) + 0x100;
    }

    return false;
}

bool
Copper::findHorizontalMatch(u32 &match, u32 comp, u32 mask) const
{
    // Iterate through all horizontal positions
    for (u32 beam = match; (beam & 0xFF) < HPOS_CNT; beam++) {

        // Check if the comparator triggers at this position
        if ((beam & mask) >= (comp & mask)) {

            // Success
            match = beam;
            return true;
        }
    }

    return false;
}

void
Copper::move(u32 addr, u16 value)
{
    assert(IS_EVEN(addr));
    assert(addr < 0x1FF);
    
    trace(COP_DEBUG,
          "COPPC: %X move(%s, $%X) (%d)\n", coppc, Memory::regName(addr), value, value);

    // Catch registers with special timing needs
    if (addr >= 0x180 && addr <= 0x1BE) {

        trace(OCSREG_DEBUG,
              "pokeCustom16(%X [%s], %X)\n", addr, Memory::regName(addr), value);

        // Color registers
        pixelEngine.colChanges.insert(4 * agnus.pos.h, RegChange { addr, value} );
        return;
    }

    // Write the value
    agnus.doCopperDmaWrite(addr, value);
}

bool
Copper::comparator(Beam beam, u16 waitpos, u16 mask) const
{
    // Get comparison bits for the vertical beam position
    u8 vBeam = beam.v & 0xFF;
    u8 vWaitpos = HI_BYTE(waitpos);
    u8 vMask = HI_BYTE(mask) | 0x80;

    // Compare vertical positions
    if ((vBeam & vMask) < (vWaitpos & vMask)) {
        // debug("beam %d waitpos %d mask 0x%x FALSE\n", beam, waitpos, mask);
        return false;
    }
    if ((vBeam & vMask) > (vWaitpos & vMask)) {
        // debug("beam %d waitpos %d mask 0x%x TRUE\n", beam, waitpos, mask);
        return true;
    }

    // Get comparison bits for horizontal position
    u8 hBeam = beam.h & 0xFE;
    u8 hWaitpos = LO_BYTE(waitpos) & 0xFE;
    u8 hMask = LO_BYTE(mask) & 0xFE;

    // Compare horizontal positions
    return (hBeam & hMask) >= (hWaitpos & hMask);
}

bool
Copper::comparator(Beam beam) const
{
    return comparator(beam, getVPHP(), getVMHM());
}

bool
Copper::comparator() const
{
    return comparator(agnus.pos);
}

void
Copper::scheduleWaitWakeup(bool bfd)
{
    Beam trigger;

    // Find the trigger position for this WAIT command
    if (findMatch(trigger)) {

        // In how many cycles do we get there?
        isize delay = trigger - agnus.pos;

        // msg("(%d,%d) matches in %d cycles\n", trigger.v, trigger.h, delay);

        if (delay == 0) {

            // Copper does not stop
            agnus.scheduleRel<SLOT_COP>(DMA_CYCLES(2), COP_FETCH);

        } else if (delay == 2) {

            // Copper does not stop
            agnus.scheduleRel<SLOT_COP>(DMA_CYCLES(2), COP_FETCH);

        } else {

            // Wake up 2 cycles earlier with a WAKEUP event
            delay -= 2;
            if (bfd) {
                agnus.scheduleRel<SLOT_COP>(DMA_CYCLES(delay), COP_WAKEUP);
            } else {
                agnus.scheduleRel<SLOT_COP>(DMA_CYCLES(delay), COP_WAKEUP_BLIT);
            }
        }

    } else {

        // msg("(%d,%d) does not match in this frame\n", trigger.v, trigger.h);
        scheduler.scheduleAbs<SLOT_COP>(NEVER, COP_REQ_DMA);
    }
}

bool
Copper::isMoveCmd() const
{
    return !(cop1ins & 1);
}

bool Copper::isMoveCmd(u32 addr) const
{
    assert(IS_EVEN(addr));

    u16 hiword = mem.spypeek16 <ACCESSOR_AGNUS> (addr);

    return IS_EVEN(hiword);
}

bool Copper::isWaitCmd() const
{
     return (cop1ins & 1) && !(cop2ins & 1);
}

bool Copper::isWaitCmd(u32 addr) const
{
    assert(IS_EVEN(addr));

    u16 hiword = mem.spypeek16 <ACCESSOR_AGNUS> (addr);
    u16 loword = mem.spypeek16 <ACCESSOR_AGNUS> (addr + 2);

    return IS_ODD(hiword) && IS_EVEN(loword);
}

u16
Copper::getRA() const
{
    return cop1ins & 0x1FE;
}

u16
Copper::getRA(u32 addr) const
{
    u16 hiword = mem.spypeek16 <ACCESSOR_AGNUS> (addr);
    return hiword & 0x1FE;
}

u16
Copper::getDW() const
{
    return cop1ins;
}

u16
Copper::getDW(u32 addr) const
{
    u16 loword = mem.spypeek16 <ACCESSOR_AGNUS> (addr + 2);
    return loword;
}

bool
Copper::getBFD() const
{
    return (cop2ins & 0x8000) != 0;
}

bool
Copper::getBFD(u32 addr) const
{
    u16 instr = mem.spypeek16 <ACCESSOR_AGNUS> (addr + 2);
    return (instr & 0x8000) != 0;
}

u16
Copper::getVPHP() const
{
    return cop1ins & 0xFFFE;
}

u16
Copper::getVPHP(u32 addr) const
{
    u16 instr = mem.spypeek16 <ACCESSOR_AGNUS> (addr);
    return instr & 0xFFFE;
}

u16
Copper::getVMHM() const
{
    return (cop2ins & 0x7FFE) | 0x8001;
}

u16
Copper::getVMHM(u32 addr) const
{
    u16 instr = mem.spypeek16 <ACCESSOR_AGNUS> (addr + 2);
    return (instr & 0x7FFE) | 0x8001;
}

bool
Copper::isIllegalAddress(u32 addr) const
{
    if (cdang) {
        return agnus.isOCS() ? addr < 0x40 : false;
    } else {
        return addr < 0x80;
    }
}

bool
Copper::isIllegalInstr(u32 addr) const
{
    return isMoveCmd(addr) && isIllegalAddress(getRA(addr));
}

void
Copper::vsyncHandler()
{
    /* "At the start of each vertical blanking interval, COP1LC is automatically
     *  used to start the program counter. That is, no matter what the Copper is
     *  doing, when the end of vertical blanking occurs, the Copper is
     *  automatically forced to restart its operations at the address contained
     *  in COP1LC." [HRM]
     */
    agnus.scheduleRel<SLOT_COP>(DMA_CYCLES(0), COP_VBLANK);
    
    if (COP_CHECKSUM) {
        
        if (checkcnt) {
            msg("[%lld] Checksum: %x (%lld) lc1 = %x lc2 = %x\n",
                agnus.frame.nr, checksum, checkcnt, cop1lc, cop2lc);
        }
        checkcnt = 0;
        checksum = util::fnv_1a_init32();
    }
}

void
Copper::blitterDidTerminate()
{
    if (scheduler.hasEvent<SLOT_COP>(COP_WAIT_BLIT)) {

        // Wake up the Copper in the next even cycle
        if (IS_EVEN(agnus.pos.h)) {
            serviceEvent(COP_WAIT_BLIT);
        } else {
            agnus.scheduleRel<SLOT_COP>(DMA_CYCLES(1), COP_WAIT_BLIT);
        }
    }
}
