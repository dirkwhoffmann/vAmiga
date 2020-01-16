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

        lockParent()
        if let amiga = parent?.amiga { refreshEvents(amiga, everything) }
        unlockParent()
    }

    func refreshEvents(_ amiga: AmigaProxy, _ everything: Bool) {

        let info = amiga.agnus.getEventInfo()

        evCpuProgress.integerValue = Int(info.cpuClock)
        evCpuProgress2.integerValue = Int(info.cpuCycles)
        evDmaProgress.integerValue = Int(info.dmaClock)
        evDmaProgress2.integerValue = Int(info.dmaClock / 8)
        evCiaAProgress.integerValue = Int(info.ciaAClock)
        evCiaAProgress2.integerValue = Int(info.ciaAClock / 40)
        evCiaBProgress.integerValue = Int(info.ciaBClock)
        evCiaBProgress2.integerValue = Int(info.ciaBClock / 40)

        evTableView.reloadData()
    }
}
