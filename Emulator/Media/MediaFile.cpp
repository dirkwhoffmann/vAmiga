// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "MediaFile.h"
#include "VAmiga.h"
#include "ADFFile.h"
#include "DMSFile.h"
#include "EADFFile.h"
#include "EXEFile.h"
#include "ExtendedRomFile.h"
#include "FloppyFile.h"
#include "Folder.h"
#include "HDFFile.h"
#include "IMGFile.h"
#include "Script.h"
#include "Snapshot.h"
#include "RomFile.h"
#include "STFile.h"

namespace vamiga {

FileType
MediaFile::type(const fs::path &path)
{
    Buffer<u8> buffer(path);

    if (!buffer.empty()) {

        if (Snapshot::isCompatible(path) &&
            Snapshot::isCompatible(buffer)) return FILETYPE_SNAPSHOT;

        if (Script::isCompatible(path) &&
            Script::isCompatible(buffer)) return FILETYPE_SCRIPT;

        if (ADFFile::isCompatible(path) &&
            ADFFile::isCompatible(buffer)) return FILETYPE_ADF;

        if (EADFFile::isCompatible(path) &&
            EADFFile::isCompatible(buffer)) return FILETYPE_EADF;

        if (HDFFile::isCompatible(path) &&
            HDFFile::isCompatible(buffer)) return FILETYPE_HDF;

        if (IMGFile::isCompatible(path) &&
            IMGFile::isCompatible(buffer)) return FILETYPE_IMG;

        if (STFile::isCompatible(path) &&
            STFile::isCompatible(buffer)) return FILETYPE_ST;

        if (DMSFile::isCompatible(path) &&
            DMSFile::isCompatible(buffer)) return FILETYPE_DMS;

        if (EXEFile::isCompatible(path) &&
            EXEFile::isCompatible(buffer)) return FILETYPE_EXE;

        if (RomFile::isCompatible(path) &&
            RomFile::isCompatible(buffer)) return FILETYPE_ROM;

        if (Folder::isCompatible(path)) return FILETYPE_DIR;
    }

    return FILETYPE_UNKNOWN;
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

        case FILETYPE_SNAPSHOT:     return new Snapshot(path);
        case FILETYPE_SCRIPT:       return new Script(path);
        case FILETYPE_ADF:          return new ADFFile(path);
        case FILETYPE_EADF:         return new EADFFile(path);
        case FILETYPE_HDF:          return new HDFFile(path);
        case FILETYPE_IMG:          return new IMGFile(path);
        case FILETYPE_ST:           return new STFile(path);
        case FILETYPE_DMS:          return new DMSFile(path);
        case FILETYPE_EXE:          return new EXEFile(path);
        case FILETYPE_ROM:          return new RomFile(path);
        case FILETYPE_EXTENDED_ROM: return new ExtendedRomFile(path);

        default:
            return nullptr;
    }
}

MediaFile *
MediaFile::make(const u8 *buf, isize len, FileType type)
{
    switch (type) {

        case FILETYPE_SNAPSHOT:     return new Snapshot(buf, len);
        case FILETYPE_SCRIPT:       return new Script(buf, len);
        case FILETYPE_ADF:          return new ADFFile(buf, len);
        case FILETYPE_EADF:         return new EADFFile(buf, len);
        case FILETYPE_HDF:          return new HDFFile(buf, len);
        case FILETYPE_IMG:          return new IMGFile(buf, len);
        case FILETYPE_ST:           return new STFile(buf, len);
        case FILETYPE_DMS:          return new DMSFile(buf, len);
        case FILETYPE_EXE:          return new EXEFile(buf, len);
        case FILETYPE_ROM:          return new RomFile(buf, len);
        case FILETYPE_EXTENDED_ROM: return new ExtendedRomFile(buf, len);

        default:
            return nullptr;
    }
}

MediaFile *
MediaFile::make(class MutableFileSystem &fs, FileType type)
{
    switch (type) {

        case FILETYPE_ADF:        return new ADFFile(fs);

        default:
            return nullptr;
    }
}

MediaFile *
MediaFile::make(FloppyDriveAPI &drive, FileType type)
{
    switch (type) {

        case FILETYPE_ADF:      return new ADFFile(drive.getDisk());
        case FILETYPE_EADF:     return new EADFFile(drive.getDisk());
        case FILETYPE_IMG:      return new IMGFile(drive.getDisk());

        default:
            return nullptr;
    }
}

MediaFile *
MediaFile::make(HardDriveAPI &drive, FileType type)
{
    switch (type) {

        case FILETYPE_HDF:      return new HDFFile(drive.getDrive());

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

        throw Error(VAERROR_FILE_TYPE_MISMATCH);
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

        throw Error(VAERROR_FILE_TYPE_MISMATCH);
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

        throw Error(VAERROR_FILE_TYPE_MISMATCH);
    }
}

}

