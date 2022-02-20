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
        
        let powered = amiga.poweredOn
        let running = amiga.running
        let paused = amiga.paused
        let recording = amiga.recorder.recording
        
        var dfn: DriveProxy { return amiga.df(item.tag)! }
        var dhn: HardDriveProxy { return amiga.dh(item.tag)! }

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
        case #selector(MyController.resetConfigAction(_:)):
            return !powered

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
        case #selector(MyController.mapCmdKeysAction(_:)):
            item.state = (myAppDelegate.eventTap != nil) ? .on : .off
            return true
            
        // Df<n> menu
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
            
        // Dh<n> menu
        case #selector(MyController.newHdrAction(_:)):
            return amiga.poweredOff

        case #selector(MyController.attachHdrAction(_:)):
            return amiga.poweredOff
            
        case #selector(MyController.attachRecentHdrDummyAction(_:)):
            return dhn.isAttached

        case #selector(MyController.attachRecentHdrAction(_:)):
            return validateURLlist(myAppDelegate.recentlyAttachedHdrURLs, image: smallHdr)
            
        case #selector(MyController.exportHdrAction(_:)):
            return dhn.isAttached
            
        case #selector(MyController.exportRecentHdrDummyAction(_:)):
            return dhn.isAttached

        case #selector(MyController.exportRecentHdrAction(_:)):
            return validateURLlist(myAppDelegate.recentlyExportedHdrURLs, image: smallHdr)

        case #selector(MyController.hdrGeometryAction(_:)):
            return dhn.isAttached

        case #selector(MyController.inspectHdrAction(_:)):
            return dhn.isAttached

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
                                           nibName: NSNib.Name("Preferences"))
        }
        myAppDelegate.prefController!.showWindow(self)
        myAppDelegate.prefController!.refresh()
    }
    
    @IBAction func resetConfigAction(_ sender: Any!) {
        
        track()
        
        UserDefaults.resetRomUserDefaults()
        UserDefaults.resetChipsetUserDefaults()
        UserDefaults.resetPeripheralsUserDefaults()
        UserDefaults.resetCompatibilityUserDefaults()
        UserDefaults.resetAudioUserDefaults()
        UserDefaults.resetVideoUserDefaults()
        UserDefaults.resetGeometryUserDefaults()
        
        amiga.suspend()
        config.loadRomUserDefaults()
        config.loadChipsetUserDefaults()
        config.loadPeripheralsUserDefaults()
        config.loadCompatibilityUserDefaults()
        config.loadAudioUserDefaults()
        config.loadVideoUserDefaults()
        config.loadGeometryUserDefaults()
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
 
    @IBAction func consoleAction(_ sender: Any!) {
        
        if renderer.console.isVisible {
            renderer.console.close(delay: 0.25)
        } else {
            renderer.console.open(delay: 0.25)
        }
    }
    
    @IBAction func takeSnapshotAction(_ sender: Any!) {
        
        amiga.requestUserSnapshot()
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
            let name = NSNib.Name("SnapshotDialog")
            snapshotBrowser = SnapshotDialog.make(parent: self, nibName: name)
        }
        snapshotBrowser?.showSheet()
    }
    
    @IBAction func takeScreenshotAction(_ sender: Any!) {
        
        track()
        
        // Determine screenshot format
        let format = ScreenshotSource(rawValue: pref.screenshotSource)!
        
        // Take screenshot
        guard let screen = renderer.canvas.screenshot(source: format) else {
            track("Failed to create screenshot")
            return
        }

        // Convert to Screenshot object
        let screenshot = Screenshot(screen: screen, format: pref.screenshotTarget)

        // Save to disk
        try? screenshot.save(id: mydocument.bootDiskID)
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
        
        track("Recording = \(amiga.recorder.recording)")
        
        if amiga.recorder.recording {
            
            amiga.recorder.stopRecording()
            exportVideoAction(self)
            return
        }
        
        if !amiga.recorder.hasFFmpeg {
            showMissingFFmpegAlert()
            return
        }
        
        var rect: CGRect
        if pref.captureSource == 0 {
            rect = renderer.canvas.textureRectAbs
        } else {
            rect = renderer.canvas.entire
        }
        
        let success = amiga.recorder.startRecording(rect,
                                                    bitRate: pref.bitRate,
                                                    aspectX: pref.aspectX,
                                                    aspectY: pref.aspectY)
        if !success {
            showFailedToLaunchFFmpegAlert()
        }
    }
    
    @IBAction func exportVideoAction(_ sender: Any!) {
        
        track()
        
        let name = NSNib.Name("ExportVideoDialog")
        let exporter = ExportVideoDialog.make(parent: self, nibName: name)
        
        exporter?.showSheet()
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
        
        keyboard.autoTypeAsync(text)
    }
    
    @IBAction func stopAndGoAction(_ sender: Any!) {
        
        amiga?.stopAndGo()
    }
    
    @IBAction func stepIntoAction(_ sender: Any!) {
        
        needsSaving = true
        amiga?.stepInto()
    }
    
    @IBAction func stepOverAction(_ sender: Any!) {
        
        needsSaving = true
        amiga?.stepOver()
    }
    
    @IBAction func resetAction(_ sender: Any!) {
        
        track()
        amiga.hardReset()
        try? amiga.run()
    }
    
    @IBAction func powerAction(_ sender: Any!) {
        
        if amiga.poweredOn {
            
            amiga.powerOff()
            
        } else {
            
            amiga.powerOn()
            
            do {
                try amiga.run()
            } catch let error as VAError {
                error.notReady()
            } catch {
                fatalError()
            }
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
    
    @IBAction func stickyKeyboardAction(_ sender: Any!) {
        
        // Create keyboard if neccessary
        if virtualKeyboard == nil {
            virtualKeyboard = VirtualKeyboardController.make(parent: self)
        }
        if virtualKeyboard?.window?.isVisible == true {
            track("Virtual keyboard already open")
        } else {
            track("Opeining virtual keyboard as a window")
        }
        virtualKeyboard?.showWindow()
    }
     
    @IBAction func mapCmdKeysAction(_ sender: Any!) {
        
        myAppDelegate.mapCommandKeys = !myAppDelegate.mapCommandKeys
        refreshStatusBar()
    }
    
    @IBAction func clearKeyboardMatrixAction(_ sender: Any!) {
        
        amiga.keyboard.releaseAllKeys()
    }
    
    @IBAction func delKeyAction(_ sender: Any!) {
        
        track()
        type(keyCode: AmigaKeycode.delete)
    }

    @IBAction func helpKeyAction(_ sender: Any!) {
        
        track()
        type(keyCode: AmigaKeycode.help)
    }
    
    func type(keyCode: Int, completion: (() -> Void)? = nil) {
        
        DispatchQueue.global().async {
            
            self.amiga.keyboard.pressKey(keyCode)
            usleep(useconds_t(20000))
            self.amiga.keyboard.releaseKey(keyCode)
            completion?()
        }
    }
    
    //
    // Action methods (Disk menu)
    //
    
    @IBAction func newDiskAction(_ sender: NSMenuItem!) {
        
        let drive = amiga.df(sender.tag)
    
        do {
            try drive?.insertNew()
            myAppDelegate.clearRecentlyExportedDiskURLs(drive: sender.tag)
            
        } catch {
            (error as? VAError)?.cantInsert()
        }
    }

    @IBAction func insertDiskAction(_ sender: NSMenuItem!) {
        
        // Ask the user if an unsafed disk should be replaced
        if !proceedWithUnexportedDisk(drive: sender.tag) { return }
        
        // Show the OpenPanel
        let openPanel = NSOpenPanel()
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = true
        openPanel.canCreateDirectories = false
        openPanel.canChooseFiles = true
        openPanel.prompt = "Insert"
        openPanel.allowedFileTypes = ["adf", "img", "ima", "dms", "exe", "adz", "zip", "gz"]
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
        
        track("insertDiskAction \(url) drive \(drive)")
        
        let types: [FileType] = [ .ADF, .EXT, .DMS, .EXE, .DIR ]
        
        do {
            // Try to create a file proxy
            try mydocument.createAttachment(from: url, allowedTypes: types)

            // Ask the user if an unsafed disk should be replaced
            if !proceedWithUnexportedDisk(drive: drive) { return }
            
            if let file = mydocument.attachment as? DiskFileProxy {
                
                do {
                    
                    // Insert the disk
                    try amiga.df(drive)!.swap(file: file)

                    // Remember the URL
                    myAppDelegate.noteNewRecentlyInsertedDiskURL(url)
                
                } catch {
                    
                    (error as? VAError)?.cantInsert()
                }
            }
            
        } catch {
            
            (error as? VAError)?.cantOpen(url: url)
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
    
    @IBAction func exportRecentDiskAction(_ sender: NSMenuItem!) {
        
        track()
        
        let drive = sender.tag / 10
        let slot  = sender.tag % 10
                
        exportRecentDiskAction(drive: drive, slot: slot)
    }
    
    func exportRecentDiskAction(drive nr: Int, slot: Int) {
        
        track("drive: \(nr) slot: \(slot)")
        
        if let url = myAppDelegate.getRecentlyExportedDiskURL(slot, drive: nr) {
            
            do {
                try mydocument.export(drive: nr, to: url)
                
            } catch let error as VAError {
                error.warning("Cannot export disk to file \"\(url.path)\"")
            } catch {
                fatalError()
            }
        }
    }

    @IBAction func clearRecentlyInsertedDisksAction(_ sender: NSMenuItem!) {
        
        myAppDelegate.clearRecentlyInsertedDiskURLs()
    }
    
    @IBAction func clearRecentlyExportedDisksAction(_ sender: NSMenuItem!) {
        
        myAppDelegate.clearRecentlyExportedDiskURLs(drive: sender.tag)
    }
    
    @IBAction func ejectDiskAction(_ sender: NSMenuItem!) {
        
        if proceedWithUnexportedDisk(drive: sender.tag) {
            
            amiga.df(sender.tag)?.eject()
            myAppDelegate.clearRecentlyExportedDiskURLs(drive: sender.tag)
        }
    }
    
    @IBAction func exportDiskAction(_ sender: NSMenuItem!) {
        
        let nibName = NSNib.Name("ExporterDialog")
        let exportPanel = ExporterDialog.make(parent: self, nibName: nibName)
        exportPanel?.showSheet(forDrive: sender.tag)
    }
    
    //
    // Action methods (Hard drive menu)
    //
    
    @IBAction func newHdrAction(_ sender: NSMenuItem!) {

        // Ask the user if a modified hard drive should be detached
        if !proceedWithUnexportedHdr(drive: sender.tag) { return }

        let nibName = NSNib.Name("HdrCreatorDialog")
        let panel = HdrCreatorDialog.make(parent: self, nibName: nibName)
        panel?.showSheet(forDrive: sender.tag)
    }
    
    @IBAction func attachHdrAction(_ sender: NSMenuItem!) {
        
        // Ask the user if a modified hard drive should be detached
        if !proceedWithUnexportedHdr(drive: sender.tag) { return }
        
        // Show the OpenPanel
        let openPanel = NSOpenPanel()
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = true
        openPanel.canCreateDirectories = false
        openPanel.canChooseFiles = true
        openPanel.prompt = "Attach"
        openPanel.allowedFileTypes = ["hdf", "hdz", "zip", "gz"]
        openPanel.beginSheetModal(for: window!, completionHandler: { result in
            
            if result == .OK, let url = openPanel.url {
                self.attachHdrAction(from: url, drive: sender.tag)
            }
        })
    }
    
    @IBAction func attachRecentHdrDummyAction(_ sender: NSMenuItem!) {}

    @IBAction func attachRecentHdrAction(_ sender: NSMenuItem!) {
        
        let drive = sender.tag / 10
        let slot  = sender.tag % 10
                
        // Ask the user if an unsafed disk should be discarded
        if !proceedWithUnexportedHdr(drive: drive) { return }

        if let url = myAppDelegate.getRecentlyAttachedHdrURL(slot) {
            attachHdrAction(from: url, drive: drive)
        }
    }
    
    private func attachHdrAction(from url: URL, drive nr: Int) {
        
        track("attachHdrAction \(url)")
        
        let types: [FileType] = [ .HDF ]
        let drive = amiga.dh(nr)!
        
        do {
            // Try to create a file proxy
            try mydocument.createAttachment(from: url, allowedTypes: types)
                        
            if let file = mydocument.attachment as? HDFFileProxy {
                
                do {
                    
                    // Attach the drive
                    try drive.attach(hdf: file)
                    
                    // Remember the URL
                    myAppDelegate.noteNewRecentlyAttachedHdrURL(url)
                    
                } catch {
                    
                    (error as? VAError)?.cantAttach()
                }
            }
            
        } catch {
            
            (error as? VAError)?.cantOpen(url: url)
        }
    }
    
    @IBAction func exportRecentHdrDummyAction(_ sender: NSMenuItem!) {}
    @IBAction func exportRecentHdrAction(_ sender: NSMenuItem!) {
        
        track("slot: \(sender.tag)")
        
        if let url = myAppDelegate.getRecentlyExportedHdrURL(sender.tag) {
            
            track("TODO")
            /*
             do {
             try mydocument.exportHdr(to: url)
             
             } catch let error as VAError {
             error.warning("Cannot export hard drive to file \"\(url.path)\"")
             } catch {
             fatalError()
             }
             */
        }
    }
    
    @IBAction func clearRecentlyAttachedHdrsAction(_ sender: NSMenuItem!) {
        
        myAppDelegate.clearRecentlyAttachedHdrURLs()
    }
    
    @IBAction func clearRecentlyExportedHdrsAction(_ sender: NSMenuItem!) {
        
        myAppDelegate.clearRecentlyExportedHdrURLs()
    }
    
    @IBAction func exportHdrAction(_ sender: NSMenuItem!) {
        
        track("TODO")
        /*
         let nibName = NSNib.Name("ExporterDialog")
         let exportPanel = ExporterDialog.make(parent: self, nibName: nibName)
         exportPanel?.showSheet(forDrive: sender.tag)
         */
    }

    @IBAction func hdrGeometryAction(_ sender: NSMenuItem!) {
        
        let nibName = NSNib.Name("HdrGeometryDialog")
        let panel = HdrGeometryDialog.make(parent: self, nibName: nibName)
        panel?.showSheet(forDrive: sender.tag)
    }
    
    @IBAction func inspectHdrAction(_ sender: NSMenuItem!) {
        
        track("TODO")
    }
}
