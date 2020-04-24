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
        
        let config = amiga.diskController.getConfig()
        let connected0 = config.connected.0
        let connected1 = config.connected.1
        let connected2 = config.connected.2
        let connected3 = config.connected.3

        let sel = amiga.diskController.selectedDrive()
        let writing = amiga.diskController.state() == DRIVE_DMA_WRITE
        let motor0 = amiga.df0.motor()
        let motor1 = amiga.df1.motor()
        let motor2 = amiga.df2.motor()
        let motor3 = amiga.df3.motor()
        let hasDisk0 = amiga.df0.hasDisk()
        let hasDisk1 = amiga.df1.hasDisk()
        let hasDisk2 = amiga.df2.hasDisk()
        let hasDisk3 = amiga.df3.hasDisk()
        let running = amiga.isRunning()

        // Icons
        df0Disk.image = amiga.df0.icon
        df1Disk.image = amiga.df1.icon
        df2Disk.image = amiga.df2.icon
        df3Disk.image = amiga.df3.icon
        warpIcon.image = hourglassIcon

        // Cylinders
        df0Cylinder.integerValue = amiga.df0.cylinder()
        df1Cylinder.integerValue = amiga.df1.cylinder()
        df2Cylinder.integerValue = amiga.df2.cylinder()
        df3Cylinder.integerValue = amiga.df3.cylinder()

        df0Cylinder.textColor = writing && (sel == 0) ? .red : .secondaryLabelColor
        df1Cylinder.textColor = writing && (sel == 1) ? .red : .secondaryLabelColor
        df2Cylinder.textColor = writing && (sel == 2) ? .red : .secondaryLabelColor
        df3Cylinder.textColor = writing && (sel == 3) ? .red : .secondaryLabelColor

        // Animation
        motor0 && running ? df0DMA.startAnimation(self) : df0DMA.stopAnimation(self)
        motor1 && running ? df1DMA.startAnimation(self) : df1DMA.stopAnimation(self)
        motor2 && running ? df2DMA.startAnimation(self) : df2DMA.stopAnimation(self)
        motor3 && running ? df3DMA.startAnimation(self) : df3DMA.stopAnimation(self)

        // Visibility
        let items: [NSView: Bool] = [
            
            powerLED: true,
            
            df0LED: connected0,
            df1LED: connected1,
            df2LED: connected2,
            df3LED: connected3,
            df0Disk: connected0 && hasDisk0,
            df1Disk: connected1 && hasDisk1,
            df2Disk: connected2 && hasDisk2,
            df3Disk: connected3 && hasDisk3,
            df0Cylinder: connected0,
            df1Cylinder: connected1,
            df2Cylinder: connected2,
            df3Cylinder: connected3,
            df0DMA: motor0,
            df1DMA: motor1,
            df2DMA: motor2,
            df3DMA: motor3,

            cmdLock: mapCommandKeys,
            
            clockSpeed: running,
            clockSpeedBar: running,
            warpIcon: running
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
