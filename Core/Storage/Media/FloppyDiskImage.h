// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskImage.h"
#include "BootBlockImage.h"
#include "FloppyDiskTypes.h"
#include "DeviceDescriptors.h"
#include "FSTypes.h"
#include "DeviceTypes.h"

namespace vamiga {

class FloppyDisk;

class FloppyDiskImage : public DiskImage {

    //
    // Initializing
    //

public:

    // Gets or sets the file system for this disk
    virtual FSFormat getDos() const = 0;
    virtual void setDos(FSFormat dos) = 0;


    //
    // Methods from BlockDevice
    //

public:

    isize capacity() const override { return numBlocks(); }
    isize bsize() const override { return 512; }
    void readBlock(u8 *dst, isize nr) override { readSector(dst, nr); }
    void writeBlock(const u8 *src, isize nr) override {
        writeSector(nr, Buffer<u8>(src, bsize())); }


    //
    // Querying disk properties
    //

public:

    // Informs about the disk type
    virtual Diameter getDiameter() const = 0;
    virtual Density getDensity() const = 0;
    virtual GeometryDescriptor getGeometry() const;
    bool isSD() { return getDensity() == Density::SD; }
    bool isDD() { return getDensity() == Density::DD; }
    bool isHD() { return getDensity() == Density::HD; }


    //
    // Managing the boot blocks
    //

    virtual BootBlockType bootBlockType() const { return BootBlockType::STANDARD; }
    virtual const char *bootBlockName() const { return ""; }
    bool hasVirus() const { return bootBlockType() == BootBlockType::VIRUS; }
    virtual void killVirus() { };
};

}
