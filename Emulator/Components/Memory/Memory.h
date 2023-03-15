// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MemoryTypes.h"
#include "SubComponent.h"
#include "RomFileTypes.h"
#include "MemUtils.h"

using util::Allocator;
using util::Buffer;

namespace vamiga {

#define SLOW_RAM_STRT 0xC00000
#define FAST_RAM_STRT ramExpansion.getBaseAddr()

// Verifies address ranges
#define ASSERT_CHIP_ADDR(x) \
assert(((x) % config.chipSize) == ((x) & chipMask));
#define ASSERT_FAST_ADDR(x) \
assert(((x) - FAST_RAM_STRT) < (u32)config.fastSize);
#define ASSERT_SLOW_ADDR(x) \
assert(((x) - SLOW_RAM_STRT) < (u32)config.slowSize);
#define ASSERT_ROM_ADDR(x) \
assert(((x) % config.romSize) == ((x) & romMask));
#define ASSERT_WOM_ADDR(x) \
assert(((x) % config.womSize) == ((x) & womMask));
#define ASSERT_EXT_ADDR(x)  \
assert(((x) % config.extSize) == ((x) & extMask));
#define ASSERT_CIA_ADDR(x) \
assert((x) >= 0xA00000 && (x) <= 0xBFFFFF);
#define ASSERT_RTC_ADDR(x) \
assert((x) >= 0xD80000 && (x) <= 0xDCFFFF);
#define ASSERT_CUSTOM_ADDR(x) \
assert((x) >= 0xC00000 && (x) <= 0xDFFFFF);
#define ASSERT_AUTO_ADDR(x) \
assert((x) >= 0xE80000 && (x) <= 0xE8FFFF);

//
// Reading
//

// Reads a value from Chip RAM in big endian format
#define READ_CHIP_8(x)      R8BE (chip + ((x) & chipMask))
#define READ_CHIP_16(x)     R16BE(chip + ((x) & chipMask))

// Reads a value from Fast RAM in big endian format
#define READ_FAST_8(x)      R8BE (fast + ((x) - FAST_RAM_STRT))
#define READ_FAST_16(x)     R16BE(fast + ((x) - FAST_RAM_STRT))

// Reads a value from Slow RAM in big endian format
#define READ_SLOW_8(x)      R8BE (slow + ((x) - SLOW_RAM_STRT))
#define READ_SLOW_16(x)     R16BE(slow + ((x) - SLOW_RAM_STRT))

// Reads a value from Boot ROM or Kickstart ROM in big endian format
#define READ_ROM_8(x)       R8BE (rom + ((x) & romMask))
#define READ_ROM_16(x)      R16BE(rom + ((x) & romMask))

// Reads a value from Kickstart WOM in big endian format
#define READ_WOM_8(x)       R8BE (wom + ((x) & womMask))
#define READ_WOM_16(x)      R16BE(wom + ((x) & womMask))

// Reads a value from Extended ROM in big endian format
#define READ_EXT_8(x)       R8BE (ext + ((x) & extMask))
#define READ_EXT_16(x)      R16BE(ext + ((x) & extMask))

//
// Writing
//

// Writes a value into Chip RAM in big endian format
#define WRITE_CHIP_8(x,y)   W8BE (chip + ((x) & chipMask), (y))
#define WRITE_CHIP_16(x,y)  W16BE(chip + ((x) & chipMask), (y))

// Writes a value into Fast RAM in big endian format
#define WRITE_FAST_8(x,y)   W8BE (fast + ((x) - FAST_RAM_STRT), (y))
#define WRITE_FAST_16(x,y)  W16BE(fast + ((x) - FAST_RAM_STRT), (y))

// Writes a value into Slow RAM in big endian format
#define WRITE_SLOW_8(x,y)   W8BE (slow + ((x) - SLOW_RAM_STRT), (y))
#define WRITE_SLOW_16(x,y)  W16BE(slow + ((x) - SLOW_RAM_STRT), (y))

// Writes a value into Boot ROM or Kickstart ROM in big endian format
#define WRITE_ROM_8(x,y)    W8BE (rom + ((x) & romMask), (y))
#define WRITE_ROM_16(x,y)   W16BE(rom + ((x) & romMask), (y))

// Writes a value into Kickstart WOM in big endian format
#define WRITE_WOM_8(x,y)    W8BE (wom + ((x) & womMask), (y))
#define WRITE_WOM_16(x,y)   W16BE(wom + ((x) & womMask), (y))

// Writes a value into Extended ROM in big endian format
#define WRITE_EXT_8(x,y)    W8BE (ext + ((x) & extMask), (y))
#define WRITE_EXT_16(x,y)   W16BE(ext + ((x) & extMask), (y))


class Memory : public SubComponent {

    // Current configuration
    MemoryConfig config = {};

    // Current workload
    MemoryStats stats = {};

public:

    /* About
     *
     * There are 6 types of dynamically allocated memory:
     *
     *     rom: Read-only memory
     *          Holds a Kickstart Rom or a Boot Rom (A1000).

     *     wom: Write-once Memory
     *          If rom holds a Boot Rom, a wom is automatically created. It
     *          is the place where the A1000 stores the Kickstart loaded
     *          from disk.
     *
     *     ext: Extended Rom
     *          Such a Rom was added to newer Amiga models when the 512 KB
     *          Kickstart Rom became too small. It is emulated to support
     *          the Aros Kickstart replacement.
     *
     *    chip: Chip Ram
     *          Holds the memory which is shared by the CPU and the Amiga Chip
     *          set. The original Agnus chip is able to address 512 KB Chip
     *          memory. Newer models are able to address up to 2 MB.
     *
     *    slow: Slow Ram (aka Bogo Ram)
     *          This Ram is addressed by the same bus as Chip Ram, but it can
     *          used by the CPU only.
     *
     *    fast: Fast Ram
     *          Only the CPU can access this Ram. It is connected via a
     *          seperate bus and doesn't slow down the Chip set when the CPU
     *          addresses it.
     *
     * Each memory type is represented by three variables:
     *
     *    A pointer to the allocates memory.
     *    A variable storing the memory size in bytes (in MemoryConfig).
     *    A bit mask to emulate address mirroring.
     *
     * The following invariants hold:
     *
     *    pointer == nullptr <=> config.size == 0 <=> mask == 0
     *    pointer != nullptr <=> mask == config.size - 1
     *
     */
    u8 *rom;
    u8 *wom;
    u8 *ext;
    u8 *chip;
    u8 *slow;
    u8 *fast;

    Allocator<u8> romAllocator = Allocator(rom);
    Allocator<u8> womAllocator = Allocator(wom);
    Allocator<u8> extAllocator = Allocator(ext);
    Allocator<u8> chipAllocator = Allocator(chip);
    Allocator<u8> slowAllocator = Allocator(slow);
    Allocator<u8> fastAllocator = Allocator(fast);

    u32 romMask = 0;
    u32 womMask = 0;
    u32 extMask = 0;
    u32 chipMask = 0;

    /* Indicates if the Kickstart Wom is writable. If an Amiga 1000 Boot Rom is
     * installed, a Kickstart WOM (Write Once Memory) is added automatically.
     * On startup, the WOM is unlocked which means that it is writable. During
     * the boot process, the WOM gets locked.
     */
    bool womIsLocked = false;
    
    /* The Amiga memory is divided into 256 banks of size 64KB. The following
     * tables indicate which memory type is seen in each bank by the CPU and
     * Agnus, respectively.
     * See also: updateMemSrcTables()
     */
    MemorySource cpuMemSrc[256];
    MemorySource agnusMemSrc[256];

    // The last value on the data bus
    u16 dataBus;

    // Static buffer for returning textual representations
    char str[256];
    

    //
    // Initializing
    //
    
public:
    
    using SubComponent::SubComponent;


    //
    // Methods from CoreObject
    //
    
private:
    
    const char *getDescription() const override { return "Memory"; }
    void _dump(Category category, std::ostream& os) const override;
    
    
    //
    // Methods from CoreComponent
    //
    
private:
    
    void _initialize() override;
    void _reset(bool hard) override;
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker
        
        << config.slowRamDelay
        << config.bankMap
        << config.ramInitPattern
        << config.unmappingType
        << config.extStart;
    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        worker

        << womIsLocked
        << cpuMemSrc
        << agnusMemSrc
        << dataBus;
    }

    isize _size() override;
    u64 _checksum() override;
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    isize didLoadFromBuffer(const u8 *buffer) override;
    isize didSaveToBuffer(u8 *buffer) override;

    
    //
    // Configuring
    //
    
public:
    
    const MemoryConfig &getConfig() const { return config; }
    void resetConfig() override;
    
    i64 getConfigItem(Option option) const;
    void setConfigItem(Option option, i64 value);

    
    //
    // Analyzing
    //
    
public:
    
    const MemoryStats &getStats() { return stats; }
    
    void clearStats() { stats = { }; }
    void updateStats();

    
    //
    // Controlling
    //
    
private:

    void _isReady() const throws override;


    //
    // Allocating memory
    //
    
public:

    void allocChip(i32 bytes, bool update = true);
    void allocSlow(i32 bytes, bool update = true);
    void allocFast(i32 bytes, bool update = true);

    void deleteChip() { allocChip(0); }
    void deleteSlow() { allocSlow(0); }
    void deleteFast() { allocFast(0); }

    void allocRom(i32 bytes, bool update = true);
    void allocWom(i32 bytes, bool update = true);
    void allocExt(i32 bytes, bool update = true);

    void deleteRom() { allocRom(0); }
    void deleteWom() { allocWom(0); }
    void deleteExt() { allocExt(0); }

private:
    
    void alloc(Allocator<u8> &allocator, isize bytes, bool update);
    void alloc(Allocator<u8> &allocator, isize bytes, u32 &mask, bool update);


    //
    // Managing RAM
    //
    
public:

    // Check if a certain Ram is present
    bool hasChipRam() const { return chip != nullptr; }
    bool hasSlowRam() const { return slow != nullptr; }
    bool hasFastRam() const { return fast != nullptr; }

    // Returns the size of a certain Ram in bytes
    isize chipRamSize() const { return config.chipSize; }
    isize slowRamSize() const { return config.slowSize; }
    isize fastRamSize() const { return config.fastSize; }
    isize ramSize() const { return config.chipSize + config.slowSize + config.fastSize; }

private:
    
    void fillRamWithInitPattern();

    
    //
    // Managing ROM
    //

public:

    // Computes a CRC-32 checksum
    u32 romFingerprint() const;
    u32 extFingerprint() const;

    // Returns the ROM identifiers of the currently installed ROMs
    RomIdentifier romIdentifier() const;
    RomIdentifier extIdentifier() const;

    const char *romTitle();
    const char *romVersion();
    const char *romReleased();
    const char *romModel();

    const char *extTitle();
    const char *extVersion();
    const char *extReleased();
    const char *extModel();

    // Checks if a certain Rom is present
    bool hasRom() const { return rom != nullptr; }
    bool hasBootRom() const { return hasRom() && config.romSize <= KB(16); }
    bool hasKickRom() const { return hasRom() && config.romSize >= KB(256); }
    bool hasArosRom() const;
    bool hasWom() const { return wom != nullptr; }
    bool hasExt() const { return ext != nullptr; }

    // Erases an installed Rom
    void eraseRom() { std::memset(rom, 0, config.romSize); }
    void eraseWom() { std::memset(wom, 0, config.womSize); }
    void eraseExt() { std::memset(ext, 0, config.extSize); }
    
    // Installs a Boot Rom or Kickstart Rom
    void loadRom(class RomFile &rom) throws;
    void loadRom(const string &path) throws;
    void loadRom(const u8 *buf, isize len) throws;
    
    // Installs a Kickstart expansion Rom
    void loadExt(class ExtendedRomFile &rom) throws;
    void loadExt(const string &path) throws;
    void loadExt(const u8 *buf, isize len) throws;

    // Saves a Rom to disk
    void saveRom(const string &path) throws;
    void saveWom(const string &path) throws;
    void saveExt(const string &path) throws;

    // Fixes two bugs in Kickstart 1.2 expansion.library
    void patchExpansionLib();

    // Returns true iff the Rom is a ReKick image
    bool isRelocated();

    
    //
    // Maintaining the memory source table
    //
    
public:

    // Returns the memory source for a given address
    template <Accessor A> MemorySource getMemSrc(u32 addr);
    
    // Updates both memory source lookup tables
    void updateMemSrcTables();
    
    // Checks if an address belongs to a certain memory area
    bool inChipRam(u32 addr);
    bool inSlowRam(u32 addr);
    bool inFastRam(u32 addr);
    bool inRam(u32 addr);
    bool inRom(u32 addr);

    
private:

    void updateCpuMemSrcTable();
    void updateAgnusMemSrcTable();

    
    //
    // Accessing memory
    //
    
public:

    template <Accessor acc, MemorySource src> u8 peek8(u32 addr);
    template <Accessor acc, MemorySource src> u16 peek16(u32 addr);
    template <Accessor acc, MemorySource src> u8 spypeek8(u32 addr) const;
    template <Accessor acc, MemorySource src> u16 spypeek16(u32 addr) const;
    template <Accessor acc, MemorySource src> u32 spypeek32(u32 addr) const;
    template <Accessor acc> u8 peek8(u32 addr);
    template <Accessor acc> u16 peek16(u32 addr);
    template <Accessor acc> u8 spypeek8(u32 addr) const;
    template <Accessor acc> u16 spypeek16(u32 addr) const;
    template <Accessor acc> u32 spypeek32(u32 addr) const;
    template <Accessor acc> void spypeek(u32 addr, isize len, u8 *buf) const;

    template <Accessor acc, MemorySource src> void poke8(u32 addr, u8 value);
    template <Accessor acc, MemorySource src> void poke16(u32 addr, u16 value);
    template <Accessor acc> void poke8(u32 addr, u8 value);
    template <Accessor acc> void poke16(u32 addr, u16 value);
    

    //
    // Accessing the CIA space
    //
    
    u8 peekCIA8(u32 addr);
    u16 peekCIA16(u32 addr);
    
    u8 spypeekCIA8(u32 addr) const;
    u16 spypeekCIA16(u32 addr) const;
    
    void pokeCIA8(u32 addr, u8 value);
    void pokeCIA16(u32 addr, u16 value);


    //
    // Accessing the RTC space
    //
    
    u8 peekRTC8(u32 addr) const;
    u16 peekRTC16(u32 addr) const;
    
    void pokeRTC8(u32 addr, u8 value);
    void pokeRTC16(u32 addr, u16 value);

    
    //
    // Accessing the custom chip space
    //
    
    u16 peekCustom16(u32 addr);
    u16 peekCustomFaulty16(u32 addr);
    
    u16 spypeekCustom16(u32 addr) const;

    template <Accessor s> void pokeCustom16(u32 addr, u16 value);
    
    
    //
    // Patching Ram or Rom
    //
    
    // Modifies Ram or Rom without causing side effects
    template <MemorySource src> void patch(u32 addr, u8 value);
    void patch(u32 addr, u8 value);
    void patch(u32 addr, u16 value);
    void patch(u32 addr, u32 value);
    void patch(u32 addr, u8 *buf, isize len);

    
    //
    // Debugging
    //
    
public:
    
    // Returns the name of a chipset register
    static const char *regName(u32 addr);
    
    // Returns 16 bytes of memory as an ASCII string
    template <Accessor A> const char *ascii(u32 addr, isize numBytes);
    
    // Returns a certain amount of bytes as a string containing hex words
    template <Accessor A> const char *hex(u32 addr, isize numBytes);

    // Creates a memory dump
    template <Accessor A> void memDump(std::ostream& os, u32 addr, isize numLines = 16);

    // Searches RAM and ROM for a certain byte sequence
    std::vector <u32> search(u64 pattern, isize bytes);
    std::vector <u32> search(auto pattern) { return search(pattern, isizeof(pattern)); }
};

}
