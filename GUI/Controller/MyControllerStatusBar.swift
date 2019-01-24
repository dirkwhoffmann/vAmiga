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
        
        guard let amiga = amigaProxy else { return }
        
        let items: [NSView : Bool] = [
            
            df0LED: true,
            df0Disk: amiga.df0.hasDisk(),
            df0DMA: amiga.diskController.doesDMA(0),
            df1LED: true,
            df1Disk: amiga.df1.hasDisk(),
            df1DMA: amiga.diskController.doesDMA(1),

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
                
                metal.shrink()
                window?.setContentBorderThickness(24, for: .minY)
                adjustWindowSize()
                
            } else {
                
                metal.expand()
                window?.setContentBorderThickness(0, for: .minY)
                adjustWindowSize()
            }
            
            statusBar = value
            refreshStatusBar()
        }
    }
}
