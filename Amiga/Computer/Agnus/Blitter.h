// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _BLITTER_INC
#define _BLITTER_INC

class Blitter : public HardwareComponent {
    
    friend class DMAController;
    
 
    // The Blitter Control Register
    uint16_t bltcon0;
    uint16_t bltcon1;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Blitter();
    
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
    // Collecting information
    //
    
public:
    
    // Collects the data shown in the GUI's debug panel.
    BlitterInfo getInfo();
    
    
    //
    // Accessing registers
    //
    
public:
    
    void pokeBLTCON0(uint16_t value);
    void pokeBLTCON1(uint16_t value);

    inline bool blitADMA() { return bltcon0 & (1 << 11); }
    inline bool blitBDMA() { return bltcon0 & (1 << 10); }
    inline bool blitCDMA() { return bltcon0 & (1 << 9); }
    inline bool blitDDMA() { return bltcon0 & (1 << 8); }

    //
    // Running the device
    //
    
private:
    
  
    
    //
    // Managing events
    //
 
    // Cancels the current Blitter event
    void cancelEvent();
    
    // Processes a Blitter event
    void serviceEvent(EventID id, int64_t data);
    
  
};

#endif
