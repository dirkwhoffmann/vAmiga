// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension MyController {
    
    func restoreSnapshot(item: Int) throws {
        
        if let snapshot = mydocument.snapshots.element(at: item) {
            try emu.loadSnapshot(snapshot)
        }
    }
    
    func restoreLatestSnapshot() throws {
        
        let count = mydocument!.snapshots.count
        if count > 0 { try restoreSnapshot(item: count - 1) }
    }
}
