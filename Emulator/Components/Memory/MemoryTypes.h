// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"
#include "Reflection.h"

namespace vamiga {

/* Memory source identifiers. The identifiers are used in the mem source lookup
 * table to specify the source and target of a peek or poke operation,
 * respectively.
 */
enum class MemorySource : long
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

struct MemorySourceEnum : util::Reflection<MemorySourceEnum, MemorySource>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = (long)MemorySource::EXT;
    
    // static const char *prefix() { return "MEM"; }
    static const char *_key(long value) { return _key(MemorySource(value)); }
    static const char *_key(MemorySource value)
    {
        switch (value) {
                
            case MemorySource::NONE:           return "NONE";
            case MemorySource::CHIP:           return "CHIP";
            case MemorySource::CHIP_MIRROR:    return "CHIP_MIRROR";
            case MemorySource::SLOW:           return "SLOW";
            case MemorySource::SLOW_MIRROR:    return "SLOW_MIRROR";
            case MemorySource::FAST:           return "FAST";
            case MemorySource::CIA:            return "CIA";
            case MemorySource::CIA_MIRROR:     return "CIA_MIRROR";
            case MemorySource::RTC:            return "RTC";
            case MemorySource::CUSTOM:         return "CUSTOM";
            case MemorySource::CUSTOM_MIRROR:  return "CUSTOM_MIRROR";
            case MemorySource::AUTOCONF:       return "AUTOCONF";
            case MemorySource::ZOR:            return "ZOR";
            case MemorySource::ROM:            return "ROM";
            case MemorySource::ROM_MIRROR:     return "ROM_MIRROR";
            case MemorySource::WOM:            return "WOM";
            case MemorySource::EXT:            return "EXT";
        }
        return "???";
    }
    static const char *help(long value) { return help(MemorySource(value)); }
    static const char *help(MemorySource value)
    {
        switch (value) {
                
            case MemorySource::NONE:           return "Unmapped";
            case MemorySource::CHIP:           return "Chip RAM";
            case MemorySource::CHIP_MIRROR:    return "Chip RAM mirror";
            case MemorySource::SLOW:           return "Slow RAM";
            case MemorySource::SLOW_MIRROR:    return "Slow RAM mirror";
            case MemorySource::FAST:           return "Fast RAM";
            case MemorySource::CIA:            return "CIA";
            case MemorySource::CIA_MIRROR:     return "CIA mirror";
            case MemorySource::RTC:            return "Real-time clock";
            case MemorySource::CUSTOM:         return "Custom chips";
            case MemorySource::CUSTOM_MIRROR:  return "Custom chips mirror";
            case MemorySource::AUTOCONF:       return "Auto config";
            case MemorySource::ZOR:            return "Zorro boards";
            case MemorySource::ROM:            return "Kickstart ROM";
            case MemorySource::ROM_MIRROR:     return "Kickstart ROM mirror";
            case MemorySource::WOM:            return "Write-only memory";
            case MemorySource::EXT:            return "Extension ROM";
        }
        return "???";
    }
};

enum_long(ACCESSOR_TYPE)
{
    ACCESSOR_CPU,
    ACCESSOR_AGNUS
};
typedef ACCESSOR_TYPE Accessor;

struct AccessorEnum : util::Reflection<AccessorEnum, Accessor>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = ACCESSOR_AGNUS;
    
    static const char *prefix() { return "ACCESSOR"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case ACCESSOR_CPU:    return "CPU";
            case ACCESSOR_AGNUS:  return "AGNUS";
        }
        return "???";
    }
    static const char *help(long value)
    {
        return "";
    }
};

enum_long(BANK_MAP)
{
    BANK_MAP_A500,
    BANK_MAP_A1000,
    BANK_MAP_A2000A,
    BANK_MAP_A2000B
};
typedef BANK_MAP BankMap;

struct BankMapEnum : util::Reflection<BankMapEnum, BankMap>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = BANK_MAP_A2000B;
    
    static const char *prefix() { return "BANK_MAP"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case BANK_MAP_A500:    return "A500";
            case BANK_MAP_A1000:   return "A1000";
            case BANK_MAP_A2000A:  return "A2000A";
            case BANK_MAP_A2000B:  return "A2000B";
        }
        return "???";
    }
    static const char *help(long value)
    {
        switch (value) {
                
            case BANK_MAP_A500:    return "Amiga 500 memory layout";
            case BANK_MAP_A1000:   return "Amiga 1000 memory layout";
            case BANK_MAP_A2000A:  return "Amiga 2000A memory layout";
            case BANK_MAP_A2000B:  return "Amiga 2000B memory layout";
        }
        return "???";
    }
};

enum_long(RAM_INIT_PATTERN)
{
    RAM_INIT_ALL_ZEROES,
    RAM_INIT_ALL_ONES,
    RAM_INIT_RANDOMIZED
};
typedef RAM_INIT_PATTERN RamInitPattern;

struct RamInitPatternEnum : util::Reflection<RamInitPatternEnum, RamInitPattern>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = RAM_INIT_RANDOMIZED;
    
    static const char *prefix() { return "RAM_INIT"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case RAM_INIT_ALL_ZEROES:  return "ALL_ZEROES";
            case RAM_INIT_ALL_ONES:    return "ALL_ONES";
            case RAM_INIT_RANDOMIZED:  return "RANDOMIZED";
        }
        return "???";
    }
    static const char *help(long value)
    {
        return "";
    }
};

enum_long(UNMAPPED_MEMORY)
{
    UNMAPPED_FLOATING,
    UNMAPPED_ALL_ZEROES,
    UNMAPPED_ALL_ONES
};
typedef UNMAPPED_MEMORY UnmappedMemory;

struct UnmappedMemoryEnum : util::Reflection<UnmappedMemoryEnum, UnmappedMemory>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = UNMAPPED_ALL_ONES;
    
    static const char *prefix() { return "UNMAPPED"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case UNMAPPED_FLOATING:    return "FLOATING";
            case UNMAPPED_ALL_ZEROES:  return "ALL_ZEROES";
            case UNMAPPED_ALL_ONES:    return "ALL_ONES";
        }
        return "???";
    }
    static const char *help(long value)
    {
        return "";
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
    
    MemorySource cpuMemSrc[256];
    MemorySource agnusMemSrc[256];
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
