// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "EXEFile.h"
#include "MutableFileSystem.h"
#include "IOUtils.h"
#include "OSDescriptors.h"

namespace vamiga {

bool
EXEFile::isCompatible(const fs::path &path)
{
    auto suffix = util::uppercased(path.extension().string());
    return suffix == ".EXE";
}

bool
EXEFile::isCompatible(const u8 *buf, isize len)
{
    u8 signature[] = { 0x00, 0x00, 0x03, 0xF3 };

    // Only accept the file if it fits onto a HD disk
    if (len > 1710000) return false;

    return util::matchingBufferHeader(buf, signature, sizeof(signature));
}

bool
EXEFile::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
}

void
EXEFile::finalizeRead()
{    
    // Check if this file requires a high-density disk
    bool hd = data.size > 853000;

    // Create a new file system
    MutableFileSystem volume(Diameter::INCH_35, hd ? Density::HD : Density::DD, FSFormat::OFS);
    volume.setName(FSName("Disk"));
    
    // Make the volume bootable
    volume.makeBootable(BootBlockId::AMIGADOS_13);

    // Start at the root directory
    if (auto *dir = &volume.root(); dir) {

        // Add the executable
        volume.createFile(*dir, FSName("file"), data);

        // Add a script directory
        dir = &volume.createDir(*dir, FSName("s"));

        // Add a startup sequence
        volume.createFile(*dir, "startup-sequence", "file");
    }

    // Finalize
    volume.updateChecksums();

    // Print some debug information about the volume
    if (FS_DEBUG) volume.dump(Category::State);
    
    // Check file system integrity
    if (FS_DEBUG) volume.doctor.xray(true, std::cout, false);

    // Convert the volume into an ADF
    adf.init(volume);
}

}
