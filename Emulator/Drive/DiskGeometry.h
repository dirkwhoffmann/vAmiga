// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DISK_GEOMETRY_H
#define _DISK_GEOMETRY_H

#include "DiskTypes.h"
#include "stdio.h"

/* MFM encoded disk data of a standard 3.5" DD disk:
 *
 *    Cylinder  Track     Head      Sectors
 *    ---------------------------------------
 *    0         0         0          0 - 10
 *    0         1         1         11 - 21
 *    1         2         0         22 - 32
 *    1         3         1         33 - 43
 *                   ...
 *    79        158       0       1738 - 1748
 *    79        159       1       1749 - 1759
 *
 *    80        160       0       1760 - 1770   <--- beyond spec
 *    80        161       1       1771 - 1781
 *                   ...
 *    83        166       0       1826 - 1836
 *    83        167       1       1837 - 1847
 *
 * A single sector consists of
 *    - A sector header build up from 64 MFM bytes.
 *    - 512 bytes of data (1024 MFM bytes).
 *
 * Hence,
 *    - a sector consists of 64 + 2*512 = 1088 MFM bytes.
 *
 * A single track of a 3.5"DD disk consists
 *    - 11 * 1088 = 11.968 MFM bytes.
 *    - A track gap of about 700 MFM bytes (varies with drive speed).
 *
 * Hence,
 *    - a track usually occupies 11.968 + 700 = 12.668 MFM bytes.
 *    - a cylinder usually occupies 25.328 MFM bytes.
 *    - a disk usually occupies 84 * 2 * 12.664 =  2.127.552 MFM bytes
 */

struct DiskGeometry {
    
    // Geometry parameters
    long cylinders;
    long sides;
    long sectors;
    
    // Constants
    static const long sectorSize = 1088;
    static const long trackGapSize = 700;

    // Derived parameters
    long trackSize;
    long cylinderSize;
    long diskSize;
    
    void init(long cylinders, long sides, long sectors) {
        
        this->cylinders = cylinders;
        this->sides = sides;
        this->sectors = sectors;
        
        trackSize = sectors * sectorSize + trackGapSize;
        cylinderSize = sides * trackSize;
        diskSize = cylinders * cylinderSize;
    }
    
    DiskGeometry(DiskType type) {
        
        printf("DiskGeometry(%ld)\n", type);
        
        switch (type) {
                
            case DISK_35_DD: init(84, 2, 11);

                printf("DISK_35_DD\n");

                assert(trackSize == 12668);
                assert(cylinderSize == 25336);
                assert(diskSize == 2128224);
                break;
                
            case DISK_35_DD_PC: init(84, 2, 9);
                
                assert(trackSize == 10492);
                assert(cylinderSize == 20984);
                assert(diskSize == 1762656);
                break;
                
            case DISK_35_HD: init(84, 2, 22);
                
                assert(trackSize == 25336);
                assert(cylinderSize == 50672);
                assert(diskSize == 4256448);
                break;
                
            case DISK_35_HD_PC: init(84, 2, 18);
                
                assert(trackSize == 20984);
                assert(cylinderSize == 41968);
                assert(diskSize == 3525312);
                break;
                
            case DISK_525_DD: init(42, 2, 11);
                
                assert(trackSize == 6334);
                assert(cylinderSize == 12668);
                assert(diskSize == 1064112);
                break;
                
            case DISK_525_DD_PC: init(42, 2, 9);
                
                assert(trackSize == 5246);
                assert(cylinderSize == 10492);
                assert(diskSize == 881328);
                break;
                
            default:
                assert(false);
        }
    }
};

#endif
