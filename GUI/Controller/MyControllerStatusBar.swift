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
        drvIcon0.image = amiga.df0.icon
        drvIcon1.image = amiga.df1.icon
        drvIcon2.image = amiga.df2.icon
        drvIcon3.image = amiga.df3.icon
        
        // Remote server icon
        debugIcon.image = amiga.remoteManager.icon
        
        // Warp mode icon
        warpIcon.image = hourglassIcon
        
        // Visibility
        let items: [NSView: Bool] = [
            
            powerLED: true,
            
            drvLED0: connected0,
            drvLED1: connected1,
            drvLED2: connected2,
            drvLED3: connected3,
            drvIcon0: connected0 && hasDisk0,
            drvIcon1: connected1 && hasDisk1,
            drvIcon2: connected2 && hasDisk2,
            drvIcon3: connected3 && hasDisk3,
            drvCyl0: connected0,
            drvCyl1: connected1,
            drvCyl2: connected2,
            drvCyl3: connected3,
            drvBusy0: motor0,
            drvBusy1: motor1,
            drvBusy2: motor2,
            drvBusy3: motor3,

            haltIcon: halted,
            cmdLock: myAppDelegate.mapCommandKeys,
            debugIcon: true,
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
    
    fileprivate func refreshStatusBar(drive: Int, led: Bool) {
        
        let image = NSImage(named: led ? "driveLedOn" : "driveLedOff")
        
        switch drive {
        case 0: drvLED0.image = image
        case 1: drvLED1.image = image
        case 2: drvLED2.image = image
        case 3: drvLED3.image = image
        default: fatalError()
        }
    }
    
    func refreshStatusBar(drive: Int, cylinder: Int) {
        
        switch drive {
        case 0: drvCyl0.integerValue = cylinder
        case 1: drvCyl1.integerValue = cylinder
        case 2: drvCyl2.integerValue = cylinder
        case 3: drvCyl3.integerValue = cylinder
        default: fatalError()
        }
    }
         
    func refreshStatusBar(writing: Bool?) {
        
        let sel = amiga.diskController.selectedDrive
        let w = writing ?? (amiga.diskController.state == .WRITE)
        
        drvCyl0.textColor = w && (sel == 0) ? .warningColor : .secondaryLabelColor
        drvCyl1.textColor = w && (sel == 1) ? .warningColor : .secondaryLabelColor
        drvCyl2.textColor = w && (sel == 2) ? .warningColor : .secondaryLabelColor
        drvCyl3.textColor = w && (sel == 3) ? .warningColor : .secondaryLabelColor
    }
        
    fileprivate func refreshStatusBar(drive: Int, motor: Bool) {
        
        let spin = amiga.running && motor
        
        switch drive {
        case 0: spin ? drvBusy0.startAnimation(self) : drvBusy0.stopAnimation(self)
        case 1: spin ? drvBusy1.startAnimation(self) : drvBusy1.stopAnimation(self)
        case 2: spin ? drvBusy2.startAnimation(self) : drvBusy2.stopAnimation(self)
        case 3: spin ? drvBusy3.startAnimation(self) : drvBusy3.stopAnimation(self)
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
