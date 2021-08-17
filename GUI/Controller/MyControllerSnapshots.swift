// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension MyController {
    
    func stopSnapshotTimer() {
        
        snapshotTimer?.invalidate()
    }
    
    func validateSnapshotTimer() {
        
        snapshotTimer?.invalidate()
        if pref.autoSnapshots && pref.snapshotInterval > 0 {
            
            snapshotTimer =
                Timer.scheduledTimer(timeInterval: TimeInterval(pref.snapshotInterval),
                                     target: self,
                                     selector: #selector(snapshotTimerFunc),
                                     userInfo: nil,
                                     repeats: true)
        }
    }

    @objc func snapshotTimerFunc() {

        if pref.autoSnapshots { amiga.requestAutoSnapshot() }
    }
        
    func restoreSnapshot(item: Int) throws {
        
        if let snapshot = mydocument.snapshots.element(at: item) {
            try amiga.loadSnapshot(snapshot)
        }
    }
    
    func restoreLatestSnapshot() throws {
        
        let count = mydocument!.snapshots.count
        if count > 0 { try restoreSnapshot(item: count - 1) }
    }
}
