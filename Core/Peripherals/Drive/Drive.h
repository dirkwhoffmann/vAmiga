// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DriveTypes.h"
#include "ImageTypes.h"
#include "SubComponent.h"

namespace vamiga {

class Drive : public SubComponent {

    //
    // Initializing
    //

public:

    using SubComponent::SubComponent;

    
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
    virtual CylNr currentCyl() const = 0;
    virtual HeadNr currentHead() const = 0;
    virtual isize currentOffset() const = 0;

    
    //
    // Managing disks
    //
    
    // Checks whether the drive contains a disk of a certain kind
    virtual bool hasDisk() const = 0;
    virtual bool hasModifiedDisk() const = 0;
    virtual bool hasProtectedDisk() const = 0;

    // Gets or sets a disk flag
    virtual bool getFlag(DiskFlags mask) const = 0;
    virtual void setFlag(DiskFlags mask, bool value) = 0;
    void setFlag(DiskFlags mask) { setFlag(mask, true); }
    void clearFlag(DiskFlags mask) { setFlag(mask, false); }

    bool hasUnmodifiedDisk() const { return hasDisk() && !hasModifiedDisk(); }
    bool hasUnprotectedDisk() const { return hasDisk() && !hasProtectedDisk(); }
    void toggleWriteProtection();

    // Changes the modification state
    virtual void setModificationFlag(bool value) = 0;
    virtual void setProtectionFlag(bool value) = 0;
    void markDiskAsModified() { setModificationFlag(true); }
    void markDiskAsUnmodified() { setModificationFlag(false); }


    //
    // Managing storage
    //

    /* Where the disk lives, and whether there is a file to write it back to.
     *
     * A memory-backed disk has no file: getPath() is empty, needsPersisting()
     * is false, and persist() does nothing. Floppy drives are always
     * memory-backed.
     *
     * needsPersisting() is about the file only. Whether a disk has been
     * modified since it was inserted, which is what an unsaved-changes
     * warning wants to know, is hasModifiedDisk(), for either kind of disk.
     */

    // Returns where the disk lives (a drive without a disk counts as memory-backed)
    virtual StorageMode getStorageMode() const = 0;

    // Returns the file the disk lives in (empty for a memory-backed disk)
    virtual fs::path getPath() const = 0;

    // Returns true if the disk holds changes its file does not have yet
    virtual bool needsPersisting() const = 0;

    // Writes all changes back to the file the disk lives in
    virtual void persist() = 0;
};

}
