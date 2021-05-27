// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension NSError {

    /*
    static func snapshotVersionError(filename: String) -> NSError {
        return NSError(domain: "vAmiga", code: 0, userInfo:
            [NSLocalizedDescriptionKey: "The document \"\(filename)\" could not be opened.",
                NSLocalizedRecoverySuggestionErrorKey: "The snapshot was created with a different version of vAmiga."])
    }
    */
    
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

    static func fileAccessError(filename: String) -> NSError {
        return NSError(domain: "vAmiga", code: 0, userInfo:
            [NSLocalizedDescriptionKey: "The document \"\(filename)\" could not be opened.",
                NSLocalizedRecoverySuggestionErrorKey: "Unable to access file."])
    }

    static func fileAccessError() -> NSError {
        return NSError(domain: "vAmiga", code: 0, userInfo:
            [NSLocalizedDescriptionKey: "The document could not be opened.",
                NSLocalizedRecoverySuggestionErrorKey: "Unable to access file."])
    }

    static func extendedAdfError() -> NSError {
        return NSError(domain: "vAmiga", code: 0, userInfo:
            [NSLocalizedDescriptionKey: "The document could not be opened.",
                NSLocalizedRecoverySuggestionErrorKey: "The file is encoded in extended ADF format which is not supported by the emulator."])
    }
}

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

    func proceedWithUnexportedDisk(drives: [DriveProxy]) -> Bool {
        
        let modified = drives.filter { $0.isModifiedDisk }
        
        if modified.isEmpty || parent.pref.ejectWithoutAsking {
            return true
        }
        
        let names = drives.map({ "df" + String($0.nr) }).joined(separator: ", ")
        let text = "Drive \(names) contains an unexported disk."

        return showDiskIsUnexportedAlert(messageText: text) == .alertFirstButtonReturn
    }
    
    func proceedWithUnexportedDisk(drive: DriveProxy) -> Bool {
        
        return proceedWithUnexportedDisk(drives: [drive])
    }
        
    func proceedWithUnexportedDisk(drive nr: Int) -> Bool {
        
        return proceedWithUnexportedDisk(drive: amiga.df(nr)!)
    }
    
    func proceedWithUnexportedDisk() -> Bool {
    
        return proceedWithUnexportedDisk(drives: [ amiga.df0,
                                                   amiga.df1,
                                                   amiga.df2,
                                                   amiga.df3 ])
    }
    
    func showConfigurationAltert(_ error: ErrorCode) {

        var msg: String

        switch error {
        case .ROM_MISSING:
            msg = "A Kickstart Rom or Boot Rom is required to power up."
        case .CHIP_RAM_LIMIT:
            msg = "The selected Agnus revision does not support the selected amout of Chip Ram."
        case .AROS_RAM_LIMIT:
            msg = "The Aros Kickstart replacement requires at least 1 MB of memory to boot."
        case .AROS_NO_EXTROM:
            msg = "The Aros Kickstart replacement requires an extension Rom."
        default:
            msg = ""
        }

        let alert = NSAlert()
        alert.alertStyle = .informational
        alert.icon = NSImage(named: "pref_transparent")
        alert.messageText = "Configuration error"
        alert.informativeText = msg
        alert.addButton(withTitle: "OK")
        alert.runModal()
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
    
    func proceedWithUnexportedDisk(drive: DriveProxy) -> Bool {
        return mydocument.proceedWithUnexportedDisk(drive: drive)
    }
    
    func proceedWithUnexportedDisk(drive nr: Int) -> Bool {
        return mydocument.proceedWithUnexportedDisk(drive: nr)
    }

    func proceedWithUnexportedDisk() -> Bool {
        return mydocument.proceedWithUnexportedDisk()
    }

    func showMissingFFmpegAlert() {

        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.icon = NSImage(named: "FFmpegIcon")
        alert.messageText = "Screen recording requires FFmpeg to be installed in /usr/local/bin."
        alert.informativeText = "Visit FFmpeg.org for installation instructions."
        alert.addButton(withTitle: "OK")
        alert.runModal()
    }

    func showFailedToLaunchFFmpegAlert() {

        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.icon = NSImage(named: "FFmpegIcon")
        alert.messageText = "Failed to launch the screen recorder."
        alert.informativeText = "No content will be recorded."
        alert.addButton(withTitle: "OK")
        alert.runModal()
    }

    func showScreenRecorderAlert(url: URL) {

        let alert = NSAlert()
        alert.alertStyle = .critical
        alert.icon = NSImage(named: "FFmpegIcon")
        alert.messageText = "\"\(url.lastPathComponent)\" cannot be opened."
        alert.informativeText = "The screen recorder failed to open this file for output."
        alert.addButton(withTitle: "OK")
        
        alert.beginSheetModal(for: self.window!) { (_: NSApplication.ModalResponse) -> Void in }
    }
}
