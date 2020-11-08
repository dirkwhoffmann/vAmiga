//
//  DiskGeometry.cpp
//  vAmiga
//
//  Created by Dirk Hoffmann on 01.11.20.
//  Copyright © 2020 Dirk Hoffmann. All rights reserved.
//

#include "DiskGeometry.h"

DiskGeometry::DiskGeometry(DiskType type)
{
    switch (type) {
            
        case DISK_35_DD:
        case DISK_35_DD_PC:
            
            init(84, 2, 12668);
            break;
            
        case DISK_35_HD:
        case DISK_35_HD_PC:
            
            init(84, 2, 24636);
            break;
            
        case DISK_525_DD:
        case DISK_525_DD_PC:
            
            init(42, 2, 12668);
            break;
            
        default:
            assert(false);
    }
}
