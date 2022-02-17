// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "HardDriveTypes.h"
#include "DiskTypes.h"
#include "HDFFile.h"
#include "SubComponent.h"

class HardDrive : public SubComponent {
    
    // Number of the emulated drive (dh<nr>)
    const isize nr;

    // Disk geometry
    DiskGeometry geometry;
    
    // Disk data
    u8 *data = nullptr;
    
    
    //
    // Initializing
    //

public:

    HardDrive(Amiga& ref, isize nr);
    
    // Deletes the current disk (if any)
    void dealloc();
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override;
    void _dump(dump::Category category, std::ostream& os) const override;
    
    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override;
    void _inspect() const override;
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {

    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        if (hard) {
            
        }
    }

    isize _size() override;
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override;
    isize _save(u8 *buffer) override;
    
    
    //
    // Attaching disks
    //

public:
    
    // Creates an empty disk
    void attach(const DiskGeometry &geometry) throws;
    
    // Creates a disk with the contents of the provided HDF
    void attach(const HDFFile &hdf) throws;
    
private:
    
    // Checks if the provided disk geometry or HDF is supported by the emulator
    void checkCompatibility(const DiskGeometry &geometry) throws;

    // Checks if the provided HDF is supported by the emulator
    void checkCompatibility(const HDFFile &hdf) throws;
};
