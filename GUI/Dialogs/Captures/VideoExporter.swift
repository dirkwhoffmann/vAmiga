// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
class VideoExporter: DialogController {

    @IBOutlet weak var text: NSTextField!
    @IBOutlet weak var duration: NSTextField!
    @IBOutlet weak var sizeOnDisk: NSTextField!
    @IBOutlet weak var frameRate: NSTextField!
    @IBOutlet weak var bitRate: NSTextField!
    @IBOutlet weak var sampleRate: NSTextField!
    @IBOutlet weak var progress: NSProgressIndicator!
    @IBOutlet weak var cancelButton: NSButton!
    @IBOutlet weak var exportButton: NSButton!
    @IBOutlet weak var icon: NSImageView!

    var panel: NSSavePanel!

    let name = "vAmiga.mp4"
    let path: URL

    public override init(window: NSWindow?) {

        path = URL(fileURLWithPath: NSTemporaryDirectory()).appendingPathComponent(name)
        super.init(window: window)
    }

    public required init?(coder: NSCoder) {
        
        path = URL(fileURLWithPath: NSTemporaryDirectory()).appendingPathComponent(name)
        super.init(coder: coder)
    }
        
    override func showAsSheet(completionHandler handler: (() -> Void)? = nil) {
            
        super.showAsSheet()

        duration.stringValue = ""
        sizeOnDisk.stringValue = ""
        frameRate.stringValue = ""
        bitRate.stringValue = ""
        sampleRate.stringValue = ""
        progress.startAnimation(self)
            
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.25) {
            self.text.isHidden = false
        }

        debug(.exec, "Export to \(path)")
        if emu.recorder.export(as: path.absoluteString) {
                        
            let config = emu.recorder.config
            
            text.stringValue = "MPEG-4 Video Stream"
            icon.isHidden = false
            exportButton.isHidden = false
            sizeOnDisk.stringValue = path.fileSizeString
            duration.stringValue = String(format: "%.1f sec", emu.recorder.duration)
            frameRate.stringValue = "\(config.frameRate) Hz"
            bitRate.stringValue = "\(config.bitRate) kHz"
            sampleRate.stringValue = "\(config.sampleRate) Hz"

        } else {
            
            text.stringValue = "Encoding error"
            text.textColor = .warning
        }
        
        cancelButton.isHidden = false
        progress.stopAnimation(self)
        progress.isHidden = true
    }
    
    @IBAction func exportAction(_ sender: NSButton!) {
        
        // Create save panel
        panel = NSSavePanel()
        panel.allowedContentTypes = [.mpeg4Movie]
        
        // Run panel as sheet
        if let win = window {

            panel.beginSheetModal(for: win, completionHandler: { result in
                if result == .OK {
                    if let url = self.panel.url {
                        FileManager.copy(from: self.path, to: url)
                    }
                }
            })
        }
    }
}

extension VideoExporter: NSFilePromiseProviderDelegate {
    
    func filePromiseProvider(_ filePromiseProvider: NSFilePromiseProvider, fileNameForType fileType: String) -> String {
        
        return name
    }
    
    func filePromiseProvider(_ filePromiseProvider: NSFilePromiseProvider, writePromiseTo url: URL, completionHandler: @escaping (Error?) -> Void) {
                
        do {
            if FileManager.default.fileExists(atPath: url.path) {
                try FileManager.default.removeItem(at: url)
            }
            try FileManager.default.copyItem(at: path, to: url)
            completionHandler(nil)
            
        } catch let error {
            print("Failed to copy \(path) to \(url): \(error)")
            completionHandler(error)
        }
    }
}
