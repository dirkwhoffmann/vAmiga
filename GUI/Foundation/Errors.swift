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

    case noMetalSupport
    case cantOpen(url: URL)
    case unknown

    var alertStyle: NSAlert.Style {
        
        switch self {
            
        case .noMetalSupport: return .critical
            
        default: return .warning
        }
    }
    
    var icon: NSImage? {
        
        switch self {
            
        case .noMetalSupport: return NSImage(named: "metal")!
            
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
            
        case .noMetalSupport:
            return "No suitable GPU hardware found"
            
        case let .cantOpen(url):
            return "\"\(url.lastPathComponent)\" can't be opened."
            
        default:
            return ""
        }
    }
    
    var explanation: String {
        
        switch self {
            
        case .noMetalSupport: return
            "vAmiga can only run on machines supporting the Metal graphics " +
            "technology (2012 models and above)."
            
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

    func showAlert(_ failure: Failure, _ explanation: String? = nil, async: Bool = false) {
 
        let alert = NSAlert()
        alert.alertStyle = failure.alertStyle
        alert.icon = failure.icon
        alert.messageText = failure.description
        alert.informativeText = explanation ?? failure.explanation
        alert.addButton(withTitle: failure.buttonTitle)
        
        if let window = windowForSheet {
            alert.runSheet(for: window, async: async)
        } else {
            alert.runModal()
        }
    }

    func showAlert(_ failure: Failure, _ error: Error, async: Bool = false) {
    
        if let error = error as? VAError {
            showAlert(failure, error.what, async: async)
        } else {
            showAlert(failure, error.localizedDescription, async: async)
        }
    }
}

extension MyController {

    func showAlert(_ failure: Failure, _ explanation: String? = nil, async: Bool = false) {
        
        mydocument.showAlert(failure, explanation, async: async)
    }

    func showAlert(_ failure: Failure, _ error: Error, async: Bool = false) {
    
        mydocument.showAlert(failure, error, async: async)
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
    
    //
    // Customized alerts
    //

    /*
    func cantOpen(url: URL) {
        warning("\"\(url.lastPathComponent)\" can't be opened.")
    }
    */
    
    func cantInsert(async: Bool = false) {
        warning("Failed to insert disk.", async: async)
    }

    func cantAttach(async: Bool = false) {
        warning("Failed to attach hard drive.", async: async)
    }

    func notReady(async: Bool = false) {
        informational("Configuration error", async: async, icon: "pref_transparent")
    }
    
    func cantRecord(async: Bool = false) {
        warning("Failed to launch the screen recorder.", async: async, icon: "FFmpegIcon")
    }

    static func recordingAborted(async: Bool = false) {
        
        warning("Screen recording has been aborted.",
                "Failed to write to the FFmpeg pipes.",
                async: async, icon: "mp4")
    }

    static func recorderSanboxed(name: String, async: Bool = false) {
        
        warning("The selected screen recorder is unavailable.",
                "vAmiga is running as a sandboxed application and has no " +
                "permission to access file \(name). " +
                "Please copy the file to the Applications folder.",
                async: async, icon: "FFmpegIcon")
    }

}
