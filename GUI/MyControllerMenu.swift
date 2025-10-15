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
        
        guard let emu = emu else { return true }
        
        let powered = emu.poweredOn
        let running = emu.running
        let paused = emu.paused
        
        var dfn: FloppyDriveProxy { return emu.df(item.tag)! }
        var hdn: HardDriveProxy { return emu.hd(item.tag)! }
        
        func validateURLlist(_ list: [URL], image: NSImage) {
            
            let slot = item.tag & 0xFF
            
            if let url = MediaManager.getRecentlyUsedURL(slot, from: list) {
                item.title = url.lastPathComponent
                item.isHidden = false
                item.image = image
            } else {
                item.title = ""
                item.isHidden = true
                item.image = nil
            }
        }
        
        switch item.action {
            
            // Edit menu
        case #selector(MyController.grabMouseAction(_:)):
            item.title = metal.gotMouse ? "Release Mouse" : "Retain Mouse"
            return true
            
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
        case #selector(MyController.mapCapsLockWarpAction(_:)):
            item.state = myAppDelegate.mapCapsLockWarp ? .on : .off
            return true
            
            // Dfn menus
        case #selector(MyController.insertRecentDiskAction(_:)):
            validateURLlist(MediaManager.insertedFloppyDisks, image: smallDisk)
            return true
            
        case #selector(MyController.exportRecentDiskDummyAction(_:)):
            let empty = mm.getRecentlyExportedDiskURL(0, df: item.tag) == nil
            return dfn.info.hasDisk && !empty
            
        case #selector(MyController.exportRecentDiskAction(_:)):
            let urls = mm.getRecentlyExportedDiskURLs(df: item.tag >> 16)
            validateURLlist(urls, image: smallDisk)
            return true
            
        case #selector(MyController.ejectDiskAction(_:)),
            #selector(MyController.exportFloppyDiskAction(_:)),
            #selector(MyController.inspectFloppyDiskAction(_:)),
            #selector(MyController.inspectDfnVolumeAction(_:)):
            return dfn.info.hasDisk
            
        case #selector(MyController.writeProtectAction(_:)):
            item.state = dfn.info.hasProtectedDisk ? .on : .off
            return dfn.info.hasDisk
            
            // Hdn menus
        case #selector(MyController.attachRecentHdrAction(_:)):
            validateURLlist(MediaManager.attachedHardDrives, image: smallHdr)
            return true
            
        case #selector(MyController.exportRecentHdrDummyAction(_:)):
            let empty = mm.getRecentlyExportedHdrURL(0, hd: item.tag) == nil
            return hdn.info.hasDisk && !empty
            
        case #selector(MyController.exportRecentHdrAction(_:)):
            let urls = mm.getRecentlyExportedHdrURLs(hd: item.tag >> 16)
            validateURLlist(urls, image: smallHdr)
            return true
            
        case #selector(MyController.writeProtectHdrAction(_:)):
            item.state = hdn.info.hasProtectedDisk ? .on : .off
            return hdn.info.hasDisk
            
        default:
            return item.isEnabled
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
    
    //
    // Action methods (App menu)
    //
    
    /*
     @IBAction func preferencesAction(_ sender: Any!) {
     
     if myAppDelegate.prefController == nil {
     myAppDelegate.prefController =
     PreferencesController(with: self, nibName: "Preferences")
     }
     myAppDelegate.prefController!.showWindow(self)
     myAppDelegate.prefController!.refresh()
     }
     */
    
    @IBAction func settingsAction(_ sender: Any?) {
        
        if myAppDelegate.settingsController == nil {
            
            let sb = NSStoryboard(name: "Settings", bundle: nil)
            let id = "SettingsWindowController"
            
            myAppDelegate.settingsController =
            sb.instantiateController(withIdentifier: id) as? SettingsWindowController
        }
        
        if let sc = myAppDelegate.settingsController {
            
            sc.window?.level = .floating
            sc.showWindow(self)
            sc.window?.makeKeyAndOrderFront(nil)
            NSApp.activate(ignoringOtherApps: true)
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
        
        // Launch the onboarding agent
        renderer.onboarding.open(delay: 1.0)
    }
    
    @IBAction func importScriptAction(_ sender: Any!) {
        
        // Power off the emulator if the user doesn't object
        // if !askToPowerOff() { return }
        
        myOpenPanel.configure(types: [ .retrosh ], prompt: "Import")
        myOpenPanel.open(for: window, { result in
            
            if result == .OK, let url = self.myOpenPanel.url {
                
                do {
                    try self.mm.mount(url: url, allowedTypes: [.SCRIPT])
                } catch {
                    self.showAlert(.cantOpen(url: url), error: error, async: true)
                }
            }
        })
    }
    
    //
    // Action methods (Machine menu)
    //
    
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
        
        guard let snapshot = emu?.amiga.takeSnapshot(pref.snapshotCompressor) else {
            
            NSSound.beep()
            return
        }
        if !mydocument.appendSnapshot(file: snapshot) {
            
            NSSound.beep()
            return
        }
        
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
                    try self.emu?.amiga.loadSnapshot(url: url)
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
                    try self.emu?.amiga.saveSnapshot(url: url)
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
        
        // Create a visual effect
        // renderer.flash()
    }
    
    @IBAction func browseScreenshotsAction(_ sender: Any!) {
        
        if screenshotBrowser == nil {
            screenshotBrowser = ScreenshotViewer(with: self, nibName: "ScreenshotDialog")
        }
        screenshotBrowser?.showAsSheet()
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
    
    @IBAction func grabMouseAction(_ sender: Any!) {
        
        metal.gotMouse ? metal.releaseMouse() : metal.retainMouse()
    }
    
    @IBAction func stopAndGoAction(_ sender: Any!) {
        
        if let emu = emu {
            if emu.running { emu.pause() } else { try? emu.run() }
        }
    }
    
    @IBAction func stepIntoAction(_ sender: Any!) {
        
        emu?.stepInto()
    }
    
    @IBAction func stepOverAction(_ sender: Any!) {
        
        emu?.stepOver()
    }
    
    @IBAction func resetAction(_ sender: Any!) {
        
        emu?.hardReset()
        try? emu?.run()
    }
    
    @IBAction func powerAction(_ sender: Any!) {
        
        if emu?.poweredOn == true {
            
            emu?.powerOff()
            
        } else {
            
            do {
                try emu?.run()
            } catch {
                showAlert(.cantRun, error: error)
            }
        }
    }
    
    @IBAction func warpAction(_ sender: Any!) {
        
        switch Warp(rawValue: config.warpMode) {
            
        case .AUTO: config.warpMode = Warp.NEVER.rawValue
        case .NEVER: config.warpMode = Warp.ALWAYS.rawValue
        case .ALWAYS: config.warpMode = Warp.AUTO.rawValue
            
        default:
            fatalError()
        }
        
        refreshStatusBar()
        myAppDelegate.settingsController?.refresh()
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
    
    @IBAction func mapCapsLockWarpAction(_ sender: NSMenuItem!) {
        
        myAppDelegate.mapCapsLockWarp = !myAppDelegate.mapCapsLockWarp
        refreshStatusBar()
    }
    
    @IBAction func clearKeyboardMatrixAction(_ sender: Any!) {
        
        emu?.keyboard.releaseAll()
    }
    
    @IBAction func delKeyAction(_ sender: Any!) {
        
        type(keyCode: AmigaKeycode.delete)
    }
    
    @IBAction func helpKeyAction(_ sender: Any!) {
        
        type(keyCode: AmigaKeycode.help)
    }
    
    func type(keyCode: Int, completion: (() -> Void)? = nil) {
        
        Task { @MainActor in
            
            self.emu?.keyboard.press(keyCode)
            usleep(useconds_t(20000))
            self.emu?.keyboard.release(keyCode)
            completion?()
        }
    }
    
    //
    // Action methods (Floppy disk menus)
    //
    
    @IBAction func newDiskAction(_ sender: NSMenuItem!) {
        
        if let drive = emu?.df(sender.tag) {
            
            // Ask the user if a modified hard drive should be detached
            if !proceedWithUnsavedFloppyDisk(drive: drive) { return }
            
            let panel = FloppyCreator(with: self, nibName: "FloppyCreator")
            panel?.showSheet(forDrive: sender.tag)
        }
    }
    
    @IBAction func insertDiskAction(_ sender: NSMenuItem!) {
        
        if let drive = emu?.df(sender.tag) {
            
            // Ask the user if an unsafed disk should be replaced
            if !proceedWithUnsavedFloppyDisk(drive: drive) { return }
            
            myOpenPanel.configure(types: [.adf, .adz, .img, .dms, .exe ], prompt: "Insert")
            myOpenPanel.panel.canChooseDirectories = true
            myOpenPanel.open(for: window, { result in
                
                if result == .OK, let url = self.myOpenPanel.url {
                    
                    do {
                        try self.mm.mount(df: sender.tag, url: url)
                    } catch {
                        self.showAlert(.cantInsert, error: error, async: true)
                    }
                }
            })
        }
    }
    
    @IBAction func insertRecentDiskDummyAction(_ sender: NSMenuItem!) {}
    @IBAction func insertRecentDiskAction(_ sender: NSMenuItem!) {
        
        let drive = sender.tag >> 16
        let slot = sender.tag & 0xFFFF
        
        insertRecentDiskAction(df: drive, slot: slot)
    }
    
    func insertRecentDiskAction(df n: Int, slot: Int) {
        
        debug(.media, "insertRecentDiskAction(df: \(n), slot: \(slot))")
        
        let types: [FileType] = [ .ADF, .EADF, .DMS, .EXE, .DIR ]
        
        if let url = MediaManager.getRecentlyInsertedDiskURL(slot) {
            
            do {
                try self.mm.mount(url: url, allowedTypes: types, drive: n)
            } catch {
                self.showAlert(.cantInsert, error: error)
            }
        }
    }
    
    @IBAction func writeProtectAction(_ sender: NSMenuItem!) {
        
        if let dfn = emu?.df(sender) {
            dfn.setFlag(.PROTECTED, value: !dfn.getFlag(.PROTECTED))
        }
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
        
        MediaManager.clearRecentlyInsertedDiskURLs()
    }
    
    @IBAction func clearRecentlyExportedDisksAction(_ sender: NSMenuItem!) {
        
        mm.clearRecentlyExportedDiskURLs(df: sender.tag)
    }
    
    @IBAction func ejectDiskAction(_ sender: NSMenuItem!) {
        
        if let drive = emu?.df(sender.tag) {
            
            if proceedWithUnsavedFloppyDisk(drive: drive) {
                
                drive.eject()
                mm.clearRecentlyExportedDiskURLs(df: drive.info.nr)
            }
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
        
        if let drive = emu?.hd(sender.tag) {
            
            // Ask the user if an unsafed disk should be discarded
            if !proceedWithUnsavedHardDisk(drive: drive) { return }
            
            // Power off the emulator if the user doesn't object
            if !askToPowerOff() { return }
            
            let panel = HardDiskCreator(with: self, nibName: "HardDiskCreator")
            panel?.show(forDrive: drive.traits.nr)
        }
    }
    
    @IBAction func attachHdrAction(_ sender: NSMenuItem!) {
        
        if let drive = emu?.hd(sender.tag) {
            
            // Ask the user if an unsafed disk should be discarded
            if !proceedWithUnsavedHardDisk(drive: drive) { return }
            
            myOpenPanel.configure(types: [ .hdf, .hdz, .zip, .gzip ], prompt: "Attach")
            myOpenPanel.open(for: window, { result in
                
                if result == .OK, let url = self.myOpenPanel.url {
                    
                    DispatchQueue.main.async {
                        
                        do {
                            try self.mm.mount(hd: sender.tag, url: url)
                        } catch {
                            self.showAlert(.cantAttach, error: error, async: true)
                        }
                    }
                }
            })
        }
    }
    
    @IBAction func attachRecentHdrDummyAction(_ sender: NSMenuItem!) {}
    @IBAction func attachRecentHdrAction(_ sender: NSMenuItem!) {
        
        let drive = sender.tag >> 16
        let slot = sender.tag & 0xFFFF
        
        attachRecentHdrAction(hd: drive, slot: slot)
    }
    
    func attachRecentHdrAction(hd n: Int, slot: Int) {
        
        debug(.media, "attachRecentHdrAction(hd: \(n), slot: \(slot))")
        
        if let url = MediaManager.getRecentlyAttachedHdrURL(slot) {
            
            do {
                try self.mm.mount(hd: n, url: url)
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
        
        debug(.media, "exportRecentAction(hd: \(n), slot: \(slot))")
        
        if let url = mm.getRecentlyExportedHdrURL(slot, hd: n) {
            
            do {
                try mydocument.export(hardDrive: n, to: url)
                
            } catch {
                showAlert(.cantExport(url: url), error: error)
            }
        }
    }
    
    @IBAction func clearRecentlyAttachedHdrsAction(_ sender: NSMenuItem!) {
        
        MediaManager.clearRecentlyAttachedHdrURLs()
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
        
        if let hdn = emu?.hd(sender) {
            hdn.setFlag(.PROTECTED, value: !hdn.getFlag(.PROTECTED))
        }
    }
    
    //
    // Action methods (Window menu)
    //
    
    // Resizes the window such that every texture line hits a display line
    @IBAction func autoResizeWindow(_ sender: Any) {
        
        let height = renderer.canvas.visible.height * 2
        
        debug(.metal, "Old metal view: \(metal.frame)")
        debug(.metal, "Visible texture lines: \(height)")
        
        adjustWindowSize(height: height)
        
        debug(.metal, "New metal view: \(metal.frame)")
    }
}
