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
    
    long cylinders;
    long sides;

    // Track lengths in bytes
    union {
        u64 cylinder[84][2];
        u64 track[168];
    } length;
    
    void init(long cylinders, long sides, long len) {
        
        for (int i = 0; i < 168; i++) {
            length.track[i] = len;
        }
        
        this->cylinders = cylinders;
        this->sides = sides;
    }
    
    template <class T>
    void applyToItems(T& worker)
    {
        worker

        & cylinders
        & sides
        & length.track;
    }
    
    DiskGeometry(DiskType type);
    
    long numTracks() { return cylinders * sides; }
};

#endif
