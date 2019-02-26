// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Copper::Copper()
{
    setDescription("Copper");
        
    registerSnapshotItems(vector<SnapshotItem> {
        
        { &state,    sizeof(state),    0 },
        { &coplc,    sizeof(coplc),    DWORD_ARRAY },
        { &cdang,    sizeof(cdang),    0 },
        { &copins1,  sizeof(copins1),  0 },
        { &copins2,  sizeof(copins2),  0 },
        { &coppc,    sizeof(coppc),    0 },
    });
}

CopperInfo
Copper::getInfo()
{
    CopperInfo info;
    
    /* Note: We call the Copper 'active' if there is a pending message in the
     * Copper event slot.
     */
    
    info.cdang     = cdang;
    info.active    = amiga->dma.eventHandler.isPending(COPPER_SLOT);
    info.coppc     = coppc;
    info.copins[0] = copins1;
    info.copins[1] = copins2;
    info.coplc[0]  = coplc[0];
    info.coplc[1]  = coplc[1];

    return info;
}

bool
Copper::illegalAddress(uint32_t address)
{
    address &= 0x1FE;
    return address < (cdang ? 0x40 : 0x80);
}

void
Copper::_powerOn()
{
    
}

void
Copper::_powerOff()
{
    
}

void
Copper::_reset()
{
    
}

void
Copper::_ping()
{
    
}

void
Copper::_dump()
{
    plainmsg("   cdang: %lld\n", cdang);
}

void
Copper::pokeCOPCON(uint16_t value)
{
    debug("pokeCOPCON(%X)\n", value);
    
    /* "This is a 1-bit register that when set true, allows the Copper to
     *  access the blitter hardware. This bit is cleared by power-on reset, so
     *  that the Copper cannot access the blitter hardware." [HRM]
     */
    cdang = (value & 0b10) != 0;
}

void
Copper::pokeCOPJMP(int x)
{
    assert(x < 2);
    
    debug("pokeCOPJMP%d\n", x);
    
    /* "When you write to a Copper strobe address, the Copper reloads its
     *  program counter from the corresponding location register." [HRM]
     */
    // coppc = coplc[1];
}

void
Copper::pokeCOPINS(uint16_t value)
{
    /* COPINS is a dummy address that can be used to write the first or
     * the secons instruction register, depending on the current state.
     */

    // TODO: The following is almost certainly wrong...
    if (state == COPPER_MOVE || state == COPPER_WAIT_OR_SKIP) {
        copins2 = value;
    } else {
        copins1 = value;
    }
}

void
Copper::pokeCOPxLCH(int x, uint16_t value)
{
    assert(x < 2);
    
    debug("pokeCOP%dLCH(%X)\n", x, value);
    coplc[x] = REPLACE_HI_WORD(coplc[x], value);
}

void
Copper::pokeCOPxLCL(int x, uint16_t value)
{
    assert(x < 2);
    
    debug("pokeCOP%dLCL(%X)\n", x, value);
    coplc[x] = REPLACE_LO_WORD(coplc[x], value & 0xFFFE);
}

bool
Copper::comparator(uint32_t beam, uint32_t waitpos, uint32_t mask)
{
    // Get comparison bits for vertical position
    uint8_t vBeam = (beam >> 8) & 0xFF;
    uint8_t vWaitpos = (waitpos >> 8) & 0xFF;
    uint8_t vMask = (mask >> 8) & 0x7F;
    
    // Compare vertical positions
    if ((vBeam & vMask) < (vWaitpos & vMask))
        return false;

    if ((vBeam & vMask) > (vWaitpos & vMask))
        return true;

    // Get comparison bits for horizontal position
    uint8_t hBeam = beam & 0xFE;
    uint8_t hWaitpos = waitpos & 0xFE;
    uint8_t hMask = mask & 0xFE;
    
    // Compare horizontal positions
    return (hBeam & hMask) >= (hWaitpos & hMask);
}

bool
Copper::comparator(uint32_t waitpos)
{
    return comparator(amiga->dma.beam, waitpos, getVMHM());
}

bool
Copper::comparator()
{
    return comparator(getVPHP());
}

uint32_t
Copper::nextTriggerPosition()
{
    // Get the current beam position
    uint32_t beam = amiga->dma.beam;

    /* We are going to compute the smallest beam position satisfying
     *
     *   1) computed position >= current beam position,
     *   2) the comparator circuit triggers.
     *
     * We do this by starting with the maximum possible value:
     */
    uint32_t pos = 0x1FFE2;
    
    /* Now, we iterate through bit from left to right and set the bit to 0.
     * If conditions 1) and 2) still hold, we continue. If not, we have
     * already found the smalles value and return
     */
    for (int i = 16; i >= 0; i--) {
        uint32_t newPos = pos & ~(1 << i);
        if (newPos >= beam && comparator(newPos)) {
            pos = newPos;
        } else {
            break;
        }
    }
    
    return pos;
}

bool
Copper::isMoveCmd()
{
    return !(copins1 & 1);
}

bool Copper::isMoveCmd(uint32_t addr)
{
    uint32_t instr = amiga->mem.spypeek32(addr);
    return !(HI_WORD(instr) & 1);
}

bool Copper::isWaitCmd()
{
     return (copins1 & 1) && !(copins2 & 1);
}

bool Copper::isWaitCmd(uint32_t addr)
{
    uint32_t instr = amiga->mem.spypeek32(addr);
    return (HI_WORD(instr) & 1) && !(LO_WORD(instr) & 1);
}

bool
Copper::isSkipCmd()
{
    return (copins1 & 1) && (copins2 & 1);
}

bool
Copper::isSkipCmd(uint32_t addr)
{
    uint32_t instr = amiga->mem.spypeek32(addr);
    return (HI_WORD(instr) & 1) && (LO_WORD(instr) & 1);
}

uint16_t
Copper::getRA()
{
    return copins1 & 0x1F;
}

uint16_t
Copper::getRA(uint32_t addr)
{
    uint32_t instr = amiga->mem.spypeek32(addr);
    debug("getRA: %X %X %X\n", instr, HI_WORD(instr), LO_WORD(instr));
    return HI_WORD(instr) & 0x1F;
}

uint16_t
Copper::getDW()
{
    return copins1;
}

uint16_t
Copper::getDW(uint32_t addr)
{
    uint32_t instr = amiga->mem.spypeek32(addr);
    return LO_WORD(instr);
}

bool
Copper::getBFD()
{
    return (copins2 & 0x8000) != 0;
}

bool
Copper::getBFD(uint32_t addr)
{
    uint32_t instr = amiga->mem.spypeek32(addr);
    return (LO_WORD(instr) & 0x8000) != 0;
}

uint16_t
Copper::getVPHP()
{
    return copins1 & 0xFFFE;
}

uint16_t
Copper::getVPHP(uint32_t addr)
{
    uint32_t instr = amiga->mem.spypeek32(addr);
    return HI_WORD(instr) & 0xFFFE;
}

uint16_t
Copper::getVMHM()
{
    return (copins2 & 0x7FFE) | 0x8001;
}

uint16_t
Copper::getVMHM(uint32_t addr)
{
    uint32_t instr = amiga->mem.spypeek32(addr);
    return (LO_WORD(instr) & 0x7FFE) | 0x8001;
}

bool
Copper::isIllegalInstr(uint32_t addr)
{
    debug("isMoveCmd(addr) = %d\n", isMoveCmd(addr));
    return isMoveCmd(addr) && illegalAddress(getRA(addr));
}

char *
Copper::disassemble(uint32_t addr)
{
    char vpos[16];
    char hpos[16];
    
    if (isMoveCmd(addr)) {
        
        uint16_t reg = getRA(addr) >> 1;
        assert(reg <= 0xFF);
        sprintf(disassembly, "MOVE %s, %04X", customReg[reg], getDW(addr));
        return disassembly;
    }
    
    const char *mnemonic = isWaitCmd(addr) ? "WAIT" : "SKIP";
    const char *suffix = getBFD() ? "_BFD" : "";
    
    if (getVM(addr) == 0xFF) {
        sprintf(vpos, "%2X", getVP(addr));
    } else {
        sprintf(vpos, "%2X & %2X", getVP(addr), getVM(addr));
    }
    
    if (getHM(addr) == 0xFF) {
        sprintf(hpos, "%2X", getHP(addr));
    } else {
        sprintf(hpos, "%2X & %2X", getHP(addr), getHM(addr));
    }
    
    sprintf(disassembly, "%s%s (%s,%s)", mnemonic, suffix, vpos, hpos);
    return disassembly;
}

char *
Copper::disassemble(unsigned list, uint32_t offset)
{
    assert(list == 1 || list == 2);
    
    uint32_t addr = (list == 1) ? coplc[0] : coplc[1];
    addr = (addr + 2 * offset) & 0x7FFFF;
    
    return disassemble(addr);
}

void
Copper::scheduleEventRel(Cycle delta, int32_t type, int64_t data)
{
    Cycle trigger = amiga->dma.clock + delta;
    amiga->dma.eventHandler.scheduleEvent(COPPER_SLOT, trigger, type, data);
    
    state = type;
}

void
Copper::cancelEvent()
{
    amiga->dma.eventHandler.cancelEvent(COPPER_SLOT);
    state = 0;
}

void
Copper::processEvent(int32_t type, int64_t data)
{
    switch (type) {
            
        case COPPER_REQUEST_DMA:
            
            /* In this state, Copper wait for a free DMA cycle.
             * Once DMA access is granted, it continues with fetching the
             * first instruction word.
             */
            if ( amiga->dma.copperCanHaveBus()) {
                scheduleEventRel(2, COPPER_FETCH);
            }
            
        case COPPER_FETCH:
            
            if (amiga->dma.copperCanHaveBus()) {
                
                // Load the first instruction word
                copins1 = amiga->mem.peek16(coppc);
                advancePC();
                
                // Determine the next state based on the instruction type
                scheduleEventRel(2, isMoveCmd() ? COPPER_MOVE : COPPER_WAIT_OR_SKIP);
            }
            break;
            
        case COPPER_MOVE:
            
            if (amiga->dma.copperCanHaveBus()) {
                
                // Load the second instruction word
                copins2 = amiga->mem.peek16(coppc);
                advancePC();
                
                // Extract register number from the first instruction word
                uint16_t reg = (copins1 & 0x1FE);
                
                if (illegalAddress(reg)) {
                    
                    cancelEvent(); // Stops the Copper
                    break;
                }
                
                // TODO: Skip instruction if prev command was SKIP
                if (1) { // if (!skip) {
                    amiga->mem.pokeCustom16(reg, copins2);
                }
                
                // Schedule next event
                scheduleEventRel(2, COPPER_FETCH);
            }
            break;
            
        case COPPER_WAIT_OR_SKIP:
            
            if (amiga->dma.copperCanHaveBus()) {
                
                // Load the second instruction word
                copins2 = amiga->mem.peek16(coppc);
                advancePC();
                
                // Is it a WAIT command?
                if (isWaitCmd()) {
                    
                    
                }
                
                // It must be a SKIP command then.
                else {
                    assert(isSkipCmd());
                    
                    uint16_t compare = copins1 & 0xFFFE;
                    uint16_t mask = (copins2 & 0x7FFE);
                    // bool bfd = copins2 & 0x1000;
                    
                    // Check if the trigger position has already been passed
                    uint32_t beam = amiga->dma.beam & 0xFFFF;
                    
                    if ((beam & mask) >= (compare & mask)) {
                        
                        // Set the skip flag
                        skip = true;
                    }
                }
              

            }
            break;
            
        case COPPER_JMP1:
        
            // Load COP1LC into the program counter
            coppc = coplc[0];
            scheduleEventRel(2, COPPER_REQUEST_DMA);
            break;

        case COPPER_JMP2:
            
            // Load COP2LC into the program counter
            coppc = coplc[1];
            scheduleEventRel(2, COPPER_REQUEST_DMA);
            break;

        default:
            
            assert(false);
            break;
    }
}

void
Copper::vsyncAction()
{
    // debug("Copper vsync\n");
    
    /* "At the start of each vertical blanking interval, COP1LC is automatically
     *  used to start the program counter. That is, no matter what the Copper is
     *  doing, when the end of vertical blanking occurs, the Copper is
     *  automatically forced to restart its operations at the address contained
     *  in COPlLC." [HRM]
     */

    // TODO: What is the exact timing here?
    scheduleEventRel(4, COPPER_JMP1);
}
