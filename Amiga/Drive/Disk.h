// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_DISK_INC
#define _AMIGA_DISK_INC

#include "HardwareComponent.h"

//
// THIS CLASS IS A STUB TO MAKE THE VISUAL PROTOTYPE WORK
//

class Disk : public AmigaObject {
    
public:
    
    //
    // Constants
    //
    
    /* MFM encoded disk data
     *
     * Track   Cylinder   Head   Sectors
     * -------------------------------------
     * 0      0         0         0 -   10
     * 1      0         1        11 -   21
     * 2      1         0        22 -   32
     * 3      1         1        33 -   43
     * ...
     * 158     79         0      1738 - 1748
     * 159     79         1      1749 - 1759
     */
    
    static const int numHeads           = 2;
    static const int numCylinders       = 80;
    static const int numSectorsPerTrack = 11;
    static const int numTracks          = numHeads * numCylinders;
    static const int numSectors         = numSectorsPerTrack * numTracks;
   
    /* A single sector consists of
     *    - A sector header build up from 64 MFM bytes.
     *    - 512 bytes of data (1024 MFM bytes).
     * Hence,
     *    - a sector consists of 64 + 2*512 = 1088 MFM bytes.
     *
     * A single track consists of
     *    - 11 * 1088 = 11.968 MFM bytes.
     *    - A track gap of about 696 MFM bytes (varies with drive speed).
     * Hence,
     *    - a track usually occupies 11.968 + 696 = 12.664 MFM bytes.
     *    - a cylinder usually occupies 25.328 MFM bytes.
     *    - a disk usually occupies 80 * 2 * 12.664 =  2.026.240 MFM bytes
     */
    
    static const long mfmBytesPerSector   = 1088;
    static const long mfmBytesInTrackGap  = 696;
    static const long mfmBytesPerTrack    = 12664;
    static const long mfmBytesPerCylinder = 25328;
    static const long mfmBytesPerDisk     = 2026240; 
    
    // MFM encoded disk data
    
    union {
        uint8_t raw[mfmBytesPerDisk];
        uint8_t track[160][mfmBytesPerTrack];
        uint8_t cyclinder[80][2][mfmBytesPerTrack];
    } data;
    
    bool writeProtected = false;
    bool modified = false;
    
    //
    // Constructing and destructing
    //
    
public:
    
    Disk();
    
  

public:
    
    bool isWriteProtected() { return writeProtected; }
    void setWriteProtection(bool value) { writeProtected = value; }
    
    bool isModified() { return modified; }
    void setModified(bool value) { modified = value; }
};

#endif
