// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

extension MyController {
    
    func refreshSID() {
        
        // let info = amiga.paula.getInfo()
        
        // Volume
        volume.intValue = 0xFF; // Int32(info.volume)
        
        // Audio buffer
        let fillLevel = Int32(c64.sid.fillLevel() * 100)
        audioBufferLevel.intValue = fillLevel
        audioBufferLevelText.stringValue = "\(fillLevel) %"
        bufferUnderflows.intValue = 0 // Int32(amiga.paula.bufferUnderflows())
        bufferOverflows.intValue = 0 // Int32(amiga.paula.bufferOverflows())
        
        waveformView.update()
    }
    
  
    //
    // Volume
    //
    
    func _volumeAction(_ value: UInt8) {
        track()
    }
    
    @IBAction func volumeAction(_ sender: Any!) {
        
        let sender = sender as! NSTextField
        _volumeAction(UInt8(sender.intValue))
    }
}

