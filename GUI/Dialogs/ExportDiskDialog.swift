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
    @IBOutlet weak var text1: NSTextField!
    @IBOutlet weak var text2: NSTextField!

    var df: DiskFileProxy? // DEPRECATED
    
    var adf: ADFFileProxy?
    var img: IMGFileProxy?
    var vol: FSVolumeProxy?
    
    var savePanel: NSSavePanel!

    func showSheet(forDrive nr: Int) {
        
        let proxy = amiga.df(nr)!
        
        // Create panel
        savePanel = NSSavePanel()
        savePanel.prompt = "Export"
        savePanel.title = "Export"
        savePanel.nameFieldLabel = "Export As:"
        savePanel.accessoryView = window?.contentView

        // Try to decode the disk with the ADF decoder
        adf = ADFFileProxy.make(withDrive: proxy)
        
        // It it's an ADF, try to extract the file system
        if adf != nil {
            vol = FSVolumeProxy.make(withADF: adf)
        }
        // If it's not an ADF, try the DOS decoder
        if vol == nil {
            img = IMGFileProxy.make(withDrive: proxy)
        }
        
        // Update text labels
        if adf != nil {
            selectFormat(0)
            text1.stringValue = "This disk has been identified as an Amiga disk."
            if vol != nil {
                text2.stringValue = "The File System has been parsed successfully."
            } else {
                text2.stringValue = "An unsupported or corrupted File System has been detected."
                text2.textColor = .red
            }
        }
        if img != nil {
            selectFormat(1)
            text1.stringValue = "This disk has been identifies as a DOS disk."
            text2.stringValue = ""
        }
        if adf == nil && img == nil {
            text1.stringValue = "This disk cannot be exported."
            text2.stringValue = "The MFM stream doesn't comply to the Amiga or MS-DOS format."
            text1.textColor = .red
            text2.textColor = .red
        }
        
        // Disable unsupported formats in the format selector popup
        button.autoenablesItems = false
        button.item(at: 0)!.isEnabled = adf != nil
        button.item(at: 1)!.isEnabled = img != nil
        button.item(at: 2)!.isEnabled = img != nil
        button.item(at: 3)!.isEnabled = vol != nil

        /*
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
        */
        
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
        button.needsDisplay = true
        
        switch tag {
        case 0:
            savePanel.allowedFileTypes = ["adf", "ADF"]
            // savePanel.canChooseDirectories = false
        case 1:
            savePanel.allowedFileTypes = ["img", "IMG"]
            // savePanel.canChooseDirectories = false
        case 2:
            savePanel.allowedFileTypes = ["ima", "IMA"]
            // savePanel.canChooseDirectories = false
        case 3:
            savePanel.allowedFileTypes = []
            // savePanel.canChooseDirectories = true
        default:
            fatalError()
        }
    }
}
