// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// DEPRECATED
// TODO: MOVE TO Error.swift

extension MyDocument {
    
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

    func proceedWithUnsavedHardDrive(drive: HardDriveProxy) -> Bool {
        return mydocument.proceedWithUnsavedHardDrive(drive: drive)
    }

    func proceedWithUnsavedHardDrive() -> Bool {
        let drives = [amiga.hd0!, amiga.hd1!, amiga.hd2!, amiga.hd3!]
        return mydocument.proceedWithUnsavedHardDrive(drives: drives)
    }
}
