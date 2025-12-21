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
#include "ADFFactory.h"
#include "FileSystem.h"
#include "OSDescriptors.h"
#include "utl/io.h"
#include "utl/support/Strings.h"

namespace vamiga {

bool
EXEFile::isCompatible(const fs::path &path)
{
    auto suffix = utl::uppercased(path.extension().string());
    return suffix == ".EXE";
}

bool
EXEFile::isCompatible(const u8 *buf, isize len)
{
    u8 signature[] = { 0x00, 0x00, 0x03, 0xF3 };

    // Only accept the file if it fits onto a HD disk
    if (len > 1710000) return false;

    return utl::matchingBufferHeader(buf, signature, sizeof(signature));
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

    // Create a suitable ADF
    adf = *ADFFactory::make(Diameter::INCH_35, hd ? Density::HD : Density::DD);

    // Mount a file system on top of it
    auto vol = Volume(adf);
    auto fs = FileSystem(vol);

    // Format the file system and make it bootable
    fs.format(FSFormat::OFS);
    fs.setName(FSName("Disk"));
    fs.makeBootable(BootBlockId::AMIGADOS_13);

    // Add the executable, a script directory, and a script
    fs.createFile(fs.root(), FSName("file"), data);
    fs.createFile(fs.mkdir(fs.root(), FSName("s")), FSName("startup-sequence"), "file");

    // Finalize
    // fs.importer.updateChecksums();
    fs.flush();

    if (FS_DEBUG) {

        // Print some debug information about the volume
        fs.dumpState();

        // Check file system integrity
        if (FS_DEBUG) fs.doctor.xray(true, std::cout, false);
    }
}

}
