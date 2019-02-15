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
        
        // Interrupt registers
        dmaDMACON.intValue = Int32(info.dmacon)
        dmaDMACON10.state = (info.dmacon & 0b0000010000000000 != 0) ? .on : .off
        dmaDMACON9.state  = (info.dmacon & 0b0000001000000000 != 0) ? .on : .off
        dmaDMACON8.state  = (info.dmacon & 0b0000000100000000 != 0) ? .on : .off
        dmaDMACON7.state  = (info.dmacon & 0b0000000010000000 != 0) ? .on : .off
        dmaDMACON6.state  = (info.dmacon & 0b0000000001000000 != 0) ? .on : .off
        dmaDMACON5.state  = (info.dmacon & 0b0000000000100000 != 0) ? .on : .off
        dmaDMACON4.state  = (info.dmacon & 0b0000000000010000 != 0) ? .on : .off
        dmaDMACON3.state  = (info.dmacon & 0b0000000000001000 != 0) ? .on : .off
        dmaDMACON2.state  = (info.dmacon & 0b0000000000000100 != 0) ? .on : .off
        dmaDMACON1.state  = (info.dmacon & 0b0000000000000010 != 0) ? .on : .off
        dmaDMACON0.state  = (info.dmacon & 0b0000000000000001 != 0) ? .on : .off
    }
    
}
