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

let logLevel = releaseBuild ? 0 : 1

public func log(_ msg: String = "", level: Int = 1,
                path: String = #file, function: String = #function, line: Int = #line) {
    
    if level <= logLevel {
        
        if let file = URL(string: path)?.deletingPathExtension().lastPathComponent {
            if msg == "" {
                print("\(file).\(line)::\(function)")
            } else {
                print("\(file).\(line)::\(function): \(msg)")
            }
        }
    }
}

public func log(warning: String,
                path: String = #file, function: String = #function, line: Int = #line) {
    
    log(warning, level: logLevel, path: path, function: function, line: line)
}

//
// Errors
//

class VAError: Error {
    
    var errorCode: ErrorCode
    var what: String

    init(_ exception: ExceptionWrapper) {

        self.errorCode = exception.errorCode
        self.what = exception.what
    }
    
    init(_ errorCode: ErrorCode, _ what: String = "") {
        
        self.errorCode = errorCode
        self.what = what
    }
}

extension NSError {
    
    convenience init(error: VAError) {
        
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
    case cantInsert
    case cantOpen(url: URL)
    case cantRecord
    case cantRun
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

        case .cantInsert:
            return "Failed to insert disk."

        case let .cantOpen(url):
            return "\"\(url.lastPathComponent)\" can't be opened."

        case .cantRecord:
            return "Failed to launch the screen recorder."

        case .cantRun:
            return "Configuration error"
            
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
            "vAmiga can only run on machines supporting the Metal graphics " +
            "technology (2012 models and above)."
            
        case .recorderAborted: return
            "Failed to write to the FFmpeg pipes."
        
        case let .recorderSandboxed(exec): return
            "vAmiga is running as a sandboxed application and has no " +
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
    
        if let error = error as? VAError {
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

extension MyController {
    
    func askToPowerOff() -> Bool {
        
        if amiga.poweredOn {
            
            let alert = NSAlert()
            
            alert.alertStyle = .informational
            alert.icon = NSImage(named: "powerSwitch")
            alert.messageText = "The emulator must be powered off to perform this operation."
            alert.informativeText = "Your changes will be lost if you proceed."
            alert.addButton(withTitle: "Proceed")
            alert.addButton(withTitle: "Cancel")
            
            if alert.runSheet(for: window!) == .alertFirstButtonReturn {
                amiga.powerOff()
            } else {
                return false
            }
        }
        return true
    }
}

// DEPRECATED (MOVE TO ALERT SECTION)

extension VAError {
    
    static func alert(_ msg1: String, _ msg2: String, style: NSAlert.Style,
                      async: Bool = false, icon: String?) {
        
        if async {
            
            DispatchQueue.main.async {
                self.alert(msg1, msg2, style: style, async: false, icon: icon)
            }
            
        } else {
            
            let alert = NSAlert()
            alert.alertStyle = style
            alert.icon = icon != nil ? NSImage(named: icon!) : nil
            alert.messageText = msg1
            alert.informativeText = msg2
            alert.addButton(withTitle: "OK")
            alert.runModal()
        }
    }
    
    static func informational(_ msg1: String, _ msg2: String,
                              async: Bool = false, icon: String? = nil) {
        alert(msg1, msg2, style: .informational, async: async, icon: icon)
    }
    
    static func warning(_ msg1: String, _ msg2: String,
                        async: Bool = false, icon: String? = nil) {
        alert(msg1, msg2, style: .warning, async: async, icon: icon)
    }
    
    static func critical(_ msg1: String, _ msg2: String,
                         async: Bool = false, icon: String? = nil) {
        alert(msg1, msg2, style: .critical, async: async, icon: icon)
    }

    func informational(_ msg: String, async: Bool = false, icon: String? = nil) {
        VAError.informational(msg, what, async: async, icon: icon)
    }

    func warning(_ msg: String, async: Bool = false, icon: String? = nil) {
        VAError.warning(msg, what, async: async, icon: icon)
    }

    func critical(_ msg: String, async: Bool = false, icon: String? = nil) {
        VAError.critical(msg, what, async: async, icon: icon)
    }
}
