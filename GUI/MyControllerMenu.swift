// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
extension MyController: NSMenuItemValidation {
    
    open func validateMenuItem(_ item: NSMenuItem) -> Bool {
        
        let powered = emu.poweredOn
        let running = emu.running
        let paused = emu.paused
        let recording = emu.recorder.recording

        var dfn: FloppyDriveProxy { return emu.df(item.tag)! }
        var hdn: HardDriveProxy { return emu.hd(item.tag)! }

        switch item.action {
            
            // Machine menu
        case #selector(MyController.captureScreenAction(_:)):
            item.title = recording ? "Stop Recording" : "Record Screen"
            return true

            // Edit menu
        case #selector(MyController.stopAndGoAction(_:)):
            item.title = running ? "Pause" : "Continue"
            return true
            
        case #selector(MyController.powerAction(_:)):
            item.title = powered ? "Power Off" : "Power On"
            return true
            
        case #selector(MyController.stepIntoAction(_:)),
            #selector(MyController.stepOverAction(_:)),
            #selector(MyController.stopAndGoAction(_:)):
            return paused

            // View menu
        case #selector(MyController.toggleStatusBarAction(_:)):
            item.title = statusBar ? "Hide Status Bar" : "Show Status Bar"
            return true
            
            // Keyboard menu
        case #selector(MyController.mapLeftCmdKeyAction(_:)):
            item.state = myAppDelegate.mapLeftCmdKey ? .on : .off
            return true
        case #selector(MyController.mapRightCmdKeyAction(_:)):
            item.state = myAppDelegate.mapRightCmdKey ? .on : .off
            return true
        case #selector(MyController.mapCapsLockWarpAction(_:)):
            item.state = myAppDelegate.mapCapsLockWarp ? .on : .off
            return true

            // Df<n> menu
        case  #selector(MyController.ejectDiskAction(_:)),
            #selector(MyController.exportFloppyDiskAction(_:)),
            #selector(MyController.inspectFloppyDiskAction(_:)),
            #selector(MyController.inspectDfnVolumeAction(_:)):
            return dfn.info.hasDisk

        case #selector(MyController.insertRecentDiskDummyAction(_:)):
            return !mm.insertedFloppyDisks.isEmpty

        case #selector(MyController.exportRecentDiskDummyAction(_:)):
            if !dfn.info.hasDisk { return false }
            switch item.tag {
            case 0: return !mm.exportedFloppyDisks0.isEmpty
            case 1: return !mm.exportedFloppyDisks1.isEmpty
            case 2: return !mm.exportedFloppyDisks2.isEmpty
            case 3: return !mm.exportedFloppyDisks3.isEmpty
            default: fatalError()
            }

        case #selector(MyController.writeProtectAction(_:)):
            item.state = dfn.info.hasProtectedDisk ? .on : .off
            return dfn.info.hasDisk

            // Hd<n> menu
        case #selector(MyController.attachRecentHdrDummyAction(_:)):
            return !mm.attachedHardDrives.isEmpty

        case #selector(MyController.exportRecentHdrDummyAction(_:)):
            if !hdn.info.hasDisk { return false }
            switch item.tag {
            case 0: return !mm.exportedHardDrives0.isEmpty
            case 1: return !mm.exportedHardDrives1.isEmpty
            case 2: return !mm.exportedHardDrives2.isEmpty
            case 3: return !mm.exportedHardDrives3.isEmpty
            default: fatalError()
            }

        case #selector(MyController.writeProtectHdrAction(_:)):
            item.state = hdn.info.hasProtectedDisk ? .on : .off
            return hdn.info.hasDisk

        default:
            return true
        }
    }
    
    func hideOrShowDriveMenus() {
        
        myAppDelegate.df0Menu.isHidden = !config.df0Connected
        myAppDelegate.df1Menu.isHidden = !config.df1Connected
        myAppDelegate.df2Menu.isHidden = !config.df2Connected
        myAppDelegate.df3Menu.isHidden = !config.df3Connected
        myAppDelegate.hd0Menu.isHidden = !config.hd0Connected
        myAppDelegate.hd1Menu.isHidden = !config.hd1Connected
        myAppDelegate.hd2Menu.isHidden = !config.hd2Connected
        myAppDelegate.hd3Menu.isHidden = !config.hd3Connected
    }
    
    @IBAction func testAction(_ sender: NSMenuItem!) {
        print("test")
    }
    
    //
    // Action methods (App menu)
    //
    
    @IBAction func preferencesAction(_ sender: Any!) {
        
        if myAppDelegate.prefController == nil {
            myAppDelegate.prefController =
            PreferencesController(with: self, nibName: "Preferences")
        }
        myAppDelegate.prefController!.showWindow(self)
        myAppDelegate.prefController!.refresh()
    }

    @IBAction func settingsAction(_ sender: Any?) {

        if myAppDelegate.settingsController == nil {

            let sb = NSStoryboard(name: "Settings", bundle: nil)
            let id = "SettingsWindowController"

            myAppDelegate.settingsController =
            sb.instantiateController(withIdentifier: id) as? SettingsWindowController

            if let sc = myAppDelegate.settingsController {

                sc.window?.level = .floating
                sc.showWindow(self)
                sc.window?.makeKeyAndOrderFront(nil)
                NSApp.activate(ignoringOtherApps: true)
            }
        }
    }

    @IBAction func factorySettingsAction(_ sender: Any!) {
        
        let defaults = EmulatorProxy.defaults!

        // Power off the emulator if the user doesn't object
        if !askToPowerOff() { return }
        
        // Wipe out all settings
        defaults.removeAll()
        defaults.resetSearchPaths()
        defaults.save()
        
        // Apply new settings
        config.applyUserDefaults()
        pref.applyUserDefaults()
        
        // Power on
        emu.powerOn()
        try? emu.run()
    }

    @IBAction func importScriptAction(_ sender: Any!) {

        // Power off the emulator if the user doesn't object
        // if !askToPowerOff() { return }

        myOpenPanel.configure(types: [ .retrosh ], prompt: "Import")
        myOpenPanel.open(for: window, { result in
            
            if result == .OK, let url = self.myOpenPanel.url {

                do {
                    try self.mm.addMedia(url: url, allowedTypes: [.SCRIPT])
                } catch {
                    self.showAlert(.cantOpen(url: url), error: error, async: true)
                }
            }
        })
    }

    //
    // Action methods (Machine menu)
    //
    
    func openConfiguratorAsSheet(tab: String = "") {
        
        if configurator == nil {
            configurator = ConfigurationController(with: self, nibName: "Configuration")
        }
        configurator?.showSheet(tab: tab)
    }

    func openConfiguratorAsWindow() {

        if configurator == nil {
            configurator = ConfigurationController(with: self, nibName: "Configuration")
        }
        configurator?.showAsWindow()
    }

    @IBAction func configureAction(_ sender: Any!) {
        
        openConfiguratorAsWindow()
    }

    func addInspector() {
    
        let count = inspectors.count
        
        // Allow 8 inspectors at a time
        if count < 8, let inspector = Inspector(with: self, nibName: "Inspector") {
            
            inspectors.append(inspector)
            inspector.showAsWindow()

        } else {
         
            NSSound.beep();
        }
    }
    
    @IBAction func inspectorAction(_ sender: Any!) {
        
        if inspectors.isEmpty {
            addInspector()
        } else {
            inspectors[0].showAsWindow()
        }
    }
    
    func addDashboard(type: PanelType = .Combined) {
    
        let count = dashboards.count
        
        // Allow 24 dashboards at a time
        if count < 24 {
            
            let myStoryboard = NSStoryboard(name: "Dashboard", bundle: nil)
            
            if let newDashboard = myStoryboard.instantiateController(withIdentifier: "MyWindowController") as? Dashboard {
                
                dashboards.append(newDashboard)
                newDashboard.setController(self)
                newDashboard.showWindow(self)
                newDashboard.viewController?.type = type
                return
            }
        }

        NSSound.beep();
    }
    
    @IBAction func dashboardAction(_ sender: Any!) {
        
        if dashboards.isEmpty {
            addDashboard()
        } else {
            dashboards[0].showWindow(self)
        }
    }

    @IBAction func consoleAction(_ sender: Any!) {
        
        if renderer.console.isVisible {
            renderer.console.close(delay: 0.25)
        } else {
            renderer.console.open(delay: 0.25)
        }
    }
    
    @IBAction func takeSnapshotAction(_ sender: Any!) {
        
        guard let snapshot = emu.amiga.takeSnapshot() else {
            
            NSSound.beep()
            return
        }
        mydocument.snapshots.append(snapshot, size: snapshot.size)
        renderer.flash()
    }
    
    @IBAction func restoreSnapshotAction(_ sender: Any!) {
        
        do {
            try restoreLatestSnapshot()
        } catch {
            NSSound.beep()
        }
    }
    
    @IBAction func browseSnapshotsAction(_ sender: Any!) {
        
        if snapshotBrowser == nil {
            snapshotBrowser = SnapshotViewer(with: self, nibName: "SnapshotDialog")
        }
        snapshotBrowser?.showAsSheet()
    }

    @IBAction func loadSnapshotAction(_ sender: Any!) {
        
        myOpenPanel.configure(types: [ .snapshot ], prompt: "Restore")
        myOpenPanel.open(for: window, { result in
            
            if result == .OK, let url = self.myOpenPanel.url {

                do {
                    try self.emu.amiga.loadSnapshot(url: url)
                } catch {
                    self.showAlert(.cantOpen(url: url), error: error, async: true)
                }
            }
        })
    }

    @IBAction func saveSnapshotAction(_ sender: Any!) {
        
        mySavePanel.configure(types: [ .snapshot ],
                              prompt: "Export",
                              title: "Export",
                              nameFieldLabel: "Export As:",
                              nameFieldStringValue: "workspace.vasnap")
        
        mySavePanel.open(for: window, { result in
            
            if result == .OK, let url = self.mySavePanel.url {
                
                do {
                    try self.emu.amiga.saveSnapshot(url: url)
                } catch {
                    self.showAlert(.cantExport(url: url), error: error, async: true)
                }
            }
        })
    }
    
    @IBAction func takeScreenshotAction(_ sender: Any!) {
        
        // Take screenshot
        guard let screen = renderer.canvas.screenshot(source: pref.screenshotSource,
                                                      cutout: pref.screenshotCutout,
                                                      width: pref.screenshotWidth,
                                                      height: pref.screenshotHeight) else {
            
            warn("Failed to create screenshot")
            return
        }

        // Convert to Screenshot object
        let screenshot = Screenshot(screen: screen, format: pref.screenshotFormat)

        // Save to disk
        try? screenshot.save()
    }
    
    @IBAction func browseScreenshotsAction(_ sender: Any!) {
        
        if screenshotBrowser == nil {
            screenshotBrowser = ScreenshotViewer(with: self, nibName: "ScreenshotDialog")
        }
        screenshotBrowser?.showAsSheet()
    }
    
    @IBAction func captureScreenAction(_ sender: Any!) {

        if emu.recorder.recording {
            
            emu.recorder.stopRecording()
            return
        }
        
        if !emu.recorder.hasFFmpeg {

            if pref.ffmpegPath != "" {
                showAlert(.noFFmpegFound(exec: pref.ffmpegPath))
            } else {
                showAlert(.noFFmpegInstalled)
            }
            return
        }

        do {
            try emu.recorder.startRecording(rect: renderer.recordingRect,
                                              rate: pref.bitRate,
                                              ax: pref.aspectX,
                                              ay: pref.aspectY)
        } catch {
            
            showAlert(.cantRecord, error: error)
        }
    }
    
    @IBAction func exportVideoAction(_ sender: Any!) {

        let exporter = VideoExporter(with: self, nibName: "VideoExporter")
        exporter?.showAsSheet()
    }
    
    //
    // Action methods (Edit menu)
    //
    
    @IBAction func paste(_ sender: Any!) {

        let pasteBoard = NSPasteboard.general
        guard let text = pasteBoard.string(forType: .string) else {
            
            warn("Cannot paste. No text in pasteboard")
            return
        }
        
        keyboard.autoType(text, max: 256)
    }
    
    @IBAction func stopAndGoAction(_ sender: Any!) {

        if let emu = emu {
            if emu.running { emu.pause() } else { try? emu.run() }
        }
    }

    @IBAction func stepIntoAction(_ sender: Any!) {
        
       emu.stepInto()
    }
    
    @IBAction func stepOverAction(_ sender: Any!) {
        
        emu.stepOver()
    }
    
    @IBAction func resetAction(_ sender: Any!) {
        
        emu.hardReset()
        try? emu.run()
    }
    
    @IBAction func powerAction(_ sender: Any!) {
        
        if emu.poweredOn {
            
            emu.powerOff()
            
        } else {

            do {
                try emu.run()
            } catch {
                showAlert(.cantRun, error: error)
            }
        }
    }

    @IBAction func warpAction(_ sender: Any!) {

        switch WarpMode(rawValue: config.warpMode) {

        case .AUTO: config.warpMode = WarpMode.NEVER.rawValue
        case .NEVER: config.warpMode = WarpMode.ALWAYS.rawValue
        case .ALWAYS: config.warpMode = WarpMode.AUTO.rawValue

        default:
            fatalError()
        }

        refreshStatusBar()
        myAppDelegate.prefController?.refresh()
    }
    
    //
    // Action methods (View menu)
    //
    
    @IBAction
    func toggleStatusBarAction(_ sender: Any!) {
        
        showStatusBar(!statusBar)
    }
    
    //
    // Action methods (Keyboard menu)
    //
    
    @IBAction func stickyKeyboardAction(_ sender: Any!) {
        
        // Create keyboard if neccessary
        if virtualKeyboard == nil {
            virtualKeyboard = VirtualKeyboardController.make(parent: self)
        }
        if virtualKeyboard?.window?.isVisible == true {
            debug(.lifetime, "Virtual keyboard already open")
        } else {
            debug(.lifetime, "Opeining virtual keyboard as a window")
        }
        virtualKeyboard?.showAsWindow()
    }

    @IBAction func mapLeftCmdKeyAction(_ sender: NSMenuItem!) {

        myAppDelegate.mapLeftCmdKey = !myAppDelegate.mapLeftCmdKey
        refreshStatusBar()
    }

    @IBAction func mapRightCmdKeyAction(_ sender: NSMenuItem!) {

        myAppDelegate.mapRightCmdKey = !myAppDelegate.mapRightCmdKey
        refreshStatusBar()
    }

    @IBAction func mapCapsLockWarpAction(_ sender: NSMenuItem!) {

        myAppDelegate.mapCapsLockWarp = !myAppDelegate.mapCapsLockWarp
        refreshStatusBar()
    }

    @IBAction func clearKeyboardMatrixAction(_ sender: Any!) {
        
        emu.keyboard.releaseAll()
    }
    
    @IBAction func delKeyAction(_ sender: Any!) {
        
        type(keyCode: AmigaKeycode.delete)
    }

    @IBAction func helpKeyAction(_ sender: Any!) {
        
        type(keyCode: AmigaKeycode.help)
    }
    
    func type(keyCode: Int, completion: (() -> Void)? = nil) {
        
        Task { @MainActor in

            self.emu.keyboard.press(keyCode)
            usleep(useconds_t(20000))
            self.emu.keyboard.release(keyCode)
            completion?()
        }
    }
    
    //
    // Action methods (Floppy disk menus)
    //
    
    @IBAction func newDiskAction(_ sender: NSMenuItem!) {

        let drive = emu.df(sender.tag)!

        // Ask the user if a modified hard drive should be detached
        if !proceedWithUnsavedFloppyDisk(drive: drive) { return }

        let panel = FloppyCreator(with: self, nibName: "FloppyCreator")
        panel?.showSheet(forDrive: sender.tag)
    }

    @IBAction func insertDiskAction(_ sender: NSMenuItem!) {

        let drive = emu.df(sender.tag)!
        
        // Ask the user if an unsafed disk should be replaced
        if !proceedWithUnsavedFloppyDisk(drive: drive) { return }
        
        myOpenPanel.configure(types: [.adf, .adz, .img, .dms, .exe ], prompt: "Insert")
        myOpenPanel.panel.canChooseDirectories = true
        myOpenPanel.open(for: window, { result in
            
            if result == .OK, let url = self.myOpenPanel.url {

                do {
                    try self.mm.addMedia(df: sender.tag, url: url)
                } catch {
                    self.showAlert(.cantInsert, error: error, async: true)
                }
            }
        })
    }
    
    @IBAction func insertRecentDiskDummyAction(_ sender: NSMenuItem!) {}
    @IBAction func insertRecentDiskAction(_ sender: NSMenuItem!) {
                
        let drive = sender.tag >> 16
        let slot = sender.tag & 0xFFFF

        insertRecentDiskAction(drive: drive, slot: slot)
    }

    func insertRecentDiskAction(drive: Int, slot: Int) {

        debug(.media, "insertRecentDiskAction(drive: \(drive), slot: \(slot)")

        let types: [FileType] = [ .ADF, .EADF, .DMS, .EXE, .DIR ]

        if let url = mm.getRecentlyInsertedDiskURL(slot) {

            do {
                try self.mm.addMedia(url: url, allowedTypes: types, drive: drive)
            } catch {
                self.showAlert(.cantInsert, error: error)
            }
        }
    }

    @IBAction func writeProtectAction(_ sender: NSMenuItem!) {
        
        let dfn = emu.df(sender)!
        dfn.setFlag(.PROTECTED, value: !dfn.getFlag(.PROTECTED))
    }

    @IBAction func exportRecentDiskDummyAction(_ sender: NSMenuItem!) {}
    @IBAction func exportRecentDiskAction(_ sender: NSMenuItem!) {

        let drive = sender.tag >> 16
        let slot = sender.tag % 0xFFFF

        exportRecentAction(df: drive, slot: slot)
    }
    
    func exportRecentAction(df n: Int, slot: Int) {
        
        if let url = mm.getRecentlyExportedDiskURL(slot, df: n) {
            
            do {
                try mydocument.export(drive: n, to: url)
            } catch {
                showAlert(.cantExport(url: url), error: error)
            }
        }
    }

    @IBAction func clearRecentlyInsertedDisksAction(_ sender: NSMenuItem!) {
        
        mm.clearRecentlyInsertedDiskURLs()
    }
    
    @IBAction func clearRecentlyExportedDisksAction(_ sender: NSMenuItem!) {
        
        mm.clearRecentlyExportedDiskURLs(df: sender.tag)
    }
    
    @IBAction func ejectDiskAction(_ sender: NSMenuItem!) {
        
        let drive = emu.df(sender.tag)!
        
        if proceedWithUnsavedFloppyDisk(drive: drive) {
            
            drive.eject()
            mm.clearRecentlyExportedDiskURLs(df: drive.info.nr)
        }
    }
    
    @IBAction func exportFloppyDiskAction(_ sender: NSMenuItem!) {
        
        let exportPanel = DiskExporter(with: self, nibName: "DiskExporter")
        exportPanel?.showSheet(diskDrive: sender.tag)
    }
    
    @IBAction func inspectFloppyDiskAction(_ sender: NSMenuItem!) {
        
        let panel = DiskInspector(with: self, nibName: "DiskInspector")
        panel?.show(diskDrive: sender.tag)
    }

    @IBAction func inspectDfnVolumeAction(_ sender: NSMenuItem!) {

        let panel = VolumeInspector(with: self, nibName: "VolumeInspector")

        do {
            
            try panel?.show(diskDrive: sender.tag)
            
        } catch {
            
            showAlert(.cantDecode, error: error, window: window)
        }
    }

    //
    // Action methods (Hard drive menus)
    //
    
    @IBAction func newHdrAction(_ sender: NSMenuItem!) {

        let drive = emu.hd(sender.tag)!
        
        // Ask the user if an unsafed disk should be discarded
        if !proceedWithUnsavedHardDisk(drive: drive) { return }
        
        // Power off the emulator if the user doesn't object
        if !askToPowerOff() { return }
        
        let panel = HardDiskCreator(with: self, nibName: "HardDiskCreator")
        panel?.show(forDrive: drive.traits.nr)
    }
    
    @IBAction func attachHdrAction(_ sender: NSMenuItem!) {
        
        let drive = emu.hd(sender.tag)!
        
        // Ask the user if an unsafed disk should be discarded
        if !proceedWithUnsavedHardDisk(drive: drive) { return }

        myOpenPanel.configure(types: [ .hdf, .hdz, .zip, .gzip ], prompt: "Attach")
        myOpenPanel.open(for: window, { result in
            
            if result == .OK, let url = self.myOpenPanel.url {
                
                DispatchQueue.main.async {
                    
                    do {
                        try self.mm.addMedia(hd: sender.tag, url: url)
                    } catch {
                        self.showAlert(.cantAttach, error: error, async: true)
                    }
                }
            }
        })
    }
    
    @IBAction func attachRecentHdrDummyAction(_ sender: NSMenuItem!) {}
    @IBAction func attachRecentHdrAction(_ sender: NSMenuItem!) {
        
        let drive = sender.tag >> 16
        let slot = sender.tag & 0xFFFF

        if let url = mm.getRecentlyAttachedHdrURL(slot) {
            
            do {
                try self.mm.addMedia(hd: drive, url: url)
            } catch {
                self.showAlert(.cantAttach, error: error)
            }
        }
    }

    @IBAction func detachHdrAction(_ sender: NSMenuItem!) {
        
        do {
            try mm.detach(hd: sender.tag)
        } catch {
            showAlert(.cantDetach, error: error)
        }
    }

    @IBAction func exportRecentHdrDummyAction(_ sender: NSMenuItem!) {}
    @IBAction func exportRecentHdrAction(_ sender: NSMenuItem!) {

        let drive = sender.tag >> 16
        let slot = sender.tag & 0xFFFF

        exportRecentAction(hd: drive, slot: slot)
    }

    func exportRecentAction(hd n: Int, slot: Int) {
        
        debug(1, "hd\(n) slot: \(slot)")

        if let url = mm.getRecentlyExportedHdrURL(slot, hd: n) {

            debug(1, "url: \(url)")

            do {
                try mydocument.export(hardDrive: n, to: url)

            } catch {
                showAlert(.cantExport(url: url), error: error)
            }
        }
    }
    
    @IBAction func clearRecentlyAttachedHdrsAction(_ sender: NSMenuItem!) {
        
        mm.clearRecentlyAttachedHdrURLs()
    }
    
    @IBAction func clearRecentlyExportedHdrsAction(_ sender: NSMenuItem!) {
        
        mm.clearRecentlyExportedHdrURLs(hd: sender.tag)
    }
    
    @IBAction func exportHdrAction(_ sender: NSMenuItem!) {
        
        let exportPanel = DiskExporter(with: self, nibName: "DiskExporter")
        exportPanel?.showSheet(hardDrive: sender.tag)
    }
        
    @IBAction func inspectHdrDiskAction(_ sender: NSMenuItem!) {

        let panel = DiskInspector(with: self, nibName: "DiskInspector")
        panel?.show(hardDrive: sender.tag)
    }

    @IBAction func inspectHdrVolumeAction(_ sender: NSMenuItem!) {
        
        let panel = VolumeInspector(with: self, nibName: "VolumeInspector")

        do {
            
            try panel?.show(hardDrive: sender.tag)

        } catch {
            
            showAlert(.cantDecode, error: error, window: window)
        }
    }

    @IBAction func configureHdrAction(_ sender: NSMenuItem!) {
        
        let panel = HardDiskConfigurator(with: self, nibName: "HardDiskConfigurator")
        panel?.show(forDrive: sender.tag)
    }
    
    @IBAction func writeProtectHdrAction(_ sender: NSMenuItem!) {
        
        let hdn = emu.hd(sender)!
        hdn.setFlag(.PROTECTED, value: !hdn.getFlag(.PROTECTED))
    }

    //
    // Action methods (Window menu)
    //

    // Resizes the window such that every texture line hits a display line
    @IBAction func autoResizeWindow(_ sender: NSMenuItem!) {

        let height = renderer.canvas.visible.height * 2

        debug(.metal, "Old metal view: \(metal.frame)")
        debug(.metal, "Visible texture lines: \(height)")

        adjustWindowSize(height: height)

        debug(.metal, "New metal view: \(metal.frame)")
    }
}
