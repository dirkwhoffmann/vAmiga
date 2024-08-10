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

/* Memory source identifiers. The identifiers are used in the mem source lookup
 * table to specify the source and target of a peek or poke operation,
 * respectively.
 */
enum_long(MEM_SOURCE)
{
    MEM_NONE,
    MEM_CHIP,
    MEM_CHIP_MIRROR,
    MEM_SLOW,
    MEM_SLOW_MIRROR,
    MEM_FAST,
    MEM_CIA,
    MEM_CIA_MIRROR,
    MEM_RTC,
    MEM_CUSTOM,
    MEM_CUSTOM_MIRROR,
    MEM_AUTOCONF,
    MEM_ZOR,
    MEM_ROM,
    MEM_ROM_MIRROR,
    MEM_WOM,
    MEM_EXT
};
typedef MEM_SOURCE MemorySource;

#ifdef __cplusplus
struct MemorySourceEnum : vamiga::util::Reflection<MemorySourceEnum, MemorySource>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = MEM_EXT;

    static const char *prefix() { return "MEM"; }
    static const char *_key(long value)
    {
        switch (value) {
                
            case MEM_NONE:           return "NONE";
            case MEM_CHIP:           return "CHIP";
            case MEM_CHIP_MIRROR:    return "CHIP_MIRROR";
            case MEM_SLOW:           return "SLOW";
            case MEM_SLOW_MIRROR:    return "SLOW_MIRROR";
            case MEM_FAST:           return "FAST";
            case MEM_CIA:            return "CIA";
            case MEM_CIA_MIRROR:     return "CIA_MIRROR";
            case MEM_RTC:            return "RTC";
            case MEM_CUSTOM:         return "CUSTOM";
            case MEM_CUSTOM_MIRROR:  return "CUSTOM_MIRROR";
            case MEM_AUTOCONF:       return "AUTOCONF";
            case MEM_ZOR:            return "ZOR";
            case MEM_ROM:            return "ROM";
            case MEM_ROM_MIRROR:     return "ROM_MIRROR";
            case MEM_WOM:            return "WOM";
            case MEM_EXT:            return "EXT";
        }
        return "???";
    }
};
#endif

enum_long(ACCESSOR_TYPE)
{
    ACCESSOR_CPU,
    ACCESSOR_AGNUS
};
typedef ACCESSOR_TYPE Accessor;

#ifdef __cplusplus
struct AccessorEnum : vamiga::util::Reflection<AccessorEnum, Accessor>
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
};
#endif

enum_long(BANK_MAP)
{
    BANK_MAP_A500,
    BANK_MAP_A1000,
    BANK_MAP_A2000A,
    BANK_MAP_A2000B
};
typedef BANK_MAP BankMap;

#ifdef __cplusplus
struct BankMapEnum : vamiga::util::Reflection<BankMapEnum, BankMap>
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
};
#endif

enum_long(RAM_INIT_PATTERN)
{
    RAM_INIT_ALL_ZEROES,
    RAM_INIT_ALL_ONES,
    RAM_INIT_RANDOMIZED
};
typedef RAM_INIT_PATTERN RamInitPattern;

#ifdef __cplusplus
struct RamInitPatternEnum : vamiga::util::Reflection<RamInitPatternEnum, RamInitPattern>
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
};
#endif

enum_long(UNMAPPED_MEMORY)
{
    UNMAPPED_FLOATING,
    UNMAPPED_ALL_ZEROES,
    UNMAPPED_ALL_ONES
};
typedef UNMAPPED_MEMORY UnmappedMemory;

#ifdef __cplusplus
struct UnmappedMemoryEnum : vamiga::util::Reflection<UnmappedMemoryEnum, UnmappedMemory>
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
};
#endif

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
