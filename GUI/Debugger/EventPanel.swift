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
        
        if let info = amigaProxy?.getInfo() {
            evMasterClock.stringValue = String(format: "%lld",
                                               info.masterClock)
            evInCpuCycles.stringValue = String(format: "%lld",
                                               info.masterClock / 4)
            evInDmaCycles.stringValue = String(format: "%lld",
                                               info.masterClock / 8)
            evInCiaCycles.stringValue = String(format: "%lld",
                                               info.masterClock / 40)
            evCpuProgress.stringValue = String(format: "%lld (%lld)",
                                               info.masterClock,
                                               info.masterClock / 4)
            evDmaProgress.stringValue = String(format: "%lld (%lld)",
                                               info.dmaClock,
                                               info.dmaClock / 8)
            evCiaAProgress.stringValue = String(format: "%lld (%lld)",
                                                info.ciaAClock,
                                                info.ciaAClock / 40)
            evCiaBProgress.stringValue = String(format: "%lld (%lld)",
                                                info.ciaBClock,
                                                info.ciaBClock / 40)
            evFrame.integerValue = info.frame
            evVPos.integerValue = info.vpos
            evHPos.integerValue = info.hpos
        }
        
        evPrimTableView.reloadData()
        evSecTableView.reloadData()
    }
        
}
