// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Drive.h"
#include "SubComponent.h"
#include "IOUtils.h"

namespace vamiga {

class Drive : public SubComponent {

    //
    // Initializing
    //

public:

    Drive(Amiga& ref, isize nr);

    isize getNr() { return objid; }

    
    //
    // Querying product information
    //
    
    virtual string getDiskVendor() const { return ""; }
    virtual string getDiskProduct() const { return ""; }
    virtual string getDiskRevision() const { return ""; }
    virtual string getControllerVendor() const { return ""; }
    virtual string getControllerProduct() const { return ""; }
    virtual string getControllerRevision() const { return ""; }
    

    //
    // Analyzing
    //
    
    // Returns the connection status
    virtual bool isConnected() const = 0;

    // Returns the current read/write position
    virtual Cylinder currentCyl() const = 0;
    virtual Head currentHead() const = 0;
    virtual isize currentOffset() const = 0;

    
    //
    // Managing disks
    //
    
    // Checks whether the drive contains a disk of a certain kind
    virtual bool hasDisk() const = 0;
    virtual bool hasModifiedDisk() const = 0;
    virtual bool hasProtectedDisk() const = 0;
    bool hasUnmodifiedDisk() const { return hasDisk() && !hasModifiedDisk(); }
    bool hasUnprotectedDisk() const { return hasDisk() && !hasProtectedDisk(); }
    void toggleWriteProtection();

    // Changes the modification state
    virtual void setModificationFlag(bool value) = 0;
    virtual void setProtectionFlag(bool value) = 0;
    void markDiskAsModified() { setModificationFlag(true); }
    void markDiskAsUnmodified() { setModificationFlag(false); }
};

}
