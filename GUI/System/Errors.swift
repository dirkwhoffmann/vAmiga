// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

//
// Logging / Debugging
//

public func log(_ enable: Int, _ msg: String = "",
                path: String = #file, function: String = #function, line: Int = #line) {

    if enable > 0 {

        if let file = URL(string: path)?.deletingPathExtension().lastPathComponent {
            if msg == "" {
                print("\(file).\(line)::\(function)")
            } else {
                print("\(file).\(line)::\(function): \(msg)")
            }
        }
    }
}

public func debug(_ enable: Int, _ msg: String = "",
                  path: String = #file, function: String = #function, line: Int = #line) {

    if !releaseBuild { log(enable, msg, path: path, function: function, line: line) }
}

public func warn(_ msg: String = "",
                 path: String = #file, function: String = #function, line: Int = #line) {

    log(1, "Warning: " + msg, path: path, function: function, line: line)
}

//
// Exceptions
//

final class AppError: Error {
    
    let errorCode: ErrorCode
    let what: String

    init(_ exception: ExceptionWrapper) {

        self.errorCode = exception.fault
        self.what = exception.what
    }
    
    init(_ errorCode: ErrorCode, _ what: String = "") {
        
        self.errorCode = errorCode
        self.what = what
    }
}

extension NSError {
    
    convenience init(error: AppError) {
        
        self.init(domain: "vAmiga",
                  code: error.errorCode.rawValue,
                  userInfo: [NSLocalizedRecoverySuggestionErrorKey: error.what])
    }
}

//
// Failures
//

enum Failure {

    case cantAttach
    case cantChangeGeometry
    case cantDecode
    case cantDetach
    case cantExport(url: URL)
    case cantInsert
    case cantLaunch
    case cantOpen(url: URL)
    case cantRecord
    case cantRestore
    case cantRun
    case cantSaveRoms
    case cantWriteThrough
    case noFFmpegFound(exec: String)
    case noFFmpegInstalled
    case noMetalSupport
    case recorderAborted
    case recorderSandboxed(exec: String)
    case unknown

    var alertStyle: NSAlert.Style {
        
        switch self {
            
        case .noMetalSupport: return .critical

        default: return .warning
        }
    }
    
    var icon: NSImage? {
        
        switch self {
            
        case .cantRecord: return NSImage(named: "FFmpegIcon")!
        case .cantRun: return NSImage(named: "pref_transparent")!
        case .noFFmpegFound: return NSImage(named: "FFmpegIcon")!
        case .noFFmpegInstalled: return NSImage(named: "FFmpegIcon")!
        case .noMetalSupport: return NSImage(named: "metal")!
        case .recorderAborted: return NSImage(named: "mp4")!
        case .recorderSandboxed: return NSImage(named: "FFmpegIcon")!

        default: return nil
        }
    }

    var buttonTitle: String {
        
        switch self {
            
        case .noMetalSupport: return "Exit"
            
        default: return "OK"
        }
    }
    
    var description: String {
        
        switch self {
            
        case .cantAttach:
            return "Failed to attach hard drive."

        case .cantChangeGeometry:
            return "Failed to change the drive geometry."
            
        case .cantDecode:
            return "Unable to decode the file system."

        case .cantDetach:
            return "Failed to detach hard drive."

        case let .cantExport(url: url):
            return "Cannot export disk to file \"\(url.path)\"."
            
        case .cantInsert:
            return "Failed to insert disk."

        case .cantLaunch:
            return "Failed to lauch the emulator." +
            "An unexpected exception has interrupted the internal startup procedure."

        case let .cantOpen(url):
            return "\"\(url.lastPathComponent)\" can't be opened."

        case .cantRecord:
            return "Failed to launch the screen recorder."

        case .cantRestore:
            return "Failed to restore snapshot."
            
        case .cantRun:
            return "Configuration error"
            
        case .cantSaveRoms:
            return "Failed to save Roms."
            
        case .cantWriteThrough:
            return "Unable to enable write-through mode."
            
        case .noFFmpegFound:
            return "Unable to locate FFmpeg."
            
        case .noFFmpegInstalled:
            return "Screen recording requires FFmpeg to be installed."

        case .noMetalSupport:
            return "No suitable GPU hardware found."
            
        case .recorderAborted:
            return "Screen recording has been aborted."
            
        case .recorderSandboxed:
            return "The selected screen recorder is unavailable."

        default:
            return ""
        }
    }
    
    var explanation: String {
        
        switch self {
            
        case let .noFFmpegFound(exec): return
            "\"\(exec)\" not found."

        case .noFFmpegInstalled: return
            "Visit FFmpeg.org for installation instructions."
            
        case .noMetalSupport: return
            "The emulator can only run on machines supporting the Metal graphics " +
            "technology (2012 models and above)."
            
        case .recorderAborted: return
            "Failed to write to the FFmpeg pipes."
        
        case let .recorderSandboxed(exec): return
            "The emulator is running as a sandboxed application and has no " +
            "permission to access file \"\(exec)\"" +
            "Please copy the file to the Applications folder."
            
        default:
            return ""
        }
    }
}

//
// Alerts
//

extension NSAlert {
    
    @discardableResult
    func runSheet(for window: NSWindow, async: Bool = false) -> NSApplication.ModalResponse {
        
        if async {
            
            beginSheetModal(for: window)
            return .OK
            
        } else {
            
            beginSheetModal(for: window, completionHandler: NSApp.stopModal(withCode:))
            return NSApp.runModal(for: window)
        }
    }
}

extension MyDocument {

    func showAlert(_ failure: Failure, what: String? = nil,
                   async: Bool = false, window: NSWindow? = nil) {
 
        let alert = NSAlert()
        alert.alertStyle = failure.alertStyle
        alert.icon = failure.icon
        alert.messageText = failure.description
        alert.informativeText = what ?? failure.explanation
        alert.addButton(withTitle: failure.buttonTitle)
        
        if let window = window {
            alert.runSheet(for: window, async: async)
        } else if let window = windowForSheet {
            alert.runSheet(for: window, async: async)
        } else {
            alert.runModal()
        }
    }

    func showAlert(_ failure: Failure, error: Error,
                   async: Bool = false, window: NSWindow? = nil) {
    
        if let error = error as? AppError {
            showAlert(failure, what: error.what, async:
                        async, window: window)
        } else {
            showAlert(failure, what: error.localizedDescription,
                      async: async, window: window)
        }
    }
}

extension MyController {

    func showAlert(_ failure: Failure, what: String? = nil,
                   async: Bool = false, window: NSWindow? = nil) {
        
        mydocument.showAlert(failure, what: what, async: async, window: window)
    }

    func showAlert(_ failure: Failure, error: Error,
                   async: Bool = false, window: NSWindow? = nil) {
    
        mydocument.showAlert(failure, error: error, async: async, window: window)
    }
}

//
// Alert dialogs
//

extension MyDocument {
    
    func showIsUnsavedAlert(msg: String, icon: String) -> NSApplication.ModalResponse {
        
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.icon = NSImage(named: icon)
        alert.messageText = msg
        alert.informativeText = "Your changes will be lost if you proceed."
        alert.addButton(withTitle: "Proceed")
        alert.addButton(withTitle: "Cancel")
        
        return alert.runSheet(for: windowForSheet!)
    }
    
    func showLaunchAlert(error: Error) {
             
        var reason: String
        if let error = error as? AppError {
            reason = error.what
        } else {
            reason = error.localizedDescription
        }
        
        let alert = NSAlert()
        alert.alertStyle = .warning
        alert.icon = NSImage(named: "biohazard")
        alert.messageText = "The emulator failed to launch."
        alert.informativeText = "An unexpected exception interrupted the startup process. " +
        "Please report the following error on GitHub:\n\n\(reason)"
        alert.addButton(withTitle: "Exit")
        
        if alert.runSheet(for: windowForSheet!) == .alertFirstButtonReturn {
            NSApp.terminate(self)
        }
    }
    
    func proceedWithUnsavedFloppyDisks(drives: [FloppyDriveProxy]) -> Bool {
        
        let modified = drives.filter { $0.info.hasModifiedDisk }

        if modified.isEmpty || pref.ejectWithoutAsking {
            return true
        }
        
        let names = modified.map({ "df" + String($0.info.nr) }).joined(separator: ", ")
        let text = modified.count == 1 ?
        "Drive \(names) contains an unsaved disk." :
        "Drives \(names) contain unsaved disks."
        
        return showIsUnsavedAlert(msg: text, icon: "adf") == .alertFirstButtonReturn
    }
    
    func proceedWithUnsavedFloppyDisk(drive: FloppyDriveProxy) -> Bool {
        
        return proceedWithUnsavedFloppyDisks(drives: [drive])
    }
            
    func proceedWithUnsavedFloppyDisks() -> Bool {
        
        let drives = [emu.df0!, emu.df1!, emu.df2!, emu.df3!]
        return proceedWithUnsavedFloppyDisks(drives: drives)
    }

    func proceedWithUnsavedHardDisks(drives: [HardDriveProxy]) -> Bool {
        
        let modified = drives.filter { $0.info.hasModifiedDisk }
        
        if modified.isEmpty || pref.detachWithoutAsking {
            return true
        }
        
        let names = modified.map({ "hd" + String($0.traits.nr) }).joined(separator: ", ")
        let text = modified.count == 1 ?
        "Hard drive \(names) contains an unsaved disk." :
        "Hard drives \(names) contain unsaved disks."
        
        return showIsUnsavedAlert(msg: text, icon: "hdf") == .alertFirstButtonReturn
    }
    
    func proceedWithUnsavedHardDisk(drive: HardDriveProxy) -> Bool {
        
        return proceedWithUnsavedHardDisks(drives: [drive])
    }
    
    func proceedWithUnsavedHardDisks() -> Bool {
        
        let drives = [emu.hd0!, emu.hd1!, emu.hd2!, emu.hd3!]
        return proceedWithUnsavedHardDisks(drives: drives)
    }
    
    func askToPowerOff() -> Bool {
        
        if emu.poweredOn {
            
            let alert = NSAlert()
            
            alert.alertStyle = .informational
            alert.icon = NSImage(named: "powerSwitch")
            alert.messageText = "The emulator must be powered off to perform this operation."
            alert.informativeText = "Your changes will be lost if you proceed."
            alert.addButton(withTitle: "Proceed")
            alert.addButton(withTitle: "Cancel")
            
            if alert.runSheet(for: windowForSheet!) == .alertFirstButtonReturn {
                emu.powerOff()
            } else {
                return false
            }
        }
        return true
    }
}

extension MyController {
    
    func proceedWithUnsavedFloppyDisk(drive: FloppyDriveProxy) -> Bool {
        return mydocument.proceedWithUnsavedFloppyDisk(drive: drive)
    }

    func proceedWithUnsavedFloppyDisks() -> Bool {
        return mydocument.proceedWithUnsavedFloppyDisks()
    }

    func proceedWithUnsavedHardDisk(drive: HardDriveProxy) -> Bool {
        return mydocument.proceedWithUnsavedHardDisk(drive: drive)
    }

    func proceedWithUnsavedHardDisk(drive: Int) -> Bool {
        return mydocument.proceedWithUnsavedHardDisk(drive: emu.hd(drive)!)
    }

    func proceedWithUnsavedHardDisks() -> Bool {
        return mydocument.proceedWithUnsavedHardDisks()
    }
    
    func askToPowerOff() -> Bool {
        return mydocument.askToPowerOff()
    }
}

extension MediaManager {
    
    func proceedWithUnsavedFloppyDisk(drive: FloppyDriveProxy) -> Bool {
        return mydocument.proceedWithUnsavedFloppyDisk(drive: drive)
    }

    func proceedWithUnsavedFloppyDisks() -> Bool {
        return mydocument.proceedWithUnsavedFloppyDisks()
    }

    func proceedWithUnsavedHardDisk(drive: HardDriveProxy) -> Bool {
        return mydocument.proceedWithUnsavedHardDisk(drive: drive)
    }

    func proceedWithUnsavedHardDisk(drive: Int) -> Bool {
        return mydocument.proceedWithUnsavedHardDisk(drive: emu.hd(drive)!)
    }

    func proceedWithUnsavedHardDisks() -> Bool {
        return mydocument.proceedWithUnsavedHardDisks()
    }
    
    func askToPowerOff() -> Bool {
        return mydocument.askToPowerOff()
    }
}
