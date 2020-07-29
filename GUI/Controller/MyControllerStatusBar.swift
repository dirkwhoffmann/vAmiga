// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension MyController {
    
    var hourglassIcon: NSImage? {
        
        switch pref.warpMode {
        case .auto where amiga.warp:
            return NSImage.init(named: "hourglass3Template")
        case .auto:
            return NSImage.init(named: "hourglass1Template")
        case .off:
            return NSImage.init(named: "warpOffTemplate")
        case .on:
            return NSImage.init(named: "warpOnTemplate")
        }
    }
    
    public func refreshStatusBar() {
        
        let config = amiga.diskController.getConfig()
        let connected0 = config.connected.0
        let connected1 = config.connected.1
        let connected2 = config.connected.2
        let connected3 = config.connected.3

        let motor0 = amiga.df0.motor()
        let motor1 = amiga.df1.motor()
        let motor2 = amiga.df2.motor()
        let motor3 = amiga.df3.motor()
        let hasDisk0 = amiga.df0.hasDisk()
        let hasDisk1 = amiga.df1.hasDisk()
        let hasDisk2 = amiga.df2.hasDisk()
        let hasDisk3 = amiga.df3.hasDisk()

        let running = amiga.isRunning
        let debug = amiga.debugMode()
        let halted = amiga.cpu.isHalted
        let warp = amiga.warp

        // Cylinders
        refreshStatusBar(drive: 0, cyclinder: amiga.df0.cylinder())
        refreshStatusBar(drive: 1, cyclinder: amiga.df1.cylinder())
        refreshStatusBar(drive: 2, cyclinder: amiga.df2.cylinder())
        refreshStatusBar(drive: 3, cyclinder: amiga.df3.cylinder())
        refreshStatusBar(writing: nil)
        
        // Animation
        refreshStatusBar(drive: 0, motor: motor0)
        refreshStatusBar(drive: 1, motor: motor1)
        refreshStatusBar(drive: 2, motor: motor2)
        refreshStatusBar(drive: 3, motor: motor3)

        // Drive icons
        df0Disk.image = amiga.df0.icon
        df1Disk.image = amiga.df1.icon
        df2Disk.image = amiga.df2.icon
        df3Disk.image = amiga.df3.icon
        
        // Warp mode icon
        warpIcon.image = hourglassIcon
        
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

            haltIcon: halted,
            cmdLock: myAppDelegate.mapCommandKeys,
            debugIcon: debug,
            muteIcon: warp || muted,

            clockSpeed: running,
            clockSpeedBar: running,
            warpIcon: running
        ]
        
        for (item, visible) in items {
            item.isHidden = !visible || !statusBar
        }
    }
    
    public func refreshStatusBar(drive: Int, led: Bool) {
        
        let image = NSImage.init(named: led ? "driveLedOn" : "driveLedOff")
        
        switch drive {
        case 0: df0LED.image = image
        case 1: df1LED.image = image
        case 2: df2LED.image = image
        case 3: df3LED.image = image
        default: fatalError()
        }
    }
    
    public func refreshStatusBar(drive: Int, cyclinder: Int) {
        
        switch drive {
        case 0: df0Cylinder.integerValue = cyclinder
        case 1: df1Cylinder.integerValue = cyclinder
        case 2: df2Cylinder.integerValue = cyclinder
        case 3: df3Cylinder.integerValue = cyclinder
        default: fatalError()
        }
    }
         
    public func refreshStatusBar(writing: Bool?) {
        
        let sel = amiga.diskController.selectedDrive()
        let w = writing ?? (amiga.diskController.state() == DRIVE_DMA_WRITE)
        
        df0Cylinder.textColor = w && (sel == 0) ? .red : .secondaryLabelColor
        df1Cylinder.textColor = w && (sel == 1) ? .red : .secondaryLabelColor
        df2Cylinder.textColor = w && (sel == 2) ? .red : .secondaryLabelColor
        df3Cylinder.textColor = w && (sel == 3) ? .red : .secondaryLabelColor
    }
        
    public func refreshStatusBar(drive: Int, motor: Bool) {
        
        let spin = amiga.isRunning && motor
        
        switch drive {
        case 0: spin ? df0DMA.startAnimation(self) : df0DMA.stopAnimation(self)
        case 1: spin ? df1DMA.startAnimation(self) : df1DMA.stopAnimation(self)
        case 2: spin ? df2DMA.startAnimation(self) : df2DMA.stopAnimation(self)
        case 3: spin ? df3DMA.startAnimation(self) : df3DMA.stopAnimation(self)
        default: fatalError()
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
