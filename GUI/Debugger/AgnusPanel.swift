// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

extension Inspector {
    
    func refreshAgnus(everything: Bool) {
        
        guard let dma = amigaProxy?.dma else { return }
        let info = dma.getInfo()
        
        track("Refreshing Agnus inspector tab")
        
        if everything {
            
        }
        
        // DMA control
        dmaDMACON.intValue = Int32(info.dmacon)
        dmaBLTPRI.state = (info.dmacon & 0b0000010000000000 != 0) ? .on : .off
        dmaDMAEN.state  = (info.dmacon & 0b0000001000000000 != 0) ? .on : .off
        dmaBPLEN.state  = (info.dmacon & 0b0000000100000000 != 0) ? .on : .off
        dmaCOPEN.state  = (info.dmacon & 0b0000000010000000 != 0) ? .on : .off
        dmaBLTEN.state  = (info.dmacon & 0b0000000001000000 != 0) ? .on : .off
        dmaSPREN.state  = (info.dmacon & 0b0000000000100000 != 0) ? .on : .off
        dmaDSKEN.state  = (info.dmacon & 0b0000000000010000 != 0) ? .on : .off
        dmaAUD3EN.state  = (info.dmacon & 0b0000000000001000 != 0) ? .on : .off
        dmaAUD2EN.state  = (info.dmacon & 0b0000000000000100 != 0) ? .on : .off
        dmaAUD1EN.state  = (info.dmacon & 0b0000000000000010 != 0) ? .on : .off
        dmaAUD0EN.state  = (info.dmacon & 0b0000000000000001 != 0) ? .on : .off
        
        // Display DMA
        dmaDIWSTRT.integerValue = Int(info.diwstrt)
        dmaDIWSTOP.integerValue = Int(info.diwstop)
        dmaDDFSTRT.integerValue = Int(info.ddfstrt)
        dmaDDFSTOP.integerValue = Int(info.ddfstop)

        dmaBPL1PT.integerValue = Int(info.bplpt.0)
        dmaBPL2PT.integerValue = Int(info.bplpt.1)
        dmaBPL3PT.integerValue = Int(info.bplpt.2)
        dmaBPL4PT.integerValue = Int(info.bplpt.3)
        dmaBPL5PT.integerValue = Int(info.bplpt.4)
        dmaBPL6PT.integerValue = Int(info.bplpt.5)

        dmaBPL1MOD.integerValue = Int(info.bpl1mod)
        dmaBPL2MOD.integerValue = Int(info.bpl2mod)
    }
    
    @IBAction func dmaDMACONTextAction(_ sender: NSTextField!) {
        
        let value = sender.integerValue & 0xFFFF;
        
        track("New value: \(value)")
        amigaProxy?.mem.pokeCustom16(DMACON, value: value)
        refresh(everything: false)
    }

    @IBAction func dmaDMACONButtonAction(_ sender: NSButton!) {
        
        let value = (1 << sender.tag) + (sender.state == .on ? 0x8000 : 0);
        amigaProxy?.mem.pokeCustom16(DMACON, value: Int(value))
        refresh(everything: false)
    }

    @IBAction func dmaVPosAction(_ sender: NSButton!) {
        
        let value = sender.integerValue;
        track("New value: \(value)")
        track("Setter not implemented yet")
        refresh(everything: false)
    }

    @IBAction func dmaHPosAction(_ sender: NSButton!) {
        
        let value = sender.integerValue;
        track("New value: \(value)")
        track("Setter not implemented yet")
        refresh(everything: false)
    }
    
}
