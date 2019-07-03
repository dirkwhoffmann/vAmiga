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

        if everything {
            
        }

        if let info = amigaProxy?.agnus.getEventInfo() {
            evMasterClock.integerValue = Int(info.masterClock)
            evInCpuCycles.integerValue = Int(info.masterClock / 4)
            evInDmaCycles.integerValue = Int(info.masterClock / 8)
            evInCiaCycles.integerValue = Int(info.masterClock / 40)
            evCpuProgress.integerValue = Int(info.masterClock)
            evCpuProgress2.integerValue = Int(info.masterClock / 4)
            evDmaProgress.integerValue = Int(info.dmaClock)
            evDmaProgress2.integerValue = Int(info.dmaClock / 8)
            evCiaAProgress.integerValue = Int(info.ciaAClock)
            evCiaAProgress2.integerValue = Int(info.ciaAClock / 40)
            evCiaBProgress.integerValue = Int(info.ciaBClock)
            evCiaBProgress2.integerValue = Int(info.ciaBClock / 40)
            evFrame.integerValue = info.frame
            evPos.stringValue = String(format: "%3d, %3d", info.vpos, info.hpos)
        }
        /*
        if let info = amigaProxy?.agnus.getEventInfo() {
            evMasterClock.stringValue = String(format: "%lld", info.masterClock)
            evInCpuCycles.stringValue = String(format: "%lld", info.masterClock / 4)
            evInDmaCycles.stringValue = String(format: "%lld", info.masterClock / 8)
            evInCiaCycles.stringValue = String(format: "%lld", info.masterClock / 40)
            evCpuProgress.stringValue = String(format: "%lld", info.masterClock)
            evCpuProgress2.stringValue = String(format: "%lld", info.masterClock / 4)
            evDmaProgress.stringValue = String(format: "%lld", info.dmaClock)
            evDmaProgress2.stringValue = String(format: "%lld", info.dmaClock / 8)
            evCiaAProgress.stringValue = String(format: "%lld", info.ciaAClock)
            evCiaAProgress2.stringValue = String(format: "%lld", info.ciaAClock / 40)
            evCiaBProgress.stringValue = String(format: "%lld", info.ciaBClock)
            evCiaBProgress2.stringValue = String(format: "%lld", info.ciaBClock / 40)
            evFrame.stringValue = String(format: "%lld", info.frame)
            evPos.stringValue = String(format: "%3d, %3d", info.vpos, info.hpos)
        }
        */
        evTableView.reloadData()
    }
        
}
