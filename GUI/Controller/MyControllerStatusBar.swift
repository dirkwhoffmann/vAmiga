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
    
    public func refreshStatusBar() {
        
        let items: [NSView : Bool] = [
            
            cmdLock: mapCommandKeys,
            
            clockSpeed: true,
            clockSpeedBar: true,
            warpIcon: true,
        ]
        
        for (item, visible) in items {
            track("\(item) \(visible)")
            item.isHidden = !visible || !statusBar
        }
        
    }
    
    public func showStatusBar(_ value: Bool) {
        
        if statusBar != value {
            
            if value {
                
                metalScreen.shrink()
                window?.setContentBorderThickness(24, for: .minY)
                adjustWindowSize()
                
            } else {
                
                metalScreen.expand()
                window?.setContentBorderThickness(0, for: .minY)
                adjustWindowSize()
            }
            
            statusBar = value
            refreshStatusBar()
        }
    }
}
