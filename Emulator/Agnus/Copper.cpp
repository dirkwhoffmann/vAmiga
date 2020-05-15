// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Copper::Copper(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("Copper");
}

void
Copper::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS
    bfd = true;
}

void
Copper::_inspect()
{
    u32 mask = agnus.chipRamMask();
    
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);

    info.copList = copList;
    info.active  = agnus.isPending<COP_SLOT>();
    info.cdang   = cdang;
    info.coppc   = coppc & mask;
    info.cop1lc  = cop1lc & mask;
    info.cop2lc  = cop2lc & mask;
    info.cop1ins = cop1ins;
    info.cop2ins = cop2ins;
    info.length1 = (cop1end - cop1lc) / 4;
    info.length2 = (cop2end - cop2lc) / 4;

    pthread_mutex_unlock(&lock);
}

void
Copper::_dump()
{
    bool active = agnus.isPending<COP_SLOT>();
    msg("    cdang: %d\n", cdang);
    msg("   active: %s\n", active ? "yes" : "no");
    if (active) msg("    state: %d\n", agnus.slot[COP_SLOT].id);
    msg("    coppc: %X\n", coppc);
    msg("  copins1: %X\n", cop1ins);
    msg("  copins2: %X\n", cop2ins);
    msg("   cop1lc: %X\n", cop1lc);
    msg("   cop2lc: %X\n", cop2lc);
    msg("  cop1end: %X\n", cop1end);
    msg("  cop2end: %X\n", cop2end);
}

void
Copper::pokeCOPCON(u16 value)
{
    debug(COPREG_DEBUG, "pokeCOPCON(%04X)\n", value);
    
    /* "This is a 1-bit register that when set true, allows the Copper to
     *  access the blitter hardware. This bit is cleared by power-on reset, so
     *  that the Copper cannot access the blitter hardware." [HRM]
     */
    cdang = (value & 0b10) != 0;
}

template <PokeSource s> void
Copper::pokeCOPJMP1()
{
    debug(COPREG_DEBUG, "pokeCOPJMP1(): Jumping to %X\n", cop1lc);

    if (s == POKE_COPPER) {

        assert(false);

    }
    if (s == POKE_CPU) {

        switchToCopperList(1);
    }
}

template <PokeSource s> void
Copper::pokeCOPJMP2()
{
    debug(COPREG_DEBUG, "pokeCOPJMP2(): Jumping to %X\n", cop2lc);

    if (s == POKE_COPPER) {

        assert(false);

    }
    if (s == POKE_CPU) {

        switchToCopperList(2);
    }
}

void
Copper::pokeCOPINS(u16 value)
{
    debug(COPREG_DEBUG, "COPPC: %X pokeCOPINS(%04X)\n", coppc, value);

    /* COPINS is a dummy address that can be used to write the first or
     * the second instruction register, depending on the current state.
     */

    // TODO: The following is certainly wrong...
    /* if (state == COP_MOVE || state == COP_WAIT_OR_SKIP) {
        cop2ins = value;
    } else {
        cop1ins = value;
    }
    */
    cop1ins = value;
}

void
Copper::pokeCOP1LCH(u16 value)
{
    debug(COPREG_DEBUG, "pokeCOP1LCH(%04X)\n", value);

    if (HI_WORD(cop1lc) != value) {
        cop1lc = REPLACE_HI_WORD(cop1lc, value);
        cop1end = cop1lc;

        if (!activeInThisFrame) {
            assert(copList == 1);
            coppc = cop1lc;
        }
    }
}

void
Copper::pokeCOP1LCL(u16 value)
{
    debug(COPREG_DEBUG, "pokeCOP1LCL(%04X)\n", value);

    value &= 0xFFFE;

    if (LO_WORD(cop1lc) != value) {
        cop1lc = REPLACE_LO_WORD(cop1lc, value);
        cop1end = cop1lc;
        
        if (!activeInThisFrame) {
            assert(copList == 1);
            coppc = cop1lc;
        }
    }
}

void
Copper::pokeCOP2LCH(u16 value)
{
    debug(COPREG_DEBUG, "pokeCOP2LCH(%04X)\n", value);

    if (HI_WORD(cop2lc) != value) {
        cop2lc = REPLACE_HI_WORD(cop2lc, value);
        cop2end = cop2lc;
    }
}

void
Copper::pokeCOP2LCL(u16 value)
{
    debug(COPREG_DEBUG, "pokeCOP2LCL(%04X)\n", value);

    value &= 0xFFFE;

    if (LO_WORD(cop2lc) != value) {
        cop2lc = REPLACE_LO_WORD(cop2lc, value);
        cop2end = cop2lc;
    }
}

void
Copper::pokeNOOP(u16 value)
{
    debug(COPREG_DEBUG, "pokeNOOP(%04X)\n", value);
}

void
Copper::advancePC()
{
    coppc += 2;
}

void
Copper::switchToCopperList(int nr)
{
    assert(nr == 1 || nr == 2);

    // debug("switchToCopperList(%d) coppc: %x -> %x\n", nr, coppc, (nr == 1) ? cop1lc : cop2lc);
    coppc = (nr == 1) ? cop1lc : cop2lc;
    copList = nr;
    agnus.scheduleRel<COP_SLOT>(0, COP_REQ_DMA);
}

bool
Copper::findMatch(Beam &result)
{
    i16 vMatch, hMatch;

    // Get the current beam position
    Beam b = agnus.pos;

    // Set up the comparison positions
    i16 vComp = getVP();
    i16 hComp = getHP();

    // Set up the comparison masks
    i16 vMask = getVM() | 0x80;
    i16 hMask = getHM() & 0xFE;

    // Check if the current line is already below the vertical trigger position
    if ((b.v & vMask) > (vComp & vMask)) {

        // Success. The current position already matches
        result = b;
        return true;
    }

    // Check if the current line matches the vertical trigger position
    if ((b.v & vMask) == (vComp & vMask)) {

        // Check if we find a horizontal match in this line
        if (findHorizontalMatch(b.h, hComp, hMask, hMatch)) {

            // Success. We've found a match in the current line
            result.v = b.v;
            result.h = hMatch;
            return true;
        }
    }

    // Find the first vertical match below the current line
    if (!findVerticalMatch(b.v + 1, vComp, vMask, vMatch)) return false;

    // Find the first horizontal match in that line
    if (!findHorizontalMatch(0, hComp, hMask, hMatch)) return false;

    // Success. We've found a match below the current line
    result.v = vMatch;
    result.h = hMatch;
    return true;
}

bool
Copper::findVerticalMatch(i16 vStrt, i16 vComp, i16 vMask, i16 &result)
{
    i16 vStop = agnus.frame.numLines();

    // Iterate through all vertical positions
    for (int v = vStrt; v < vStop; v++) {

        // Check if the comparator triggers at this position
        if ((v & vMask) >= (vComp & vMask)) {
            result = v;
            return true;
        }
    }
    return false;
}

bool
Copper::findHorizontalMatch(i16 hStrt, i16 hComp, i16 hMask, i16 &result)
{
    i16 hStop = HPOS_CNT;

    debug("findHorizontalMatch(%X,%X,%X)\n", hStrt, hComp, hMask);

    // Iterate through all horizontal positions
    for (int h = hStrt; h < hStop; h++) {

        // Check if the comparator triggers at this position
        if ((h & hMask) >= (hComp & hMask)) {
            result = h;
            return true;
        }
    }
    return false;
}

bool
Copper::findMatchNew(Beam &match)
{
    // Start searching at the current beam position
    u32 beam = (agnus.pos.v << 8) | agnus.pos.h;

    // Get the comparison position and the comparison mask
    u32 comp = getVPHP();
    u32 mask = getVMHM();

    // Iterate through all lines starting from the current position
    int numLines = agnus.frame.numLines();
    while ((beam >> 8) < numLines) {

        // Check if the vertical components are equal
        if ((beam & mask & ~0xFF) == (comp & mask & ~0xFF)) {

            // debug("Matching vertically: beam = %X comp = %X mask = %X\n", beam, comp, mask);

            // Try to match the horizontal coordinate as well
            if (findHorizontalMatchNew(beam, comp, mask)) {

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
Copper::findHorizontalMatchNew(u32 &match, u32 comp, u32 mask)
{
    i16 hStop = HPOS_CNT;

    // Iterate through all horizontal positions
    for (u32 beam = match; (beam & 0xFF) < hStop; beam++) {

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
    debug(COP_DEBUG,
          "COPPC: %X move(%s, $%X) (%d)\n", coppc, regName(addr), value, value);

    assert(IS_EVEN(addr));
    assert(addr < 0x1FF);

    // Catch registers with special timing needs

    if (addr >= 0x180 && addr <= 0x1BE) {

        plaindebug(BLTTIM_DEBUG, "(%d,%d) COLOR%02d\n", agnus.pos.v, agnus.pos.h, addr - 0x180);

        // Color registers
        pixelEngine.colChanges.insert(4 * agnus.pos.h, RegChange { addr, value} );
        return;
    }

    // Write the value
    agnus.doCopperDMA(addr, value);
}

#if 0
bool
Copper::comparator(u32 beam, u32 waitpos, u32 mask)
{
    // Get comparison bits for the vertical beam position
    u8 vBeam = (beam >> 8) & 0xFF;
    u8 vWaitpos = (waitpos >> 8) & 0xFF;
    u8 vMask = (mask >> 8) | 0x80;
    
    if (verbose) debug(" * vBeam = %X vWaitpos = %X vMask = %X\n", vBeam, vWaitpos, vMask);

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
    u8 hBeam = beam & 0xFE;
    u8 hWaitpos = waitpos & 0xFE;
    u8 hMask = mask & 0xFE;

    if (verbose) debug(" * hBeam = %X hWaitpos = %X hMask = %X\n", hBeam, hWaitpos, hMask);
    /*
    debug("Comparing horizontal position waitpos = %d vWait = %d hWait = %d \n", waitpos, vWaitpos, hWaitpos);
    debug("hBeam = %d ($x) hMask = %X\n", hBeam, hBeam, hMask);
    debug("Result = %d\n", (hBeam & hMask) >= (hWaitpos & hMask));
    */

    // Compare horizontal positions
    return (hBeam & hMask) >= (hWaitpos & hMask);
}
#endif

bool
Copper::comparator(Beam beam, u16 waitpos, u16 mask)
{
    // Get comparison bits for the vertical beam position
    u8 vBeam = beam.v & 0xFF;
    u8 vWaitpos = HI_BYTE(waitpos);
    u8 vMask = HI_BYTE(mask) | 0x80;

    if (verbose) debug(" * vBeam = %X vWaitpos = %X vMask = %X\n", vBeam, vWaitpos, vMask);

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

    if (verbose) debug(" * hBeam = %X hWaitpos = %X hMask = %X\n", hBeam, hWaitpos, hMask);
    /*
     debug("Comparing horizontal position waitpos = %d vWait = %d hWait = %d \n", waitpos, vWaitpos, hWaitpos);
     debug("hBeam = %d ($x) hMask = %X\n", hBeam, hBeam, hMask);
     debug("Result = %d\n", (hBeam & hMask) >= (hWaitpos & hMask));
     */

    // Compare horizontal positions
    return (hBeam & hMask) >= (hWaitpos & hMask);
}

bool
Copper::comparator(Beam beam)
{
    return comparator(beam, getVPHP(), getVMHM());
}

bool
Copper::comparator()
{
    return comparator(agnus.pos);
}

void
Copper::scheduleWaitWakeup()
{
    Beam trigger;

    // Find the trigger position for this WAIT command
    if (findMatchNew(trigger)) {

        // In how many cycles do we get there?
        int delay = trigger - agnus.pos;

        if (verbose) debug("(%d,%d) matches in %d cycles\n", trigger.v, trigger.h, delay);

        if (delay == 0) {

            // Copper does not stop
            agnus.scheduleRel<COP_SLOT>(DMA_CYCLES(2), COP_FETCH);

        } else if (delay == 2) {

            // Copper does not stop
            agnus.scheduleRel<COP_SLOT>(DMA_CYCLES(2), COP_FETCH);

        } else {

            // Wake up 2 cycles earlier with a COP_REQ_DMA event
            delay -= 2;
            agnus.scheduleRel<COP_SLOT>(DMA_CYCLES(delay), COP_REQ_DMA);
        }

    } else {

        if (verbose) debug("(%d,%d) does not match in this frame\n", trigger.v, trigger.h);
        agnus.scheduleAbs<COP_SLOT>(NEVER, COP_REQ_DMA);
    }
}

bool
Copper::isMoveCmd()
{
    return !(cop1ins & 1);
}

bool Copper::isMoveCmd(u32 addr)
{
    assert(IS_EVEN(addr));

    u16 hiword = mem.spypeekChip16(addr);

    return IS_EVEN(hiword);
}

bool Copper::isWaitCmd()
{
     return (cop1ins & 1) && !(cop2ins & 1);
}

bool Copper::isWaitCmd(u32 addr)
{
    assert(IS_EVEN(addr));

    u16 hiword = mem.spypeekChip16(addr);
    u16 loword = mem.spypeekChip16(addr + 2);

    return IS_ODD(hiword) && IS_EVEN(loword);
}

bool
Copper::isSkipCmd()
{
    return (cop1ins & 1) && (cop2ins & 1);
}

bool
Copper::isSkipCmd(u32 addr)
{
    assert(IS_EVEN(addr));

    u16 hiword = mem.spypeekChip16(addr);
    u16 loword = mem.spypeekChip16(addr + 2);

    return IS_ODD(hiword) && IS_ODD(loword);
}

u16
Copper::getRA()
{
    return cop1ins & 0x1FE;
}

u16
Copper::getRA(u32 addr)
{
    u16 hiword = mem.spypeekChip16(addr);
    return hiword & 0x1FE;
}

u16
Copper::getDW()
{
    return cop1ins;
}

u16
Copper::getDW(u32 addr)
{
    u16 loword = mem.spypeekChip16(addr + 2);
    return loword;
}

bool
Copper::getBFD()
{
    return (cop2ins & 0x8000) != 0;
}

bool
Copper::getBFD(u32 addr)
{
    u32 instr = mem.spypeek32(addr);
    return (LO_WORD(instr) & 0x8000) != 0;
}

u16
Copper::getVPHP()
{
    return cop1ins & 0xFFFE;
}

u16
Copper::getVPHP(u32 addr)
{
    u32 instr = mem.spypeek32(addr);
    return HI_WORD(instr) & 0xFFFE;
}

u16
Copper::getVMHM()
{
    return (cop2ins & 0x7FFE) | 0x8001;
}

u16
Copper::getVMHM(u32 addr)
{
    u32 instr = mem.spypeek32(addr);
    return (LO_WORD(instr) & 0x7FFE) | 0x8001;
}

bool
Copper::isIllegalAddress(u32 addr)
{
    if (cdang) {
        return agnus.isOCS() ? addr < 0x40 : false;
    } else {
        return addr < 0x80;
    }
}

bool
Copper::isIllegalInstr(u32 addr)
{
    return isMoveCmd(addr) && isIllegalAddress(getRA(addr));
}

void
Copper::serviceEvent(EventID id)
{
    u16 reg;
    Beam beam;

    servicing = true;

    switch (id) {
            
        case COP_REQ_DMA:

            if (verbose) debug("COP_REQ_DMA\n");
            
            // Check if we need to wait for the Blitter
            if (!getBFD() && agnus.blitter.isRunning()) {
                agnus.scheduleAbs<COP_SLOT>(NEVER, COP_WAIT_BLIT);
                break;
            }

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Don't wake up in an odd cycle
            if (agnus.pos.h % 2) { reschedule(); break; }

            // Continue with fetching the first instruction word
            schedule(COP_FETCH);
            break;

        case COP_FETCH:

            if (verbose) debug("COP_FETCH\n");

            // Check if we need to wait for the Blitter
            if (!getBFD() && agnus.blitter.isRunning()) {
                agnus.scheduleAbs<COP_SLOT>(NEVER, COP_WAIT_BLIT);
                break;
            }

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Load the first instruction word
            cop1ins = agnus.doCopperDMA(coppc);
            advancePC();

            if (COP_CHECKSUM) {
                checkcnt++;
                checksum = fnv_1a_it32(checksum, cop1ins);
            }

            // Dynamically determine the end of the Copper list
            if (copList == 1) {
                if (coppc > cop1end) cop1end = coppc;
            } else {
                if (coppc > cop2end) cop2end = coppc;
            }

            // Fork execution depending on the instruction type
            schedule(isMoveCmd() ? COP_MOVE : COP_WAIT_OR_SKIP);
            break;
            
        case COP_MOVE:

            if (verbose) debug("COP_MOVE\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Load the second instruction word
            cop2ins = agnus.doCopperDMA(coppc);
            advancePC();

            // Extract register number from the first instruction word
            reg = (cop1ins & 0x1FE);

            // Stop the Copper if address is illegal
            if (isIllegalAddress(reg)) { agnus.cancel<COP_SLOT>(); break; }

            // Continue with fetching the new command
            schedule(COP_FETCH);

            // Only proceed if the skip flag is not set
            if (skip) { skip = false; break; }

            // Write value into custom register
            switch (reg) {
                case 0x88:
                    schedule(COP_JMP1);
                    agnus.slot[COP_SLOT].data = 1;
                    break;
                case 0x8A:
                    schedule(COP_JMP1);
                    agnus.slot[COP_SLOT].data = 2;
                    break;
                default:
                    move(reg, cop2ins);
            }
            break;
            
        case COP_WAIT_OR_SKIP:

            if (verbose) debug("COP_WAIT_OR_SKIP\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Load the second instruction word
            cop2ins = agnus.doCopperDMA(coppc);
            advancePC();

            // Fork execution depending on the instruction type
            schedule(isWaitCmd() ? COP_WAIT1 : COP_SKIP1);
            break;

        case COP_WAIT1:
            
            if (verbose) debug("COP_WAIT1\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Schedule next state
            schedule(COP_WAIT2);
            break;

        case COP_WAIT2:

            if (verbose) debug("COP_WAIT2\n");

            // Clear the skip flag
            skip = false;

            // Remember the Blitter Finish Disable bit
            bfd = getBFD();
            
            // Check if we need to wait for the Blitter
            if (!getBFD() && agnus.blitter.isRunning()) {
                agnus.scheduleAbs<COP_SLOT>(NEVER, COP_WAIT_BLIT);
                break;
            }
            
            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Test 'coptim3' suggests that cycle $E1 is blocked in this state
            if (agnus.pos.h == 0xE1) { reschedule(); break; }

            // Schedule a wakeup event at the target position
            scheduleWaitWakeup();
            break;

        case COP_WAIT_BLIT:
            
            if (verbose) debug("COP_WAIT_BLIT\n");

            bfd = true;
            
            // Wait for the next free cycle
            if (agnus.busOwner[agnus.pos.h] != BUS_NONE &&
                agnus.busOwner[agnus.pos.h] != BUS_BLITTER) {
                // debug("COP_WAIT_BLIT delay\n");
                reschedule(); break;
            }
            
            // Schedule a wakeup event at the target position
            scheduleWaitWakeup();
            break;

        case COP_SKIP1:

            if (verbose) debug("COP_SKIP1\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Schedule next state
            schedule(COP_SKIP2);
            break;

        case COP_SKIP2:

            if (verbose) debug("COP_SKIP2\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Test 'coptim3' suggests that cycle $E1 is blocked in this state
            if (agnus.pos.h == 0xE1) { reschedule(); break; }

            // Compute the beam position that needs to be compared
            beam = agnus.addToBeam(agnus.pos, 2);

            // Run the comparator to see if the next command is skipped
            if (verbose) debug("Running comparator with (%d,%d)\n", beam.v, beam.h);
            skip = comparator(beam);

            // Continue with the next command
            schedule(COP_FETCH);
            break;

        case COP_JMP1:

            if (verbose) debug("COP_JMP1\n");

            // The bus is not needed in this cycle, but still allocated
            (void)agnus.allocateBus<BUS_COPPER>();

            // In cycle $E0, Copper continues with the next state in $E1 (?!)
            if (agnus.pos.h == 0xE0) {
                schedule(COP_JMP2, 1);
                break;
            }

            schedule(COP_JMP2);
            break;

        case COP_JMP2:

            if (verbose) debug("COP_JMP2\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            switchToCopperList(agnus.slot[COP_SLOT].data);
            schedule(COP_FETCH);
            break;

        case COP_VBLANK:

            if (verbose) debug("COP_VBLANK\n");

            // Allocate the bus
            // TODO: FIND OUT IF THE BUS IS REALLY ALLOCATED IN THIS STATE
            if (!agnus.allocateBus<BUS_COPPER>()) { reschedule(); break; }

            switchToCopperList(1);
            schedule(COP_FETCH);

            break;

        default:
            
            assert(false);
            break;
    }

    servicing = false;
}

void
Copper::schedule(EventID next, int delay)
{
    agnus.scheduleRel<COP_SLOT>(DMA_CYCLES(delay), next);
}

void
Copper::reschedule(int delay)
{
    agnus.rescheduleRel<COP_SLOT>(DMA_CYCLES(delay));
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
    agnus.scheduleRel<COP_SLOT>(DMA_CYCLES(0), COP_VBLANK);
    activeInThisFrame = agnus.copdma();
    
    if (COP_CHECKSUM) {

        if (checkcnt) debug("Checksum: %x (%d)\n", checksum, checkcnt);

        checkcnt = 0;
        checksum = fnv_1a_init32();
    }
}

void
Copper::blitterDidTerminate()
{
    if (agnus.hasEvent<COP_SLOT>(COP_WAIT_BLIT)) {

        // Wake up the Copper in the next even cycle
        if (IS_EVEN(agnus.pos.h)) {
            serviceEvent(COP_WAIT_BLIT);
        } else {
            agnus.scheduleRel<COP_SLOT>(DMA_CYCLES(1), COP_WAIT_BLIT);
        }
    }
}

int
Copper::instrCount(int nr)
{
    assert(nr == 1 || nr == 2);

    int strt = (nr == 1) ? cop1lc  : cop2lc;
    int stop = (nr == 1) ? cop1end : cop2end;

    return MAX(0, 1 + (stop - strt) / 4);
}

void
Copper::adjustInstrCount(int nr, int offset)
{
    assert(nr == 1 || nr == 2);

    if (nr == 1) {
        if (cop1end + offset >= cop1lc) cop1end += offset;
    } else {
        if (cop2end + offset >= cop2lc) cop2end += offset;
    }
    inspect();
}

char *
Copper::disassemble(u32 addr)
{
    char pos[16];
    char mask[16];
    
    if (isMoveCmd(addr)) {
        
        sprintf(disassembly, "MOVE $%04X, %s", getDW(addr), regName(getRA(addr)));
        return disassembly;
    }
    
    const char *mnemonic = isWaitCmd(addr) ? "WAIT" : "SKIP";
    const char *suffix = getBFD(addr) ? "" : "b";
    
    sprintf(pos, "($%02X,$%02X)", getVP(addr), getHP(addr));
    
    if (getVM(addr) == 0xFF && getHM(addr) == 0xFF) {
        sprintf(mask, "");
    } else {
        sprintf(mask, ", ($%02X,$%02X)", getHM(addr), getVM(addr));
    }
    
    sprintf(disassembly, "%s%s %s%s", mnemonic, suffix, pos, mask);
    return disassembly;
}

char *
Copper::disassemble(unsigned list, u32 offset)
{
    assert(list == 1 || list == 2);
    
    u32 addr = (list == 1 ? cop1lc : cop2lc) + 2 * offset;
    return disassemble(addr);
}

void
Copper::dumpCopperList(unsigned list, unsigned length)
{
    for (unsigned i = 0; i < length; i++) {
        printf("%s\n", disassemble(list, 2*i));
    }
}

template void Copper::pokeCOPJMP1<POKE_CPU>();
template void Copper::pokeCOPJMP1<POKE_COPPER>();
template void Copper::pokeCOPJMP2<POKE_CPU>();
template void Copper::pokeCOPJMP2<POKE_COPPER>();
