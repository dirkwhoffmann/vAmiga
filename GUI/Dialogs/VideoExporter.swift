// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

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

    let path = "/tmp/vAmiga.mp4"
    
    override func showSheet(completionHandler handler: (() -> Void)? = nil) {
            
        track()
        super.showSheet()

        duration.stringValue = ""
        sizeOnDisk.stringValue = ""
        frameRate.stringValue = ""
        bitRate.stringValue = ""
        sampleRate.stringValue = ""
        progress.startAnimation(self)
            
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.25) {
            self.text.isHidden = false
        }

        if amiga.recorder.export(as: path) {
                        
            text.stringValue = "MPEG-4 Video Stream"
            icon.isHidden = false
            exportButton.isHidden = false
            sizeOnDisk.stringValue = URL(fileURLWithPath: path).fileSizeString
            duration.stringValue = String(format: "%.1f sec", amiga.recorder.duration)
            frameRate.stringValue = "\(amiga.recorder.frameRate) Hz"
            bitRate.stringValue = "\(amiga.recorder.bitRate) kHz"
            sampleRate.stringValue = "\(amiga.recorder.sampleRate) Hz"

        } else {
            
            text.stringValue = "Encoding error"
            text.textColor = .warningColor
        }
        
        cancelButton.isHidden = false
        progress.stopAnimation(self)
        progress.isHidden = true
    }
    
    @IBAction func exportAction(_ sender: NSButton!) {

        track()
        
        // Create save panel
        panel = NSSavePanel()
        panel.allowedFileTypes = ["mp4"]
        
        // Run panel as sheet
        if let win = window {
            track()
            panel.beginSheetModal(for: win, completionHandler: { result in
                if result == .OK {
                    track()
                    if let url = self.panel.url {
                        track("url = \(url)")
                        let source = URL(fileURLWithPath: self.path)
                        FileManager.copy(from: source, to: url)
                    }
                }
            })
        }
    }
}

extension VideoExporter: NSFilePromiseProviderDelegate {
   
    func filePromiseProvider(_ filePromiseProvider: NSFilePromiseProvider, fileNameForType fileType: String) -> String {
        
        return "vAmiga.mp4"
    }
    
    func filePromiseProvider(_ filePromiseProvider: NSFilePromiseProvider, writePromiseTo url: URL, completionHandler: @escaping (Error?) -> Void) {
        
        track("\(url)")

        let source = URL(fileURLWithPath: "/tmp/vAmiga.mp4")
        
        do {
            if FileManager.default.fileExists(atPath: url.path) {
                try FileManager.default.removeItem(at: url)
            }
            try FileManager.default.copyItem(at: source, to: url)
            completionHandler(nil)
            
        } catch let error {
            print("Failed to copy \(source) to \(url): \(error)")
            completionHandler(error)
        }
    }
}
