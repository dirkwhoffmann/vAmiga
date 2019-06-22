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
#include "ExtRom.h"

const uint32_t FAST_RAM_STRT = 0x0200000;
const uint32_t SLOW_RAM_MASK = 0x007FFFF;
// const uint32_t BOOT_ROM_MASK = 0x003FFFF;
const uint32_t BOOT_ROM_MASK = 0x0007FFF;
const uint32_t KICK_ROM_MASK = 0x007FFFF;
const uint32_t EXT_ROM_MASK  = 0x007FFFF;

// Verifies the range of an address
#define ASSERT_CHIP_ADDR(x) assert(chipRam != NULL);
#define ASSERT_FAST_ADDR(x) assert(fastRam != NULL); assert(((x) - FAST_RAM_STRT) < fastRamSize);
#define ASSERT_SLOW_ADDR(x) assert(slowRam != NULL); assert(((x) & SLOW_RAM_MASK) < slowRamSize);
#define ASSERT_BOOT_ADDR(x) assert(bootRom != NULL); assert(((x) & BOOT_ROM_MASK) < bootRomSize);
#define ASSERT_KICK_ADDR(x) assert(kickRom != NULL); 
#define ASSERT_EXT_ADDR(x)  assert(extRom  != NULL); assert(((x) & EXT_ROM_MASK) < extRomSize);
#define ASSERT_CIA_ADDR(x)  assert((x) >= 0xA00000 && (x) <= 0xBFFFFF);
#define ASSERT_RTC_ADDR(x)  assert((x) >= 0xDC0000 && (x) <= 0xDEFFFF);
#define ASSERT_OCS_ADDR(x)  assert((x) >= 0xC00000 && (x) <= 0xDFFFFF);
#define ASSERT_AUTO_ADDR(x) assert((x) >= 0xE80000 && (x) <= 0xEFFFFF);

// Reads a value from memory in big endian format
#define READ_8(x)  (*(uint8_t *)(x))
#define READ_16(x) (ntohs(*(uint16_t *)(x)))
#define READ_32(x) (ntohl(*(uint32_t *)(x)))

// Reads a value from Chip RAM in big endian format
#define READ_CHIP_8(x)  READ_8(chipRam + ((x) % chipRamSize))
#define READ_CHIP_16(x) READ_16(chipRam + ((x) % chipRamSize))
#define READ_CHIP_32(x) READ_32(chipRam + ((x) % chipRamSize))

// Reads a value from Fast RAM in big endian format
#define READ_FAST_8(x)  READ_8(fastRam + ((x) - FAST_RAM_STRT))
#define READ_FAST_16(x) READ_16(fastRam + ((x) - FAST_RAM_STRT))
#define READ_FAST_32(x) READ_32(fastRam + ((x) - FAST_RAM_STRT))

// Reads a value from Slow RAM in big endian format
#define READ_SLOW_8(x)  READ_8(slowRam + ((x) & SLOW_RAM_MASK))
#define READ_SLOW_16(x) READ_16(slowRam + ((x) & SLOW_RAM_MASK))
#define READ_SLOW_32(x) READ_32(slowRam + ((x) & SLOW_RAM_MASK))

// Reads a value from Boot ROM in big endian format
#define READ_BOOT_8(x)  READ_8(bootRom + ((x) & BOOT_ROM_MASK))
#define READ_BOOT_16(x) READ_16(bootRom + ((x) & BOOT_ROM_MASK))
#define READ_BOOT_32(x) READ_32(bootRom + ((x) & BOOT_ROM_MASK))

// Reads a value from Kickstart ROM in big endian format
#define READ_KICK_8(x)  READ_8(kickRom + ((x) % kickRomSize))
#define READ_KICK_16(x) READ_16(kickRom + ((x) % kickRomSize))
#define READ_KICK_32(x) READ_32(kickRom + ((x) % kickRomSize))

// Reads a value from Extended ROM in big endian format
#define READ_EXT_8(x)  READ_8(extRom + ((x) % extRomSize))
#define READ_EXT_16(x) READ_16(extRom + ((x) % extRomSize))
#define READ_EXT_32(x) READ_32(extRom + ((x) % extRomSize))

// Writes a value into memory in big endian format
#define WRITE_8(x,y)  (*(uint8_t *)(x) = y)
#define WRITE_16(x,y) (*(uint16_t *)(x) = htons(y))
#define WRITE_32(x,y) (*(uint32_t *)(x) = htonl(y))

// Writes a value into Chip RAM in big endian format
#define WRITE_CHIP_8(x,y)  WRITE_8(chipRam + ((x) % chipRamSize), (y))
#define WRITE_CHIP_16(x,y) WRITE_16(chipRam + ((x) % chipRamSize), (y))
#define WRITE_CHIP_32(x,y) WRITE_32(chipRam + ((x) % chipRamSize), (y))

// Writes a value into Fast RAM in big endian format
#define WRITE_FAST_8(x,y)  WRITE_8(fastRam + ((x) - FAST_RAM_STRT), (y))
#define WRITE_FAST_16(x,y) WRITE_16(fastRam + ((x) - FAST_RAM_STRT), (y))
#define WRITE_FAST_32(x,y) WRITE_32(fastRam + ((x) - FAST_RAM_STRT), (y))

// Writes a value into Slow RAM in big endian format
#define WRITE_SLOW_8(x,y)  WRITE_8(slowRam + ((x) & SLOW_RAM_MASK), (y))
#define WRITE_SLOW_16(x,y) WRITE_16(slowRam + ((x) & SLOW_RAM_MASK), (y))
#define WRITE_SLOW_32(x,y) WRITE_32(slowRam + ((x) & SLOW_RAM_MASK), (y))

// Writes a value into Boot ROM in big endian format
#define WRITE_BOOT_8(x,y)  WRITE_8(bootRom + ((x) & BOOT_ROM_MASK), (y))
#define WRITE_BOOT_16(x,y) WRITE_16(bootRom + ((x) & BOOT_ROM_MASK), (y))
#define WRITE_BOOT_32(x,y) WRITE_32(bootRom + ((x) & BOOT_ROM_MASK), (y))

// Writes a value into Kickstart ROM in big endian format
#define WRITE_KICK_8(x,y)  WRITE_8(kickRom + ((x) % kickRomSize), (y))
#define WRITE_KICK_16(x,y) WRITE_16(kickRom + ((x) % kickRomSize), (y))
#define WRITE_KICK_32(x,y) WRITE_32(kickRom + ((x) % kickRomSize), (y))

// Writes a value into Extended ROM in big endian format
#define WRITE_EXT_8(x,y)  WRITE_8(extRom + ((x) & EXT_ROM_MASK), (y))
#define WRITE_EXT_16(x,y) WRITE_16(extRom + ((x) & EXT_ROM_MASK), (y))
#define WRITE_EXT_32(x,y) WRITE_32(extRom + ((x) & EXT_ROM_MASK), (y))


class Memory : public HardwareComponent {
    
    friend class Copper;

    // Quick-access references
    class CPU *cpu;
    class CIAA *ciaA;
    class CIAB *ciaB;
    class Agnus *agnus;
    class Copper *copper;
    class Denise *denise;
    class Paula *paula;
    class ZorroManager *zorro; 
    
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
    
    // Boot Rom and size (Amiga 1000 only)
    uint8_t *bootRom = NULL;
    size_t bootRomSize = 0;
    
    // Kickstart Rom and size
    uint8_t *kickRom = NULL;
    size_t kickRomSize = 0;

    // Extended Rom and size
    uint8_t *extRom = NULL;
    size_t extRomSize = 0;

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
     * A1000 is emulated, it is true on startup to emulate a WOM (Write Once
     * Memory).
     */
    bool kickIsWritable = false;
    
    /* We divide the memory into banks of size 64KB.
     * The Amiga has 24 address lines. Hence, the accessible memory is divided
     * into 256 different banks. For each bank, this array indicates the type
     * of memory seen by the Amiga.
     * See also: updateMemSrcTable()
     */
    MemorySource memSrc[256];
    
    // Buffer for returning string values
    char str[256];
    
    
    //
    // Constructing and destructing
    //
    
    public:
    
    Memory();
    ~Memory();
    
    // Frees the allocated memory
    void dealloc();
    
    
    //
    // Methods from HardwareComponent
    //
    
    private:

    void _initialize() override;
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
    
    size_t stateSize() override;
    void didLoadFromBuffer(uint8_t **buffer) override;
    void didSaveToBuffer(uint8_t **buffer) override;
    
    
    //
    // Allocating memory
    //
    
    private:
    
    /* Allocates 'size' bytes of memory.
     * As side effects, the memory lookup table is updated and a memory
     * layout message is sent to the GUI.
     */
    bool alloc(size_t size, uint8_t *&ptrref, size_t &sizeref);
    
    
    //
    // Managing RAM
    //
    
    public:
    
    bool hasChipRam() { return chipRam != NULL; }
    bool allocateChipRam(size_t size) { return alloc(size, chipRam, chipRamSize); }
    
    bool hasSlowRam() { return slowRam != NULL; }
    bool allocateSlowRam(size_t size) { return alloc(size, slowRam, slowRamSize); }
    
    bool hasFastRam() { return fastRam != NULL; }
    bool allocateFastRam(size_t size) { return alloc(size, fastRam, fastRamSize); }
    
    
    //
    // Managing ROM
    //
    
    private:
    
    // Loads Rom data from a file.
    void loadRom(AmigaFile *rom, uint8_t *target, size_t length);
    
    public:
    
    // Checks if a certain ROM is installed
    bool hasBootRom() { return bootRom != NULL; }
    bool hasKickRom() { return kickRom != NULL; }
    bool hasExtRom() { return extRom != NULL; }

    // Returns a fingerprint for a certain ROM
    uint64_t bootRomFingerprint() { return fnv_1a(bootRom, bootRomSize); }
    uint64_t kickRomFingerprint() { return fnv_1a(kickRom, kickRomSize); }
    uint64_t extRomFingerprint() { return fnv_1a(extRom,  extRomSize); }

    // Deletes a previously installed ROM
    void deleteBootRom() { alloc(0, bootRom, bootRomSize); }
    void deleteKickRom() { alloc(0, kickRom, kickRomSize); }
    void deleteExtRom() { alloc(0, extRom, extRomSize); }

    // Installs a new ROM
    bool loadBootRom(BootRom *rom);
    bool loadBootRomFromBuffer(const uint8_t *buffer, size_t length);
    bool loadBootRomFromFile(const char *path);

    bool loadKickRom(KickRom *rom);
    bool loadKickRomFromBuffer(const uint8_t *buffer, size_t length);
    bool loadKickRomFromFile(const char *path);

    bool loadExtRom(ExtRom *rom);
    bool loadExtRomFromBuffer(const uint8_t *buffer, size_t length);
    bool loadExtRomFromFile(const char *path);

    
    //
    // Managing the memory source table
    //
    
    public:
    
    // Returns the memory source lookup table.
    MemorySource *getMemSrcTable() { return memSrc; }
    
    // Returns the memory source for a given address.
    MemorySource getMemSrc(uint32_t addr) { return memSrc[(addr >> 16) & 0xFF]; }
    
    // Updates the memory source lookup table.
    void updateMemSrcTable();
    
    
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
    
    inline uint8_t peekChip8(uint32_t addr) {
        ASSERT_CHIP_ADDR(addr); return READ_CHIP_8(addr);
    }
    inline uint16_t peekChip16(uint32_t addr) {
        ASSERT_CHIP_ADDR(addr); return READ_CHIP_16(addr);
    }
    inline uint32_t peekChip32(uint32_t addr) {
        ASSERT_CHIP_ADDR(addr); return READ_CHIP_32(addr);
    }
    
    inline uint8_t spypeekChip8(uint32_t addr) {
        return peekChip8(addr);
    }
    inline uint16_t spypeekChip16(uint32_t addr) {
        return peekChip16(addr);
    }
    inline uint32_t spypeekChip32(uint32_t addr) {
        return peekChip32(addr);
    }
    
    inline void pokeChip8(uint32_t addr, uint8_t value) {
        ASSERT_CHIP_ADDR(addr); WRITE_CHIP_8(addr, value);
    }
    inline void pokeChip16(uint32_t addr, uint16_t value) {
        ASSERT_CHIP_ADDR(addr); WRITE_CHIP_16(addr, value);
    }
    inline void pokeChip32(uint32_t addr, uint32_t value) {
        ASSERT_CHIP_ADDR(addr); WRITE_CHIP_32(addr, value);
    }
    
    //
    // CIA space
    //
    
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
    // RTC space
    //
    
    uint8_t peekRTC8(uint32_t addr);
    uint16_t peekRTC16(uint32_t addr);
    uint32_t peekRTC32(uint32_t addr);
    
    uint8_t spypeekRTC8(uint32_t addr) { return peekRTC8(addr); }
    uint16_t spypeekRTC16(uint32_t addr) { return peekRTC16(addr); }
    uint32_t spypeekRTC32(uint32_t addr) { return peekRTC32(addr); }
    
    void pokeRTC8(uint32_t addr, uint8_t value);
    void pokeRTC16(uint32_t addr, uint16_t value);
    void pokeRTC32(uint32_t addr, uint32_t value);
    
    //
    // Custom chips (OCS)
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
    
    //
    // Auto-config space (Zorro II)
    //
    
    uint8_t peekAutoConf8(uint32_t addr);
    uint16_t peekAutoConf16(uint32_t addr);
    
    uint8_t spypeekAutoConf8(uint32_t addr) { return peekAutoConf8(addr); }
    uint16_t spypeekAutoConf16(uint32_t addr) { return peekAutoConf16(addr); }
    
    void pokeAutoConf8(uint32_t addr, uint8_t value);
    void pokeAutoConf16(uint32_t addr, uint16_t value);

    //
    // Debugging
    //
    
    public:
    
    // Returns 16 bytes of memory as an ASCII string.
    const char *ascii(uint32_t addr);
    
    // Returns a certain amount of bytes as a string containing hex words.
    void hex(char *buffer, uint32_t addr, size_t bytes, size_t bufferSize);
    const char *hex(uint32_t addr, size_t bytes);

};

#endif
