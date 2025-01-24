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

struct MemSrcEnum : util::Reflection<MemSrcEnum, MemSrc>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = (long)MemSrc::EXT;
    
    // static const char *prefix() { return "MEM"; }
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

enum class Accessor : long
{
    CPU,
    AGNUS
};

struct AccessorEnum : util::Reflection<AccessorEnum, Accessor>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(Accessor::AGNUS);
    
    static const char *prefix() { return "ACCESSOR"; }
    static const char *_key(Accessor value)
    {
        switch (value) {
                
            case Accessor::CPU:    return "CPU";
            case Accessor::AGNUS:  return "AGNUS";
        }
        return "???";
    }
    static const char *help(Accessor value)
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
