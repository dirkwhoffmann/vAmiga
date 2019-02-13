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
        
        track("Refreshing Paula inspector tab")
        
        if everything {
         
        }
        
        // Volume
        audioVolume.intValue = 0xFF; // Int32(info.volume)
        
        // Audio buffer
        let fillLevel = Int32(paula.fillLevel() * 100)
        audioBufferLevel.intValue = fillLevel
        audioBufferLevelText.stringValue = "\(fillLevel) %"
        audioBufferUnderflows.intValue = Int32(paula.bufferUnderflows())
        audioBufferOverflows.intValue = Int32(paula.bufferOverflows())
        
        audioWaveformView.update()
    }
    
}
