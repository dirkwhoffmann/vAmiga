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
        
        var dfn: FloppyDriveProxy { return amiga.df(item.tag)! }
        var hdn: HardDriveProxy { return amiga.hd(item.tag)! }

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
            return validateURLlist(myAppDelegate.insertedFloppyDisks, image: smallDisk)
            
        case  #selector(MyController.ejectDiskAction(_:)),
            #selector(MyController.exportFloppyDiskAction(_:)),
            #selector(MyController.inspectFloppyDiskAction(_:)),
            #selector(MyController.inspectDfnVolumeAction(_:)):
            return dfn.hasDisk
                        
        case #selector(MyController.exportRecentDiskDummyAction(_:)):
            return amiga.df(item)!.hasDisk
                        
        case #selector(MyController.exportRecentDiskAction(_:)):
            return validateURLlist(myAppDelegate.exportedFloppyDisks[item.tag],
                                   image: smallDisk)
            
        case #selector(MyController.writeProtectAction(_:)):
            item.state = dfn.hasProtectedDisk ? .on : .off
            return dfn.hasDisk
            
        // Hd<n> menu
        case #selector(MyController.attachRecentHdrAction(_:)):
            return validateURLlist(myAppDelegate.attachedHardDrives, image: smallHdr)

        case #selector(MyController.exportRecentHdDummyAction(_:)):
            return amiga.hd(item)!.hasDisk

        case #selector(MyController.exportRecentHdrAction(_:)):
            return validateURLlist(myAppDelegate.exportedFloppyDisks[item.tag],
                                   image: smallHdr)

        case #selector(MyController.writeProtectHdrAction(_:)):
            item.state = hdn.hasProtectedDisk ? .on : .off
            return hdn.hasDisk

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
        
        log()
        
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
            configurator = ConfigurationController.make(parent: self, nibName: "Configuration")
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
            snapshotBrowser = SnapshotDialog.make(parent: self, nibName: "SnapshotDialog")
        }
        snapshotBrowser?.showSheet()
    }
    
    @IBAction func takeScreenshotAction(_ sender: Any!) {
                
        // Determine screenshot format
        let format = ScreenshotSource(rawValue: pref.screenshotSource)!
        
        // Take screenshot
        guard let screen = renderer.canvas.screenshot(source: format) else {
            
            log(warning: "Failed to create screenshot")
            return
        }

        // Convert to Screenshot object
        let screenshot = Screenshot(screen: screen, format: pref.screenshotTarget)

        // Save to disk
        try? screenshot.save()
    }
    
    @IBAction func browseScreenshotsAction(_ sender: Any!) {
        
        if screenshotBrowser == nil {
            screenshotBrowser = ScreenshotDialog.make(parent: self, nibName: "ScreenshotDialog")
        }
        screenshotBrowser?.checksum = amiga.df0.fnv
        screenshotBrowser?.showSheet()
    }
    
    @IBAction func captureScreenAction(_ sender: Any!) {
                
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
        
        do {
            try amiga.recorder.startRecording(rect: rect,
                                              rate: pref.bitRate,
                                              ax: pref.aspectX,
                                              ay: pref.aspectY)
        } catch {
            
            (error as? VAError)?.cantRecord()            
        }
    }
    
    @IBAction func exportVideoAction(_ sender: Any!) {
                
        let exporter = VideoExporter.make(parent: self, nibName: "VideoExporter")
        exporter?.showSheet()
    }
    
    //
    // Action methods (Edit menu)
    //
    
    @IBAction func paste(_ sender: Any!) {
                
        let pasteBoard = NSPasteboard.general
        guard let text = pasteBoard.string(forType: .string) else {
            
            log(warning: "Cannot paste. No text in pasteboard")
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
            log("Virtual keyboard already open")
        } else {
            log("Opeining virtual keyboard as a window")
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
        
        type(keyCode: AmigaKeycode.delete)
    }

    @IBAction func helpKeyAction(_ sender: Any!) {
        
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
    // Action methods (Floppy disk menus)
    //
    
    @IBAction func newDiskAction(_ sender: NSMenuItem!) {

        let drive = amiga.df(sender.tag)!
        
        // Ask the user if a modified hard drive should be detached
        if !proceedWithUnexportedDisk(drive: drive) { return }

        let panel = FloppyCreator.make(parent: self, nibName: "FloppyCreator")
        panel?.showSheet(forDrive: sender.tag)
    }

    @IBAction func insertDiskAction(_ sender: NSMenuItem!) {
       
        let drive = amiga.df(sender.tag)!
        
        // Ask the user if an unsafed disk should be replaced
        if !proceedWithUnexportedDisk(drive: drive) { return }
        
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
        
        log("insertDiskAction \(url) drive \(drive)")
        
        let drive = amiga.df(drive)!
        let types: [FileType] = [ .ADF, .EXT, .DMS, .EXE, .DIR ]
        
        do {
            // Try to create a file proxy
            try mydocument.createAttachment(from: url, allowedTypes: types)

            // Ask the user if an unsafed disk should be replaced
            if !proceedWithUnexportedDisk(drive: drive) { return }
            
            if let file = mydocument.attachment as? FloppyFileProxy {
                
                do {
                    
                    // Insert the disk
                    try drive.swap(file: file)

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
    
    @IBAction func exportRecentDiskDummyAction(_ sender: NSMenuItem!) {}
    @IBAction func exportRecentDiskAction(_ sender: NSMenuItem!) {
                
        let n = sender.tag / 10
        let slot = sender.tag % 10
                
        exportRecentAction(df: n, slot: slot)
    }
    
    func exportRecentAction(df n: Int, slot: Int) {
        
        log("df\(n) slot: \(slot)")
        
        if let url = myAppDelegate.getRecentlyExportedDiskURL(slot, df: n) {
            
            do {
                try mydocument.export(drive: n, to: url)
                
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
        
        myAppDelegate.clearRecentlyExportedDiskURLs(df: sender.tag)
    }
    
    @IBAction func ejectDiskAction(_ sender: NSMenuItem!) {
        
        let drive = amiga.df(sender.tag)!
        
        if proceedWithUnexportedDisk(drive: drive) {
            
            drive.eject()
            myAppDelegate.clearRecentlyExportedDiskURLs(df: drive.nr)
        }
    }
    
    @IBAction func exportFloppyDiskAction(_ sender: NSMenuItem!) {
        
        let exportPanel = DiskExporter.make(parent: self, nibName: "DiskExporter")
        exportPanel?.showSheet(diskDrive: sender.tag)
    }
    
    @IBAction func inspectFloppyDiskAction(_ sender: NSMenuItem!) {
        
        let panel = DiskInspector.make(parent: self, nibName: "DiskInspector")
        panel?.show(diskDrive: sender.tag)
    }

    @IBAction func inspectDfnVolumeAction(_ sender: NSMenuItem!) {
                
        let panel = VolumeInspector.make(parent: self, nibName: "VolumeInspector")
        panel?.show(diskDrive: sender.tag)
    }

    //
    // Action methods (Hard drive menus)
    //
    
    @IBAction func newHdrAction(_ sender: NSMenuItem!) {

        let drive = amiga.hd(sender.tag)!
        
        // Power off the emulator if the user doesn't object
        if !askToPowerOff() { return }
        
        let panel = HardDiskCreator.make(parent: self, nibName: "HardDiskCreator")
        panel?.show(forDrive: drive.nr)
    }
    
    @IBAction func attachHdrAction(_ sender: NSMenuItem!) {
        
        let drive = amiga.hd(sender.tag)!
        
        // Power off the emulator if the user doesn't object
        if !askToPowerOff() { return }

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
                self.attachHdrAction(from: url, drive: drive.nr)
            }
        })
    }
    
    @IBAction func attachRecentHdrDummyAction(_ sender: NSMenuItem!) {}

    @IBAction func attachRecentHdrAction(_ sender: NSMenuItem!) {
        
        let drive = amiga.hd(sender.tag / 10)!
        let slot  = sender.tag % 10
                    
        // Power off the emulator if the user doesn't object
        if !askToPowerOff() { return }

        if let url = myAppDelegate.getRecentlyAttachedHdrURL(slot) {
            attachHdrAction(from: url, drive: drive.nr)
        }
    }
    
    private func attachHdrAction(from url: URL, drive nr: Int) {
                
        let types: [FileType] = [ .HDF ]
        let drive = amiga.hd(nr)!
        
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
    
    @IBAction func exportRecentHdDummyAction(_ sender: NSMenuItem!) {}
    @IBAction func exportRecentHdrAction(_ sender: NSMenuItem!) {
                
        let n = sender.tag / 10
        let slot = sender.tag % 10
                
        exportRecentAction(hd: n, slot: slot)
    }

    func exportRecentAction(hd n: Int, slot: Int) {
        
        log("hd\(n) slot: \(slot)")

        if let url = myAppDelegate.getRecentlyExportedHdrURL(slot, hd: n) {
            
             do {
                 try mydocument.export(hardDrive: n, to: url)
                 
             } catch let error as VAError {
                 error.warning("Cannot export hard drive to file \"\(url.path)\"")
             } catch {
                 fatalError()
             }
        }
    }
    
    @IBAction func clearRecentlyAttachedHdrsAction(_ sender: NSMenuItem!) {
        
        myAppDelegate.clearRecentlyAttachedHdrURLs()
    }
    
    @IBAction func clearRecentlyExportedHdrsAction(_ sender: NSMenuItem!) {
        
        myAppDelegate.clearRecentlyExportedHdrURLs(hd: sender.tag)
    }
    
    @IBAction func exportHdrAction(_ sender: NSMenuItem!) {
        
        let exportPanel = DiskExporter.make(parent: self, nibName: "DiskExporter")
        exportPanel?.showSheet(hardDrive: sender.tag)
    }
    
    @IBAction func inspectHdrDiskAction(_ sender: NSMenuItem!) {

        let panel = DiskInspector.make(parent: self, nibName: "DiskInspector")
        panel?.show(hardDrive: sender.tag)
    }

    @IBAction func inspectHdrVolumeAction(_ sender: NSMenuItem!) {
        
        let panel = VolumeInspector.make(parent: self, nibName: "VolumeInspector")
        panel?.show(hardDrive: sender.tag)
    }

    @IBAction func configureHdrAction(_ sender: NSMenuItem!) {
        
        let panel = HardDiskConfigurator.make(parent: self, nibName: "HardDiskConfigurator")
        panel?.show(forDrive: sender.tag)
    }
    
    @IBAction func writeProtectHdrAction(_ sender: NSMenuItem!) {
        
        amiga.hd(sender)!.toggleWriteProtection()
    }

}
