// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
extension Inspector {

    private func cacheEvents() {

        // eventInfo = emu.agnus.eventInfo
        agnusInfo = emu.paused ? emu.agnus.info : emu.agnus.cachedInfo
        // eventInfo = agnusInfo.eventInfo
    }

    func refreshEvents(count: Int = 0, full: Bool = false) {

        cacheEvents()

        evCpuProgress.integerValue = Int(agnusInfo.eventInfo.cpuClock)
        evCpuProgress2.integerValue = Int(agnusInfo.eventInfo.cpuCycles)
        evDmaProgress.integerValue = Int(agnusInfo.eventInfo.dmaClock)
        evDmaProgress2.integerValue = Int(agnusInfo.eventInfo.dmaClock / 8)
        evCiaAProgress.integerValue = Int(agnusInfo.eventInfo.ciaAClock)
        evCiaAProgress2.integerValue = Int(agnusInfo.eventInfo.ciaAClock / 40)
        evCiaBProgress.integerValue = Int(agnusInfo.eventInfo.ciaBClock)
        evCiaBProgress2.integerValue = Int(agnusInfo.eventInfo.ciaBClock / 40)

        evTableView.refresh(count: count, full: full)
    }
}
