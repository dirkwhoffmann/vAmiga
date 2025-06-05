// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Memory.h"
#include "Emulator.h"
#include "Agnus.h"
#include "Checksum.h"
#include "CIA.h"
#include "CPU.h"
#include "Denise.h"
#include "RomFile.h"
#include "MsgQueue.h"
#include "Paula.h"
#include "RomFile.h"
#include "RTC.h"
#include "ZorroManager.h"
#include "RomDatabase.h"

namespace vamiga {

Memory::Memory(Amiga& ref) : SubComponent(ref)
{
    subComponents = std::vector<CoreComponent *> {

        &debugger
    };
}

void
Memory::_dump(Category category, std::ostream &os) const
{
    if (category == Category::Config) {
        
        dumpConfig(os);
    }

    if (category == Category::State) {

        auto romTraits = getRomTraits();
        auto womTraits = getWomTraits();
        auto extTraits = getExtTraits();

        os << util::tab("Rom");
        os << util::hex(romTraits.crc) << " (CRC32)  ";
        os << romTraits.title << " " << romTraits.released << std::endl;
        os << util::tab("Wom");
        os << util::hex(womTraits.crc) << " (CRC32)  ";
        os << womTraits.title << " " << womTraits.released << std::endl;
        os << util::tab("Extended Rom");
        os << util::hex(extTraits.crc) << " (CRC32)  ";
        os << extTraits.title << " " << extTraits.released << std::endl;
        os << util::tab("Chip Ram");
        os << util::hex(util::crc32(chip, config.chipSize)) << " (CRC32)  " << std::endl;
        os << util::tab("Slow Ram");
        os << util::hex(util::crc32(slow, config.slowSize)) << " (CRC32)  " << std::endl;
        os << util::tab("Fast Ram");
        os << util::hex(util::crc32(fast, config.fastSize)) << " (CRC32)  " << std::endl;

        os << std::endl;
        os << util::tab("Data bus");
        os << util::hex(dataBus) << std::endl;
        os << util::tab("Wom is locked");
        os << util::bol(womIsLocked) << std::endl;
    }
    
    if (category == Category::BankMap) {
        
        MemSrc oldsrc = cpuMemSrc[0];
        isize oldi = 0;
        
        for (isize i = 0; i <= 0x100; i++) {
            
            MemSrc newsrc = i < 0x100 ? cpuMemSrc[i] : (MemSrc)-1;
            
            if (oldsrc != newsrc) {
                
                os << "        ";
                os << util::hex((u8)(oldi)) << "0000" << " - ";
                os << util::hex((u8)(i - 1)) << "ffff : ";
                os << MemSrcEnum::key(oldsrc) << std::endl;

                oldsrc = newsrc;
                oldi = i;
            }
        }
    }
}

void
Memory::_initialize()
{    
    if (auto romPath = Emulator::defaults.getRaw("ROM_PATH"); romPath != "") {

        debug(CNF_DEBUG, "Trying to load Rom from %s...\n", romPath.c_str());
        
        try { loadRom(romPath); } catch (std::exception& e) {
            debug(CNF_DEBUG, "Error: %s\n", e.what());
        }
    }
    
    if (auto extPath = Emulator::defaults.getRaw("EXT_PATH"); extPath != "") {

        debug(CNF_DEBUG, "Trying to load extension Rom from %s...\n", extPath.c_str());
        
        try { loadExt(extPath); } catch (std::exception& e) {
            debug(CNF_DEBUG, "Error: %s\n", e.what());
        }
    }
}

i64
Memory::getOption(Opt option) const
{
    switch (option) {
            
        case Opt::MEM_CHIP_RAM:          return config.chipSize / KB(1);
        case Opt::MEM_SLOW_RAM:          return config.slowSize / KB(1);
        case Opt::MEM_FAST_RAM:          return config.fastSize / KB(1);
        case Opt::MEM_EXT_START:         return config.extStart;
        case Opt::MEM_SAVE_ROMS:         return config.saveRoms;
        case Opt::MEM_SLOW_RAM_DELAY:    return config.slowRamDelay;
        case Opt::MEM_SLOW_RAM_MIRROR:   return config.slowRamMirror;
        case Opt::MEM_BANKMAP:           return (i64)config.bankMap;
        case Opt::MEM_UNMAPPING_TYPE:    return (i64)config.unmappingType;
        case Opt::MEM_RAM_INIT_PATTERN:  return (i64)config.ramInitPattern;

        default:
            fatalError;
    }
}

void
Memory::checkOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::MEM_CHIP_RAM:

            if (!isPoweredOff()) {
                throw AppError(Fault::OPT_LOCKED);
            }
            if (value != 256 && value != 512 && value != 1024 && value != 2048) {
                throw AppError(Fault::OPT_INV_ARG, "256, 512, 1024, 2048");
            }
            return;

        case Opt::MEM_SLOW_RAM:

            if (!isPoweredOff()) {
                throw AppError(Fault::OPT_LOCKED);
            }
            if ((value % 256) != 0 || value > 1536) {
                throw AppError(Fault::OPT_INV_ARG, "0, 256, 512, ..., 1536");
            }
            return;

        case Opt::MEM_FAST_RAM:

            if (!isPoweredOff()) {
                throw AppError(Fault::OPT_LOCKED);
            }
            if ((value % 64) != 0 || value > 8192) {
                throw AppError(Fault::OPT_INV_ARG, "0, 64, 128, ..., 8192");
            }
            return;

        case Opt::MEM_EXT_START:

            if (!isPoweredOff()) {
                throw AppError(Fault::OPT_LOCKED);
            }
            if (value != 0xE0 && value != 0xF0) {
                throw AppError(Fault::OPT_INV_ARG, "E0, F0");
            }
            return;

        case Opt::MEM_SAVE_ROMS:
        case Opt::MEM_SLOW_RAM_DELAY:
        case Opt::MEM_SLOW_RAM_MIRROR:

            return;

        case Opt::MEM_BANKMAP:

            if (!BankMapEnum::isValid(value)) {
                throw AppError(Fault::OPT_INV_ARG, BankMapEnum::keyList());
            }
            return;

        case Opt::MEM_UNMAPPING_TYPE:

            if (!UnmappedMemoryEnum::isValid(value)) {
                throw AppError(Fault::OPT_INV_ARG, UnmappedMemoryEnum::keyList());
            }
            return;

        case Opt::MEM_RAM_INIT_PATTERN:

            if (!RamInitPatternEnum::isValid(value)) {
                throw AppError(Fault::OPT_INV_ARG, RamInitPatternEnum::keyList());
            }
            return;

        default:
            throw(Fault::OPT_UNSUPPORTED);
    }
}


void
Memory::setOption(Opt option, i64 value)
{
    switch (option) {
            
        case Opt::MEM_CHIP_RAM:

            mem.allocChip((i32)KB(value));
            return;
            
        case Opt::MEM_SLOW_RAM:

            mem.allocSlow((i32)KB(value));
            return;
            
        case Opt::MEM_FAST_RAM:

            mem.allocFast((i32)KB(value));
            return;
            
        case Opt::MEM_EXT_START:

            config.extStart = (u32)value;
            updateMemSrcTables();
            return;
            
        case Opt::MEM_SAVE_ROMS:

            config.saveRoms = value;
            return;

        case Opt::MEM_SLOW_RAM_DELAY:


            config.slowRamDelay = value;
            return;

        case Opt::MEM_SLOW_RAM_MIRROR:

            config.slowRamMirror = value;
            return;

        case Opt::MEM_BANKMAP:

            config.bankMap = (BankMap)value;
            updateMemSrcTables();
            return;

        case Opt::MEM_UNMAPPING_TYPE:

            config.unmappingType = (UnmappedMemory)value;
            return;

        case Opt::MEM_RAM_INIT_PATTERN:

            config.ramInitPattern = (RamInitPattern)value;
            if (isPoweredOff()) fillRamWithInitPattern();
            return;

        default:
            fatalError;
    }
}

void
Memory::operator << (SerResetter &worker)
{
    serialize(worker);

    if (isHardResetter(worker)) {

        // Erase WOM (if any)
        if (hasWom()) eraseWom();

        // Fill RAM with the proper startup pattern
        fillRamWithInitPattern();
    }
}

void
Memory::_didReset(bool hard)
{
    // Rebuild the memory lookup table
    updateMemSrcTables();

    // Initialize statistical counters
    clearStats();
}

void
Memory::operator << (SerChecker &worker)
{
    serialize(worker);

    if (config.chipSize) {
        for (isize i = 0; i < config.chipSize; i++) worker << chip[i];
    }
    if (config.slowSize) {
        for (isize i = 0; i < config.slowSize; i++) worker << slow[i];
    }
    if (config.fastSize) {
        for (isize i = 0; i < config.fastSize; i++) worker << fast[i];
    }
    if (config.saveRoms) {

        if (romAllocator.size) {
            for (isize i = 0; i < romAllocator.size; i++) worker << rom[i];
        }
        if (womAllocator.size) {
            for (isize i = 0; i < womAllocator.size; i++) worker << wom[i];
        }
        if (extAllocator.size) {
            for (isize i = 0; i < extAllocator.size; i++) worker << ext[i];
        }
    }
}

void
Memory::operator << (SerCounter &worker)
{
    // Determine memory size information
    i32 romSize = config.saveRoms ? config.romSize : 0;
    i32 womSize = config.saveRoms ? config.womSize : 0;
    i32 extSize = config.saveRoms ? config.extSize : 0;
    i32 chipSize = config.chipSize;
    i32 slowSize = config.slowSize;
    i32 fastSize = config.fastSize;

    serialize(worker);

    worker
    << romSize
    << womSize
    << extSize
    << chipSize
    << slowSize
    << fastSize;

    worker.count += romSize;
    worker.count += womSize;
    worker.count += extSize;
    worker.count += chipSize;
    worker.count += slowSize;
    worker.count += fastSize;
}

void
Memory::operator << (SerReader &worker)
{
    i32 romSize, womSize, extSize, chipSize, slowSize, fastSize;

    serialize(worker);
    
    // Load memory size information
    worker
    << romSize
    << womSize
    << extSize
    << chipSize
    << slowSize
    << fastSize;

    // Check the integrity of the new values before allocating memory
    if (romSize > KB(512)) throw AppError(Fault::SNAP_CORRUPTED);
    if (womSize > KB(256)) throw AppError(Fault::SNAP_CORRUPTED);
    if (extSize > KB(512)) throw AppError(Fault::SNAP_CORRUPTED);
    if (chipSize > MB(2)) throw AppError(Fault::SNAP_CORRUPTED);
    if (slowSize > KB(1792)) throw AppError(Fault::SNAP_CORRUPTED);
    if (fastSize > MB(8)) throw AppError(Fault::SNAP_CORRUPTED);

    // Allocate ROM space
    if (config.saveRoms) {

        allocRom(romSize, false);
        allocWom(womSize, false);
        allocExt(extSize, false);
    }

    // Allocate RAM space
    allocChip(chipSize, false);
    allocSlow(slowSize, false);
    allocFast(fastSize, false);

    // Load memory contents
    worker.copy(rom, romSize);
    worker.copy(wom, womSize);
    worker.copy(ext, extSize);
    worker.copy(chip, chipSize);
    worker.copy(slow, slowSize);
    worker.copy(fast, fastSize);
}

void
Memory::operator << (SerWriter &worker)
{
    serialize(worker);

    // Determine memory size information
    i32 romSize = config.saveRoms ? config.romSize : 0;
    i32 womSize = config.saveRoms ? config.womSize : 0;
    i32 extSize = config.saveRoms ? config.extSize : 0;
    i32 chipSize = config.chipSize;
    i32 slowSize = config.slowSize;
    i32 fastSize = config.fastSize;

    // Save memory size information
    worker
    << romSize
    << womSize
    << extSize
    << chipSize
    << slowSize
    << fastSize;

    // Save memory contents
    worker.copy(rom, romSize);
    worker.copy(wom, womSize);
    worker.copy(ext, extSize);
    worker.copy(chip, chipSize);
    worker.copy(slow, slowSize);
    worker.copy(fast, fastSize);
}

void
Memory::cacheInfo(MemInfo &result) const
{
    {   SYNCHRONIZED
        
        result.hasRom = hasRom();
        result.hasWom = hasWom();
        result.hasExt = hasExt();
        result.hasBootRom = hasBootRom();
        result.hasKickRom = hasKickRom();
        result.womLock = womIsLocked;
        
        result.romMask = romMask;
        result.womMask = womMask;
        result.extMask = extMask;
        result.chipMask = chipMask;
        
        for (isize i = 0; i < 256; i++) result.cpuMemSrc[i] = cpuMemSrc[i];
        for (isize i = 0; i < 256; i++) result.agnusMemSrc[i] = agnusMemSrc[i];
    }
}

void
Memory::_isReady() const
{    
    auto traits = getRomTraits();

    bool hasRom = traits.crc != 0;
    bool hasAros = traits.vendor == RomVendor::AROS;

    if (!hasRom || FORCE_ROM_MISSING) {
        throw AppError(Fault::ROM_MISSING);
    }
    if (!chip || FORCE_CHIP_RAM_MISSING) {
        throw AppError(Fault::CHIP_RAM_MISSING);
    }
    if ((hasAros && !ext) || FORCE_AROS_NO_EXTROM) {
        throw AppError(Fault::AROS_NO_EXTROM);
    }
    if ((hasAros && ramSize() < MB(1)) || FORCE_AROS_RAM_LIMIT) {
        throw AppError(Fault::AROS_RAM_LIMIT);
    }
}

void
Memory::updateStats()
{
    const double w = 0.5;
    
    stats.chipReads.accumulated =
    w * stats.chipReads.accumulated + (1.0 - w) * stats.chipReads.raw;
    stats.chipWrites.accumulated =
    w * stats.chipWrites.accumulated + (1.0 - w) * stats.chipWrites.raw;
    stats.slowReads.accumulated =
    w * stats.slowReads.accumulated + (1.0 - w) * stats.slowReads.raw;
    stats.slowWrites.accumulated =
    w * stats.slowWrites.accumulated + (1.0 - w) * stats.slowWrites.raw;
    stats.fastReads.accumulated =
    w * stats.fastReads.accumulated + (1.0 - w) * stats.fastReads.raw;
    stats.fastWrites.accumulated =
    w * stats.fastWrites.accumulated + (1.0 - w) * stats.fastWrites.raw;
    stats.kickReads.accumulated =
    w * stats.kickReads.accumulated + (1.0 - w) * stats.kickReads.raw;
    stats.kickWrites.accumulated =
    w * stats.kickWrites.accumulated + (1.0 - w) * stats.kickWrites.raw;

    stats.chipReads.raw = 0;
    stats.chipWrites.raw = 0;
    stats.slowReads.raw = 0;
    stats.slowWrites.raw = 0;
    stats.fastReads.raw = 0;
    stats.fastWrites.raw = 0;
    stats.kickReads.raw = 0;
    stats.kickWrites.raw = 0;
}

void
Memory::allocChip(i32 bytes, bool update)
{
    config.chipSize = bytes;
    alloc(chipAllocator, bytes, chipMask, update);
}

void
Memory::allocSlow(i32 bytes, bool update)
{
    config.slowSize = bytes;
    alloc(slowAllocator, bytes, update);
}

void
Memory::allocFast(i32 bytes, bool update)
{
    config.fastSize = bytes;
    alloc(fastAllocator, bytes, update);
}

void
Memory::allocRom(i32 bytes, bool update)
{
    config.romSize = bytes;
    alloc(romAllocator, bytes, romMask, update);
}

void
Memory::allocWom(i32 bytes, bool update)
{
    config.womSize = bytes;
    alloc(womAllocator, bytes, womMask, update);
}

void
Memory::allocExt(i32 bytes, bool update)
{
    config.extSize = bytes;
    alloc(extAllocator, bytes, extMask, update);
}

void
Memory::alloc(Allocator<u8> &allocator, isize bytes, bool update)
{
    // Only proceed if memory layout will change
    if (bytes == allocator.size) return;

    // Allocate memory
    allocator.alloc(bytes);

    // Update the memory source tables if requested
    if (update) updateMemSrcTables();
}

void
Memory::alloc(Allocator<u8> &allocator, isize bytes, u32 &mask, bool update)
{
    // Set the memory mask
    mask = bytes ? u32(bytes - 1) : 0;

    // Allocate
    alloc(allocator, bytes, update);
}

void
Memory::fillRamWithInitPattern()
{
    switch (config.ramInitPattern) {
                        
        case RamInitPattern::ALL_ZEROES:

            if (chip) std::memset(chip, 0x00, config.chipSize);
            if (slow) std::memset(slow, 0x00, config.slowSize);
            if (fast) std::memset(fast, 0x00, config.fastSize);
            break;
            
        case RamInitPattern::ALL_ONES:
            
            if (chip) std::memset(chip, 0xFF, config.chipSize);
            if (slow) std::memset(slow, 0xFF, config.slowSize);
            if (fast) std::memset(fast, 0xFF, config.fastSize);
            break;
            
        case RamInitPattern::RANDOMIZED:

            srand(0);
            if (chip) for (isize i = 0; i < config.chipSize; i++) chip[i] = (u8)rand();
            if (slow) for (isize i = 0; i < config.slowSize; i++) slow[i] = (u8)rand();
            if (fast) for (isize i = 0; i < config.fastSize; i++) fast[i] = (u8)rand();
            break;

        default:
            break;
    }
}

const RomTraits &
Memory::getRomTraits(u32 crc)
{
    static RomTraits fallback;

    // Crawl through the Rom database
    for (auto &traits : roms) if (traits.crc == crc) return traits;

    fallback = RomTraits {

        .crc = crc,
        .title = crc ? "Unknown ROM" : "",
        .revision = "",
        .released = "",
        .model = "",
        .vendor = RomVendor::OTHER
    };

    return fallback;
}

const RomTraits &
Memory::getRomTraits() const
{
    return getRomTraits(util::crc32(rom, config.romSize));
}

const RomTraits &
Memory::getWomTraits() const
{
    return getRomTraits(util::crc32(wom, config.womSize));
}

const RomTraits &
Memory::getExtTraits() const
{
    return getRomTraits(util::crc32(ext, config.extSize));
}

u32
Memory::romFingerprint() const
{
    return util::crc32(rom, config.romSize);
}

u32
Memory::extFingerprint() const
{
    return util::crc32(ext, config.extSize);
}

void
Memory::loadRom(MediaFile &file)
{
    try {

        auto &romFile = dynamic_cast<RomFile &>(file);

        // Decrypt Rom
        if (romFile.isEncrypted()) romFile.decrypt();

        // Allocate memory
        allocRom((i32)romFile.data.size);

        // Load Rom
        romFile.flash(rom);

        // Add a Wom if a Boot Rom is installed instead of a Kickstart Rom
        hasBootRom() ? (void)allocWom(KB(256)) : deleteWom();

        // Remove extended Rom (if any)
        deleteExt();

    } catch (...) {

        throw AppError(Fault::FILE_TYPE_MISMATCH);
    }
}

void
Memory::loadRom(const fs::path &path)
{
    RomFile file(path);
    loadRom(file);
}

void
Memory::loadRom(const u8 *buf, isize len)
{
    RomFile file(buf, len);
    loadRom(file);
}

void
Memory::loadExt(MediaFile &file)
{
    try {

        RomFile &extFile = dynamic_cast<RomFile &>(file);

        // Allocate memory
        allocExt((i32)extFile.data.size);

        // Load Rom
        file.flash(ext);

    } catch (...) {

        throw AppError(Fault::FILE_TYPE_MISMATCH);
    }
}

void
Memory::loadExt(const fs::path &path)
{
    RomFile file(path);
    loadExt(file);
}

void
Memory::loadExt(const u8 *buf, isize len)
{
    RomFile file(buf, len);
    loadExt(file);
}

void
Memory::saveRom(const fs::path &path) const
{
    if (rom == nullptr) throw AppError(Fault::ROM_MISSING);

    RomFile file(rom, config.romSize);
    file.writeToFile(path);
}

void
Memory::saveWom(const fs::path &path) const
{
    if (wom == nullptr) throw AppError(Fault::ROM_MISSING);

    RomFile file(wom, config.womSize);
    file.writeToFile(path);
}

void
Memory::saveExt(const fs::path &path) const
{
    if (ext == nullptr) throw AppError(Fault::ROM_MISSING);

    RomFile file(ext, config.extSize);
    file.writeToFile(path);
}

void
Memory::patchExpansionLib()
{
    /* In Kickstart 1.2, function 'ConfigBoard' is broken. A patch needs to
     * be applied to make the debugger board compatible with this Rom.
     */
    switch (romFingerprint()) {

        case CRC32_KICK12_33_166:
        case CRC32_KICK12_33_180:
        case CRC32_KICK121_34_004:
        case CRC32_KICK12_33_180_G11R:
        {
            for (isize i = 0; i < KB(512) - 22; i += 2) {
                
                if (R16BE(rom + i)      == 0x2c6e &&
                    R16BE(rom + i + 2)  == 0x0024 &&
                    R16BE(rom + i + 4)  == 0x4eae &&
                    R16BE(rom + i + 6)  == 0xff3a &&
                    R16BE(rom + i + 20) == 0x202f &&
                    R16BE(rom + i + 22) == 0x0002) {
                    
                    xfiles("Patching Kickstart 1.2 at %lx\n", i);

                    W32BE(rom + i, 0x426f0004);
                    W16BE(rom + i + 22, 0x0000);
                    return;
                }
            }
            warn("patchExpansionLib: Can't find patch location\n");
            break;
        }

        default:
            break;
    }
}

bool
Memory::isRelocated()
{
    if (rom) {
        auto addr = HI_HI_LO_LO(rom[4], rom[5], rom[6], rom[7]);
        return (addr & 0x00F00000) != 0x00F00000;
    } else {
        return false;
    }
}

template <> MemSrc
Memory::getMemSrc <Accessor::CPU> (u32 addr)
{
    return cpuMemSrc[(addr >> 16) & 0xFF];
}

template <> MemSrc
Memory::getMemSrc <Accessor::AGNUS> (u32 addr)
{
    return agnusMemSrc[(addr >> 16) & 0xFF];
}

void
Memory::updateMemSrcTables()
{    
    updateCpuMemSrcTable();
    updateAgnusMemSrcTable();
}

void
Memory::updateCpuMemSrcTable()
{
    MemSrc mem_rom = rom ? MemSrc::ROM : MemSrc::NONE;
    MemSrc mem_wom = wom ? MemSrc::WOM : mem_rom;
    MemSrc mem_rom_mirror = rom ? MemSrc::ROM_MIRROR : MemSrc::NONE;

    assert(config.chipSize % 0x10000 == 0);
    assert(config.slowSize % 0x10000 == 0);
    assert(config.fastSize % 0x10000 == 0);

    isize chipRamPages = config.chipSize / 0x10000;
    isize slowRamPages = config.slowSize / 0x10000;
    
    bool ovl = ciaa.getPA() & 1;
    bool old = config.bankMap == BankMap::A1000 || config.bankMap == BankMap::A2000A;

    // Start from scratch
    for (isize i = 0x00; i <= 0xFF; i++) {
        cpuMemSrc[i] = MemSrc::NONE;
    }
    
    // Chip Ram
    if (chipRamPages) {
        for (isize i = 0x00; i < chipRamPages; i++) {
            cpuMemSrc[i] = MemSrc::CHIP;
        }
        for (isize i = chipRamPages; i <= 0x1F; i++) {
            cpuMemSrc[i] = MemSrc::CHIP_MIRROR;
        }
    }

    // CIAs
    for (isize i = 0xA0; i <= 0xBE; i++) {
        cpuMemSrc[i] = MemSrc::CIA_MIRROR;
    }
    cpuMemSrc[0xBF] = MemSrc::CIA;
    
    // Slow Ram
    for (isize i = 0xC0; i <= 0xD7; i++) {
        cpuMemSrc[i] = (i - 0xC0) < slowRamPages ? MemSrc::SLOW : MemSrc::CUSTOM_MIRROR;
    }
    
    // Real-time clock (older Amigas)
    for (isize i = 0xD8; i <= 0xDB; i++) {
        cpuMemSrc[i] = old ? MemSrc::RTC : MemSrc::CUSTOM;
    }

    // Real-time clock (newer Amigas)
    cpuMemSrc[0xDC] = old ? MemSrc::CUSTOM : MemSrc::RTC;
    
    
    // Reserved
    cpuMemSrc[0xDD] = MemSrc::NONE;

    // Custom chip set
    for (isize i = 0xDE; i <= 0xDF; i++) {
        cpuMemSrc[i] = MemSrc::CUSTOM;
    }
    
    // Kickstart mirror, unmapped, or Extended Rom
    if (config.bankMap != BankMap::A1000) {
        for (isize i = 0xE0; i <= 0xE7; i++) {
            cpuMemSrc[i] = mem_rom_mirror;
        }
    }
    if (ext && config.extStart == 0xE0) {
        for (isize i = 0xE0; i <= 0xE7; i++) {
            cpuMemSrc[i] = MemSrc::EXT;
        }
    }

    // Auto-config (Zorro II)
    cpuMemSrc[0xE8] = MemSrc::AUTOCONF;
    for (isize i = 0xE9; i <= 0xEF; i++) {
        assert(cpuMemSrc[i] == MemSrc::NONE);
    }
    
    // Unmapped or Extended Rom
    if (ext && config.extStart == 0xF0) {
        for (isize i = 0xF0; i <= 0xF7; i++) {
            cpuMemSrc[i] = MemSrc::EXT;
        }
    }

    // Kickstart Wom or Kickstart Rom
    for (isize i = 0xF8; i <= 0xFF; i++) {
        cpuMemSrc[i] = mem_wom;
    }
    
    // Blend in Boot Rom if a writeable Wom is present
    if (hasWom() && !womIsLocked) {
        for (isize i = 0xF8; i <= 0xFB; i++)
            cpuMemSrc[i] = mem_rom;
    }

    // Blend in Rom in lower memory area if the overlay line (OVL) is high
    if (ovl) {
        for (isize i = 0; i < 8 && cpuMemSrc[0xF8 + i] != MemSrc::NONE; i++)
            cpuMemSrc[i] = cpuMemSrc[0xF8 + i];
    }

    // Expansion boards
    zorro.updateMemSrcTables();

    msgQueue.put(Msg::MEM_LAYOUT);
}

void
Memory::updateAgnusMemSrcTable()
{
    isize banks = config.chipSize / 0x10000;
    
    // Start from scratch
    for (isize i = 0x00; i <= 0xFF; i++) {
        agnusMemSrc[i] = MemSrc::NONE;
    }
    
    // Chip Ram banks
    for (isize i = 0x0; i < banks; i++) {
        agnusMemSrc[i] = MemSrc::CHIP;
    }
    
    // Slow Ram mirror
    if (slowRamIsMirroredIn()) {
        for (isize i = 0x8; i <= 0xF; i++) {
            agnusMemSrc[i] = MemSrc::SLOW_MIRROR;
        }
    }
}

bool
Memory::slowRamIsMirroredIn() const
{

    /* The ECS revision of Agnus has a special feature that makes Slow Ram
     * accessible for DMA. In the 512 MB Chip Ram + 512 Slow Ram configuration,
     * Slow Ram is mapped into the second Chip Ram segment. OCS Agnus does not
     * have this feature. It is able to access Chip Ram, only.
     */

    if (config.slowRamMirror && agnus.isECS()) {
        return chipRamSize() == KB(512) && slowRamSize() == KB(512);
    } else {
        return false;
    }
}

bool
Memory::inChipRam(u32 addr)
{
    if (addr > 0xFFFFFF) return false;

    auto memSrc = cpuMemSrc[addr >> 16];
    return memSrc == MemSrc::CHIP || memSrc == MemSrc::CHIP_MIRROR;
}

bool
Memory::inSlowRam(u32 addr)
{
    if (addr > 0xFFFFFF) return false;

    auto memSrc = cpuMemSrc[addr >> 16];
    return memSrc == MemSrc::SLOW;
}

bool
Memory::inFastRam(u32 addr)
{
    if (addr > 0xFFFFFF) return false;

    auto memSrc = cpuMemSrc[addr >> 16];
    return memSrc == MemSrc::FAST;
}

bool
Memory::inRam(u32 addr)
{
    return inChipRam(addr) || inSlowRam(addr) || inFastRam(addr);
}

bool
Memory::inRom(u32 addr)
{
    if (addr > 0xFFFFFF) return false;

    auto memSrc = cpuMemSrc[addr >> 16];
    
    return
    memSrc == MemSrc::ROM ||
    memSrc == MemSrc::ROM_MIRROR ||
    memSrc == MemSrc::WOM ||
    memSrc == MemSrc::EXT;
}

bool 
Memory::isUnmapped(u32 addr)
{
    if (addr > 0xFFFFFF) return true;

    return cpuMemSrc[addr >> 16] == MemSrc::NONE;
}


//
// Peek (CPU)
//

template<> u16
Memory::spypeek16 <Accessor::CPU, MemSrc::NONE> (u32 addr) const
{
    switch (config.unmappingType) {
            
        case UnmappedMemory::FLOATING:   return dataBus;
        case UnmappedMemory::ALL_ONES:   return 0xFFFF;
        case UnmappedMemory::ALL_ZEROES: return 0x0000;

        default:
            fatalError;
    }
}

template <Accessor acc, MemSrc src> u8
Memory::spypeek8(u32 addr) const
{
    auto word = spypeek16 <acc, src> (addr & ~1);
    return IS_EVEN(addr) ? HI_BYTE(word) : LO_BYTE(word);
}

template <Accessor acc, MemSrc src> u32
Memory::spypeek32(u32 addr) const
{
    auto hi = spypeek16 <acc, src> (addr);
    auto lo = spypeek16 <acc, src> (addr + 2);
    
    return HI_W_LO_W(hi, lo);
}

template<> u8
Memory::peek8 <Accessor::CPU, MemSrc::NONE> (u32 addr)
{
    return (u8)(spypeek16 <Accessor::CPU, MemSrc::NONE> (addr));
}

template<> u16
Memory::peek16 <Accessor::CPU, MemSrc::NONE> (u32 addr)
{
    return spypeek16 <Accessor::CPU, MemSrc::NONE> (addr);
}

template<> u8
Memory::peek8 <Accessor::CPU, MemSrc::CHIP> (u32 addr)
{
    ASSERT_CHIP_ADDR(addr);
    agnus.executeUntilBusIsFree();

    dataBus = READ_CHIP_8(addr);

    stats.chipReads.raw++;
    agnus.busAddr[agnus.pos.h] = addr;
    agnus.busData[agnus.pos.h] = dataBus;

    return (u8)dataBus;
}

template<> u16
Memory::peek16 <Accessor::CPU, MemSrc::CHIP> (u32 addr)
{
    ASSERT_CHIP_ADDR(addr);
    agnus.executeUntilBusIsFree();

    dataBus = READ_CHIP_16(addr);

    stats.chipReads.raw++;
    agnus.busAddr[agnus.pos.h] = addr;
    agnus.busData[agnus.pos.h] = dataBus;
    
    return dataBus;
}

template<> u16
Memory::spypeek16 <Accessor::CPU, MemSrc::CHIP> (u32 addr) const
{
    return READ_CHIP_16(addr);
}

template<> u8
Memory::peek8 <Accessor::CPU, MemSrc::SLOW> (u32 addr)
{
    ASSERT_SLOW_ADDR(addr);
    agnus.executeUntilBusIsFree();

    dataBus = READ_SLOW_8(addr);

    stats.slowReads.raw++;
    agnus.busAddr[agnus.pos.h] = addr;
    agnus.busData[agnus.pos.h] = dataBus;
    
    return (u8)dataBus;
}

template<> u16
Memory::peek16 <Accessor::CPU, MemSrc::SLOW> (u32 addr)
{
    ASSERT_SLOW_ADDR(addr);
    agnus.executeUntilBusIsFree();
    
    dataBus = READ_SLOW_16(addr);

    stats.slowReads.raw++;
    agnus.busAddr[agnus.pos.h] = addr;
    agnus.busData[agnus.pos.h] = dataBus;
    
    return dataBus;
}

template<> u16
Memory::spypeek16 <Accessor::CPU, MemSrc::SLOW> (u32 addr) const
{
    return READ_SLOW_16(addr);
}

template<> u8
Memory::peek8 <Accessor::CPU, MemSrc::FAST> (u32 addr)
{
    ASSERT_FAST_ADDR(addr);
    
    stats.fastReads.raw++;
    return READ_FAST_8(addr);
}

template<> u16
Memory::peek16 <Accessor::CPU, MemSrc::FAST> (u32 addr)
{
    if (!((addr - FAST_RAM_STRT) < (u32)config.fastSize)) {
        printf("addr = %x (start: %x size: %x)\n", addr, FAST_RAM_STRT, (u32)config.fastSize);
    }
    
    ASSERT_FAST_ADDR(addr);
    
    stats.fastReads.raw++;
    return READ_FAST_16(addr);
}

template<> u16
Memory::spypeek16 <Accessor::CPU, MemSrc::FAST> (u32 addr) const
{
    return READ_FAST_16(addr);
}

template<> u8
Memory::peek8 <Accessor::CPU, MemSrc::CIA> (u32 addr)
{
    ASSERT_CIA_ADDR(addr);
    
    agnus.executeUntilBusIsFreeForCIA();

    dataBus = peekCIA8(addr);
    return (u8)dataBus;
}

template<> u16
Memory::peek16 <Accessor::CPU, MemSrc::CIA> (u32 addr)
{
    ASSERT_CIA_ADDR(addr);
    xfiles("CIA: Reading a WORD from %x\n", addr);

    agnus.executeUntilBusIsFreeForCIA();
    
    dataBus = peekCIA16(addr);
    return dataBus;
}

template<> u16
Memory::spypeek16 <Accessor::CPU, MemSrc::CIA> (u32 addr) const
{
    return spypeekCIA16(addr);
}

template<> u8
Memory::peek8 <Accessor::CPU, MemSrc::RTC> (u32 addr)
{
    ASSERT_RTC_ADDR(addr);
    
    // agnus.executeUntilBusIsFree();
    
    dataBus = peekRTC8(addr);
    return (u8)dataBus;
}

template<> u16
Memory::peek16 <Accessor::CPU, MemSrc::RTC> (u32 addr)
{
    ASSERT_RTC_ADDR(addr);
    
    // agnus.executeUntilBusIsFree();

    dataBus = peekRTC16(addr);
    return dataBus;
}

template<> u16
Memory::spypeek16 <Accessor::CPU, MemSrc::RTC> (u32 addr) const
{
    ASSERT_RTC_ADDR(addr);
    return peekRTC16(addr);
}

template<> u8
Memory::peek8 <Accessor::CPU, MemSrc::CUSTOM> (u32 addr)
{
    ASSERT_CUSTOM_ADDR(addr);

    agnus.executeUntilBusIsFree();

    if (IS_EVEN(addr)) {
        dataBus = HI_BYTE(peekCustom16(addr));
    } else {
        dataBus = LO_BYTE(peekCustom16(addr & 0x1FE));
    }
    return (u8)dataBus;
}

template<> u16
Memory::peek16 <Accessor::CPU, MemSrc::CUSTOM> (u32 addr)
{
    ASSERT_CUSTOM_ADDR(addr);
    
    agnus.executeUntilBusIsFree();
    
    dataBus = peekCustom16(addr);

    agnus.busAddr[agnus.pos.h] = addr;
    agnus.busData[agnus.pos.h] = dataBus;
    
    return dataBus;
}

template<> u16
Memory::spypeek16 <Accessor::CPU, MemSrc::CUSTOM> (u32 addr) const
{
    return spypeekCustom16(addr);
}

template<> u8
Memory::peek8 <Accessor::CPU, MemSrc::AUTOCONF> (u32 addr)
{
    ASSERT_AUTO_ADDR(addr);
    
    // Experimental code to match UAE output (for debugging)
    if (MIMIC_UAE) {
        if (fastRamSize() == 0) {
            dataBus = (addr & 0b10) ? 0xE8 : 0x02;
            return (u8)dataBus;
        }
    }

    dataBus = (u16)(zorro.peekACF(addr));
    return (u8)dataBus;
}

template<> u16
Memory::peek16 <Accessor::CPU, MemSrc::AUTOCONF> (u32 addr)
{
    ASSERT_AUTO_ADDR(addr);

    auto hi = zorro.peekACF(addr);
    auto lo = zorro.peekACF(addr + 1);
    
    dataBus = HI_LO(hi,lo);
    return dataBus;
}

template<> u16
Memory::spypeek16 <Accessor::CPU, MemSrc::AUTOCONF> (u32 addr) const
{
    auto hi = zorro.spypeekACF(addr);
    auto lo = zorro.spypeekACF(addr + 1);
    
    return HI_LO(hi,lo);
}

template<> u8
Memory::peek8 <Accessor::CPU, MemSrc::ZOR> (u32 addr)
{
    dataBus = u16(zorro.peek8(addr));
    return u8(dataBus);
}

template<> u16
Memory::peek16 <Accessor::CPU, MemSrc::ZOR> (u32 addr)
{
    dataBus = u16(zorro.peek16(addr));
    return dataBus;
}

template<> u16
Memory::spypeek16 <Accessor::CPU, MemSrc::ZOR> (u32 addr) const
{
    return zorro.spypeek16(addr);
}

template<> u8
Memory::peek8 <Accessor::CPU, MemSrc::ROM> (u32 addr)
{
    ASSERT_ROM_ADDR(addr);
    
    stats.kickReads.raw++;
    return READ_ROM_8(addr);
}

template<> u16
Memory::peek16 <Accessor::CPU, MemSrc::ROM> (u32 addr)
{
    ASSERT_ROM_ADDR(addr);
    
    stats.kickReads.raw++;
    return READ_ROM_16(addr);
}

template<> u16
Memory::spypeek16 <Accessor::CPU, MemSrc::ROM> (u32 addr) const
{
    return READ_ROM_16(addr);
}

template<> u8
Memory::peek8 <Accessor::CPU, MemSrc::WOM> (u32 addr)
{
    ASSERT_WOM_ADDR(addr);
    
    stats.kickReads.raw++;
    return READ_WOM_8(addr);
}

template<> u16
Memory::peek16 <Accessor::CPU, MemSrc::WOM> (u32 addr)
{
    ASSERT_WOM_ADDR(addr);
    
    stats.kickReads.raw++;
    return READ_WOM_16(addr);
}

template<> u16
Memory::spypeek16 <Accessor::CPU, MemSrc::WOM> (u32 addr) const
{
    return READ_WOM_16(addr);
}

template<> u8
Memory::peek8 <Accessor::CPU, MemSrc::EXT> (u32 addr)
{
    ASSERT_EXT_ADDR(addr);
    
    stats.kickReads.raw++;
    return READ_EXT_8(addr);
}

template<> u16
Memory::peek16 <Accessor::CPU, MemSrc::EXT> (u32 addr)
{
    ASSERT_EXT_ADDR(addr);
    
    stats.kickReads.raw++;
    return READ_EXT_16(addr);
}

template<> u16
Memory::spypeek16 <Accessor::CPU, MemSrc::EXT> (u32 addr) const
{
    return READ_EXT_16(addr);
}

template<> u8
Memory::peek8 <Accessor::CPU> (u32 addr)
{
    addr &= 0xFFFFFF;
    
    switch (cpuMemSrc[addr >> 16]) {
            
        case MemSrc::NONE:          return peek8 <Accessor::CPU, MemSrc::NONE>     (addr);
        case MemSrc::CHIP:          return peek8 <Accessor::CPU, MemSrc::CHIP>     (addr);
        case MemSrc::CHIP_MIRROR:   return peek8 <Accessor::CPU, MemSrc::CHIP>     (addr);
        case MemSrc::SLOW:          return peek8 <Accessor::CPU, MemSrc::SLOW>     (addr);
        case MemSrc::FAST:          return peek8 <Accessor::CPU, MemSrc::FAST>     (addr);
        case MemSrc::CIA:           return peek8 <Accessor::CPU, MemSrc::CIA>      (addr);
        case MemSrc::CIA_MIRROR:    return peek8 <Accessor::CPU, MemSrc::CIA>      (addr);
        case MemSrc::RTC:           return peek8 <Accessor::CPU, MemSrc::RTC>      (addr);
        case MemSrc::CUSTOM:        return peek8 <Accessor::CPU, MemSrc::CUSTOM>   (addr);
        case MemSrc::CUSTOM_MIRROR: return peek8 <Accessor::CPU, MemSrc::CUSTOM>   (addr);
        case MemSrc::AUTOCONF:      return peek8 <Accessor::CPU, MemSrc::AUTOCONF> (addr);
        case MemSrc::ZOR:           return peek8 <Accessor::CPU, MemSrc::ZOR>      (addr);
        case MemSrc::ROM:           return peek8 <Accessor::CPU, MemSrc::ROM>      (addr);
        case MemSrc::ROM_MIRROR:    return peek8 <Accessor::CPU, MemSrc::ROM>      (addr);
        case MemSrc::WOM:           return peek8 <Accessor::CPU, MemSrc::WOM>      (addr);
        case MemSrc::EXT:           return peek8 <Accessor::CPU, MemSrc::EXT>      (addr);

        default:
            fatalError;
    }
}

template<> u16
Memory::peek16 <Accessor::CPU> (u32 addr)
{
    addr &= 0xFFFFFF;

    switch (cpuMemSrc[addr >> 16]) {
            
        case MemSrc::NONE:          return peek16 <Accessor::CPU, MemSrc::NONE>     (addr);
        case MemSrc::CHIP:          return peek16 <Accessor::CPU, MemSrc::CHIP>     (addr);
        case MemSrc::CHIP_MIRROR:   return peek16 <Accessor::CPU, MemSrc::CHIP>     (addr);
        case MemSrc::SLOW:          return peek16 <Accessor::CPU, MemSrc::SLOW>     (addr);
        case MemSrc::FAST:          return peek16 <Accessor::CPU, MemSrc::FAST>     (addr);
        case MemSrc::CIA:           return peek16 <Accessor::CPU, MemSrc::CIA>      (addr);
        case MemSrc::CIA_MIRROR:    return peek16 <Accessor::CPU, MemSrc::CIA>      (addr);
        case MemSrc::RTC:           return peek16 <Accessor::CPU, MemSrc::RTC>      (addr);
        case MemSrc::CUSTOM:        return peek16 <Accessor::CPU, MemSrc::CUSTOM>   (addr);
        case MemSrc::CUSTOM_MIRROR: return peek16 <Accessor::CPU, MemSrc::CUSTOM>   (addr);
        case MemSrc::AUTOCONF:      return peek16 <Accessor::CPU, MemSrc::AUTOCONF> (addr);
        case MemSrc::ZOR:           return peek16 <Accessor::CPU, MemSrc::ZOR>      (addr);
        case MemSrc::ROM:           return peek16 <Accessor::CPU, MemSrc::ROM>      (addr);
        case MemSrc::ROM_MIRROR:    return peek16 <Accessor::CPU, MemSrc::ROM>      (addr);
        case MemSrc::WOM:           return peek16 <Accessor::CPU, MemSrc::WOM>      (addr);
        case MemSrc::EXT:           return peek16 <Accessor::CPU, MemSrc::EXT>      (addr);

        default:
            fatalError;
    }
}

template<> u16
Memory::spypeek16 <Accessor::CPU> (u32 addr) const
{
    addr &= 0xFFFFFF;

    switch (cpuMemSrc[addr >> 16]) {
            
        case MemSrc::NONE:          return spypeek16 <Accessor::CPU, MemSrc::NONE>     (addr);
        case MemSrc::CHIP:          return spypeek16 <Accessor::CPU, MemSrc::CHIP>     (addr);
        case MemSrc::CHIP_MIRROR:   return spypeek16 <Accessor::CPU, MemSrc::CHIP>     (addr);
        case MemSrc::SLOW:          return spypeek16 <Accessor::CPU, MemSrc::SLOW>     (addr);
        case MemSrc::FAST:          return spypeek16 <Accessor::CPU, MemSrc::FAST>     (addr);
        case MemSrc::CIA:           return spypeek16 <Accessor::CPU, MemSrc::CIA>      (addr);
        case MemSrc::CIA_MIRROR:    return spypeek16 <Accessor::CPU, MemSrc::CIA>      (addr);
        case MemSrc::RTC:           return spypeek16 <Accessor::CPU, MemSrc::RTC>      (addr);
        case MemSrc::CUSTOM:        return spypeek16 <Accessor::CPU, MemSrc::CUSTOM>   (addr);
        case MemSrc::CUSTOM_MIRROR: return spypeek16 <Accessor::CPU, MemSrc::CUSTOM>   (addr);
        case MemSrc::AUTOCONF:      return spypeek16 <Accessor::CPU, MemSrc::AUTOCONF> (addr);
        case MemSrc::ZOR:           return spypeek16 <Accessor::CPU, MemSrc::ZOR>      (addr);
        case MemSrc::ROM:           return spypeek16 <Accessor::CPU, MemSrc::ROM>      (addr);
        case MemSrc::ROM_MIRROR:    return spypeek16 <Accessor::CPU, MemSrc::ROM>      (addr);
        case MemSrc::WOM:           return spypeek16 <Accessor::CPU, MemSrc::WOM>      (addr);
        case MemSrc::EXT:           return spypeek16 <Accessor::CPU, MemSrc::EXT>      (addr);

        default:
            fatalError;
    }
}

template<> u8
Memory::spypeek8 <Accessor::CPU> (u32 addr) const
{
    auto word = spypeek16 <Accessor::CPU> (addr & ~1);
    return IS_EVEN(addr) ? HI_BYTE(word) : LO_BYTE(word);
}

template<> u32
Memory::spypeek32 <Accessor::CPU> (u32 addr) const
{
    auto hi = spypeek16 <Accessor::CPU> (addr);
    auto lo = spypeek16 <Accessor::CPU> (addr + 2);
    
    return HI_W_LO_W(hi, lo);
}

template <> void
Memory::spypeek <Accessor::CPU> (u32 addr, isize len, u8 *buf) const
{
    assert(buf);
    
    for (isize i = 0; i < len; i++) {
        buf[i] = spypeek8 <Accessor::CPU> (u32(addr + i));
    }
}


//
// Peek (Agnus)
//

template<> u16
Memory::peek16 <Accessor::AGNUS, MemSrc::NONE> (u32 addr)
{
    assert((addr & agnus.ptrMask) == addr);
    xfiles("Agnus reads from unmapped RAM\n");
    return peek16 <Accessor::CPU, MemSrc::NONE> (addr);
}

template<> u16
Memory::spypeek16 <Accessor::AGNUS, MemSrc::NONE> (u32 addr) const
{
    return spypeek16 <Accessor::CPU, MemSrc::NONE> (addr);
}

template<> u16
Memory::peek16 <Accessor::AGNUS, MemSrc::CHIP> (u32 addr)
{
    assert((addr & agnus.ptrMask) == addr);
    dataBus = READ_CHIP_16(addr);
    return dataBus;
}

template<> u16
Memory::spypeek16 <Accessor::AGNUS, MemSrc::CHIP> (u32 addr) const
{
    assert((addr & agnus.ptrMask) == addr);
    return READ_CHIP_16(addr);
}

template<> u16
Memory::peek16 <Accessor::AGNUS, MemSrc::SLOW> (u32 addr)
{
    xfiles("Agnus reads from Slow RAM mirror at %x\n", addr);

    dataBus = READ_SLOW_16(SLOW_RAM_STRT + (addr & 0x7FFFF));
    return dataBus;
}

template<> u16
Memory::spypeek16 <Accessor::AGNUS, MemSrc::SLOW> (u32 addr) const
{
    return READ_SLOW_16(SLOW_RAM_STRT + (addr & 0x7FFFF));
}

template<> u16
Memory::peek16 <Accessor::AGNUS> (u32 addr)
{
    addr &= agnus.ptrMask;

    switch (agnusMemSrc[addr >> 16]) {
            
        case MemSrc::NONE:        return peek16 <Accessor::AGNUS, MemSrc::NONE> (addr);
        case MemSrc::CHIP:        return peek16 <Accessor::AGNUS, MemSrc::CHIP> (addr);
        case MemSrc::SLOW_MIRROR: return peek16 <Accessor::AGNUS, MemSrc::SLOW> (addr);
            
        default:
            fatalError;
    }
}

template<> u16
Memory::spypeek16 <Accessor::AGNUS> (u32 addr) const
{
    addr &= agnus.ptrMask;
    
    switch (agnusMemSrc[addr >> 16]) {
            
        case MemSrc::NONE:        return spypeek16 <Accessor::AGNUS, MemSrc::NONE> (addr);
        case MemSrc::CHIP:        return spypeek16 <Accessor::AGNUS, MemSrc::CHIP> (addr);
        case MemSrc::SLOW_MIRROR: return spypeek16 <Accessor::AGNUS, MemSrc::SLOW> (addr);
            
        default:
            fatalError;
    }
}

template<> u8
Memory::spypeek8 <Accessor::AGNUS> (u32 addr) const
{
    auto word = spypeek16 <Accessor::AGNUS> (addr & ~1);
    return IS_EVEN(addr) ? HI_BYTE(word) : LO_BYTE(word);
}


//
// Poke (CPU)
//

template <> void
Memory::poke8 <Accessor::CPU, MemSrc::NONE> (u32 addr, u8 value)
{
    trace(MEM_DEBUG, "poke8(%x [NONE], %x)\n", addr, value);
    dataBus = value;
}

template <> void
Memory::poke16 <Accessor::CPU, MemSrc::NONE> (u32 addr, u16 value)
{
    trace(MEM_DEBUG, "poke16 <CPU> (%x [NONE], %x)\n", addr, value);
    dataBus = value;
}

template <> void
Memory::poke8 <Accessor::CPU, MemSrc::CHIP> (u32 addr, u8 value)
{
    ASSERT_CHIP_ADDR(addr);
    
    if (BLT_MEM_GUARD) {
        if (blitter.checkMemguard(addr & mem.chipMask)) {
            trace(true, "CPU(8) OVERWRITES BLITTER AT ADDR %x\n", addr);
        }
    }

    agnus.executeUntilBusIsFree();
    
    dataBus = value;

    stats.chipWrites.raw++;
    agnus.busAddr[agnus.pos.h] = addr;
    agnus.busData[agnus.pos.h] = dataBus;

    WRITE_CHIP_8(addr, value);
}

template <> void
Memory::poke16 <Accessor::CPU, MemSrc::CHIP> (u32 addr, u16 value)
{
    ASSERT_CHIP_ADDR(addr);
    
    if (BLT_MEM_GUARD) {
        if (blitter.checkMemguard(addr & mem.chipMask)) {
            trace(true, "CPU(16) OVERWRITES BLITTER AT ADDR %x\n", addr);
        }
    }

    agnus.executeUntilBusIsFree();
    
    dataBus = value;

    stats.chipWrites.raw++;
    agnus.busAddr[agnus.pos.h] = addr;
    agnus.busData[agnus.pos.h] = dataBus;

    WRITE_CHIP_16(addr, value);
}

template <> void
Memory::poke8 <Accessor::CPU, MemSrc::SLOW> (u32 addr, u8 value)
{
    ASSERT_SLOW_ADDR(addr);
    
    agnus.executeUntilBusIsFree();
    
    dataBus = value;

    stats.slowWrites.raw++;
    agnus.busAddr[agnus.pos.h] = addr;
    agnus.busData[agnus.pos.h] = dataBus;
    
    WRITE_SLOW_8(addr, value);
}

template <> void
Memory::poke16 <Accessor::CPU, MemSrc::SLOW> (u32 addr, u16 value)
{
    ASSERT_SLOW_ADDR(addr);
    
    agnus.executeUntilBusIsFree();
    
    dataBus = value;

    stats.slowWrites.raw++;
    agnus.busAddr[agnus.pos.h] = addr;
    agnus.busData[agnus.pos.h] = dataBus;
    
    WRITE_SLOW_16(addr, value);
}

template <> void
Memory::poke8 <Accessor::CPU, MemSrc::FAST> (u32 addr, u8 value)
{
    ASSERT_FAST_ADDR(addr);
    
    stats.fastWrites.raw++;
    WRITE_FAST_8(addr, value);
}

template <> void
Memory::poke16 <Accessor::CPU, MemSrc::FAST> (u32 addr, u16 value)
{
    ASSERT_FAST_ADDR(addr);
    
    stats.fastWrites.raw++;
    WRITE_FAST_16(addr, value);
}

template <> void
Memory::poke8 <Accessor::CPU, MemSrc::CIA> (u32 addr, u8 value)
{
    ASSERT_CIA_ADDR(addr);

    agnus.executeUntilBusIsFreeForCIA();
    
    dataBus = value;
    pokeCIA8(addr, value);
}

template <> void
Memory::poke16 <Accessor::CPU, MemSrc::CIA> (u32 addr, u16 value)
{
    ASSERT_CIA_ADDR(addr);
    xfiles("CIA: Writing a WORD into %x\n", addr);

    agnus.executeUntilBusIsFreeForCIA();
    
    dataBus = value;
    pokeCIA16(addr, value);
}

template <> void
Memory::poke8 <Accessor::CPU, MemSrc::RTC> (u32 addr, u8 value)
{
    ASSERT_RTC_ADDR(addr);
    
    agnus.executeUntilBusIsFree();
    
    dataBus = value;
    pokeRTC8(addr, value);
}

template <> void
Memory::poke16 <Accessor::CPU, MemSrc::RTC> (u32 addr, u16 value)
{
    ASSERT_RTC_ADDR(addr);
    
    agnus.executeUntilBusIsFree();
    
    dataBus = value;
    
    agnus.busAddr[agnus.pos.h] = addr;
    agnus.busData[agnus.pos.h] = dataBus;
    
    pokeRTC16(addr, value);
}

template <> void
Memory::poke8 <Accessor::CPU, MemSrc::CUSTOM> (u32 addr, u8 value)
{
    ASSERT_CUSTOM_ADDR(addr);
    
    agnus.executeUntilBusIsFree();
    
    dataBus = value;
    
    agnus.busAddr[agnus.pos.h] = addr;
    agnus.busData[agnus.pos.h] = dataBus;
    
    // http://eab.abime.net/showthread.php?p=1156399
    pokeCustom16<Accessor::CPU>(addr & 0x1FE, HI_LO(value, value));
}

template <> void
Memory::poke16 <Accessor::CPU, MemSrc::CUSTOM> (u32 addr, u16 value)
{
    ASSERT_CUSTOM_ADDR(addr);

    agnus.executeUntilBusIsFree();

    dataBus = value;

    agnus.busAddr[agnus.pos.h] = addr;
    agnus.busData[agnus.pos.h] = dataBus;
    
    pokeCustom16<Accessor::CPU>(addr, value);
}

template <> void
Memory::poke8 <Accessor::CPU, MemSrc::AUTOCONF> (u32 addr, u8 value)
{
    ASSERT_AUTO_ADDR(addr);

    dataBus = value;
    zorro.pokeACF(addr, value);
}

template <> void
Memory::poke16 <Accessor::CPU, MemSrc::AUTOCONF> (u32 addr, u16 value)
{
    ASSERT_AUTO_ADDR(addr);
    
    dataBus = value;
    zorro.pokeACF(addr, HI_BYTE(value));
    zorro.pokeACF(addr + 1, LO_BYTE(value));
}

template <> void
Memory::poke8 <Accessor::CPU, MemSrc::ZOR> (u32 addr, u8 value)
{
    dataBus = value;
    zorro.poke8(addr, value);
}

template <> void
Memory::poke16 <Accessor::CPU, MemSrc::ZOR> (u32 addr, u16 value)
{
    dataBus = value;
    zorro.poke16(addr, value);
}

template <> void
Memory::poke8 <Accessor::CPU, MemSrc::ROM> (u32 addr, u8 value)
{
    ASSERT_ROM_ADDR(addr);
    
    stats.kickWrites.raw++;
    
    // On Amigas with a WOM, writing into ROM space locks the WOM
    if (hasWom() && !womIsLocked) {
        debug(MEM_DEBUG, "Locking WOM\n");
        womIsLocked = true;
        updateMemSrcTables();
    }
}

template <> void
Memory::poke16 <Accessor::CPU, MemSrc::ROM> (u32 addr, u16 value)
{
    poke8 <Accessor::CPU, MemSrc::ROM> (addr, (u8)value);
}

template <> void
Memory::poke8 <Accessor::CPU, MemSrc::WOM> (u32 addr, u8 value)
{
    ASSERT_WOM_ADDR(addr);
    
    stats.kickWrites.raw++;
    if (!womIsLocked) WRITE_WOM_8(addr, value);
}

template <> void
Memory::poke16 <Accessor::CPU, MemSrc::WOM> (u32 addr, u16 value)
{
    ASSERT_WOM_ADDR(addr);

    stats.kickWrites.raw++;
    if (!womIsLocked) WRITE_WOM_16(addr, value);
}

template <> void
Memory::poke8 <Accessor::CPU, MemSrc::EXT> (u32 addr, u8 value)
{
    ASSERT_EXT_ADDR(addr);
    stats.kickWrites.raw++;
}

template <> void
Memory::poke16 <Accessor::CPU, MemSrc::EXT> (u32 addr, u16 value)
{
    ASSERT_EXT_ADDR(addr);
    stats.kickWrites.raw++;
}

template<> void
Memory::poke8 <Accessor::CPU> (u32 addr, u8 value)
{
    addr &= 0xFFFFFF;
    
    switch (cpuMemSrc[addr >> 16]) {
            
        case MemSrc::NONE:          poke8 <Accessor::CPU, MemSrc::NONE>     (addr, value); return;
        case MemSrc::CHIP:          poke8 <Accessor::CPU, MemSrc::CHIP>     (addr, value); return;
        case MemSrc::CHIP_MIRROR:   poke8 <Accessor::CPU, MemSrc::CHIP>     (addr, value); return;
        case MemSrc::SLOW:          poke8 <Accessor::CPU, MemSrc::SLOW>     (addr, value); return;
        case MemSrc::FAST:          poke8 <Accessor::CPU, MemSrc::FAST>     (addr, value); return;
        case MemSrc::CIA:           poke8 <Accessor::CPU, MemSrc::CIA>      (addr, value); return;
        case MemSrc::CIA_MIRROR:    poke8 <Accessor::CPU, MemSrc::CIA>      (addr, value); return;
        case MemSrc::RTC:           poke8 <Accessor::CPU, MemSrc::RTC>      (addr, value); return;
        case MemSrc::CUSTOM:        poke8 <Accessor::CPU, MemSrc::CUSTOM>   (addr, value); return;
        case MemSrc::CUSTOM_MIRROR: poke8 <Accessor::CPU, MemSrc::CUSTOM>   (addr, value); return;
        case MemSrc::AUTOCONF:      poke8 <Accessor::CPU, MemSrc::AUTOCONF> (addr, value); return;
        case MemSrc::ZOR:           poke8 <Accessor::CPU, MemSrc::ZOR>      (addr, value); return;
        case MemSrc::ROM:           poke8 <Accessor::CPU, MemSrc::ROM>      (addr, value); return;
        case MemSrc::ROM_MIRROR:    poke8 <Accessor::CPU, MemSrc::ROM>      (addr, value); return;
        case MemSrc::WOM:           poke8 <Accessor::CPU, MemSrc::WOM>      (addr, value); return;
        case MemSrc::EXT:           poke8 <Accessor::CPU, MemSrc::EXT>      (addr, value); return;

        default:
            fatalError;
    }
}

template<> void
Memory::poke16 <Accessor::CPU> (u32 addr, u16 value)
{
    addr &= 0xFFFFFF;
    
    switch (cpuMemSrc[addr >> 16]) {
            
        case MemSrc::NONE:          poke16 <Accessor::CPU, MemSrc::NONE>     (addr, value); return;
        case MemSrc::CHIP:          poke16 <Accessor::CPU, MemSrc::CHIP>     (addr, value); return;
        case MemSrc::CHIP_MIRROR:   poke16 <Accessor::CPU, MemSrc::CHIP>     (addr, value); return;
        case MemSrc::SLOW:          poke16 <Accessor::CPU, MemSrc::SLOW>     (addr, value); return;
        case MemSrc::FAST:          poke16 <Accessor::CPU, MemSrc::FAST>     (addr, value); return;
        case MemSrc::CIA:           poke16 <Accessor::CPU, MemSrc::CIA>      (addr, value); return;
        case MemSrc::CIA_MIRROR:    poke16 <Accessor::CPU, MemSrc::CIA>      (addr, value); return;
        case MemSrc::RTC:           poke16 <Accessor::CPU, MemSrc::RTC>      (addr, value); return;
        case MemSrc::CUSTOM:        poke16 <Accessor::CPU, MemSrc::CUSTOM>   (addr, value); return;
        case MemSrc::CUSTOM_MIRROR: poke16 <Accessor::CPU, MemSrc::CUSTOM>   (addr, value); return;
        case MemSrc::AUTOCONF:      poke16 <Accessor::CPU, MemSrc::AUTOCONF> (addr, value); return;
        case MemSrc::ZOR:           poke16 <Accessor::CPU, MemSrc::ZOR>      (addr, value); return;
        case MemSrc::ROM:           poke16 <Accessor::CPU, MemSrc::ROM>      (addr, value); return;
        case MemSrc::ROM_MIRROR:    poke16 <Accessor::CPU, MemSrc::ROM>      (addr, value); return;
        case MemSrc::WOM:           poke16 <Accessor::CPU, MemSrc::WOM>      (addr, value); return;
        case MemSrc::EXT:           poke16 <Accessor::CPU, MemSrc::EXT>      (addr, value); return;

        default:
            fatalError;
    }
}

//
// Poke (Agnus)
//

template <> void
Memory::poke16 <Accessor::AGNUS, MemSrc::NONE> (u32 addr, u16 value)
{
    trace(MEM_DEBUG, "poke16 <AGNUS> (%x [NONE], %x)\n", addr, value);
    dataBus = value;
}

template <> void
Memory::poke16 <Accessor::AGNUS, MemSrc::CHIP> (u32 addr, u16 value)
{
    assert((addr & agnus.ptrMask) == addr);

    dataBus = value;
    WRITE_CHIP_16(addr, value);
}

template <> void
Memory::poke16 <Accessor::AGNUS, MemSrc::SLOW> (u32 addr, u16 value)
{
    xfiles("Agnus writes to Slow RAM mirror at %x\n", addr);

    dataBus = value;
    WRITE_SLOW_16(SLOW_RAM_STRT + (addr & 0x7FFFF), value);
}

template<> void
Memory::poke16 <Accessor::AGNUS> (u32 addr, u16 value)
{
    addr &= agnus.ptrMask;
    
    switch (agnusMemSrc[addr >> 16]) {
            
        case MemSrc::NONE:          poke16 <Accessor::AGNUS, MemSrc::NONE> (addr, value); return;
        case MemSrc::CHIP:          poke16 <Accessor::AGNUS, MemSrc::CHIP> (addr, value); return;
        case MemSrc::SLOW_MIRROR:   poke16 <Accessor::AGNUS, MemSrc::SLOW> (addr, value); return;
            
        default:
            fatalError;
    }
}

u8
Memory::peekCIA8(u32 addr)
{
    u16 reg = (u16)(addr >> 8 & 0b1111);
    u32 sel = (addr >> 12) & 0b11;
    bool a0 = addr & 1;
    
    switch (sel) {
            
        case 0b00:
            return a0 ? ciaa.peek(reg) : ciab.peek(reg);
            
        case 0b01:
            return a0 ? LO_BYTE(cpu.getIRD()) : ciab.peek(reg);
            
        case 0b10:
            return a0 ? ciaa.peek(reg) : HI_BYTE(cpu.getIRD());
            
        case 0b11:
            return a0 ? LO_BYTE(cpu.getIRD()) : HI_BYTE(cpu.getIRD());
            
        default:
            fatalError;
    }
}

u16
Memory::peekCIA16(u32 addr)
{
    u16 reg = (u16)(addr >> 8 & 0b1111);
    u32 sel = (addr >> 12) & 0b11;
    
    switch (sel) {
            
        case 0b00:
            return HI_LO(ciab.peek(reg), ciaa.peek(reg));
            
        case 0b01:
            return HI_LO(ciab.peek(reg), 0xFF);
            
        case 0b10:
            return HI_LO(0xFF, ciaa.peek(reg));
            
        case 0b11:
            return cpu.getIRD();
            
        default:
            fatalError;
    }
}

u8
Memory::spypeekCIA8(u32 addr) const
{
    u16 reg = (u16)(addr >> 8 & 0b1111);
    u32 sel = (addr >> 12) & 0b11;
    bool a0 = addr & 1;
    
    switch (sel) {
            
        case 0b00:
            return a0 ? ciaa.spypeek(reg) : ciab.spypeek(reg);
            
        case 0b01:
            return a0 ? LO_BYTE(cpu.getIRD()) : ciab.spypeek(reg);
            
        case 0b10:
            return a0 ? ciaa.spypeek(reg) : HI_BYTE(cpu.getIRD());
            
        case 0b11:
            return a0 ? LO_BYTE(cpu.getIRD()) : HI_BYTE(cpu.getIRD());
            
        default:
            fatalError;
    }
}

u16
Memory::spypeekCIA16(u32 addr) const
{
    u16 reg = (u16)(addr >> 8 & 0b1111);
    u32 sel = (addr >> 12) & 0b11;
    
    switch (sel) {
            
        case 0b00:
            return HI_LO(ciab.spypeek(reg), ciaa.spypeek(reg));
            
        case 0b01:
            return HI_LO(ciab.spypeek(reg), 0xFF);
            
        case 0b10:
            return HI_LO(0xFF, ciaa.spypeek(reg));
            
        case 0b11:
            return cpu.getIRD();
            
        default:
            fatalError;
    }
}

void
Memory::pokeCIA8(u32 addr, u8 value)
{
    u16 reg = (u16)(addr >> 8 & 0b1111);
    u32 selA = (addr & 0x1000) == 0;
    u32 selB = (addr & 0x2000) == 0;

    if (selA) ciaa.poke(reg, value);
    if (selB) ciab.poke(reg, value);
}

void
Memory::pokeCIA16(u32 addr, u16 value)
{
    u16 reg = (u16)(addr >> 8 & 0b1111);
    u32 selA = (addr & 0x1000) == 0;
    u32 selB = (addr & 0x2000) == 0;
    
    if (selA) ciaa.poke(reg, LO_BYTE(value));
    if (selB) ciab.poke(reg, HI_BYTE(value));
}

u8
Memory::peekRTC8(u32 addr) const
{
    /* Addr: 0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011
     * Reg:   --        --        --        --        --        --
     */
    if (IS_EVEN(addr)) return HI_BYTE(dataBus);
    
    /* Addr: 0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011
     * Reg:        00        00        11        11        22        22
     */
    if (rtc.getConfig().model == RTCRevision::NONE) {
        return 0x40; // This is the value I've seen on my A500
    } else {
        return rtc.peek((addr >> 2) & 0b1111);
    }
}

u16
Memory::peekRTC16(u32 addr) const
{
    return HI_LO(peekRTC8(addr), peekRTC8(addr + 1));
}

void
Memory::pokeRTC8(u32 addr, u8 value)
{
    /* Addr: 0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011
     * Reg:   --   -0   --   -0   --   -1   --   -1   --   -2   --   -2
     */
    if (IS_EVEN(addr)) return;
    
    /* Addr: 0001 0011 0101 0111 1001 1011
     * Reg:   -0   -0   -1   -1   -2   -2
     */
    rtc.poke((addr >> 2) & 0b1111, value);
}

void
Memory::pokeRTC16(u32 addr, u16 value)
{
    pokeRTC8(addr, HI_BYTE(value));
    pokeRTC8(addr + 1, LO_BYTE(value));
}

u16
Memory::peekCustom16(u32 addr)
{
    u16 result;

    switch ((addr >> 1) & 0xFF) {

        case 0x002 >> 1: // DMACONR
            result = agnus.peekDMACONR(); break;
        case 0x004 >> 1: // VPOSR
            result = agnus.peekVPOSR(); break;
        case 0x006 >> 1: // VHPOSR
            result = agnus.peekVHPOSR(); break;
        case 0x008 >> 1: // DSKDATR
            result = paula.diskController.peekDSKDATR(); break;
        case 0x00A >> 1: // JOY0DAT
            result = denise.peekJOY0DATR(); break;
        case 0x00C >> 1: // JOY1DAT
            result = denise.peekJOY1DATR(); break;
        case 0x00E >> 1: // CLXDAT
            result = denise.peekCLXDAT(); break;
        case 0x010 >> 1: // ADKCONR
            result = paula.peekADKCONR(); break;
        case 0x012 >> 1: // POT0DAT
            result = paula.peekPOTxDAT<0>(); break;
        case 0x014 >> 1: // POT1DAT
            result = paula.peekPOTxDAT<1>(); break;
        case 0x016 >> 1: // POTGOR
            result = paula.peekPOTGOR(); break;
        case 0x018 >> 1: // SERDATR
            result = uart.peekSERDATR(); break;
        case 0x01A >> 1: // DSKBYTR
            result = diskController.peekDSKBYTR(); break;
        case 0x01C >> 1: // INTENAR
            result = paula.peekINTENAR(); break;
        case 0x01E >> 1: // INTREQR
            result = paula.peekINTREQR(); break;
        case 0x07C >> 1: // DENISEID
            result = denise.peekDENISEID(); break;
        default:
            result = peekCustomFaulty16(addr);

    }

    trace(OCSREG_DEBUG, "peekCustom16(%X [%s]) = %X\n", addr, MemoryDebugger::regName(addr), result);

    dataBus = result;
    return result;
}

u16
Memory::peekCustomFaulty16(u32 addr)
{
    /* This functions is called when a write-only register or a
     * non-existing chipset register is read.
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

    pokeCustom16<Accessor::CPU>(addr, dataBus);
    
    return dataBus;
}

u16
Memory::spypeekCustom16(u32 addr) const
{
    switch ((addr >> 1) & 0xFF) {

        case 0x002 >> 1: // DMACONR
            return agnus.peekDMACONR();
        case 0x004 >> 1: // VPOSR
            return agnus.peekVPOSR();
        case 0x006 >> 1: // VHPOSR
            return agnus.peekVHPOSR();
        case 0x008 >> 1: // DSKDATR
            return diskController.peekDSKDATR();
        case 0x00A >> 1: // JOY0DAT
            return denise.peekJOY0DATR();
        case 0x00C >> 1: // JOY1DAT
            return denise.peekJOY1DATR();
        case 0x00E >> 1: // CLXDAT
            return denise.spypeekCLXDAT();
        case 0x010 >> 1: // ADKCONR
            return paula.peekADKCONR();
        case 0x012 >> 1: // POT0DAT
            return paula.peekPOTxDAT<0>();
        case 0x014 >> 1: // POT1DAT
            return paula.peekPOTxDAT<1>();
        case 0x016 >> 1: // POTGOR
            return paula.peekPOTGOR();
        case 0x018 >> 1: // SERDATR
            return uart.peekSERDATR();
        case 0x01A >> 1: // DSKBYTR
            return diskController.computeDSKBYTR();
        case 0x01C >> 1: // INTENAR
            return paula.peekINTENAR();
        case 0x01E >> 1: // INTREQR
            return paula.peekINTREQR();
        case 0x07C >> 1: // DENISEID
            return denise.spypeekDENISEID();

        default:
            return 0;
    }
}

template <Accessor s> void
Memory::pokeCustom16(u32 addr, u16 value)
{
    if ((addr & 0xFFF) == 0x30) {
        trace(OCSREG_DEBUG, "pokeCustom16(SERDAT, '%c')\n", (char)value);
    } else {
        trace(OCSREG_DEBUG, "pokeCustom16(%X [%s], %X)\n", addr, MemoryDebugger::regName(addr), value);
    }

    dataBus = value;

    switch ((addr >> 1) & 0xFF) {

        case 0x020 >> 1: // DSKPTH
            agnus.pokeDSKPTH<s>(value); return;
        case 0x022 >> 1: // DSKPTL
            agnus.pokeDSKPTL<s>(value); return;
        case 0x024 >> 1: // DSKLEN
            diskController.pokeDSKLEN(value); return;
        case 0x026 >> 1: // DSKDAT
            diskController.pokeDSKDAT(value); return;
        case 0x028 >> 1: // REFPTR
            return;
        case 0x02A >> 1: // VPOSW
            agnus.pokeVPOS(value); return;
        case 0x02C >> 1: // VHPOSW
            agnus.pokeVHPOS(value); return;
        case 0x02E >> 1: // COPCON
            copper.pokeCOPCON(value); return;
        case 0x030 >> 1: // SERDAT
            uart.pokeSERDAT(value); return;
        case 0x032 >> 1: // SERPER
            uart.pokeSERPER(value); return;
        case 0x034 >> 1: // POTGO
            paula.pokePOTGO(value); return;
        case 0x036 >> 1: // JOYTEST
            denise.pokeJOYTEST(value); return;
        case 0x038 >> 1: // STREQU
        case 0x03A >> 1: // STRVBL
        case 0x03C >> 1: // STRHOR
        case 0x03E >> 1: // STRLONG
            xfiles("STROBE register write: %04x\n", addr);
            return; // ignore
        case 0x040 >> 1: // BLTCON0
            blitter.pokeBLTCON0(value); return;
        case 0x042 >> 1: // BLTCON1
            blitter.pokeBLTCON1(value); return;
        case 0x044 >> 1: // BLTAFWM
            blitter.pokeBLTAFWM(value); return;
        case 0x046 >> 1: // BLTALWM
            blitter.pokeBLTALWM(value); return;
        case 0x048 >> 1: // BLTCPTH
            blitter.pokeBLTCPTH(value); return;
        case 0x04A >> 1: // BLTCPTL
            blitter.pokeBLTCPTL(value); return;
        case 0x04C >> 1: // BLTBPTH
            blitter.pokeBLTBPTH(value); return;
        case 0x04E >> 1: // BLTBPTL
            blitter.pokeBLTBPTL(value); return;
        case 0x050 >> 1: // BLTAPTH
            blitter.pokeBLTAPTH(value); return;
        case 0x052 >> 1: // BLTAPTL
            blitter.pokeBLTAPTL(value); return;
        case 0x054 >> 1: // BLTDPTH
            blitter.pokeBLTDPTH(value); return;
        case 0x056 >> 1: // BLTDPTL
            blitter.pokeBLTDPTL(value); return;
        case 0x058 >> 1: // BLTSIZE
            blitter.pokeBLTSIZE<s>(value); return;
        case 0x05A >> 1: // BLTCON0L (ECS)
            blitter.pokeBLTCON0L(value); return;
        case 0x05C >> 1: // BLTSIZV (ECS)
            blitter.pokeBLTSIZV(value); return;
        case 0x05E >> 1: // BLTSIZH (ECS)
            blitter.pokeBLTSIZH(value); return;
        case 0x060 >> 1: // BLTCMOD
            blitter.pokeBLTCMOD(value); return;
        case 0x062 >> 1: // BLTBMOD
            blitter.pokeBLTBMOD(value); return;
        case 0x064 >> 1: // BLTAMOD
            blitter.pokeBLTAMOD(value); return;
        case 0x066 >> 1: // BLTDMOD
            blitter.pokeBLTDMOD(value); return;
        case 0x068 >> 1: // unused
        case 0x06A >> 1: // unused
        case 0x06C >> 1: // unused
        case 0x06E >> 1: // unused
            break;
        case 0x070 >> 1: // BLTCDAT
            blitter.pokeBLTCDAT(value); return;
        case 0x072 >> 1: // BLTBDAT
            blitter.pokeBLTBDAT(value); return;
        case 0x074 >> 1: // BLTADAT
            blitter.pokeBLTADAT(value); return;
        case 0x076 >> 1: // unused
        case 0x078 >> 1: // unused
        case 0x07A >> 1: // unused
        case 0x07C >> 1: // unused
            break;
        case 0x07E >> 1: // DSKSYNC
            diskController.pokeDSKSYNC(value); return;
        case 0x080 >> 1: // COP1LCH
            copper.pokeCOP1LCH(value); return;
        case 0x082 >> 1: // COP1LCL
            copper.pokeCOP1LCL(value); return;
        case 0x084 >> 1: // COP2LCH
            copper.pokeCOP2LCH(value); return;
        case 0x086 >> 1: // COP2LCL
            copper.pokeCOP2LCL(value); return;
        case 0x088 >> 1: // COPJMP1
            copper.pokeCOPJMP1<s>(); return;
        case 0x08A >> 1: // COPJMP2
            copper.pokeCOPJMP2<s>(); return;
        case 0x08C >> 1: // COPINS
            copper.pokeCOPINS(value); return;
        case 0x08E >> 1: // DIWSTRT
            agnus.pokeDIWSTRT<s>(value); return;
        case 0x090 >> 1: // DIWSTOP
            agnus.pokeDIWSTOP<s>(value); return;
        case 0x092 >> 1: // DDFSTRT
            agnus.sequencer.pokeDDFSTRT<s>(value); return;
        case 0x094 >> 1: // DDFSTOP
            agnus.sequencer.pokeDDFSTOP<s>(value); return;
        case 0x096 >> 1: // DMACON
            agnus.pokeDMACON<s>(value); return;
        case 0x098 >> 1:  // CLXCON
            denise.pokeCLXCON(value); return;
        case 0x09A >> 1: // INTENA
            paula.pokeINTENA<s>(value); return;
        case 0x09C >> 1: // INTREQ
            paula.pokeINTREQ<s>(value); return;
        case 0x09E >> 1: // ADKCON
            paula.pokeADKCON(value); return;
        case 0x0A0 >> 1: // AUD0LCH
            agnus.pokeAUDxLCH<0,s>(value); return;
        case 0x0A2 >> 1: // AUD0LCL
            agnus.pokeAUDxLCL<0,s>(value); return;
        case 0x0A4 >> 1: // AUD0LEN
            paula.channel0.pokeAUDxLEN(value); return;
        case 0x0A6 >> 1: // AUD0PER
            paula.channel0.pokeAUDxPER(value); return;
        case 0x0A8 >> 1: // AUD0VOL
            paula.channel0.pokeAUDxVOL(value); return;
        case 0x0AA >> 1: // AUD0DAT
            paula.channel0.pokeAUDxDAT(value); return;
        case 0x0AC >> 1: // Unused
        case 0x0AE >> 1: // Unused
            break;
        case 0x0B0 >> 1: // AUD1LCH
            agnus.pokeAUDxLCH<1,s>(value); return;
        case 0x0B2 >> 1: // AUD1LCL
            agnus.pokeAUDxLCL<1,s>(value); return;
        case 0x0B4 >> 1: // AUD1LEN
            paula.channel1.pokeAUDxLEN(value); return;
        case 0x0B6 >> 1: // AUD1PER
            paula.channel1.pokeAUDxPER(value); return;
        case 0x0B8 >> 1: // AUD1VOL
            paula.channel1.pokeAUDxVOL(value); return;
        case 0x0BA >> 1: // AUD1DAT
            paula.channel1.pokeAUDxDAT(value); return;
        case 0x0BC >> 1: // Unused
        case 0x0BE >> 1: // Unused
            break;
        case 0x0C0 >> 1: // AUD2LCH
            agnus.pokeAUDxLCH<2,s>(value); return;
        case 0x0C2 >> 1: // AUD2LCL
            agnus.pokeAUDxLCL<2,s>(value); return;
        case 0x0C4 >> 1: // AUD2LEN
            paula.channel2.pokeAUDxLEN(value); return;
        case 0x0C6 >> 1: // AUD2PER
            paula.channel2.pokeAUDxPER(value); return;
        case 0x0C8 >> 1: // AUD2VOL
            paula.channel2.pokeAUDxVOL(value); return;
        case 0x0CA >> 1: // AUD2DAT
            paula.channel2.pokeAUDxDAT(value); return;
        case 0x0CC >> 1: // Unused
        case 0x0CE >> 1: // Unused
            break;
        case 0x0D0 >> 1: // AUD3LCH
            agnus.pokeAUDxLCH<3,s>(value); return;
        case 0x0D2 >> 1: // AUD3LCL
            agnus.pokeAUDxLCL<3,s>(value); return;
        case 0x0D4 >> 1: // AUD3LEN
            paula.channel3.pokeAUDxLEN(value); return;
        case 0x0D6 >> 1: // AUD3PER
            paula.channel3.pokeAUDxPER(value); return;
        case 0x0D8 >> 1: // AUD3VOL
            paula.channel3.pokeAUDxVOL(value); return;
        case 0x0DA >> 1: // AUD3DAT
            paula.channel3.pokeAUDxDAT(value); return;
        case 0x0DC >> 1: // Unused
        case 0x0DE >> 1: // Unused
            break;
        case 0x0E0 >> 1: // BPL1PTH
            agnus.pokeBPLxPTH<1,s>(value); return;
        case 0x0E2 >> 1: // BPL1PTL
            agnus.pokeBPLxPTL<1,s>(value); return;
        case 0x0E4 >> 1: // BPL2PTH
            agnus.pokeBPLxPTH<2,s>(value); return;
        case 0x0E6 >> 1: // BPL2PTL
            agnus.pokeBPLxPTL<2,s>(value); return;
        case 0x0E8 >> 1: // BPL3PTH
            agnus.pokeBPLxPTH<3,s>(value); return;
        case 0x0EA >> 1: // BPL3PTL
            agnus.pokeBPLxPTL<3,s>(value); return;
        case 0x0EC >> 1: // BPL4PTH
            agnus.pokeBPLxPTH<4,s>(value); return;
        case 0x0EE >> 1: // BPL4PTL
            agnus.pokeBPLxPTL<4,s>(value); return;
        case 0x0F0 >> 1: // BPL5PTH
            agnus.pokeBPLxPTH<5,s>(value); return;
        case 0x0F2 >> 1: // BPL5PTL
            agnus.pokeBPLxPTL<5,s>(value); return;
        case 0x0F4 >> 1: // BPL6PTH
            agnus.pokeBPLxPTH<6,s>(value); return;
        case 0x0F6 >> 1: // BPL6PTL
            agnus.pokeBPLxPTL<6,s>(value); return;
        case 0x0F8 >> 1: // Unused
        case 0x0FA >> 1: // Unused
        case 0x0FC >> 1: // Unused
        case 0x0FE >> 1: // Unused
            break;
        case 0x100 >> 1: // BPLCON0
            agnus.pokeBPLCON0<s>(value);
            denise.pokeBPLCON0<s>(value);
            return;
        case 0x102 >> 1: // BPLCON1
            agnus.pokeBPLCON1(value);
            denise.pokeBPLCON1<s>(value);
            return;
        case 0x104 >> 1: // BPLCON2
            denise.pokeBPLCON2<s>(value); return;
        case 0x106 >> 1: // BPLCON3 (ECS)
            denise.pokeBPLCON3<s>(value);
            break;
        case 0x108 >> 1: // BPL1MOD
            agnus.pokeBPL1MOD(value); return;
        case 0x10A >> 1: // BPL2MOD
            agnus.pokeBPL2MOD(value); return;
        case 0x10C >> 1: // Unused
        case 0x10E >> 1: // Unused
            break;
        case 0x110 >> 1: // BPL1DAT
            denise.pokeBPLxDAT<0,s>(value); return;
        case 0x112 >> 1: // BPL2DAT
            denise.pokeBPLxDAT<1,s>(value); return;
        case 0x114 >> 1: // BPL3DAT
            denise.pokeBPLxDAT<2,s>(value); return;
        case 0x116 >> 1: // BPL4DAT
            denise.pokeBPLxDAT<3,s>(value); return;
        case 0x118 >> 1: // BPL5DAT
            denise.pokeBPLxDAT<4,s>(value); return;
        case 0x11A >> 1: // BPL6DAT
            denise.pokeBPLxDAT<5,s>(value); return;
        case 0x11C >> 1: // Unused
        case 0x11E >> 1: // Unused
            break;
        case 0x120 >> 1: // SPR0PTH
            agnus.pokeSPRxPTH<0,s>(value); return;
        case 0x122 >> 1: // SPR0PTL
            agnus.pokeSPRxPTL<0,s>(value); return;
        case 0x124 >> 1: // SPR1PTH
            agnus.pokeSPRxPTH<1,s>(value); return;
        case 0x126 >> 1: // SPR1PTL
            agnus.pokeSPRxPTL<1,s>(value); return;
        case 0x128 >> 1: // SPR2PTH
            agnus.pokeSPRxPTH<2,s>(value); return;
        case 0x12A >> 1: // SPR2PTL
            agnus.pokeSPRxPTL<2,s>(value); return;
        case 0x12C >> 1: // SPR3PTH
            agnus.pokeSPRxPTH<3,s>(value); return;
        case 0x12E >> 1: // SPR3PTL
            agnus.pokeSPRxPTL<3,s>(value); return;
        case 0x130 >> 1: // SPR4PTH
            agnus.pokeSPRxPTH<4,s>(value); return;
        case 0x132 >> 1: // SPR4PTL
            agnus.pokeSPRxPTL<4,s>(value); return;
        case 0x134 >> 1: // SPR5PTH
            agnus.pokeSPRxPTH<5,s>(value); return;
        case 0x136 >> 1: // SPR5PTL
            agnus.pokeSPRxPTL<5,s>(value); return;
        case 0x138 >> 1: // SPR6PTH
            agnus.pokeSPRxPTH<6,s>(value); return;
        case 0x13A >> 1: // SPR6PTL
            agnus.pokeSPRxPTL<6,s>(value); return;
        case 0x13C >> 1: // SPR7PTH
            agnus.pokeSPRxPTH<7,s>(value); return;
        case 0x13E >> 1: // SPR7PTL
            agnus.pokeSPRxPTL<7,s>(value); return;
        case 0x140 >> 1: // SPR0POS
            agnus.pokeSPRxPOS<0,s>(value); denise.pokeSPRxPOS<0>(value); return;
        case 0x142 >> 1: // SPR0CTL
            agnus.pokeSPRxCTL<0,s>(value); denise.pokeSPRxCTL<0>(value); return;
        case 0x144 >> 1: // SPR0DATA
            denise.pokeSPRxDATA<0>(value); return;
        case 0x146 >> 1: // SPR0DATB
            denise.pokeSPRxDATB<0>(value); return;
        case 0x148 >> 1: // SPR1POS
            agnus.pokeSPRxPOS<1,s>(value); denise.pokeSPRxPOS<1>(value); return;
        case 0x14A >> 1: // SPR1CTL
            agnus.pokeSPRxCTL<1,s>(value); denise.pokeSPRxCTL<1>(value); return;
        case 0x14C >> 1: // SPR1DATA
            denise.pokeSPRxDATA<1>(value); return;
        case 0x14E >> 1: // SPR1DATB
            denise.pokeSPRxDATB<1>(value); return;
        case 0x150 >> 1: // SPR2POS
            agnus.pokeSPRxPOS<2,s>(value); denise.pokeSPRxPOS<2>(value); return;
        case 0x152 >> 1: // SPR2CTL
            agnus.pokeSPRxCTL<2,s>(value); denise.pokeSPRxCTL<2>(value); return;
        case 0x154 >> 1: // SPR2DATA
            denise.pokeSPRxDATA<2>(value); return;
        case 0x156 >> 1: // SPR2DATB
            denise.pokeSPRxDATB<2>(value); return;
        case 0x158 >> 1: // SPR3POS
            agnus.pokeSPRxPOS<3,s>(value); denise.pokeSPRxPOS<3>(value); return;
        case 0x15A >> 1: // SPR3CTL
            agnus.pokeSPRxCTL<3,s>(value); denise.pokeSPRxCTL<3>(value); return;
        case 0x15C >> 1: // SPR3DATA
            denise.pokeSPRxDATA<3>(value); return;
        case 0x15E >> 1: // SPR3DATB
            denise.pokeSPRxDATB<3>(value); return;
        case 0x160 >> 1: // SPR4POS
            agnus.pokeSPRxPOS<4,s>(value); denise.pokeSPRxPOS<4>(value); return;
        case 0x162 >> 1: // SPR4CTL
            agnus.pokeSPRxCTL<4,s>(value); denise.pokeSPRxCTL<4>(value); return;
        case 0x164 >> 1: // SPR4DATA
            denise.pokeSPRxDATA<4>(value); return;
        case 0x166 >> 1: // SPR4DATB
            denise.pokeSPRxDATB<4>(value); return;
        case 0x168 >> 1: // SPR5POS
            agnus.pokeSPRxPOS<5,s>(value); denise.pokeSPRxPOS<5>(value); return;
        case 0x16A >> 1: // SPR5CTL
            agnus.pokeSPRxCTL<5,s>(value); denise.pokeSPRxCTL<5>(value); return;
        case 0x16C >> 1: // SPR5DATA
            denise.pokeSPRxDATA<5>(value); return;
        case 0x16E >> 1: // SPR5DATB
            denise.pokeSPRxDATB<5>(value); return;
        case 0x170 >> 1: // SPR6POS
            agnus.pokeSPRxPOS<6,s>(value); denise.pokeSPRxPOS<6>(value); return;
        case 0x172 >> 1: // SPR6CTL
            agnus.pokeSPRxCTL<6,s>(value); denise.pokeSPRxCTL<6>(value); return;
        case 0x174 >> 1: // SPR6DATA
            denise.pokeSPRxDATA<6>(value); return;
        case 0x176 >> 1: // SPR6DATB
            denise.pokeSPRxDATB<6>(value); return;
        case 0x178 >> 1: // SPR7POS
            agnus.pokeSPRxPOS<7,s>(value); denise.pokeSPRxPOS<7>(value); return;
        case 0x17A >> 1: // SPR7CTL
            agnus.pokeSPRxCTL<7,s>(value); denise.pokeSPRxCTL<7>(value); return;
        case 0x17C >> 1: // SPR7DATA
            denise.pokeSPRxDATA<7>(value); return;
        case 0x17E >> 1: // SPR7DATB
            denise.pokeSPRxDATB<7>(value); return;
        case 0x180 >> 1: // COLOR00
            denise.pokeCOLORxx<0,s>(value); return;
        case 0x182 >> 1: // COLOR01
            denise.pokeCOLORxx<1,s>(value); return;
        case 0x184 >> 1: // COLOR02
            denise.pokeCOLORxx<2,s>(value); return;
        case 0x186 >> 1: // COLOR03
            denise.pokeCOLORxx<3,s>(value); return;
        case 0x188 >> 1: // COLOR04
            denise.pokeCOLORxx<4,s>(value); return;
        case 0x18A >> 1: // COLOR05
            denise.pokeCOLORxx<5,s>(value); return;
        case 0x18C >> 1: // COLOR06
            denise.pokeCOLORxx<6,s>(value); return;
        case 0x18E >> 1: // COLOR07
            denise.pokeCOLORxx<7,s>(value); return;
        case 0x190 >> 1: // COLOR08
            denise.pokeCOLORxx<8,s>(value); return;
        case 0x192 >> 1: // COLOR09
            denise.pokeCOLORxx<9,s>(value); return;
        case 0x194 >> 1: // COLOR10
            denise.pokeCOLORxx<10,s>(value); return;
        case 0x196 >> 1: // COLOR11
            denise.pokeCOLORxx<11,s>(value); return;
        case 0x198 >> 1: // COLOR12
            denise.pokeCOLORxx<12,s>(value); return;
        case 0x19A >> 1: // COLOR13
            denise.pokeCOLORxx<13,s>(value); return;
        case 0x19C >> 1: // COLOR14
            denise.pokeCOLORxx<14,s>(value); return;
        case 0x19E >> 1: // COLOR15
            denise.pokeCOLORxx<15,s>(value); return;
        case 0x1A0 >> 1: // COLOR16
            denise.pokeCOLORxx<16,s>(value); return;
        case 0x1A2 >> 1: // COLOR17
            denise.pokeCOLORxx<17,s>(value); return;
        case 0x1A4 >> 1: // COLOR18
            denise.pokeCOLORxx<18,s>(value); return;
        case 0x1A6 >> 1: // COLOR19
            denise.pokeCOLORxx<19,s>(value); return;
        case 0x1A8 >> 1: // COLOR20
            denise.pokeCOLORxx<20,s>(value); return;
        case 0x1AA >> 1: // COLOR21
            denise.pokeCOLORxx<21,s>(value); return;
        case 0x1AC >> 1: // COLOR22
            denise.pokeCOLORxx<22,s>(value); return;
        case 0x1AE >> 1: // COLOR23
            denise.pokeCOLORxx<23,s>(value); return;
        case 0x1B0 >> 1: // COLOR24
            denise.pokeCOLORxx<24,s>(value); return;
        case 0x1B2 >> 1: // COLOR25
            denise.pokeCOLORxx<25,s>(value); return;
        case 0x1B4 >> 1: // COLOR26
            denise.pokeCOLORxx<26,s>(value); return;
        case 0x1B6 >> 1: // COLOR27
            denise.pokeCOLORxx<27,s>(value); return;
        case 0x1B8 >> 1: // COLOR28
            denise.pokeCOLORxx<28,s>(value); return;
        case 0x1BA >> 1: // COLOR29
            denise.pokeCOLORxx<29,s>(value); return;
        case 0x1BC >> 1: // COLOR30
            denise.pokeCOLORxx<30,s>(value); return;
        case 0x1BE >> 1: // COLOR31
            denise.pokeCOLORxx<31,s>(value); return;
        case 0x1DC >> 1: // BEAMCON0
            agnus.pokeBEAMCON0(value); return;
        case 0x1E4 >> 1: // DIWHIGH (ECS)
            agnus.pokeDIWHIGH<s>(value); return;
        case 0x1FE >> 1: // NO-OP
            copper.pokeNOOP(value); return;
    }
    
    if (addr <= 0x1E) {
        trace(INVREG_DEBUG,
              "pokeCustom16(%X [%s]): READ-ONLY\n", addr, MemoryDebugger::regName(addr));
    } else {
        trace(INVREG_DEBUG,
              "pokeCustom16(%X [%s]): NON-OCS\n", addr, MemoryDebugger::regName(addr));
    }
}

template <> void
Memory::patch <MemSrc::CHIP> (u32 addr, u8 value)
{
    ASSERT_CHIP_ADDR(addr);
    WRITE_CHIP_8(addr, value);
}

template <> void
Memory::patch <MemSrc::SLOW> (u32 addr, u8 value)
{
    ASSERT_SLOW_ADDR(addr);
    WRITE_SLOW_8(addr, value);
}

template <> void
Memory::patch <MemSrc::FAST> (u32 addr, u8 value)
{
    ASSERT_FAST_ADDR(addr);
    WRITE_FAST_8(addr, value);
}

template <> void
Memory::patch <MemSrc::ROM> (u32 addr, u8 value)
{
    ASSERT_ROM_ADDR(addr);
    WRITE_ROM_8(addr, value);
}

template <> void
Memory::patch <MemSrc::WOM> (u32 addr, u8 value)
{
    ASSERT_WOM_ADDR(addr);
    WRITE_WOM_8(addr, value);
}

template <> void
Memory::patch <MemSrc::EXT> (u32 addr, u8 value)
{
    ASSERT_EXT_ADDR(addr);
    WRITE_EXT_8(addr, value);
}

void
Memory::patch(u32 addr, u8 value)
{
    addr &= 0xFFFFFF;
    
    switch (cpuMemSrc[addr >> 16]) {
            
        case MemSrc::CHIP:          patch <MemSrc::CHIP>     (addr, value); return;
        case MemSrc::CHIP_MIRROR:   patch <MemSrc::CHIP>     (addr, value); return;
        case MemSrc::SLOW:          patch <MemSrc::SLOW>     (addr, value); return;
        case MemSrc::FAST:          patch <MemSrc::FAST>     (addr, value); return;
        case MemSrc::ROM:           patch <MemSrc::ROM>      (addr, value); return;
        case MemSrc::ROM_MIRROR:    patch <MemSrc::ROM>      (addr, value); return;
        case MemSrc::WOM:           patch <MemSrc::WOM>      (addr, value); return;
        case MemSrc::EXT:           patch <MemSrc::EXT>      (addr, value); return;
            
        default:
            break;
    }
}

void
Memory::patch(u32 addr, u16 value)
{
    assert(IS_EVEN(addr));
    
    patch(addr,     (u8)HI_BYTE(value));
    patch(addr + 1, (u8)LO_BYTE(value));
}

void
Memory::patch(u32 addr, u32 value)
{
    assert(IS_EVEN(addr));
    
    patch(addr,     (u16)HI_WORD(value));
    patch(addr + 2, (u16)LO_WORD(value));
}

void
Memory::patch(u32 addr, u8 *buf, isize len)
{
    assert(buf);
    
    for (isize i = 0; i < len; i++) {
        patch(u32(addr + i), buf[i]);
    }
}

void 
Memory::eofHandler()
{
    // Update statistics
    updateStats();
}

std::vector <u32>
Memory::search(u64 pattern, isize bytes)
{
    std::vector <u32> result;

    // Iterate through all memory banks
    for (isize bank = 0; bank < 256; bank++) {
        
        // Only proceed if this memory bank is mapped
        if (cpuMemSrc[bank] == MemSrc::NONE) continue;

        auto lo = (u32)(bank << 16);
        auto hi = lo + 0xFFFF;

        for (u32 addr = lo; addr <= hi; addr++) {

            isize j;
            
            for (j = 0; j < bytes; j++) {

                auto mem = spypeek8 <Accessor::CPU> (addr + (u32)j);
                if (GET_BYTE(pattern, bytes - 1 - j) != mem) break;
            }
            if (j == bytes && result.size() < 128) {
                
                result.push_back(addr);
                if (result.size() >= 128) break;
            }
        }
    }
    
    return result;
}

template void Memory::pokeCustom16 <Accessor::CPU> (u32 addr, u16 value);
template void Memory::pokeCustom16 <Accessor::AGNUS> (u32 addr, u16 value);

}
