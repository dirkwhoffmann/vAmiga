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
    
    // Blitter A first and last word masks
    uint16_t bltafwm;
    uint16_t bltalwm;

    // The Blitter size register
    uint16_t bltsize;
    
    // The Blitter modulo registers
    uint16_t bltamod;
    uint16_t bltbmod;
    uint16_t bltcmod;
    uint16_t bltdmod;

    // The Blitter data registers
    uint16_t bltadat;
    uint16_t bltbdat;
    uint16_t bltcdat;

    
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
    
    // BLTCON0
    inline bool blitADMA() { return bltcon0 & (1 << 11); }
    inline bool blitBDMA() { return bltcon0 & (1 << 10); }
    inline bool blitCDMA() { return bltcon0 & (1 << 9); }
    inline bool blitDDMA() { return bltcon0 & (1 << 8); }
    void pokeBLTCON0(uint16_t value);

    // BLTCON1
    void pokeBLTCON1(uint16_t value);

    // BLTAFWM, BLTALWM
    void pokeBLTAFWM(uint16_t value);
    void pokeBLTALWM(uint16_t value);

    // BLTSIZE
    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // H9 H8 H7 H6 H5 H4 H3 H2 H1 H0 W5 W4 W3 W2 W1 W0
    inline uint16_t bltsizeH() { return bltsize >> 6; }
    inline uint16_t bltsizeW() { return bltsize & 0x3F; }
    void pokeBLTSIZE(uint16_t value);

    // BLTxMOD
    void pokeBLTAMOD(uint16_t value) { bltamod = value; }
    void pokeBLTBMOD(uint16_t value) { bltbmod = value; }
    void pokeBLTCMOD(uint16_t value) { bltcmod = value; }
    void pokeBLTDMOD(uint16_t value) { bltdmod = value; }
    
    // BLTxDAT
    void pokeBLTADAT(uint16_t value) { bltadat = value; }
    void pokeBLTBDAT(uint16_t value) { bltbdat = value; }
    void pokeBLTCDAT(uint16_t value) { bltcdat = value; }

    
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
