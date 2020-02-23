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
Copper::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    info.cdang   = cdang;
    info.active  = agnus.isPending<COP_SLOT>();
    info.coppc   = coppc; // coppcBase;
    info.cop1ins = cop1ins;
    info.cop2ins = cop2ins;
    info.cop1lc  = cop1lc;
    info.cop2lc  = cop2lc;
    info.length1 = cop1end - cop1lc;
    info.length2 = cop2end - cop2lc;

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

CopperInfo
Copper::getInfo()
{
    CopperInfo result;
    
    pthread_mutex_lock(&lock);
    result = info;
    pthread_mutex_unlock(&lock);
    
    return result;
}

void
Copper::pokeCOPCON(uint16_t value)
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

        // Continue Copper execution if it was waiting
        /*
         if (agnus.hasEvent<COP_SLOT>(COP_REQ_DMA)) {
         agnus.scheduleImm<COP_SLOT>(COP_REQ_DMA);
         }
         */
    }
}

void
Copper::pokeCOPINS(uint16_t value)
{
    debug(COPREG_DEBUG, "COPPC: %X pokeCOPINS(%04X)\n", coppc, value);

    /* COPINS is a dummy address that can be used to write the first or
     * the secons instruction register, depending on the current state.
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
Copper::pokeCOP1LCH(uint16_t value)
{
    debug(COPREG_DEBUG, "pokeCOP1LCH(%04X)\n", value);

    if (HI_WORD(cop1lc) != value) {
        cop1lc = CHIP_PTR(REPLACE_HI_WORD(cop1lc, value));
    }
}

void
Copper::pokeCOP1LCL(uint16_t value)
{
    debug(COPREG_DEBUG, "pokeCOP1LCL(%04X)\n", value);

    if (LO_WORD(cop1lc) != value) {
        cop1lc = REPLACE_LO_WORD(cop1lc, value & 0xFFFE);
    }
}

void
Copper::pokeCOP2LCH(uint16_t value)
{
    debug(COPREG_DEBUG, "pokeCOP2LCH(%04X)\n", value);

    if (HI_WORD(cop2lc) != value) {
        cop2lc = CHIP_PTR(REPLACE_HI_WORD(cop2lc, value));
    }
}

void
Copper::pokeCOP2LCL(uint16_t value)
{
    debug(COPREG_DEBUG, "pokeCOP2LCL(%04X)\n", value);

    if (LO_WORD(cop2lc) != value) {
        cop2lc = REPLACE_LO_WORD(cop2lc, value & 0xFFFE);
    }
}

void
Copper::pokeNOOP(uint16_t value)
{
    debug(COPREG_DEBUG, "pokeNOOP(%04X)\n", value);
}

void
Copper::advancePC()
{
    INC_CHIP_PTR_BY(coppc, 2);
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
    int16_t vMatch, hMatch;

    // Get the current beam position
    Beam b = agnus.pos;

    // Set up the comparison positions
    int16_t vComp = getVP();
    int16_t hComp = getHP();

    // Set up the comparison masks
    int16_t vMask = getVM() | 0x80;
    int16_t hMask = getHM() & 0xFE;

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
Copper::findVerticalMatch(int16_t vStrt, int16_t vComp, int16_t vMask, int16_t &result)
{
    int16_t vStop = agnus.frameInfo.numLines;

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
Copper::findHorizontalMatch(int16_t hStrt, int16_t hComp, int16_t hMask, int16_t &result)
{
    int16_t hStop = HPOS_CNT;

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
    uint32_t beam = (agnus.pos.v << 8) | agnus.pos.h;

    // Get the comparison position and the comparison mask
    uint32_t comp = getVPHP();
    uint32_t mask = getVMHM();

    // Iterate through all lines starting from the current position
    while ((beam >> 8) < agnus.frameInfo.numLines) {

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
Copper::findHorizontalMatchNew(uint32_t &match, uint32_t comp, uint32_t mask)
{
    int16_t hStop = HPOS_CNT;

    // Iterate through all horizontal positions
    for (uint32_t beam = match; (beam & 0xFF) < hStop; beam++) {

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
Copper::move(int addr, uint16_t value)
{
    debug(COP_DEBUG, "COPPC: %X move(%s, $%X) (%d)\n", coppc, customReg[addr >> 1], value, value);

    assert(IS_EVEN(addr));
    assert(addr < 0x1FF);

    // Catch registers with special timing needs

    if (addr >= 0x180 && addr <= 0x1BE) {

        plaindebug(BLTTIM_DEBUG, "(%d,%d) COLOR%02d\n", agnus.pos.v, agnus.pos.h, addr - 0x180);

        // Color registers
        pixelEngine.colRegChanges.add(4 * agnus.pos.h, addr, value);
        return;
    }

    // Write the value
    agnus.copperWrite(addr, value);
}

#if 0
bool
Copper::comparator(uint32_t beam, uint32_t waitpos, uint32_t mask)
{
    // Get comparison bits for the vertical beam position
    uint8_t vBeam = (beam >> 8) & 0xFF;
    uint8_t vWaitpos = (waitpos >> 8) & 0xFF;
    uint8_t vMask = (mask >> 8) | 0x80;
    
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
    uint8_t hBeam = beam & 0xFE;
    uint8_t hWaitpos = waitpos & 0xFE;
    uint8_t hMask = mask & 0xFE;

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
Copper::comparator(Beam beam, uint16_t waitpos, uint16_t mask)
{
    // Get comparison bits for the vertical beam position
    uint8_t vBeam = beam.v & 0xFF;
    uint8_t vWaitpos = HI_BYTE(waitpos);
    uint8_t vMask = HI_BYTE(mask) | 0x80;

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
    uint8_t hBeam = beam.h & 0xFE;
    uint8_t hWaitpos = LO_BYTE(waitpos) & 0xFE;
    uint8_t hMask = LO_BYTE(mask) & 0xFE;

    if (verbose) debug(" * hBeam = %X hWaitpos = %X hMask = %X\n", hBeam, hWaitpos, hMask);
    /*
     debug("Comparing horizontal position waitpos = %d vWait = %d hWait = %d \n", waitpos, vWaitpos, hWaitpos);
     debug("hBeam = %d ($x) hMask = %X\n", hBeam, hBeam, hMask);
     debug("Result = %d\n", (hBeam & hMask) >= (hWaitpos & hMask));
     */

    // Compare horizontal positions
    return (hBeam & hMask) >= (hWaitpos & hMask);
}

/*
bool
Copper::comparator(uint32_t beam)
{
    return comparator(beam, getVPHP(), getVMHM());
}
*/

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
        // agnus.cancel<COP_SLOT>();
    }
}

bool
Copper::isMoveCmd()
{
    return !(cop1ins & 1);
}

bool Copper::isMoveCmd(uint32_t addr)
{
    uint32_t instr = mem.spypeek32(addr);
    return !(HI_WORD(instr) & 1);
}

bool Copper::isWaitCmd()
{
     return (cop1ins & 1) && !(cop2ins & 1);
}

bool Copper::isWaitCmd(uint32_t addr)
{
    uint32_t instr = mem.spypeek32(addr);
    return (HI_WORD(instr) & 1) && !(LO_WORD(instr) & 1);
}

bool
Copper::isSkipCmd()
{
    return (cop1ins & 1) && (cop2ins & 1);
}

bool
Copper::isSkipCmd(uint32_t addr)
{
    uint32_t instr = mem.spypeek32(addr);
    return (HI_WORD(instr) & 1) && (LO_WORD(instr) & 1);
}

uint16_t
Copper::getRA()
{
    return cop1ins & 0x1FE;
}

uint16_t
Copper::getRA(uint32_t addr)
{
    uint32_t instr = mem.spypeek32(addr);
    return HI_WORD(instr) & 0x1FE;
}

uint16_t
Copper::getDW()
{
    return cop1ins;
}

uint16_t
Copper::getDW(uint32_t addr)
{
    uint32_t instr = mem.spypeek32(addr);
    return LO_WORD(instr);
}

bool
Copper::getBFD()
{
    return (cop2ins & 0x8000) != 0;
}

bool
Copper::getBFD(uint32_t addr)
{
    uint32_t instr = mem.spypeek32(addr);
    return (LO_WORD(instr) & 0x8000) != 0;
}

uint16_t
Copper::getVPHP()
{
    return cop1ins & 0xFFFE;
}

uint16_t
Copper::getVPHP(uint32_t addr)
{
    uint32_t instr = mem.spypeek32(addr);
    return HI_WORD(instr) & 0xFFFE;
}

uint16_t
Copper::getVMHM()
{
    return (cop2ins & 0x7FFE) | 0x8001;
}

uint16_t
Copper::getVMHM(uint32_t addr)
{
    uint32_t instr = mem.spypeek32(addr);
    return (LO_WORD(instr) & 0x7FFE) | 0x8001;
}

bool
Copper::isIllegalAddress(uint32_t addr)
{
    if (cdang) {
        return agnus.isOCS() ? addr < 0x40 : false;
    } else {
        return addr < 0x80;
    }
}

bool
Copper::isIllegalInstr(uint32_t addr)
{
    return isMoveCmd(addr) && isIllegalAddress(getRA(addr));
}

void
Copper::serviceEvent(EventID id)
{
    uint16_t reg;
    Beam beam;

    servicing = true;

    switch (id) {
            
        case COP_REQ_DMA:

            if (verbose) debug("COP_REQ_DMA\n");

            // Wait for the next free cycle suitable for DMA
            if (!agnus.copperCanDoDMA()) { reschedule(); break; }

            // Don't wake up in an odd cycle
            if (agnus.pos.h % 2) { reschedule(); break; }

            // Continue with fetching the first instruction word
            schedule(COP_FETCH);
            break;

        case COP_FETCH:

            if (verbose) debug("COP_FETCH\n");

            // Wait for the next free cycle suitable for DMA
            if (!agnus.copperCanDoDMA()) { reschedule(); break; }

            // Load the first instruction word
            cop1ins = agnus.copperRead(coppc);
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

            // Wait for the next free cycle suitable for DMA
            if (!agnus.copperCanDoDMA()) { reschedule(); break; }

            // Load the second instruction word
            cop2ins = agnus.copperRead(coppc);
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

            // Wait for the next free cycle suitable for DMA
            if (!agnus.copperCanDoDMA()) { reschedule(); break; }

            // Load the second instruction word
            cop2ins = agnus.copperRead(coppc);
            advancePC();

            // Fork execution depending on the instruction type
            schedule(isWaitCmd() ? COP_WAIT1 : COP_SKIP1);
            break;

        case COP_WAIT1:
            
            // verbose = true;
            if (verbose) debug("COP_WAIT1\n");

            // Wait for the next free cycle
            if (!agnus.copperCanRun()) { reschedule(); break; }

            // Schedule next state
            schedule(COP_WAIT2);
            break;

        case COP_WAIT2:

            if (verbose) debug("COP_WAIT2\n");

            // Clear the skip flag
            skip = false;

            // Check the Blitte Finish Disable bit
            if (!getBFD()) {
                if (agnus.blitter.isRunning()) {
                    agnus.scheduleAbs<COP_SLOT>(NEVER, COP_WAIT_BLIT);
                    break;
                }
            }

            // Wait for the next free cycle
            if (!agnus.copperCanRun()) { reschedule(); break; }

            // Schedule a wakeup event at the target position
            scheduleWaitWakeup();
            break;

        case COP_WAIT_BLIT:

            // debug("COP_WAIT_BLIT\n");

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

            // Wait for the next free cycle
            if (!agnus.copperCanRun()) { reschedule(); break; }

            // Schedule next state
            schedule(COP_SKIP2);
            break;

        case COP_SKIP2:

            if (verbose) debug("COP_SKIP2\n");

            // Wait for the next free DMA cycle
            if (!agnus.copperCanRun()) { reschedule(); break; }

            // vAmigaTS::copskip2 indicates that this state already blocks at 0xE0
            if (agnus.pos.h == 0xE0) { reschedule(); break; }

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

            // Wait for the next free DMA cycle
            if (!agnus.copperCanDoDMA()) {
                reschedule();
                break;
            }

            switchToCopperList(agnus.slot[COP_SLOT].data);
            schedule(COP_FETCH);
            break;

        case COP_VBLANK:

            // verbose = true;
            if (verbose) debug("COP_VBLANK\n");

            // Wait until the bus is free
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

        // debug("Blitter did terminate\n");

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

    return 1 + (stop - strt) / 4;
}


char *
Copper::disassemble(uint32_t addr)
{
    char pos[16];
    char mask[16];
    
    if (isMoveCmd(addr)) {
        
        uint16_t reg = getRA(addr) >> 1;
        assert(reg <= 0xFF);
        sprintf(disassembly, "MOVE $%04X, %s", getDW(addr), customReg[reg]);
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
Copper::disassemble(unsigned list, uint32_t offset)
{
    assert(list == 1 || list == 2);
    
    uint32_t addr = (list == 1) ? cop1lc : cop2lc;
    addr = CHIP_PTR(addr + 2 * offset);

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
