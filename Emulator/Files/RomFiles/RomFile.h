// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaFile.h"

class RomFile : public AmigaFile {

    // Accepted header signatures
    static const u8 bootRomHeaders[1][8];
    static const u8 kickRomHeaders[6][7];
    static const u8 encrRomHeaders[1][11];

    // Indicates if a rom.key file is required to use this Rom
    bool needsRomKey = false;
    
public:
    
    //
    // Class methods
    //
    
    static bool isCompatibleName(const std::string &name);
    static bool isCompatibleStream(std::istream &stream);
    
    // Returns true if buffer contains a Boot Rom or an Kickstart Rom image
    static bool isRomBuffer(const u8 *buffer, size_t length);
    
    // Returns true if path points to a Boot Rom file or a Kickstart Rom file
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
    [[deprecated]] bool matchingBuffer(const u8 *buffer, size_t length) override {
        return isRomBuffer(buffer, length); }
    [[deprecated]] bool matchingFile(const char *path) override { return isRomFile(path); }
    usize readFromStream(std::istream &stream) override;

    
    //
    // Decrypting
    //
    
    // Returns true iff the Rom was encrypted at the time it was loaded
    bool wasEncrypted() { return needsRomKey; }
    
    // Returns true iff the Rom is currently encrypted
    bool isEncrypted();
    
    /* Tries to decrypt the Rom. If this method is applied to an encrypted Rom,
     * a rom.key file is seeked in the directory the encrypted Rom was loaded
     * from and applies to the encrypted data.
     */
    void decrypt() throws;
};
