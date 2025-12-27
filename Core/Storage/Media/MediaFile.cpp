// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "VAmiga.h"
#include "Media.h"
#include "FileFactories.h"
#include "utl/io.h"
#include <typeindex>

namespace vamiga {

FileType
MediaFile::type(const fs::path &path)
{
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
    if (fs::is_directory(path))        return FileType::DIR;

    return FileType::UNKNOWN;
}

FileType
MediaFile::type(const AnyFile &file)
{
    if (dynamic_cast<const ADFFile *>(&file))   return FileType::ADF;
    if (dynamic_cast<const ADZFile *>(&file))   return FileType::ADZ;
    if (dynamic_cast<const EADFFile *>(&file))  return FileType::EADF;
    if (dynamic_cast<const HDFFile *>(&file))   return FileType::HDF;
    if (dynamic_cast<const HDZFile *>(&file))   return FileType::HDZ;
    if (dynamic_cast<const IMGFile *>(&file))   return FileType::IMG;
    if (dynamic_cast<const STFile *>(&file))    return FileType::ST;
    if (dynamic_cast<const DMSFile *>(&file))   return FileType::DMS;
    if (dynamic_cast<const EXEFile *>(&file))   return FileType::EXE;
    if (dynamic_cast<const RomFile *>(&file))   return FileType::ROM;

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

        case FileType::ADF:          return new MediaFile(ADFFactory::make(path));
        case FileType::ADZ:          return new MediaFile(ADZFactory::make(path));
        case FileType::EADF:         return new MediaFile(make_unique<EADFFile>(path));
        case FileType::HDF:          return new MediaFile(HDFFactory::make(path));
        case FileType::HDZ:          return new MediaFile(HDZFactory::make(path));
        case FileType::IMG:          return new MediaFile(IMGFactory::make(path));
        case FileType::ST:           return new MediaFile(STFactory::make(path));
        case FileType::DMS:          return new MediaFile(DMSFactory::make(path));
        case FileType::EXE:          return new MediaFile(EXEFactory::make(path));
        case FileType::ROM:          return new MediaFile(make_unique<RomFile>(path));

        default:
            throw IOError(IOError::FILE_TYPE_MISMATCH, path);
    }
}

MediaFile *
MediaFile::make(const u8 *buf, isize len, FileType type)
{
    switch (type) {

        case FileType::ADF:          return new MediaFile(ADFFactory::make(buf, len));
        case FileType::ADZ:          return new MediaFile(ADZFactory::make(buf, len));
        case FileType::EADF:         return new MediaFile(make_unique<EADFFile>(buf, len));
        case FileType::HDF:          return new MediaFile(HDFFactory::make(buf, len));
        case FileType::HDZ:          return new MediaFile(HDZFactory::make(buf, len));
        case FileType::IMG:          return new MediaFile(IMGFactory::make(buf, len));
        case FileType::ST:           return new MediaFile(STFactory::make(buf, len));
        case FileType::DMS:          return new MediaFile(DMSFactory::make(buf, len));
        case FileType::EXE:          return new MediaFile(EXEFactory::make(buf, len));
        case FileType::ROM:          return new MediaFile(make_unique<RomFile>(buf, len));

        default:
            return nullptr;
    }
}

MediaFile *
MediaFile::make(FileSystem &fs, FileType type)
{
    switch (type) {

        case FileType::ADF:          return new MediaFile(ADFFactory::make(fs));

        default:
            return nullptr;
    }
}

MediaFile *
MediaFile::make(FloppyDriveAPI &drive, FileType type)
{
    switch (type) {

        case FileType::ADF:          return new MediaFile(ADFFactory::make(drive.getDisk()));
        case FileType::ADZ:          return new MediaFile(ADZFactory::make(drive.getDisk()));
        case FileType::EADF:         return new MediaFile(EADFFactory::make(drive.getDisk()));
        case FileType::IMG:          return new MediaFile(IMGFactory::make(drive.getDisk()));

        default:
            return nullptr;
    }
}

MediaFile *
MediaFile::make(HardDriveAPI &drive, FileType type)
{
    switch (type) {

        case FileType::HDF:      return new MediaFile(HDFFactory::make(drive.getDrive()));
        case FileType::HDZ:      return new MediaFile(HDZFactory::make(drive.getDrive()));

        default:
            return nullptr;
    }
}

FileType
MediaFile::type() const
{
    return type(*file);
}

std::pair <isize,isize>
MediaFile::previewImageSize() const
{
    if (auto snapshot = dynamic_cast<const Snapshot *>(file.get())) {
        return snapshot->previewImageSize();
    }
    return {0,0};
}

const u32 *
MediaFile::previewImageData() const
{
    if (auto snapshot = dynamic_cast<const Snapshot *>(file.get())) {
        return snapshot->previewImageData();
    }
    return nullptr;
}

Compressor
MediaFile::compressor() const
{
    if (auto snapshot = dynamic_cast<const Snapshot *>(file.get())) {
        return snapshot->compressor();
    }
    return Compressor::NONE;
}

bool
MediaFile::isCompressed() const
{
    if (auto snapshot = dynamic_cast<const Snapshot *>(file.get())) {
        return snapshot->isCompressed();
    }
    return false;
}

void
MediaFile::compress(Compressor method)
{
    if (auto snapshot = dynamic_cast<Snapshot *>(file.get())) {
        snapshot->compress(method);
    }
}

void
MediaFile::uncompress()
{
    if (auto snapshot = dynamic_cast<Snapshot *>(file.get())) {
        snapshot->uncompress();
    }
}

DiskInfo
MediaFile::getDiskInfo() const
{
    DiskInfo result;

    try {

        auto &disk = dynamic_cast<const DiskImage &>(*file);

        result.cyls = disk.numCyls();
        result.heads = disk.numHeads();
        result.sectors = disk.numSectors(0); // TODO: SECTOR COUNTS MAY VARY
        result.bsize = disk.bsize();
        result.tracks = disk.numTracks();
        result.blocks = disk.numBlocks();
        result.bytes = disk.numBytes();

        return result;

    } catch (...) {

        throw IOError(IOError::FILE_TYPE_MISMATCH);
    }
}

FloppyDiskInfo
MediaFile::getFloppyDiskInfo() const
{
    FloppyDiskInfo result;

    try {

        auto &disk = dynamic_cast<const FloppyDiskImage &>(*file);

        result.dos = disk.getDos();
        result.diameter = disk.getDiameter();
        result.density = disk.getDensity();
        result.bootBlockType = disk.bootBlockType();
        result.bootBlockName = disk.bootBlockName();
        result.hasVirus = disk.hasVirus();

        return result;

    } catch (...) {

        throw IOError(IOError::FILE_TYPE_MISMATCH);
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

        throw IOError(IOError::FILE_TYPE_MISMATCH);
    }
}

u8
MediaFile::readByte(isize b, isize offset) const
{
    if (auto *disk = dynamic_cast<const DiskImage *>(file.get())) {
        return const_cast<DiskImage *>(disk)->readByte(b * disk->bsize() + offset);
    }
    return 0;
}

u8
MediaFile::readByte(isize t, isize s, isize offset) const
{
    if (auto *disk = dynamic_cast<const DiskImage *>(file.get())) {
        return const_cast<DiskImage *>(disk)->readByte(disk->bindex(DiskImage::TS{t,s}) + offset);
    }
    return 0;
}

void
MediaFile::readSector(u8 *dst, isize b) const
{
    if (auto *disk = dynamic_cast<const DiskImage *>(file.get())) {
        const_cast<DiskImage *>(disk)->readBlock(dst, b);
    }
}

void
MediaFile::readSector(u8 *dst, isize t, isize s) const
{
    if (auto *disk = dynamic_cast<const DiskImage *>(file.get())) {
        const_cast<DiskImage *>(disk)->readBlock(dst, disk->bindex(DiskImage::TS{t,s}));
    }
}

}
