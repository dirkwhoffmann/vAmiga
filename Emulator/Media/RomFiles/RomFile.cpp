// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RomFile.h"
#include "IOUtils.h"
#include "MemUtils.h"

namespace vamiga {

//
// Boot Roms
//

const u8 RomFile::bootRomHeaders[1][8] = {

    // Amiga 1000 Bootstrap (1985)
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xF8, 0x00, 0x8A }
};

//
// Kickstart Roms
//

const u8 RomFile::kickRomHeaders[8][7] = {

    // AROS Kickstart replacement
    { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },

    // Kickstart 0.7, 1.0, 1.1, 1.2 and 1.3
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xFC, 0x00 },

    // Kickstart 1.3 (Guardian)
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xFE, 0x09 },

    // Kickstart 2.04, 3.1
    { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },

    // Kickstart relocation patches
    { 0x11, 0x16, 0x4E, 0xF9, 0x00, 0x20, 0x00 },

    // Diagnostic v2.0 (Logica)
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xF8, 0x04 },

    // QDOS Classic (experimental)
    { 0x00, 0x02, 0x84, 0x80, 0x00, 0xFC, 0x00 }
};

//
// Encrypted Kickstart Roms
//

const u8 RomFile::encrRomHeaders[1][11] = {

    // Cloanto Rom Header Signature
    { 'A', 'M', 'I', 'R', 'O', 'M', 'T', 'Y', 'P', 'E', '1' }
};

bool
RomFile::isCompatible(const string &path)
{
    return true;
}

bool
RomFile::isBootRom(u32 crc32)
{
    switch (crc32) {

        case CRC32_BOOT_A1000_8K:
        case CRC32_BOOT_A1000_64K: return true;

        default: return false;
    }
}

bool
RomFile::isArosRom(u32 crc32)
{
    switch (crc32) {

        case 0x00000000: return CRC32_MISSING;

        case CRC32_AROS_54705:
        case CRC32_AROS_54705_EXT:
        case CRC32_AROS_55696:
        case CRC32_AROS_55696_EXT:
        case CRC32_AROS_1ED13DE6E3:
        case CRC32_AROS_1ED13DE6E3_EXT: return true;

        default: return false;
    }
}

bool
RomFile::isDiagRom(u32 crc32)
{
    switch (crc32) {

        case CRC32_DIAG11:
        case CRC32_DIAG12:
        case CRC32_DIAG121:
        case CRC32_LOGICA20: return true;

        default: return false;
    }
}

bool
RomFile::isCommodoreRom(u32 crc32)
{
    switch (crc32) {

        case CRC32_BOOT_A1000_8K:
        case CRC32_BOOT_A1000_64K:

        case CRC32_KICK07_27_003_BETA:

        case CRC32_KICK10_30_NTSC:
        case CRC32_KICK11_31_034_NTSC:
        case CRC32_KICK11_32_034_PAL:
        case CRC32_KICK12_33_166:
        case CRC32_KICK12_33_180:
        case CRC32_KICK121_34_004:
        case CRC32_KICK13_34_005_A500:
        case CRC32_KICK13_34_005_A3000:

        case CRC32_KICK12_33_180_MRAS:

        case CRC32_KICK20_36_028:
        case CRC32_KICK202_36_207_A3000:
        case CRC32_KICK204_37_175_A500:
        case CRC32_KICK204_37_175_A3000:
        case CRC32_KICK205_37_299_A600:
        case CRC32_KICK205_37_300_A600HD:
        case CRC32_KICK205_37_350_A600HD:

        case CRC32_KICK30_39_106_A1200:
        case CRC32_KICK30_39_106_A4000:
        case CRC32_KICK31_40_063_A500:
        case CRC32_KICK31_40_068_A1200:
        case CRC32_KICK31_40_068_A3000:
        case CRC32_KICK31_40_068_A4000:
        case CRC32_KICK31_40_070_A4000T: return true;

        default: return false;
    }
}

bool
RomFile::isHyperionRom(u32 crc32)
{
    switch (crc32) {

        case CRC32_HYP314_46_143_A500:
        case CRC32_HYP314_46_143_A1200:
        case CRC32_HYP314_46_143_A2000:
        case CRC32_HYP314_46_143_A3000:
        case CRC32_HYP314_46_143_A4000:
        case CRC32_HYP314_46_143_A4000T:
        case CRC32_HYP320_47_96_A500:
        case CRC32_HYP320_47_96_A1200:
        case CRC32_HYP320_47_96_A3000:
        case CRC32_HYP320_47_96_A4000:
        case CRC32_HYP320_47_96_A4000T:
        case CRC32_HYP321_47_102_A500:
        case CRC32_HYP321_47_102_A1200:
        case CRC32_HYP321_47_102_A3000:
        case CRC32_HYP321_47_102_A4000:
        case CRC32_HYP321_47_102_A4000T:
        case CRC32_HYP322_47_111_A500:
        case CRC32_HYP322_47_111_A1200:
        case CRC32_HYP322_47_111_A3000:
        case CRC32_HYP322_47_111_A4000:
        case CRC32_HYP322_47_111_A4000T: return true;

        default: return false;
    }
}

bool
RomFile::isPatchedRom(u32 crc32)
{
    switch (crc32) {

        case CRC32_KICK12_33_180_G11R:
        case CRC32_KICK13_34_005_G12R: return true;

        default: return false;
    }
}

const char *
RomFile::shortName(u32 crc32)
{
    static char str[32];

    switch (crc32) {

        case CRC32_MISSING:                 return "";

        case CRC32_BOOT_A1000_8K:           return "A1000 Boot Rom 8KB";
        case CRC32_BOOT_A1000_64K:          return "A1000 Boot Rom 64KB";

        case CRC32_KICK07_27_003_BETA:      return "Kickstart 0.7 27.003 NTSC";
        case CRC32_KICK10_30_NTSC:          return "Kickstart 1.0 30.000 NTSC";
        case CRC32_KICK11_31_034_NTSC:      return "Kickstart 1.1 31.034 NTSC";
        case CRC32_KICK11_32_034_PAL:       return "Kickstart 1.1 31.034 PAL";
        case CRC32_KICK12_33_166:           return "Kickstart 1.2 33.166";
        case CRC32_KICK12_33_180:           return "Kickstart 1.2 33.180";
        case CRC32_KICK121_34_004:          return "Kickstart 1.21 34.004";
        case CRC32_KICK13_34_005_A500:      return "Kickstart 1.3 34.005 A500";
        case CRC32_KICK13_34_005_A3000:     return "Kickstart 1.3 34.005 A3000";

        case CRC32_KICK12_33_180_MRAS:      return "Kickstart 1.2 33.180 MRAS";

        case CRC32_KICK12_33_180_G11R:      return "Kickstart 1.2 33.180 G11R";
        case CRC32_KICK13_34_005_G12R:      return "Kickstart 1.3 34.005 G12R";

        case CRC32_KICK20_36_028:           return "Kickstart 2.0 36.028";
        case CRC32_KICK202_36_207_A3000:    return "Kickstart 2.02 36.207 A3000";
        case CRC32_KICK204_37_175_A500:     return "Kickstart 2.04 37.175 A500";
        case CRC32_KICK204_37_175_A3000:    return "Kickstart 2.04 37.175 A3000";
        case CRC32_KICK205_37_299_A600:     return "Kickstart 2.05 37.299 A600";
        case CRC32_KICK205_37_300_A600HD:   return "Kickstart 2.04 37.300 A600";
        case CRC32_KICK205_37_350_A600HD:   return "Kickstart 2.05 37.350 A600";

        case CRC32_KICK30_39_106_A1200:     return "Kickstart 3.0 39.106 A1200";
        case CRC32_KICK30_39_106_A4000:     return "Kickstart 3.0 39.106 A4000";
        case CRC32_KICK31_40_063_A500:      return "Kickstart 3.1 40.063 A500";
        case CRC32_KICK31_40_063_A500_R:    return "Kickstart 3.1 40.063 A500R";
        case CRC32_KICK31_40_068_A1200:     return "Kickstart 3.1 40.068 A1200";
        case CRC32_KICK31_40_068_A3000:     return "Kickstart 3.1 40.068 A3000";
        case CRC32_KICK31_40_068_A4000:     return "Kickstart 3.1 40.068 A4000";
        case CRC32_KICK31_40_070_A4000T:    return "Kickstart 3.1 40.070 A4000T";

        case CRC32_HYP314_46_143_A500:      return "Hyperion 3.1.4 46.143 A500";
        case CRC32_HYP314_46_143_A1200:     return "Hyperion 3.1.4 46.143 A1200";
        case CRC32_HYP314_46_143_A2000:     return "Hyperion 3.1.4 46.143 A2000";
        case CRC32_HYP314_46_143_A3000:     return "Hyperion 3.1.4 46.143 A3000";
        case CRC32_HYP314_46_143_A4000:     return "Hyperion 3.1.4 46.143 A4000";
        case CRC32_HYP314_46_143_A4000T:    return "Hyperion 3.1.4 46.143 A4000T";
        case CRC32_HYP320_47_96_A500:       return "Hyperion 3.2 47.96 A500";
        case CRC32_HYP320_47_96_A1200:      return "Hyperion 3.2 47.96 A1200";
        case CRC32_HYP320_47_96_A3000:      return "Hyperion 3.2 47.96 A3000";
        case CRC32_HYP320_47_96_A4000:      return "Hyperion 3.2 47.96 A4000";
        case CRC32_HYP320_47_96_A4000T:     return "Hyperion 3.2 47.96 A4000T";
        case CRC32_HYP321_47_102_A500:      return "Hyperion 3.2.1 47.102 A500";
        case CRC32_HYP321_47_102_A1200:     return "Hyperion 3.2.1 47.102 A1200";
        case CRC32_HYP321_47_102_A3000:     return "Hyperion 3.2.1 47.102 A3000";
        case CRC32_HYP321_47_102_A4000:     return "Hyperion 3.2.1 47.102 A4000";
        case CRC32_HYP321_47_102_A4000T:    return "Hyperion 3.2.1 47.102 A4000T";
        case CRC32_HYP322_47_111_A500:      return "Hyperion 3.2.2 47.111 A500";
        case CRC32_HYP322_47_111_A1200:     return "Hyperion 3.2.2 47.111 A1200";
        case CRC32_HYP322_47_111_A3000:     return "Hyperion 3.2.2 47.111 A3000";
        case CRC32_HYP322_47_111_A4000:     return "Hyperion 3.2.2 47.111 A4000";
        case CRC32_HYP322_47_111_A4000T:    return "Hyperion 3.2.2 47.111 A4000T";

        case CRC32_AROS_54705:              return "AROS SVN 54705";
        case CRC32_AROS_54705_EXT:          return "AROS SVN 54705 Extension";
        case CRC32_AROS_55696:              return "AROS SVN 55696";
        case CRC32_AROS_55696_EXT:          return "AROS SVN 55696 Extension";
        case CRC32_AROS_1ED13DE6E3:         return "AROS 1ed13de6e3";
        case CRC32_AROS_1ED13DE6E3_EXT:     return "AROS 1ed13de6e3 Extension";

        case CRC32_DIAG11:                  return "DiagROM 1.1";
        case CRC32_DIAG12:                  return "DiagROM 1.2";
        case CRC32_DIAG121:                 return "DiagROM 1.2.1";
        case CRC32_LOGICA20:                return "Logica Diagnostic 2.0";

        default:
            snprintf(str, sizeof(str), "CRC %x", crc32);
            return str;
    }
}

const char *
RomFile::title(u32 crc32)
{
    switch (crc32) {

        case CRC32_MISSING:                 return "";

        case CRC32_BOOT_A1000_8K:
        case CRC32_BOOT_A1000_64K:          return "Amiga 1000 Boot Rom";

        case CRC32_KICK07_27_003_BETA:      return "Kickstart 0.7 Beta";
        case CRC32_KICK10_30_NTSC:          return "Kickstart 1.0";
        case CRC32_KICK11_31_034_NTSC:
        case CRC32_KICK11_32_034_PAL:       return "Kickstart 1.1";
        case CRC32_KICK12_33_166:
        case CRC32_KICK12_33_180:           return "Kickstart 1.2";
        case CRC32_KICK121_34_004:          return "Kickstart 1.21";
        case CRC32_KICK13_34_005_A500:
        case CRC32_KICK13_34_005_A3000:     return "Kickstart 1.3";

        case CRC32_KICK12_33_180_MRAS:      return "Kickstart 1.2";
            
        case CRC32_KICK12_33_180_G11R:      return "Kickstart 1.2";
        case CRC32_KICK13_34_005_G12R:      return "Kickstart 1.3";

        case CRC32_KICK20_36_028:           return "Kickstart 2.0";
        case CRC32_KICK202_36_207_A3000:    return "Kickstart 2.02";
        case CRC32_KICK204_37_175_A500:
        case CRC32_KICK204_37_175_A3000:    return "Kickstart 2.04";
        case CRC32_KICK205_37_299_A600:
        case CRC32_KICK205_37_300_A600HD:
        case CRC32_KICK205_37_350_A600HD:   return "Kickstart 2.05";

        case CRC32_KICK30_39_106_A1200:
        case CRC32_KICK30_39_106_A4000:     return "Kickstart 3.0";
        case CRC32_KICK31_40_063_A500:
        case CRC32_KICK31_40_063_A500_R:
        case CRC32_KICK31_40_068_A1200:
        case CRC32_KICK31_40_068_A3000:
        case CRC32_KICK31_40_068_A4000:
        case CRC32_KICK31_40_070_A4000T:    return "Kickstart 3.1";

        case CRC32_HYP314_46_143_A500:
        case CRC32_HYP314_46_143_A1200:
        case CRC32_HYP314_46_143_A2000:
        case CRC32_HYP314_46_143_A3000:
        case CRC32_HYP314_46_143_A4000:
        case CRC32_HYP314_46_143_A4000T:    return "Kickstart 3.1.4 (Hyperion)";
        case CRC32_HYP320_47_96_A500:
        case CRC32_HYP320_47_96_A1200:
        case CRC32_HYP320_47_96_A3000:
        case CRC32_HYP320_47_96_A4000:
        case CRC32_HYP320_47_96_A4000T:     return "Kickstart 3.2 (Hyperion)";
        case CRC32_HYP321_47_102_A500:
        case CRC32_HYP321_47_102_A1200:
        case CRC32_HYP321_47_102_A3000:
        case CRC32_HYP321_47_102_A4000:
        case CRC32_HYP321_47_102_A4000T:    return "Kickstart 3.2.1 (Hyperion)";
        case CRC32_HYP322_47_111_A500:
        case CRC32_HYP322_47_111_A1200:
        case CRC32_HYP322_47_111_A3000:
        case CRC32_HYP322_47_111_A4000:
        case CRC32_HYP322_47_111_A4000T:    return "Kickstart 3.2.2 (Hyperion)";

        case CRC32_AROS_54705:
        case CRC32_AROS_55696:
        case CRC32_AROS_1ED13DE6E3:         return "AROS Kickstart replacement";
        case CRC32_AROS_54705_EXT:
        case CRC32_AROS_55696_EXT:
        case CRC32_AROS_1ED13DE6E3_EXT:     return "AROS Kickstart extension";

        case CRC32_DIAG11:
        case CRC32_DIAG12:
        case CRC32_DIAG121:                 return "Amiga DiagROM";
        case CRC32_LOGICA20:                return "Logica Diagnostic";

        default:                            return "Unknown or patched Rom";
    }
}

const char *
RomFile::version(u32 crc32)
{
    static char str[32];
    
    switch (crc32) {

        case CRC32_MISSING:                 return "";

        case CRC32_BOOT_A1000_8K:           return "8KB";
        case CRC32_BOOT_A1000_64K:          return "64KB";

        case CRC32_KICK07_27_003_BETA:      return "Rev 27.003 NTSC";
        case CRC32_KICK10_30_NTSC:          return "Rev 30.000 NTSC";
        case CRC32_KICK11_31_034_NTSC:      return "Rev 31.034 NTSC";
        case CRC32_KICK11_32_034_PAL:       return "Rev 32.034 PAL";
        case CRC32_KICK12_33_166:           return "Rev 33.166";
        case CRC32_KICK12_33_180:           return "Rev 33.180";
        case CRC32_KICK121_34_004:          return "Rev 34.004";
        case CRC32_KICK13_34_005_A500:
        case CRC32_KICK13_34_005_A3000:     return "Rev 34.005";

        case CRC32_KICK12_33_180_MRAS:      return "Rev 33.180";

        case CRC32_KICK12_33_180_G11R:      return "Rev 33.180 (Guardian patch)";
        case CRC32_KICK13_34_005_G12R:      return "Rev 34.005 (Guardian patch)";

        case CRC32_KICK20_36_028:           return "Rev 36.028";
        case CRC32_KICK202_36_207_A3000:    return "Rev 36.207";
        case CRC32_KICK204_37_175_A500:
        case CRC32_KICK204_37_175_A3000:    return "Rev 37.175";
        case CRC32_KICK205_37_299_A600:     return "Rev 37.299";
        case CRC32_KICK205_37_300_A600HD:   return "Rev 37.300";
        case CRC32_KICK205_37_350_A600HD:   return "Rev 37.350";

        case CRC32_KICK30_39_106_A1200:
        case CRC32_KICK30_39_106_A4000:     return "Rev 39.106";
        case CRC32_KICK31_40_063_A500:      return "Rev 40.063";
        case CRC32_KICK31_40_063_A500_R:    return "Rev 40.063 (patched)";
        case CRC32_KICK31_40_068_A1200:
        case CRC32_KICK31_40_068_A3000:
        case CRC32_KICK31_40_068_A4000:     return "Rev 40.068";
        case CRC32_KICK31_40_070_A4000T:    return "Rev 40.070";

        case CRC32_HYP314_46_143_A500:
        case CRC32_HYP314_46_143_A1200:
        case CRC32_HYP314_46_143_A2000:
        case CRC32_HYP314_46_143_A3000:
        case CRC32_HYP314_46_143_A4000:
        case CRC32_HYP314_46_143_A4000T:    return "Rev 46.143";
        case CRC32_HYP320_47_96_A500:
        case CRC32_HYP320_47_96_A1200:
        case CRC32_HYP320_47_96_A3000:
        case CRC32_HYP320_47_96_A4000:
        case CRC32_HYP320_47_96_A4000T:     return "Rev 47.96";
        case CRC32_HYP321_47_102_A500:
        case CRC32_HYP321_47_102_A1200:
        case CRC32_HYP321_47_102_A3000:
        case CRC32_HYP321_47_102_A4000:
        case CRC32_HYP321_47_102_A4000T:    return "Rev 47.102";
        case CRC32_HYP322_47_111_A500:
        case CRC32_HYP322_47_111_A1200:
        case CRC32_HYP322_47_111_A3000:
        case CRC32_HYP322_47_111_A4000:
        case CRC32_HYP322_47_111_A4000T:    return "Rev 47.111";

        case CRC32_AROS_54705:
        case CRC32_AROS_54705_EXT:          return "SVN 54705";
        case CRC32_AROS_55696:
        case CRC32_AROS_55696_EXT:          return "SVN 55696";
        case CRC32_AROS_1ED13DE6E3:
        case CRC32_AROS_1ED13DE6E3_EXT:     return "Version 1ed13de6e3";

        case CRC32_DIAG11:                  return "Version 1.1";
        case CRC32_DIAG12:                  return "Version 1.2";
        case CRC32_DIAG121:                 return "Version 1.2.1";
        case CRC32_LOGICA20:                return "Version 2.0";

        default:
            snprintf(str, sizeof(str), "CRC %x", crc32);
            return str;
    }
}

const char *
RomFile::released(u32 crc32)
{
    switch (crc32) {

        case CRC32_MISSING:                 return "";

        case CRC32_BOOT_A1000_8K:           return "1985";
        case CRC32_BOOT_A1000_64K:          return "1985";

        case CRC32_KICK07_27_003_BETA:      return "July 1985";
        case CRC32_KICK10_30_NTSC:          return "September 1985";
        case CRC32_KICK11_31_034_NTSC:      return "November 1985";
        case CRC32_KICK11_32_034_PAL:       return "February 1986";
        case CRC32_KICK12_33_166:           return "September 1986";
        case CRC32_KICK12_33_180:           return "October 1986";
        case CRC32_KICK121_34_004:          return "November 1987";
        case CRC32_KICK13_34_005_A500:      return "December 1987";
        case CRC32_KICK13_34_005_A3000:     return "December 1987";

        case CRC32_KICK12_33_180_MRAS:      return "2022";

        case CRC32_KICK12_33_180_G11R:      return "1988";
        case CRC32_KICK13_34_005_G12R:      return "1988";

        case CRC32_KICK20_36_028:           return "March 1990";
        case CRC32_KICK202_36_207_A3000:    return "October 1990";
        case CRC32_KICK204_37_175_A500:
        case CRC32_KICK204_37_175_A3000:    return "May 1991";
        case CRC32_KICK205_37_299_A600:     return "November 1991";
        case CRC32_KICK205_37_300_A600HD:   return "November 1991";
        case CRC32_KICK205_37_350_A600HD:   return "April 1992";

        case CRC32_KICK30_39_106_A1200:
        case CRC32_KICK30_39_106_A4000:     return "September 1992";
        case CRC32_KICK31_40_063_A500:      return "July 1993";
        case CRC32_KICK31_40_063_A500_R:    return "ReKick image";
        case CRC32_KICK31_40_068_A1200:
        case CRC32_KICK31_40_068_A3000:
        case CRC32_KICK31_40_068_A4000:     return "December 1993";
        case CRC32_KICK31_40_070_A4000T:    return "February 1994";

        case CRC32_HYP314_46_143_A500:
        case CRC32_HYP314_46_143_A1200:
        case CRC32_HYP314_46_143_A2000:
        case CRC32_HYP314_46_143_A3000:
        case CRC32_HYP314_46_143_A4000:
        case CRC32_HYP314_46_143_A4000T:    return "September 2018";
        case CRC32_HYP320_47_96_A500:
        case CRC32_HYP320_47_96_A1200:
        case CRC32_HYP320_47_96_A3000:
        case CRC32_HYP320_47_96_A4000:
        case CRC32_HYP320_47_96_A4000T:     return "May 2021";
        case CRC32_HYP321_47_102_A500:
        case CRC32_HYP321_47_102_A1200:
        case CRC32_HYP321_47_102_A3000:
        case CRC32_HYP321_47_102_A4000:
        case CRC32_HYP321_47_102_A4000T:    return "December 2021";
        case CRC32_HYP322_47_111_A500:
        case CRC32_HYP322_47_111_A1200:
        case CRC32_HYP322_47_111_A3000:
        case CRC32_HYP322_47_111_A4000:
        case CRC32_HYP322_47_111_A4000T:    return "March 2023";

        case CRC32_AROS_54705:              return "May 2017";
        case CRC32_AROS_54705_EXT:          return "May 2017";
        case CRC32_AROS_55696:              return "February 2019";
        case CRC32_AROS_55696_EXT:          return "February 2019";
        case CRC32_AROS_1ED13DE6E3:         return "September 2021";
        case CRC32_AROS_1ED13DE6E3_EXT:     return "September 2021";

        case CRC32_DIAG11:                  return "October 2018";
        case CRC32_DIAG12:                  return "August 2019";
        case CRC32_DIAG121:                 return "July 2020";
        case CRC32_LOGICA20:                return "";

        default:                            return "";
    }
}

const char *
RomFile::model(u32 crc32)
{
    switch (crc32) {

        case CRC32_MISSING:                 return "";

        case CRC32_BOOT_A1000_8K:
        case CRC32_BOOT_A1000_64K:          return "A1000";

        case CRC32_KICK07_27_003_BETA:
        case CRC32_KICK10_30_NTSC:
        case CRC32_KICK11_31_034_NTSC:
        case CRC32_KICK11_32_034_PAL:
        case CRC32_KICK12_33_166:           return "A1000";
        case CRC32_KICK12_33_180:           return "A500, A1000, A2000";
        case CRC32_KICK121_34_004:          return "???";
        case CRC32_KICK13_34_005_A500:      return "A500, A1000, A2000, CDTV";
        case CRC32_KICK13_34_005_A3000:     return "A3000";

        case CRC32_KICK12_33_180_MRAS:      return "MRAS patch";

        case CRC32_KICK12_33_180_G11R:      return "";
        case CRC32_KICK13_34_005_G12R:      return "";

        case CRC32_KICK20_36_028:           return "";
        case CRC32_KICK202_36_207_A3000:    return "A3000";
        case CRC32_KICK204_37_175_A500:     return "A500";
        case CRC32_KICK204_37_175_A3000:    return "A3000";
        case CRC32_KICK205_37_299_A600:     return "A600";
        case CRC32_KICK205_37_300_A600HD:
        case CRC32_KICK205_37_350_A600HD:   return "A600HD";

        case CRC32_KICK30_39_106_A1200:     return "A1200";
        case CRC32_KICK30_39_106_A4000:     return "A4000";
        case CRC32_KICK31_40_063_A500:      return "A500, A600, A2000";
        case CRC32_KICK31_40_063_A500_R:    return "";
        case CRC32_KICK31_40_068_A1200:     return "A1200";
        case CRC32_KICK31_40_068_A3000:     return "A3000";
        case CRC32_KICK31_40_068_A4000:     return "A4000";
        case CRC32_KICK31_40_070_A4000T:    return "A4000T";

        case CRC32_HYP314_46_143_A500:      return "A500, A600, A1000";
        case CRC32_HYP314_46_143_A1200:     return "A1200";
        case CRC32_HYP314_46_143_A2000:     return "A2000";
        case CRC32_HYP314_46_143_A3000:     return "A3000";
        case CRC32_HYP314_46_143_A4000:     return "A4000";
        case CRC32_HYP314_46_143_A4000T:    return "A4000T";
        case CRC32_HYP320_47_96_A500:       return "A500, A600, A1000, A2000, CDTV";
        case CRC32_HYP320_47_96_A1200:      return "A1200";
        case CRC32_HYP320_47_96_A3000:      return "A3000";
        case CRC32_HYP320_47_96_A4000:      return "A4000";
        case CRC32_HYP320_47_96_A4000T:     return "A4000T";
        case CRC32_HYP321_47_102_A500:      return "A500, A600, A1000, A2000, CDTV";
        case CRC32_HYP321_47_102_A1200:     return "A1200";
        case CRC32_HYP321_47_102_A3000:     return "A3000";
        case CRC32_HYP321_47_102_A4000:     return "A4000";
        case CRC32_HYP321_47_102_A4000T:    return "A4000T";
        case CRC32_HYP322_47_111_A500:      return "A500, A600, A1000, A2000, CDTV";
        case CRC32_HYP322_47_111_A1200:     return "A1200";
        case CRC32_HYP322_47_111_A3000:     return "A3000";
        case CRC32_HYP322_47_111_A4000:     return "A4000";
        case CRC32_HYP322_47_111_A4000T:    return "A4000T";


        case CRC32_AROS_54705:              return "UAE version";
        case CRC32_AROS_54705_EXT:          return "UAE version";
        case CRC32_AROS_55696:              return "SAE version";
        case CRC32_AROS_55696_EXT:          return "SAE version";
        case CRC32_AROS_1ED13DE6E3:         return "";
        case CRC32_AROS_1ED13DE6E3_EXT:     return "";

        case CRC32_DIAG11:                  return "";
        case CRC32_DIAG12:                  return "";
        case CRC32_DIAG121:                 return "";
        case CRC32_LOGICA20:                return "";

        default:                            return "";
    }
}

bool
RomFile::isCompatible(std::istream &stream)
{
    isize length = util::streamLength(stream);

    // Boot Roms
    if (length == KB(8) || length == KB(16)) {

        isize len = isizeof(bootRomHeaders[0]);
        isize cnt = isizeof(bootRomHeaders) / len;

        for (isize i = 0; i < cnt; i++) {
            if (util::matchingStreamHeader(stream, bootRomHeaders[i], len)) return true;
        }
        return false;
    }

    // Kickstart Roms
    if (length == KB(256) || length == KB(512)) {

        isize len = isizeof(kickRomHeaders[0]);
        isize cnt = isizeof(kickRomHeaders) / len;

        for (isize i = 0; i < cnt; i++) {
            if (util::matchingStreamHeader(stream, kickRomHeaders[i], len)) return true;
        }
        return false;
    }

    // Encrypted Kickstart Roms
    if (length == KB(256) + 11 || length == KB(512) + 11) {

        isize len = isizeof(encrRomHeaders[0]);
        isize cnt = isizeof(encrRomHeaders) / len;

        for (isize i = 0; i < cnt; i++) {
            if (util::matchingStreamHeader(stream, encrRomHeaders[i], len)) return true;
        }
    }

    return false;
}

bool
RomFile::isRomBuffer(const u8 *buf, isize len)
{
    std::stringstream stream;
    stream.write((const char *)buf, len);

    return isCompatible(stream);
}

bool
RomFile::isRomFile(const string &path)
{
    std::ifstream stream(path, std::ifstream::binary);
    return stream.is_open() ? isCompatible(stream) : false;
}

bool
RomFile::isEncrypted()
{
    return util::matchingBufferHeader(data.ptr, encrRomHeaders[0], sizeof(encrRomHeaders[0]));
}

void
RomFile::decrypt()
{
    const isize headerSize = 11;

    Buffer<u8> romKey;
    Buffer<u8> decrypted;
    
    // Only proceed if the file is encrypted
    if (!isEncrypted()) return;

    // Locate the rom.key file
    romKeyPath = util::extractPath(path) + "rom.key";

    // Load the rom.key file
    romKey.init(romKeyPath);
    if (romKey.empty()) throw VAError(ERROR_MISSING_ROM_KEY);
    
    // Decrypt
    decrypted.alloc(data.size - headerSize);
    for (isize i = 0, j = headerSize; j < data.size; i++, j++) {
        decrypted[i] = data[j] ^ romKey[i % romKey.size];
    }
    
    // Replace the old data by the decrypted data
    data.init(decrypted);
    
    // Check if we've got a valid ROM
    if (!isRomBuffer(data.ptr, data.size)) {
        throw VAError(ERROR_INVALID_ROM_KEY);
    }
}

}
