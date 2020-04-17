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

        var dfn: DriveProxy {
            
            switch item.tag {
            case 0: return amiga.df0
            case 1: return amiga.df1
            case 2: return amiga.df2
            case 3: return amiga.df3
            default: fatalError()
            }
        }
        
        func validateURLlist(_ list: [URL], image: NSImage) -> Bool {
            
            if let pos = Int(item.title) {
                
                if let url = myAppDelegate.getRecentlyUsedURL(pos, from: list) {
                    item.title = url.lastPathComponent
                    item.isHidden = false
                    item.image = image
                } else {
                    item.isHidden = true
                    item.image = nil
                }
            }
            return true
        }
        
        switch item.action {
            
            //
            // Edit menu
            //
            
        case #selector(MyController.stopAndGoAction(_:)):
            item.title = amiga.isRunning() ? "Pause" : "Continue"
            return true
            
        case #selector(MyController.powerAction(_:)):
            item.title = amiga.isPoweredOn() ? "Power Off" : "Power On"
            return true
            
            //
            // View menu
            //
            
        case #selector(MyController.toggleStatusBarAction(_:)):
            item.title = statusBar ? "Hide Status Bar" : "Show Status Bar"
            return true

            //
            // Keyboard menu
            //
            
        case #selector(MyController.mapCmdKeysAction(_:)):
            item.state = (eventTap != nil) ? .on : .off
            return true

            //
            // Drive menu
            //
            
        case #selector(MyController.insertRecentDiskAction(_:)):
            
            return validateURLlist(myAppDelegate.recentlyInsertedDiskURLs, image: smallDisk)

        case  #selector(MyController.ejectDiskAction(_:)),
              #selector(MyController.exportDiskAction(_:)):
            
            return dfn.hasDisk()

        case #selector(MyController.exportRecentDiskDummyAction0(_:)):
            return amiga.df0.hasDisk()

        case #selector(MyController.exportRecentDiskDummyAction1(_:)):
            return amiga.df1.hasDisk()

        case #selector(MyController.exportRecentDiskDummyAction2(_:)):
            return amiga.df2.hasDisk()

        case #selector(MyController.exportRecentDiskDummyAction3(_:)):
            return amiga.df3.hasDisk()

        case #selector(MyController.exportRecentDiskAction(_:)):
            switch item.tag {
            case 0: return validateURLlist(myAppDelegate.recentlyExportedDisk0URLs, image: smallDisk)
            case 1: return validateURLlist(myAppDelegate.recentlyExportedDisk1URLs, image: smallDisk)
            case 2: return validateURLlist(myAppDelegate.recentlyExportedDisk2URLs, image: smallDisk)
            case 3: return validateURLlist(myAppDelegate.recentlyExportedDisk3URLs, image: smallDisk)
            default: fatalError()
            }
            
        case #selector(MyController.writeProtectAction(_:)):
            item.state = dfn.hasWriteProtectedDisk() ? .on : .off
            return dfn.hasDisk()

        case #selector(MyController.dragAndDropTargetAction(_:)):
            item.state = dfn === dragAndDropDrive ? .on : .off
            return true
            
            //
            // Debug menu
            //
            
        case #selector(MyController.stepIntoAction(_:)),
             #selector(MyController.stepOverAction(_:)),
             #selector(MyController.stopAndGoAction(_:)):
            return amiga.isPaused()

        case #selector(MyController.dumpStateAction(_:)):
            return !amiga.releaseBuild()
            
        default:
            return true
        }
    }

    //
    // Action methods (App menu)
    //

    @IBAction func preferencesAction(_ sender: Any!) {
        
        if myAppDelegate.prefsController == nil {
            let name = NSNib.Name("Preferences")
            myAppDelegate.prefsController = PreferencesController.make(parent: self,
                                                                       nibName: name)
        }
        myAppDelegate.prefsController?.showSheet()
    }
    
    func importPrefs(prefixes: [String]) {
        
        track("Importing user defaults with prefixes \(prefixes)")
        
        let panel = NSOpenPanel()
        panel.prompt = "Import"
        panel.allowedFileTypes = ["amigacnf"]
        
        panel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = panel.url {
                    self.loadUserDefaults(url: url, prefixes: prefixes)
                }
            }
        })
    }
    
    func exportPrefs(prefixes: [String]) {
        
        track("Exporting user defaults with prefixes \(prefixes)")
        
        let panel = NSSavePanel()
        panel.prompt = "Export"
        panel.allowedFileTypes = ["amigacnf"]
        
        panel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = panel.url {
                    track()
                    self.saveUserDefaults(url: url, prefixes: prefixes)
                }
            }
        })
    }
    
    @IBAction func importAppPrefsAction(_ sender: Any!) {
        
        importPrefs(prefixes: ["VAMIGA1", "VAMIGA2"])
    }

    @IBAction func exportAppPrefsAction(_ sender: Any!) {
        
        exportPrefs(prefixes: ["VAMIGA1", "VAMIGA2"])
    }
    
    @IBAction func importEmuPrefsAction(_ sender: Any!) {
        
        importPrefs(prefixes: ["VAMIGA3", "VAMIGA4", "VAMIGA5", "VAMIGA6"])
    }
    
    @IBAction func exportEmuPrefsAction(_ sender: Any!) {
        
        exportPrefs(prefixes: ["VAMIGA3", "VAMIGA4", "VAMIGA5", "VAMIGA6"])
    }
    
    @IBAction func factorySettingsAction(_ sender: Any!) {
        
        track()
        resetUserDefaults()
    }
    
    //
    // Action methods (File menu)
    //
    
    func openConfigurator(tab: String = "") {
        
        if configurator == nil {
            let name = NSNib.Name("Configuration")
            configurator = ConfigController.make(parent: self, nibName: name)
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
        screenshotBrowser?.checksum = amiga.df0.fnv()
        screenshotBrowser?.showSheet()
    }
    
    /*
    // DEPRECATED
    @IBAction func saveScreenshotDialog(_ sender: Any!) {
        
        // Halt emulation to freeze the current texture
        amiga.pause()
        
        // Create save panel
        let savePanel = NSSavePanel()
        savePanel.prompt = "Export"
        
        // Set allowed file types
        switch screenshotTarget {
        case .tiff: savePanel.allowedFileTypes = ["jpg"]
        case .bmp: savePanel.allowedFileTypes = ["bmp"]
        case .gif: savePanel.allowedFileTypes = ["gif"]
        case .jpeg: savePanel.allowedFileTypes = ["jpg"]
        case .png: savePanel.allowedFileTypes = ["png"]
        default:
            track("Unsupported image format: \(screenshotTarget)")
            return
        }
        
        // Run panel as sheet
        savePanel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = savePanel.url {
                    do {
                        try self.saveScreenshot(url: url)
                    } catch {
                        NSApp.presentError(error)
                    }
                }
            }
            self.amiga.run()
        })
    }

    // DEPRECATED
    @IBAction func quicksaveScreenshot(_ sender: Any!) {
        
        // Determine file suffix
        var suffix: String
        switch screenshotTarget {
        case .tiff: suffix = "tiff"
        case .bmp: suffix = "bmp"
        case .gif: suffix = "gif"
        case .jpeg: suffix = "jpg"
        case .png: suffix = "png"
        default:
            track("Unsupported image format: \(screenshotTarget)")
            return
        }
        
        // Assemble URL and save
        let paths = NSSearchPathForDirectoriesInDomains(.desktopDirectory, .userDomainMask, true)
        let desktopUrl = NSURL.init(fileURLWithPath: paths[0])
        if let url = desktopUrl.appendingPathComponent("Screenshot." + suffix) {
            do {
                try saveScreenshot(url: url.addTimeStamp().makeUnique())
            } catch {
                track("Cannot quicksave screenshot")
            }
        }
    }
    
    // DEPRECATED
    func saveScreenshot(url: URL) throws {
        
        // Take screenshot
        let image = renderer.screenshot(afterUpscaling: screenshotSource > 0)
        
        // Convert to target format
        let data = image?.representation(using: screenshotTarget)
        
        // Save to file
        try data?.write(to: url, options: .atomic)
    }
    */
    
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
        
        kbController.autoType(text)
    }

    @IBAction func stopAndGoAction(_ sender: NSButton!) {

        amiga?.stopAndGo()
    }
    
    @IBAction func stepIntoAction(_ sender: NSButton!) {

        amiga?.stepInto()
    }
    
    @IBAction func stepOverAction(_ sender: NSButton!) {

        amiga?.stepOver()
    }

    @IBAction func resetAction(_ sender: Any!) {

        track()
        renderer.blendIn(steps: 20)
        amiga.reset()
    }
    
    @IBAction func powerAction(_ sender: Any!) {

        if amiga.isPoweredOn() {
            amiga.powerOff()
            return
        }

        switch amiga.readyToPowerOn() {

        case ERR_OK:
            amiga.run()

        default:
            mydocument?.showConfigurationAltert(amiga.readyToPowerOn())
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
        
        // Open the virtual keyboard as a window
        if virtualKeyboardSheet == nil {
            virtualKeyboardSheet = VKBController.make(parent: self)
        }
        virtualKeyboardSheet?.showWindow(autoClose: false)
    }

    @IBAction func mapCmdKeysAction(_ sender: Any!) {
        
        mapCommandKeys = !mapCommandKeys
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
        
        let drive = amiga.df(sender)
        let model = drive.type()
        
        var adf: ADFFileProxy
        
        // Create a blank disk
        switch model {
        case DRIVE_35_DD:  adf = ADFFileProxy.make(with: DISK_35_DD)
        case DRIVE_525_SD: adf = ADFFileProxy.make(with: DISK_525_SD)
        default: fatalError()
        }
        
        // Write file system
        adf.formatDisk(prefs.driveBlankDiskFormat)
        
        // Insert disk into drive
        amiga.diskController.insert(sender.tag, adf: adf)
        // drive.insertDisk(adf)
        
        myAppDelegate.clearRecentlyExportedDiskURLs(drive: sender.tag)
    }

    @IBAction func insertDiskAction(_ sender: NSMenuItem!) {
        
        // Ask user to continue if the current disk contains modified data
        if !proceedWithUnexportedDisk(drive: sender.tag) {
            return
        }
        
        // Show the OpenPanel
        let openPanel = NSOpenPanel()
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        openPanel.canCreateDirectories = false
        openPanel.canChooseFiles = true
        openPanel.prompt = "Insert"
        openPanel.allowedFileTypes = ["adf"]
        openPanel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = openPanel.url {
                    do {
                        let adf = try self.mydocument?.createADF(from: url)
                        // self.amiga.df(sender).insertDisk(adf)
                        self.amiga.diskController.insert(sender.tag, adf: adf)
                    } catch {
                        NSApp.presentError(error)
                    }
                }
            }
        })
    }
    
    @IBAction func insertRecentDiskAction(_ sender: NSMenuItem!) {
        
        let drive = sender.tag / 10
        let slot  = sender.tag % 10
        
        track("\(drive) \(slot)")
        
        // Get URL and insert
        if let url = myAppDelegate.getRecentlyInsertedDiskURL(slot) {
            do {
                let adf = try self.mydocument?.createADF(from: url)
                if proceedWithUnexportedDisk(drive: drive) {
                    // amiga.df(drive).insertDisk(adf)
                    amiga.diskController.insert(sender.tag, adf: adf)
                }
            } catch {
                NSApp.presentError(error)
            }
        }
    }
    
    func insertRecentDiskAction(drive: DriveProxy, slot: Int) {
        
        if let url = myAppDelegate.getRecentlyInsertedDiskURL(slot) {
            do {
                let adf = try self.mydocument?.createADF(from: url)
                if proceedWithUnexportedDisk(drive: drive) {
                    // drive.insertDisk(adf)
                    amiga.diskController.insert(drive.nr(), adf: adf)
                }
            } catch {
                NSApp.presentError(error)
            }
        }
    }
    
    @IBAction func writeProtectAction(_ sender: NSMenuItem!) {
        
        let drive = sender.tag == 0 ? amiga.df0! : amiga.df1!
        drive.toggleWriteProtection()
    }

    @IBAction func exportRecentDiskDummyAction0(_ sender: NSMenuItem!) {}
    @IBAction func exportRecentDiskDummyAction1(_ sender: NSMenuItem!) {}
    @IBAction func exportRecentDiskDummyAction2(_ sender: NSMenuItem!) {}
    @IBAction func exportRecentDiskDummyAction3(_ sender: NSMenuItem!) {}
    @IBAction func exportRecentDiskAction(_ sender: NSMenuItem!) {
        
        track()
        
        let nr = sender.tag
        let slot = Int(sender.title)!
        
        // Get URL and export
        if let url = myAppDelegate.getRecentlyExportedDiskURL(slot, drive: nr) {
            do {
                mydocument?.export(drive: nr, to: url)
            }
        }
    }
    
    @IBAction func clearRecentlyInsertedDisksAction(_ sender: NSMenuItem!) {

        myAppDelegate.recentlyInsertedDiskURLs = []
    }

    @IBAction func clearRecentlyExportedDisksAction(_ sender: NSMenuItem!) {

        myAppDelegate.clearRecentlyExportedDiskURLs(drive: sender.tag)
    }

    @IBAction func ejectDiskAction(_ sender: NSMenuItem!) {
        
        if proceedWithUnexportedDisk(drive: sender.tag) {
            // amiga.df(sender.tag).ejectDisk()
            amiga.diskController.eject(sender.tag)
            myAppDelegate.clearRecentlyExportedDiskURLs(drive: sender.tag)
        }
    }
    
    @IBAction func exportDiskAction(_ sender: NSMenuItem!) {
        
        let nibName = NSNib.Name("ExportDiskDialog")
        let exportPanel = ExportDiskDialog.init(windowNibName: nibName)
        exportPanel.showSheet(forDrive: sender.tag)
    }
    
    @IBAction func dragAndDropTargetAction(_ sender: NSMenuItem!) {
        
        let drive = amiga.df(sender)
        dragAndDropDrive = (dragAndDropDrive == drive) ? nil : drive
    }
    
    //
    // Action methods (Debug menu)
    //

    @IBAction func emulateSpritesAction(_ sender: NSMenuItem!) {

        amiga.suspend()
        amiga.configure(VA_EMULATE_SPRITES, enable: sender.state == .off)
        amiga.resume()
        sender.state = (sender.state == .off) ? .on : .off
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
    @IBAction func dumpEventsAction(_ sender: Any!) {
        amiga.suspend()
        amiga.agnus.dumpEvents()
        amiga.resume()
        track()
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
        amiga.df(sender).dump()
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
        amiga.agnus.dumpCopper()
        amiga.resume()
    }
    @IBAction func dumpBlitterAction(_ sender: Any!) {
        amiga.suspend()
        amiga.agnus.dumpBlitter()
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
