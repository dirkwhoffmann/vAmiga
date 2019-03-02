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
        
        track("Refreshing Denise inspector tab")
        
        /*
        if everything {
            
        }
        */
        
        deniseBPLCON0.integerValue = Int(info.bplcon0)
        deniseHIRES.state = (info.bplcon0 & 0b1000000000000000 != 0) ? .on : .off
        deniseHOMOD.state = (info.bplcon0 & 0b0000100000000000 != 0) ? .on : .off
        deniseDBPLF.state = (info.bplcon0 & 0b0000010000000000 != 0) ? .on : .off
        deniseLACE.state  = (info.bplcon0 & 0b0000000000000100 != 0) ? .on : .off
        deniseBPLCON1.integerValue = Int(info.bplcon1)
        deniseBPLCON2.integerValue = Int(info.bplcon2)
        
        let planes = (info.bplcon0 & 0b0111000000000000) >> 12;
        deniseBpl1Enable.state = planes >= 1 ? .on : .off;
        deniseBpl2Enable.state = planes >= 2 ? .on : .off;
        deniseBpl3Enable.state = planes >= 3 ? .on : .off;
        deniseBpl4Enable.state = planes >= 4 ? .on : .off;
        deniseBpl5Enable.state = planes >= 5 ? .on : .off;
        deniseBpl6Enable.state = planes >= 6 ? .on : .off;
    }
    
    @IBAction func dmaBitplaneButtonAction(_ sender: NSButton!) {
        
        let activePlanes = (sender.state == .on) ? sender.tag : sender.tag - 1

        assert(activePlanes >= 0 && activePlanes <= 6)
        amigaProxy?.denise.setActivePlanes(activePlanes)
        refresh(everything: false)
    }
}
