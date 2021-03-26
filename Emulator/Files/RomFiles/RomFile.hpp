// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaFile.hpp"

namespace va {

#include "RomFile.h"

//
// Reflection APIs
//

struct RomIdentifierEnum : util::Reflection<RomIdentifierEnum, RomIdentifier> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < ROM_COUNT;
    }
    
    static const char *prefix() { return "ROM"; }
    static const char *key(RomIdentifier value)
    {
        switch (value) {
                
            case ROM_MISSING:           return "MISSING";
            case ROM_UNKNOWN:           return "UNKNOWN";

            case ROM_BOOT_A1000_8K:     return "BOOT_A1000_8K";
            case ROM_BOOT_A1000_64K:    return "BOOT_A1000_64K";

            case ROM_KICK11_31_034:     return "KICK11_31_034";
            case ROM_KICK12_33_166:     return "KICK12_33_166";
            case ROM_KICK12_33_180:     return "KICK12_33_180";
            case ROM_KICK121_34_004:    return "KICK121_34_004";
            case ROM_KICK13_34_005:     return "KICK13_34_005";
            case ROM_KICK13_34_005_SK:  return "KICK13_34_005_SK";

            case ROM_KICK20_36_028:     return "KICK20_36_028";
            case ROM_KICK202_36_207:    return "KICK202_36_207";
            case ROM_KICK204_37_175:    return "KICK204_37_175";
            case ROM_KICK205_37_299:    return "KICK205_37_299";
            case ROM_KICK205_37_300:    return "KICK205_37_300";
            case ROM_KICK205_37_350:    return "KICK205_37_350";

            case ROM_KICK30_39_106:     return "KICK30_39_106";
            case ROM_KICK31_40_063:     return "KICK31_40_063";

            case ROM_HYP314_46_143:     return "HYP314_46_143";

            case ROM_AROS_55696:        return "AROS_55696";
            case ROM_AROS_55696_EXT:    return "AROS_55696_EXT";

            case ROM_DIAG11:            return "DIAG11";
            case ROM_DIAG12:            return "DIAG12";
            case ROM_DIAG121:           return "DIAG121";
            case ROM_LOGICA20:          return "LOGICA20";
                
            case ROM_COUNT:             return "???";
        }
        return "???";
    }
};

class RomFile : public AmigaFile {

    // Accepted header signatures
    static const u8 bootRomHeaders[1][8];
    static const u8 kickRomHeaders[6][7];
    static const u8 encrRomHeaders[1][11];

    // Path to the rom.key file (if needed)
    string romKeyPath = "";
        
public:
    
    //
    // Class methods
    //
    
    static bool isCompatiblePath(const string &path);
    static bool isCompatibleStream(std::istream &stream);
    
    static bool isRomBuffer(const u8 *buf, isize len);
    static bool isRomFile(const char *path);
    
    // Translates a CRC-32 checksum into a ROM identifier
    static RomIdentifier identifier(u32 fingerprint);

    // Classifies a ROM identifier by type
    static bool isBootRom(RomIdentifier rev);
    static bool isArosRom(RomIdentifier rev);
    static bool isDiagRom(RomIdentifier rev);
    static bool isCommodoreRom(RomIdentifier rev);
    static bool isHyperionRom(RomIdentifier rev);

    // Translates a ROM indentifier into a textual description
    static const char *title(RomIdentifier rev);
    static const char *version(RomIdentifier rev);
    static const char *released(RomIdentifier rev);

    
    //
    // Initializing
    //
    
    RomFile();
    
    const char *getDescription() const override { return "ROM"; }
        
    
    //
    // Methods from AmigaFile
    //
    
    FileType type() const override { return FILETYPE_ROM; }

    
    //
    // Decrypting
    //
    
    // Returns true iff the Rom was encrypted at the time it was loaded
    bool wasEncrypted() { return romKeyPath != ""; }
    
    // Returns true iff the Rom is currently encrypted
    bool isEncrypted();
    
    /* Tries to decrypt the Rom. If this method is applied to an encrypted Rom,
     * a rom.key file is seeked in the directory the encrypted Rom was loaded
     * from and applied to the encrypted data.
     */
    void decrypt() throws;
};

}
