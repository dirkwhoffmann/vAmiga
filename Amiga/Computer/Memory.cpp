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

/*
const uint32_t CHIP_RAM_START = 0x000000;
const uint32_t FAST_RAM_START = 0x200000;
const uint32_t SLOW_RAM_START = 0xC00000;
*/

const uint32_t FAST_RAM_START = 0x200000;

const uint32_t SLOW_RAM_MASK = 0x007FFFF;
const uint32_t BOOT_ROM_MASK = 0x003FFFF;
const uint32_t KICK_ROM_MASK = 0x003FFFF;


#define ASSERT_CHIP_ADDR(x) assert(chipRam != NULL);
#define ASSERT_FAST_ADDR(x) assert(fastRam != NULL); assert(((x) - FAST_RAM_START) < slowRamSize);
#define ASSERT_SLOW_ADDR(x) assert(slowRam != NULL); assert(((x) & SLOW_RAM_MASK) < slowRamSize);
#define ASSERT_BOOT_ADDR(x) assert(bootRom != NULL); assert(((x) & BOOT_ROM_MASK) < bootRomSize);
#define ASSERT_KICK_ADDR(x) assert(kickRom != NULL); assert(((x) & KICK_ROM_MASK) < kickRomSize);
#define ASSERT_CIA_ADDR(x)  assert((x) >= 0xA00000 && (x) <= 0xBFFFFF);
#define ASSERT_RTC_ADDR(x)  assert((x) >= 0xDC0000 && (x) <= 0xDEFFFF);
#define ASSERT_OCS_ADDR(x)  assert((x) >= 0xDF0000 && (x) <= 0xDFFFFF);

#define READ_CHIP_8(x)         (*(uint8_t *)(chipRam + (x % chipRamSize)))
#define READ_CHIP_16(x) (ntohs(*(uint16_t *)(chipRam + (x % chipRamSize))))
#define READ_CHIP_32(x) (ntohl(*(uint32_t *)(chipRam + (x % chipRamSize))))

#define READ_FAST_8(x)         (*(uint8_t *)(fastRam + ((x) - FAST_RAM_START)))
#define READ_FAST_16(x) (ntohs(*(uint16_t *)(fastRam + ((x) - FAST_RAM_START))))
#define READ_FAST_32(x) (ntohl(*(uint32_t *)(fastRam + ((x) - FAST_RAM_START))))

#define READ_SLOW_8(x)         (*(uint8_t *)(slowRam + ((x) & SLOW_RAM_MASK)))
#define READ_SLOW_16(x) (ntohs(*(uint16_t *)(slowRam + ((x) & SLOW_RAM_MASK))))
#define READ_SLOW_32(x) (ntohl(*(uint32_t *)(slowRam + ((x) & SLOW_RAM_MASK))))

#define READ_BOOT_8(x)         (*(uint8_t *)(bootRom + ((x) & BOOT_ROM_MASK)))
#define READ_BOOT_16(x) (ntohs(*(uint16_t *)(bootRom + ((x) & BOOT_ROM_MASK))))
#define READ_BOOT_32(x) (ntohl(*(uint32_t *)(bootRom + ((x) & BOOT_ROM_MASK))))

#define READ_KICK_8(x)         (*(uint8_t *)(kickRom + ((x) & KICK_ROM_MASK)))
#define READ_KICK_16(x) (ntohs(*(uint16_t *)(kickRom + ((x) & KICK_ROM_MASK))))
#define READ_KICK_32(x) (ntohl(*(uint32_t *)(kickRom + ((x) & KICK_ROM_MASK))))


#define WRITE_CHIP_8(x,y)   (*(uint8_t *)(chipRam + (x % chipRamSize)) = (y))
#define WRITE_CHIP_16(x,y) (*(uint16_t *)(chipRam + (x % chipRamSize)) = htons(y))
#define WRITE_CHIP_32(x,y) (*(uint32_t *)(chipRam + (x % chipRamSize)) = htonl(y))

#define WRITE_FAST_8(x,y)   (*(uint8_t *)(fastRam + ((x) - FAST_RAM_START)) = (y))
#define WRITE_FAST_16(x,y) (*(uint16_t *)(fastRam + ((x) - FAST_RAM_START)) = htons(y))
#define WRITE_FAST_32(x,y) (*(uint32_t *)(fastRam + ((x) - FAST_RAM_START)) = htonl(y))

#define WRITE_SLOW_8(x,y)   (*(uint8_t *)(slowRam + ((x) & SLOW_RAM_MASK)) = (y))
#define WRITE_SLOW_16(x,y) (*(uint16_t *)(slowRam + ((x) & SLOW_RAM_MASK)) = htons(y))
#define WRITE_SLOW_32(x,y) (*(uint32_t *)(slowRam + ((x) & SLOW_RAM_MASK)) = htonl(y))

#define WRITE_BOOT_8(x,y)   (*(uint8_t *)(bootRom + ((x) & BOOT_ROM_MASK)) = (y))
#define WRITE_BOOT_16(x,y) (*(uint16_t *)(bootRom + ((x) & BOOT_ROM_MASK)) = htons(y))
#define WRITE_BOOT_32(x,y) (*(uint32_t *)(bootRom + ((x) & BOOT_ROM_MASK)) = htonl(y))

#define WRITE_KICK_8(x,y)   (*(uint8_t *)(kickRom + ((x) & KICK_ROM_MASK)) = (y))
#define WRITE_KICK_16(x,y) (*(uint16_t *)(kickRom + ((x) & KICK_ROM_MASK)) = htons(y))
#define WRITE_KICK_32(x,y) (*(uint32_t *)(kickRom + ((x) & KICK_ROM_MASK)) = htonl(y))


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
        memset(ptrref, 0, allocSize); 
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
    poke16(addr + 2, HI_WORD(value));
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
    
    switch (addr & 0x1FE) {
            
        case 0x01C: // INTENAR
            return amiga->paula.peekINTENA();
            
        case 0x01E: // INTREQR
            return amiga->paula.peekINTREQ();
            
        default:
            warn("peekCustom16(%X): MISSING IMPLEMENTATION\n", addr);
    }
    
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
    
    switch (addr & 0x1FE) {
            
        case 0x09A: // INTENA
            amiga->paula.pokeINTENA(value);
            break;
            
        case  0x09C: // INTREQ
            amiga->paula.pokeINTREQ(value);
            break;
            
        default:
            warn("pokeCustom16(%X, %X): MISSING IMPLEMENTATION\n", addr, value);
    }
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
    for (unsigned i = 0; i < bytes / 2; i += 2) {
        
        uint16_t value = peek16(addr + i);
        sprint16x(ptr, value);
        ptr += 4;
        *ptr++ = ' ';
    }
    *ptr = 0;
    return str;
}
