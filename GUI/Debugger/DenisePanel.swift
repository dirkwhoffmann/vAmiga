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
        
    func refreshDenise(everything: Bool) {
        
        guard let denise = amigaProxy?.denise else { return }
        
        let info = denise.getInfo()
        let sprInfo = denise.getSpriteInfo(selectedSprite)
        
        if everything {
            
        }
        
        //
        // Bitplane section
        //
        
        deniseBPLCON0.integerValue = Int(info.bplcon0)
        deniseHIRES.state = (info.bplcon0 & 0b1000000000000000 != 0) ? .on : .off
        deniseHOMOD.state = (info.bplcon0 & 0b0000100000000000 != 0) ? .on : .off
        deniseDBPLF.state = (info.bplcon0 & 0b0000010000000000 != 0) ? .on : .off
        deniseLACE.state  = (info.bplcon0 & 0b0000000000000100 != 0) ? .on : .off
        deniseBPLCON1.integerValue = Int(info.bplcon1)
        deniseBPLCON2.integerValue = Int(info.bplcon2)
        
        
        //
        // Sprite section
        //
        
        sprHStart.integerValue = Int(sprInfo.hstrt);
        sprVStart.integerValue = Int(sprInfo.vstrt);
        sprVStop.integerValue = Int(sprInfo.vstop);
        sprAttach.state = sprInfo.attach ? .on : .off;
        sprTableView.refresh(everything: everything)
        
    }
    
    @IBAction func deniseBPLCON0ButtonAction(_ sender: NSButton!) {
        
        amigaProxy?.denise.setBPLCON0Bit(sender.tag, value: sender.state == .on)
        refresh(everything: false)
    }
    
    @IBAction func selectSpriteAction(_ sender: Any!) {
        
        refreshDenise(everything: true)
    }
}
