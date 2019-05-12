// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _ZORRO_MANAGER_INC
#define _ZORRO_MANAGER_INC

// Manager for plugged in Zorro II devices
class ZorroManager : public HardwareComponent {
    
    //
    // Constructing and destructing
    //
    
public:
    
    ZorroManager();
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
    
    
public:
    
    //
    // Getter and setter
    //
};

#endif
