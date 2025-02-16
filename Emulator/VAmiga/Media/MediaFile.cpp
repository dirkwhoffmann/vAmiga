// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
#include "VAmiga.h"
#include "Media.h"

namespace vamiga {

FileType
MediaFile::type(const fs::path &path)
{
    if (Workspace::isCompatible(path)) return FileType::WORKSPACE;
    if (Snapshot::isCompatible(path))  return FileType::SNAPSHOT;
    if (Script::isCompatible(path))    return FileType::SCRIPT;
    if (ADFFile::isCompatible(path))   return FileType::ADF;
    if (ADZFile::isCompatible(path))   return FileType::ADZ;
    if (EADFFile::isCompatible(path))  return FileType::EADF;
    if (HDFFile::isCompatible(path))   return FileType::HDF;
    if (HDZFile::isCompatible(path))   return FileType::HDZ;
    if (IMGFile::isCompatible(path))   return FileType::IMG;
    if (STFile::isCompatible(path))    return FileType::ST;
    if (DMSFile::isCompatible(path))   return FileType::DMS;
    if (EXEFile::isCompatible(path))   return FileType::EXE;
    if (RomFile::isCompatible(path))   return FileType::ROM;
    if (Folder::isCompatible(path))    return FileType::DIR;

    return FileType::UNKNOWN;
}

MediaFile *
MediaFile::make(const fs::path &path)
{
    return make(path, type(path));
}

MediaFile *
MediaFile::make(const fs::path &path, FileType type)
{
    switch (type) {

        case FileType::WORKSPACE:    return new Workspace(path);
        case FileType::SNAPSHOT:     return new Snapshot(path);
        case FileType::SCRIPT:       return new Script(path);
        case FileType::ADF:          return new ADFFile(path);
        case FileType::ADZ:          return new ADZFile(path);
        case FileType::EADF:         return new EADFFile(path);
        case FileType::HDF:          return new HDFFile(path);
        case FileType::HDZ:          return new HDZFile(path);
        case FileType::IMG:          return new IMGFile(path);
        case FileType::ST:           return new STFile(path);
        case FileType::DMS:          return new DMSFile(path);
        case FileType::EXE:          return new EXEFile(path);
        case FileType::ROM:          return new RomFile(path);
        case FileType::EXTENDED_ROM: return new ExtendedRomFile(path);

        default:
            throw CoreError(Fault::FILE_TYPE_MISMATCH, path);
    }
}

MediaFile *
MediaFile::make(const u8 *buf, isize len, FileType type)
{
    switch (type) {

        case FileType::SNAPSHOT:     return new Snapshot(buf, len);
        case FileType::SCRIPT:       return new Script(buf, len);
        case FileType::ADF:          return new ADFFile(buf, len);
        case FileType::ADZ:          return new ADZFile(buf, len);
        case FileType::EADF:         return new EADFFile(buf, len);
        case FileType::HDF:          return new HDFFile(buf, len);
        case FileType::HDZ:          return new HDZFile(buf, len);
        case FileType::IMG:          return new IMGFile(buf, len);
        case FileType::ST:           return new STFile(buf, len);
        case FileType::DMS:          return new DMSFile(buf, len);
        case FileType::EXE:          return new EXEFile(buf, len);
        case FileType::ROM:          return new RomFile(buf, len);
        case FileType::EXTENDED_ROM: return new ExtendedRomFile(buf, len);

        default:
            return nullptr;
    }
}

MediaFile *
MediaFile::make(class MutableFileSystem &fs, FileType type)
{
    switch (type) {

        case FileType::ADF:          return new ADFFile(fs);

        default:
            return nullptr;
    }
}

MediaFile *
MediaFile::make(FloppyDriveAPI &drive, FileType type)
{
    switch (type) {

        case FileType::ADF:          return new ADFFile(drive.getDisk());
        case FileType::ADZ:          return new ADZFile(drive.getDisk());
        case FileType::EADF:         return new EADFFile(drive.getDisk());
        case FileType::IMG:          return new IMGFile(drive.getDisk());

        default:
            return nullptr;
    }
}

MediaFile *
MediaFile::make(HardDriveAPI &drive, FileType type)
{
    switch (type) {

        case FileType::HDF:      return new HDFFile(drive.getDrive());
        case FileType::HDZ:      return new HDZFile(drive.getDrive());

        default:
            return nullptr;
    }
}

string
MediaFile::getSizeAsString() const
{
    return util::byteCountAsString(getSize());
}

DiskInfo
MediaFile::getDiskInfo() const
{
    DiskInfo result;

    try {

        auto &disk = dynamic_cast<const DiskFile &>(*this);

        result.cyls = disk.numCyls();
        result.heads = disk.numHeads();
        result.sectors = disk.numSectors();
        result.bsize = disk.bsize();
        result.tracks = disk.numTracks();
        result.blocks = disk.numBlocks();
        result.bytes = disk.numBytes();

        return result;

    } catch (...) {

        throw CoreError(Fault::FILE_TYPE_MISMATCH);
    }
}

FloppyDiskInfo
MediaFile::getFloppyDiskInfo() const
{
    FloppyDiskInfo result;

    try {

        auto &disk = dynamic_cast<const FloppyFile &>(*this);

        result.dos = disk.getDos();
        result.diameter = disk.getDiameter();
        result.density = disk.getDensity();
        result.bootBlockType = disk.bootBlockType();
        result.bootBlockName = disk.bootBlockName();
        result.hasVirus = disk.hasVirus();

        return result;

    } catch (...) {

        throw CoreError(Fault::FILE_TYPE_MISMATCH);
    }
}

HDFInfo
MediaFile::getHDFInfo() const
{
    HDFInfo result;

    try {

        auto &hdf = dynamic_cast<const HDFFile &>(*this);

        result.partitions = hdf.numPartitions();
        result.drivers = hdf.numDrivers();
        result.hasRDB = hdf.hasRDB();

        return result;

    } catch (...) {

        throw CoreError(Fault::FILE_TYPE_MISMATCH);
    }
}

}

