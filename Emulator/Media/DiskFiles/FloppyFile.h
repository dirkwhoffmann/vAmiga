// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskFile.h"
#include "BootBlockImage.h"
#include "FloppyDisk.h"

namespace vamiga {

class FloppyDisk;

class FloppyFile : public DiskFile {

    //
    // Creating
    //

public:

    static FloppyFile *make(const std::filesystem::path &path) throws;


    //
    // Initializing
    //

public:

    // Gets or sets the file system for this disk
    virtual FSVolumeType getDos() const = 0;
    virtual void setDos(FSVolumeType dos) = 0;


    //
    // Querying disk properties
    //

public:

    // Informs about the disk type
    virtual Diameter getDiameter() const = 0;
    virtual Density getDensity() const = 0;
    virtual FloppyDiskDescriptor getDescriptor() const;
    bool isSD() { return getDensity() == DENSITY_SD; }
    bool isDD() { return getDensity() == DENSITY_DD; }
    bool isHD() { return getDensity() == DENSITY_HD; }

    // Analyzes the boot block
    virtual BootBlockType bootBlockType() const { return BB_STANDARD; }
    virtual const char *bootBlockName() const { return ""; }
    bool hasVirus() const { return bootBlockType() == BB_VIRUS; }


    //
    // Repairing
    //

    virtual void killVirus() { };


    //
    // Encoding
    //

public:

    virtual void encodeDisk(FloppyDisk &disk) const throws { fatalError; }
    virtual void decodeDisk(FloppyDisk &disk) throws { fatalError; }
};

}
