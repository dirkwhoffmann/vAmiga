// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "RomFile.h"
#include "IOUtils.h"
#include "MemUtils.h"
#include "Macros.h"
#include "RomDatabase.h"

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

const u8 RomFile::kickRomHeaders[10][7] = {

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
    { 0x00, 0x02, 0x84, 0x80, 0x00, 0xFC, 0x00 },

    // Hardware demos (CPUBLTRO)
    { 0x11, 0x14, 0x4e, 0xf9, 0x00, 0xf8, 0x00 },
    { 0x11, 0x14, 0x4e, 0xf9, 0x00, 0xfc, 0x00 },
};

//
// Encrypted Kickstart Roms
//

const u8 RomFile::encrRomHeaders[1][11] = {

    // Cloanto Rom Header Signature
    { 'A', 'M', 'I', 'R', 'O', 'M', 'T', 'Y', 'P', 'E', '1' }
};

bool
RomFile::isCompatible(const fs::path &path)
{
    auto size = util::getSizeOfFile(path);
    
    const std::vector<isize> allowedSizes = {
        
        KB(8), KB(16),              // BOOT ROMs
        KB(256), KB(512),           // Kickstart ROMs
        KB(256) + 11, KB(512) + 11  // Encrypted ROMs
    };

    for (auto allowed: allowedSizes) {
        
        if (size == allowed) {
            
            Buffer<u8> buffer(path);
            return isCompatible(buffer);
        }
    }
    
    return false;
}

bool
RomFile::isCompatible(const u8 *buf, isize length)
{
    // Boot Roms
    if (length == KB(8) || length == KB(16)) {

        isize len = isizeof(bootRomHeaders[0]);
        isize cnt = isizeof(bootRomHeaders) / len;

        for (isize i = 0; i < cnt; i++) {
            if (util::matchingBufferHeader(buf, bootRomHeaders[i], len)) return true;
        }
        return ALLOW_ALL_ROMS;
    }

    // Kickstart Roms
    if (length == KB(256) || length == KB(512)) {

        isize len = isizeof(kickRomHeaders[0]);
        isize cnt = isizeof(kickRomHeaders) / len;

        for (isize i = 0; i < cnt; i++) {
            if (util::matchingBufferHeader(buf, kickRomHeaders[i], len)) return true;
        }
        return ALLOW_ALL_ROMS;
    }

    // Encrypted Kickstart Roms
    if (length == KB(256) + 11 || length == KB(512) + 11) {

        isize len = isizeof(encrRomHeaders[0]);
        isize cnt = isizeof(encrRomHeaders) / len;

        for (isize i = 0; i < cnt; i++) {
            if (util::matchingBufferHeader(buf, encrRomHeaders[i], len)) return true;
        }
    }

    return ALLOW_ALL_ROMS;
}

bool
RomFile::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
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
    romKeyPath = path.remove_filename() / "rom.key";

    // Load the rom.key file
    romKey.init(romKeyPath);
    if (romKey.empty()) throw CoreError(Fault::MISSING_ROM_KEY);
    
    // Decrypt
    decrypted.alloc(data.size - headerSize);
    for (isize i = 0, j = headerSize; j < data.size; i++, j++) {
        decrypted[i] = data[j] ^ romKey[i % romKey.size];
    }
    
    // Replace the old data by the decrypted data
    data.init(decrypted);
    
    // Check if we've got a valid ROM
    if (!isCompatible(data.ptr, data.size)) {
        throw CoreError(Fault::INVALID_ROM_KEY);
    }
}

}
