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

#include "SubComponent.h"
#include "RomFile.h"
#include "ExtFile.h"

const uint32_t FAST_RAM_STRT = 0x200000;
const uint32_t SLOW_RAM_MASK = 0x07FFFF; // 512 KB
const uint32_t EXT_ROM_MASK  = 0x07FFFF; // 512 KB

// Verifies the range of an address
#define ASSERT_CHIP_ADDR(x) assert(chipRam != NULL);
#define ASSERT_FAST_ADDR(x) assert(fastRam != NULL); assert(((x) - FAST_RAM_STRT) < config.fastRamSize);
#define ASSERT_SLOW_ADDR(x) assert(slowRam != NULL); assert(((x) & SLOW_RAM_MASK) < config.slowRamSize);
#define ASSERT_ROM_ADDR(x) assert(rom != NULL);
#define ASSERT_WOM_ADDR(x) assert(wom != NULL);
#define ASSERT_EXT_ADDR(x)  assert(ext  != NULL); assert(((x) & EXT_ROM_MASK) < config.extSize);
#define ASSERT_CIA_ADDR(x)  assert((x) >= 0xA00000 && (x) <= 0xBFFFFF);
#define ASSERT_RTC_ADDR(x)  assert((x) >= 0xDC0000 && (x) <= 0xDEFFFF);
#define ASSERT_OCS_ADDR(x)  assert((x) >= 0xC00000 && (x) <= 0xDFFFFF);
#define ASSERT_AUTO_ADDR(x) assert((x) >= 0xE80000 && (x) <= 0xEFFFFF);

// Reads a value from memory in big endian format
#define READ_8(x)  (*(uint8_t *)(x))
#define READ_16(x) (ntohs(*(uint16_t *)(x)))
#define READ_32(x) (ntohl(*(uint32_t *)(x)))

// Reads a value from Chip RAM in big endian format
#define READ_CHIP_8(x)  READ_8(chipRam + ((x) % config.chipRamSize))
#define READ_CHIP_16(x) READ_16(chipRam + ((x) % config.chipRamSize))
#define READ_CHIP_32(x) READ_32(chipRam + ((x) % config.chipRamSize))

// Reads a value from Fast RAM in big endian format
#define READ_FAST_8(x)  READ_8(fastRam + ((x) - FAST_RAM_STRT))
#define READ_FAST_16(x) READ_16(fastRam + ((x) - FAST_RAM_STRT))
#define READ_FAST_32(x) READ_32(fastRam + ((x) - FAST_RAM_STRT))

// Reads a value from Slow RAM in big endian format
#define READ_SLOW_8(x)  READ_8(slowRam + ((x) & SLOW_RAM_MASK))
#define READ_SLOW_16(x) READ_16(slowRam + ((x) & SLOW_RAM_MASK))
#define READ_SLOW_32(x) READ_32(slowRam + ((x) & SLOW_RAM_MASK))

// Reads a value from Boot ROM or Kickstart ROM in big endian format
#define READ_ROM_8(x)  READ_8(rom + ((x) % config.romSize))
#define READ_ROM_16(x) READ_16(rom + ((x) % config.romSize))
#define READ_ROM_32(x) READ_32(rom + ((x) % config.romSize))

// Reads a value from Kickstart WOM in big endian format
#define READ_WOM_8(x)  READ_8(wom + ((x) % config.womSize))
#define READ_WOM_16(x) READ_16(wom + ((x) % config.womSize))
#define READ_WOM_32(x) READ_32(wom + ((x) % config.womSize))

// Reads a value from Extended ROM in big endian format
// #define READ_EXT_8(x)  READ_8(ext + ((x) % config.extSize))
// #define READ_EXT_16(x) READ_16(ext + ((x) % config.extSize))
// #define READ_EXT_32(x) READ_32(ext + ((x) % config.extSize))

#define READ_EXT_8(x)  READ_8(ext + ((x) & 0x7FFFF))
#define READ_EXT_16(x) READ_16(ext + ((x) & 0x7FFFF))
#define READ_EXT_32(x) READ_32(ext + ((x) & 0x7FFFF))

// Writes a value into memory in big endian format
#define WRITE_8(x,y)  (*(uint8_t *)(x) = y)
#define WRITE_16(x,y) (*(uint16_t *)(x) = htons(y))
#define WRITE_32(x,y) (*(uint32_t *)(x) = htonl(y))

// Writes a value into Chip RAM in big endian format
#define WRITE_CHIP_8(x,y)  WRITE_8(chipRam + ((x) % config.chipRamSize), (y))
#define WRITE_CHIP_16(x,y) WRITE_16(chipRam + ((x) % config.chipRamSize), (y))
#define WRITE_CHIP_32(x,y) WRITE_32(chipRam + ((x) % config.chipRamSize), (y))

// Writes a value into Fast RAM in big endian format
#define WRITE_FAST_8(x,y)  WRITE_8(fastRam + ((x) - FAST_RAM_STRT), (y))
#define WRITE_FAST_16(x,y) WRITE_16(fastRam + ((x) - FAST_RAM_STRT), (y))
#define WRITE_FAST_32(x,y) WRITE_32(fastRam + ((x) - FAST_RAM_STRT), (y))

// Writes a value into Slow RAM in big endian format
#define WRITE_SLOW_8(x,y)  WRITE_8(slowRam + ((x) & SLOW_RAM_MASK), (y))
#define WRITE_SLOW_16(x,y) WRITE_16(slowRam + ((x) & SLOW_RAM_MASK), (y))
#define WRITE_SLOW_32(x,y) WRITE_32(slowRam + ((x) & SLOW_RAM_MASK), (y))

// Writes a value into Kickstart ROM in big endian format (DEPRECATED)
#define WRITE_ROM_8(x,y)  WRITE_8(rom + ((x) % config.romSize), (y))
#define WRITE_ROM_16(x,y) WRITE_16(rom + ((x) % config.romSize), (y))
#define WRITE_ROM_32(x,y) WRITE_32(rom + ((x) % config.romSize), (y))

// Writes a value into Kickstart WOM in big endian format
#define WRITE_WOM_8(x,y)  WRITE_8(wom + ((x) % config.womSize), (y))
#define WRITE_WOM_16(x,y) WRITE_16(wom + ((x) % config.womSize), (y))
#define WRITE_WOM_32(x,y) WRITE_32(wom + ((x) % config.womSize), (y))

// Writes a value into Extended ROM in big endian format
#define WRITE_EXT_8(x,y)  WRITE_8(ext + ((x) & EXT_ROM_MASK), (y))
#define WRITE_EXT_16(x,y) WRITE_16(ext + ((x) & EXT_ROM_MASK), (y))
#define WRITE_EXT_32(x,y) WRITE_32(ext + ((x) & EXT_ROM_MASK), (y))


class Memory : public SubComponent {

    friend class Copper;
    friend class ZorroManager;

    // The current configuration
    MemoryConfig config;

    // Statistics shown in the GUI monitor panel
    MemoryStats stats;

public:
    
    /* Each memory area is represented by three variables:
     *
     *   A pointer to the allocates memory.
     *   A variable storing the memory size in bytes (in MemoryConfig).
     *
     * The following invariant holds:
     *
     *   pointer == NULL <=> config.size == 0
     */
    uint8_t *rom = NULL;
    uint8_t *wom = NULL;
    uint8_t *ext = NULL;
    uint8_t *chipRam = NULL;
    uint8_t *slowRam = NULL;
    uint8_t *fastRam = NULL;
    
    /* Indicates if the Kickstart Wom is writable
     * If an Amiga 1000 Boot Rom is installed, a Kickstart WOM (Write Once
     * Memory) is added automatically. On startup, the WOM is unlocked which
     * means that it is writable. During the boot process, the WOM will
     * be locked.
     */
    bool womIsLocked = false;
    // bool kickIsWritable = false;
    
    /* We divide the memory into banks of size 64KB.
     * The Amiga has 24 address lines. Hence, the accessible memory is divided
     * into 256 different banks. For each bank, this array indicates the type
     * of memory seen by the Amiga.
     * See also: updateMemSrcTable()
     */
    MemorySource memSrc[256];

    // The last value on the data bus
    uint16_t dataBus;

    // Buffer for returning string values
    char str[256];
    

    //
    // Iterating over snapshot items
    //

    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & womIsLocked
        & memSrc
        & dataBus;
    }


    //
    // Constructing and configuring
    //
    
public:
    
    Memory(Amiga& ref);
    ~Memory();
    
    // Frees the allocated memory
    void dealloc();

    // Returns the current configuration
    MemoryConfig getConfig() { return config; }



    //
    // Methods from HardwareComponent
    //
    
private:

    void _powerOn() override;
    void _reset() override;
    void _dump() override;
    
    size_t _size() override;
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(uint8_t *buffer) override { SAVE_SNAPSHOT_ITEMS }
    size_t didLoadFromBuffer(uint8_t *buffer) override;
    size_t didSaveToBuffer(uint8_t *buffer) const override;


    //
    // Statistics
    //

public:
    
    // Returns statistical information about the current activiy
    MemoryStats getStats() { return stats; }

    // Resets the collected statistical information
    void clearStats() { memset(&stats, 0, sizeof(stats)); }

    
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
    bool allocateChipRam(size_t size) { return alloc(size, chipRam, config.chipRamSize); }
    
    bool hasSlowRam() { return slowRam != NULL; }
    bool allocateSlowRam(size_t size) { return alloc(size, slowRam, config.slowRamSize); }
    
    bool hasFastRam() { return fastRam != NULL; }
    bool allocateFastRam(size_t size) { return alloc(size, fastRam, config.fastRamSize); }

    void initializeRam();

    
    //
    // Managing ROM
    //
    
private:
    
    // Loads Rom data from a file.
    void loadRom(AmigaFile *rom, uint8_t *target, size_t length);
    
public:
    
    // Checks if a certain ROM is present
    bool hasRom() { return rom != NULL; }
    bool hasBootRom() { return hasRom() && config.romSize <= KB(16); }
    bool hasKickRom() { return hasRom() && config.romSize >= KB(256); }
    bool hasWom() { return wom != NULL; }
    bool hasExtRom() { return ext != NULL; }

    // Returns a fingerprint for a certain ROM
    uint64_t kickRomFingerprint() { return fnv_1a_64(rom, config.romSize); }
    uint64_t extRomFingerprint() { return fnv_1a_64(ext,  config.extSize); }

    // Removes a previously installed ROM
    void deleteKickRom() { alloc(0, rom, config.romSize); }
    void deleteWom() { alloc(0, wom, config.womSize); }
    void deleteExtRom() { alloc(0, ext, config.extSize); }

    // Erases an installed ROM
    void eraseKickRom() { assert(rom); memset(rom, 0, config.romSize); }
    void eraseWom() { assert(wom); memset(wom, 0, config.womSize); }
    void eraseExtRom() { assert(ext); memset(ext, 0, config.extSize); }

    // Installs a new Boot Rom or Kickstart Rom
    bool loadRom(RomFile *rom);
    bool loadRomFromBuffer(const uint8_t *buffer, size_t length);
    bool loadRomFromFile(const char *path);

    bool loadExtRom(ExtFile *rom);
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
    template <BusOwner owner> uint16_t peek16(uint32_t addr);
    uint32_t peek32(uint32_t addr);

    uint8_t spypeek8(uint32_t addr);
    uint16_t spypeek16(uint32_t addr);
    uint32_t spypeek32(uint32_t addr);
    
    void poke8(uint32_t addr, uint8_t value);
    template <BusOwner owner> void poke16(uint32_t addr, uint16_t value);
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
    template <PokeSource s> void pokeCustom16(uint32_t addr, uint16_t value);
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
    // Boot ROM or Kickstart ROM
    //

    void pokeRom8(uint32_t addr, uint8_t value);
    void pokeRom16(uint32_t addr, uint16_t value);


    //
    // Kickstart WOM (Amiga 1000)
    //

    void pokeWom8(uint32_t addr, uint8_t value);
    void pokeWom16(uint32_t addr, uint16_t value);


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
