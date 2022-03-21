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

    func cantOpen(url: URL, async: Bool = false) {
        warning("\"\(url.lastPathComponent)\" can't be opened.", async: async)
    }

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

extension NSError {
    
    convenience init(error: VAError) {
        
        self.init(domain: "vAmiga",
                  code: error.errorCode.rawValue,
                  userInfo: [NSLocalizedRecoverySuggestionErrorKey: error.what])
    }
}
