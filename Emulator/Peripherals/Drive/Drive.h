// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Drive.h"
#include "SubComponent.h"

class Drive : public SubComponent {

protected:
    
    // Number of the emulated drive (0 = df0 or hd0, 1 = df1 or hd1, etc.)
    const isize nr;
    
    
    //
    // Initializing
    //

public:

    Drive(Amiga& ref, isize nr);
        
    
    //
    // Analyzing
    //

    isize getNr() { return nr; }
    

    //
    // Managing disk properties
    //
    
    // Returns a fingerprint for the current disk or 0 if there is none
    virtual u64 fnv() const = 0;

    // Checks whether the drive contains a disk of a certain kind
    virtual bool hasDisk() const = 0;
    virtual bool hasModifiedDisk() const = 0;
    virtual bool hasProtectedDisk() const = 0;

    // Changes the modification state
    virtual void setModificationFlag(bool value) = 0;
    virtual void setProtectionFlag(bool value) = 0;
    
    // Convenience wrappers
    bool hasUnmodifiedDisk() const { return hasDisk() && !hasModifiedDisk(); }
    bool hasUnprotectedDisk() const { return hasDisk() && !hasProtectedDisk(); }
    void markDiskAsModified() { setModificationFlag(true); }
    void markDiskAsUnmodified() { setModificationFlag(false); }
    void toggleWriteProtection();
};
