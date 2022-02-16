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
#include "SubComponent.h"

class HardDrive : public SubComponent {
    
    // Number of the emulated drive (0 = dh0, 1 = dh1, etc.)
    const isize nr;

    
    //
    // Initializing
    //

public:

    HardDrive(Amiga& ref, isize nr);
    
    
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
};
