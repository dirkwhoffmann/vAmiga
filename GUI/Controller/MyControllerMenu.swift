// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension MyController: NSMenuItemValidation {
    
    open func validateMenuItem(_ item: NSMenuItem) -> Bool {
        
        let powered = amiga.isPoweredOn
        let running = amiga.isRunning
        let paused = amiga.isPaused
        let recording = amiga.screenRecorder.recording
        
        var dfn: DriveProxy { return amiga.df(item.tag)! }
        
        func validateURLlist(_ list: [URL], image: NSImage) -> Bool {
            
            let slot = item.tag % 10
            
            if let url = myAppDelegate.getRecentlyUsedURL(slot, from: list) {
                item.title = url.lastPathComponent
                item.isHidden = false
                item.image = image
            } else {
                item.title = ""
                item.isHidden = true
                item.image = nil
            }
            
            return true
        }
        
        switch item.action {
            
        // Machine menu
        case #selector(MyController.importConfigAction(_:)),
             #selector(MyController.exportConfigAction(_:)),
             #selector(MyController.resetConfigAction(_:)):
            return !powered

        case #selector(MyController.captureScreenAction(_:)):
            item.title = recording ? "Stop screen recording" : "Start screen recording"
            return true

        // Edit menu
        case #selector(MyController.stopAndGoAction(_:)):
            item.title = running ? "Pause" : "Continue"
            return true
            
        case #selector(MyController.powerAction(_:)):
            item.title = powered ? "Power Off" : "Power On"
            return true
            
        // View menu
        case #selector(MyController.toggleStatusBarAction(_:)):
            item.title = statusBar ? "Hide Status Bar" : "Show Status Bar"
            return true
            
        // Keyboard menu
        case #selector(MyController.mapCmdKeysAction(_:)):
            item.state = (myAppDelegate.eventTap != nil) ? .on : .off
            return true
            
        // Drive menu
        case #selector(MyController.insertRecentDiskAction(_:)):
            
            return validateURLlist(myAppDelegate.recentlyInsertedDiskURLs, image: smallDisk)
            
        case  #selector(MyController.ejectDiskAction(_:)),
              #selector(MyController.exportDiskAction(_:)):
            return dfn.hasDisk
            
        case #selector(MyController.exportRecentDiskDummyAction0(_:)):
            return amiga.df0.hasDisk
            
        case #selector(MyController.exportRecentDiskDummyAction1(_:)):
            return amiga.df1.hasDisk
            
        case #selector(MyController.exportRecentDiskDummyAction2(_:)):
            return amiga.df2.hasDisk
            
        case #selector(MyController.exportRecentDiskDummyAction3(_:)):
            return amiga.df3.hasDisk
            
        case #selector(MyController.exportRecentDiskAction(_:)):
            switch item.tag {
            case 0: return validateURLlist(myAppDelegate.recentlyExportedDisk0URLs, image: smallDisk)
            case 10: return validateURLlist(myAppDelegate.recentlyExportedDisk1URLs, image: smallDisk)
            case 20: return validateURLlist(myAppDelegate.recentlyExportedDisk2URLs, image: smallDisk)
            case 30: return validateURLlist(myAppDelegate.recentlyExportedDisk3URLs, image: smallDisk)
            default: fatalError()
            }
            
        case #selector(MyController.writeProtectAction(_:)):
            item.state = dfn.hasWriteProtectedDisk() ? .on : .off
            return dfn.hasDisk
            
        case #selector(MyController.dragAndDropTargetAction(_:)):
            item.state = dfn === dragAndDropDrive ? .on : .off
            return true
            
        // Debug menu
        case #selector(MyController.stepIntoAction(_:)),
             #selector(MyController.stepOverAction(_:)),
             #selector(MyController.stopAndGoAction(_:)):
            return paused
            
        case #selector(MyController.dumpStateAction(_:)):
            return !amiga.isReleaseBuild
            
        default:
            return true
        }
    }
    
    func hideOrShowDriveMenus() {
        
        myAppDelegate.df0Menu.isHidden = !config.df0Connected
        myAppDelegate.df1Menu.isHidden = !config.df1Connected
        myAppDelegate.df2Menu.isHidden = !config.df2Connected
        myAppDelegate.df3Menu.isHidden = !config.df3Connected
    }
    
    //
    // Action methods (App menu)
    //
    
    @IBAction func preferencesAction(_ sender: Any!) {
        
        if myAppDelegate.prefController == nil {
            myAppDelegate.prefController =
                PreferencesController.make(parent: self,
                                           nibName: NSNib.Name("NewPreferences"))
        }
        myAppDelegate.prefController!.showWindow(self)
        myAppDelegate.prefController!.refresh()
    }
    
    func importPrefs(_ prefixes: [String]) {
        
        track("Importing user defaults with prefixes \(prefixes)")
        
        let panel = NSOpenPanel()
        panel.prompt = "Import"
        panel.allowedFileTypes = ["vaconf"]
        
        panel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = panel.url {
                    self.loadUserDefaults(url: url, prefixes: prefixes)
                }
            }
        })
    }
    
    func exportPrefs(_ prefixes: [String]) {
        
        track("Exporting user defaults with prefixes \(prefixes)")
        
        let panel = NSSavePanel()
        panel.prompt = "Export"
        panel.allowedFileTypes = ["vaconf"]
        
        panel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = panel.url {
                    track()
                    self.saveUserDefaults(url: url, prefixes: prefixes)
                }
            }
        })
    }
    
    @IBAction func importConfigAction(_ sender: Any!) {
        
        importPrefs(["VAMIGA_ROM", "VAMIGA_HW", "VAMIGA_COM", "VAMIGA_VID", "VAMIGA_AUD"])
    }
    
    @IBAction func exportConfigAction(_ sender: Any!) {
        
        exportPrefs(["VAMIGA_ROM", "VAMIGA_HW", "VAMIGA_COM", "VAMIGA_VID", "VAMIGA_AUD"])
    }
    
    @IBAction func resetConfigAction(_ sender: Any!) {
        
        track()
        
        UserDefaults.resetRomUserDefaults()
        UserDefaults.resetHardwareUserDefaults()
        UserDefaults.resetCompatibilityUserDefaults()
        UserDefaults.resetAudioUserDefaults()
        UserDefaults.resetVideoUserDefaults()
        
        amiga.suspend()
        config.loadRomUserDefaults()
        config.loadHardwareUserDefaults()
        config.loadCompatibilityUserDefaults()
        config.loadAudioUserDefaults()
        config.loadVideoUserDefaults()
        amiga.resume()
    }
    
    //
    // Action methods (File menu)
    //
    
    func openConfigurator(tab: String = "") {
        
        if configurator == nil {
            let name = NSNib.Name("Configuration")
            configurator = ConfigurationController.make(parent: self, nibName: name)
        }
        configurator?.showSheet(tab: tab)
    }
    
    @IBAction func configureAction(_ sender: Any!) {
        
        openConfigurator()
    }
 
    @IBAction func inspectorAction(_ sender: Any!) {
        
        if inspector == nil {
            inspector = Inspector.make(parent: self, nibName: "Inspector")
        }
        inspector?.showWindow(self)
    }
    
    @IBAction func monitorAction(_ sender: Any!) {
        
        if monitor == nil {
            monitor = Monitor.make(parent: self, nibName: "Monitor")
        }
        monitor?.showWindow(self)
    }
    
    @IBAction func takeSnapshotAction(_ sender: Any!) {
        
        takeUserSnapshot()
        renderer.blendIn(steps: 20)
    }
    
    @IBAction func restoreSnapshotAction(_ sender: Any!) {
        
        if !restoreLatestUserSnapshot() {
            NSSound.beep()
            return
        }
        
        renderer.blendIn(steps: 20)
    }
    
    @IBAction func browseSnapshotsAction(_ sender: Any!) {
        
        if snapshotBrowser == nil {
            let name = NSNib.Name("SnapshotDialog")
            snapshotBrowser = SnapshotDialog.make(parent: self, nibName: name)
        }
        snapshotBrowser?.showSheet()
    }
    
    @IBAction func takeScreenshotAction(_ sender: Any!) {
        
        takeUserScreenshot()
        renderer.blendIn(steps: 20)
    }
    
    @IBAction func browseScreenshotsAction(_ sender: Any!) {
        
        if screenshotBrowser == nil {
            let name = NSNib.Name("ScreenshotDialog")
            screenshotBrowser = ScreenshotDialog.make(parent: self, nibName: name)
        }
        screenshotBrowser?.checksum = amiga.df0.fnv
        screenshotBrowser?.showSheet()
    }
    
    @IBAction func captureScreenAction(_ sender: Any!) {
        
        track()
        
        if amiga.screenRecorder.recording {
            amiga.screenRecorder.stopRecording()
        } else {
            let rect = NSRect.init(x: 0, y: 0, width: 800, height: 600)
            amiga.screenRecorder.startRecording(rect,
                                                bitRate: pref.bitRate,
                                                videoCodec: pref.videoCodec,
                                                audioCodec: pref.audioCodec)
        }
    }

    //
    // Action methods (Edit menu)
    //
    
    @IBAction func paste(_ sender: Any!) {
        
        track()
        
        let pasteBoard = NSPasteboard.general
        guard let text = pasteBoard.string(forType: .string) else {
            track("Cannot paste. No text in pasteboard")
            return
        }
        
        keyboard.autoType(text)
    }
    
    @IBAction func stopAndGoAction(_ sender: NSButton!) {
        
        amiga?.stopAndGo()
    }
    
    @IBAction func stepIntoAction(_ sender: NSButton!) {
        
        needsSaving = true
        amiga?.stepInto()
    }
    
    @IBAction func stepOverAction(_ sender: NSButton!) {
        
        needsSaving = true
        amiga?.stepOver()
    }
    
    @IBAction func resetAction(_ sender: Any!) {
        
        track()
        renderer.blendIn(steps: 20)
        amiga.hardReset()
        amiga.run()
    }
    
    @IBAction func powerAction(_ sender: Any!) {
        
        var error: ErrorCode = .ERR_OK

        if amiga.isPoweredOn {
            amiga.powerOff()
            return
        }
        
        if amiga.isReady(&error) {
            amiga.run()
        } else {
            mydocument.showConfigurationAltert(error)
        }
    }
    
    //
    // Action methods (View menu)
    //
    
    @IBAction func toggleStatusBarAction(_ sender: Any!) {
        
        undoManager?.registerUndo(withTarget: self) { targetSelf in
            targetSelf.toggleStatusBarAction(sender)
        }
        
        showStatusBar(!statusBar)
    }
    
    //
    // Action methods (Keyboard menu)
    //
    
    @IBAction func keyboardAction(_ sender: Any!) {
        
        // Open the virtual keyboard as a sheet
        if virtualKeyboard == nil {
            virtualKeyboard = VirtualKeyboardController.make(parent: self)
        }
        virtualKeyboard?.showSheet()
    }
     
    @IBAction func mapCmdKeysAction(_ sender: Any!) {
        
        myAppDelegate.mapCommandKeys = !myAppDelegate.mapCommandKeys
        refreshStatusBar()
    }
    
    @IBAction func clearKeyboardMatrixAction(_ sender: Any!) {
        
        amiga.keyboard.releaseAllKeys()
    }
    
    //
    // Action methods (Disk menu)
    //
    
    @IBAction func newDiskAction(_ sender: NSMenuItem!) {
        
        track()

        // Get drive type
        let type = DriveType.init(rawValue: config.dfnType(sender.tag))
        
        // Create a blank disk
        var adf: ADFFileProxy
        switch type {
        case .DRIVE_35_DD:  adf = ADFFileProxy.make(with: .DISK_35_DD)
        case .DRIVE_525_SD: adf = ADFFileProxy.make(with: .DISK_525_SD)
        default: fatalError()
        }
        
        // Write file system
        adf.formatDisk(pref.driveBlankDiskFormat)
        
        // Insert disk into drive
        amiga.diskController.insert(sender.tag, adf: adf)

        myAppDelegate.clearRecentlyExportedDiskURLs(drive: sender.tag)
    }
    
    @IBAction func insertDiskAction(_ sender: NSMenuItem!) {
        
        // Ask the user if an unsafed disk should be replaced
        if !proceedWithUnexportedDisk(drive: sender.tag) { return }
        
        // Show the OpenPanel
        let openPanel = NSOpenPanel()
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        openPanel.canCreateDirectories = false
        openPanel.canChooseFiles = true
        openPanel.prompt = "Insert"
        openPanel.allowedFileTypes = ["adf", "dms", "img", "ima", "adz", "zip", "gz"]
        openPanel.beginSheetModal(for: window!, completionHandler: { result in

            if result == .OK, let url = openPanel.url {
                self.insertDiskAction(from: url, drive: sender.tag)
            }
        })
    }
    
    @IBAction func insertRecentDiskAction(_ sender: NSMenuItem!) {
        
        let drive = sender.tag / 10
        let slot  = sender.tag % 10
        
        insertRecentDiskAction(drive: drive, slot: slot)
    }

    func insertRecentDiskAction(drive: Int, slot: Int) {
        
        if let url = myAppDelegate.getRecentlyInsertedDiskURL(slot) {
            insertDiskAction(from: url, drive: drive)
        }
    }
    
    func insertDiskAction(from url: URL, drive: Int) {
        
        do {
            // Try to create the ADF proxy object
            let proxy = try mydocument.createADFProxy(from: url)
            
            // Ask the user if an unsafed disk should be replaced
            if !proceedWithUnexportedDisk(drive: drive) { return }
            
            // Insert the disk
            amiga.diskController.insert(drive, adf: proxy)
            
            // Remember the URL
            myAppDelegate.noteNewRecentlyUsedURL(url)
            
        } catch {
            NSApp.presentError(error)
        }
    }
    
    @IBAction func writeProtectAction(_ sender: NSMenuItem!) {
        
        amiga.suspend()
        amiga.df(sender)!.toggleWriteProtection()
        amiga.resume()
    }
    
    @IBAction func exportRecentDiskDummyAction0(_ sender: NSMenuItem!) {}
    @IBAction func exportRecentDiskDummyAction1(_ sender: NSMenuItem!) {}
    @IBAction func exportRecentDiskDummyAction2(_ sender: NSMenuItem!) {}
    @IBAction func exportRecentDiskDummyAction3(_ sender: NSMenuItem!) {}
    
    func exportRecentDiskAction(drive: Int, slot: Int) {
        
        track("drive: \(drive) slot: \(slot)")
        
        if let url = myAppDelegate.getRecentlyExportedDiskURL(slot, drive: drive) {
            do {
                mydocument.export(drive: drive, to: url)
            }
        }
    }

    @IBAction func exportRecentDiskAction(_ sender: NSMenuItem!) {
        
        track()
        
        let drive = sender.tag / 10
        let slot  = sender.tag % 10
                
        exportRecentDiskAction(drive: drive, slot: slot)
    }
    
    @IBAction func clearRecentlyInsertedDisksAction(_ sender: NSMenuItem!) {
        
        myAppDelegate.recentlyInsertedDiskURLs = []
    }
    
    @IBAction func clearRecentlyExportedDisksAction(_ sender: NSMenuItem!) {
        
        myAppDelegate.clearRecentlyExportedDiskURLs(drive: sender.tag)
    }
    
    @IBAction func ejectDiskAction(_ sender: NSMenuItem!) {
        
        if proceedWithUnexportedDisk(drive: sender.tag) {
            amiga.diskController.eject(sender.tag)
            myAppDelegate.clearRecentlyExportedDiskURLs(drive: sender.tag)
        }
    }
    
    @IBAction func exportDiskAction(_ sender: NSMenuItem!) {
        
        let nibName = NSNib.Name("ExportDiskDialog")
        let exportPanel = ExportDiskDialog.make(parent: self, nibName: nibName)
        exportPanel?.showSheet(forDrive: sender.tag)
    }
    
    @IBAction func dragAndDropTargetAction(_ sender: NSMenuItem!) {
        
        let drive = amiga.df(sender)
        dragAndDropDrive = (dragAndDropDrive == drive) ? nil : drive
    }
    
    //
    // Action methods (Debug menu)
    //
    
    @IBAction func hideSpriteAction(_ sender: NSMenuItem!) {

        var mask = amiga.getConfig(.OPT_HIDDEN_SPRITES)
        
        sender.state = (sender.state == .off) ? .on : .off
        if sender.state == .on {
            mask |= 1 << sender.tag
        } else {
            mask &= ~(1 << sender.tag)
        }
        
        amiga.suspend()
        amiga.configure(.OPT_HIDDEN_SPRITES, value: mask)
        amiga.resume()
        
        track()
    }
    
    @IBAction func dumpStateAction(_ sender: Any!) {
        // Dummy target to make menu item validatable
    }
    @IBAction func dumpAmigaAction(_ sender: Any!) {
        amiga.suspend()
        amiga.dump()
        amiga.resume()
    }
    @IBAction func dumpCPUAction(_ sender: Any!) {
        amiga.suspend()
        amiga.cpu.dump()
        amiga.resume()
    }
    @IBAction func dumpCIAAAction(_ sender: Any!) {
        amiga.suspend()
        amiga.ciaA.dump()
        amiga.resume()
    }
    @IBAction func dumpCIABAction(_ sender: Any!) {
        amiga.suspend()
        amiga.ciaB.dump()
        amiga.resume()
    }
    @IBAction func dumpMemoryAction(_ sender: Any!) {
        amiga.suspend()
        amiga.mem.dump()
        amiga.resume()
    }
    @IBAction func dumpDiskControllerAction(_ sender: Any!) {
        amiga.suspend()
        amiga.diskController.dump()
        amiga.resume()
    }
    @IBAction func dumpDfxAction(_ sender: NSMenuItem!) {
        amiga.suspend()
        amiga.df(sender)!.dump()
        amiga.resume()
    }
    @IBAction func dumpDf0Action(_ sender: Any!) {
        amiga.suspend()
        amiga.df0.dump()
        amiga.resume()
    }
    @IBAction func dumpDf1Action(_ sender: Any!) {
        amiga.suspend()
        amiga.df1.dump()
        amiga.resume()
    }
    @IBAction func dumpAgnusAction(_ sender: Any!) {
        amiga.suspend()
        amiga.agnus.dump()
        amiga.resume()
    }
    @IBAction func dumpCopperAction(_ sender: Any!) {
        amiga.suspend()
        amiga.copper.dump()
        amiga.resume()
    }
    @IBAction func dumpBlitterAction(_ sender: Any!) {
        amiga.suspend()
        amiga.blitter.dump()
        amiga.resume()
    }
    @IBAction func dumpPaulaAction(_ sender: Any!) {
        amiga.suspend()
        amiga.paula.dump()
        amiga.resume()
    }
    @IBAction func dumpDeniseAction(_ sender: Any!) {
        amiga.suspend()
        amiga.denise.dump()
        amiga.resume()
    }
    @IBAction func dumpKeyboardAction(_ sender: Any!) {
        amiga.suspend()
        amiga.keyboard.dump()
        amiga.resume()
    }
    @IBAction func dumpControlPort1Action(_ sender: Any!) {
        amiga.suspend()
        amiga.controlPort1.dump()
        amiga.resume()
    }
    @IBAction func dumpControlPort2Action(_ sender: Any!) {
        amiga.suspend()
        amiga.controlPort2.dump()
        amiga.resume()
    }
    @IBAction func dumpSerialPortAction(_ sender: Any!) {
        amiga.suspend()
        amiga.serialPort.dump()
        amiga.resume()
    }
}
