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
    // Allocate memory
    /*
    allocateBootRom();
    allocateKickRom();
    allocateChipRam(amiga->config.chipRamSize);
    allocateFastRam(amiga->config.fastRamSize);
    allocateSlowRam(amiga->config.slowRamSize);
    */
    
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

/*
bool
AmigaMemory::allocateBootRom()
{
    // Only the Amiga 1000 has a Boot Rom
    if (amiga->config.model == A1000) {
        return alloc(KB(64), bootRom, bootRomSize);
    }
    dealloc(bootRom, bootRomSize);
    return true;
}

bool
AmigaMemory::allocateKickRom()
{
    return alloc(KB(256), kickRom, kickRomSize);
}

bool
AmigaMemory::allocateChipRam(size_t size)
{
    return alloc(size, chipRam, chipRamSize);
}

bool
AmigaMemory::allocateSlowRam(size_t size)
{
    return alloc(size, slowRam, slowRamSize);
}

bool
AmigaMemory::allocateFastRam(size_t size)
{
    return alloc(size, fastRam, fastRamSize);
}
*/

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
        if (!(ptrref = new (std::nothrow) uint8_t[KB(size)])) {
            warn("Cannot allocate %X bytes of memory\n", size);
            return false;
        }
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
    
    
    // Start from scratch
    for (unsigned i = 0x00; i <= 0xFF; i++)
        memSrc[i] = MEM_UNMAPPED;
    
    // Chip Ram
    for (unsigned i = 0; i < chipRamSize / 0x10000; i++)
        memSrc[i] = MEM_CHIP;
    
    // Install Fast Ram
    for (unsigned i = 0; i < fastRamSize / 0x10000; i++)
        memSrc[0x20 + i] = MEM_FAST;

    // CIA range
    for (unsigned i = 0xA0; i <= 0xBF; i++)
        memSrc[i] = MEM_CIA;

    // Slow Ram
    for (unsigned i = 0; i < slowRamSize / 0x10000; i++)
        memSrc[0xC0 + i] = MEM_SLOW;

    // Real-time clock
    for (unsigned i = 0xDC; rtc && i <= 0xDE; i++)
        memSrc[i] = MEM_RTC;

    // OCS
    for (unsigned i = 0xDF; i <= 0xDF; i++)
        memSrc[i] = MEM_OCS;

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

MemorySource
Memory::getMemSrc(uint32_t addr)
{
    assert(is_uint24_t(addr));
    return memSrc[addr >> 16];
}

uint8_t
Memory::peek8(uint32_t addr)
{
    addr &= 0xFFFFFF;
    switch (memSrc[addr >> 16]) {
            
        case MEM_UNMAPPED:
            return 0;
            
        case MEM_CHIP:
            assert(chipRam != NULL);
            return chipRam[addr % chipRamSize];
            
        case MEM_FAST:
            assert(fastRam != NULL);
            return fastRam[addr - 0x200000];
            
        case MEM_CIA:
            return peekCIA8(addr);
            
        case MEM_SLOW:
            assert(slowRam != NULL);
            return slowRam[addr - 0xC00000];
            
        case MEM_RTC:
            return 1;
            
        case MEM_OCS:
            return 2;
            
        case MEM_BOOT:
            assert(bootRom != NULL);
            return bootRom[addr % bootRomSize];
            
        case MEM_KICK:
            assert(kickRom != NULL);
            return kickRom[addr % kickRomSize];
            
        default:
            assert(false);
    }
    return 0;
}

uint16_t
Memory::peek16(uint32_t addr)
{
    addr &= 0xFFFFFF;
    return HI_LO(peek8(addr), peek8(addr+1));
}

uint32_t
Memory::peek32(uint32_t addr)
{
    addr &= 0xFFFFFF;
    return HI_HI_LO_LO(peek8(addr), peek8(addr+1), peek8(addr+2), peek8(addr+3));
}

uint8_t
Memory::spypeek8(uint32_t addr)
{
    addr &= 0xFFFFFF;
    return peek8(addr);
}
uint16_t
Memory::spypeek16(uint32_t addr)
{
    addr &= 0xFFFFFF;
    return peek16(addr);
}
uint32_t
Memory::spypeek32(uint32_t addr)
{
    addr &= 0xFFFFFF;
    return peek32(addr);
}

void
Memory::poke8(uint32_t addr, uint8_t value)
{
    addr &= 0xFFFFFF;
    switch (memSrc[addr >> 16]) {
            
        case MEM_CHIP:
            assert(chipRam != NULL);
            chipRam[addr % chipRamSize] = value;
            return;
            
        case MEM_FAST:
            assert(fastRam != NULL);
            fastRam[addr - 0x200000] = value;
            return;
            
        case MEM_CIA:
            pokeCIA8(addr, value);
            return;
            
        case MEM_SLOW:
            assert(slowRam != NULL);
            slowRam[addr - 0xC00000] = value;
            return;
            
        case MEM_RTC:
            return;
            
        case MEM_OCS:
            return;
            
        case MEM_BOOT:
            assert(bootRom != NULL);
            bootRom[addr % bootRomSize] = value;
            return;
            
        case MEM_KICK:
            assert(kickRom != NULL);
            kickRom[addr % kickRomSize] = value;
            return;
            
        default:
            assert(false);
    }
}
void
Memory::poke16(uint32_t addr, uint16_t value)
{
    assert(is_uint24_t(addr));
    debug("Poking %04X to %06X.", value, addr);
}
void
Memory::poke32(uint32_t addr, uint32_t value)
{
    assert(is_uint24_t(addr));
    debug("Poking %04X to %06X.", value, addr);
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
    return (peekCIA16(addr) << 16) | peekCIA16(addr + 2);
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
        return HI_BYTE(peekCustomReg(addr));
    } else {
        return LO_BYTE(peekCustomReg(addr - 1));
    }
}

uint16_t
Memory::peekCustom16(uint32_t addr)
{
    if (IS_EVEN(addr)) {
        return peekCustomReg(addr);
    } else {
        uint8_t byte1 = LO_BYTE(peekCustomReg(addr - 1));
        uint8_t byte2 = HI_BYTE(peekCustomReg(addr + 1));
        return HI_LO(byte1, byte2);
    }
}

uint32_t
Memory::peekCustom32(uint32_t addr)
{
   return HI_W_LO_W(peekCustom16(addr), peekCustom16(addr + 2));
}

uint16_t
Memory::peekCustomReg(uint32_t addr)
{
    // This function required that addr is word aligned
    assert(IS_EVEN(addr));
    
    switch (addr & 0x1FE) {
            
        case 0x01C: // INTENAR
            return amiga->paula.getINTENA();
        case 0x01E: // INTREQR
            return amiga->paula.getINTREQ();
            
        default:
            warn("peekCustom16(%X): MISSING IMPLEMENTATION\n", addr);
    }
    
    return 42;
}

uint8_t
Memory::spypeekCustom8(uint32_t addr)
{
    if (IS_EVEN(addr)) {
        return HI_BYTE(spypeekCustomReg(addr));
    } else {
        return LO_BYTE(spypeekCustomReg(addr - 1));
    }
}

uint16_t
Memory::spypeekCustom16(uint32_t addr)
{
    if (IS_EVEN(addr)) {
        return spypeekCustomReg(addr);
    } else {
        uint8_t byte1 = LO_BYTE(spypeekCustomReg(addr - 1));
        uint8_t byte2 = HI_BYTE(spypeekCustomReg(addr + 1));
        return HI_LO(byte1, byte2);
    }
}

uint32_t
Memory::spypeekCustom32(uint32_t addr)
{
    return HI_W_LO_W(spypeekCustom16(addr), spypeekCustom16(addr + 2));
}

uint16_t
Memory::spypeekCustomReg(uint32_t addr)
{
    return 42; 
}

void
Memory::pokeCustom8(uint32_t addr, uint8_t value)
{
    pokeCustom16(addr & 0x1FE, HI_LO(value,value));
}

void
Memory::pokeCustom16(uint32_t addr, uint16_t value)
{
    if (IS_EVEN(addr)) {
        pokeCustomReg(addr, value);
    } else {
        uint8_t hi = HI_BYTE(value);
        uint8_t lo = LO_BYTE(value);
        pokeCustomReg(addr - 1, HI_LO(hi,hi));
        pokeCustomReg(addr + 1, HI_LO(lo,lo));
    }
}

void Memory::pokeCustom32(uint32_t addr, uint32_t value)
{
    pokeCustom16((addr & 0x1FE), HI_WORD(value));
    pokeCustom16((addr & 0x1FE) + 2, LO_WORD(value));
}

void
Memory::pokeCustomReg(uint32_t addr, uint16_t value)
{
    switch (addr & 0x1FE) {
            
        case 0x09A: // INTENA
            amiga->paula.setINTENA(value);
            break;
            
        case  0x09C: // INTREQ
            amiga->paula.setINTREQ(value);
            break;
            
        default:
            warn("pokeCustom16(%X, %X): MISSING IMPLEMENTATION\n", addr, value);
    }
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
    for (unsigned i = 0; i < bytes / 2; i++) {
        
        uint16_t value = peek16(addr + i);
        sprint16x(ptr, value);
        ptr += 4;
        *ptr++ = ' ';
    }
    *ptr = 0;
    return str;
}
