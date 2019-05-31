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
        
        { &skip,    sizeof(skip),    0 },
        { &coplc,   sizeof(coplc),   DWORD_ARRAY },
        { &cdang,   sizeof(cdang),   0 },
        { &copins1, sizeof(copins1), 0 },
        { &copins2, sizeof(copins2), 0 },
        { &coppc,   sizeof(coppc),   0 },
    });
}

void
Copper::_initialize()
{
    mem = &amiga->mem;
    agnus = &amiga->agnus;
    events = &amiga->agnus.events; 
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
Copper::_inspect()
{
    // Prevent external access to variable 'info'
    pthread_mutex_lock(&lock);
    
    info.cdang     = cdang;
    info.active    = agnus->events.isPending(COP_SLOT);
    info.coppc     = coppc;
    info.copins[0] = copins1;
    info.copins[1] = copins2;
    info.coplc[0]  = coplc[0];
    info.coplc[1]  = coplc[1];
    
    pthread_mutex_unlock(&lock);
}

void
Copper::_dump()
{
    plainmsg("   cdang: %d\n", cdang);
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
    debug(2, "pokeCOPCON(%X)\n", value);
    
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
    
    debug(2, "pokeCOPJMP%d\n", x + 1);
    
    /* "When you write to a Copper strobe address, the Copper reloads its
     *  program counter from the corresponding location register." [HRM]
     */
    coppc = coplc[1];
}

void
Copper::pokeCOPINS(uint16_t value)
{
    /* COPINS is a dummy address that can be used to write the first or
     * the secons instruction register, depending on the current state.
     */

    // TODO: The following is almost certainly wrong...
    /* if (state == COP_MOVE || state == COP_WAIT_OR_SKIP) {
        copins2 = value;
    } else {
        copins1 = value;
    }
    */
    copins1 = value;
}

void
Copper::pokeCOPxLCH(int x, uint16_t value)
{
    assert(x < 2);
    
    debug(2, "pokeCOP%dLCH(%X)\n", x, value);
    coplc[x] = REPLACE_HI_WORD(coplc[x], value);
}

void
Copper::pokeCOPxLCL(int x, uint16_t value)
{
    assert(x < 2);
    
    debug(2, "pokeCOP%dLCL(%X)\n", x, value);
    coplc[x] = REPLACE_LO_WORD(coplc[x], value & 0xFFFE);
}

bool
Copper::comparator(uint32_t beam, uint32_t waitpos, uint32_t mask)
{
    // Get comparison bits for the vertical beam position
    uint8_t vBeam = (beam >> 8) & 0xFF;
    uint8_t vWaitpos = (waitpos >> 8) & 0xFF;
    uint8_t vMask = (mask >> 8) | 0x80;
    
    // debug(" * vBeam = %X vWaitpos = %X vMask = %X\n", vBeam, vWaitpos, vMask);
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
    
    // Compare horizontal positions
    return (hBeam & hMask) >= (hWaitpos & hMask);
}

bool
Copper::comparator(uint32_t beam)
{
    return comparator(beam, getVPHP(), getVMHM());
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
    Beam b = agnus->beamPosition();

    // Advance two cycles to get to the first possible trigger position
    b = agnus->addToBeam(b, 2);

    // Translate position to a binary 17-bit representation
    uint32_t beam = (b.y << 8) + b.x;

    /* We are going to compute the smallest beam position satisfying
     *
     *   1) computed position >= current beam position + 2,
     *   2) the comparator circuit triggers.
     *
     * We do this by starting with the maximum possible value:
     */
    uint32_t pos = 0x1FFE2;
    
    /* Now, we iterate through bit from left to right and set the bit we see
     * to 0 as long as conditions 1) and 2) hold.
     */
    for (int i = 16; i >= 0; i--) {
        uint32_t newPos = pos & ~(1 << i);
        if (newPos >= beam && comparator(newPos)) {
            pos = newPos;
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
    uint32_t instr = mem->spypeek32(addr);
    return !(HI_WORD(instr) & 1);
}

bool Copper::isWaitCmd()
{
     return (copins1 & 1) && !(copins2 & 1);
}

bool Copper::isWaitCmd(uint32_t addr)
{
    uint32_t instr = mem->spypeek32(addr);
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
    uint32_t instr = mem->spypeek32(addr);
    return (HI_WORD(instr) & 1) && (LO_WORD(instr) & 1);
}

uint16_t
Copper::getRA()
{
    return copins1 & 0x1FE;
}

uint16_t
Copper::getRA(uint32_t addr)
{
    uint32_t instr = mem->spypeek32(addr);
    return HI_WORD(instr) & 0x1FE;
}

uint16_t
Copper::getDW()
{
    return copins1;
}

uint16_t
Copper::getDW(uint32_t addr)
{
    uint32_t instr = mem->spypeek32(addr);
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
    uint32_t instr = mem->spypeek32(addr);
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
    uint32_t instr = mem->spypeek32(addr);
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
    uint32_t instr = mem->spypeek32(addr);
    return (LO_WORD(instr) & 0x7FFE) | 0x8001;
}

bool
Copper::isIllegalAddress(uint32_t addr)
{
    addr &= 0x1FE;
    return addr < (cdang ? 0x40 : 0x80);
}

bool
Copper::isIllegalInstr(uint32_t addr)
{
    return isMoveCmd(addr) && isIllegalAddress(getRA(addr));
}

void
Copper::serviceEvent(EventID id)
{
    debug(2, "(%d,%d): ", agnus->vpos, agnus->hpos);
    
    switch (id) {
            
        case COP_REQUEST_DMA:
            
            // plainmsg("COP_REQUEST_DMA\n");
            
            /* In this state, Copper waits for a free DMA cycle.
             * Once DMA access is granted, it continues with fetching the
             * first instruction word.
             */
            if (agnus->copperCanHaveBus()) {
                events->scheduleRel(COP_SLOT, DMA_CYCLES(2), COP_FETCH);
            }
            
        case COP_FETCH:
            
            if (agnus->copperCanHaveBus()) {
                
                // Load the first instruction word
                copins1 = mem->peek16(coppc);
                debug(2, "COP_FETCH: coppc = %X copins1 = %X\n", coppc, copins1);
                advancePC();
                
                // Determine the next state based on the instruction type
                events->scheduleRel(COP_SLOT, DMA_CYCLES(2), isMoveCmd() ? COP_MOVE : COP_WAIT_OR_SKIP);
            }
            break;
            
        case COP_MOVE:
            
            if (agnus->copperCanHaveBus()) {
                
                // Load the second instruction word
                copins2 = mem->peek16(coppc);
                debug(2, "COP_MOVE: coppc = %X copins2 = %X\n", coppc, copins2);
                advancePC();
                
                // Extract register number from the first instruction word
                uint16_t reg = (copins1 & 0x1FE);
                
                if (isIllegalAddress(reg)) {
                    
                    events->cancel(COP_SLOT); // Stops the Copper
                    break;
                }
                
                // Write into the custom register
                if (!skip) {
                    debug(COP_DEBUG, "MOVE %X <- %X\n", reg, copins2);
                    mem->pokeCustom16(reg, copins2);
                }
                skip = false;
                
                // Schedule next event
                events->scheduleRel(COP_SLOT, DMA_CYCLES(2), COP_FETCH);
            }
            break;
            
        case COP_WAIT_OR_SKIP:
            
            if (agnus->copperCanHaveBus()) {

                // Load the second instruction word
                copins2 = mem->peek16(coppc);
                debug(2, "COP_WAIT_OR_SKIP: coppc = %X copins2 = %X\n", coppc, copins2);
                debug(2, "    VPHP = %X VMHM = %X\n", getVPHP(), getVMHM());
                advancePC();
                
                // Is it a WAIT command?
                if (isWaitCmd()) {
                    
                    // Clear the skip flag
                    skip = false;
                    
                    // Determine where the WAIT command will trigger
                    uint32_t trigger = nextTriggerPosition();
                    
                    // In how many cycles do we get there?
                    Cycle delay = agnus->beamDiff(trigger);
                    
                    debug(2, "   trigger = (%d,%d) delay = %lld\n",
                             VPOS(trigger), HPOS(trigger), delay);
                    
                    // Stop the Copper or schedule a wake up event
                    if (delay == NEVER) {
                        events->disable(COP_SLOT);
                    } else {
                        events->scheduleRel(COP_SLOT, delay, COP_FETCH);
                    }
                    // amiga->agnus.eventHandler.dump();
                }
                
                // It must be a SKIP command then.
                else {
                    
                    // Determine if the next command has to be skipped by
                    // running the comparator circuit.
                    assert(isSkipCmd());
                    skip = comparator();
                }
              

            }
            break;
            
        case COP_JMP1:
        
            // Load COP1LC into the program counter
            coppc = coplc[0];
            debug(2, "COP_JMP1: coppc = %X\n", coppc);
            events->scheduleRel(COP_SLOT, DMA_CYCLES(2), COP_REQUEST_DMA);
            break;

        case COP_JMP2:
            
            // Load COP2LC into the program counter
            coppc = coplc[1];
            debug(2, "COP_JMP2: coppc = %X\n", coppc);
            events->scheduleRel(COP_SLOT, DMA_CYCLES(2), COP_REQUEST_DMA);
            break;

        default:
            
            assert(false);
            break;
    }
}

void
Copper::vsyncAction()
{
    /* "At the start of each vertical blanking interval, COP1LC is automatically
     *  used to start the program counter. That is, no matter what the Copper is
     *  doing, when the end of vertical blanking occurs, the Copper is
     *  automatically forced to restart its operations at the address contained
     *  in COPlLC." [HRM]
     */

    // TODO: What is the exact timing here?
    if (agnus->copDMA()) {
        events->scheduleRel(COP_SLOT, DMA_CYCLES(4), COP_JMP1);
    } else {
        events->cancel(COP_SLOT);
    }
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
    const char *suffix = getBFD(addr) ? "*" : "";
    
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
    
    uint32_t addr = (list == 1) ? coplc[0] : coplc[1];
    addr = (addr + 2 * offset) & 0x7FFFF;
    
    return disassemble(addr);
}
