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
    var type: AmigaFileType = FILETYPE_ADF
    var savePanel: NSSavePanel!
    var selectedURL: URL?
    
    func showSheet(forDrive nr: Int) {
        
        assert(nr >= 0 && nr <= 3)
        
        // Create save panel
        savePanel = NSSavePanel()
        savePanel.allowedFileTypes = ["adf"]
        savePanel.prompt = "Export"
        savePanel.title = "Export"
        savePanel.nameFieldLabel = "Export As:"
        savePanel.accessoryView = window?.contentView

        // Run panel as sheet
        if let win = myWindow {
            savePanel.beginSheetModal(for: win, completionHandler: { result in
                if result == .OK {
                    myDocument?.export(drive: nr, to: self.savePanel.url)
                }
            })
        }
    }
    
    @IBAction func selectADF(_ sender: Any!) {
        track()
        savePanel.allowedFileTypes = ["adf"]
        type = FILETYPE_ADF
    }
}
