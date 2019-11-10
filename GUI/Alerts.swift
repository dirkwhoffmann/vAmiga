// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension NSError {

    static func snapshotVersionError(filename: String) -> NSError {
        return NSError(domain: "vAmiga", code: 0, userInfo:
            [NSLocalizedDescriptionKey: "The document \"\(filename)\" could not be opened.",
                NSLocalizedRecoverySuggestionErrorKey: "The snapshot was created with a different version of vAmiga."])
    }

    static func unsupportedFormatError(filename: String) -> NSError {
        return NSError(domain: "vAmiga", code: 0, userInfo:
            [NSLocalizedDescriptionKey: "The document \"\(filename)\" could not be opened.",
                NSLocalizedRecoverySuggestionErrorKey: "The format of this file is not supported."])
    }

    static func corruptedFileError(filename: String) -> NSError {        
        return NSError(domain: "vAmiga", code: 0, userInfo:
            [NSLocalizedDescriptionKey: "The document \"\(filename)\" could not be opened.",
                NSLocalizedRecoverySuggestionErrorKey: "The file appears to be corrupt. It's contents does not match the purported format."])
    }
}

public extension MetalView {
    
    func showNoMetalSupportAlert() {
        
        let alert = NSAlert()
        alert.alertStyle = .critical
        alert.icon = NSImage.init(named: "metal")
        alert.messageText = "No suitable GPU hardware found"
        alert.informativeText = "vAmiga can only run on machines supporting the Metal graphics technology (2012 models and above)."
        alert.addButton(withTitle: "Exit")
        alert.runModal()
    }
}

extension MyDocument {
    
    @discardableResult
    func showDiskIsUnexportedAlert(messageText: String) -> NSApplication.ModalResponse {
       
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.icon = NSImage.init(named: "diskette")
        alert.messageText = messageText
        alert.informativeText = "Your changes will be lost if you proceed."
        alert.addButton(withTitle: "Proceed")
        alert.addButton(withTitle: "Cancel")
        return alert.runModal()
    }

    func proceedWithUnexportedDisk(drives: [DriveProxy]) -> Bool {
        
        let modified = drives.filter { $0.hasModifiedDisk() }
        
        if modified.isEmpty || myController!.ejectWithoutAsking {
            return true
        }
        
        let names = drives.map({ "df" + String($0.nr()) }).joined(separator: ", ")
        let text = "Drive \(names) contains an unexported disk."

        return showDiskIsUnexportedAlert(messageText: text) == .alertFirstButtonReturn
    }
    
    func proceedWithUnexportedDisk(drive: DriveProxy) -> Bool {
        
        return proceedWithUnexportedDisk(drives: [drive])
    }
        
    func proceedWithUnexportedDisk(drive nr: Int) -> Bool {
        
        switch nr {
        case 0: return proceedWithUnexportedDisk(drives: [amiga.df0])
        case 1: return proceedWithUnexportedDisk(drives: [amiga.df1])
        case 2: return proceedWithUnexportedDisk(drives: [amiga.df2])
        case 3: return proceedWithUnexportedDisk(drives: [amiga.df3])
        default: fatalError()
        }
    }
    
    func proceedWithUnexportedDisk() -> Bool {
    
        return proceedWithUnexportedDisk(drives: [ amiga.df0,
                                                   amiga.df1,
                                                   amiga.df2,
                                                   amiga.df3 ])
    }

    func showExportErrorAlert(url: URL) {
        
        let path = url.path
        let alert = NSAlert()
        alert.alertStyle = .critical
        alert.icon = NSImage.init(named: "diskette")
        alert.messageText = "Failed to export disk to file"
        alert.informativeText = "\(path)."
        alert.addButton(withTitle: "OK")
        alert.runModal()
    }

    func showConfigurationAltert(_ error: UInt32) {

        var msg: String

        switch error {
        case MSG_ROM_MISSING.rawValue:
            msg = "A Kickstart Rom or Boot Rom is required to power up."
        case MSG_CHIP_RAM_LIMIT.rawValue:
            msg = "The selected Agnus revision does not support the selected amout of Chip Ram."
        case MSG_AROS_RAM_LIMIT.rawValue:
            msg = "The Aros Kickstart replacement requires at least 1 MB of memory to boot."
        default:
            msg = ""
        }

        let alert = NSAlert()
        alert.alertStyle = .informational
        alert.icon = NSImage.init(named: "pref_transparent")
        alert.messageText = "Configuration error"
        alert.informativeText = msg
        alert.addButton(withTitle: "OK")
        alert.runModal()
    }
}

extension MyController {
            
    func proceedWithUnexportedDisk(drive: DriveProxy) -> Bool {
        return mydocument?.proceedWithUnexportedDisk(drive: drive) ?? false
    }
    
    func proceedWithUnexportedDisk(drive nr: Int) -> Bool {
        return mydocument?.proceedWithUnexportedDisk(drive: nr) ?? false
    }

    func proceedWithUnexportedDisk() -> Bool {
        return mydocument?.proceedWithUnexportedDisk() ?? false
    }

}
