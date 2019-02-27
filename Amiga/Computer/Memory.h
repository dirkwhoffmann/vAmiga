// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_MEMORY_INC
#define _AMIGA_MEMORY_INC

#include "HardwareComponent.h"
#include "BootRom.h"
#include "KickRom.h"


const uint32_t FAST_RAM_START = 0x200000;

const uint32_t SLOW_RAM_MASK = 0x007FFFF;
const uint32_t BOOT_ROM_MASK = 0x003FFFF;
const uint32_t KICK_ROM_MASK = 0x003FFFF;

#define ASSERT_CHIP_ADDR(x) assert(chipRam != NULL);
#define ASSERT_FAST_ADDR(x) assert(fastRam != NULL); assert(((x) - FAST_RAM_START) < fastRamSize);
#define ASSERT_SLOW_ADDR(x) assert(slowRam != NULL); assert(((x) & SLOW_RAM_MASK) < slowRamSize);
#define ASSERT_BOOT_ADDR(x) assert(bootRom != NULL); assert(((x) & BOOT_ROM_MASK) < bootRomSize);
#define ASSERT_KICK_ADDR(x) assert(kickRom != NULL); assert(((x) & KICK_ROM_MASK) < kickRomSize);
#define ASSERT_CIA_ADDR(x)  assert((x) >= 0xA00000 && (x) <= 0xBFFFFF);
#define ASSERT_RTC_ADDR(x)  assert((x) >= 0xDC0000 && (x) <= 0xDEFFFF);
#define ASSERT_OCS_ADDR(x)  assert((x) >= 0xC00000 && (x) <= 0xDFFFFF);

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


class Memory : public HardwareComponent {
    
    friend class Copper;
    
public:
    
    /* Each memory area is represented by three variables:
     *
     *   A pointer to the allocates memory.
     *   A variable storing the memory size in bytes.
     *
     * The following invariant holds:
     *
     *   pointer == NULL <=> size == 0
     */
    
    // Boot Rom (Amiga 1000 only)
    uint8_t *bootRom = NULL;
    size_t bootRomSize = 0;
    
    // Kickstart Rom and size
    uint8_t *kickRom = NULL;
    size_t kickRomSize = 0;
    
    // Chip Ram and size
    uint8_t *chipRam = NULL;
    size_t chipRamSize = 0;
    
    // Slow Ram and size
    uint8_t *slowRam = NULL;
    size_t slowRamSize = 0;
    
    // Fast Ram and size
    uint8_t *fastRam = NULL;
    size_t fastRamSize = 0;
    
    /* Indicates if the Kickstart Rom is writable
     * If an A500 or A2000 is emulated, this variable is always false. If an
     * A1000 is emulated, it is true on startup to emulate WOM (Write Once
     * Memory).
     */
    bool kickIsWritable = false;
    
    /* We divide the memory into banks of size 64KB.
     * The Amiga has 24 address lines. Hence, the accessible memory is divided
     * into 256 different banks. For each bank, this array indicates the
     * type of memory that is seen by the Amiga.
     */
    MemorySource memSrc[256];
    
    // Used by functions returning string values.
    char str[256];
    
public:
    
    //
    // Constructing and destructing
    //
    
public:
    
    Memory();
    ~Memory();
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
    
    
    //
    // Allocating memory
    //
    
    /*
    bool allocateBootRom();
    bool allocateKickRom();
    bool allocateChipRam(size_t size);
    bool allocateSlowRam(size_t size);
    bool allocateFastRam(size_t size);
    */
    
    // Deallocates all previously allocated memory
    // void dealloc();
    
private:
    
    /* Allocates 'size' bytes of memory.
     * As side effects, the memory lookup table is updated and a memory
     * layout message is sent to the GUI.
     */
    bool alloc(size_t size, uint8_t *&ptrref, size_t &sizeref);
    
 
    //
    // Managing Ram
    //
    
public:
    
    bool hasChipRam() { return chipRam != NULL; }
    bool allocateChipRam(size_t size) { return alloc(size, chipRam, chipRamSize); }

    bool hasSlowRam() { return slowRam != NULL; }
    bool allocateSlowRam(size_t size) { return alloc(size, slowRam, slowRamSize); }

    bool hasFastRam() { return fastRam != NULL; }
    bool allocateFastRam(size_t size) { return alloc(size, fastRam, fastRamSize); }
    
    
    //
    // Managing Roms
    //
    
private:
    
    // Loads Rom data
    void loadRom(AmigaFile *rom, uint8_t *target, size_t length);
    
public:

    // Returns true if a Boot Rom is installed
    bool hasBootRom() { return bootRom != NULL; }

    // Returns a fingerprint for the currently installed Boot Rom
    uint64_t bootRomFingerprint() { return fnv_1a(bootRom, bootRomSize); }
    
    // Installs the Boot Rom.
    bool loadBootRomFromBuffer(const uint8_t *buffer, size_t length);
    bool loadBootRomFromFile(const char *path);
    bool loadBootRom(BootRom *rom);

    // Deletes the currently installed Boot Rom.
    void deleteBootRom() { alloc(0, bootRom, bootRomSize); }
    
    // Returns true if a Kickstart Rom is installed
    bool hasKickRom() { return kickRom != NULL; }

    // Returns a fingerprint for the currently installed Kickstart Rom
    uint64_t kickRomFingerprint() { return fnv_1a(kickRom, kickRomSize); }
    
    // Installs the Kickstart Rom.
    bool loadKickRomFromBuffer(const uint8_t *buffer, size_t length);
    bool loadKickRomFromFile(const char *path);
    bool loadKickRom(KickRom *rom);
    
    // Deletes the currently installed Kickstart Rom.
    void deleteKickRom()  { alloc(0, kickRom, kickRomSize); }

    
    //
    // Accessing memory
    //
    
public:
    
    // Returns the memory source lookup table.
    MemorySource *getMemSrcTable() { return memSrc; }

    // Returns the memory source for a given address.
    MemorySource getMemSrc(uint32_t addr) { return memSrc[(addr >> 16) & 0xFF]; }

    // Updates the memory source lookup table.
    void updateMemSrcTable();
    
private:
    
    /* Computes the memory source for a given memory bank
     * This method is used inside updateMemSrcTable()
     */
    // MemorySource computeMemSrc(uint16_t bank);
    
    //
    // Accessing memory cells
    //
    
public:
    
    uint8_t peek8(uint32_t addr);
    uint16_t peek16(uint32_t addr);
    uint32_t peek32(uint32_t addr);
    
    uint8_t spypeek8(uint32_t addr);
    uint16_t spypeek16(uint32_t addr);
    uint32_t spypeek32(uint32_t addr);
    
    void poke8(uint32_t addr, uint8_t value);
    void poke16(uint32_t addr, uint16_t value);
    void poke32(uint32_t addr, uint32_t value);

    //
    // Chip Ram
    //
    
    inline uint8_t peekChip8(uint32_t addr) { ASSERT_CHIP_ADDR(addr); return READ_CHIP_8(addr); }
    inline uint16_t peekChip16(uint32_t addr) { ASSERT_CHIP_ADDR(addr); return READ_CHIP_16(addr); }
    inline uint32_t peekChip32(uint32_t addr) { ASSERT_CHIP_ADDR(addr); return READ_CHIP_32(addr); }
    
    inline uint8_t spypeekChip8(uint32_t addr) { return peekChip8(addr); }
    inline uint16_t spypeekChip16(uint32_t addr) { return peekChip16(addr); }
    inline uint32_t spypeekChip32(uint32_t addr) { return peekChip32(addr); }
    
    inline void pokeChip8(uint32_t addr, uint8_t value) { ASSERT_CHIP_ADDR(addr); WRITE_CHIP_8(addr, value); }
    inline void pokeChip16(uint32_t addr, uint16_t value) { ASSERT_CHIP_ADDR(addr); WRITE_CHIP_16(addr, value); }
    inline void pokeChip32(uint32_t addr, uint32_t value) { ASSERT_CHIP_ADDR(addr); WRITE_CHIP_32(addr, value); }
    
    
    //
    // CIAs
    //
    
private:
    
    uint8_t peekCIA8(uint32_t addr);
    uint16_t peekCIA16(uint32_t addr);
    uint32_t peekCIA32(uint32_t addr);
 
    uint8_t spypeekCIA8(uint32_t addr);
    uint16_t spypeekCIA16(uint32_t addr);
    uint32_t spypeekCIA32(uint32_t addr);

    void pokeCIA8(uint32_t addr, uint8_t value);
    void pokeCIA16(uint32_t addr, uint16_t value);
    void pokeCIA32(uint32_t addr, uint32_t value);
    
    
    //
    // Custom chip set
    //
    
    uint8_t peekCustom8(uint32_t addr);
    uint16_t peekCustom16(uint32_t addr);
    uint32_t peekCustom32(uint32_t addr);
    
    uint8_t spypeekCustom8(uint32_t addr);
    uint16_t spypeekCustom16(uint32_t addr);
    uint32_t spypeekCustom32(uint32_t addr);

    void pokeCustom8(uint32_t addr, uint8_t value);
    void pokeCustom16(uint32_t addr, uint16_t value);
    void pokeCustom32(uint32_t addr, uint32_t value);

    
    
public:
    
    // Returns 16 bytes of memory as an ASCII string.
    const char *ascii(uint32_t addr);
 
    // Returns a certain amount of bytes as a string containing hex words.
    const char *hex(uint32_t addr, size_t bytes);

};

#endif
