// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension MyController {
    
    //
    // Snapshots
    //
    
    func load(snapshot: SnapshotProxy?) {
        
        if snapshot == nil { return }
        
        amiga.suspend()
        amiga.load(fromSnapshot: snapshot)
        amiga.resume()
    }
        
    func restoreSnapshot(item: Int) -> Bool {
        
        if let snapshot = mydocument.snapshots.element(at: item) {
            load(snapshot: snapshot)
            return true
        }
        
        return false
    }
        
    func restoreLatestSnapshot() -> Bool {
        
        let count = mydocument!.snapshots.count
        return count > 0 ? restoreSnapshot(item: count - 1) : false
    }
}
