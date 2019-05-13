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
    
    registerSnapshotItems(vector<SnapshotItem> {

        { &kickIsWritable,  sizeof(kickIsWritable),  0 },
        { &memSrc,          sizeof(memSrc),          0 },
        
    });
}

Memory::~Memory()
{
    dealloc();
}

void
Memory::dealloc()
{
    if (bootRom) { delete[] bootRom; bootRom = NULL; }
    if (kickRom) { delete[] kickRom; kickRom = NULL; }
    if (chipRam) { delete[] chipRam; chipRam = NULL; }
    if (slowRam) { delete[] slowRam; slowRam = NULL; }
    if (fastRam) { delete[] fastRam; fastRam = NULL; }
}

void
Memory::_powerOn()
{
    // Make Rom writable if an A1000 is emulated
    kickIsWritable = _amiga->getConfig().model == AMIGA_1000;
    
    // Wipe out RAM
    if (chipRam) memset(chipRam, 0, chipRamSize);
    if (slowRam) memset(slowRam, 0, slowRamSize);
    if (fastRam) memset(fastRam, 0, fastRamSize);

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

size_t
Memory::stateSize()
{
    size_t result = HardwareComponent::stateSize();
    
    result += sizeof(uint32_t) + bootRomSize;
    result += sizeof(uint32_t) + kickRomSize;
    result += sizeof(uint32_t) + chipRamSize;
    result += sizeof(uint32_t) + slowRamSize;
    result += sizeof(uint32_t) + fastRamSize;
    
    return result;
}

void
Memory::didLoadFromBuffer(uint8_t **buffer)
{
    // Load memory size information
    bootRomSize = (size_t)read32(buffer);
    kickRomSize = (size_t)read32(buffer);
    chipRamSize = (size_t)read32(buffer);
    slowRamSize = (size_t)read32(buffer);
    fastRamSize = (size_t)read32(buffer);
    
    // Do some consistency checks
    // Note: We should do this a little less agressive, e.g., by returning
    // false. Furthermore, the real maximum size limits should be used.
    assert(bootRomSize < 0xFFFFFF);
    assert(kickRomSize < 0xFFFFFF);
    assert(chipRamSize < 0xFFFFFF);
    assert(slowRamSize < 0xFFFFFF);
    assert(fastRamSize < 0xFFFFFF);
    
    // Free previously allocated memory
    dealloc();

    // Allocate new memory
    if (bootRomSize) bootRom = new (std::nothrow) uint8_t[bootRomSize + 3];
    if (kickRomSize) kickRom = new (std::nothrow) uint8_t[kickRomSize + 3];
    if (chipRamSize) chipRam = new (std::nothrow) uint8_t[chipRamSize + 3];
    if (slowRamSize) slowRam = new (std::nothrow) uint8_t[slowRamSize + 3];
    if (fastRamSize) fastRam = new (std::nothrow) uint8_t[fastRamSize + 3];

    // Load memory contents from buffer
    readBlock(buffer, bootRom, bootRomSize);
    readBlock(buffer, kickRom, kickRomSize);
    readBlock(buffer, chipRam, chipRamSize);
    readBlock(buffer, slowRam, slowRamSize);
    readBlock(buffer, fastRam, fastRamSize);
}

void
Memory::didSaveToBuffer(uint8_t **buffer)
{
    // Save memory size information
    write32(buffer, (uint32_t)bootRomSize);
    write32(buffer, (uint32_t)kickRomSize);
    write32(buffer, (uint32_t)chipRamSize);
    write32(buffer, (uint32_t)slowRamSize);
    write32(buffer, (uint32_t)fastRamSize);

    // Save memory contents
    writeBlock(buffer, bootRom, bootRomSize);
    writeBlock(buffer, kickRom, kickRomSize);
    writeBlock(buffer, chipRam, chipRamSize);
    writeBlock(buffer, slowRam, slowRamSize);
    writeBlock(buffer, fastRam, fastRamSize);
    
    /*
    debug("bootRomSize = %d\n", bootRomSize);
    debug("kickRomSize = %d\n", kickRomSize);
    debug("chipRamSize = %d\n", chipRamSize);
    debug("slowRamSize = %d\n", slowRamSize);
    debug("fastRamSize = %d\n", fastRamSize);
    */
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
        
        // Note: We allocate three bytes more than we need to handle the case
        // that a long word access is performed on the last memory address.
        size_t allocSize = size + 3;
        
        if (!(ptrref = new (std::nothrow) uint8_t[allocSize])) {
            warn("Cannot allocate %d KB of memory\n", size);
            return false;
        }
        memset(ptrref, 0, allocSize);
        sizeref = size;
    }
    
    // Update the memory lookup table
    updateMemSrcTable();
    
    return true;
}

void
Memory::loadRom(AmigaFile *rom, uint8_t *target, size_t length)
{
    if (rom) {
        
        assert(target != NULL);
        memset(target, 0, length);
        
        rom->seek(0);
        
        int c;
        for (size_t i = 0; i < length; i++) {
            if ((c = rom->read()) == EOF) break;
            *(target++) = c;
        }
    }
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
Memory::loadKickRom(KickRom *rom)
{
    assert(rom != NULL);
    
    if (!alloc(rom->getSize(), kickRom, kickRomSize))
        return false;
    
    loadRom(rom, kickRom, kickRomSize);
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

void
Memory::updateMemSrcTable()
{
    AmigaConfiguration config = _amiga->getConfig();
    MemorySource mem_boot = bootRom ? MEM_BOOT : MEM_UNMAPPED;
    MemorySource mem_kick = kickRom ? MEM_KICK : MEM_UNMAPPED;
    
    assert(chipRamSize % 0x10000 == 0);
    assert(slowRamSize % 0x10000 == 0);
    assert(fastRamSize % 0x10000 == 0);

    bool rtc = _amiga ? config.realTimeClock : false;
    bool ovl = _amiga ? (_ciaA->getPA() & 1) : false;
    
    // debug("updateMemSrcTable: rtc = %d ovl = %d\n", rtc, ovl);
    
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

    // OCS (some assignments will be overwritten below by Slow Ram and RTC)
    for (unsigned i = 0xC0; i <= 0xDF; i++)
        memSrc[i] = MEM_OCS;
    
    // Slow Ram
    for (unsigned i = 0; i < slowRamSize / 0x10000; i++)
        memSrc[0xC0 + i] = MEM_SLOW;

    // Real-time clock (RTC)
    for (unsigned i = 0xDC; rtc && i <= 0xDE; i++)
        memSrc[i] = MEM_RTC;

    // Auto-config (Zorro II)
    for (unsigned i = 0xE8; i <= 0xEF; i++)
    memSrc[i] = MEM_AUTOCONF;
    
    // Boot Rom or Kickstart mirror
    for (unsigned i = 0xF8; i <= 0xFB; i++)
        memSrc[i] = kickIsWritable ? mem_boot : mem_kick;

    // Kickstart
    for (unsigned i = 0xFC; i <= 0xFF; i++)
        memSrc[i] = mem_kick;

    // Overlay Rom with lower memory area if the OVL line is high
    for (unsigned i = 0; ovl && i < 8 && memSrc[0xF8 + i] != MEM_UNMAPPED; i++)
        memSrc[i] = memSrc[0xF8 + i];
    
    if (_amiga) _amiga->putMessage(MSG_MEM_LAYOUT);
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
        case MEM_RTC:      ASSERT_RTC_ADDR(addr);  return peekRTC8(addr);
        case MEM_OCS:      ASSERT_OCS_ADDR(addr);  return peekCustom8(addr);
        case MEM_AUTOCONF: ASSERT_AUTO_ADDR(addr); return peekAutoConf8(addr);
        case MEM_BOOT:     ASSERT_BOOT_ADDR(addr); assert(false); return READ_BOOT_8(addr);
        case MEM_KICK:     ASSERT_KICK_ADDR(addr); return READ_KICK_8(addr);
        default:           assert(false);
    }
    return 0;
}

uint16_t
Memory::peek16(uint32_t addr)
{
    if (!IS_EVEN(addr)) {
        debug("PC: %X peek16(%X) memSrc = %d\n", _cpu->getPC(), addr, memSrc[(addr & 0xFFFFFF) >> 16]);
        _amiga->dump();
    }
    
    assert(IS_EVEN(addr));
    
    addr &= 0xFFFFFF;
    switch (memSrc[addr >> 16]) {
            
        case MEM_UNMAPPED: return 0;
        case MEM_CHIP:     ASSERT_CHIP_ADDR(addr); return READ_CHIP_16(addr);
        case MEM_FAST:     ASSERT_FAST_ADDR(addr); return READ_FAST_16(addr);
        case MEM_CIA:      ASSERT_CIA_ADDR(addr);  return peekCIA16(addr);
        case MEM_SLOW:     ASSERT_SLOW_ADDR(addr); return READ_SLOW_16(addr);
        case MEM_RTC:      ASSERT_RTC_ADDR(addr);  return peekRTC16(addr);
        case MEM_OCS:      ASSERT_OCS_ADDR(addr);  return peekCustom16(addr);
        case MEM_AUTOCONF: ASSERT_AUTO_ADDR(addr); return peekAutoConf16(addr);
        case MEM_BOOT:     ASSERT_BOOT_ADDR(addr); assert(false); return READ_BOOT_16(addr);
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
        case MEM_RTC:      ASSERT_RTC_ADDR(addr);  return spypeekRTC8(addr);
        case MEM_OCS:      ASSERT_OCS_ADDR(addr);  return spypeekCustom8(addr);
        case MEM_AUTOCONF: ASSERT_AUTO_ADDR(addr); return spypeekAutoConf8(addr);
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
        case MEM_RTC:      ASSERT_RTC_ADDR(addr);  return spypeekRTC8(addr);
        case MEM_OCS:      ASSERT_OCS_ADDR(addr);  return spypeekCustom16(addr);
        case MEM_AUTOCONF: ASSERT_AUTO_ADDR(addr); return spypeekAutoConf16(addr);
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
        case MEM_RTC:      ASSERT_RTC_ADDR(addr);  pokeRTC8(addr, value); break;
        case MEM_OCS:      ASSERT_OCS_ADDR(addr);  pokeCustom8(addr, value); break;
        case MEM_AUTOCONF: ASSERT_AUTO_ADDR(addr); pokeAutoConf8(addr, value); break;
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
        case MEM_RTC:      ASSERT_RTC_ADDR(addr);  pokeRTC16(addr, value);;
        case MEM_OCS:      ASSERT_OCS_ADDR(addr);  pokeCustom16(addr, value); break;
        case MEM_AUTOCONF: ASSERT_AUTO_ADDR(addr); pokeAutoConf16(addr, value); break;
        case MEM_BOOT:     ASSERT_BOOT_ADDR(addr); break;
        case MEM_KICK:     ASSERT_KICK_ADDR(addr); break;
        default:           assert(false);
    }
}

void
Memory::poke32(uint32_t addr, uint32_t value)
{
    /*
    if ((addr & 0xDF0000) == 0xDF0000)
        debug("PC: %X poke32(%X,%X)\n", amiga->cpu.getPC(), addr, value);
    */
    
    poke16(addr,     HI_WORD(value));
    poke16(addr + 2, LO_WORD(value));
}

uint8_t
Memory::peekCIA8(uint32_t addr)
{
    // debug("peekCIA8(%6X)\n", addr);
    
    uint32_t reg = (addr >> 8)  & 0b1111;
    uint32_t sel = (addr >> 12) & 0b11;
    bool a0 = addr & 1;
    
    switch (sel) {
            
        case 0b00:
            return a0 ? _ciaA->peek(reg) : _ciaB->peek(reg);
            
        case 0b01:
            return a0 ? LO_BYTE(_cpu->getIR()) : _ciaB->peek(reg);
            
        case 0b10:
            return a0 ? _ciaA->peek(reg) : HI_BYTE(_cpu->getIR());
            
        case 0b11:
            return a0 ? LO_BYTE(_cpu->getIR()) : HI_BYTE(_cpu->getIR());
    }
    assert(false);
    return 0;
}

uint16_t
Memory::peekCIA16(uint32_t addr)
{
    debug("peekCIA16(%6X)\n", addr);
    // assert(false);
    
    uint32_t reg = (addr >> 8)  & 0b1111;
    uint32_t sel = (addr >> 12) & 0b11;
    
    switch (sel) {
            
        case 0b00:
            return HI_LO(_ciaB->peek(reg), _ciaA->peek(reg));
            
        case 0b01:
            return HI_LO(_ciaB->peek(reg), 0xFF);
            
        case 0b10:
            return HI_LO(0xFF, _ciaA->peek(reg));
            
        case 0b11:
            return _cpu->getIR();
            
    }
    assert(false);
    return 0;
}

uint32_t
Memory::peekCIA32(uint32_t addr)
{
    debug("peekCIA32(%6X)\n", addr);
    assert(false);
    
    return HI_W_LO_W(peekCIA16(addr), peekCIA16(addr + 2));
}

uint8_t
Memory::spypeekCIA8(uint32_t addr)
{
    uint32_t reg = (addr >> 8)  & 0b1111;
    uint32_t sel = (addr >> 12) & 0b11;
    bool a0 = addr & 1;
    
    switch (sel) {
            
        case 0b00:
            return a0 ? _ciaA->spypeek(reg) : _ciaB->spypeek(reg);
            
        case 0b01:
            return a0 ? LO_BYTE(_cpu->getIR()) : _ciaB->spypeek(reg);
            
        case 0b10:
            return a0 ? _ciaA->spypeek(reg) : HI_BYTE(_cpu->getIR());
            
        case 0b11:
            return a0 ? LO_BYTE(_cpu->getIR()) : HI_BYTE(_cpu->getIR());
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
            return HI_LO(_ciaB->spypeek(reg), _ciaA->spypeek(reg));
            
        case 0b01:
            return HI_LO(_ciaB->spypeek(reg), 0xFF);
            
        case 0b10:
            return HI_LO(0xFF, _ciaA->spypeek(reg));
            
        case 0b11:
            return _cpu->getIR();
            
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
    // debug("pokeCIA8(%6X, %X)\n", addr, value);
    
    uint32_t reg = (addr >> 8) & 0b1111;
    uint32_t selA = (addr & 0x1000) == 0;
    uint32_t selB = (addr & 0x2000) == 0;

    if (selA) _ciaA->poke(reg, value);
    if (selB) _ciaB->poke(reg, value);
}

void
Memory::pokeCIA16(uint32_t addr, uint16_t value)
{
    debug("pokeCIA16(%6X, %X)\n", addr, value);
    assert(false);
    
    assert(IS_EVEN(addr));
    
    uint32_t reg = (addr >> 8) & 0b1111;
    uint32_t selA = (addr & 0x1000) == 0;
    uint32_t selB = (addr & 0x2000) == 0;
    
    if (selA) _ciaA->poke(reg, LO_BYTE(value));
    if (selB) _ciaB->poke(reg, HI_BYTE(value));
}

void
Memory::pokeCIA32(uint32_t addr, uint32_t value)
{
    debug("pokeCIA32(%6X, %X)\n", addr, value);
    assert(false);
    
    pokeCIA16(addr,     HI_WORD(value));
    pokeCIA16(addr + 2, LO_WORD(value));
}

uint8_t
Memory::peekRTC8(uint32_t addr)
{
    /* Addr: 0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011
     * Reg:   --   -0   --   -0   --   -1   --   -1   --   -2   --   -2
     */
    if (IS_EVEN(addr)) return 0;
    
    /* Addr: 0001 0011 0101 0111 1001 1011
     * Reg:   -0   -0   -1   -1   -2   -2
     */
    return _amiga->rtc.peek((addr >> 2) & 0b1111);
}

uint16_t
Memory::peekRTC16(uint32_t addr)
{
    return HI_LO(peekRTC8(addr), peekRTC8(addr + 1));
}

void
Memory::pokeRTC8(uint32_t addr, uint8_t value)
{
    /* Addr: 0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011
     * Reg:   --   -0   --   -0   --   -1   --   -1   --   -2   --   -2
     */
    if (IS_EVEN(addr)) return;
    
    /* Addr: 0001 0011 0101 0111 1001 1011
     * Reg:   -0   -0   -1   -1   -2   -2
     */
    _amiga->rtc.poke((addr >> 2) & 0b1111, value);
}

void
Memory::pokeRTC16(uint32_t addr, uint16_t value)
{
    pokeRTC8(addr, HI_BYTE(value));
    pokeRTC8(addr + 1, LO_BYTE(value));
}

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
            
        case 0x000 >> 1: // BLTDDAT
            return 0xFF;
        case 0x002 >> 1: // DMACONR
            return _agnus->peekDMACONR();
        case 0x004 >> 1: // VPOSR
            return _agnus->peekVPOSR();
        case 0x006 >> 1: // VHPOSR
            return _agnus->peekVHPOSR();
        case 0x008 >> 1: // DSKDATR
            return _paula->diskController.peekDSKDATR();
        case 0x00A >> 1: // JOY0DAT
            return _denise->peekJOY0DATR();
        case 0x00C >> 1: // JOY1DAT
            return _denise->peekJOY1DATR();
        case 0x00E >> 1: // CLXDAT
            break;
        case 0x010 >> 1: // ADKCONR
            return _paula->peekADKCONR();
        case 0x012 >> 1: // POT0DAT
            break;
        case 0x014 >> 1: // POT1DAT
            break;
        case 0x016 >> 1: // POTGOR
            return _paula->peekPOTGOR();
        case 0x018 >> 1: // SERDATR
            return _paula->peekSERDATR();
        case 0x01A >> 1: // DSKBYTR
            return _paula->diskController.peekDSKBYTR();
        case 0x01C >> 1: // INTENAR
            return _paula->peekINTENAR();
        case 0x01E >> 1: // INTREQR
            return _paula->peekINTREQR();

        default: // Write-only register
            return 0xFF;
    }
    
    warn("peekCustom16(%X [%s]): MISSING IMPLEMENTATION\n",
         addr, customReg[(addr >> 1) & 0xFF]);
    _amiga->pause();
    return 42;
}

uint32_t
Memory::peekCustom32(uint32_t addr)
{
    assert(false);
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

    return 42;
    // return peekCustom16(addr);
}

uint32_t
Memory::spypeekCustom32(uint32_t addr)
{
    return HI_W_LO_W(spypeekCustom16(addr), spypeekCustom16(addr + 2));
}

void
Memory::pokeCustom8(uint32_t addr, uint8_t value)
{
    assert(false);
    pokeCustom16(addr & 0x1FE, HI_LO(value,value));
}

void
Memory::pokeCustom16(uint32_t addr, uint16_t value)
{
    assert(IS_EVEN(addr));
    
    switch ((addr >> 1) & 0xFF) {

        case 0x020 >> 1: // DSKPTH
            _agnus->pokeDSKPTH(value); return;
        case 0x022 >> 1: // DSKPTL
            _agnus->pokeDSKPTL(value); return;
        case 0x024 >> 1: // DSKLEN
            _paula->diskController.pokeDSKLEN(value); return;
        case 0x026 >> 1: // DSKDAT
            _paula->diskController.pokeDSKDAT(value); return;
        case 0x28 >> 1: // REFPTR
            return;
        case 0x02A >> 1: // VPOSW
            _agnus->pokeVPOS(value); return;
        case 0x02C >> 1: // VHPOSW
            _agnus->pokeVHPOS(value); return;
        case 0x02E >> 1: // COPCON
            _agnus->copper.pokeCOPCON(value); return;
        case 0x030 >> 1: // SERDAT
            _paula->pokeSERDAT(value); return;
        case 0x032 >> 1: // SERPER
            _paula->pokeSERPER(value); return;
        case 0x034 >> 1: // POTGO
            _paula->pokePOTGO(value); return;
        case 0x036 >> 1: // JOYTEST
            _denise->pokeJOYTEST(value); return;
        case 0x038 >> 1: // STREQU
        case 0x03A >> 1: // STRVBL
        case 0x03C >> 1: // STRHOR
        case 0x03E >> 1: // STRLONG
            return; // ignore
        case 0x040 >> 1: // BLTCON0
            _agnus->blitter.pokeBLTCON0(value); return;
        case 0x042 >> 1: // BLTCON1
            _agnus->blitter.pokeBLTCON1(value); return;
        case 0x044 >> 1: // BLTAFWM
            _agnus->blitter.pokeBLTAFWM(value); return;
        case 0x046 >> 1: // BLTALWM
            _agnus->blitter.pokeBLTALWM(value); return;
        case 0x048 >> 1: // BLTCPTH
            _agnus->blitter.pokeBLTCPTH(value); return;
        case 0x04A >> 1: // BLTCPTL
            _agnus->blitter.pokeBLTCPTL(value); return;
        case 0x04C >> 1: // BLTBPTH
            _agnus->blitter.pokeBLTBPTH(value); return;
        case 0x04E >> 1: // BLTBPTL
            _agnus->blitter.pokeBLTBPTL(value); return;
        case 0x050 >> 1: // BLTAPTH
            _agnus->blitter.pokeBLTAPTH(value); return;
        case 0x052 >> 1: // BLTAPTL
            _agnus->blitter.pokeBLTAPTL(value); return;
        case 0x054 >> 1: // BLTDPTH
            _agnus->blitter.pokeBLTDPTH(value); return;
        case 0x056 >> 1: // BLTDPTL
            _agnus->blitter.pokeBLTDPTL(value); return;
        case 0x058 >> 1: // BLTSIZE
            _agnus->blitter.pokeBLTSIZE(value); return;
        case 0x05A >> 1: // unused
        case 0x05C >> 1: // unused
        case 0x05E >> 1: // unused
            return;
        case 0x060 >> 1: // BLTCMOD
            _agnus->blitter.pokeBLTCMOD(value); return;
        case 0x062 >> 1: // BLTBMOD
            _agnus->blitter.pokeBLTBMOD(value); return;
        case 0x064 >> 1: // BLTAMOD
            _agnus->blitter.pokeBLTAMOD(value); return;
        case 0x066 >> 1: // BLTDMOD
            _agnus->blitter.pokeBLTDMOD(value); return;
        case 0x068 >> 1: // unused
        case 0x06A >> 1: // unused
        case 0x06C >> 1: // unused
        case 0x06E >> 1: // unused
            return;
        case 0x070 >> 1: // BLTCDAT
            _agnus->blitter.pokeBLTCDAT(value); return;
        case 0x072 >> 1: // BLTBDAT
            _agnus->blitter.pokeBLTBDAT(value); return;
        case 0x074 >> 1: // BLTADAT
            _agnus->blitter.pokeBLTADAT(value); return;
        case 0x076 >> 1: // unused
        case 0x078 >> 1: // unused
        case 0x07A >> 1: // unused
        case 0x07C >> 1: // unused
            return;
        case 0x07E >> 1: // DSKSYNC
            _paula->diskController.pokeDSKSYNC(value); return;
        case 0x080 >> 1: // COP1LCH
            _agnus->copper.pokeCOPxLCH(0, value); return;
        case 0x082 >> 1: // COP1LCL
            _agnus->copper.pokeCOPxLCL(0, value); return;
        case 0x084 >> 1: // COP2LCH
            _agnus->copper.pokeCOPxLCH(1, value); return;
        case 0x086 >> 1: // COP2LCL
            _agnus->copper.pokeCOPxLCL(1, value); return;
        case 0x088 >> 1: // COPJMP1
            _agnus->copper.pokeCOPJMP(0); return;
        case 0x08A >> 1: // COPJMP1
            _agnus->copper.pokeCOPJMP(1); return;
        case 0x08C >> 1: // COPINS
            _agnus->copper.pokeCOPINS(value); return;
        case 0x08E >> 1: // DIWSTRT
            _agnus->pokeDIWSTRT(value); return;
        case 0x090 >> 1: // DIWSTOP
            _agnus->pokeDIWSTOP(value); return;
        case 0x092 >> 1: // DDFSTRT
            _agnus->pokeDDFSTRT(value); return;
        case 0x094 >> 1: // DDFSTOP
            _agnus->pokeDDFSTOP(value); return;
        case 0x096 >> 1: // DMACON
            _agnus->pokeDMACON(value); return;
        
        case 0x098 >> 1:  // CLXCON
        warn("pokeCustom16(CLXCON, %X): MISSING IMPLEMENTATION\n", value);
        return;
        
        case 0x09A >> 1: // INTENA
            _paula->pokeINTENA(value); return;
        case 0x09C >> 1: // INTREQ
            _paula->pokeINTREQ(value); return;
        case 0x09E >> 1: // ADKCON
            _paula->pokeADKCON(value); return;
        case 0x0A0 >> 1: // AUD0LCH
            _agnus->pokeAUDxLCH(0, value); return;
        case 0x0A2 >> 1: // AUD0LCL
            _agnus->pokeAUDxLCL(0, value); return;
        case 0x0A4 >> 1: // AUD0LEN
            _paula->audioUnit.pokeAUDxLEN(0, value); return;
        case 0x0A6 >> 1: // AUD0PER
            _paula->audioUnit.pokeAUDxPER(0, value); return;
        case 0x0A8 >> 1: // AUD0VOL
            _paula->audioUnit.pokeAUDxVOL(0, value); return;
        case 0x0AA >> 1: // AUD0DAT
            _paula->audioUnit.pokeAUDxDAT(0, value); return;
        case 0x0AC >> 1: // Unused
        case 0x0AE >> 1: // Unused
            return;
        case 0x0B0 >> 1: // AUD1LCH
            _agnus->pokeAUDxLCH(1, value); return;
        case 0x0B2 >> 1: // AUD1LCL
            _agnus->pokeAUDxLCL(1, value); return;
        case 0x0B4 >> 1: // AUD1LEN
            _paula->audioUnit.pokeAUDxLEN(1, value); return;
        case 0x0B6 >> 1: // AUD1PER
            _paula->audioUnit.pokeAUDxPER(1, value); return;
        case 0x0B8 >> 1: // AUD1VOL
            _paula->audioUnit.pokeAUDxVOL(1, value); return;
        case 0x0BA >> 1: // AUD1DAT
            _paula->audioUnit.pokeAUDxDAT(1, value); return;
        case 0x0BC >> 1: // Unused
        case 0x0BE >> 1: // Unused
            return;
        case 0x0C0 >> 1: // AUD2LCH
            _agnus->pokeAUDxLCH(2, value); return;
        case 0x0C2 >> 1: // AUD2LCL
            _agnus->pokeAUDxLCL(2, value); return;
        case 0x0C4 >> 1: // AUD2LEN
            _paula->audioUnit.pokeAUDxLEN(2, value); return;
        case 0x0C6 >> 1: // AUD2PER
            _paula->audioUnit.pokeAUDxPER(2, value); return;
        case 0x0C8 >> 1: // AUD2VOL
            _paula->audioUnit.pokeAUDxVOL(2, value); return;
        case 0x0CA >> 1: // AUD2DAT
            _paula->audioUnit.pokeAUDxDAT(2, value); return;
        case 0x0CC >> 1: // Unused
        case 0x0CE >> 1: // Unused
            return;
        case 0x0D0 >> 1: // AUD3LCH
            _agnus->pokeAUDxLCH(3, value); return;
        case 0x0D2 >> 1: // AUD3LCL
            _agnus->pokeAUDxLCL(3, value); return;
        case 0x0D4 >> 1: // AUD3LEN
            _paula->audioUnit.pokeAUDxLEN(3, value); return;
        case 0x0D6 >> 1: // AUD3PER
            _paula->audioUnit.pokeAUDxPER(3, value); return;
        case 0x0D8 >> 1: // AUD3VOL
            _paula->audioUnit.pokeAUDxVOL(3, value); return;
        case 0x0DA >> 1: // AUD3DAT
            _paula->audioUnit.pokeAUDxDAT(3, value); return;
        case 0x0DC >> 1: // Unused
        case 0x0DE >> 1: // Unused
            return;
        case 0x0E0 >> 1: // BPL1PTH
            _agnus->pokeBPLxPTH(0, value); return;
        case 0x0E2 >> 1: // BPL1PTL
            _agnus->pokeBPLxPTL(0, value); return;
        case 0x0E4 >> 1: // BPL2PTH
            _agnus->pokeBPLxPTH(1, value); return;
        case 0x0E6 >> 1: // BPL2PTL
            _agnus->pokeBPLxPTL(1, value); return;
        case 0x0E8 >> 1: // BPL3PTH
            _agnus->pokeBPLxPTH(2, value); return;
        case 0x0EA >> 1: // BPL3PTL
            _agnus->pokeBPLxPTL(2, value); return;
        case 0x0EC >> 1: // BPL4PTH
            _agnus->pokeBPLxPTH(3, value); return;
        case 0x0EE >> 1: // BPL4PTL
            _agnus->pokeBPLxPTL(3, value); return;
        case 0x0F0 >> 1: // BPL5PTH
            _agnus->pokeBPLxPTH(4, value); return;
        case 0x0F2 >> 1: // BPL5PTL
            _agnus->pokeBPLxPTL(4, value); return;
        case 0x0F4 >> 1: // BPL6PTH
            _agnus->pokeBPLxPTH(5, value); return;
        case 0x0F6 >> 1: // BPL6PTL
            _agnus->pokeBPLxPTL(5, value); return;
        case 0x0F8 >> 1: // Unused
        case 0x0FA >> 1: // Unused
        case 0x0FC >> 1: // Unused
        case 0x0FE >> 1: // Unused
            return;
        case 0x100 >> 1: // BPLCON0
            _denise->pokeBPLCON0(value); return;
        case 0x102 >> 1: // BPLCON1
            _denise->pokeBPLCON1(value); return;
        case 0x104 >> 1: // BPLCON2
            _denise->pokeBPLCON2(value); return;
        case 0x106 >> 1: // Unused
            return;
        case 0x108 >> 1: // BPL1MOD
            _agnus->pokeBPL1MOD(value); return;
        case 0x10A >> 1: // BPL2MOD
            _agnus->pokeBPL2MOD(value); return;
        case 0x10C >> 1: // Unused
        case 0x10E >> 1: // Unused
            return;
        case 0x110 >> 1: // BPL1DAT
            _denise->pokeBPLxDAT(0, value); return;
        case 0x112 >> 1: // BPL2DAT
            _denise->pokeBPLxDAT(1, value); return;
        case 0x114 >> 1: // BPL3DAT
            _denise->pokeBPLxDAT(2, value); return;
        case 0x116 >> 1: // BPL4DAT
            _denise->pokeBPLxDAT(3, value); return;
        case 0x118 >> 1: // BPL5DAT
            _denise->pokeBPLxDAT(4, value); return;
        case 0x11A >> 1: // BPL6DAT
            _denise->pokeBPLxDAT(5, value); return;
        case 0x11C >> 1: // Unused
        case 0x11E >> 1: // Unused
            return;
        case 0x120 >> 1: // SPR0PTH
            _agnus->pokeSPRxPTH(0, value); return;
        case 0x122 >> 1: // SPR0PTL
            _agnus->pokeSPRxPTL(0, value); return;
        case 0x124 >> 1: // SPR1PTH
            _agnus->pokeSPRxPTH(1, value); return;
        case 0x126 >> 1: // SPR1PTL
            _agnus->pokeSPRxPTL(1, value); return;
        case 0x128 >> 1: // SPR2PTH
            _agnus->pokeSPRxPTH(2, value); return;
        case 0x12A >> 1: // SPR2PTL
            _agnus->pokeSPRxPTL(2, value); return;
        case 0x12C >> 1: // SPR3PTH
            _agnus->pokeSPRxPTH(3, value); return;
        case 0x12E >> 1: // SPR3PTL
            _agnus->pokeSPRxPTL(3, value); return;
        case 0x130 >> 1: // SPR4PTH
            _agnus->pokeSPRxPTH(4, value); return;
        case 0x132 >> 1: // SPR4PTL
            _agnus->pokeSPRxPTL(4, value); return;
        case 0x134 >> 1: // SPR5PTH
            _agnus->pokeSPRxPTH(5, value); return;
        case 0x136 >> 1: // SPR5PTL
            _agnus->pokeSPRxPTL(5, value); return;
        case 0x138 >> 1: // SPR6PTH
            _agnus->pokeSPRxPTH(6, value); return;
        case 0x13A >> 1: // SPR6PTL
            _agnus->pokeSPRxPTL(6, value); return;
        case 0x13C >> 1: // SPR7PTH
            _agnus->pokeSPRxPTH(7, value); return;
        case 0x13E >> 1: // SPR7PTL
            _agnus->pokeSPRxPTL(7, value); return;
        case 0x140 >> 1: // SPR0POS
            _denise->pokeSPRxPOS(0, value); return;
        case 0x142 >> 1: // SPR0CTL
            _denise->pokeSPRxCTL(0, value); return;
        case 0x144 >> 1: // SPR0DATA
            _denise->pokeSPRxDATA(0, value); return;
        case 0x146 >> 1: // SPR0DATB
            _denise->pokeSPRxDATB(0, value); return;
        case 0x148 >> 1: // SPR1POS
            _denise->pokeSPRxPOS(1, value); return;
        case 0x14A >> 1: // SPR1CTL
            _denise->pokeSPRxCTL(1, value); return;
        case 0x14C >> 1: // SPR1DATA
            _denise->pokeSPRxDATA(1, value); return;
        case 0x14E >> 1: // SPR1DATB
            _denise->pokeSPRxDATB(1, value); return;
        case 0x150 >> 1: // SPR2POS
            _denise->pokeSPRxPOS(2, value); return;
        case 0x152 >> 1: // SPR2CTL
            _denise->pokeSPRxCTL(2, value); return;
        case 0x154 >> 1: // SPR2DATA
            _denise->pokeSPRxDATA(2, value); return;
        case 0x156 >> 1: // SPR2DATB
            _denise->pokeSPRxDATB(2, value); return;
        case 0x158 >> 1: // SPR3POS
            _denise->pokeSPRxPOS(3, value); return;
        case 0x15A >> 1: // SPR3CTL
            _denise->pokeSPRxCTL(3, value); return;
        case 0x15C >> 1: // SPR3DATA
            _denise->pokeSPRxDATA(3, value); return;
        case 0x15E >> 1: // SPR3DATB
            _denise->pokeSPRxDATB(3, value); return;
        case 0x160 >> 1: // SPR4POS
            _denise->pokeSPRxPOS(4, value); return;
        case 0x162 >> 1: // SPR4CTL
            _denise->pokeSPRxCTL(4, value); return;
        case 0x164 >> 1: // SPR4DATA
            _denise->pokeSPRxDATA(4, value); return;
        case 0x166 >> 1: // SPR4DATB
            _denise->pokeSPRxDATB(4, value); return;
        case 0x168 >> 1: // SPR5POS
            _denise->pokeSPRxPOS(5, value); return;
        case 0x16A >> 1: // SPR5CTL
            _denise->pokeSPRxCTL(5, value); return;
        case 0x16C >> 1: // SPR5DATA
            _denise->pokeSPRxDATA(5, value); return;
        case 0x16E >> 1: // SPR5DATB
            _denise->pokeSPRxDATB(5, value); return;
        case 0x170 >> 1: // SPR6POS
            _denise->pokeSPRxPOS(6, value); return;
        case 0x172 >> 1: // SPR6CTL
            _denise->pokeSPRxCTL(6, value); return;
        case 0x174 >> 1: // SPR6DATA
            _denise->pokeSPRxDATA(6, value); return;
        case 0x176 >> 1: // SPR6DATB
            _denise->pokeSPRxDATB(6, value); return;
        case 0x178 >> 1: // SPR7POS
            _denise->pokeSPRxPOS(7, value); return;
        case 0x17A >> 1: // SPR7CTL
            _denise->pokeSPRxCTL(7, value); return;
        case 0x17C >> 1: // SPR7DATA
            _denise->pokeSPRxDATA(7, value); return;
        case 0x17E >> 1: // SPR7DATB
            _denise->pokeSPRxDATB(7, value); return;
        case 0x180 >> 1: // COLOR00
            _denise->colorizer.pokeColorReg(0, value); return;
        case 0x182 >> 1: // COLOR01
            _denise->colorizer.pokeColorReg(1, value); return;
        case 0x184 >> 1: // COLOR02
            _denise->colorizer.pokeColorReg(2, value); return;
        case 0x186 >> 1: // COLOR03
            _denise->colorizer.pokeColorReg(3, value); return;
        case 0x188 >> 1: // COLOR04
            _denise->colorizer.pokeColorReg(4, value); return;
        case 0x18A >> 1: // COLOR05
            _denise->colorizer.pokeColorReg(5, value); return;
        case 0x18C >> 1: // COLOR06
            _denise->colorizer.pokeColorReg(6, value); return;
        case 0x18E >> 1: // COLOR07
            _denise->colorizer.pokeColorReg(7, value); return;
        case 0x190 >> 1: // COLOR08
            _denise->colorizer.pokeColorReg(8, value); return;
        case 0x192 >> 1: // COLOR09
            _denise->colorizer.pokeColorReg(9, value); return;
        case 0x194 >> 1: // COLOR10
            _denise->colorizer.pokeColorReg(10, value); return;
        case 0x196 >> 1: // COLOR11
            _denise->colorizer.pokeColorReg(11, value); return;
        case 0x198 >> 1: // COLOR12
            _denise->colorizer.pokeColorReg(12, value); return;
        case 0x19A >> 1: // COLOR13
            _denise->colorizer.pokeColorReg(13, value); return;
        case 0x19C >> 1: // COLOR14
            _denise->colorizer.pokeColorReg(14, value); return;
        case 0x19E >> 1: // COLOR15
            _denise->colorizer.pokeColorReg(15, value); return;
        case 0x1A0 >> 1: // COLOR16
            _denise->colorizer.pokeColorReg(16, value); return;
        case 0x1A2 >> 1: // COLOR17
            _denise->colorizer.pokeColorReg(17, value); return;
        case 0x1A4 >> 1: // COLOR18
            _denise->colorizer.pokeColorReg(18, value); return;
        case 0x1A6 >> 1: // COLOR19
            _denise->colorizer.pokeColorReg(19, value); return;
        case 0x1A8 >> 1: // COLOR20
            _denise->colorizer.pokeColorReg(20, value); return;
        case 0x1AA >> 1: // COLOR21
            _denise->colorizer.pokeColorReg(21, value); return;
        case 0x1AC >> 1: // COLOR22
            _denise->colorizer.pokeColorReg(22, value); return;
        case 0x1AE >> 1: // COLOR23
            _denise->colorizer.pokeColorReg(23, value); return;
        case 0x1B0 >> 1: // COLOR24
            _denise->colorizer.pokeColorReg(24, value); return;
        case 0x1B2 >> 1: // COLOR25
            _denise->colorizer.pokeColorReg(25, value); return;
        case 0x1B4 >> 1: // COLOR26
            _denise->colorizer.pokeColorReg(26, value); return;
        case 0x1B6 >> 1: // COLOR27
            _denise->colorizer.pokeColorReg(27, value); return;
        case 0x1B8 >> 1: // COLOR28
            _denise->colorizer.pokeColorReg(28, value); return;
        case 0x1BA >> 1: // COLOR29
            _denise->colorizer.pokeColorReg(29, value); return;
        case 0x1BC >> 1: // COLOR30
            _denise->colorizer.pokeColorReg(30, value); return;
        case 0x1BE >> 1: // COLOR31
            _denise->colorizer.pokeColorReg(31, value); return;
    }
    
    if (addr <= 0x1E) {
        warn("pokeCustom16(%s,%X): Trying to write into a read-only register.\n",
             customReg[(addr >> 1) & 0xFF], value);
    }
}

void
Memory::pokeCustom32(uint32_t addr, uint32_t value)
{
    assert(false);
    pokeCustom16(addr,     HI_WORD(value));
    pokeCustom16(addr + 2, LO_WORD(value));
}

uint8_t
Memory::peekAutoConf8(uint32_t addr)
{
    uint8_t result = _zorro->peekFastRamDevice(addr) << 4;
    
    debug("peekAutoConf8(%X) = %X\n", addr, result);
    return result;
}

uint16_t
Memory::peekAutoConf16(uint32_t addr)
{
    uint16_t result = HI_LO(peekAutoConf8(addr), peekAutoConf8(addr + 1));
    
    debug("peekAutoConf16(%X) = %d\n", addr, result);
    return result;
}

void
Memory::pokeAutoConf8(uint32_t addr, uint8_t value)
{
    debug("pokeAutoConf8(%X, %X)\n", addr, value);
    _zorro->pokeFastRamDevice(addr, value);
}

void
Memory::pokeAutoConf16(uint32_t addr, uint16_t value)
{
    debug("pokeAutoConf16(%X, %X)\n", addr, value);
    _zorro->pokeFastRamDevice(addr, HI_BYTE(value));
    _zorro->pokeFastRamDevice(addr + 1, LO_BYTE(value));
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

void
Memory::hex(char *buffer, uint32_t addr, size_t bytes, size_t bufferSize)
{
    assert(buffer != NULL);
    assert(is_uint24_t(addr));
    assert(bytes % 2 == 0);
    assert(bufferSize != 0);
    
    unsigned maxWords = (bufferSize - 1) / 5;
    unsigned words = MIN(maxWords, bytes / 2);

    for (unsigned i = 0; i < words; i++) {
        
        uint16_t value = peek16(addr + 2*i);
        sprint16x(buffer, value);
        buffer += 4;
        *buffer++ = ' ';
    }
    *buffer = 0;
}

const char *
Memory::hex(uint32_t addr, size_t bytes)
{
    hex(str, addr, bytes, sizeof(str));
    return str;
}
