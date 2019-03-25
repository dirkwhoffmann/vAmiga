// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {
    
    func refreshEvents(everything: Bool) {
        
        // guard let dma = amigaProxy?.dma else { return }
        // let info = dma.getInfo()
        
        track("Refreshing Event inspector tab")
        
        if everything {
            
        }
        
        evPrimTableView.reloadData()
        evSecTableView.reloadData()
    }
        
}
