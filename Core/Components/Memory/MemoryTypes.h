// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#pragma once

#include "BasicTypes.h"

namespace vamiga {

/* Memory source identifiers. The identifiers are used in the mem source lookup
 * table to specify the source and target of a peek or poke operation,
 * respectively.
 */
/// Source of a memory access.
enum class MemSrc : long
{
    NONE,
    CHIP,
    CHIP_MIRROR,
    SLOW,
    SLOW_MIRROR,
    FAST,
    CIA,
    CIA_MIRROR,
    RTC,
    CUSTOM,
    CUSTOM_MIRROR,
    AUTOCONF,
    ZOR,
    ROM,
    ROM_MIRROR,
    WOM,
    EXT
};

struct MemSrcEnum : utl::Reflectable<MemSrcEnum, MemSrc>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = (long)MemSrc::EXT;
    
    static const char *_key(long value) { return _key(MemSrc(value)); }
    static const char *_key(MemSrc value)
    {
        switch (value) {
                
            case MemSrc::NONE:           return "NONE";
            case MemSrc::CHIP:           return "CHIP";
            case MemSrc::CHIP_MIRROR:    return "CHIP_MIRROR";
            case MemSrc::SLOW:           return "SLOW";
            case MemSrc::SLOW_MIRROR:    return "SLOW_MIRROR";
            case MemSrc::FAST:           return "FAST";
            case MemSrc::CIA:            return "CIA";
            case MemSrc::CIA_MIRROR:     return "CIA_MIRROR";
            case MemSrc::RTC:            return "RTC";
            case MemSrc::CUSTOM:         return "CUSTOM";
            case MemSrc::CUSTOM_MIRROR:  return "CUSTOM_MIRROR";
            case MemSrc::AUTOCONF:       return "AUTOCONF";
            case MemSrc::ZOR:            return "ZOR";
            case MemSrc::ROM:            return "ROM";
            case MemSrc::ROM_MIRROR:     return "ROM_MIRROR";
            case MemSrc::WOM:            return "WOM";
            case MemSrc::EXT:            return "EXT";
        }
        return "???";
    }
    static const char *help(long value) { return help(MemSrc(value)); }
    static const char *help(MemSrc value)
    {
        switch (value) {
                
            case MemSrc::NONE:           return "Unmapped";
            case MemSrc::CHIP:           return "Chip RAM";
            case MemSrc::CHIP_MIRROR:    return "Chip RAM mirror";
            case MemSrc::SLOW:           return "Slow RAM";
            case MemSrc::SLOW_MIRROR:    return "Slow RAM mirror";
            case MemSrc::FAST:           return "Fast RAM";
            case MemSrc::CIA:            return "CIA";
            case MemSrc::CIA_MIRROR:     return "CIA mirror";
            case MemSrc::RTC:            return "Real-time clock";
            case MemSrc::CUSTOM:         return "Custom chips";
            case MemSrc::CUSTOM_MIRROR:  return "Custom chips mirror";
            case MemSrc::AUTOCONF:       return "Auto config";
            case MemSrc::ZOR:            return "Zorro boards";
            case MemSrc::ROM:            return "Kickstart ROM";
            case MemSrc::ROM_MIRROR:     return "Kickstart ROM mirror";
            case MemSrc::WOM:            return "Write-only memory";
            case MemSrc::EXT:            return "Extension ROM";
        }
        return "???";
    }
};

/// The component that performs a memory access.
enum class Accessor : i8
{
    CPU,
    AGNUS,
    DENISE
};

struct AccessorEnum : utl::Reflectable<AccessorEnum, Accessor>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Accessor::DENISE);
    
    static const char *_key(Accessor value)
    {
        switch (value) {
                
            case Accessor::CPU:     return "CPU";
            case Accessor::AGNUS:   return "AGNUS";
            case Accessor::DENISE:  return "DENISE";
        }
        return "???";
    }
    static const char *help(Accessor value)
    {
        return "";
    }
};

/// Memory bank layout of an Amiga model.
enum class BankMap : long
{
    A500,
    A1000,
    A2000A,
    A2000B,
    A1200
};

struct BankMapEnum : utl::Reflectable<BankMapEnum, BankMap>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(BankMap::A1200);
    
    static const char *_key(BankMap value)
    {
        switch (value) {
                
            case BankMap::A500:    return "A500";
            case BankMap::A1000:   return "A1000";
            case BankMap::A2000A:  return "A2000A";
            case BankMap::A2000B:  return "A2000B";
            case BankMap::A1200:   return "A1200";
        }
        return "???";
    }
    static const char *help(BankMap value)
    {
        switch (value) {
                
            case BankMap::A500:    return "Amiga 500 memory layout";
            case BankMap::A1000:   return "Amiga 1000 memory layout";
            case BankMap::A2000A:  return "Amiga 2000A memory layout";
            case BankMap::A2000B:  return "Amiga 2000B memory layout";
            case BankMap::A1200:   return "Amiga 1200 memory layout";
        }
        return "???";
    }
};

/// Pattern that RAM is initialized with.
enum class RamInitPattern
{
    ALL_ZEROES,
    ALL_ONES,
    RANDOMIZED
};

struct RamInitPatternEnum : utl::Reflectable<RamInitPatternEnum, RamInitPattern>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(RamInitPattern::RANDOMIZED);
    
    static const char *_key(RamInitPattern value)
    {
        switch (value) {
                
            case RamInitPattern::ALL_ZEROES:    return "ALL_ZEROES";
            case RamInitPattern::ALL_ONES:      return "ALL_ONES";
            case RamInitPattern::RANDOMIZED:    return "RANDOMIZED";
        }
        return "???";
    }
    static const char *help(RamInitPattern value)
    {
        switch (value) {
                
            case RamInitPattern::ALL_ZEROES:    return "All bits set to 0";
            case RamInitPattern::ALL_ONES:      return "All bits set to 1";
            case RamInitPattern::RANDOMIZED:    return "Random bit patterns";
        }
        return "???";
    }
};

/// Value that is read from unmapped memory.
enum class UnmappedMemory
{
    FLOATING,
    ALL_ZEROES,
    ALL_ONES
};

struct UnmappedMemoryEnum : utl::Reflectable<UnmappedMemoryEnum, UnmappedMemory>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(UnmappedMemory::ALL_ONES);
    
    static const char *_key(UnmappedMemory value)
    {
        switch (value) {
                
            case UnmappedMemory::FLOATING:      return "FLOATING";
            case UnmappedMemory::ALL_ZEROES:    return "ALL_ZEROES";
            case UnmappedMemory::ALL_ONES:      return "ALL_ONES";
        }
        return "???";
    }
    static const char *help(UnmappedMemory value)
    {
        switch (value) {
                
            case UnmappedMemory::FLOATING:      return "Folating bus";
            case UnmappedMemory::ALL_ZEROES:    return "All bits read as 0";
            case UnmappedMemory::ALL_ONES:      return "All bits read as 1";
        }
        return "???";
    }
};


//
// Structures
//

/// Configuration options of the memory.
typedef struct
{
    /// RAM size in bytes
    i32 chipSize;
    i32 slowSize;                           ///< Slow RAM size (in bytes)
    i32 fastSize;                           ///< Fast RAM size (in bytes)
    
    /// ROM size in bytes
    i32 romSize;
    i32 womSize;                            ///< Size of the Write-Once-Memory (in bytes)
    i32 extSize;                            ///< Size of the Extension ROM (in bytes)
    
    /// First memory page where the extended ROM is blended it
    u32 extStart;
    
    /// Data bus width (16 or 32)
    u8 busWidth;
    
    /// Indicates if snapshots should contain Roms
    bool saveRoms;
    
    /// Indicates if slow Ram accesses need a free bus
    bool slowRamDelay;
    
    /// Special ECS feature
    bool slowRamMirror;
    
    /// Memory layout
    BankMap bankMap;
    
    /// Ram contents on startup
    RamInitPattern ramInitPattern;
    
    /// Specifies how to deal with unmapped memory
    UnmappedMemory unmappingType;
}
MemConfig;

/// State of the memory.
typedef struct
{
    bool hasRom;                            ///< A Kickstart ROM is installed
    bool hasWom;                            ///< A Write-Once-Memory is installed
    bool hasExt;                            ///< An Extension ROM is installed
    bool hasBootRom;                        ///< The installed ROM is a Boot ROM
    bool hasKickRom;                        ///< The installed ROM is a Kickstart ROM
    bool womLock;                           ///< The Write-Once-Memory is write-protected
    
    u32 romMask;                            ///< Address mask of the ROM
    u32 womMask;                            ///< Address mask of the Write-Once-Memory
    u32 extMask;                            ///< Address mask of the Extension ROM
    u32 chipMask;                           ///< Address mask of the Chip RAM
    
    MemSrc cpuMemSrc[256];                  ///< Memory source of each 64 KB bank as seen by the CPU
    MemSrc agnusMemSrc[256];                ///< Memory source of each 64 KB bank as seen by Agnus
}
MemInfo;

/// Statistics collected by the memory (access counters).
typedef struct
{
    struct
    {
        isize raw;              ///< Number of accesses since the last update
        double accumulated;     ///< Exponentially weighted average of the access count
    } chipReads;  ///< Chip RAM reads
    struct
    {
        isize raw;              ///< Number of accesses since the last update
        double accumulated;     ///< Exponentially weighted average of the access count
    } chipWrites;  ///< Chip RAM writes
    struct
    {
        isize raw;              ///< Number of accesses since the last update
        double accumulated;     ///< Exponentially weighted average of the access count
    } slowReads;  ///< Slow RAM reads
    struct
    {
        isize raw;              ///< Number of accesses since the last update
        double accumulated;     ///< Exponentially weighted average of the access count
    } slowWrites;  ///< Slow RAM writes
    struct
    {
        isize raw;              ///< Number of accesses since the last update
        double accumulated;     ///< Exponentially weighted average of the access count
    } fastReads;  ///< Fast RAM reads
    struct
    {
        isize raw;              ///< Number of accesses since the last update
        double accumulated;     ///< Exponentially weighted average of the access count
    } fastWrites;  ///< Fast RAM writes
    struct
    {
        isize raw;              ///< Number of accesses since the last update
        double accumulated;     ///< Exponentially weighted average of the access count
    } kickReads;  ///< Kickstart ROM reads
    struct
    {
        isize raw;              ///< Number of accesses since the last update
        double accumulated;     ///< Exponentially weighted average of the access count
    } kickWrites;  ///< Kickstart ROM writes
}
MemMetrics;

}
