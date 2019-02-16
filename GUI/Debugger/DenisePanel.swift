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
        deniseHIRES.state = (info.bplcon0 & 0b100000000000000 != 0) ? .on : .off
        deniseHOMOD.state = (info.bplcon0 & 0b000010000000000 != 0) ? .on : .off
        deniseDBPLF.state = (info.bplcon0 & 0b000001000000000 != 0) ? .on : .off
        deniseLACE.state  = (info.bplcon0 & 0b000000000000100 != 0) ? .on : .off
        deniseBPLCON1.integerValue = Int(info.bplcon1)
        deniseBPLCON2.integerValue = Int(info.bplcon2)
        

        

    }
    
}
