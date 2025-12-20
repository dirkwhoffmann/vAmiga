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

    // Mount a file system on top of the ADF
    auto vol = Volume(adf);
    auto fs = FileSystem(vol);

    // Format the file system
    fs.format(FSFormat::OFS);

    // Name the file system
    fs.setName(FSName("Disk"));

    // Make the volume bootable
    fs.makeBootable(BootBlockId::AMIGADOS_13);

    // Start at the root directory
    auto dir = fs.root();

    // Add the executable
    fs.createFile(dir, FSName("file"), data);

    // Add a script directory
    dir = fs.mkdir(dir, FSName("s"));

    // Add a startup sequence
    fs.createFile(dir, "startup-sequence", "file");

    /*
    if (auto *dir = &fs.deprecatedRoot(); dir) {

        // Add the executable
        fs.createFile(*dir, FSName("file"), data);

        // Add a script directory
        dir = &fs.mkdir(*dir, FSName("s"));

        // Add a startup sequence
        fs.createFile(*dir, "startup-sequence", "file");
    }
    */

    // Finalize
    fs.importer.updateChecksums();

    // Print some debug information about the volume
    if (FS_DEBUG) fs.dumpState();

    // Check file system integrity
    if (FS_DEBUG) fs.doctor.xray(true, std::cout, false);

    // Write back
    fs.flush();

    /*
    // Create a new file system
    auto dev = make_unique<Device>(Diameter::INCH_35, hd ? Density::HD : Density::DD);
    auto volume = FileSystemFactory::createLowLevel(*dev, Diameter::INCH_35, hd ? Density::HD : Density::DD, FSFormat::OFS);
    volume->setName(FSName("Disk"));

    // Make the volume bootable
    volume->makeBootable(BootBlockId::AMIGADOS_13);

    // Start at the root directory
    if (auto *dir = &volume->root(); dir) {

        // Add the executable
        volume->createFile(*dir, FSName("file"), data);

        // Add a script directory
        dir = &volume->mkdir(*dir, FSName("s"));

        // Add a startup sequence
        volume->createFile(*dir, "startup-sequence", "file");
    }

    // Finalize
    volume->importer.updateChecksums();

    // Print some debug information about the volume
    if (FS_DEBUG) volume->dumpState();

    // Check file system integrity
    if (FS_DEBUG) volume->doctor.xray(true, std::cout, false);

    // Convert the volume into an ADF
    // adf.init(volume);
    adf = *ADFFactory::make(*volume);
    */
}

}
