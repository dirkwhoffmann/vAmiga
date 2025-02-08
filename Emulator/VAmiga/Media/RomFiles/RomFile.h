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
#include "AnyFile.h"

namespace vamiga {

class RomFile : public AnyFile {

    // Accepted header signatures
    static const u8 bootRomHeaders[1][8];
    static const u8 kickRomHeaders[10][7];
    static const u8 encrRomHeaders[1][11];

    // Path to the rom.key file (if needed)
    std::filesystem::path romKeyPath;

public:

    static bool isCompatible(const std::filesystem::path &path);
    static bool isCompatible(const u8 *buf, isize len);
    static bool isCompatible(const Buffer<u8> &buffer);


    //
    // Initializing
    //

    RomFile(const std::filesystem::path &path) throws { init(path); }
    RomFile(const u8 *buf, isize len) throws { init(buf, len); }

    const char *objectName() const override { return "ROM"; }


    //
    // Methods from AnyFile
    //

    FileType type() const override { return FileType::ROM; }
    bool isCompatiblePath(const std::filesystem::path &path) const override { return isCompatible(path); }
    bool isCompatibleBuffer(const u8 *buf, isize len) override { return isCompatible(buf, len); }


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
