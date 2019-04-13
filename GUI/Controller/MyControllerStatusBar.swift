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
        
        let df0IsConnected = amiga.diskController.isConnected(0)
        let df1IsConnected = amiga.diskController.isConnected(1)
        let df0spinning = amiga.diskController.spinning(0)
        let df1spinning = amiga.diskController.spinning(1)

        // Icons
        df0Disk.image = amiga.df0.icon
        df1Disk.image = amiga.df1.icon
        warpIcon.image = hourglassIcon
        
        // Animation
        df0spinning ? df0DMA.startAnimation(self) : df0DMA.stopAnimation(self)
        df1spinning ? df1DMA.startAnimation(self) : df1DMA.stopAnimation(self)
        // TODO:
        // df2spinning ? df2DMA.startAnimation(self) : df2DMA.stopAnimation(self)
        // df3spinning ? df3DMA.startAnimation(self) : df3DMA.stopAnimation(self)

        // Visibility
        let items: [NSView : Bool] = [
            
            powerLED: true,
            
            df0LED:  df0IsConnected,
            df0Disk: df0IsConnected && amiga.df0.hasDisk(),
            df0DMA:  df0spinning,
            df1LED:  df1IsConnected,
            df1Disk: df1IsConnected && amiga.df1.hasDisk(),
            df1DMA:  df1spinning,

            cmdLock: mapCommandKeys,
            
            clockSpeed: true,
            clockSpeedBar: true,
            warpIcon: true,
        ]
        
        for (item, visible) in items {
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
