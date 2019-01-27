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
        
        var dfn : AmigaDriveProxy {
            
            switch (item.tag) {
                
            case 0: return amiga.df0
            case 1: return amiga.df1
                
            default: fatalError()
            }
        }
        
        func validateURLlist(_ list : [URL], image: NSImage) -> Bool {
            
            if let pos = Int(item.title) {
                
                if let url = mydocument.getRecentlyUsedURL(pos, from: list) {
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
        
        switch (item.action) {
            
            //
            // Edit menu
            //
            
        case #selector(MyController.runOrHaltAction(_:)):
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

        case #selector(MyController.hideMouseAction(_:)):
            item.title = hideMouse ? "Show Mouse Cursor" : "Hide Mouse Cursor"
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
            
            return validateURLlist(mydocument.recentlyInsertedDiskURLs, image: smallDisk)
        
        case  #selector(MyController.ejectDiskAction(_:)),
              #selector(MyController.exportDiskAction(_:)):
            
            return dfn.hasDisk()

        case #selector(MyController.exportRecentDiskDummyAction0(_:)):
            return amiga.df0.hasDisk()

        case #selector(MyController.exportRecentDiskDummyAction1(_:)):
            return amiga.df1.hasDisk()
            
        case #selector(MyController.exportRecentDiskAction(_:)):
            switch item.tag {
            case 0: return validateURLlist(mydocument.recentlyExportedDisk0URLs, image: smallDisk)
            case 1: return validateURLlist(mydocument.recentlyExportedDisk1URLs, image: smallDisk)
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
            
        case #selector(MyController.pauseAction(_:)):
            return amiga.isRunning()
            
        case #selector(MyController.continueAction(_:)),
             #selector(MyController.stepIntoAction(_:)),
             #selector(MyController.stepOverAction(_:)),
             #selector(MyController.stopAndGoAction(_:)):
            return amiga.isPaused();
            
        case #selector(MyController.traceAction(_:)),
             #selector(MyController.dumpStateAction(_:)):
            return !amiga.releaseBuild()
            
        default:
            return true
        }
    }

    //
    // Action methods (App menu)
    //

    @IBAction func importPrefsAction(_ sender: Any!) {
        
        track()
        
        let panel = NSOpenPanel()
        panel.prompt = "Import"
        panel.allowedFileTypes = ["amigacnf"]
        
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
        panel.allowedFileTypes = ["amigacnf"]
        
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
        
        amiga.takeUserSnapshot()
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
        
        amiga.reset()
        refresh()
    }
    
    @IBAction func powerAction(_ sender: Any!) {
        
        amiga.powerOnOrOff()
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


    //
    // Action methods (Disk menu)
    //
    
    @IBAction func newDiskAction(_ sender: NSMenuItem!) {
        
        let emptyArchive =  ADFFileProxy.make()
        
        amiga.df(sender).insertDisk(emptyArchive)
        mydocument.clearRecentlyExportedDiskURLs(drive: sender.tag)
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
                        let adf = try self.mydocument.createADF(from: url)
                        self.amiga.df(sender).insertDisk(adf)
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
        if let url = mydocument.getRecentlyInsertedDiskURL(slot) {
            do {
                let adf = try self.mydocument.createADF(from: url)
                if (mydocument.proceedWithUnexportedDisk(drive: drive)) {
                    amiga.df(drive).insertDisk(adf)
                }
            } catch {
                NSApp.presentError(error)
            }
        }
    }
    
    func insertRecentDiskAction(drive: AmigaDriveProxy, slot: Int) {
        
        if let url = mydocument.getRecentlyInsertedDiskURL(slot) {
            do {
                let adf = try self.mydocument.createADF(from: url)
                if (mydocument.proceedWithUnexportedDisk(drive: drive)) {
                    drive.insertDisk(adf)
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
    @IBAction func exportRecentDiskAction(_ sender: NSMenuItem!) {
        
        track()
        
        let nr = sender.tag
        let slot = Int(sender.title)!
        
        // Get URL and export
        if let url = mydocument.getRecentlyExportedDiskURL(slot, drive: nr) {
            do {
                mydocument.export(drive: nr, to: url)
            }
        }
    }
    
    @IBAction func clearRecentlyInsertedDisksAction(_ sender: NSMenuItem!) {
        mydocument.recentlyInsertedDiskURLs = []
    }

    @IBAction func clearRecentlyExportedDisksAction(_ sender: NSMenuItem!) {

        mydocument.clearRecentlyExportedDiskURLs(drive: sender.tag)
    }

    @IBAction func ejectDiskAction(_ sender: NSMenuItem!) {
        
        assert(sender.tag == 0 || sender.tag == 1)
        let drive = sender.tag == 0 ? amiga.df0 : amiga.df1
        
        if proceedWithUnexportedDisk(drive: sender.tag) {
            drive?.ejectDisk()
            mydocument.clearRecentlyExportedDiskURLs(drive: sender.tag)
        }
    }
    
    @IBAction func exportDiskAction(_ sender: NSMenuItem!) {
        
        let nibName = NSNib.Name("ExportDiskDialog")
        let exportPanel = ExportDiskController.init(windowNibName: nibName)
        exportPanel.showSheet(forDrive: sender.tag)
    }
    
    @IBAction func dragAndDropTargetAction(_ sender: NSMenuItem!) {
        
        let drive = amiga.df(sender)
        dragAndDropDrive = (dragAndDropDrive == drive) ? nil : drive
    }
    
    
    //
    // Action methods (Debug menu)
    //

    @IBAction func traceAction(_ sender: Any!) {
        // Dummy target to make menu item validatable
    }
    
    @IBAction func dumpStateAction(_ sender: Any!) {
        // Dummy target to make menu item validatable
    }
  
}
