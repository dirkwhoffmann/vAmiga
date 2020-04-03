// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

extension MyController {
 
    //
    // Snapshots
    //
    
    func takeSnapshot(auto: Bool) {
        
        track("auto = \(auto)")
        
        if let s = SnapshotProxy.make(withAmiga: amiga) {
        
            if auto {
                mydocument?.autoSnapshots.append(s)
            } else {
                mydocument?.userSnapshots.append(s)
            }
        }
    }
    
    func takeAutoSnapshot() { takeSnapshot(auto: true) }
    func takeUserSnapshot() { takeSnapshot(auto: false) }

    func restoreSnapshot(item: Int, auto: Bool) -> Bool {
        
        track("auto = \(auto)")
        
        if auto && item < mydocument!.autoSnapshots.count {
            amiga.load(fromSnapshot: mydocument!.autoSnapshots[item])
            return true
        }
        if !auto && item < mydocument!.userSnapshots.count {
            amiga.load(fromSnapshot: mydocument!.userSnapshots[item])
            return true
        }
        return false
    }
    
    func restoreAutoSnapshot(item: Int) -> Bool {
        return restoreSnapshot(item: item, auto: true)
    }
    func restoreUserSnapshot(item: Int) -> Bool {
        return restoreSnapshot(item: item, auto: false)
    }
    func restoreLatestAutoSnapshot() -> Bool {
        if mydocument!.autoSnapshots.count > 0 {
            return restoreAutoSnapshot(item: mydocument!.autoSnapshots.count - 1)
        }
        return false
    }
    func restoreLatestUserSnapshot() -> Bool {
        if mydocument!.userSnapshots.count > 0 {
            return restoreUserSnapshot(item: mydocument!.userSnapshots.count - 1)
        }
        return false
    }

    //
    // Screenshots
    //
    
    func takeScreenshot(auto: Bool) {
        
        let upscaled = screenshotSource > 0
        let checksum = amiga.df0.fnv()
        
        // Take screenshot
        guard let screen = renderer.screenshot(afterUpscaling: upscaled) else {
            track("Failed to create screenshot")
            return
        }
        let screenshot = Screenshot.init(screen: screen, upscaled: upscaled)
        
        // Compute URL
        guard let url = Screenshot.newUrl(checksum: checksum, auto: auto) else {
            track("Failed to create URL")
            return
        }
        
        // Save screenshot
        // track("Saving screenshot to \(url.path)")
        try? screenshot.save(url: url, format: screenshotTarget)
        
        // Thin out screenshot directory to reduce the amout of stored data
        Screenshot.thinOutAuto(checksum: checksum, counter: screenshotCounter)
        screenshotCounter += 1
    }
    
    func takeAutoScreenshot() { takeScreenshot(auto: true) }
    func takeUserScreenshot() { takeScreenshot(auto: false) }
}
