// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {
    
    func refreshCopper(everything: Bool) {
        
        // guard let dma = amigaProxy?.dma else { return }
        // let info = dma.getInfo()
        
        track("Refreshing Copper inspector tab")
        
        if everything {
            
        }
        
        copList1.refresh(everything: everything)
        copList2.refresh(everything: everything)
    }
 
    @IBAction func copCOPLCAction(_ sender: NSTextField!) {
        
        track()
    }
    
}
