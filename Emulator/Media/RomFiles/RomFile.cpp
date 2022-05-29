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

const u8 RomFile::kickRomHeaders[7][7] = {

    // AROS Kickstart replacement
    { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },

    // Kickstart 0.7, 1.0, 1.1, 1.2 and 1.3
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xFC, 0x00 },

    // Kickstart 1.3 (Guardian)
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xFE, 0x09 },

    // Kickstart 2.04
    { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },

    // Kickstart 3.1
    { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },
    // { 0x11, 0x16, 0x4E, 0xF9, 0x00, 0x20, 0x00 }, not working

    // Diagnostic v2.0 (Logica)
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xF8, 0x04 }
};

//
// Encrypted Kickstart Roms
//

const u8 RomFile::encrRomHeaders[1][11] = {

    // Cloanto Rom Header Signature
    { 'A', 'M', 'I', 'R', 'O', 'M', 'T', 'Y', 'P', 'E', '1' }
};

RomIdentifier
RomFile::identifier(u32 fingerprint)
{
    switch(fingerprint) {

        case 0x00000000: return ROM_MISSING;

        case 0x62F11C04: return ROM_BOOT_A1000_8K;
        case 0x0B1AD2D0: return ROM_BOOT_A1000_64K;

        case 0x428A9A4B: return ROM_KICK07_27_003_BETA;
        case 0x299790FF: return ROM_KICK10_30_NTSC;
        case 0xD060572A: return ROM_KICK11_31_034_NTSC;
        case 0xEC86DAE2: return ROM_KICK11_32_034_PAL;
        case 0x9ED783D0: return ROM_KICK12_33_166;
        case 0xA6CE1636: return ROM_KICK12_33_180;
        case 0xDB4C8033: return ROM_KICK121_34_004;
        case 0xC4F0F55F: return ROM_KICK13_34_005_A500;
        case 0xE0F37258: return ROM_KICK13_34_005_A3000;

        case 0xF80F0FC5: return ROM_KICK12_33_180_MRAS;

        case 0x85067666: return ROM_KICK12_33_180_G11R;
        case 0x74680D37: return ROM_KICK13_34_005_G12R;

        case 0xB4113910: return ROM_KICK20_36_028;

        case 0x9A15519D: return ROM_KICK202_36_207_A3000;
        case 0xC3BDB240: return ROM_KICK204_37_175_A500;
        case 0x234A7233: return ROM_KICK204_37_175_A3000;
        case 0x83028FB5: return ROM_KICK205_37_299_A600;
        case 0x64466C2A: return ROM_KICK205_37_300_A600HD;
        case 0x43B0DF7B: return ROM_KICK205_37_350_A600HD;

        case 0x6C9B07D2: return ROM_KICK30_39_106_A1200;
        case 0x9E6AC152: return ROM_KICK30_39_106_A4000;
        case 0xFC24AE0D: return ROM_KICK31_40_063_A500;
        case 0x1483A091: return ROM_KICK31_40_068_A1200;
        case 0xEFB239CC: return ROM_KICK31_40_068_A3000;
        case 0xD6BAE334: return ROM_KICK31_40_068_A4000;
        case 0x75932C3A: return ROM_KICK31_40_070_A4000T;

        case 0xD52B52FD: return ROM_HYP314_46_143_A500;
        case 0xF17FA97F: return ROM_HYP314_46_143_A1200;
        case 0xC25939AC: return ROM_HYP314_46_143_A2000;
        case 0x50C3529C: return ROM_HYP314_46_143_A3000;
        case 0xD47E18FD: return ROM_HYP314_46_143_A4000;
        case 0x75A2B2A5: return ROM_HYP314_46_143_A4000T;
        case 0x8173D7B6: return ROM_HYP320_47_96_A500;
        case 0xBD1FF75E: return ROM_HYP320_47_96_A1200;
        case 0xF3AF46CC: return ROM_HYP320_47_96_A3000;
        case 0x9BB8FC93: return ROM_HYP320_47_96_A4000;
        case 0x9188A509: return ROM_HYP320_47_96_A4000T;
        case 0x4F078456: return ROM_HYP321_47_102_A500;

        case 0x9CE0F009: return ROM_AROS_54705;
        case 0xE2C7F70A: return ROM_AROS_54705_EXT;
        case 0x3F4FCC0A: return ROM_AROS_55696;
        case 0xF2E52B07: return ROM_AROS_55696_EXT;
        case 0x4CE7C8D6: return ROM_AROS_1ED13DE6E3;
        case 0xF2A9CDC5: return ROM_AROS_1ED13DE6E3_EXT;

        case 0x4C4B5C05: return ROM_DIAG11;
        case 0x771CD0EA: return ROM_DIAG12;
        case 0x850209CD: return ROM_DIAG121;
        case 0x8484F426: return ROM_LOGICA20;

        default: return ROM_UNKNOWN;
    }
}

bool
RomFile::isCompatible(const string &path)
{
    return true;
}

bool
RomFile::isBootRom(RomIdentifier rev)
{
    switch (rev) {

        case ROM_BOOT_A1000_8K:
        case ROM_BOOT_A1000_64K: return true;

        default: return false;
    }
}

bool
RomFile::isArosRom(RomIdentifier rev)
{
    switch (rev) {

        case 0x00000000: return ROM_MISSING;

        case ROM_AROS_54705:
        case ROM_AROS_54705_EXT:
        case ROM_AROS_55696:
        case ROM_AROS_55696_EXT:
        case ROM_AROS_1ED13DE6E3:
        case ROM_AROS_1ED13DE6E3_EXT: return true;

        default: return false;
    }
}

bool
RomFile::isDiagRom(RomIdentifier rev)
{
    switch (rev) {

        case ROM_DIAG11:
        case ROM_DIAG12:
        case ROM_DIAG121:
        case ROM_LOGICA20: return true;

        default: return false;
    }
}

bool
RomFile::isCommodoreRom(RomIdentifier rev)
{
    switch (rev) {

        case ROM_BOOT_A1000_8K:
        case ROM_BOOT_A1000_64K:

        case ROM_KICK07_27_003_BETA:

        case ROM_KICK10_30_NTSC:
        case ROM_KICK11_31_034_NTSC:
        case ROM_KICK11_32_034_PAL:
        case ROM_KICK12_33_166:
        case ROM_KICK12_33_180:
        case ROM_KICK121_34_004:
        case ROM_KICK13_34_005_A500:
        case ROM_KICK13_34_005_A3000:

        case ROM_KICK12_33_180_MRAS:

        case ROM_KICK20_36_028:
        case ROM_KICK202_36_207_A3000:
        case ROM_KICK204_37_175_A500:
        case ROM_KICK204_37_175_A3000:
        case ROM_KICK205_37_299_A600:
        case ROM_KICK205_37_300_A600HD:
        case ROM_KICK205_37_350_A600HD:

        case ROM_KICK30_39_106_A1200:
        case ROM_KICK30_39_106_A4000:
        case ROM_KICK31_40_063_A500:
        case ROM_KICK31_40_068_A1200:
        case ROM_KICK31_40_068_A3000:
        case ROM_KICK31_40_068_A4000:
        case ROM_KICK31_40_070_A4000T: return true;

        default: return false;
    }
}

bool
RomFile::isHyperionRom(RomIdentifier rev)
{
    switch (rev) {

        case ROM_HYP314_46_143_A500:
        case ROM_HYP314_46_143_A1200:
        case ROM_HYP314_46_143_A2000:
        case ROM_HYP314_46_143_A3000:
        case ROM_HYP314_46_143_A4000:
        case ROM_HYP314_46_143_A4000T:
        case ROM_HYP320_47_96_A500:
        case ROM_HYP320_47_96_A1200:
        case ROM_HYP320_47_96_A3000:
        case ROM_HYP320_47_96_A4000:
        case ROM_HYP320_47_96_A4000T:
        case ROM_HYP321_47_102_A500:    return true;

        default: return false;
    }
}

bool
RomFile::isPatchedRom(RomIdentifier rev)
{
    switch (rev) {

        case ROM_KICK12_33_180_G11R:
        case ROM_KICK13_34_005_G12R: return true;

        default: return false;
    }
}

const char *
RomFile::title(RomIdentifier rev)
{
    switch (rev) {

        case ROM_UNKNOWN:               return "Unknown or patched Rom";

        case ROM_BOOT_A1000_8K:
        case ROM_BOOT_A1000_64K:        return "Amiga 1000 Boot Rom";

        case ROM_KICK07_27_003_BETA:    return "Kickstart 0.7 Beta";
        case ROM_KICK10_30_NTSC:        return "Kickstart 1.0";
        case ROM_KICK11_31_034_NTSC:
        case ROM_KICK11_32_034_PAL:     return "Kickstart 1.1";
        case ROM_KICK12_33_166:
        case ROM_KICK12_33_180:         return "Kickstart 1.2";
        case ROM_KICK121_34_004:        return "Kickstart 1.21";
        case ROM_KICK13_34_005_A500:
        case ROM_KICK13_34_005_A3000:   return "Kickstart 1.3";

        case ROM_KICK12_33_180_MRAS:    return "Kickstart 1.2";
            
        case ROM_KICK12_33_180_G11R:    return "Kickstart 1.2";
        case ROM_KICK13_34_005_G12R:    return "Kickstart 1.3";

        case ROM_KICK20_36_028:         return "Kickstart 2.0";
        case ROM_KICK202_36_207_A3000:  return "Kickstart 2.02";
        case ROM_KICK204_37_175_A500:
        case ROM_KICK204_37_175_A3000:  return "Kickstart 2.04";
        case ROM_KICK205_37_299_A600:
        case ROM_KICK205_37_300_A600HD:
        case ROM_KICK205_37_350_A600HD: return "Kickstart 2.05";

        case ROM_KICK30_39_106_A1200:
        case ROM_KICK30_39_106_A4000:   return "Kickstart 3.0";
        case ROM_KICK31_40_063_A500:
        case ROM_KICK31_40_068_A1200:
        case ROM_KICK31_40_068_A3000:
        case ROM_KICK31_40_068_A4000:
        case ROM_KICK31_40_070_A4000T:   return "Kickstart 3.1";

        case ROM_HYP314_46_143_A500:
        case ROM_HYP314_46_143_A1200:
        case ROM_HYP314_46_143_A2000:
        case ROM_HYP314_46_143_A3000:
        case ROM_HYP314_46_143_A4000:
        case ROM_HYP314_46_143_A4000T:  return "Kickstart 3.1.4 (Hyperion)";
        case ROM_HYP320_47_96_A500:
        case ROM_HYP320_47_96_A1200:
        case ROM_HYP320_47_96_A3000:
        case ROM_HYP320_47_96_A4000:
        case ROM_HYP320_47_96_A4000T:   return "Kickstart 3.2 (Hyperion)";
        case ROM_HYP321_47_102_A500:    return "Kickstart 3.2.1 (Hyperion)";

        case ROM_AROS_54705:
        case ROM_AROS_55696:
        case ROM_AROS_1ED13DE6E3:       return "AROS Kickstart replacement";
        case ROM_AROS_54705_EXT:
        case ROM_AROS_55696_EXT:
        case ROM_AROS_1ED13DE6E3_EXT:   return "AROS Kickstart extension";

        case ROM_DIAG11:
        case ROM_DIAG12:
        case ROM_DIAG121:               return "Amiga DiagROM";
        case ROM_LOGICA20:              return "Logica Diagnostic";

        default:                        return "";
    }
}

const char *
RomFile::version(RomIdentifier rev)
{
    switch (rev) {

        case ROM_BOOT_A1000_8K:         return "8KB";
        case ROM_BOOT_A1000_64K:        return "64KB";

        case ROM_KICK07_27_003_BETA:    return "Rev 27.003 NTSC";
        case ROM_KICK10_30_NTSC:        return "Rev 30 NTSC";
        case ROM_KICK11_31_034_NTSC:    return "Rev 31.034 NTSC";
        case ROM_KICK11_32_034_PAL:     return "Rev 32.034 PAL";
        case ROM_KICK12_33_166:         return "Rev 33.166";
        case ROM_KICK12_33_180:         return "Rev 33.180";
        case ROM_KICK121_34_004:        return "Rev 34.004";
        case ROM_KICK13_34_005_A500:
        case ROM_KICK13_34_005_A3000:   return "Rev 34.005";

        case ROM_KICK12_33_180_MRAS:    return "Rev 33.180";

        case ROM_KICK12_33_180_G11R:    return "Rev 33.180 (Guardian patch)";
        case ROM_KICK13_34_005_G12R:    return "Rev 34.005 (Guardian patch)";

        case ROM_KICK20_36_028:         return "Rev 36.028";
        case ROM_KICK202_36_207_A3000:  return "Rev 36.207";
        case ROM_KICK204_37_175_A500:
        case ROM_KICK204_37_175_A3000:  return "Rev 37.175";
        case ROM_KICK205_37_299_A600:   return "Rev 37.299";
        case ROM_KICK205_37_300_A600HD: return "Rev 37.300";
        case ROM_KICK205_37_350_A600HD: return "Rev 37.350";

        case ROM_KICK30_39_106_A1200:
        case ROM_KICK30_39_106_A4000:   return "Rev 39.106";
        case ROM_KICK31_40_063_A500:    return "Rev 40.063";
        case ROM_KICK31_40_068_A1200:
        case ROM_KICK31_40_068_A3000:
        case ROM_KICK31_40_068_A4000:   return "Rev 40.068";
        case ROM_KICK31_40_070_A4000T:  return "Rev 40.070";

        case ROM_HYP314_46_143_A500:
        case ROM_HYP314_46_143_A1200:
        case ROM_HYP314_46_143_A2000:
        case ROM_HYP314_46_143_A3000:
        case ROM_HYP314_46_143_A4000:
        case ROM_HYP314_46_143_A4000T:  return "Rev 46.143";
        case ROM_HYP320_47_96_A500:
        case ROM_HYP320_47_96_A1200:
        case ROM_HYP320_47_96_A3000:
        case ROM_HYP320_47_96_A4000:
        case ROM_HYP320_47_96_A4000T:   return "Rev 47.96";
        case ROM_HYP321_47_102_A500:    return "Rev 47.102";

        case ROM_AROS_54705:            return "SVN 54705";
        case ROM_AROS_54705_EXT:        return "SVN 54705";
        case ROM_AROS_55696:            return "SVN 55696";
        case ROM_AROS_55696_EXT:        return "SVN 55696";
        case ROM_AROS_1ED13DE6E3:       return "Version 1ed13de6e3";
        case ROM_AROS_1ED13DE6E3_EXT:   return "Version 1ed13de6e3";

        case ROM_DIAG11:                return "Version 1.1";
        case ROM_DIAG12:                return "Version 1.2";
        case ROM_DIAG121:               return "Version 1.2.1";
        case ROM_LOGICA20:              return "Version 2.0";

        default:                        return "";
    }
}

const char *
RomFile::released(RomIdentifier rev)
{
    switch (rev) {

        case ROM_BOOT_A1000_8K:         return "1985";
        case ROM_BOOT_A1000_64K:        return "1985";

        case ROM_KICK07_27_003_BETA:    return "July 1985";
        case ROM_KICK10_30_NTSC:        return "September 1985";
        case ROM_KICK11_31_034_NTSC:    return "November 1985";
        case ROM_KICK11_32_034_PAL:     return "February 1986";
        case ROM_KICK12_33_166:         return "September 1986";
        case ROM_KICK12_33_180:         return "October 1986";
        case ROM_KICK121_34_004:        return "November 1987";
        case ROM_KICK13_34_005_A500:    return "December 1987";
        case ROM_KICK13_34_005_A3000:   return "December 1987";

        case ROM_KICK12_33_180_MRAS:    return "2022";

        case ROM_KICK12_33_180_G11R:    return "1988";
        case ROM_KICK13_34_005_G12R:    return "1988";

        case ROM_KICK20_36_028:         return "March 1990";
        case ROM_KICK202_36_207_A3000:  return "October 1990";
        case ROM_KICK204_37_175_A500:
        case ROM_KICK204_37_175_A3000:  return "May 1991";
        case ROM_KICK205_37_299_A600:   return "November 1991";
        case ROM_KICK205_37_300_A600HD: return "November 1991";
        case ROM_KICK205_37_350_A600HD: return "April 1992";

        case ROM_KICK30_39_106_A1200:
        case ROM_KICK30_39_106_A4000:   return "September 1992";
        case ROM_KICK31_40_063_A500:    return "July 1993";
        case ROM_KICK31_40_068_A1200:
        case ROM_KICK31_40_068_A3000:
        case ROM_KICK31_40_068_A4000:   return "December 1993";
        case ROM_KICK31_40_070_A4000T:  return "February 1994";

        case ROM_HYP314_46_143_A500:
        case ROM_HYP314_46_143_A1200:
        case ROM_HYP314_46_143_A2000:
        case ROM_HYP314_46_143_A3000:
        case ROM_HYP314_46_143_A4000:
        case ROM_HYP314_46_143_A4000T:  return "September 2018";
        case ROM_HYP320_47_96_A500:
        case ROM_HYP320_47_96_A1200:
        case ROM_HYP320_47_96_A3000:
        case ROM_HYP320_47_96_A4000:
        case ROM_HYP320_47_96_A4000T:   return "May 2021";
        case ROM_HYP321_47_102_A500:    return "December 2021";

        case ROM_AROS_54705:            return "May 2017";
        case ROM_AROS_54705_EXT:        return "May 2017";
        case ROM_AROS_55696:            return "February 2019";
        case ROM_AROS_55696_EXT:        return "February 2019";
        case ROM_AROS_1ED13DE6E3:       return "September 2021";
        case ROM_AROS_1ED13DE6E3_EXT:   return "September 2021";

        case ROM_DIAG11:                return "October 2018";
        case ROM_DIAG12:                return "August 2019";
        case ROM_DIAG121:               return "July 2020";
        case ROM_LOGICA20:              return "";

        default:                        return "";
    }
}

const char *
RomFile::model(RomIdentifier rev) {
    switch (rev) {

        case ROM_BOOT_A1000_8K:
        case ROM_BOOT_A1000_64K:        return "A1000";

        case ROM_KICK07_27_003_BETA:
        case ROM_KICK10_30_NTSC:
        case ROM_KICK11_31_034_NTSC:
        case ROM_KICK11_32_034_PAL:
        case ROM_KICK12_33_166:         return "A1000";
        case ROM_KICK12_33_180:         return "A500, A1000, A2000";
        case ROM_KICK121_34_004:        return "???";
        case ROM_KICK13_34_005_A500:    return "A500, A1000, A2000, CDTV";
        case ROM_KICK13_34_005_A3000:   return "A3000";

        case ROM_KICK12_33_180_MRAS:    return "MRAS patch";

        case ROM_KICK12_33_180_G11R:    return "";
        case ROM_KICK13_34_005_G12R:    return "";

        case ROM_KICK20_36_028:         return "";
        case ROM_KICK202_36_207_A3000:  return "A3000";
        case ROM_KICK204_37_175_A500:   return "A500";
        case ROM_KICK204_37_175_A3000:  return "A3000";
        case ROM_KICK205_37_299_A600:   return "A600";
        case ROM_KICK205_37_300_A600HD:
        case ROM_KICK205_37_350_A600HD: return "A600HD";

        case ROM_KICK30_39_106_A1200:   return "A1200";
        case ROM_KICK30_39_106_A4000:   return "A4000";
        case ROM_KICK31_40_063_A500:    return "A500, A600, A2000";
        case ROM_KICK31_40_068_A1200:   return "A1200";
        case ROM_KICK31_40_068_A3000:   return "A3000";
        case ROM_KICK31_40_068_A4000:   return "A4000";
        case ROM_KICK31_40_070_A4000T:  return "A4000T";

        case ROM_HYP314_46_143_A500:    return "A500, A600, A1000";
        case ROM_HYP314_46_143_A1200:   return "A1200";
        case ROM_HYP314_46_143_A2000:   return "A2000";
        case ROM_HYP314_46_143_A3000:   return "A3000";
        case ROM_HYP314_46_143_A4000:   return "A4000";
        case ROM_HYP314_46_143_A4000T:  return "A4000T";
        case ROM_HYP320_47_96_A500:     return "A500, A600, A1000, A2000, CDTV";
        case ROM_HYP320_47_96_A1200:    return "A1200";
        case ROM_HYP320_47_96_A3000:    return "A3000";
        case ROM_HYP320_47_96_A4000:    return "A4000";
        case ROM_HYP320_47_96_A4000T:   return "A4000T";
        case ROM_HYP321_47_102_A500:    return "A500, A600, A1000, A2000, CDTV";

        case ROM_AROS_54705:            return "UAE version";
        case ROM_AROS_54705_EXT:        return "UAE version";
        case ROM_AROS_55696:            return "SAE version";
        case ROM_AROS_55696_EXT:        return "SAE version";
        case ROM_AROS_1ED13DE6E3:       return "";
        case ROM_AROS_1ED13DE6E3_EXT:   return "";

        case ROM_DIAG11:                return "";
        case ROM_DIAG12:                return "";
        case ROM_DIAG121:               return "";
        case ROM_LOGICA20:              return "";

        default:                        return "";
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
