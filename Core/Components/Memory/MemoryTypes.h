// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Infrastructure/Reflection.h"

namespace vamiga {

/* Memory source identifiers. The identifiers are used in the mem source lookup
 * table to specify the source and target of a peek or poke operation,
 * respectively.
 */
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

struct MemSrcEnum : Reflection<MemSrcEnum, MemSrc>
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

enum class Accessor : i8
{
    CPU,
    AGNUS,
    DENISE
};

struct AccessorEnum : Reflection<AccessorEnum, Accessor>
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

enum class BankMap : long
{
    A500,
    A1000,
    A2000A,
    A2000B
};

struct BankMapEnum : Reflection<BankMapEnum, BankMap>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(BankMap::A2000B);
    
    static const char *_key(BankMap value)
    {
        switch (value) {
                
            case BankMap::A500:    return "A500";
            case BankMap::A1000:   return "A1000";
            case BankMap::A2000A:  return "A2000A";
            case BankMap::A2000B:  return "A2000B";
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
        }
        return "???";
    }
};

enum class RamInitPattern
{
    ALL_ZEROES,
    ALL_ONES,
    RANDOMIZED
};

struct RamInitPatternEnum : Reflection<RamInitPatternEnum, RamInitPattern>
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

enum class UnmappedMemory
{
    FLOATING,
    ALL_ZEROES,
    ALL_ONES
};

struct UnmappedMemoryEnum : Reflection<UnmappedMemoryEnum, UnmappedMemory>
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

typedef struct
{
    // RAM size in bytes
    i32 chipSize;
    i32 slowSize;
    i32 fastSize;
    
    // ROM size in bytes
    i32 romSize;
    i32 womSize;
    i32 extSize;
    
    // First memory page where the extended ROM is blended it
    u32 extStart;
    
    // Indicates if snapshots should contain Roms
    bool saveRoms;
    
    // Indicates if slow Ram accesses need a free bus
    bool slowRamDelay;
    
    // Special ECS feature
    bool slowRamMirror;
    
    // Memory layout
    BankMap bankMap;
    
    // Ram contents on startup
    RamInitPattern ramInitPattern;
    
    // Specifies how to deal with unmapped memory
    UnmappedMemory unmappingType;
}
MemConfig;

typedef struct
{
    bool hasRom;
    bool hasWom;
    bool hasExt;
    bool hasBootRom;
    bool hasKickRom;
    bool womLock;
    
    u32 romMask;
    u32 womMask;
    u32 extMask;
    u32 chipMask;
    
    MemSrc cpuMemSrc[256];
    MemSrc agnusMemSrc[256];
}
MemInfo;

typedef struct
{
    struct { isize raw; double accumulated; } chipReads;
    struct { isize raw; double accumulated; } chipWrites;
    struct { isize raw; double accumulated; } slowReads;
    struct { isize raw; double accumulated; } slowWrites;
    struct { isize raw; double accumulated; } fastReads;
    struct { isize raw; double accumulated; } fastWrites;
    struct { isize raw; double accumulated; } kickReads;
    struct { isize raw; double accumulated; } kickWrites;
}
MemStats;

}
