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
        
        // let config = amiga.diskController.getConfig()
        /*
        let connected0 = config.connected.0
        let connected1 = config.connected.1
        let connected2 = config.connected.2
        let connected3 = config.connected.3
        */
        
        let motor0 = amiga.df0.motor
        let motor1 = amiga.df1.motor
        let motor2 = amiga.df2.motor
        let motor3 = amiga.df3.motor
        /*
        let hasDisk0 = amiga.df0.hasDisk
        let hasDisk1 = amiga.df1.hasDisk
        let hasDisk2 = amiga.df2.hasDisk
        let hasDisk3 = amiga.df3.hasDisk
        */
        
        let running = amiga.running
        let halted = amiga.cpu.halted
        let warp = amiga.warpMode
        
        // Drive LEDs
        refreshStatusBar(drive: 0, led: motor0)
        refreshStatusBar(drive: 1, led: motor1)
        refreshStatusBar(drive: 2, led: motor2)
        refreshStatusBar(drive: 3, led: motor3)

        // Cylinders
        refreshStatusBar(drive: 0, cylinder: amiga.df0.cylinder)
        refreshStatusBar(drive: 1, cylinder: amiga.df1.cylinder)
        refreshStatusBar(drive: 2, cylinder: amiga.df2.cylinder)
        refreshStatusBar(drive: 3, cylinder: amiga.df3.cylinder)
        refreshStatusBar(writing: nil)
        
        // Animation
        refreshStatusBar(drive: 0, busy: motor0)
        refreshStatusBar(drive: 1, busy: motor1)
        refreshStatusBar(drive: 2, busy: motor2)
        refreshStatusBar(drive: 3, busy: motor3)

        // Drive icons
        /*
        iconSlot0.image = amiga.df0.icon
        iconSlot1.image = amiga.df1.icon
        iconSlot2.image = amiga.df2.icon
        iconSlot3.image = amiga.df3.icon
         */
        
        // Remote server icon
        debugIcon.image = amiga.remoteManager.icon
        
        // Warp mode icon
        warpIcon.image = hourglassIcon
        
        // Visibility
        let items: [NSView: Bool] = [
            
            powerLED: true,
            
            ledSlot0: true,
            ledSlot1: true,
            letSlot2: true,
            ledSlot3: true,
            iconSlot0: true,
            iconSlot1: true,
            iconSlot2: true,
            iconSlot3: true,
            cylSlot0: true,
            cylSlot1: true,
            cylSlot2: true,
            cylSlot3: true,
            busySlot0: true,
            busySlot1: true,
            busySlot2: true,
            busySlot3: true,

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
    
    fileprivate func refreshStatusBar(drive n: Int, led: Bool) {
        
        if let widget = drvLED[n] {

            let image = NSImage(named: led ? "driveLedOn" : "driveLedOff")
            widget.image = image
        }
    }
    
    func refreshStatusBar(drive n: Int, cylinder: Int) {
    
        if let widget = drvCyl[n] {
            
            track("Setting cylidner for slot \(n)")
            widget.integerValue = cylinder
        }
    }

    func refreshStatusBar(drive n: Int, icon: Bool) {
    
        if let widget = drvIcon[n] {
        
            widget.image = amiga.df(n)!.icon
        }
    }

    fileprivate func refreshStatusBar(drive n: Int, busy: Bool) {

        if let widget = drvBusy[n] {
            
            let spin = amiga.running && busy
            spin ? widget.startAnimation(self) : widget.stopAnimation(self)
        }
    }

    func refreshStatusBar(writing: Bool?) {
        
        // TODO
        /*
        let sel = amiga.diskController.selectedDrive
        let w = writing ?? (amiga.diskController.state == .WRITE)

        cylSlot0.textColor = w && (sel == 0) ? .warningColor : .secondaryLabelColor
        cylSlot1.textColor = w && (sel == 1) ? .warningColor : .secondaryLabelColor
        cylSlot2.textColor = w && (sel == 2) ? .warningColor : .secondaryLabelColor
        cylSlot3.textColor = w && (sel == 3) ? .warningColor : .secondaryLabelColor
        */
    }
            
    func assignSlots() {
    
        let config = amiga.diskController.getConfig()
        var nr = 0

        func isConnected(drive: Int) -> Bool {
            
            switch drive {
            case 0: return config.connected.0
            case 1: return config.connected.1
            case 2: return config.connected.2
            case 3: return config.connected.3
            default: return false
            }
        }
        
        for drive in 0...7 {
            
            if isConnected(drive: drive) {
                
                drvLED[drive] = ledSlot[nr]
                drvCyl[drive] = cylSlot[nr]
                drvIcon[drive] = iconSlot[nr]
                drvBusy[drive] = busySlot[nr]
                
                nr += 1
            
            } else {
                
                drvLED[drive] = nil
                drvCyl[drive] = nil
                drvIcon[drive] = nil
                drvBusy[drive] = nil
            }
        }
        
        for slot in nr ..< 4 {

            track("Hiding slot \(slot)")
            
            ledSlot[slot].image = nil
            cylSlot[slot].stringValue = ""
            iconSlot[slot].image = nil
            busySlot[slot].stopAnimation(self)
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
