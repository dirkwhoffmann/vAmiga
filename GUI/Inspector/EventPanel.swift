// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {

    private func cacheEvents() {

        eventInfo = amiga.agnus.eventInfo
    }

    func refreshEvents(count: Int = 0, full: Bool = false) {

        cacheEvents()

        evCpuProgress.integerValue = Int(eventInfo.cpuClock)
        evCpuProgress2.integerValue = Int(eventInfo.cpuCycles)
        evDmaProgress.integerValue = Int(eventInfo.dmaClock)
        evDmaProgress2.integerValue = Int(eventInfo.dmaClock / 8)
        evCiaAProgress.integerValue = Int(eventInfo.ciaAClock)
        evCiaAProgress2.integerValue = Int(eventInfo.ciaAClock / 40)
        evCiaBProgress.integerValue = Int(eventInfo.ciaBClock)
        evCiaBProgress2.integerValue = Int(eventInfo.ciaBClock / 40)

        evTableView.refresh(count: count, full: full)
    }
}
