// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// DEPRECATED
// TODO: MOVE TO VAError as static functions

extension MyDocument {
    
    func showNoMetalSupportAlert() {
        
        let alert = NSAlert()
        alert.alertStyle = .critical
        alert.icon = NSImage(named: "metal")
        alert.messageText = "No suitable GPU hardware found"
        alert.informativeText = "vAmiga can only run on machines supporting the Metal graphics technology (2012 models and above)."
        alert.addButton(withTitle: "Exit")
        alert.runModal()
    }
    
    @discardableResult
    func showDiskIsUnexportedAlert(messageText: String) -> NSApplication.ModalResponse {
       
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.icon = NSImage(named: "adf")
        alert.messageText = messageText
        alert.informativeText = "Your changes will be lost if you proceed."
        alert.addButton(withTitle: "Proceed")
        alert.addButton(withTitle: "Cancel")
        return alert.runModal()
    }

    func proceedWithUnsavedFloppyDisk(drives: [FloppyDriveProxy]) -> Bool {
        
        let modified = drives.filter { $0.hasModifiedDisk }
        
        if modified.isEmpty || parent.pref.ejectWithoutAsking {
            return true
        }
        
        let names = modified.map({ "df" + String($0.nr) }).joined(separator: ", ")
        let text = modified.count == 1 ?
        "Drive \(names) contains an unsaved disk." :
        "Drives \(names) contain unsaved disks."

        return showDiskIsUnexportedAlert(messageText: text) == .alertFirstButtonReturn
    }
    
    func proceedWithUnsavedFloppyDisk(drive: FloppyDriveProxy) -> Bool {
        
        return proceedWithUnsavedFloppyDisk(drives: [drive])
    }
        
    func proceedWithUnsavedFloppyDisk(drive nr: Int) -> Bool {
        
        return proceedWithUnsavedFloppyDisk(drive: amiga.df(nr)!)
    }
        
    @discardableResult
    func showHdrIsUnexportedAlert(messageText: String) -> NSApplication.ModalResponse {
       
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.icon = NSImage(named: "hdf")
        alert.messageText = messageText
        alert.informativeText = "Your changes will be lost if you proceed."
        alert.addButton(withTitle: "Proceed")
        alert.addButton(withTitle: "Cancel")
        return alert.runModal()
    }

    func proceedWithUnsavedHardDrive(drives: [HardDriveProxy]) -> Bool {
        
        let modified = drives.filter { $0.hasModifiedDisk }
        
        if modified.isEmpty || parent.pref.detachWithoutAsking {
            return true
        }
        
        let names = modified.map({ "hd" + String($0.nr) }).joined(separator: ", ")
        let text = modified.count == 1 ?
        "Hard drive \(names) contains an unsaved disk." :
        "Hard drives \(names) contain unsaved disks."

        return showHdrIsUnexportedAlert(messageText: text) == .alertFirstButtonReturn
    }
    
    func proceedWithUnsavedHardDrive(drive: HardDriveProxy) -> Bool {
        
        return proceedWithUnsavedHardDrive(drives: [drive])
    }
        
    func proceedWithUnsavedHardDrive(drive nr: Int) -> Bool {
        
        return proceedWithUnsavedHardDrive(drive: amiga.hd(nr)!)
    }    
}

extension MyController {

    func warning(_ msg1: String, _ msg2: String, icon: String? = nil) {

        alert(msg1, msg2, style: .warning, icon: icon)
    }

    func critical(_ msg1: String, _ msg2: String, icon: String? = nil) {

        alert(msg1, msg2, style: .critical, icon: icon)
    }

    func alert(_ msg1: String, _ msg2: String, style: NSAlert.Style, icon: String?) {
        
        let alert = NSAlert()
        alert.alertStyle = style
        if icon != nil { alert.icon = NSImage(named: icon!) }
        alert.messageText = msg1
        alert.informativeText = msg2
        alert.addButton(withTitle: "OK")
        alert.runModal()
    }
    
    func proceedWithUnsavedFloppyDisk(drive: FloppyDriveProxy) -> Bool {
        return mydocument.proceedWithUnsavedFloppyDisk(drive: drive)
    }

    func proceedWithUnsavedFloppyDisk() -> Bool {
        let drives = [amiga.df0!, amiga.df1!, amiga.df2!, amiga.df3!]
        return mydocument.proceedWithUnsavedFloppyDisk(drives: drives)
    }

    func proceedWithUnsavedHdr(drive: HardDriveProxy) -> Bool {
        return mydocument.proceedWithUnsavedHardDrive(drive: drive)
    }

    func proceedWithUnsavedHdr() -> Bool {
        let drives = [amiga.hd0!, amiga.hd1!, amiga.hd2!, amiga.hd3!]
        return mydocument.proceedWithUnsavedHardDrive(drives: drives)
    }

    @discardableResult
    func askToPowerOffAlert() -> NSApplication.ModalResponse {
       
        let alert = NSAlert()
        alert.alertStyle = .informational
        alert.icon = NSImage(named: "powerSwitch")
        alert.messageText = "The emulator must be powered off to perform this operation."
        alert.informativeText = "Your changes will be lost if you proceed."
        alert.addButton(withTitle: "Proceed")
        alert.addButton(withTitle: "Cancel")
        return alert.runModal()
    }

    func askToPowerOff() -> Bool {
        
        if amiga.poweredOn {
            
            if askToPowerOffAlert() != .alertFirstButtonReturn { return false }
            amiga.powerOff()
        }
        
        return true
    }
    
    @discardableResult
    func askToRebootAlert() -> NSApplication.ModalResponse {
       
        let alert = NSAlert()
        alert.alertStyle = .informational
        alert.icon = NSImage(named: "powerSwitch")
        alert.messageText = "The Amiga must be restarted to perform this operation."
        alert.informativeText = "Your changes will be lost if you proceed."
        alert.addButton(withTitle: "Proceed")
        alert.addButton(withTitle: "Cancel")
        return alert.runModal()
    }

    func askToReboot() -> Bool {
            
        return amiga.poweredOff || askToRebootAlert() == .alertFirstButtonReturn
    }
    
    func showMissingFFmpegAlert() {

        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.icon = NSImage(named: "FFmpegIcon")

        if pref.ffmpegPath == "" {

            alert.messageText = "Screen recording requires FFmpeg to be installed."
            alert.informativeText = "Visit FFmpeg.org for installation instructions."

        } else {

            alert.messageText = "Unable to locate FFmpeg."
            alert.informativeText = "\(pref.ffmpegPath) not found."
        }
        
        alert.addButton(withTitle: "OK")
        alert.runModal()
    }

    /*
    func showFailedToLaunchFFmpegAlert() {

        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.icon = NSImage(named: "FFmpegIcon")
        alert.messageText = "Failed to launch the screen recorder."
        alert.informativeText = "No content will be recorded."
        alert.addButton(withTitle: "OK")
        alert.runModal()
    }
    */
    
    /*
    func showScreenRecorderAlert(url: URL) {

        let alert = NSAlert()
        alert.alertStyle = .critical
        alert.icon = NSImage(named: "FFmpegIcon")
        alert.messageText = "\"\(url.lastPathComponent)\" cannot be opened."
        alert.informativeText = "The screen recorder failed to open this file for output."
        alert.addButton(withTitle: "OK")
        
        alert.beginSheetModal(for: self.window!) { (_: NSApplication.ModalResponse) -> Void in }
    }
    */
}
