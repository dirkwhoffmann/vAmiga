// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RomFileTypes.h"

namespace vamiga {

static std::vector<RomTraits> roms = {
    
    {
        .crc        = CRC32_BOOT_A1000_8K,
        .title      = "Amiga 1000 Boot Rom",
        .revision   = "8K",
        .released   = "1985",
        .model      = "A1000",
        .boot       = true
    },{
        .crc        = CRC32_BOOT_A1000_64K,
        .title      = "Amiga 1000 Boot Rom",
        .revision   = "64KB",
        .released   = "1985",
        .model      = "A1000",
        .boot       = true
    },{
        .crc        = CRC32_KICK07_27_003_BETA,
        .title      = "Kickstart 0.7 Beta",
        .revision   = "Rev 27.003 NTSC",
        .released   = "July 1985",
        .model      = ""
    },{
        .crc        = CRC32_KICK10_30_NTSC,
        .title      = "Kickstart 1.0",
        .revision   = "Rev 30.000 NTSC",
        .released   = "September 1985",
        .model      = "A1000"
    },{
        .crc        = CRC32_KICK11_31_034_NTSC,
        .title      = "Kickstart 1.1",
        .revision   = "Rev 31.034 NTSC",
        .released   = "November 1985",
        .model      = "A1000"
    },{
        .crc        = CRC32_KICK11_32_034_PAL,
        .title      = "Kickstart 1.1",
        .revision   = "Rev 32.034 PAL",
        .released   = "February 1986",
        .model      = "A1000"
    },{
        .crc        = CRC32_KICK12_33_166,
        .title      = "Kickstart 1.2",
        .revision   = "Rev 33.166",
        .released   = "September 1986",
        .model      = "A1000"
    },{
        .crc        = CRC32_KICK12_33_180,
        .title      = "Kickstart 1.2",
        .revision   = "Rev 33.180",
        .released   = "October 1986",
        .model      = "A500, A1000, A2000"
    },{
        .crc        = CRC32_KICK121_34_004,
        .title      = "Kickstart 1.2",
        .revision   = "Rev 34.004",
        .released   = "October 1986",
        .model      = ""
    },{
        .crc        = CRC32_KICK13_34_005_A500,
        .title      = "Kickstart 1.3",
        .revision   = "Rev 34.005",
        .released   = "December 1987",
        .model      = "A500, A1000, A2000, CDTV"
    },{
        .crc        = CRC32_KICK13_34_005_A3000,
        .title      = "Kickstart 1.3",
        .revision   = "Rev 34.005",
        .released   = "December 1987",
        .model      = "A3000"
    },{
        .crc        = CRC32_KICK12_33_180_MRAS,
        .title      = "Kickstart 1.2",
        .revision   = "Rev 33.180",
        .released   = "2022",
        .model      = "MRAS patch"
    },{
        .crc        = CRC32_KICK12_33_180_G11R,
        .title      = "Kickstart 1.2",
        .revision   = "Rev 33.180 (Guardian patch)",
        .released   = "1988",
        .model      = "",
        .patched    = true
    },{
        .crc        = CRC32_KICK13_34_005_G12R,
        .title      = "Kickstart 1.3",
        .revision   = "Rev 34.005 (Guardian patch)",
        .released   = "1988",
        .model      = "",
        .patched    = true
    },{
        .crc        = CRC32_KICK20_36_028,
        .title      = "Kickstart 2.0",
        .revision   = "Rev 36.028",
        .released   = "March 1990",
        .model      = ""
    },{
        .crc        = CRC32_KICK202_36_207_A3000,
        .title      = "Kickstart 2.02",
        .revision   = "Rev 36.207",
        .released   = "October 1990",
        .model      = "A3000"
    },{
        .crc        = CRC32_KICK204_37_175_A500,
        .title      = "Kickstart 2.04",
        .revision   = "Rev 37.175",
        .released   = "May 1991",
        .model      = "A500"
    },{
        .crc        = CRC32_KICK204_37_175_A3000,
        .title      = "Kickstart 2.04",
        .revision   = "Rev 37.175",
        .released   = "May 1991",
        .model      = "A3000"
    },{
        .crc        = CRC32_KICK205_37_299_A600,
        .title      = "Kickstart 2.05",
        .revision   = "Rev 37.299",
        .released   = "November 1991",
        .model      = "A600"
    },{
        .crc        = CRC32_KICK205_37_300_A600HD,
        .title      = "Kickstart 2.05",
        .revision   = "Rev 37.300",
        .released   = "November 1991",
        .model      = ""
    },{
        .crc        = CRC32_KICK205_37_350_A600HD,
        .title      = "Kickstart 2.05",
        .revision   = "Rev 37.350",
        .released   = "April 1992",
        .model      = "A600HD"
    },{
        .crc        = CRC32_KICK30_39_106_A1200,
        .title      = "Kickstart 3.0",
        .revision   = "Rev 39.106",
        .released   = "September 1992",
        .model      = "A1200"
    },{
        .crc        = CRC32_KICK30_39_106_A4000,
        .title      = "Kickstart 3.0",
        .revision   = "Rev 39.106",
        .released   = "September 1992",
        .model      = "A4000"
    },{
        .crc        = CRC32_KICK31_40_063_A500,
        .title      = "Kickstart 3.1",
        .revision   = "Rev 40.063",
        .released   = "July 1993",
        .model      = "A500, A600, A2000"
    },{
        .crc        = CRC32_KICK31_40_063_A500_R,
        .title      = "Kickstart 3.1",
        .revision   = "Rev 40.063 (patched)",
        .released   = "ReKick image",
        .model      = ""
    },{
        .crc        = CRC32_KICK31_40_068_A1200,
        .title      = "Kickstart 3.1",
        .revision   = "Rev 40.068",
        .released   = "December 1993",
        .model      = "A1200"
    },{
        .crc        = CRC32_KICK31_40_068_A3000,
        .title      = "Kickstart 3.1",
        .revision   = "Rev 40.068",
        .released   = "December 1993",
        .model      = "A3000"
    },{
        .crc        = CRC32_KICK31_40_068_A4000,
        .title      = "Kickstart 3.1",
        .revision   = "Rev 40.068",
        .released   = "December 1993",
        .model      = "A4000"
    },{
        .crc        = CRC32_KICK31_40_070_A4000T,
        .title      = "Kickstart 3.1",
        .revision   = "Rev 40.070",
        .released   = "February 1994",
        .model      = "A4000T"
    },{
        .crc        = CRC32_HYP314_46_143_A500,
        .title      = "Kickstart 3.1.4 (Hyperion)",
        .revision   = "Rev 46.143",
        .released   = "September 2018",
        .model      = "A500, A600, A1000",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP314_46_143_A1200,
        .title      = "Kickstart 3.1.4 (Hyperion)",
        .revision   = "Rev 46.143",
        .released   = "September 2018",
        .model      = "A1200",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP314_46_143_A2000,
        .title      = "Kickstart 3.1.4 (Hyperion)",
        .revision   = "Rev 46.143",
        .released   = "September 2018",
        .model      = "A2000",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP314_46_143_A3000,
        .title      = "Kickstart 3.1.4 (Hyperion)",
        .revision   = "Rev 46.143",
        .released   = "September 2018",
        .model      = "A3000",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP314_46_143_A4000,
        .title      = "Kickstart 3.1.4 (Hyperion)",
        .revision   = "Rev 46.143",
        .released   = "September 2018",
        .model      = "A4000",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP314_46_143_A4000T,
        .title      = "Kickstart 3.1.4 (Hyperion)",
        .revision   = "Rev 46.143",
        .released   = "May 2021",
        .model      = "A4000T",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP320_47_96_A500,
        .title      = "Kickstart 3.2 (Hyperion)",
        .revision   = "Rev 47.96",
        .released   = "May 2021",
        .model      = "A500, A600, A1000, A2000, CDTV",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP320_47_96_A1200,
        .title      = "Kickstart 3.2 (Hyperion)",
        .revision   = "Rev 47.96",
        .released   = "May 2021",
        .model      = "A1200",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP320_47_96_A3000,
        .title      = "Kickstart 3.2 (Hyperion)",
        .revision   = "Rev 47.96",
        .released   = "May 2021",
        .model      = "A3000",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP320_47_96_A4000,
        .title      = "Kickstart 3.2 (Hyperion)",
        .revision   = "Rev 47.96",
        .released   = "May 2021",
        .model      = "A4000",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP320_47_96_A4000T,
        .title      = "Kickstart 3.2 (Hyperion)",
        .revision   = "Rev 47.96",
        .released   = "May 2021",
        .model      = "A4000T",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP321_47_102_A500,
        .title      = "Kickstart 3.2.1 (Hyperion)",
        .revision   = "Rev 47.102",
        .released   = "December 2021",
        .model      = "A500, A600, A1000, A2000, CDTV",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP321_47_102_A1200,
        .title      = "Kickstart 3.2.1 (Hyperion)",
        .revision   = "Rev 47.102",
        .released   = "December 2021",
        .model      = "A1200",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP321_47_102_A3000,
        .title      = "Kickstart 3.2.1 (Hyperion)",
        .revision   = "Rev 47.102",
        .released   = "December 2021",
        .model      = "A3000",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP321_47_102_A4000,
        .title      = "Kickstart 3.2.1 (Hyperion)",
        .revision   = "Rev 47.102",
        .released   = "December 2021",
        .model      = "A4000",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP321_47_102_A4000T,
        .title      = "Kickstart 3.2.1 (Hyperion)",
        .revision   = "Rev 47.102",
        .released   = "December 2021",
        .model      = "A4000T",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP322_47_111_A500,
        .title      = "Kickstart 3.2.2 (Hyperion)",
        .revision   = "Rev 47.111",
        .released   = "March 2023",
        .model      = "A500, A600, A1000, A2000, CDTV",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP322_47_111_A1200,
        .title      = "Kickstart 3.2.2 (Hyperion)",
        .revision   = "Rev 47.111",
        .released   = "March 2023",
        .model      = "A1200",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP322_47_111_A3000,
        .title      = "Kickstart 3.2.2 (Hyperion)",
        .revision   = "Rev 47.111",
        .released   = "March 2023",
        .model      = "A3000",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP322_47_111_A4000,
        .title      = "Kickstart 3.2.2 (Hyperion)",
        .revision   = "Rev 47.111",
        .released   = "March 2023",
        .model      = "A4000",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_HYP322_47_111_A4000T,
        .title      = "Kickstart 3.2.2 (Hyperion)",
        .revision   = "Rev 47.111",
        .released   = "March 2023",
        .model      = "A4000T",
        .vendor     = RomVendor::HYPERION
    },{
        .crc        = CRC32_AROS_54705,
        .title      = "AROS Kickstart replacement",
        .revision   = "SVN 54705",
        .released   = "May 2017",
        .model      = "UAE version",
        .vendor     = RomVendor::AROS
    },{
        .crc        = CRC32_AROS_54705_EXT,
        .title      = "AROS Kickstart extension",
        .revision   = "SVN 54705",
        .released   = "May 2017",
        .model      = "UAE version",
        .vendor     = RomVendor::AROS
    },{
        .crc        = CRC32_AROS_55696,
        .title      = "AROS Kickstart replacement",
        .revision   = "SVN 55696",
        .released   = "February 2019",
        .model      = "SAE version",
        .vendor     = RomVendor::AROS
    },{
        .crc        = CRC32_AROS_55696_EXT,
        .title      = "AROS Kickstart extension",
        .revision   = "SVN 55696",
        .released   = "February 2019",
        .model      = "SAE version",
        .vendor     = RomVendor::AROS
    },{
        .crc        = CRC32_AROS_1ED13DE6E3,
        .title      = "AROS Kickstart replacement",
        .revision   = "Version 1ed13de6e3",
        .released   = "September 2021",
        .model      = "",
        .vendor     = RomVendor::AROS
    },{
        .crc        = CRC32_AROS_1ED13DE6E3_EXT,
        .title      = "AROS Kickstart extension",
        .revision   = "Version 1ed13de6e3",
        .released   = "September 2021",
        .model      = "",
        .vendor     = RomVendor::AROS
    },{
        .crc        = CRC32_EMUTOS13,
        .title      = "EmuTOS Kickstart replacement",
        .revision   = "Version 1.3",
        .released   = "March 2024",
        .model      = "",
        .vendor     = RomVendor::EMUTOS
    },{
        .crc        = CRC32_DIAG11,
        .title      = "Amiga DiagROM",
        .revision   = "Version 1.1",
        .released   = "October 2018",
        .model      = "",
        .vendor     = RomVendor::DIAG
    },{
        .crc        = CRC32_DIAG12,
        .title      = "Amiga DiagROM",
        .revision   = "Version 1.2",
        .released   = "August 2019",
        .model      = "",
        .vendor     = RomVendor::DIAG
    },{
        .crc        = CRC32_DIAG121,
        .title      = "Amiga DiagROM",
        .revision   = "Version 1.2.1",
        .released   = "July 2020",
        .model      = "",
        .vendor     = RomVendor::DIAG
    },{
        .crc        = CRC32_DIAG13,
        .title      = "Amiga DiagROM",
        .revision   = "Version 1.3",
        .released   = "April 2023",
        .model      = "",
        .vendor     = RomVendor::DIAG
    },{
        .crc        = CRC32_CPUBLTRO_F8_0_3_2,
        .title      = "OCS CPUBLTRO",
        .revision   = "Version 0.3.2",
        .released   = "November 2024",
        .model      = "512 KB",
        .vendor     = RomVendor::DEMO
    },{
        .crc        = CRC32_CPUBLTRO_FC_0_3_2,
        .title      = "OCS CPUBLTRO",
        .revision   = "Version 0.3.2",
        .released   = "November 2024",
        .model      = "256 KB",
        .vendor     = RomVendor::DEMO
    },
};

}
