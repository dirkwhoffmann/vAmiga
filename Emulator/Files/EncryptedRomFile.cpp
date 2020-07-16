// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "EncryptedRomFile.h"

//
// Encrypted Kickstart Roms
//

const u8 EncryptedRomFile::headers[1][11] = {

    // Cloanto Rom Header Signature
    { 'A', 'M', 'I', 'R', 'O', 'M', 'T', 'Y', 'P', 'E', '1' }
};

EncryptedRomFile::EncryptedRomFile()
{
    setDescription("EncryptedRom");
}

bool
EncryptedRomFile::isEncryptedRomBuffer(const u8 *buffer, size_t length)
{
    if (length == KB(256) + 11) {

        int len = sizeof(headers[0]);
        int cnt = sizeof(headers) / len;

        for (int i = 0; i < cnt; i++)
            if (matchingBufferHeader(buffer, headers[i], len)) return true;

        return false;
    }

    return false;
}

bool
EncryptedRomFile::isEncryptedRomFile(const char *path)
{
     if (checkFileSize(path, KB(256) + 11)) {

         int len = sizeof(headers[0]);
         int cnt = sizeof(headers) / len;

         for (int i = 0; i < cnt; i++)
             if (matchingFileHeader(path, headers[i], len)) return true;

         return false;
     }

    return false;
}

EncryptedRomFile *
EncryptedRomFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    EncryptedRomFile *rom = new EncryptedRomFile();
    
    if (!rom->readFromBuffer(buffer, length)) {
        delete rom;
        return NULL;
    }
    
    return rom;
}

EncryptedRomFile *
EncryptedRomFile::makeWithFile(const char *path)
{
    EncryptedRomFile *rom = new EncryptedRomFile();
    
    if (!rom->readFromFile(path)) {
        delete rom;
        return NULL;
    }
    
    printf("Made encr Rom at %p\n", rom);    
    return rom;
}

bool
EncryptedRomFile::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isEncryptedRomBuffer(buffer, length);
}

RomFile *
EncryptedRomFile::decrypt()
{
    u8 *romKeyBuffer;
    long romKeySize;
    
    printf("Path = %s\n", path);
    
    char *directory = stripFilename(path);
    printf("Directory = %s\n", directory);

    char *romKeyPath = strcat(directory, "rom.key");
    printf("romKeyPath = %s\n", directory);
    
    if (!loadFile(romKeyPath, &romKeyBuffer, &romKeySize)) {
        printf("Failed to load file\n");
        return NULL;
    }
    
    printf("buffer: %p size: %ld\n", romKeyBuffer, romKeySize);
    
    const int headerSize = 11;
    u8 *encrypted = data + headerSize;
    long decryptedSize = size - headerSize;
    u8 *decrypted = new u8[decryptedSize];
    
    // Decrypt
    for (long i = 0; i < decryptedSize; i++) {
        decrypted[i] = encrypted[i] ^ (romKeyBuffer[i % romKeySize]);
    }

    // Write out
    FILE *out = fopen("/tmp/decrypted.rom", "w");
    for (long i = 0; i < decryptedSize; i++) {
        fputc(decrypted[i], out);
    }
    fclose(out);
    printf("Written /tmp/decrypted.rom\n");
    
    RomFile *rom = RomFile::makeWithBuffer(decrypted, decryptedSize);
    delete [] decrypted;
    delete [] romKeyBuffer;
    return rom;
}
