// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"
#include <new>

Memory::Memory()
{
    setDescription("Memory");
    
    // Start with 256 KB Chip Ram
    allocateChipRam(KB(256));
}

Memory::~Memory()
{
    if (bootRom) delete[] bootRom;
    if (kickRom) delete[] kickRom;
    if (chipRam) delete[] chipRam;
    if (slowRam) delete[] slowRam;
    if (fastRam) delete[] fastRam;
}

void
Memory::_powerOn()
{
    // Make Rom writable if an A1000 is emulated
    kickIsWritable = amiga->config.model == A1000;
    
    // Set up the memory lookup table
    updateMemSrcTable();
    
}

void
Memory::_powerOff()
{
    
}

void
Memory::_reset()
{
    
}

void
Memory::_ping()
{
    
}

void
Memory::_dump()
{
    plainmsg("     Boot Rom: %d KB at %p\n", bootRomSize >> 10, bootRom);
    plainmsg("     Kick Rom: %d KB at %p (%s)\n", kickRomSize >> 10,
             kickRom, kickIsWritable ? "unlocked" : "locked");
    plainmsg("     Chip Ram: %d KB at %p\n", chipRamSize >> 10, chipRam);
    plainmsg("     Slow Ram: %d KB at %p\n", slowRamSize >> 10, slowRam);
    plainmsg("     Fast Ram: %d KB at %p\n", fastRamSize >> 10, fastRam);
}

bool
Memory::alloc(size_t size, uint8_t *&ptrref, size_t &sizeref)
{
    // Do some consistency checking
    assert((ptrref == NULL) == (sizeref == 0));

    // Only proceed if memory layout changes
    if (size == sizeref)
        return true;
    
    // Delete previous allocation
    if (ptrref) {
        delete[] ptrref;
        ptrref = NULL;
        sizeref = 0;
    }
    
    // Allocate memory
    if (size) {
        
        // We allocate three bytes more than we need to handle the case that
        // a long word access is performed on the last valid memory address.
        size_t allocSize = KB(size) + 3;
        if (!(ptrref = new (std::nothrow) uint8_t[allocSize])) {
            warn("Cannot allocate %X bytes of memory\n", size);
            return false;
        }
        memset(ptrref, 42, allocSize); // TODO: 42 is just a test value
        sizeref = size;
    }
    
    // Update the memory lookup table
    updateMemSrcTable();
    return true;
}

/*
void
AmigaMemory::dealloc(uint8_t *&ptrref, size_t &sizeref)
{    
    // Do some consistency checking
    assert((ptrref == NULL) == (sizeref == 0));
    
    if (ptrref) {
        delete[] ptrref;
        ptrref = NULL;
        sizeref = 0;
    }
}
*/

void
Memory::loadRom(AmigaFile *rom, uint8_t *target, size_t length)
{
    if (rom) {
        
        assert(target != NULL);
        memset(target, 0, length);
        
        int c;
        rom->seek(0);
        for (size_t i = 0; i < length; i++) {
            if ((c = rom->read()) == EOF) break;
            *(target++) = c;
        }
    }
}

bool
Memory::loadBootRomFromBuffer(const uint8_t *buffer, size_t length)
{
    assert(buffer != NULL);
    
    BootRom *rom = BootRom::makeWithBuffer(buffer, length);
    
    if (!rom) {
        msg("Failed to read Boot Rom from buffer at %p\n", buffer);
        return false;
    }
    
    return loadBootRom(rom);
}

bool
Memory::loadBootRomFromFile(const char *path)
{
    assert(path != NULL);
    
    BootRom *rom = BootRom::makeWithFile(path);
    
    if (!rom) {
        msg("Failed to read Boot Rom from file %s\n", path);
        return false;
    }
    
    return loadBootRom(rom);
}

bool
Memory::loadBootRom(BootRom *rom)
{
    assert(rom != NULL);
    
    if (!alloc(rom->getSize(), bootRom, bootRomSize))
        return false;

    loadRom(rom, bootRom, bootRomSize);
    return true;
}

bool
Memory::loadKickRomFromBuffer(const uint8_t *buffer, size_t length)
{
    assert(buffer != NULL);
    
    KickRom *rom = KickRom::makeWithBuffer(buffer, length);
    
    if (!rom) {
        msg("Failed to read Kick Rom from buffer at %p\n", buffer);
        return false;
    }
    
    return loadKickRom(rom);
}

bool
Memory::loadKickRomFromFile(const char *path)
{
    assert(path != NULL);
    
    KickRom *rom = KickRom::makeWithFile(path);
    
    if (!rom) {
        msg("Failed to read Kick Rom from file %s\n", path);
        return false;
    }
    
    return loadKickRom(rom);
}

bool
Memory::loadKickRom(KickRom *rom)
{
    assert(rom != NULL);
    
    if (!alloc(rom->getSize(), kickRom, kickRomSize))
        return false;
    
    loadRom(rom, kickRom, kickRomSize);
    return true;
}

void
Memory::updateMemSrcTable()
{
    MemorySource mem_boot = bootRom ? MEM_BOOT : MEM_UNMAPPED;
    MemorySource mem_kick = kickRom ? MEM_KICK : MEM_UNMAPPED;
    
    assert(chipRamSize % 0x10000 == 0);
    assert(slowRamSize % 0x10000 == 0);
    assert(fastRamSize % 0x10000 == 0);

    bool rtc = amiga ? amiga->config.realTimeClock : false;
    bool ovl = amiga ? (amiga->ciaA.getPA() & 1) : false;
    
    debug("updateMemSrcTable: rtc = %d ovl = %d\n", rtc, ovl);
    
    // Start from scratch
    for (unsigned i = 0x00; i <= 0xFF; i++)
        memSrc[i] = MEM_UNMAPPED;
    
    // Chip Ram and Chip Ram mirror
    for (unsigned i = 0; i < 32; i++)
        memSrc[i] = MEM_CHIP;
    
    // Fast Ram
    for (unsigned i = 0; i < fastRamSize / 0x10000; i++)
        memSrc[0x20 + i] = MEM_FAST;

    // CIA range
    for (unsigned i = 0xA0; i <= 0xBF; i++)
        memSrc[i] = MEM_CIA;

    // OCS (some assignments will be overwritten below by slow ram and RTC)
    for (unsigned i = 0xC0; i <= 0xDF; i++)
        memSrc[i] = MEM_OCS;
    
    // Slow Ram
    for (unsigned i = 0; i < slowRamSize / 0x10000; i++)
        memSrc[0xC0 + i] = MEM_SLOW;

    // Real-time clock
    for (unsigned i = 0xDC; rtc && i <= 0xDE; i++)
        memSrc[i] = MEM_RTC;

    // Boot Rom or Kickstart mirror
    for (unsigned i = 0xF8; i <= 0xFB; i++)
        memSrc[i] = kickIsWritable ? mem_boot : mem_kick;

    // Kickstart
    for (unsigned i = 0xFC; i <= 0xFF; i++)
        memSrc[i] = mem_kick;

    // Overlay Rom with lower memory area if the OVL line is high
    for (unsigned i = 0; ovl && i < 8 && memSrc[0xF8 + i] != MEM_UNMAPPED; i++)
        memSrc[i] = memSrc[0xF8 + i];
    
    if (amiga) amiga->putMessage(MSG_MEM_LAYOUT);
}

/*
MemorySource
Memory::computeMemSrc(uint16_t bank)
{

}
*/


uint8_t
Memory::peek8(uint32_t addr)
{
    // debug("PC: %X peek8(%X)\n", amiga->cpu.getPC(), addr);

    addr &= 0xFFFFFF;
    switch (memSrc[addr >> 16]) {
            
        case MEM_UNMAPPED: return 0;
        case MEM_CHIP:     ASSERT_CHIP_ADDR(addr); return READ_CHIP_8(addr);
        case MEM_FAST:     ASSERT_FAST_ADDR(addr); return READ_FAST_8(addr);
        case MEM_CIA:      ASSERT_CIA_ADDR(addr);  return peekCIA8(addr);
        case MEM_SLOW:     ASSERT_SLOW_ADDR(addr); return READ_SLOW_8(addr);
        case MEM_RTC:      ASSERT_RTC_ADDR(addr);  return 1;
        case MEM_OCS:      ASSERT_OCS_ADDR(addr);  return peekCustom8(addr);
        case MEM_BOOT:     ASSERT_BOOT_ADDR(addr); return READ_BOOT_8(addr);
        case MEM_KICK:     ASSERT_KICK_ADDR(addr); return READ_KICK_8(addr);
        default:           assert(false);
    }
    return 0;
}

uint16_t
Memory::peek16(uint32_t addr)
{
    // debug("PC: %X peek16(%X)\n", amiga->cpu.getPC(), addr);

    assert(IS_EVEN(addr));
    
    addr &= 0xFFFFFF;
    switch (memSrc[addr >> 16]) {
            
        case MEM_UNMAPPED: return 0;
        case MEM_CHIP:     ASSERT_CHIP_ADDR(addr); return READ_CHIP_16(addr);
        case MEM_FAST:     ASSERT_FAST_ADDR(addr); return READ_FAST_16(addr);
        case MEM_CIA:      ASSERT_CIA_ADDR(addr);  return peekCIA16(addr);
        case MEM_SLOW:     ASSERT_SLOW_ADDR(addr); return READ_SLOW_16(addr);
        case MEM_RTC:      ASSERT_RTC_ADDR(addr);  return 0;
        case MEM_OCS:      ASSERT_OCS_ADDR(addr);  return peekCustom16(addr);
        case MEM_BOOT:     ASSERT_BOOT_ADDR(addr); return READ_BOOT_16(addr);
        case MEM_KICK:     ASSERT_KICK_ADDR(addr); return READ_KICK_16(addr);
        default:           assert(false);
    }
    return 0;
}

uint32_t
Memory::peek32(uint32_t addr)
{
    // debug("PC: %X peek32(%X)\n", amiga->cpu.getPC(), addr);
    return HI_W_LO_W(peek16(addr), peek16(addr + 2));
}

uint8_t
Memory::spypeek8(uint32_t addr)
{
    addr &= 0xFFFFFF;
    switch (memSrc[addr >> 16]) {
            
        case MEM_UNMAPPED: return 0;
        case MEM_CHIP:     ASSERT_CHIP_ADDR(addr); return READ_CHIP_8(addr);
        case MEM_FAST:     ASSERT_FAST_ADDR(addr); return READ_FAST_8(addr);
        case MEM_CIA:      ASSERT_CIA_ADDR(addr);  return spypeekCIA8(addr);
        case MEM_SLOW:     ASSERT_SLOW_ADDR(addr); return READ_SLOW_8(addr);
        case MEM_RTC:      ASSERT_RTC_ADDR(addr);  return 0;
        case MEM_OCS:      ASSERT_OCS_ADDR(addr);  return spypeekCustom8(addr);
        case MEM_BOOT:     ASSERT_BOOT_ADDR(addr); return READ_BOOT_8(addr);
        case MEM_KICK:     ASSERT_KICK_ADDR(addr); return READ_KICK_8(addr);
        default:           assert(false);
    }
    return 0;
}

uint16_t
Memory::spypeek16(uint32_t addr)
{
    assert(IS_EVEN(addr));
    
    addr &= 0xFFFFFF;
    switch (memSrc[addr >> 16]) {
            
        case MEM_UNMAPPED: return 0;
        case MEM_CHIP:     ASSERT_CHIP_ADDR(addr); return READ_CHIP_16(addr);
        case MEM_FAST:     ASSERT_FAST_ADDR(addr); return READ_FAST_16(addr);
        case MEM_CIA:      ASSERT_CIA_ADDR(addr);  return spypeekCIA16(addr);
        case MEM_SLOW:     ASSERT_SLOW_ADDR(addr); return READ_SLOW_16(addr);
        case MEM_RTC:      ASSERT_RTC_ADDR(addr);  return 0;
        case MEM_OCS:      ASSERT_OCS_ADDR(addr);  return spypeekCustom16(addr);
        case MEM_BOOT:     ASSERT_BOOT_ADDR(addr); return READ_BOOT_16(addr);
        case MEM_KICK:     ASSERT_KICK_ADDR(addr); return READ_KICK_16(addr);
        default:           assert(false);
    }
    return 0;
}

uint32_t
Memory::spypeek32(uint32_t addr)
{
    return HI_W_LO_W(spypeek16(addr), spypeek16(addr + 2));
}

void
Memory::poke8(uint32_t addr, uint8_t value)
{
    // debug("PC: %X poke8(%X,%X)\n", amiga->cpu.getPC(), addr, value);

    addr &= 0xFFFFFF;
    switch (memSrc[addr >> 16]) {
            
        case MEM_UNMAPPED: return;
        case MEM_CHIP:     ASSERT_CHIP_ADDR(addr); WRITE_CHIP_8(addr, value); break;
        case MEM_FAST:     ASSERT_FAST_ADDR(addr); WRITE_FAST_8(addr, value); break;
        case MEM_CIA:      ASSERT_CIA_ADDR(addr);  pokeCIA8(addr, value); break;
        case MEM_SLOW:     ASSERT_SLOW_ADDR(addr); WRITE_SLOW_8(addr, value); break;
        case MEM_RTC:      ASSERT_RTC_ADDR(addr);  break;
        case MEM_OCS:      ASSERT_OCS_ADDR(addr);  pokeCustom8(addr, value); break;
        case MEM_BOOT:     ASSERT_BOOT_ADDR(addr); break;
        case MEM_KICK:     ASSERT_KICK_ADDR(addr); break;
        default:           assert(false);
    }
}

void
Memory::poke16(uint32_t addr, uint16_t value)
{
    // debug("PC: %X poke16(%X,%X)\n", amiga->cpu.getPC(), addr, value);

    addr &= 0xFFFFFF;
    switch (memSrc[addr >> 16]) {
            
        case MEM_UNMAPPED: return;
        case MEM_CHIP:     ASSERT_CHIP_ADDR(addr); WRITE_CHIP_16(addr, value); break;
        case MEM_FAST:     ASSERT_FAST_ADDR(addr); WRITE_FAST_16(addr, value); break;
        case MEM_CIA:      ASSERT_CIA_ADDR(addr);  pokeCIA16(addr, value); break;
        case MEM_SLOW:     ASSERT_SLOW_ADDR(addr); WRITE_SLOW_16(addr, value); break;
        case MEM_RTC:      ASSERT_RTC_ADDR(addr);  break;
        case MEM_OCS:      ASSERT_OCS_ADDR(addr);  pokeCustom16(addr, value); break;
        case MEM_BOOT:     ASSERT_BOOT_ADDR(addr); break;
        case MEM_KICK:     ASSERT_KICK_ADDR(addr); break;
        default:           assert(false);
    }
}

void
Memory::poke32(uint32_t addr, uint32_t value)
{
    // debug("PC: %X poke32(%X,%X)\n", amiga->cpu.getPC(), addr, value);

    poke16(addr,     HI_WORD(value));
    poke16(addr + 2, LO_WORD(value));
}

//
// CIAs
//

uint8_t
Memory::peekCIA8(uint32_t addr)
{
    uint32_t reg = (addr >> 8) & 0b1111;
    uint32_t sel = (addr >> 12) & 0b11;
    bool a0 = addr & 1;
    
    switch (sel) {
            
        case 0b00:
            return a0 ? amiga->ciaA.peek(reg) : amiga->ciaB.peek(reg);
            
        case 0b01:
            return a0 ? LO_BYTE(amiga->cpu.getIR()) : amiga->ciaB.peek(reg);
            
        case 0b10:
            return a0 ? amiga->ciaA.peek(reg) : HI_BYTE(amiga->cpu.getIR());
            
        case 0b11:
            return a0 ? LO_BYTE(amiga->cpu.getIR()) : HI_BYTE(amiga->cpu.getIR());
    }
    assert(false);
    return 0;
}

uint16_t
Memory::peekCIA16(uint32_t addr)
{
    uint32_t reg = (addr >> 8) & 0b1111;
    uint32_t sel = (addr >> 12) & 0b11;
    
    switch (sel) {
            
        case 0b00:
            return HI_LO(amiga->ciaB.peek(reg), amiga->ciaA.peek(reg));
            
        case 0b01:
            return HI_LO(amiga->ciaB.peek(reg), 0xFF);
            
        case 0b10:
            return HI_LO(0xFF, amiga->ciaA.peek(reg));
            
        case 0b11:
            return amiga->cpu.getIR();
            
    }
    assert(false);
    return 0;
}

uint32_t
Memory::peekCIA32(uint32_t addr)
{
    return HI_W_LO_W(peekCIA16(addr), peekCIA16(addr + 2));
}

uint8_t
Memory::spypeekCIA8(uint32_t addr)
{
    uint32_t reg = (addr >> 8) & 0b1111;
    uint32_t sel = (addr >> 12) & 0b11;
    bool a0 = addr & 1;
    
    switch (sel) {
            
        case 0b00:
            return a0 ? amiga->ciaA.spypeek(reg) : amiga->ciaB.spypeek(reg);
            
        case 0b01:
            return a0 ? LO_BYTE(amiga->cpu.getIR()) : amiga->ciaB.spypeek(reg);
            
        case 0b10:
            return a0 ? amiga->ciaA.spypeek(reg) : HI_BYTE(amiga->cpu.getIR());
            
        case 0b11:
            return a0 ? LO_BYTE(amiga->cpu.getIR()) : HI_BYTE(amiga->cpu.getIR());
    }
    assert(false);
    return 0;
}

uint16_t
Memory::spypeekCIA16(uint32_t addr)
{
    uint32_t reg = (addr >> 8) & 0b1111;
    uint32_t sel = (addr >> 12) & 0b11;
    
    switch (sel) {
            
        case 0b00:
            return HI_LO(amiga->ciaB.spypeek(reg), amiga->ciaA.spypeek(reg));
            
        case 0b01:
            return HI_LO(amiga->ciaB.spypeek(reg), 0xFF);
            
        case 0b10:
            return HI_LO(0xFF, amiga->ciaA.spypeek(reg));
            
        case 0b11:
            return amiga->cpu.getIR();
            
    }
    assert(false);
    return 0;
}

uint32_t
Memory::spypeekCIA32(uint32_t addr)
{
    return HI_W_LO_W(spypeekCIA16(addr), spypeekCIA16(addr + 2));
}

void
Memory::pokeCIA8(uint32_t addr, uint8_t value)
{
    uint32_t reg = (addr >> 8) & 0b1111;
    uint32_t selA = (addr & 0x1000) == 0;
    uint32_t selB = (addr & 0x2000) == 0;

    if (selA) amiga->ciaA.poke(reg, value);
    if (selB) amiga->ciaB.poke(reg, value);
}

void
Memory::pokeCIA16(uint32_t addr, uint16_t value)
{
    assert(IS_EVEN(addr));
    
    uint32_t reg = (addr >> 8) & 0b1111;
    uint32_t selA = (addr & 0x1000) == 0;
    uint32_t selB = (addr & 0x2000) == 0;
    
    if (selA) amiga->ciaA.poke(reg, LO_BYTE(value));
    if (selB) amiga->ciaB.poke(reg, HI_BYTE(value));
}

void
Memory::pokeCIA32(uint32_t addr, uint32_t value)
{
    pokeCIA16(addr,     HI_WORD(value));
    pokeCIA16(addr + 2, LO_WORD(value));
}


//
// Custom chip set
//

uint8_t
Memory::peekCustom8(uint32_t addr)
{
    if (IS_EVEN(addr)) {
        return HI_BYTE(peekCustom16(addr));
    } else {
        return LO_BYTE(peekCustom16(addr & 0x1FE));
    }
}

uint16_t
Memory::peekCustom16(uint32_t addr)
{
    assert(IS_EVEN(addr));
    
    switch ((addr >> 1) & 0xFF) {
            
        case 0x000 >> 1: // BLTDAT
            break;
        case 0x002 >> 1: // DMACONR
            return amiga->dma.peekDMACON();
        case 0x004 >> 1: // VPOSR
            return amiga->dma.peekVPOS();
        case 0x006 >> 1: // VHPOSR
            return amiga->dma.peekVHPOS();
        case 0x008 >> 1: // DSKDATR
            return amiga->paula.peekDSKDAT();
        case 0x00A >> 1: // JOY0DAT
            break;
        case 0x00C >> 1: // JOY1DAT
            break;
        case 0x00E >> 1: // CLXDAT
            break;
        case 0x010 >> 1: // ADKCONR
            break;
        case 0x012 >> 1: // POT0DAT
            break;
        case 0x014 >> 1: // POT1DAT
            break;
        case 0x016 >> 1: // POTINP
            break;
        case 0x018 >> 1: // SERDATR
            return amiga->paula.peekSERDAT();
        case 0x01A >> 1: // DSKBYTR
            break;
        case 0x01C >> 1: // INTENAR
            return amiga->paula.peekINTENA();
        case 0x01E >> 1: // INTREQR
            return amiga->paula.peekINTREQ();

        default: // Write-only register
            break; // return 0; // TODO: What do we return here?
    }
    
    warn("peekCustom16(%X [%s]): MISSING IMPLEMENTATION\n",
         addr, customReg[(addr >> 1) & 0xFF]);
    amiga->pause();
    return 42;
}

uint32_t
Memory::peekCustom32(uint32_t addr)
{
    return HI_W_LO_W(peekCustom16(addr), peekCustom16(addr + 2));
}

uint8_t
Memory::spypeekCustom8(uint32_t addr)
{
    if (IS_EVEN(addr)) {
        return HI_BYTE(spypeekCustom16(addr));
    } else {
        return LO_BYTE(spypeekCustom16(addr & 0x1FE));
    }
}

uint16_t
Memory::spypeekCustom16(uint32_t addr)
{
    assert(IS_EVEN(addr));
    
    switch (addr & 0x1FE) {
            
    }

    return peekCustom16(addr);
}

uint32_t
Memory::spypeekCustom32(uint32_t addr)
{
    return HI_W_LO_W(spypeekCustom16(addr), spypeekCustom16(addr + 2));
}

void
Memory::pokeCustom8(uint32_t addr, uint8_t value)
{
    pokeCustom16(addr & 0x1FE, HI_LO(value,value));
}

void
Memory::pokeCustom16(uint32_t addr, uint16_t value)
{
    assert(IS_EVEN(addr));
    
    switch ((addr >> 1) & 0xFF) {

        case 0x020 >> 1: // DSKPTH
            amiga->dma.pokeDSKPTH(value); return;
        case 0x022 >> 1: // DSKPTL
            amiga->dma.pokeDSKPTL(value); return;
        case 0x024 >> 1: // DSKLEN
            amiga->paula.pokeDSKLEN(value); return;
        case 0x026 >> 1: // DSKDAT
            amiga->paula.pokeDSKDAT(value); return;
            // case 0x28 >> 1: // REFPTR
            // case 0x2A >> 1: // VPOSW
            // case 0x2C >> 1: // VHPOSW
        case 0x02E >> 1: // COPCON
            amiga->dma.copper.pokeCOPCON(value); return;
        case 0x030 >> 1: // SERDAT
            amiga->paula.pokeSERDAT(value); return;
        case 0x032 >> 1: // SERPER
            amiga->paula.pokeSERPER(value); return;
        case 0x40 >> 1: // BLTCON0
            amiga->dma.blitter.pokeBLTCON0(value); return;
        case 0x42 >> 1: // BLTCON1
            amiga->dma.blitter.pokeBLTCON1(value); return;
        case 0x44 >> 1: // BLTAFWM
            amiga->dma.blitter.pokeBLTAFWM(value); return;
        case 0x46 >> 1: // BLTALWM
            amiga->dma.blitter.pokeBLTALWM(value); return;
        case 0x48 >> 1: // BLTCPTH
            amiga->dma.blitter.pokeBLTCPTH(value); return;
        case 0x4A >> 1: // BLTCPTL
            amiga->dma.blitter.pokeBLTCPTL(value); return;
        case 0x4C >> 1: // BLTBPTH
            amiga->dma.blitter.pokeBLTBPTH(value); return;
        case 0x4E >> 1: // BLTBPTL
            amiga->dma.blitter.pokeBLTBPTH(value); return;
        case 0x50 >> 1: // BLTAPTH
            amiga->dma.blitter.pokeBLTAPTH(value); return;
        case 0x52 >> 1: // BLTAPTL
            amiga->dma.blitter.pokeBLTAPTL(value); return;
        case 0x54 >> 1: // BLTDPTH
            amiga->dma.blitter.pokeBLTDPTH(value); return;
        case 0x56 >> 1: // BLTDPTL
            amiga->dma.blitter.pokeBLTDPTL(value); return;
        case 0x58 >> 1: // BLTSIZE
            amiga->dma.blitter.pokeBLTSIZE(value); return;
        case 0x5A >> 1: // unused
        case 0x5C >> 1: // unused
        case 0x5E >> 1: // unused
            return; 
        case 0x60 >> 1: // BLTCMOD
            amiga->dma.blitter.pokeBLTCMOD(value); return;
        case 0x62 >> 1: // BLTBMOD
            amiga->dma.blitter.pokeBLTBMOD(value); return;
        case 0x64 >> 1: // BLTAMOD
            amiga->dma.blitter.pokeBLTAMOD(value); return;
        case 0x66 >> 1: // BLTDMOD
            amiga->dma.blitter.pokeBLTDMOD(value); return;
        case 0x68 >> 1: // unused
        case 0x6A >> 1: // unused
        case 0x6C >> 1: // unused
        case 0x6E >> 1: // unused
            return;
        case 0x70 >> 1: // BLTCDAT
            amiga->dma.blitter.pokeBLTCDAT(value); return;
        case 0x72 >> 1: // BLTBDAT
            amiga->dma.blitter.pokeBLTBDAT(value); return;
        case 0x74 >> 1: // BLTADAT
            amiga->dma.blitter.pokeBLTADAT(value); return;
        case 0x76 >> 1: // unused
        case 0x78 >> 1: // unused
        case 0x7A >> 1: // unused
        case 0x7C >> 1: // unused
            return;
        // case 0x7E >> 1: // DSKSYNC
        //     amiga->paule.pokeDSKSYNC(value); return;
        case 0x080 >> 1: // COP1LCH
            amiga->dma.copper.pokeCOPxLCH(0, value); return;
        case 0x082 >> 1: // COP1LCL
            amiga->dma.copper.pokeCOPxLCL(0, value); return;
        case 0x084 >> 1: // COP2LCH
            amiga->dma.copper.pokeCOPxLCH(1, value); return;
        case 0x086 >> 1: // COP2LCL
            amiga->dma.copper.pokeCOPxLCL(1, value); return;
        case 0x088 >> 1: // COPJMP1
            amiga->dma.copper.pokeCOPJMP(0); return;
        case 0x08A >> 1: // COPJMP1
            amiga->dma.copper.pokeCOPJMP(1); return;
        case 0x08C >> 1: // COPINS
            amiga->dma.copper.pokeCOPINS(value); return;
        case 0x08E >> 1: // DIWSTRT
            amiga->dma.pokeDIWSTRT(value); return;
        case 0x090 >> 1: // DIWSTOP
            amiga->dma.pokeDIWSTOP(value); return;
        case 0x092 >> 1: // DDFSTRT
            amiga->dma.pokeDDFSTRT(value); return;
        case 0x094 >> 1: // DDFSTOP
            amiga->dma.pokeDDFSTOP(value); return;
        case 0x096 >> 1: // DMACON
            amiga->dma.pokeDMACON(value); return;
        case 0x09A >> 1: // INTENA
            amiga->paula.pokeINTENA(value); return;
        case 0x09C >> 1: // INTREQ
            amiga->paula.pokeINTREQ(value); return;
        case 0x09E >> 1: // unsed
            return;
            
        case 0x0A0 >> 1: // AUD0LCH
            amiga->dma.pokeAUDxLCH(0, value); return;
        case 0x0A2 >> 1: // AUD0LCL
            amiga->dma.pokeAUDxLCH(0, value); return;

        case 0x0B0 >> 1: // AUD1LCH
            amiga->dma.pokeAUDxLCH(1, value); return;
        case 0x0B2 >> 1: // AUD1LCL
            amiga->dma.pokeAUDxLCH(1, value); return;

        case 0x0C0 >> 1: // AUD2LCH
            amiga->dma.pokeAUDxLCH(2, value); return;
        case 0x0C2 >> 1: // AUD2LCL
            amiga->dma.pokeAUDxLCH(2, value); return;

        case 0x0D0 >> 1: // AUD3LCH
            amiga->dma.pokeAUDxLCH(3, value); return;
        case 0x0D2 >> 1: // AUD3LCL
            amiga->dma.pokeAUDxLCH(3, value); return;

        case 0x0E0 >> 1: // BPL1PTH
            amiga->dma.pokeBPLxPTH(0, value); return;
        case 0x0E2 >> 1: // BPL1PTL
            amiga->dma.pokeBPLxPTL(0, value); return;
        case 0x0E4 >> 1: // BPL2PTH
            amiga->dma.pokeBPLxPTH(1, value); return;
        case 0x0E6 >> 1: // BPL2PTL
            amiga->dma.pokeBPLxPTL(1, value); return;
        case 0x0E8 >> 1: // BPL3PTH
            amiga->dma.pokeBPLxPTH(2, value); return;
        case 0x0EA >> 1: // BPL3PTL
            amiga->dma.pokeBPLxPTL(2, value); return;
        case 0x0EC >> 1: // BPL4PTH
            amiga->dma.pokeBPLxPTH(3, value); return;
        case 0x0EE >> 1: // BPL4PTL
            amiga->dma.pokeBPLxPTL(3, value); return;
        case 0x0F0 >> 1: // BPL5PTH
            amiga->dma.pokeBPLxPTH(4, value); return;
        case 0x0F2 >> 1: // BPL5PTL
            amiga->dma.pokeBPLxPTL(4, value); return;
        case 0x0F4 >> 1: // BPL6PTH
            amiga->dma.pokeBPLxPTH(5, value); return;
        case 0x0F6 >> 1: // BPL6PTL
            amiga->dma.pokeBPLxPTL(5, value); return;
        case 0x0F8 >> 1: // Unused
        case 0x0FA >> 1: // Unused
        case 0x0FC >> 1: // Unused
        case 0x0FE >> 1: // Unused
            return;
        case 0x100 >> 1: // BPLCON0
            amiga->denise.pokeBPLCON0(value); return;
        case 0x102 >> 1: // BPLCON1
            amiga->denise.pokeBPLCON1(value); return;
        case 0x104 >> 1: // BPLCON2
            amiga->denise.pokeBPLCON2(value); return;
        case 0x106 >> 1: // Unused
            return;
        case 0x108 >> 1: // BPL1MOD
            amiga->dma.pokeBPL1MOD(value); return;
        case 0x10A >> 1: // BPL2MOD
            amiga->dma.pokeBPL2MOD(value); return;
        case 0x10C >> 1: // Unused
        case 0x10E >> 1: // Unused
            return;
        case 0x110 >> 1: // BPL1DAT
            amiga->denise.pokeBPLxDAT(0, value); return;
        case 0x112 >> 1: // BPL2DAT
            amiga->denise.pokeBPLxDAT(1, value); return;
        case 0x114 >> 1: // BPL3DAT
            amiga->denise.pokeBPLxDAT(2, value); return;
        case 0x116 >> 1: // BPL4DAT
            amiga->denise.pokeBPLxDAT(3, value); return;
        case 0x118 >> 1: // BPL5DAT
            amiga->denise.pokeBPLxDAT(4, value); return;
        case 0x11A >> 1: // BPL6DAT
            amiga->denise.pokeBPLxDAT(5, value); return;
        case 0x11C >> 1: // Unused
        case 0x11E >> 1: // Unused
            return;
        case 0x120 >> 1: // SPR0PTH
            amiga->dma.pokeSPRxPTH(0, value); return;
        case 0x122 >> 1: // SPR0PTL
            amiga->dma.pokeSPRxPTL(0, value); return;
        case 0x124 >> 1: // SPR1PTH
            amiga->dma.pokeSPRxPTH(1, value); return;
        case 0x126 >> 1: // SPR1PTL
            amiga->dma.pokeSPRxPTL(1, value); return;
        case 0x128 >> 1: // SPR2PTH
            amiga->dma.pokeSPRxPTH(2, value); return;
        case 0x12A >> 1: // SPR2PTL
            amiga->dma.pokeSPRxPTL(2, value); return;
        case 0x12C >> 1: // SPR3PTH
            amiga->dma.pokeSPRxPTH(3, value); return;
        case 0x12E >> 1: // SPR3PTL
            amiga->dma.pokeSPRxPTL(3, value); return;
        case 0x130 >> 1: // SPR4PTH
            amiga->dma.pokeSPRxPTH(4, value); return;
        case 0x132 >> 1: // SPR4PTL
            amiga->dma.pokeSPRxPTL(4, value); return;
        case 0x134 >> 1: // SPR5PTH
            amiga->dma.pokeSPRxPTH(5, value); return;
        case 0x136 >> 1: // SPR5PTL
            amiga->dma.pokeSPRxPTL(5, value); return;
        case 0x138 >> 1: // SPR6PTH
            amiga->dma.pokeSPRxPTH(6, value); return;
        case 0x13A >> 1: // SPR6PTL
            amiga->dma.pokeSPRxPTL(6, value); return;
        case 0x13C >> 1: // SPR7PTH
            amiga->dma.pokeSPRxPTH(7, value); return;
        case 0x13E >> 1: // SPR7PTL
            amiga->dma.pokeSPRxPTL(7, value); return;
            
        case 0x140 >> 1: // SPR0POS
            amiga->denise.pokeSPRxPOS(0, value); return;
        case 0x142 >> 1: // SPR0CTL
            amiga->denise.pokeSPRxCTL(0, value); return;

        case 0x148 >> 1: // SPR1POS
            amiga->denise.pokeSPRxPOS(1, value); return;
        case 0x14A >> 1: // SPR1CTL
            amiga->denise.pokeSPRxCTL(1, value); return;

        case 0x150 >> 1: // SPR2POS
            amiga->denise.pokeSPRxPOS(2, value); return;
        case 0x152 >> 1: // SPR2CTL
            amiga->denise.pokeSPRxCTL(2, value); return;

        case 0x158 >> 1: // SPR3POS
            amiga->denise.pokeSPRxPOS(3, value); return;
        case 0x15A >> 1: // SPR3CTL
            amiga->denise.pokeSPRxCTL(3, value); return;

        case 0x160 >> 1: // SPR4POS
            amiga->denise.pokeSPRxPOS(4, value); return;
        case 0x162 >> 1: // SPR4CTL
            amiga->denise.pokeSPRxCTL(4, value); return;

        case 0x168 >> 1: // SPR5POS
            amiga->denise.pokeSPRxPOS(5, value); return;
        case 0x16A >> 1: // SPR5CTL
            amiga->denise.pokeSPRxCTL(5, value); return;

        case 0x170 >> 1: // SPR6POS
            amiga->denise.pokeSPRxPOS(6, value); return;
        case 0x172 >> 1: // SPR6CTL
            amiga->denise.pokeSPRxCTL(6, value); return;

        case 0x178 >> 1: // SPR7POS
            amiga->denise.pokeSPRxPOS(7, value); return;
        case 0x17A >> 1: // SPR7CTL
            amiga->denise.pokeSPRxCTL(7, value); return;

        case 0x180 >> 1: // COLOR00
            amiga->denise.colorizer.pokeColorReg(0, value); return;
        case 0x182 >> 1: // COLOR01
            amiga->denise.colorizer.pokeColorReg(1, value); return;
        case 0x184 >> 1: // COLOR02
            amiga->denise.colorizer.pokeColorReg(2, value); return;
        case 0x186 >> 1: // COLOR03
            amiga->denise.colorizer.pokeColorReg(3, value); return;
        case 0x188 >> 1: // COLOR04
            amiga->denise.colorizer.pokeColorReg(4, value); return;
        case 0x18A >> 1: // COLOR05
            amiga->denise.colorizer.pokeColorReg(5, value); return;
        case 0x18C >> 1: // COLOR06
            amiga->denise.colorizer.pokeColorReg(6, value); return;
        case 0x18E >> 1: // COLOR07
            amiga->denise.colorizer.pokeColorReg(7, value); return;
        case 0x190 >> 1: // COLOR08
            amiga->denise.colorizer.pokeColorReg(8, value); return;
        case 0x192 >> 1: // COLOR09
            amiga->denise.colorizer.pokeColorReg(9, value); return;
        case 0x194 >> 1: // COLOR10
            amiga->denise.colorizer.pokeColorReg(10, value); return;
        case 0x196 >> 1: // COLOR11
            amiga->denise.colorizer.pokeColorReg(11, value); return;
        case 0x198 >> 1: // COLOR12
            amiga->denise.colorizer.pokeColorReg(12, value); return;
        case 0x19A >> 1: // COLOR13
            amiga->denise.colorizer.pokeColorReg(13, value); return;
        case 0x19C >> 1: // COLOR14
            amiga->denise.colorizer.pokeColorReg(14, value); return;
        case 0x19E >> 1: // COLOR15
            amiga->denise.colorizer.pokeColorReg(15, value); return;
        case 0x1A0 >> 1: // COLOR16
            amiga->denise.colorizer.pokeColorReg(16, value); return;
        case 0x1A2 >> 1: // COLOR17
            amiga->denise.colorizer.pokeColorReg(17, value); return;
        case 0x1A4 >> 1: // COLOR18
            amiga->denise.colorizer.pokeColorReg(18, value); return;
        case 0x1A6 >> 1: // COLOR19
            amiga->denise.colorizer.pokeColorReg(19, value); return;
        case 0x1A8 >> 1: // COLOR20
            amiga->denise.colorizer.pokeColorReg(20, value); return;
        case 0x1AA >> 1: // COLOR21
            amiga->denise.colorizer.pokeColorReg(21, value); return;
        case 0x1AC >> 1: // COLOR22
            amiga->denise.colorizer.pokeColorReg(22, value); return;
        case 0x1AE >> 1: // COLOR23
            amiga->denise.colorizer.pokeColorReg(23, value); return;
        case 0x1B0 >> 1: // COLOR24
            amiga->denise.colorizer.pokeColorReg(24, value); return;
        case 0x1B2 >> 1: // COLOR25
            amiga->denise.colorizer.pokeColorReg(25, value); return;
        case 0x1B4 >> 1: // COLOR26
            amiga->denise.colorizer.pokeColorReg(26, value); return;
        case 0x1B6 >> 1: // COLOR27
            amiga->denise.colorizer.pokeColorReg(27, value); return;
        case 0x1B8 >> 1: // COLOR28
            amiga->denise.colorizer.pokeColorReg(28, value); return;
        case 0x1BA >> 1: // COLOR29
            amiga->denise.colorizer.pokeColorReg(29, value); return;
        case 0x1BC >> 1: // COLOR30
            amiga->denise.colorizer.pokeColorReg(30, value); return;
        case 0x1BE >> 1: // COLOR31
            amiga->denise.colorizer.pokeColorReg(31, value); return;
    }
    
    warn("pokeCustom16(%X [%s], %X): MISSING IMPLEMENTATION\n",
         addr, customReg[(addr >> 1) & 0xFF], value);
    amiga->pause();
}

void Memory::pokeCustom32(uint32_t addr, uint32_t value)
{
    pokeCustom16(addr,     HI_WORD(value));
    pokeCustom16(addr + 2, LO_WORD(value));
}





const char *
Memory::ascii(uint32_t addr)
{
    assert(is_uint24_t(addr));
    
    for (unsigned i = 0; i < 16; i++) {
        uint8_t value = peek8(addr + i);
        str[i] = isprint(value) ? value : '.';
    }
    str[16] = 0;
    return str;
}

const char *
Memory::hex(uint32_t addr, size_t bytes)
{
    assert(is_uint24_t(addr));
    assert(bytes % 2 == 0);
    assert(bytes <= 32);
    
    char *ptr = str;
    for (unsigned i = 0; i < bytes; i += 2) {
        
        uint16_t value = peek16(addr + i);
        sprint16x(ptr, value);
        ptr += 4;
        *ptr++ = ' ';
    }
    *ptr = 0;
    return str;
}
