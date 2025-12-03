// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "ADFFactory.h"
#include "BootBlockImage.h"
#include "Checksum.h"
#include "EADFFile.h"
#include "IOUtils.h"
#include "MemUtils.h"
#include "FileSystem.h"

namespace vamiga {

bool
ADFFile::isCompatible(const fs::path &path)
{
    // Check the suffix
    auto suffix = util::uppercased(path.extension().string());
    if (suffix != ".ADF") return false;
    
    // Make sure it's not an extended ADF
    return !EADFFile::isCompatible(path);
}

bool
ADFFile::isCompatible(const u8 *buf, isize len)
{
    // Some ADFs contain an additional byte at the end. Ignore it.
    len &= ~1;

    // The size must be a multiple of the cylinder size
    if (len % 11264) return false;

    // Check some more limits
    return len <= ADFSIZE_35_DD_84 || len == ADFSIZE_35_HD;
}

bool
ADFFile::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
}

isize
ADFFile::fileSize(Diameter diameter, Density density)
{
    assert_enum(Diameter, diameter);
    assert_enum(Density, density);

    if (diameter != Diameter::INCH_35) throw AppError(Fault::DISK_INVALID_DIAMETER);
    
    if (density == Density::DD) return ADFSIZE_35_DD;
    if (density == Density::HD) return ADFSIZE_35_HD;

    throw AppError(Fault::DISK_INVALID_DENSITY);
}

/*
void
ADFFile::init(Diameter diameter, Density density)
{
    assert_enum(Diameter, diameter);
    assert(data.empty());
    
    data.init(fileSize(diameter, density));
}

void
ADFFile::init(const FloppyDiskDescriptor &descr)
{
    if (descr.diameter != Diameter::INCH_35) throw AppError(Fault::DISK_INVALID_DIAMETER);

    switch (descr.density) {

        case Density::DD:

            switch (descr.cylinders) {

                case 80: init(ADFSIZE_35_DD); break;
                case 81: init(ADFSIZE_35_DD_81); break;
                case 82: init(ADFSIZE_35_DD_82); break;
                case 83: init(ADFSIZE_35_DD_83); break;
                case 84: init(ADFSIZE_35_DD_84); break;

                default:
                    throw AppError(Fault::DISK_INVALID_LAYOUT);
            }
            break;

        case Density::HD:

            init(ADFSIZE_35_HD);
            break;

        default:
            throw AppError(Fault::DISK_INVALID_DENSITY);
    }
}

void
ADFFile::init(const FloppyDisk &disk)
{
    init(disk.getDiameter(), disk.getDensity());
    
    assert(numTracks() == 160);
    assert(numSectors() == 11 || numSectors() == 22);
    
    decodeDisk(disk);
}

void
ADFFile::init(const FloppyDrive &drive)
{
    if (drive.disk == nullptr) throw AppError(Fault::DISK_MISSING);
    init(*drive.disk);
}

void
ADFFile::init(const FileSystem &volume)
{
    switch (volume.blocks()) {

        case 2 * 880:
            init(Diameter::INCH_35, Density::DD);
            break;
            
        case 4 * 880:
            init(Diameter::INCH_35, Density::HD);
            break;
            
        default:
            throw AppError(Fault::FS_WRONG_CAPACITY);
    }

    volume.exporter.exportVolume(data.ptr, data.size);
}
*/

void
ADFFile::finalizeRead()
{
    // Add some empty cylinders if the file contains less than 80
    if (data.size < ADFSIZE_35_DD) data.resize(ADFSIZE_35_DD, 0);
}

isize
ADFFile::numCyls() const
{
    switch(data.size & ~1) {
            
        case ADFSIZE_35_DD:    return 80;
        case ADFSIZE_35_DD_81: return 81;
        case ADFSIZE_35_DD_82: return 82;
        case ADFSIZE_35_DD_83: return 83;
        case ADFSIZE_35_DD_84: return 84;
        case ADFSIZE_35_HD:    return 80;
            
        default:
            fatalError;
    }
}

isize
ADFFile::numHeads() const
{
    return 2;
}

isize
ADFFile::numSectors() const
{
    switch (getDensity()) {
            
        case Density::DD: return 11;
        case Density::HD: return 22;
            
        default:
            fatalError;
    }
}

FSFormat
ADFFile::getDos() const
{
    if (strncmp((const char *)data.ptr, "DOS", 3) || data[3] > 7) {
        return FSFormat::NODOS;
    }

    return (FSFormat)data[3];
}

void
ADFFile::setDos(FSFormat dos)
{
    if (dos == FSFormat::NODOS) {
        std::memset(data.ptr, 0, 4);
    } else {
        std::memcpy(data.ptr, "DOS", 3);
        data[3] = (u8)dos;
    }
}

Diameter
ADFFile::getDiameter() const
{
    return Diameter::INCH_35;
}

Density
ADFFile::getDensity() const
{
    return (data.size & ~1) == ADFSIZE_35_HD ? Density::HD : Density::DD;
}

FSDescriptor
ADFFile::getFileSystemDescriptor() const
{
    FSDescriptor result;
    
    // Determine the root block location
    Block root = data.size < ADFSIZE_35_HD ? 880 : 1760;

    // Determine the bitmap block location
    Block bitmap = FSBlock::read32(data.ptr + root * 512 + 316);
    
    // Assign a default location if the bitmap block reference is invalid
    if (bitmap == 0 || bitmap >= (Block)numBlocks()) bitmap = root + 1;

    // Setup the descriptor
    result.numBlocks = numBlocks();
    result.bsize = 512;
    result.numReserved = 2;
    result.dos = getDos();
    result.rootBlock = root;
    result.bmBlocks.push_back(bitmap);
    
    return result;
}

BootBlockType
ADFFile::bootBlockType() const
{
    return BootBlockImage(data.ptr).type;
}

const char *
ADFFile::bootBlockName() const
{
    return BootBlockImage(data.ptr).name;
}

void
ADFFile::killVirus()
{
    debug(ADF_DEBUG, "Overwriting boot block virus with ");
    
    if (isOFSVolumeType(getDos())) {

        debug(ADF_DEBUG, "a standard OFS bootblock\n");
        BootBlockImage bb = BootBlockImage(BootBlockId::AMIGADOS_13);
        bb.write(data.ptr + 4, 4, 1023);

    } else if (isFFSVolumeType(getDos())) {

        debug(ADF_DEBUG, "a standard FFS bootblock\n");
        BootBlockImage bb = BootBlockImage(BootBlockId::AMIGADOS_20);
        bb.write(data.ptr + 4, 4, 1023);

    } else {

        debug(ADF_DEBUG, "zeroes\n");
        std::memset(data.ptr + 4, 0, 1020);
    }
}

void
ADFFile::formatDisk(FSFormat fs, BootBlockId id, string name)
{
    assert_enum(FSFormat, fs);

    debug(ADF_DEBUG,
          "Formatting disk (%ld, %s)\n", numBlocks(), FSFormatEnum::key(fs));

    // Only proceed if a file system is given
    if (fs == FSFormat::NODOS) return;
    
    // Get a device descriptor for this ADF
    auto descriptor = getFileSystemDescriptor();
    descriptor.dos = fs;
    
    // Create an empty file system
    FileSystem volume(descriptor);
    volume.setName(FSName(name));
    
    // Write boot code
    volume.makeBootable(id);
    
    // Export the file system to the ADF
    if (!volume.exporter.exportVolume(data.ptr, data.size)) throw FSError(FSFault::FS_UNKNOWN);
}

void
ADFFile::dumpSector(Sector s) const
{
    util::hexdump(data.ptr + 512 * s, 512);
}

}
