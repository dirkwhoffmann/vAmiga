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

class AmigaDisk : public AmigaObject {
    
public:
    
    bool writeProtected = false;
    bool unsaved = false;
    
    //
    // Constructing and destructing
    //
    
public:
    
    AmigaDisk();
    
  

public:
    
    bool isWriteProtected() { return writeProtected; }
    void setWriteProtection(bool value) { writeProtected = value; }
    
    bool isUnsaved() { return unsaved; }
    void setUnsaved(bool value) { unsaved = value; }
};

#endif
