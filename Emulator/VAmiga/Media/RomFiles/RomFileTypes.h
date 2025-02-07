// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "VAmiga/Foundation/Reflection.h"

namespace vamiga {

//
// Constants
//

static const u32 CRC32_MISSING                  = 0x00000000;
static const u32 CRC32_BOOT_A1000_8K            = 0x62F11C04;
static const u32 CRC32_BOOT_A1000_64K           = 0x0B1AD2D0;

static const u32 CRC32_KICK07_27_003_BETA       = 0x428A9A4B;
static const u32 CRC32_KICK10_30_NTSC           = 0x299790FF;
static const u32 CRC32_KICK11_31_034_NTSC       = 0xD060572A;
static const u32 CRC32_KICK11_32_034_PAL        = 0xEC86DAE2;
static const u32 CRC32_KICK12_33_166            = 0x9ED783D0;
static const u32 CRC32_KICK12_33_180            = 0xA6CE1636;
static const u32 CRC32_KICK121_34_004           = 0xDB4C8033;
static const u32 CRC32_KICK13_34_005_A500       = 0xC4F0F55F;
static const u32 CRC32_KICK13_34_005_A3000      = 0xE0F37258;

static const u32 CRC32_KICK12_33_180_MRAS       = 0xF80F0FC5;

static const u32 CRC32_KICK12_33_180_G11R       = 0x85067666;
static const u32 CRC32_KICK13_34_005_G12R       = 0x74680D37;

static const u32 CRC32_KICK20_36_028            = 0xB4113910;

static const u32 CRC32_KICK202_36_207_A3000     = 0x9A15519D;
static const u32 CRC32_KICK204_37_175_A500      = 0xC3BDB240;
static const u32 CRC32_KICK204_37_175_A3000     = 0x234A7233;
static const u32 CRC32_KICK205_37_299_A600      = 0x83028FB5;
static const u32 CRC32_KICK205_37_300_A600HD    = 0x64466C2A;
static const u32 CRC32_KICK205_37_350_A600HD    = 0x43B0DF7B;

static const u32 CRC32_KICK30_39_106_A1200      = 0x6C9B07D2;
static const u32 CRC32_KICK30_39_106_A4000      = 0x9E6AC152;
static const u32 CRC32_KICK31_40_063_A500       = 0xFC24AE0D;
static const u32 CRC32_KICK31_40_063_A500_R     = 0x88136CA9;
static const u32 CRC32_KICK31_40_068_A1200      = 0x1483A091;
static const u32 CRC32_KICK31_40_068_A3000      = 0xEFB239CC;
static const u32 CRC32_KICK31_40_068_A4000      = 0xD6BAE334;
static const u32 CRC32_KICK31_40_070_A4000T     = 0x75932C3A;

// static const u32 CRC32_HYP314_46_143_A500       = 0xD52B52FD;
static const u32 CRC32_HYP314_46_143_A500       = 0x568F8786;
static const u32 CRC32_HYP314_46_143_A1200      = 0xF17FA97F;
static const u32 CRC32_HYP314_46_143_A2000      = 0xC25939AC;
static const u32 CRC32_HYP314_46_143_A3000      = 0x50C3529C;
static const u32 CRC32_HYP314_46_143_A4000      = 0xD47E18FD;
static const u32 CRC32_HYP314_46_143_A4000T     = 0x75A2B2A5;
static const u32 CRC32_HYP320_47_96_A500        = 0x8173D7B6;
static const u32 CRC32_HYP320_47_96_A1200       = 0xBD1FF75E;
static const u32 CRC32_HYP320_47_96_A3000       = 0xF3AF46CC;
static const u32 CRC32_HYP320_47_96_A4000       = 0x9BB8FC93;
static const u32 CRC32_HYP320_47_96_A4000T      = 0x9188A509;
static const u32 CRC32_HYP321_47_102_A500       = 0x4F078456;
static const u32 CRC32_HYP321_47_102_A1200      = 0x2B653371;
static const u32 CRC32_HYP321_47_102_A3000      = 0x0078F607;
static const u32 CRC32_HYP321_47_102_A4000      = 0xF3CED3B8;
static const u32 CRC32_HYP321_47_102_A4000T     = 0xAF3452EC;
static const u32 CRC32_HYP322_47_111_A500       = 0xE4458462;
static const u32 CRC32_HYP322_47_111_A1200      = 0x5C40328A;
static const u32 CRC32_HYP322_47_111_A3000      = 0x46335B57;
static const u32 CRC32_HYP322_47_111_A4000      = 0x4BEA9798;
static const u32 CRC32_HYP322_47_111_A4000T     = 0x36BBCD8A;

static const u32 CRC32_AROS_54705               = 0x9CE0F009;
static const u32 CRC32_AROS_54705_EXT           = 0xE2C7F70A;
static const u32 CRC32_AROS_55696               = 0x3F4FCC0A;
static const u32 CRC32_AROS_55696_EXT           = 0xF2E52B07;
static const u32 CRC32_AROS_1ED13DE6E3          = 0x4CE7C8D6;
static const u32 CRC32_AROS_1ED13DE6E3_EXT      = 0xF2A9CDC5;

static const u32 CRC32_EMUTOS13                 = 0x7123C1C1;

static const u32 CRC32_DIAG11                   = 0x4C4B5C05;
static const u32 CRC32_DIAG12                   = 0x771CD0EA;
static const u32 CRC32_DIAG121                  = 0x850209CD;
static const u32 CRC32_DIAG13                   = 0x55E2E127;
static const u32 CRC32_LOGICA20                 = 0x8484F426;

static const u32 CRC32_CPUBLTRO_F8_0_3_2        = 0x169C8FCE;
static const u32 CRC32_CPUBLTRO_FC_0_3_2        = 0x86CB5B1B;

//
// Enumerations
//

enum class RomVendor
{
    COMMODORE,
    AROS,
    HYPERION,
    DEMO,
    DIAG,
    EMUTOS,
    OTHER
};

struct RomVendorEnum : Reflection<RomVendorEnum, RomVendor> {
    
    static constexpr long minVal = 0;
    static constexpr long maxVal = long(RomVendor::OTHER);
    static bool isValid(auto value) { return value >= minVal && value <= maxVal; }
    
    static const char *_key(RomVendor value)
    {
        switch (value) {
                
            case RomVendor::COMMODORE:  return "COMMODORE";
            case RomVendor::AROS:       return "AROS";
            case RomVendor::HYPERION:   return "HYPERION";
            case RomVendor::DEMO:       return "DEMO";
            case RomVendor::DIAG:       return "DIAG";
            case RomVendor::EMUTOS:     return "EMUTOS";
            case RomVendor::OTHER:      return "OTHER";
        }
        return "???";
    }
    static const char *help(RomVendor value)
    {
        return "";
    }
};


//
// Structures
//

typedef struct {
    
    u32 crc;
    
    const char *title;
    const char *revision;
    const char *released;
    const char *model;
    
    RomVendor vendor;
    bool boot;
    bool patched;
    bool relocated;
}
RomTraits;

}
