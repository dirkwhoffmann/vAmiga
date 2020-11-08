// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "DiskGeometry.h"

DiskGeometry::DiskGeometry(DiskType t, DiskDensity d)
{
    if (t == DISK_35_DD) {
        assert(d == DISK_DD);
        init(t, d, 84, 2, 12668);
        return;
    }
    
    if (t == DISK_35_HD) {
        assert(d == DISK_HD);
        init(t, d, 84, 2, 24636);
        return;
    }
    
    if (t == DISK_525_DD) {
        assert(d == DISK_HD);
        init(t, d, 42, 2, 12668);
        return;
    }

    assert(false);
}
