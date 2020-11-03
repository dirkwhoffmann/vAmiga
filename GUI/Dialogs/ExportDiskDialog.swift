// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class ExportDiskDialog: DialogController {

    @IBOutlet weak var button: NSPopUpButton!

    var df: DiskFileProxy?
    var savePanel: NSSavePanel!

    func showSheet(forDrive nr: Int) {
        
        let proxy = amiga.df(nr)!
        
        // Create save panel
        savePanel = NSSavePanel()
        savePanel.prompt = "Export"
        savePanel.title = "Export"
        savePanel.nameFieldLabel = "Export As:"
        savePanel.accessoryView = window?.contentView

        // Try to decode the disk with the ADF decoder
        if df == nil {
            df = ADFFileProxy.make(withDrive: proxy)
            if df != nil { selectFormat(0) }
        }
        // Try to decode the disk with the DOS decoder if the ADF decoder failed
        if df == nil {
            df = IMGFileProxy.make(withDrive: proxy)
            if df != nil { selectFormat(1) }
        }

        // Abort if both decoders failed
        if df == nil {
            parent.mydocument.showExportDecodingAlert(driveNr: nr)
            return
        }

        // Run panel as sheet
        if let win = parent.window {
            savePanel.beginSheetModal(for: win, completionHandler: { result in
                if result == .OK {
                    if let url = self.savePanel.url {
                        track("url = \(url)")
                        self.parent.mydocument.export(drive: nr,
                                                      to: url,
                                                      diskFileProxy: self.df!)
                    }
                }
            })
        }
    }
    
    @IBAction func selectFormatAction(_ sender: NSMenuItem!) {
        
        selectFormat(sender.tag)
    }
    
    func selectFormat(_ tag: Int) {
        
        button.selectItem(withTag: tag)
        button.autoenablesItems = false
        button.item(at: 0)!.isEnabled = tag == 0
        button.item(at: 1)!.isEnabled = tag == 1 || tag == 2
        button.item(at: 2)!.isEnabled = tag == 1 || tag == 2
        button.needsDisplay = true
        switch tag {
        case 0: savePanel.allowedFileTypes = ["adf", "ADF"]
        case 1: savePanel.allowedFileTypes = ["img", "IMG"]
        case 2: savePanel.allowedFileTypes = ["ima", "IMA"]
        default: fatalError()
        }
    }
}
