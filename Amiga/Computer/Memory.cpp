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
    if (extRom) { delete[] extRom; extRom = NULL; }
    if (chipRam) { delete[] chipRam; chipRam = NULL; }
    if (slowRam) { delete[] slowRam; slowRam = NULL; }
    if (fastRam) { delete[] fastRam; fastRam = NULL; }
}

void
Memory::_initialize()
{
    cpu = &amiga->cpu;
    ciaA = &amiga->ciaA;
    ciaB = &amiga->ciaB;
    agnus = &amiga->agnus;
    copper = &amiga->agnus.copper;
    denise = &amiga->denise;
    paula = &amiga->paula;
    zorro = &amiga->zorro;
}

void
Memory::_powerOn()
{
    // Make Rom writable if an A1000 is emulated
    kickIsWritable = amiga->getConfig().model == AMIGA_1000;

    // Remove Extended Rom if an A1000 is emulated.
    if (hasExtRom() && amiga->getConfig().model == AMIGA_1000) {
        deleteExtRom();
    }

    // Wipe out RAM
    if (chipRam) memset(chipRam, 0, chipRamSize);
    if (slowRam) memset(slowRam, 0, slowRamSize);
    if (fastRam) memset(fastRam, 0, fastRamSize);

    // Set up the memory lookup table
    updateMemSrcTable();
}

void
Memory::_reset()
{
    RESET_SNAPSHOT_ITEMS

    // Set up the memory lookup table
    updateMemSrcTable();
}

void
Memory::_dump()
{
    plainmsg("     Boot Rom: %d KB at %p\n", bootRomSize >> 10, bootRom);
    plainmsg("     Kick Rom: %d KB at %p (%s)\n", kickRomSize >> 10,
             kickRom, kickIsWritable ? "unlocked" : "locked");
    plainmsg("     Ext  Rom: %d KB at %p\n", extRomSize >> 10, extRom);
    plainmsg("     Chip Ram: %d KB at %p\n", chipRamSize >> 10, chipRam);
    plainmsg("     Slow Ram: %d KB at %p\n", slowRamSize >> 10, slowRam);
    plainmsg("     Fast Ram: %d KB at %p\n", fastRamSize >> 10, fastRam);
}

size_t
Memory::_size()
{
    SerCounter counter;

    applyToPersistentItems(counter);
    applyToResetItems(counter);

    counter.count += sizeof(bootRomSize) + bootRomSize;
    counter.count += sizeof(kickRomSize) + kickRomSize;
    counter.count += sizeof(extRomSize) + extRomSize;
    counter.count += sizeof(chipRamSize) + chipRamSize;
    counter.count += sizeof(slowRamSize) + slowRamSize;
    counter.count += sizeof(fastRamSize) + fastRamSize;

    debug(SNAP_DEBUG, "Snapshot size is %d bytes\n", counter.count);
    return counter.count;
}

size_t
Memory::didLoadFromBuffer(uint8_t *buffer)
{
    SerReader reader(buffer);

    // Load memory size information
    reader
    & bootRomSize
    & kickRomSize
    & extRomSize
    & chipRamSize
    & slowRamSize
    & fastRamSize;

    // Do some consistency checks
    // Note: We should do this a little less agressive, e.g., by returning
    // false. Furthermore, the real maximum size limits should be used.
    assert(bootRomSize < 0xFFFFFF);
    assert(kickRomSize < 0xFFFFFF);
    assert(extRomSize < 0xFFFFFF);
    assert(chipRamSize < 0xFFFFFF);
    assert(slowRamSize < 0xFFFFFF);
    assert(fastRamSize < 0xFFFFFF);
    
    // Free previously allocated memory
    dealloc();

    // Allocate new memory
    if (bootRomSize) bootRom = new (std::nothrow) uint8_t[bootRomSize + 3];
    if (kickRomSize) kickRom = new (std::nothrow) uint8_t[kickRomSize + 3];
    if (extRomSize) extRom = new (std::nothrow) uint8_t[extRomSize + 3];
    if (chipRamSize) chipRam = new (std::nothrow) uint8_t[chipRamSize + 3];
    if (slowRamSize) slowRam = new (std::nothrow) uint8_t[slowRamSize + 3];
    if (fastRamSize) fastRam = new (std::nothrow) uint8_t[fastRamSize + 3];

    // Load memory contents from buffer
    reader.copy(bootRom, bootRomSize);
    reader.copy(kickRom, kickRomSize);
    reader.copy(extRom, extRomSize);
    reader.copy(chipRam, chipRamSize);
    reader.copy(slowRam, slowRamSize);
    reader.copy(fastRam, fastRamSize);

    return reader.ptr - buffer;
}

size_t
Memory::didSaveToBuffer(uint8_t *buffer) const
{
    // Save memory size information
    SerWriter writer(buffer);
    writer
    & bootRomSize
    & kickRomSize
    & extRomSize
    & chipRamSize
    & slowRamSize
    & fastRamSize;

    // Save memory contents
    writer.copy(bootRom, bootRomSize);
    writer.copy(kickRom, kickRomSize);
    writer.copy(extRom, extRomSize);
    writer.copy(chipRam, chipRamSize);
    writer.copy(slowRam, slowRamSize);
    writer.copy(fastRam, fastRamSize);

    /*
    debug("bootRomSize = %d\n", bootRomSize);
    debug("kickRomSize = %d\n", kickRomSize);
    debug("extRomSize = %d\n", extRomSize);
    debug("chipRamSize = %d\n", chipRamSize);
    debug("slowRamSize = %d\n", slowRamSize);
    debug("fastRamSize = %d\n", fastRamSize);
    */

    return writer.ptr - buffer;
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

bool
Memory::loadExtRom(ExtRom *rom)
{
    assert(rom != NULL);

    if (!alloc(rom->getSize(), extRom, extRomSize))
        return false;

    loadRom(rom, extRom, extRomSize);
    return true;
}

bool
Memory::loadExtRomFromBuffer(const uint8_t *buffer, size_t length)
{
    assert(buffer != NULL);

    debug("loadExtRomFromBuffer\n");

    ExtRom *rom = ExtRom::makeWithBuffer(buffer, length);

    if (!rom) {
        msg("Failed to read Extended Rom from buffer at %p\n", buffer);
        return false;
    }

    return loadExtRom(rom);
}

bool
Memory::loadExtRomFromFile(const char *path)
{
    assert(path != NULL);

    debug("loadExtRomFromFile\n");

    ExtRom *rom = ExtRom::makeWithFile(path);

    if (!rom) {
        msg("Failed to read Extended Rom from file %s\n", path);
        return false;
    }

    return loadExtRom(rom);
}

void
Memory::updateMemSrcTable()
{
    AmigaConfiguration config = amiga->getConfig();
    MemorySource mem_boot = bootRom ? MEM_BOOT : MEM_UNMAPPED;
    MemorySource mem_kick = kickRom ? MEM_KICK : MEM_UNMAPPED;
    MemorySource mem_ext = extRom ? MEM_EXTROM : MEM_UNMAPPED;

    assert(chipRamSize % 0x10000 == 0);
    assert(slowRamSize % 0x10000 == 0);
    assert(fastRamSize % 0x10000 == 0);

    bool rtc = amiga ? config.realTimeClock : false;
    bool ovl = amiga ? (ciaA->getPA() & 1) : false;
    
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
    
    // Extended Rom
    for (unsigned i = 0xF0; i <= 0xF7; i++)
        memSrc[i] = mem_ext;

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

uint8_t
Memory::peek8(uint32_t addr)
{
    // debug("PC: %X peek8(%X)\n", amiga->cpu.getPC(), addr);

    addr &= 0xFFFFFF;
    switch (memSrc[addr >> 16]) {
            
        case MEM_UNMAPPED:

            agnus->executeUntilBusIsFree();
            return 0;

        case MEM_CHIP:

            agnus->executeUntilBusIsFree();
            ASSERT_CHIP_ADDR(addr);
            return READ_CHIP_8(addr);

        case MEM_FAST:

            ASSERT_FAST_ADDR(addr);
            return READ_FAST_8(addr);

        case MEM_CIA:

            agnus->executeUntilBusIsFree();
            ASSERT_CIA_ADDR(addr);
            return peekCIA8(addr);

        case MEM_SLOW:

            agnus->executeUntilBusIsFree();
            ASSERT_SLOW_ADDR(addr);
            return READ_SLOW_8(addr);

        case MEM_RTC:

            agnus->executeUntilBusIsFree();
            ASSERT_RTC_ADDR(addr);
            return peekRTC8(addr);

        case MEM_OCS:

            agnus->executeUntilBusIsFree();
            ASSERT_OCS_ADDR(addr);
            return peekCustom8(addr);

        case MEM_AUTOCONF:

            agnus->executeUntilBusIsFree();
            ASSERT_AUTO_ADDR(addr);
            return peekAutoConf8(addr);

        case MEM_BOOT:

            ASSERT_BOOT_ADDR(addr);
            return READ_BOOT_8(addr);

        case MEM_KICK:

            ASSERT_KICK_ADDR(addr);
            return READ_KICK_8(addr);

        case MEM_EXTROM:

            ASSERT_EXT_ADDR(addr);
            return READ_EXT_8(addr);

        default:
            assert(false);
    }
    return 0;
}

template <BusOwner owner> uint16_t
Memory::peek16(uint32_t addr)
{
    if (!IS_EVEN(addr)) {
        warn("peek16(%X): Address violation error (reading odd address)\n", addr);
    }

    addr &= 0xFFFFFF;

    switch(owner) {

        case BUS_COPPER:

            ASSERT_CHIP_ADDR(addr);
            return (memSrc[addr >> 16] == MEM_UNMAPPED) ? 0 : READ_CHIP_16(addr);

        case BUS_BLITTER:

            ASSERT_CHIP_ADDR(addr);
            return (memSrc[addr >> 16] == MEM_UNMAPPED) ? 0 : READ_CHIP_16(addr);

        case BUS_CPU:

            switch (memSrc[addr >> 16]) {

                case MEM_UNMAPPED:

                    agnus->executeUntilBusIsFree();
                    return 0;

                case MEM_CHIP:

                    ASSERT_CHIP_ADDR(addr);
                    agnus->executeUntilBusIsFree();
                    return READ_CHIP_16(addr);

                case MEM_FAST:

                    ASSERT_FAST_ADDR(addr);
                    return READ_FAST_16(addr);

                case MEM_CIA:

                    ASSERT_CIA_ADDR(addr);
                    agnus->executeUntilBusIsFree();
                    return peekCIA16(addr);

                case MEM_SLOW:

                    ASSERT_SLOW_ADDR(addr);
                    agnus->executeUntilBusIsFree();
                    return READ_SLOW_16(addr);

                case MEM_RTC:

                    ASSERT_RTC_ADDR(addr);
                    agnus->executeUntilBusIsFree();
                    return peekRTC16(addr);

                case MEM_OCS:

                    ASSERT_OCS_ADDR(addr);
                    agnus->executeUntilBusIsFree();
                    return peekCustom16(addr);

                case MEM_AUTOCONF:

                    ASSERT_AUTO_ADDR(addr);
                    agnus->executeUntilBusIsFree();
                    return peekAutoConf16(addr);

                case MEM_BOOT:

                    ASSERT_BOOT_ADDR(addr);
                    return READ_BOOT_16(addr);

                case MEM_KICK:

                    ASSERT_KICK_ADDR(addr);
                    return READ_KICK_16(addr);

                case MEM_EXTROM:

                    ASSERT_EXT_ADDR(addr);
                    return READ_EXT_16(addr);
            }
    }

    assert(false);
    return 0;
}

uint32_t
Memory::peek32(uint32_t addr)
{
    // debug("PC: %X peek32(%X)\n", amiga->cpu.getPC(), addr);
    return HI_W_LO_W(peek16<BUS_CPU>(addr), peek16<BUS_CPU>(addr + 2));
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
        case MEM_EXTROM:   ASSERT_EXT_ADDR(addr);  return READ_EXT_8(addr);
        default:           assert(false);
    }
    return 0;
}

uint16_t
Memory::spypeek16(uint32_t addr)
{
    if (!IS_EVEN(addr)) {
        warn("spypeek16(%X): Address violation error (reading odd address)\n", addr);
    }

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
        case MEM_EXTROM:   ASSERT_EXT_ADDR(addr);  return READ_EXT_16(addr);
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
    // if (addr >= 0xC2F3A0 && addr <= 0xC2F3B0) debug("**** poke8(%X,%X)\n", addr, value);

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
        case MEM_BOOT:     ASSERT_BOOT_ADDR(addr); pokeBoot8(addr, value); break;
        case MEM_KICK:     ASSERT_KICK_ADDR(addr); pokeKick16(addr, value); break;
        case MEM_EXTROM:   ASSERT_EXT_ADDR(addr); break;
        default:           assert(false);
    }
}

template <BusOwner owner> void
Memory::poke16(uint32_t addr, uint16_t value)
{
    if (!IS_EVEN(addr)) {
        warn("peek16(%X): Address violation error (reading odd address)\n", addr);
    }

    addr &= 0xFFFFFF;

    switch(owner) {

        case BUS_COPPER:

            ASSERT_CHIP_ADDR(addr);
            if (memSrc[addr >> 16] != MEM_UNMAPPED) WRITE_CHIP_16(addr, value);
            return;

        case BUS_BLITTER:

            ASSERT_CHIP_ADDR(addr);
            if (memSrc[addr >> 16] != MEM_UNMAPPED) WRITE_CHIP_16(addr, value);
            return;

        case BUS_CPU:

            switch (memSrc[addr >> 16]) {

                case MEM_UNMAPPED:

                    agnus->executeUntilBusIsFree();
                    return;

                case MEM_CHIP:

                    ASSERT_CHIP_ADDR(addr);
                    agnus->executeUntilBusIsFree();
                    WRITE_CHIP_16(addr, value);
                    return;

                case MEM_FAST:

                    ASSERT_FAST_ADDR(addr);
                    WRITE_FAST_16(addr, value);
                    return;

                case MEM_CIA:

                    ASSERT_CIA_ADDR(addr);
                    agnus->executeUntilBusIsFree();
                    pokeCIA16(addr, value);
                    return;

                case MEM_SLOW:

                    ASSERT_SLOW_ADDR(addr);
                    agnus->executeUntilBusIsFree();
                    WRITE_SLOW_16(addr, value);
                    return;

                case MEM_RTC:

                    ASSERT_RTC_ADDR(addr);
                    agnus->executeUntilBusIsFree();
                    pokeRTC16(addr, value);
                    return;

                case MEM_OCS:

                    ASSERT_OCS_ADDR(addr);
                    agnus->executeUntilBusIsFree();
                    pokeCustom16<POKE_CPU>(addr, value);
                    return;

                case MEM_AUTOCONF:

                    ASSERT_AUTO_ADDR(addr);
                    agnus->executeUntilBusIsFree();
                    pokeAutoConf16(addr, value);
                    return;

                case MEM_BOOT:

                    ASSERT_BOOT_ADDR(addr);
                    pokeBoot16(addr, value);
                    return;

                case MEM_KICK:

                    ASSERT_KICK_ADDR(addr);
                    pokeKick16(addr, value);
                    return;

                case MEM_EXTROM:

                    ASSERT_EXT_ADDR(addr);
                    return;

                default:
                    assert(false);
            }
            break;



    }
}

void
Memory::poke32(uint32_t addr, uint32_t value)
{
    poke16<BUS_CPU>(addr,     HI_WORD(value));
    poke16<BUS_CPU>(addr + 2, LO_WORD(value));
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
            return a0 ? ciaA->peek(reg) : ciaB->peek(reg);
            
        case 0b01:
            return a0 ? LO_BYTE(cpu->getIR()) : ciaB->peek(reg);
            
        case 0b10:
            return a0 ? ciaA->peek(reg) : HI_BYTE(cpu->getIR());
            
        case 0b11:
            return a0 ? LO_BYTE(cpu->getIR()) : HI_BYTE(cpu->getIR());
    }
    assert(false);
    return 0;
}

uint16_t
Memory::peekCIA16(uint32_t addr)
{
    debug(CIA_DEBUG, "peekCIA16(%6X)\n", addr);
    // assert(false);
    
    uint32_t reg = (addr >> 8)  & 0b1111;
    uint32_t sel = (addr >> 12) & 0b11;
    
    switch (sel) {
            
        case 0b00:
            return HI_LO(ciaB->peek(reg), ciaA->peek(reg));
            
        case 0b01:
            return HI_LO(ciaB->peek(reg), 0xFF);
            
        case 0b10:
            return HI_LO(0xFF, ciaA->peek(reg));
            
        case 0b11:
            return cpu->getIR();
            
    }
    assert(false);
    return 0;
}

uint32_t
Memory::peekCIA32(uint32_t addr)
{
    debug(CIA_DEBUG, "peekCIA32(%6X)\n", addr);
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
            return a0 ? ciaA->spypeek(reg) : ciaB->spypeek(reg);
            
        case 0b01:
            return a0 ? LO_BYTE(cpu->getIR()) : ciaB->spypeek(reg);
            
        case 0b10:
            return a0 ? ciaA->spypeek(reg) : HI_BYTE(cpu->getIR());
            
        case 0b11:
            return a0 ? LO_BYTE(cpu->getIR()) : HI_BYTE(cpu->getIR());
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
            return HI_LO(ciaB->spypeek(reg), ciaA->spypeek(reg));
            
        case 0b01:
            return HI_LO(ciaB->spypeek(reg), 0xFF);
            
        case 0b10:
            return HI_LO(0xFF, ciaA->spypeek(reg));
            
        case 0b11:
            return cpu->getIR();
            
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
    // debug(CIA_DEBUG, "pokeCIA8(%6X, %X)\n", addr, value);
    
    uint32_t reg = (addr >> 8) & 0b1111;
    uint32_t selA = (addr & 0x1000) == 0;
    uint32_t selB = (addr & 0x2000) == 0;

    if (selA) ciaA->poke(reg, value);
    if (selB) ciaB->poke(reg, value);
}

void
Memory::pokeCIA16(uint32_t addr, uint16_t value)
{
    debug("pokeCIA16(%6X, %X)\n", addr, value);
    // assert(false);
    
    assert(IS_EVEN(addr));
    
    uint32_t reg = (addr >> 8) & 0b1111;
    uint32_t selA = (addr & 0x1000) == 0;
    uint32_t selB = (addr & 0x2000) == 0;
    
    if (selA) ciaA->poke(reg, LO_BYTE(value));
    if (selB) ciaB->poke(reg, HI_BYTE(value));
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
    return amiga->rtc.peek((addr >> 2) & 0b1111);
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
    amiga->rtc.poke((addr >> 2) & 0b1111, value);
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

    /*
    if (addr != 0xDFF018)
        debug("peekCustom16(%X [%s])\n", addr, customReg[(addr >> 1) & 0xFF]);
    */

    switch ((addr >> 1) & 0xFF) {
            
        case 0x000 >> 1: // BLTDDAT
            return 0x00;
        case 0x002 >> 1: // DMACONR
            return agnus->peekDMACONR();
        case 0x004 >> 1: // VPOSR
            return agnus->peekVPOSR();
        case 0x006 >> 1: // VHPOSR
            return agnus->peekVHPOSR();
        case 0x008 >> 1: // DSKDATR
            return paula->diskController.peekDSKDATR();
        case 0x00A >> 1: // JOY0DAT
            return denise->peekJOY0DATR();
        case 0x00C >> 1: // JOY1DAT
            return denise->peekJOY1DATR();
        case 0x00E >> 1: // CLXDAT
            return denise->peekCLXDAT();
        case 0x010 >> 1: // ADKCONR
            return paula->peekADKCONR();
        case 0x012 >> 1: // POT0DAT
            return paula->peekPOTxDAT(0);
        case 0x014 >> 1: // POT1DAT
            return paula->peekPOTxDAT(1);
        case 0x016 >> 1: // POTGOR
            return paula->peekPOTGOR();
        case 0x018 >> 1: // SERDATR
            return paula->uart.peekSERDATR();
        case 0x01A >> 1: // DSKBYTR
            return paula->diskController.peekDSKBYTR();
        case 0x01C >> 1: // INTENAR
            return paula->peekINTENAR();
        case 0x01E >> 1: // INTREQR
            return paula->peekINTREQR();
    }

    /* Reading a write-only register
     *
     * Derived from the UAE source code documentation:
     *
     * Reading a write-only OCS register causes the last value of the
     * data bus to be written into this registers.
     *
     * Return values:
     *
     * - BLTDDAT (0x000) always returns the last data bus value.
     * - All other registers return
     *   - DMA cycle data (if DMA happened on the bus).
     *   - 0xFFFF or some some ANDed old data otherwise.
     */

    debug(REGACC_DEBUG, "peekCustom16(%X [%s]): WRITE-ONLY-REGISTER\n",
          addr, customReg[(addr >> 1) & 0xFF]);

    // TODO: Remember the last data bus value
    // In the meantime, we write 0, because SAE is doing this.
    pokeCustom16<POKE_CPU>(addr, 0);

    if (agnus->busOwner[agnus->pos.h] != BUS_NONE) {
        return agnus->busValue[agnus->pos.h];
    } else {
        return 0xFFFF;
    }
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
    /* "Custom register byte write bug = normally byte write to custom register
     *  writes same value to upper and lower byte."
     *     [http://eab.abime.net/showthread.php?p=1156399]
     */
    pokeCustom16<POKE_CPU>(addr & 0x1FE, HI_LO(value, value));
}

template <PokeSource s> void
Memory::pokeCustom16(uint32_t addr, uint16_t value)
{
    // if (addr >= 0x180 && addr <= 0x1BE) debug("Poke Color reg %X\n", addr);

    // debug("pokeCustom16(%X [%s], %X)\n", addr, customReg[(addr >> 1) & 0xFF], value);

    assert(IS_EVEN(addr));

    switch ((addr >> 1) & 0xFF) {

        case 0x020 >> 1: // DSKPTH
            agnus->pokeDSKPTH(value); return;
        case 0x022 >> 1: // DSKPTL
            agnus->pokeDSKPTL(value); return;
        case 0x024 >> 1: // DSKLEN
            paula->diskController.pokeDSKLEN(value); return;
        case 0x026 >> 1: // DSKDAT
            paula->diskController.pokeDSKDAT(value); return;
        case 0x028 >> 1: // REFPTR
            return;
        case 0x02A >> 1: // VPOSW
            agnus->pokeVPOS(value); return;
        case 0x02C >> 1: // VHPOSW
            agnus->pokeVHPOS(value); return;
        case 0x02E >> 1: // COPCON
            copper->pokeCOPCON(value); return;
        case 0x030 >> 1: // SERDAT
            paula->uart.pokeSERDAT(value); return;
        case 0x032 >> 1: // SERPER
            paula->uart.pokeSERPER(value); return;
        case 0x034 >> 1: // POTGO
            paula->pokePOTGO(value); return;
        case 0x036 >> 1: // JOYTEST
            denise->pokeJOYTEST(value); return;
        case 0x038 >> 1: // STREQU
        case 0x03A >> 1: // STRVBL
        case 0x03C >> 1: // STRHOR
        case 0x03E >> 1: // STRLONG
            return; // ignore
        case 0x040 >> 1: // BLTCON0
            agnus->blitter.pokeBLTCON0(value); return;
        case 0x042 >> 1: // BLTCON1
            agnus->blitter.pokeBLTCON1(value); return;
        case 0x044 >> 1: // BLTAFWM
            agnus->blitter.pokeBLTAFWM(value); return;
        case 0x046 >> 1: // BLTALWM
            agnus->blitter.pokeBLTALWM(value); return;
        case 0x048 >> 1: // BLTCPTH
            agnus->blitter.pokeBLTCPTH(value); return;
        case 0x04A >> 1: // BLTCPTL
            agnus->blitter.pokeBLTCPTL(value); return;
        case 0x04C >> 1: // BLTBPTH
            agnus->blitter.pokeBLTBPTH(value); return;
        case 0x04E >> 1: // BLTBPTL
            agnus->blitter.pokeBLTBPTL(value); return;
        case 0x050 >> 1: // BLTAPTH
            agnus->blitter.pokeBLTAPTH(value); return;
        case 0x052 >> 1: // BLTAPTL
            agnus->blitter.pokeBLTAPTL(value); return;
        case 0x054 >> 1: // BLTDPTH
            agnus->blitter.pokeBLTDPTH(value); return;
        case 0x056 >> 1: // BLTDPTL
            agnus->blitter.pokeBLTDPTL(value); return;
        case 0x058 >> 1: // BLTSIZE
            agnus->blitter.pokeBLTSIZE(value); return;
        case 0x05A >> 1: // unused
        case 0x05C >> 1: // unused
        case 0x05E >> 1: // unused
            break;
        case 0x060 >> 1: // BLTCMOD
            agnus->blitter.pokeBLTCMOD(value); return;
        case 0x062 >> 1: // BLTBMOD
            agnus->blitter.pokeBLTBMOD(value); return;
        case 0x064 >> 1: // BLTAMOD
            agnus->blitter.pokeBLTAMOD(value); return;
        case 0x066 >> 1: // BLTDMOD
            agnus->blitter.pokeBLTDMOD(value); return;
        case 0x068 >> 1: // unused
        case 0x06A >> 1: // unused
        case 0x06C >> 1: // unused
        case 0x06E >> 1: // unused
            break;
        case 0x070 >> 1: // BLTCDAT
            agnus->blitter.pokeBLTCDAT(value); return;
        case 0x072 >> 1: // BLTBDAT
            agnus->blitter.pokeBLTBDAT(value); return;
        case 0x074 >> 1: // BLTADAT
            agnus->blitter.pokeBLTADAT(value); return;
        case 0x076 >> 1: // unused
        case 0x078 >> 1: // unused
        case 0x07A >> 1: // unused
        case 0x07C >> 1: // unused
            break;
        case 0x07E >> 1: // DSKSYNC
            paula->diskController.pokeDSKSYNC(value); return;
        case 0x080 >> 1: // COP1LCH
            copper->pokeCOP1LCH(value); return;
        case 0x082 >> 1: // COP1LCL
            copper->pokeCOP1LCL(value); return;
        case 0x084 >> 1: // COP2LCH
            copper->pokeCOP2LCH(value); return;
        case 0x086 >> 1: // COP2LCL
            copper->pokeCOP2LCL(value); return;
        case 0x088 >> 1: // COPJMP1
            copper->pokeCOPJMP1(); return;
        case 0x08A >> 1: // COPJMP2
            copper->pokeCOPJMP2(); return;
        case 0x08C >> 1: // COPINS
            copper->pokeCOPINS(value); return;
        case 0x08E >> 1: // DIWSTRT
            agnus->pokeDIWSTRT<s>(value); return;
        case 0x090 >> 1: // DIWSTOP
            agnus->pokeDIWSTOP<s>(value); return;
        case 0x092 >> 1: // DDFSTRT
            agnus->pokeDDFSTRT(value); return;
        case 0x094 >> 1: // DDFSTOP
            agnus->pokeDDFSTOP(value); return;
        case 0x096 >> 1: // DMACON
            agnus->scheduleRegEvent<s>(DMA_CYCLES(2), REG_DMACON, value);
            return;
        case 0x098 >> 1:  // CLXCON
            denise->pokeCLXCON(value); return;
        case 0x09A >> 1: // INTENA
            paula->pokeINTENA(value); return;
        case 0x09C >> 1: // INTREQ
            paula->pokeINTREQ(value); return;
        case 0x09E >> 1: // ADKCON
            paula->pokeADKCON(value); return;
        case 0x0A0 >> 1: // AUD0LCH
            paula->audioUnit.channel0.pokeAUDxLCH(value); return;
        case 0x0A2 >> 1: // AUD0LCL
            paula->audioUnit.channel0.pokeAUDxLCL(value); return;
        case 0x0A4 >> 1: // AUD0LEN
            paula->audioUnit.channel0.pokeAUDxLEN(value); return;
        case 0x0A6 >> 1: // AUD0PER
            paula->audioUnit.channel0.pokeAUDxPER(value); return;
        case 0x0A8 >> 1: // AUD0VOL
            paula->audioUnit.channel0.pokeAUDxVOL(value); return;
        case 0x0AA >> 1: // AUD0DAT
            paula->audioUnit.channel0.pokeAUDxDAT(value); return;
        case 0x0AC >> 1: // Unused
        case 0x0AE >> 1: // Unused
            break;
        case 0x0B0 >> 1: // AUD1LCH
            paula->audioUnit.channel1.pokeAUDxLCH(value); return;
        case 0x0B2 >> 1: // AUD1LCL
            paula->audioUnit.channel1.pokeAUDxLCL(value); return;
        case 0x0B4 >> 1: // AUD1LEN
            paula->audioUnit.channel1.pokeAUDxLEN(value); return;
        case 0x0B6 >> 1: // AUD1PER
            paula->audioUnit.channel1.pokeAUDxPER(value); return;
        case 0x0B8 >> 1: // AUD1VOL
            paula->audioUnit.channel1.pokeAUDxVOL(value); return;
        case 0x0BA >> 1: // AUD1DAT
            paula->audioUnit.channel1.pokeAUDxDAT(value); return;
        case 0x0BC >> 1: // Unused
        case 0x0BE >> 1: // Unused
            break;
        case 0x0C0 >> 1: // AUD2LCH
            paula->audioUnit.channel2.pokeAUDxLCH(value); return;
        case 0x0C2 >> 1: // AUD2LCL
            paula->audioUnit.channel2.pokeAUDxLCL(value); return;
        case 0x0C4 >> 1: // AUD2LEN
            paula->audioUnit.channel2.pokeAUDxLEN(value); return;
        case 0x0C6 >> 1: // AUD2PER
            paula->audioUnit.channel2.pokeAUDxPER(value); return;
        case 0x0C8 >> 1: // AUD2VOL
            paula->audioUnit.channel2.pokeAUDxVOL(value); return;
        case 0x0CA >> 1: // AUD2DAT
            paula->audioUnit.channel2.pokeAUDxDAT(value); return;
        case 0x0CC >> 1: // Unused
        case 0x0CE >> 1: // Unused
            break;
        case 0x0D0 >> 1: // AUD3LCH
            paula->audioUnit.channel3.pokeAUDxLCH(value); return;
        case 0x0D2 >> 1: // AUD3LCL
            paula->audioUnit.channel3.pokeAUDxLCL(value); return;
        case 0x0D4 >> 1: // AUD3LEN
            paula->audioUnit.channel3.pokeAUDxLEN(value); return;
        case 0x0D6 >> 1: // AUD3PER
            paula->audioUnit.channel3.pokeAUDxPER(value); return;
        case 0x0D8 >> 1: // AUD3VOL
            paula->audioUnit.channel3.pokeAUDxVOL(value); return;
        case 0x0DA >> 1: // AUD3DAT
            paula->audioUnit.channel3.pokeAUDxDAT(value); return;
        case 0x0DC >> 1: // Unused
        case 0x0DE >> 1: // Unused
            break;
        case 0x0E0 >> 1: // BPL1PTH
            agnus->pokeBPLxPTH<1,s>(value); return;
        case 0x0E2 >> 1: // BPL1PTL
            agnus->pokeBPLxPTL<1,s>(value); return;
        case 0x0E4 >> 1: // BPL2PTH
            agnus->pokeBPLxPTH<2,s>(value); return;
        case 0x0E6 >> 1: // BPL2PTL
            agnus->pokeBPLxPTL<2,s>(value); return;
        case 0x0E8 >> 1: // BPL3PTH
            agnus->pokeBPLxPTH<3,s>(value); return;
        case 0x0EA >> 1: // BPL3PTL
            agnus->pokeBPLxPTL<3,s>(value); return;
        case 0x0EC >> 1: // BPL4PTH
            agnus->pokeBPLxPTH<4,s>(value); return;
        case 0x0EE >> 1: // BPL4PTL
            agnus->pokeBPLxPTL<4,s>(value); return;
        case 0x0F0 >> 1: // BPL5PTH
            agnus->pokeBPLxPTH<5,s>(value); return;
        case 0x0F2 >> 1: // BPL5PTL
            agnus->pokeBPLxPTL<5,s>(value); return;
        case 0x0F4 >> 1: // BPL6PTH
            agnus->pokeBPLxPTH<6,s>(value); return;
        case 0x0F6 >> 1: // BPL6PTL
            agnus->pokeBPLxPTL<6,s>(value); return;
        case 0x0F8 >> 1: // Unused
        case 0x0FA >> 1: // Unused
        case 0x0FC >> 1: // Unused
        case 0x0FE >> 1: // Unused
            break;
        case 0x100 >> 1: // BPLCON0
            agnus->pokeBPLCON0(value);
            denise->pokeBPLCON0(value);
            // agnus->scheduleRegEvent<s>(DMA_CYCLES(2), REG_BPLCON0, value);
            return;
        case 0x102 >> 1: // BPLCON1
            agnus->scheduleRegEvent<s>(DMA_CYCLES(2), REG_BPLCON1, value);
            return;
        case 0x104 >> 1: // BPLCON2
            agnus->scheduleRegEvent<s>(DMA_CYCLES(2), REG_BPLCON2, value);
            return;
        case 0x106 >> 1: // Unused
            break;
        case 0x108 >> 1: // BPL1MOD
            agnus->scheduleRegEvent<s>(DMA_CYCLES(2), REG_BPL1MOD, value);
            return;
        case 0x10A >> 1: // BPL2MOD
            agnus->scheduleRegEvent<s>(DMA_CYCLES(2), REG_BPL2MOD, value);
            return;
        case 0x10C >> 1: // Unused
        case 0x10E >> 1: // Unused
            break;
        case 0x110 >> 1: // BPL1DAT
            denise->pokeBPLxDAT<0>(value); return;
        case 0x112 >> 1: // BPL2DAT
            denise->pokeBPLxDAT<1>(value); return;
        case 0x114 >> 1: // BPL3DAT
            denise->pokeBPLxDAT<2>(value); return;
        case 0x116 >> 1: // BPL4DAT
            denise->pokeBPLxDAT<3>(value); return;
        case 0x118 >> 1: // BPL5DAT
            denise->pokeBPLxDAT<4>(value); return;
        case 0x11A >> 1: // BPL6DAT
            denise->pokeBPLxDAT<5>(value); return;
        case 0x11C >> 1: // Unused
        case 0x11E >> 1: // Unused
            break;
        case 0x120 >> 1: // SPR0PTH
            agnus->pokeSPRxPTH<0>(value); return;
        case 0x122 >> 1: // SPR0PTL
            agnus->pokeSPRxPTL<0>(value); return;
        case 0x124 >> 1: // SPR1PTH
            agnus->pokeSPRxPTH<1>(value); return;
        case 0x126 >> 1: // SPR1PTL
            agnus->pokeSPRxPTL<1>(value); return;
        case 0x128 >> 1: // SPR2PTH
            agnus->pokeSPRxPTH<2>(value); return;
        case 0x12A >> 1: // SPR2PTL
            agnus->pokeSPRxPTL<2>(value); return;
        case 0x12C >> 1: // SPR3PTH
            agnus->pokeSPRxPTH<3>(value); return;
        case 0x12E >> 1: // SPR3PTL
            agnus->pokeSPRxPTL<3>(value); return;
        case 0x130 >> 1: // SPR4PTH
            agnus->pokeSPRxPTH<4>(value); return;
        case 0x132 >> 1: // SPR4PTL
            agnus->pokeSPRxPTL<4>(value); return;
        case 0x134 >> 1: // SPR5PTH
            agnus->pokeSPRxPTH<5>(value); return;
        case 0x136 >> 1: // SPR5PTL
            agnus->pokeSPRxPTL<5>(value); return;
        case 0x138 >> 1: // SPR6PTH
            agnus->pokeSPRxPTH<6>(value); return;
        case 0x13A >> 1: // SPR6PTL
            agnus->pokeSPRxPTL<6>(value); return;
        case 0x13C >> 1: // SPR7PTH
            agnus->pokeSPRxPTH<7>(value); return;
        case 0x13E >> 1: // SPR7PTL
            agnus->pokeSPRxPTL<7>(value); return;
        case 0x140 >> 1: // SPR0POS
            denise->pokeSPRxPOS<0>(value); return;
        case 0x142 >> 1: // SPR0CTL
            denise->pokeSPRxCTL<0>(value); return;
        case 0x144 >> 1: // SPR0DATA
            denise->pokeSPRxDATA<0>(value); return;
        case 0x146 >> 1: // SPR0DATB
            denise->pokeSPRxDATB<0>(value); return;
        case 0x148 >> 1: // SPR1POS
            denise->pokeSPRxPOS<1>(value); return;
        case 0x14A >> 1: // SPR1CTL
            denise->pokeSPRxCTL<1>(value); return;
        case 0x14C >> 1: // SPR1DATA
            denise->pokeSPRxDATA<1>(value); return;
        case 0x14E >> 1: // SPR1DATB
            denise->pokeSPRxDATB<1>(value); return;
        case 0x150 >> 1: // SPR2POS
            denise->pokeSPRxPOS<2>(value); return;
        case 0x152 >> 1: // SPR2CTL
            denise->pokeSPRxCTL<2>(value); return;
        case 0x154 >> 1: // SPR2DATA
            denise->pokeSPRxDATA<2>(value); return;
        case 0x156 >> 1: // SPR2DATB
            denise->pokeSPRxDATB<2>(value); return;
        case 0x158 >> 1: // SPR3POS
            denise->pokeSPRxPOS<3>(value); return;
        case 0x15A >> 1: // SPR3CTL
            denise->pokeSPRxCTL<3>(value); return;
        case 0x15C >> 1: // SPR3DATA
            denise->pokeSPRxDATA<3>(value); return;
        case 0x15E >> 1: // SPR3DATB
            denise->pokeSPRxDATB<3>(value); return;
        case 0x160 >> 1: // SPR4POS
            denise->pokeSPRxPOS<4>(value); return;
        case 0x162 >> 1: // SPR4CTL
            denise->pokeSPRxCTL<4>(value); return;
        case 0x164 >> 1: // SPR4DATA
            denise->pokeSPRxDATA<4>(value); return;
        case 0x166 >> 1: // SPR4DATB
            denise->pokeSPRxDATB<4>(value); return;
        case 0x168 >> 1: // SPR5POS
            denise->pokeSPRxPOS<5>(value); return;
        case 0x16A >> 1: // SPR5CTL
            denise->pokeSPRxCTL<5>(value); return;
        case 0x16C >> 1: // SPR5DATA
            denise->pokeSPRxDATA<5>(value); return;
        case 0x16E >> 1: // SPR5DATB
            denise->pokeSPRxDATB<5>(value); return;
        case 0x170 >> 1: // SPR6POS
            denise->pokeSPRxPOS<6>(value); return;
        case 0x172 >> 1: // SPR6CTL
            denise->pokeSPRxCTL<6>(value); return;
        case 0x174 >> 1: // SPR6DATA
            denise->pokeSPRxDATA<6>(value); return;
        case 0x176 >> 1: // SPR6DATB
            denise->pokeSPRxDATB<6>(value); return;
        case 0x178 >> 1: // SPR7POS
            denise->pokeSPRxPOS<7>(value); return;
        case 0x17A >> 1: // SPR7CTL
            denise->pokeSPRxCTL<7>(value); return;
        case 0x17C >> 1: // SPR7DATA
            denise->pokeSPRxDATA<7>(value); return;
        case 0x17E >> 1: // SPR7DATB
            denise->pokeSPRxDATB<7>(value); return;
        case 0x180 >> 1: // COLOR00
        case 0x182 >> 1: // COLOR01
        case 0x184 >> 1: // COLOR02
        case 0x186 >> 1: // COLOR03
        case 0x188 >> 1: // COLOR04
        case 0x18A >> 1: // COLOR05
        case 0x18C >> 1: // COLOR06
        case 0x18E >> 1: // COLOR07
        case 0x190 >> 1: // COLOR08
        case 0x192 >> 1: // COLOR09
        case 0x194 >> 1: // COLOR10
        case 0x196 >> 1: // COLOR11
        case 0x198 >> 1: // COLOR12
        case 0x19A >> 1: // COLOR13
        case 0x19C >> 1: // COLOR14
        case 0x19E >> 1: // COLOR15
        case 0x1A0 >> 1: // COLOR16
        case 0x1A2 >> 1: // COLOR17
        case 0x1A4 >> 1: // COLOR18
        case 0x1A6 >> 1: // COLOR19
        case 0x1A8 >> 1: // COLOR20
        case 0x1AA >> 1: // COLOR21
        case 0x1AC >> 1: // COLOR22
        case 0x1AE >> 1: // COLOR23
        case 0x1B0 >> 1: // COLOR24
        case 0x1B2 >> 1: // COLOR25
        case 0x1B4 >> 1: // COLOR26
        case 0x1B6 >> 1: // COLOR27
        case 0x1B8 >> 1: // COLOR28
        case 0x1BA >> 1: // COLOR29
        case 0x1BC >> 1: // COLOR30
        case 0x1BE >> 1: // COLOR31
            denise->pokeColorReg(addr & 0x1FF, value); return;
        case 0x1FE >> 1: // NO-OP (NULL)
            copper->pokeNOOP(value); return;
    }
    
    if (addr <= 0x1E) {
        debug(REGACC_DEBUG, "pokeCustom16(%X [%s]): READ-ONLY-REGISTER\n",
             addr, customReg[(addr >> 1) & 0xFF]);
    } else {
        debug(REGACC_DEBUG, "pokeCustom16(%X [%s]): NO OCS REGISTER\n",
              addr, customReg[(addr >> 1) & 0xFF]);
    }
}

void
Memory::pokeCustom32(uint32_t addr, uint32_t value)
{
    assert(false);
    pokeCustom16<POKE_CPU>(addr,     HI_WORD(value));
    pokeCustom16<POKE_CPU>(addr + 2, LO_WORD(value));
}

uint8_t
Memory::peekAutoConf8(uint32_t addr)
{
    uint8_t result = zorro->peekFastRamDevice(addr) << 4;
    
    // debug("peekAutoConf8(%X) = %X\n", addr, result);
    return result;
}

uint16_t
Memory::peekAutoConf16(uint32_t addr)
{
    uint16_t result = HI_LO(peekAutoConf8(addr), peekAutoConf8(addr + 1));
    
    // debug("peekAutoConf16(%X) = %d\n", addr, result);
    return result;
}

void
Memory::pokeAutoConf8(uint32_t addr, uint8_t value)
{
    // debug("pokeAutoConf8(%X, %X)\n", addr, value);
    zorro->pokeFastRamDevice(addr, value);
}

void
Memory::pokeAutoConf16(uint32_t addr, uint16_t value)
{
    // debug("pokeAutoConf16(%X, %X)\n", addr, value);
    zorro->pokeFastRamDevice(addr, HI_BYTE(value));
    zorro->pokeFastRamDevice(addr + 1, LO_BYTE(value));
}

void
Memory::pokeBoot8(uint32_t addr, uint8_t value)
{
    debug("pokeBoot8(%X, %X)\n", addr, value);
    reportSuspiciousBehavior();
}

void
Memory::pokeBoot16(uint32_t addr, uint16_t value)
{
    debug("pokeBoot16(%X, %X)\n", addr, value);

    // Turn the WOM into a ROM
    debug("Locking WOM\n");
    kickIsWritable = false;
}

void
Memory::pokeKick8(uint32_t addr, uint8_t value)
{
    debug("pokeKick8(%X, %X)\n", addr, value);

    // It's suspicious if a program is doing that, so we better investigate...
    assert(false);
}

void
Memory::pokeKick16(uint32_t addr, uint16_t value)
{
    // debug("pokeKick16(%X, %X)\n", addr, value);

    if (kickIsWritable) {

        // Make sure we're emulating an A1000
        assert(amiga->getConfig().model == AMIGA_1000);

        WRITE_KICK_16(addr, value);
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
        
        uint16_t value = spypeek16(addr + 2*i);
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

template void Memory::pokeCustom16<POKE_CPU>(uint32_t addr, uint16_t value);
template void Memory::pokeCustom16<POKE_COPPER>(uint32_t addr, uint16_t value);

template uint16_t Memory::peek16<BUS_CPU>(uint32_t addr);
template uint16_t Memory::peek16<BUS_COPPER>(uint32_t addr);
template uint16_t Memory::peek16<BUS_BLITTER>(uint32_t addr);

template void Memory::poke16<BUS_CPU>(uint32_t addr, uint16_t value);
template void Memory::poke16<BUS_COPPER>(uint32_t addr, uint16_t value);
template void Memory::poke16<BUS_BLITTER>(uint32_t addr, uint16_t value);
