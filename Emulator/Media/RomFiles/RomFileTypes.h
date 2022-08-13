// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(ROM_IDENTIFIER)
{
    ROM_MISSING,
    ROM_UNKNOWN,

    // Boot Roms (A1000)
    ROM_BOOT_A1000_8K,
    ROM_BOOT_A1000_64K,

    // Kickstart V0.x
    ROM_KICK07_27_003_BETA,

    // Kickstart V1.x
    ROM_KICK10_30_NTSC,
    ROM_KICK11_31_034_NTSC,
    ROM_KICK11_32_034_PAL,
    ROM_KICK12_33_166,
    ROM_KICK12_33_180,
    ROM_KICK121_34_004,
    ROM_KICK13_34_005_A500,
    ROM_KICK13_34_005_A3000,

    // Expansion lib patches
    ROM_KICK12_33_180_MRAS,

    // Guardian patches
    ROM_KICK12_33_180_G11R,
    ROM_KICK13_34_005_G12R,

    // Kickstart V2.x
    ROM_KICK20_36_028,
    ROM_KICK202_36_207_A3000,
    ROM_KICK204_37_175_A500,
    ROM_KICK204_37_175_A3000,
    ROM_KICK205_37_299_A600,
    ROM_KICK205_37_300_A600HD,
    ROM_KICK205_37_350_A600HD,

    // Kickstart V3.x
    ROM_KICK30_39_106_A1200,
    ROM_KICK30_39_106_A4000,
    ROM_KICK31_40_063_A500,
    ROM_KICK31_40_063_A500_R,
    ROM_KICK31_40_068_A1200,
    ROM_KICK31_40_068_A3000,
    ROM_KICK31_40_068_A4000,
    ROM_KICK31_40_070_A4000T,

    // Hyperion
    ROM_HYP314_46_143_A500,
    ROM_HYP314_46_143_A1200,
    ROM_HYP314_46_143_A2000,
    ROM_HYP314_46_143_A3000,
    ROM_HYP314_46_143_A4000,
    ROM_HYP314_46_143_A4000T,
    ROM_HYP320_47_96_A500,
    ROM_HYP320_47_96_A1200,
    ROM_HYP320_47_96_A3000,
    ROM_HYP320_47_96_A4000,
    ROM_HYP320_47_96_A4000T,
    ROM_HYP321_47_102_A500,
    ROM_HYP321_47_102_A1200,
    ROM_HYP321_47_102_A3000,
    ROM_HYP321_47_102_A4000,
    ROM_HYP321_47_102_A4000T,

    // Free Kickstart Rom replacements
    ROM_AROS_54705,
    ROM_AROS_54705_EXT,
    ROM_AROS_55696,
    ROM_AROS_55696_EXT,
    ROM_AROS_1ED13DE6E3,
    ROM_AROS_1ED13DE6E3_EXT,

    // Diagnostic cartridges
    ROM_DIAG11,
    ROM_DIAG12,
    ROM_DIAG121,
    ROM_LOGICA20
};
typedef ROM_IDENTIFIER RomIdentifier;

#ifdef __cplusplus
struct RomIdentifierEnum : util::Reflection<RomIdentifierEnum, RomIdentifier>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = ROM_LOGICA20;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "ROM"; }
    static const char *key(RomIdentifier value)
    {
        switch (value) {

            case ROM_MISSING:               return "MISSING";
            case ROM_UNKNOWN:               return "UNKNOWN";

            case ROM_BOOT_A1000_8K:         return "BOOT_A1000_8K";
            case ROM_BOOT_A1000_64K:        return "BOOT_A1000_64K";

            case ROM_KICK07_27_003_BETA:    return "KICK07_27_003";
            case ROM_KICK10_30_NTSC:        return "KICK10_30";
            case ROM_KICK11_31_034_NTSC:    return "KICK11_31_034";
            case ROM_KICK11_32_034_PAL:     return "KICK11_32_034";
            case ROM_KICK12_33_166:         return "KICK12_33_166";
            case ROM_KICK12_33_180:         return "KICK12_33_180";
            case ROM_KICK121_34_004:        return "KICK121_34_004";
            case ROM_KICK13_34_005_A500:
            case ROM_KICK13_34_005_A3000:   return "KICK13_34_005";

            case ROM_KICK12_33_180_MRAS:    return "KICK12_33_180_MRAS";

            case ROM_KICK12_33_180_G11R:    return "ROM_KICK12_33_180_G11R";
            case ROM_KICK13_34_005_G12R:    return "ROM_KICK13_34_005_G12R";

            case ROM_KICK20_36_028:         return "KICK20_36_028";
            case ROM_KICK202_36_207_A3000:  return "KICK202_36_207";
            case ROM_KICK204_37_175_A500:
            case ROM_KICK204_37_175_A3000:  return "KICK204_37_175";
            case ROM_KICK205_37_299_A600:   return "KICK205_37_299";
            case ROM_KICK205_37_300_A600HD: return "KICK205_37_300";
            case ROM_KICK205_37_350_A600HD: return "KICK205_37_350";

            case ROM_KICK30_39_106_A1200:
            case ROM_KICK30_39_106_A4000:   return "KICK30_39_106";
            case ROM_KICK31_40_063_A500:
            case ROM_KICK31_40_063_A500_R:  return "KICK31_40_063";
            case ROM_KICK31_40_068_A1200:
            case ROM_KICK31_40_068_A3000:
            case ROM_KICK31_40_068_A4000:   return "KICK31_40_068";
            case ROM_KICK31_40_070_A4000T:  return "KICK31_40_070";

            case ROM_HYP314_46_143_A500:
            case ROM_HYP314_46_143_A1200:
            case ROM_HYP314_46_143_A2000:
            case ROM_HYP314_46_143_A3000:
            case ROM_HYP314_46_143_A4000:
            case ROM_HYP314_46_143_A4000T:  return "HYP314_46_143";
            case ROM_HYP320_47_96_A500:
            case ROM_HYP320_47_96_A1200:
            case ROM_HYP320_47_96_A3000:
            case ROM_HYP320_47_96_A4000:
            case ROM_HYP320_47_96_A4000T:   return "HYP320_47_96";
            case ROM_HYP321_47_102_A500:
            case ROM_HYP321_47_102_A1200:
            case ROM_HYP321_47_102_A3000:
            case ROM_HYP321_47_102_A4000:
            case ROM_HYP321_47_102_A4000T:  return "HYP321_47_102";

            case ROM_AROS_54705:            return "AROS_54705";
            case ROM_AROS_54705_EXT:        return "AROS_54705_EXT";
            case ROM_AROS_55696:            return "AROS_55696";
            case ROM_AROS_55696_EXT:        return "AROS_55696_EXT";
            case ROM_AROS_1ED13DE6E3:       return "AROS_1ED13DE6E3";
            case ROM_AROS_1ED13DE6E3_EXT:   return "AROS_1ED13DE6E3_EXT";

            case ROM_DIAG11:                return "DIAG11";
            case ROM_DIAG12:                return "DIAG12";
            case ROM_DIAG121:               return "DIAG121";
            case ROM_LOGICA20:              return "LOGICA20";
        }
        return "???";
    }
};
#endif
