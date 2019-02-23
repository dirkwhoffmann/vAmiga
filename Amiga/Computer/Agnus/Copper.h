// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------


#ifndef _COPPER_INC
#define _COPPER_INC

class Copper : public HardwareComponent {
   
    // The Copper Danger Bit (CDANG)
    bool cdang;
    
    // The Copper instruction register
    uint16_t copins = 0;
    
    // The Copper program counter
    uint32_t coppc = 0;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Copper();
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
    
    
    //
    // Accessing registers
    //
    
public:
    
    void pokeCOPCON(uint16_t value);
    void pokeCOPJMP(int x);
    void pokeCOPINS(uint16_t value);
    
private:
    
    // Returns true if the Copper has no access to this custom register
    bool illegalAddress(uint32_t address);
};

#endif 
