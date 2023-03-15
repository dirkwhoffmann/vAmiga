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
#include "IOUtils.h"
#include "PixelEngine.h"

namespace vamiga {

Copper::Copper(Amiga& ref) : SubComponent(ref)
{
    subComponents = std::vector<CoreComponent *> {
        
        &debugger
    };
}

void
Copper::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
}

void
Copper::setPC(u32 addr)
{
    coppc = addr;

    // Notify the debugger
    if (amiga.isTracking()) { debugger.jumped(); }
}

void
Copper::advancePC()
{
    coppc += 2;

    // Notify the debugger
    if (amiga.isTracking()) { debugger.advanced(); }
}

void
Copper::switchToCopperList(isize nr)
{
    assert(nr == 1 || nr == 2);

    copList = nr;
    setPC(nr == 1 ? cop1lc : cop2lc);
    agnus.scheduleRel <SLOT_COP> (0, COP_REQ_DMA);
}

bool
Copper::findMatchOld(Beam &match) const
{
    // Start searching at the current beam position
    u32 beam = (u32)(agnus.pos.v << 8 | agnus.pos.h);

    // Get the comparison position and the comparison mask
    u32 comp = getVPHP();
    u32 mask = getVMHM();

    // Iterate through all lines starting from the current position
    isize numLines = agnus.pos.vCnt();
    while ((isize)(beam >> 8) < numLines) {

        // Check if the vertical components are equal
        if ((beam & mask & ~0xFF) == (comp & mask & ~0xFF)) {

            // Try to match the horizontal coordinate as well
            if (findHorizontalMatchOld(beam, comp, mask)) {

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
Copper::findMatch(Beam &match) const
{
    // Start searching at the current beam position
    u32 beam = (u32)(agnus.pos.v << 8 | agnus.pos.h);

    // Get the comparison position and the comparison mask
    u32 comp = getVPHP();
    u32 mask = getVMHM();

    // Iterate through all lines starting from the current position
    isize numLines = agnus.pos.vCnt();
    while ((isize)(beam >> 8) < numLines) {

        // Check if the vertical components are equal
        if ((beam & mask & ~0xFF) == (comp & mask & ~0xFF)) {

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
Copper::findHorizontalMatchOld(u32 &match, u32 comp, u32 mask) const
{
    // The maximum horizontal trigger positon is $E1 in PAL machines
    const u32 maxhpos = 0xE1;
    
    // Iterate through all horizontal positions
    for (u32 beam = match; (beam & 0xFF) < maxhpos; beam++) {

        // Check if the comparator triggers at this position
        if ((beam & mask) >= (comp & mask)) {

            // Success
            match = beam;
            return true;
        }
    }

    return false;
}

bool
Copper::findHorizontalMatch(u32 &match, u32 comp, u32 mask) const
{
    u32 v = match & 0x1FF00;
    u32 h = match & 0x000FF;

    // Iterate through all horizontal positions execept the last three
    for (auto i = h + 2; i <= 0xE1; i++, h++) {

        // Check if the comparator triggers at this position
        if (((v | i) & mask) >= (comp & mask)) {

            match = v | h;
            return true;
        }
    }
    
    // Iterate through the last three cycles with a wrapped over counter
    for (auto i = 0; i <= 2; i++, h++) {

        // Check if the comparator triggers at this position
        if (((v | i) & mask) >= (comp & mask)) {

            match = v | h;
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
          "COPPC: %X move(%s, $%X) (%d)\n", coppc0, Memory::regName(addr), value, value);

    // Catch registers with special timing needs
    if (addr >= 0x180 && addr <= 0x1BE) {

        trace(OCSREG_DEBUG,
              "pokeCustom16(%X [%s], %X)\n", addr, Memory::regName(addr), value);

        // Color registers
        pixelEngine.colChanges.insert(agnus.pos.pixel(), RegChange { addr, value} );
        return;
    }

    // Write the value
    agnus.doCopperDmaWrite(addr, value);
}

bool
Copper::runComparator() const
{
    return runComparator(agnus.pos);
}

bool
Copper::runComparator(Beam beam) const
{
    return runComparator(beam, getVPHP(), getVMHM());
}

bool
Copper::runComparator(Beam beam, u16 waitpos, u16 mask) const
{    
    // Compare vertical position
    if ((beam.v & HI_BYTE(mask)) < HI_BYTE(waitpos & mask)) return false;
    if ((beam.v & HI_BYTE(mask)) > HI_BYTE(waitpos & mask)) return true;
    
    // Compare horizontal position
    return runHorizontalComparator(beam, waitpos, mask);
}

bool
Copper::runHorizontalComparator(Beam beam, u16 waitpos, u16 mask) const
{
    if (beam.h < 0xE0) {
        return ((beam.h + 0x02) & mask) >= (waitpos & 0xFF & mask);
    } else {
        return ((beam.h - 0xE0) & mask) >= (waitpos & 0xFF & mask);
    }
}

void
Copper::scheduleWaitWakeup(bool bfd)
{
    Beam trigger;

    // Find the trigger position for this WAIT command
    if (findMatch(trigger)) {

        // In how many cycles do we get there?
        // auto delay = agnus.frame.diff(trigger.v, trigger.h, agnus.pos.v, agnus.pos.h);
        // assert(delay == DMA_CYCLES(agnus.pos.diff(trigger.v, trigger.h)));
        auto delay = DMA_CYCLES(agnus.pos.diff(trigger.v, trigger.h));

        if (delay == 0) {

            EventID event = COP_FETCH;
            agnus.scheduleRel <SLOT_COP> (DMA_CYCLES(2), event);

        } else {

            EventID event = bfd ? COP_WAKEUP : COP_WAKEUP_BLIT;
            agnus.scheduleRel <SLOT_COP> (delay, event);
        }

    } else {

        agnus.scheduleAbs <SLOT_COP> (NEVER, COP_REQ_DMA);
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

bool Copper::isSkipCmd() const
{
    return (cop1ins & 1) && (cop2ins & 1);
}

bool Copper::isSkipCmd(u32 addr) const
{
    assert(IS_EVEN(addr));

    u16 hiword = mem.spypeek16 <ACCESSOR_AGNUS> (addr);
    u16 loword = mem.spypeek16 <ACCESSOR_AGNUS> (addr + 2);

    return IS_ODD(hiword) && IS_ODD(loword);
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
Copper::eofHandler()
{
    /* "At the start of each vertical blanking interval, COP1LC is automatically
     *  used to start the program counter. That is, no matter what the Copper is
     *  doing, when the end of vertical blanking occurs, the Copper is
     *  automatically forced to restart its operations at the address contained
     *  in COP1LC." [HRM]
     */
    agnus.scheduleRel <SLOT_COP> (DMA_CYCLES(0), COP_VBLANK);
    
    if constexpr (COP_CHECKSUM) {
        
        if (checkcnt) {
            msg("[%lld] Checksum: %x (%lld) lc1 = %x lc2 = %x\n",
                agnus.pos.frame, checksum, checkcnt, cop1lc, cop2lc);
        }
        checkcnt = 0;
        checksum = util::fnvInit32();
    }
}

void
Copper::blitterDidTerminate()
{
    if (agnus.hasEvent <SLOT_COP> (COP_WAIT_BLIT)) {

        // Wake up the Copper in the next even cycle
        if (IS_EVEN(agnus.pos.h)) {
            serviceEvent(COP_WAIT_BLIT);
        } else {
            agnus.scheduleRel <SLOT_COP> (DMA_CYCLES(1), COP_WAIT_BLIT);
        }
    }
}

}
