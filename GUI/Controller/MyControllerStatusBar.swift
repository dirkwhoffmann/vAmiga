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
        case .auto where amiga.warpMode:
            return NSImage(named: "hourglass3Template")
        case .auto:
            return NSImage(named: "hourglass1Template")
        case .off:
            return NSImage(named: "warpOffTemplate")
        case .on:
            return NSImage(named: "warpOnTemplate")
        }
    }
    
    func refreshStatusBar() {
        
        let config = amiga.diskController.getConfig()
        let connected0 = config.connected.0
        let connected1 = config.connected.1
        let connected2 = config.connected.2
        let connected3 = config.connected.3

        let motor0 = amiga.df0.motor
        let motor1 = amiga.df1.motor
        let motor2 = amiga.df2.motor
        let motor3 = amiga.df3.motor
        let hasDisk0 = amiga.df0.hasDisk
        let hasDisk1 = amiga.df1.hasDisk
        let hasDisk2 = amiga.df2.hasDisk
        let hasDisk3 = amiga.df3.hasDisk

        let running = amiga.running
        let debug = amiga.debugMode
        let halted = amiga.cpu.halted
        let warp = amiga.warpMode

        // Cylinders
        refreshStatusBar(drive: 0, cylinder: amiga.df0.cylinder)
        refreshStatusBar(drive: 1, cylinder: amiga.df1.cylinder)
        refreshStatusBar(drive: 2, cylinder: amiga.df2.cylinder)
        refreshStatusBar(drive: 3, cylinder: amiga.df3.cylinder)
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

            warpIcon: running,
            activityType: running,
            activityInfo: running,
            activityBar: running
        ]
        
        for (item, visible) in items {
            item.isHidden = !visible || !statusBar
        }
    }
    
    func refreshStatusBar(drive: Int, led: Bool) {
        
        let image = NSImage(named: led ? "driveLedOn" : "driveLedOff")
        
        switch drive {
        case 0: df0LED.image = image
        case 1: df1LED.image = image
        case 2: df2LED.image = image
        case 3: df3LED.image = image
        default: fatalError()
        }
    }
    
    func refreshStatusBar(drive: Int, cylinder: Int) {
        
        switch drive {
        case 0: df0Cylinder.integerValue = cylinder
        case 1: df1Cylinder.integerValue = cylinder
        case 2: df2Cylinder.integerValue = cylinder
        case 3: df3Cylinder.integerValue = cylinder
        default: fatalError()
        }
    }
         
    func refreshStatusBar(writing: Bool?) {
        
        let sel = amiga.diskController.selectedDrive
        let w = writing ?? (amiga.diskController.state == .WRITE)
        
        df0Cylinder.textColor = w && (sel == 0) ? .warningColor : .secondaryLabelColor
        df1Cylinder.textColor = w && (sel == 1) ? .warningColor : .secondaryLabelColor
        df2Cylinder.textColor = w && (sel == 2) ? .warningColor : .secondaryLabelColor
        df3Cylinder.textColor = w && (sel == 3) ? .warningColor : .secondaryLabelColor
    }
        
    func refreshStatusBar(drive: Int, motor: Bool) {
        
        let spin = amiga.running && motor
        
        switch drive {
        case 0: spin ? df0DMA.startAnimation(self) : df0DMA.stopAnimation(self)
        case 1: spin ? df1DMA.startAnimation(self) : df1DMA.stopAnimation(self)
        case 2: spin ? df2DMA.startAnimation(self) : df2DMA.stopAnimation(self)
        case 3: spin ? df3DMA.startAnimation(self) : df3DMA.stopAnimation(self)
        default: fatalError()
        }
    }
         
    func showStatusBar(_ value: Bool) {
        
        if statusBar != value {
            
            if value {
                
                metal.shrink()
                window?.setContentBorderThickness(26, for: .minY)
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
    
    func updateSpeedometer() {
        
        speedometer.updateWith(cycle: amiga.cpu.clock, frame: renderer.frames)
        
        switch activityType.selectedTag() {

        case 0:
            let mhz = speedometer.mhz
            activityBar.doubleValue = 10 * mhz
            activityInfo.stringValue = String(format: "%.2f MHz", mhz)
            
        case 1:
            let cpu = amiga.cpuLoad
            activityBar.integerValue = cpu
            activityInfo.stringValue = String(format: "%d%% CPU", cpu)
            
        case 2:
            let fps = speedometer.fps
            activityBar.doubleValue = fps
            activityInfo.stringValue = String(format: "%d FPS", Int(fps))

        default:
            activityBar.integerValue = 0
            activityInfo.stringValue = "???"
        }
    }
    
    //
    // Action methods
    //
    
    @IBAction func warpAction(_ sender: Any!) {
        
        track()
        
        switch pref.warpMode {
        case .auto: pref.warpMode = .off
        case .off: pref.warpMode = .on
        case .on: pref.warpMode = .auto
        }
        
        refreshStatusBar()
    }

    @IBAction func activityTypeAction(_ sender: NSPopUpButton!) {

        track()
        
        var min, max, warn, crit: Double
        
        switch sender.selectedTag() {
        
        case 0: min = 0; max = 140; warn = 77; crit = 105
        case 1: min = 0; max = 100; warn = 50; crit = 75
        case 2: min = 0; max = 120; warn = 75; crit = 100

        default:
            fatalError()
        }
        
        activityBar.minValue = min
        activityBar.maxValue = max
        activityBar.warningValue = warn
        activityBar.criticalValue = crit

        refreshStatusBar()
    }
}
