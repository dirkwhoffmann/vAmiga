//
// This file is part of VirtualC64 - A cycle accurate Commodore 64 emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
//

import Foundation

extension MyController : NSMenuItemValidation {
    
    open func validateMenuItem(_ item: NSMenuItem) -> Bool {

        // track("validateMenuItem")
        
        func firstDrive() -> Bool {
            precondition(item.tag == 1 || item.tag == 2)
            return item.tag == 1
        }
    
        func validateURLlist(_ list : [URL], image: String) -> Bool {
            
            let pos = (item.tag < 10) ? item.tag : item.tag - 10
            
            if let url = mydocument.getRecentlyUsedURL(pos, from: list) {
                item.title = url.lastPathComponent
                item.isHidden = false
                item.image = NSImage.init(named: image)
            } else {
                item.isHidden = true
                item.image = nil
            }
            return true
        }
        
        // Edit menu
        if item.action == #selector(MyController.runOrHaltAction(_:)) {
            // item.title = amiga.isRunning() ? "Pause" : "Continue"
            return true
        }
        if item.action == #selector(MyController.powerOnOrOffAction(_:)) {
            // item.title = amiga.isPoweredOn() ? "Power Off" : "Power On"
            return true
        }

        
        // View menu
        if item.action == #selector(MyController.toggleStatusBarAction(_:)) {
            item.title = statusBar ? "Hide Status Bar" : "Show Status Bar"
            return true
        }
        
        if item.action == #selector(MyController.hideMouseAction(_:)) {
            item.title = hideMouse ? "Show Mouse Cursor" : "Hide Mouse Cursor"
            return true
        }
        
        // Keyboard menu
        if item.action == #selector(MyController.mapCmdKeysAction(_:)) {
            item.state = (eventTap != nil) ? .on : .off
            return true
        }
        
        // Drive menu
        if item.action == #selector(MyController.insertRecentDiskAction(_:)) {
            return validateURLlist(mydocument.recentlyInsertedDiskURLs, image: "disk_small")
        }
        if item.action == #selector(MyController.ejectDiskAction(_:)) {
            return firstDrive() ? c64.drive1.hasDisk() : c64.drive2.hasDisk()
        }
        if item.action == #selector(MyController.exportDiskAction(_:)) {
            return firstDrive() ? c64.drive1.hasDisk() : c64.drive2.hasDisk()
        }
        if item.action == #selector(MyController.exportRecentDiskAction(_:)) {
            if item.tag < 10 {
                track("\(mydocument.recentlyExportedDisk1URLs)")
                return validateURLlist(mydocument.recentlyExportedDisk1URLs, image: "disk_small")
            } else {
                track("\(mydocument.recentlyExportedDisk2URLs)")
                return validateURLlist(mydocument.recentlyExportedDisk2URLs, image: "disk_small")
            }
        }
        if item.action == #selector(MyController.writeProtectAction(_:)) {
            let hasDisk = firstDrive() ?
                c64.drive1.hasDisk() :
                c64.drive2.hasDisk()
            let hasWriteProtecteDisk = firstDrive() ?
                c64.drive1.hasWriteProtectedDisk() :
                c64.drive2.hasWriteProtectedDisk()
            item.state = hasWriteProtecteDisk ? .on : .off
            return hasDisk
        }
        if item.action == #selector(MyController.drivePowerAction(_:)) {
            let poweredOn = firstDrive() ?
                c64.drive1.isPoweredOn() :
                c64.drive2.isPoweredOn()
            item.title = poweredOn ? "Disconnect" : "Connect"
            return true
        }

        // Tape menu
        if item.action == #selector(MyController.insertRecentTapeAction(_:)) {
            return validateURLlist(mydocument.recentlyInsertedTapeURLs, image: "tape_small")
        }
        if item.action == #selector(MyController.ejectTapeAction(_:)) {
            return c64.datasette.hasTape()
        }
        if item.action == #selector(MyController.playOrStopAction(_:)) {
            item.title = c64.datasette.playKey() ? "Press Stop Key" : "Press Play On Tape"
            return c64.datasette.hasTape()
        }
        if item.action == #selector(MyController.rewindAction(_:)) {
            return c64.datasette.hasTape()
        }
        
        // Debug menu
        if item.action == #selector(MyController.pauseAction(_:)) {
            return c64.isRunning();
        }
        if item.action == #selector(MyController.continueAction(_:)) ||
            item.action == #selector(MyController.stepIntoAction(_:)) ||
            item.action == #selector(MyController.stepOverAction(_:)) ||
            item.action == #selector(MyController.stopAndGoAction(_:)) {
            return c64.isHalted();
        }
        if item.action == #selector(MyController.markIRQLinesAction(_:)) {
            item.state = c64.vic.showIrqLines() ? .on : .off
        }
        if item.action == #selector(MyController.markDMALinesAction(_:)) {
            item.state = c64.vic.showDmaLines() ? .on : .off
        }
        if item.action == #selector(MyController.hideSpritesAction(_:)) {
            item.state = c64.vic.hideSprites() ? .on : .off
        }

        if item.action == #selector(MyController.traceAction(_:)) {
            return c64.developmentMode();
        }
        if item.action == #selector(MyController.traceIecAction(_:)) {
            item.state = c64.iec.tracing() ? .on : .off
        }
        if item.action == #selector(MyController.traceVC1541CpuAction(_:)) {
            item.state = c64.drive1.cpu.tracing() ? .on : .off
        }
        if item.action == #selector(MyController.traceViaAction(_:)) {
            item.state = c64.drive1.via1.tracing() ? .on : .off
        }
        
        if item.action == #selector(MyController.dumpStateAction(_:)) {
            return c64.developmentMode();
        }

        return true
    }

    //
    // Action methods (VirtualC64 menu)
    //

    @IBAction func importPrefsAction(_ sender: Any!) {
        
        track()
        
        let panel = NSOpenPanel()
        panel.prompt = "Import"
        panel.allowedFileTypes = ["vc64conf"]
        
        panel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = panel.url {
                    self.loadUserDefaults(url: url)
                }
            }
        })
    }
   
    @IBAction func exportPrefsAction(_ sender: Any!) {
        
        track()
        
        let panel = NSSavePanel()
        panel.prompt = "Export"
        panel.allowedFileTypes = ["vc64conf"]
        
        panel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = panel.url {
                    track()
                    self.saveUserDefaults(url: url)
                }
            }
        })
    }
    
    @IBAction func factorySettingsAction(_ sender: Any!) {
        
        track()
        resetUserDefaults()
    }
    
    //
    // Action methods (File menu)
    //
    
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
    
    func saveScreenshot(url: URL) throws {
        
        // Take screenshot
        let image = metal.screenshot(afterUpscaling: screenshotSource > 0)
        
        // Convert to target format
        let data = image?.representation(using: screenshotTarget)
        
        // Save to file
        try data?.write(to: url, options: .atomic)
    }
    
    @IBAction func takeSnapshot(_ sender: Any!) {
        
        c64.takeUserSnapshot()
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
        
        keyboardcontroller.type(string: text, completion: nil)
    }

    @IBAction func runOrHaltAction(_ sender: Any!) {
        
        // amiga.runOrHalt()
        refresh()
    }
    
    @IBAction func resetAction(_ sender: Any!) {
        
        // TODO: amiga.reset()
        c64.powerUp() // REMOVE
        refresh()
    }
 
    @IBAction func powerOnOrOffAction(_ sender: Any!) {
        
        // TODO: amiga.powerOnOrOff()
        refresh()
    }
    
    
    //
    // Action methods (View menu)
    //

    @IBAction func toggleStatusBarAction(_ sender: Any!) {
        
        undoManager?.registerUndo(withTarget: self) {
            targetSelf in targetSelf.toggleStatusBarAction(sender)
        }
        
        showStatusBar(!statusBar)
    }
    
    @IBAction func hideMouseAction(_ sender: Any!) {
        
        undoManager?.registerUndo(withTarget: self) {
            targetSelf in targetSelf.hideMouseAction(sender)
        }
        
        if (hideMouse) {
            NSCursor.unhide()
            CGAssociateMouseAndMouseCursorPosition(boolean_t(truncating: true))
        } else {
            NSCursor.hide()
            CGAssociateMouseAndMouseCursorPosition(boolean_t(truncating: false))
        }
        
        hideMouse = !hideMouse
    }
    
    //
    // Action methods (Keyboard menu)
    //

    @IBAction func stickyKeyboardAction(_ sender: Any!) {
        
        // Open the virtual keyboard as a window
        let kb = VirtualKeyboardController.make()
        myAppDelegate.virtualKeyboard = kb
        myAppDelegate.virtualKeyboard?.showWindow()
    }
    
    @IBAction func mapCmdKeysAction(_ sender: Any!) {
        
        mapCommandKeys = !mapCommandKeys
        refreshStatusBar()
    }
    
    @IBAction func clearKeyboardMatrixAction(_ sender: Any!) {
        
        amiga.keyboard.releaseAllKeys()
    }

    // -----------------------------------------------------------------
    @IBAction func loadDirectoryAction(_ sender: Any!) {
        keyboardcontroller.type(string: "LOAD \"$\",8", completion: nil)
    }
    @IBAction func listAction(_ sender: Any!) {
        keyboardcontroller.type(string: "LIST", completion: nil)
    }
    @IBAction func loadFirstFileAction(_ sender: Any!) {
        keyboardcontroller.type(string: "LOAD \"*\",8,1", completion: nil)
    }
    @IBAction func runProgramAction(_ sender: Any!) {
        keyboardcontroller.type(string: "RUN", completion: nil)
    }
    @IBAction func formatDiskAction(_ sender: Any!) {
        keyboardcontroller.type(string: "OPEN 1,8,15,\"N:TEST, ID\": CLOSE 1", completion: nil)
    }

 
    //
    // Action methods (Disk menu)
    //

    func driveNr(fromTagOf menuItem: Any!) -> Int {
        let tag = (menuItem as! NSMenuItem).tag
        precondition(tag == 1 || tag == 2)
        return tag
    }
    @IBAction func newDiskAction(_ sender: Any!) {
        
        let tag = (sender as! NSMenuItem).tag
        let emptyArchive = AnyArchiveProxy.make()
        
        mydocument.attachment = D64FileProxy.make(withAnyArchive: emptyArchive)
        mydocument.mountAttachmentAsDisk(drive: tag)
        mydocument.clearRecentlyExportedDiskURLs(drive: tag)
    }
    
    @IBAction func insertDiskAction(_ sender: Any!) {
        
        let tag = (sender as! NSMenuItem).tag
        
        // Ask user to continue if the current disk contains modified data
        if !proceedWithUnexportedDisk(drive: tag) {
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
                        try self.mydocument.createAttachment(from: url)
                        self.mydocument.mountAttachmentAsDisk(drive: tag)
                    } catch {
                        NSApp.presentError(error)
                    }
                }
            }
        })
    }
    
    @IBAction func insertRecentDiskAction(_ sender: Any!) {
        
        track()
        var tag = (sender as! NSMenuItem).tag
        
        // Extrace drive number from tag
        var nr: Int
        if tag < 10 { nr = 1 } else { nr = 2; tag -= 10 }
        
        // Get URL and insert
        if let url = mydocument.getRecentlyInsertedDiskURL(tag) {
            do {
                try mydocument.createAttachment(from: url)
                if (mydocument.proceedWithUnexportedDisk(drive: nr)) {
                    mydocument.mountAttachmentAsDisk(drive: nr)
                }
            } catch {
                NSApp.presentError(error)
            }
        }
    }
    
    @IBAction func exportRecentDiskAction(_ sender: Any!) {
        
        track()
        var tag = (sender as! NSMenuItem).tag
        
        // Extract drive number from tag
        let nr = (tag < 10) ? 1 : 2
        tag = (tag < 10) ? tag : tag - 10
       
        // Get URL and export
        if let url = mydocument.getRecentlyExportedDiskURL(tag, drive: nr) {
            mydocument.export(drive: nr, to: url)
        }
    }
    
    @IBAction func clearRecentlyInsertedDisksAction(_ sender: Any!) {
        mydocument.recentlyInsertedDiskURLs = []
    }

    @IBAction func clearRecentlyExportedDisksAction(_ sender: Any!) {

        let driveNr = (sender as! NSMenuItem).tag
        mydocument.clearRecentlyExportedDiskURLs(drive: driveNr)
    }

    @IBAction func clearRecentlyInsertedTapesAction(_ sender: Any!) {
        mydocument.recentlyInsertedTapeURLs = []
    }
    
    @IBAction func clearRecentlyAttachedCartridgesAction(_ sender: Any!) {
        mydocument.recentlyAttachedCartridgeURLs = []
    }
    
    @IBAction func ejectDiskAction(_ sender: Any!) {
        
        let tag = (sender as! NSMenuItem).tag
        
        if proceedWithUnexportedDisk(drive: tag) {
            changeDisk(nil, drive: tag)
            mydocument.clearRecentlyExportedDiskURLs(drive: tag)
        }
    }
    
    @IBAction func exportDiskAction(_ sender: Any!) {

        let nr = (sender as! NSMenuItem).tag
        precondition(nr == 1 || nr == 2)
        
        let nibName = NSNib.Name("ExportDiskDialog")
        let exportPanel = ExportDiskController.init(windowNibName: nibName)
        exportPanel.showSheet(forDrive: nr)
    }
     
    @IBAction func writeProtectAction(_ sender: Any!) {
        
        let nr = driveNr(fromTagOf: sender)
        if (nr == 1) {
            c64.drive1.disk.toggleWriteProtection()
        } else {
            c64.drive2.disk.toggleWriteProtection()
        }
    }
    
    @IBAction func drivePowerAction(_ sender: Any!) {
        
        let sender = sender as! NSMenuItem
        precondition(sender.tag == 1 || sender.tag == 2)
        drivePowerAction(driveNr: sender.tag)
    }

    @IBAction func drivePowerButtonAction(_ sender: Any!) {
        
        let sender = sender as! NSButton
        precondition(sender.tag == 1 || sender.tag == 2)
        drivePowerAction(driveNr: sender.tag)
    }
    
    func drivePowerAction(driveNr: Int) {
        if (driveNr == 1) {
            c64.drive1.togglePowerSwitch()
        } else {
            c64.drive2.togglePowerSwitch()
        }
    }
    
    //
    // Action methods (Datasette menu)
    //
    
    @IBAction func insertTapeAction(_ sender: Any!) {
        
        // Show the OpenPanel
        let openPanel = NSOpenPanel()
        openPanel.allowsMultipleSelection = false
        openPanel.canChooseDirectories = false
        openPanel.canCreateDirectories = false
        openPanel.canChooseFiles = true
        openPanel.prompt = "Insert"
        openPanel.allowedFileTypes = ["tap"]
        openPanel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = openPanel.url {
                    do {
                        try self.mydocument.createAttachment(from: url)
                        self.mydocument.mountAttachmentAsTape()
                    } catch {
                        NSApp.presentError(error)
                    }
                }
            }
        })
    }
    
    @IBAction func insertRecentTapeAction(_ sender: Any!) {
        
        track()
        let sender = sender as! NSMenuItem
        let tag = sender.tag
        
        if let url = mydocument.getRecentlyInsertedTapeURL(tag) {
            do {
                try mydocument.createAttachment(from: url)
                mydocument.mountAttachmentAsTape()
            } catch {
                NSApp.presentError(error)
            }
        }
    }
    
    @IBAction func ejectTapeAction(_ sender: Any!) {
        track()
        c64.datasette.ejectTape()
    }
    
    @IBAction func playOrStopAction(_ sender: Any!) {
        track()
        if c64.datasette.playKey() {
            c64.datasette.pressStop()
        } else {
            c64.datasette.pressPlay()
        }
    }
    
    @IBAction func rewindAction(_ sender: Any!) {
        track()
        c64.datasette.rewind()
    }

    
    //
    // Action methods ()
    //


        
    //
    // Action methods (Debug menu)
    //

    @IBAction func hideSpritesAction(_ sender: Any!) {

        undoManager?.registerUndo(withTarget: self) {
            targetSelf in targetSelf.hideSpritesAction(sender)
        }
        
        c64.vic.setHideSprites(!c64.vic.hideSprites())
    }
  
    @IBAction func markIRQLinesAction(_ sender: Any!) {
    
        undoManager?.registerUndo(withTarget: self) {
            targetSelf in targetSelf.markIRQLinesAction(sender)
        }
        
        c64.vic.setShowIrqLines(!c64.vic.showIrqLines())
    }
    
    @IBAction func markDMALinesAction(_ sender: Any!) {
    
        undoManager?.registerUndo(withTarget: self) {
            targetSelf in targetSelf.markDMALinesAction(sender)
        }
        
        c64.vic.setShowDmaLines(!c64.vic.showDmaLines())
    }
    
    @IBAction func traceAction(_ sender: Any!) {
        // Dummy target to make menu item validatable
    }
    
    @IBAction func dumpStateAction(_ sender: Any!) {
        // Dummy target to make menu item validatable
    }
  
    @IBAction func traceIecAction(_ sender: Any!) {
        
        undoManager?.registerUndo(withTarget: self) {
            targetSelf in targetSelf.traceIecAction(sender)
        }
        
        c64.iec.setTracing(!c64.iec.tracing())
    }
 
    @IBAction func traceVC1541CpuAction(_ sender: Any!) {
        
        undoManager?.registerUndo(withTarget: self) {
            targetSelf in targetSelf.traceVC1541CpuAction(sender)
        }
        
        c64.drive1.cpu.setTracing(!c64.drive1.cpu.tracing())
    }
  
    @IBAction func traceViaAction(_ sender: Any!) {
        
        undoManager?.registerUndo(withTarget: self) {
            targetSelf in targetSelf.traceViaAction(sender)
        }
        
        c64.drive1.via1.setTracing(!c64.drive1.via1.tracing())
        c64.drive1.via2.setTracing(!c64.drive1.via2.tracing())
    }
    
    @IBAction func dumpC64(_ sender: Any!) { c64.dump() }
    @IBAction func dumpC64CPU(_ sender: Any!) { c64.cpu.dump() }
    @IBAction func dumpC64CIA1(_ sender: Any!) {c64.cia1.dump() }
    @IBAction func dumpC64CIA2(_ sender: Any!) { c64.cia2.dump() }
    @IBAction func dumpC64VIC(_ sender: Any!) { c64.vic.dump() }
    @IBAction func dumpC64SID(_ sender: Any!) { c64.sid.dump() }
    @IBAction func dumpC64Memory(_ sender: Any!) { c64.mem.dump() }
    @IBAction func dumpVC1541(_ sender: Any!) { c64.drive1.dump() }
    @IBAction func dumpVC1541CPU(_ sender: Any!) { c64.drive1.dump() }
    @IBAction func dumpVC1541VIA1(_ sender: Any!) { c64.drive1.via1.dump() }
    @IBAction func dumpVC1541VIA2(_ sender: Any!) { c64.drive1.via2.dump() }
    @IBAction func dumpDisk(_ sender: Any!) { c64.drive1.disk.dump() }
    @IBAction func dumpKeyboard(_ sender: Any!) { c64.keyboard.dump() }
    @IBAction func dumpC64JoystickA(_ sender: Any!) { c64.port1.dump() }
    @IBAction func dumpC64JoystickB(_ sender: Any!) { c64.port2.dump(); gamePadManager.listDevices()}
    @IBAction func dumpIEC(_ sender: Any!) { c64.iec.dump() }
    @IBAction func dumpC64ExpansionPort(_ sender: Any!) { c64.expansionport.dump() }
    
    //
    // Action methods ()
    //
    

    
}
