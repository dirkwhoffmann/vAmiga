// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension MyController {
    
    public func refreshStatusBar() {
        
        guard let amiga = amigaProxy else { return }

        let df0IsConnected = amiga.diskController.isConnected(0)
        let df1IsConnected = amiga.diskController.isConnected(1)
        let df2IsConnected = amiga.diskController.isConnected(2)
        let df3IsConnected = amiga.diskController.isConnected(3)
        let df0spinning = amiga.diskController.spinning(0)
        let df1spinning = amiga.diskController.spinning(1)
        let df2spinning = amiga.diskController.spinning(2)
        let df3spinning = amiga.diskController.spinning(3)
        let userWarp = warpMode != .auto

        // Icons
        df0Disk.image = amiga.df0.icon
        df1Disk.image = amiga.df1.icon
        df2Disk.image = amiga.df2.icon
        df3Disk.image = amiga.df3.icon
        warpIcon.image = hourglassIcon

        // Speed and frame rate
        /*
        let x = userWarp ? warpLockIcon.frame.origin.x : warpIcon.frame.origin.x
        let w = clockSpeed.frame.width
        clockSpeed.frame.origin.x = x - w
        */

        // Animation
        df0spinning ? df0DMA.startAnimation(self) : df0DMA.stopAnimation(self)
        df1spinning ? df1DMA.startAnimation(self) : df1DMA.stopAnimation(self)
        df2spinning ? df2DMA.startAnimation(self) : df2DMA.stopAnimation(self)
        df3spinning ? df3DMA.startAnimation(self) : df3DMA.stopAnimation(self)

        // Visibility
        let items: [NSView: Bool] = [
            
            powerLED: true,
            
            df0LED: df0IsConnected,
            df1LED: df1IsConnected,
            df2LED: df2IsConnected,
            df3LED: df3IsConnected,
            df0Disk: df0IsConnected && amiga.df0.hasDisk(),
            df1Disk: df1IsConnected && amiga.df1.hasDisk(),
            df2Disk: df2IsConnected && amiga.df2.hasDisk(),
            df3Disk: df3IsConnected && amiga.df3.hasDisk(),
            df0DMA: df0spinning,
            df1DMA: df1spinning,
            df2DMA: df2spinning,
            df3DMA: df3spinning,

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
