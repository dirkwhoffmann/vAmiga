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
#include "AmigaFile.h"

namespace vamiga {

class RomFile : public AmigaFile {

    // Accepted header signatures
    static const u8 bootRomHeaders[1][8];
    static const u8 kickRomHeaders[8][7];
    static const u8 encrRomHeaders[1][11];

    // Path to the rom.key file (if needed)
    string romKeyPath = "";

public:

    static bool isCompatible(const string &path);
    static bool isCompatible(std::istream &stream);

    static bool isRomBuffer(const u8 *buf, isize len);
    static bool isRomFile(const string &path);

    // Classifies a ROM identifier by type
    static bool isBootRom(u32 crc32);
    static bool isArosRom(u32 crc32);
    static bool isDiagRom(u32 crc32);
    static bool isCommodoreRom(u32 crc32);
    static bool isHyperionRom(u32 crc32);
    static bool isEmutosRom(u32 crc32);
    static bool isPatchedRom(u32 crc32);

    // Translates a ROM indentifier into a textual description
    static const char *shortName(u32 crc32);
    static const char *title(u32 crc32);
    static const char *version(u32 crc32);
    static const char *released(u32 crc32);
    static const char *model(u32 crc32);


    //
    // Initializing
    //

    RomFile(const string &path) throws { init(path); }
    RomFile(const string &path, std::istream &stream) throws { init(path, stream); }
    RomFile(const u8 *buf, isize len) throws { init(buf, len); }

    const char *getDescription() const override { return "ROM"; }


    //
    // Methods from AmigaFile
    //

    FileType type() const override { return FILETYPE_ROM; }
    bool isCompatiblePath(const string &path) const override { return isCompatible(path); }
    bool isCompatibleStream(std::istream &stream) const override { return isCompatible(stream); }


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
