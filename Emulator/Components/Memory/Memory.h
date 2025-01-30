// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MemoryTypes.h"
#include "MemoryDebugger.h"
#include "RomFileTypes.h"
#include "MemUtils.h"
#include "Buffer.h"

namespace vamiga {

using util::Allocator;
using util::Buffer;

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


class Memory final : public SubComponent, public Inspectable<MemInfo, MemStats> {

    Descriptions descriptions = {{

        .type           = Class::Memory,
        .name           = "Memory",
        .description    = "Memory",
        .shell          = "mem"
    }};

    ConfigOptions options = {

        Opt::MEM_CHIP_RAM,
        Opt::MEM_SLOW_RAM,
        Opt::MEM_FAST_RAM,
        Opt::MEM_EXT_START,
        Opt::MEM_SAVE_ROMS,
        Opt::MEM_SLOW_RAM_DELAY,
        Opt::MEM_SLOW_RAM_MIRROR,
        Opt::MEM_BANKMAP,
        Opt::MEM_UNMAPPING_TYPE,
        Opt::MEM_RAM_INIT_PATTERN
    };

    // Current configuration
    MemConfig config = {};

public:

    // Subcomponents
    MemoryDebugger debugger = MemoryDebugger(amiga);

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
     *    A variable storing the memory size in bytes (in MemConfig).
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
    MemSrc cpuMemSrc[256];
    MemSrc agnusMemSrc[256];

    // The last value on the data bus
    u16 dataBus;

    // Static buffer for returning textual representations
    // TODO: Replace by "static string str" and make it local
    char str[256];
    

    //
    // Methods
    //

public:
    
    Memory(Amiga& ref);

    Memory& operator= (const Memory& other) {

        CLONE(romAllocator)
        CLONE(womAllocator)
        CLONE(extAllocator)
        CLONE(chipAllocator)
        CLONE(slowAllocator)
        CLONE(fastAllocator)

        CLONE(womIsLocked)
        CLONE_ARRAY(cpuMemSrc)
        CLONE_ARRAY(agnusMemSrc)
        CLONE(dataBus)

        CLONE(romMask)
        CLONE(womMask)
        CLONE(extMask)
        CLONE(chipMask)

        CLONE(config)
        
        return *this;
    }


    //
    // Methods from Serializable
    //

public:

    void _initialize() override;

    template <class T>
    void serialize(T& worker)
    {
        worker

        << womIsLocked
        << cpuMemSrc
        << agnusMemSrc
        << dataBus;

        if (isResetter(worker)) return;

        worker

        << romMask
        << womMask
        << extMask
        << chipMask

        << config.extStart
        << config.saveRoms
        << config.slowRamDelay
        << config.slowRamMirror
        << config.bankMap
        << config.ramInitPattern
        << config.unmappingType;
    }

    void operator << (SerResetter &worker) override;
    void operator << (SerChecker &worker) override;
    void operator << (SerCounter &worker) override;
    void operator << (SerReader &worker) override;
    void operator << (SerWriter &worker) override;
    void _didReset(bool hard) override;
    

    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream& os) const override;


    //
    // Methods from Inspectable
    //

public:

    void cacheInfo(MemInfo &result) const override;
    // void cacheStats(MemStats &result) const override;

private:
    
    void updateStats();
    

    //
    // Methods from Configurable
    //

public:
    
    const MemConfig &getConfig() const { return config; }
    const ConfigOptions &getOptions() const override { return options; }
    i64 getOption(Opt option) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt option, i64 value) override;

    
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

    // Queries ROM information
    static RomTraits &getRomTraits(u32 crc);
    RomTraits &getRomTraits() const;
    RomTraits &getWomTraits() const;
    RomTraits &getExtTraits() const;

    // Computes a CRC-32 checksum
    u32 romFingerprint() const;
    u32 extFingerprint() const;

    // Checks if a certain Rom is present
    bool hasRom() const { return rom != nullptr; }
    bool hasBootRom() const { return hasRom() && config.romSize <= KB(16); }
    bool hasKickRom() const { return hasRom() && config.romSize >= KB(256); }
    bool hasWom() const { return wom != nullptr; }
    bool hasExt() const { return ext != nullptr; }

    // Erases an installed Rom
    void eraseRom() { std::memset(rom, 0, config.romSize); }
    void eraseWom() { std::memset(wom, 0, config.womSize); }
    void eraseExt() { std::memset(ext, 0, config.extSize); }
    
    // Installs a Boot Rom or Kickstart Rom
    void loadRom(class MediaFile &file) throws;
    void loadRom(const std::filesystem::path &path) throws;
    void loadRom(const u8 *buf, isize len) throws;
    
    // Installs a Kickstart expansion Rom
    void loadExt(class MediaFile &file) throws;
    void loadExt(const std::filesystem::path &path) throws;
    void loadExt(const u8 *buf, isize len) throws;

    // Saves a Rom to disk
    void saveRom(const std::filesystem::path &path) const throws;
    void saveWom(const std::filesystem::path &path) const throws;
    void saveExt(const std::filesystem::path &path) const throws;

    // Fixes two bugs in Kickstart 1.2 expansion.library
    void patchExpansionLib();

    // Returns true iff the Rom is a ReKick image
    bool isRelocated();

    
    //
    // Maintaining the memory source table
    //
    
public:

    // Returns the memory source for a given address
    template <Accessor A> MemSrc getMemSrc(u32 addr);
    
    // Updates both memory source lookup tables
    void updateMemSrcTables();
    
    // Checks if an address belongs to a certain memory area
    bool inChipRam(u32 addr);
    bool inSlowRam(u32 addr);
    bool inFastRam(u32 addr);
    bool inRam(u32 addr);
    bool inRom(u32 addr);
    bool isUnmapped(u32 addr);

private:

    // Called inside updateMemSrcTables()
    void updateCpuMemSrcTable();
    void updateAgnusMemSrcTable();

    // Checks whether Agnus is able to access Slow Ram
    bool slowRamIsMirroredIn() const;


    //
    // Accessing memory
    //
    
public:

    template <Accessor acc, MemSrc src> u8 peek8(u32 addr);
    template <Accessor acc, MemSrc src> u16 peek16(u32 addr);
    template <Accessor acc, MemSrc src> u8 spypeek8(u32 addr) const;
    template <Accessor acc, MemSrc src> u16 spypeek16(u32 addr) const;
    template <Accessor acc, MemSrc src> u32 spypeek32(u32 addr) const;
    template <Accessor acc> u8 peek8(u32 addr);
    template <Accessor acc> u16 peek16(u32 addr);
    template <Accessor acc> u8 spypeek8(u32 addr) const;
    template <Accessor acc> u16 spypeek16(u32 addr) const;
    template <Accessor acc> u32 spypeek32(u32 addr) const;
    template <Accessor acc> void spypeek(u32 addr, isize len, u8 *buf) const;

    template <Accessor acc, MemSrc src> void poke8(u32 addr, u8 value);
    template <Accessor acc, MemSrc src> void poke16(u32 addr, u16 value);
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
    template <MemSrc src> void patch(u32 addr, u8 value);
    void patch(u32 addr, u8 value);
    void patch(u32 addr, u16 value);
    void patch(u32 addr, u32 value);
    void patch(u32 addr, u8 *buf, isize len);


    //
    // Perfoming periodic tasks
    //

    // Finishes up the current frame
    void eofHandler();


    //
    // Debugging
    //
    
public:

    // Searches RAM and ROM for a certain byte sequence
    std::vector <u32> search(u64 pattern, isize bytes);
    std::vector <u32> search(auto pattern) { return search(pattern, isizeof(pattern)); }
};

}
