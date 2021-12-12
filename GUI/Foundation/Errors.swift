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

public func track(_ message: String = "",
                  path: String = #file, function: String = #function, line: Int = #line ) {
    
    if let file = URL(string: path)?.deletingPathExtension().lastPathComponent {
        if message == "" {
            print("\(file).\(line)::\(function)")
        } else {
            print("\(file).\(line)::\(function): \(message)")
        }
    }
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
    
    func notReady(async: Bool = false) {
        track("what = \(what)")
        informational("Configuration error", async: async, icon: "pref_transparent")
    }
    
    static func recordingAborted(async: Bool = false) {
        
        warning("Screen recording has been aborted.",
                "Failed to write to the FFmpeg pipes.",
                async: async, icon: "mp4")
    }

    static func gdbServerAborted(async: Bool = false) {
        
        warning("Connection lost.",
                "The GDB server has terminated unexpectedly.",
                async: async, icon: "server")
    }    
}
