// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
extension MyController {
    
    var snapshotCount: Int { return mydocument!.snapshots.count }
    
    func restoreSnapshot(item: Int) throws {
        
        if let snapshot = mydocument.snapshots.element(at: item) {
            try emu?.amiga.loadSnapshot(snapshot)
        }
    }
    
    func restoreLatestSnapshot() throws {
        
        if snapshotCount > 0 { try restoreSnapshot(item: snapshotCount - 1) }
    }
}
