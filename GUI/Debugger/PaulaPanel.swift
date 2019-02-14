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
    
    func refreshPaula(everything: Bool) {
        
        guard let paula = amigaProxy?.paula else { return }
        let info = paula.getInfo()

        track("Refreshing Paula inspector tab")
        
        if everything {
         
        }
        
        // Interrupt registers
        paulaIntena.intValue = Int32(info.intena)
        paulaEna14.state = (info.intena & 0b0100000000000000 != 0) ? .on : .off
        paulaEna13.state = (info.intena & 0b0010000000000000 != 0) ? .on : .off
        paulaEna12.state = (info.intena & 0b0001000000000000 != 0) ? .on : .off
        paulaEna11.state = (info.intena & 0b0000100000000000 != 0) ? .on : .off
        paulaEna10.state = (info.intena & 0b0000010000000000 != 0) ? .on : .off
        paulaEna9.state  = (info.intena & 0b0000001000000000 != 0) ? .on : .off
        paulaEna8.state  = (info.intena & 0b0000000100000000 != 0) ? .on : .off
        paulaEna7.state  = (info.intena & 0b0000000010000000 != 0) ? .on : .off
        paulaEna6.state  = (info.intena & 0b0000000001000000 != 0) ? .on : .off
        paulaEna5.state  = (info.intena & 0b0000000000100000 != 0) ? .on : .off
        paulaEna4.state  = (info.intena & 0b0000000000010000 != 0) ? .on : .off
        paulaEna3.state  = (info.intena & 0b0000000000001000 != 0) ? .on : .off
        paulaEna2.state  = (info.intena & 0b0000000000000100 != 0) ? .on : .off
        paulaEna1.state  = (info.intena & 0b0000000000000010 != 0) ? .on : .off
        paulaEna0.state  = (info.intena & 0b0000000000000001 != 0) ? .on : .off
        
        paulaIntreq.intValue = Int32(info.intreq)
        paulaReq14.state = (info.intreq & 0b0100000000000000 != 0) ? .on : .off
        paulaReq13.state = (info.intreq & 0b0010000000000000 != 0) ? .on : .off
        paulaReq12.state = (info.intreq & 0b0001000000000000 != 0) ? .on : .off
        paulaReq11.state = (info.intreq & 0b0000100000000000 != 0) ? .on : .off
        paulaReq10.state = (info.intreq & 0b0000010000000000 != 0) ? .on : .off
        paulaReq9.state  = (info.intreq & 0b0000001000000000 != 0) ? .on : .off
        paulaReq8.state  = (info.intreq & 0b0000000100000000 != 0) ? .on : .off
        paulaReq7.state  = (info.intreq & 0b0000000010000000 != 0) ? .on : .off
        paulaReq6.state  = (info.intreq & 0b0000000001000000 != 0) ? .on : .off
        paulaReq5.state  = (info.intreq & 0b0000000000100000 != 0) ? .on : .off
        paulaReq4.state  = (info.intreq & 0b0000000000010000 != 0) ? .on : .off
        paulaReq3.state  = (info.intreq & 0b0000000000001000 != 0) ? .on : .off
        paulaReq2.state  = (info.intreq & 0b0000000000000100 != 0) ? .on : .off
        paulaReq1.state  = (info.intreq & 0b0000000000000010 != 0) ? .on : .off
        paulaReq0.state  = (info.intreq & 0b0000000000000001 != 0) ? .on : .off
        
        // Audio buffer
        let fillLevel = Int32(paula.fillLevel() * 100)
        audioBufferLevel.intValue = fillLevel
        audioBufferLevelText.stringValue = "\(fillLevel) %"
        audioBufferUnderflows.intValue = Int32(paula.bufferUnderflows())
        audioBufferOverflows.intValue = Int32(paula.bufferOverflows())
        audioWaveformView.update()
    }
    
}
