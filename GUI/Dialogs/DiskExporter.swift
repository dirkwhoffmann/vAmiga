// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class DiskExporter: DialogController {

    enum Format {
        
        static let adf = 0
        static let hdf = 1
        static let ext = 2
        static let img = 3
        static let ima = 4
        static let vol = 5
    }

    var myDocument: MyDocument { return parent.mydocument! }

    @IBOutlet weak var icon: NSImageView!
    @IBOutlet weak var title: NSTextField!
    @IBOutlet weak var layoutInfo: NSTextField!
    @IBOutlet weak var volumeInfo: NSTextField!
    @IBOutlet weak var bootInfo: NSTextField!
    @IBOutlet weak var formatPopup: NSPopUpButton!
    @IBOutlet weak var exportButton: NSButton!
    
    // Panel for exporting files
    var savePanel: NSSavePanel!
    
    // Panel for exporting directories
    var openPanel: NSOpenPanel!

    var nr = -1
    var dfn: FloppyDriveProxy { return amiga.df(nr)! }
    var dhn: HardDriveProxy { return amiga.hd(nr)! }

    // Results of the different decoders
    var hdf: HDFFileProxy?
    var adf: ADFFileProxy?
    var ext: EXTFileProxy?
    var img: IMGFileProxy?
    var vol: FileSystemProxy?

    // Returns true if a DD or a HD floppy disk is to be exported
    var isDD: Bool { return adf?.isDD ?? img?.isDD ?? ext?.isDD ?? false }
    var isHD: Bool { return adf?.isHD ?? img?.isHD ?? ext?.isHD ?? false }
    
    func showSheet(diskDrive nr: Int) {
        
        track()
        
        self.nr = nr

        // Run the ADF decoder
        adf = try? ADFFileProxy.make(drive: dfn) as ADFFileProxy

        // Run the extended ADF decoder
        ext = try? EXTFileProxy.make(drive: dfn) as EXTFileProxy

        // Run the DOS decoder
        img = try? IMGFileProxy.make(drive: dfn) as IMGFileProxy
                        
        // Try to decode the file system from the ADF
        if adf != nil { vol = try? FileSystemProxy.make(withADF: adf!) }

        super.showSheet()
    }

    func showSheet(hardDrive nr: Int) {
        
        track()
        
        self.nr = nr

        // Run the HDF decoder
        hdf = try? HDFFileProxy.make(hdr: dhn) as HDFFileProxy
                        
        // Try to decode the file system from the HDF
        if hdf != nil { vol = try? FileSystemProxy.make(withHDF: hdf!, partition: 0) }
        
        super.showSheet()
    }
    
    override public func awakeFromNib() {
        
        func addItem(_ title: String, tag: Int) {
            
            formatPopup.addItem(withTitle: title)
            formatPopup.lastItem?.tag = tag
        }

        track()
        super.awakeFromNib()
                
        formatPopup.autoenablesItems = false
        formatPopup.removeAllItems()

        if adf != nil { addItem("ADF", tag: Format.adf) }
        if hdf != nil { addItem("HDF", tag: Format.hdf) }
        if ext != nil { addItem("ADF", tag: Format.ext) }
        if img != nil { addItem("IMG", tag: Format.img) }
        if img != nil { addItem("IMA", tag: Format.ima) }
        if vol != nil { addItem("Files", tag: Format.vol) }

        if formatPopup.numberOfItems > 0 {

            exportButton.isEnabled = true
            formatPopup.selectItem(at: 0)

        } else {
                
            exportButton.isEnabled = false
        }
    
        update()
    }

    override func windowDidLoad() {
                    
    }
    
    override func sheetDidShow() {
        
    }
    
    func update() {
          
        // Update icons
        updateIcon()

        // Update disk description
        updateTitleText()
        updateTrackAndSectorInfo()
        updateVolumeInfo()
        updateBootInfo()
    }
    
    func updateIcon() {
            
        var name = ""

        track("formatPopup.selectedTag() = \(formatPopup.selectedTag())")
        
        switch formatPopup.selectedTag() {

        case Format.hdf:
            
            name = "hdf"
            
        case Format.adf, Format.ext, Format.img, Format.ima:
            
            if ext != nil { name = isHD ? "hd_other" : "dd_other" }
            if adf != nil { name = isHD ? "hd_adf" : "dd_adf" }
            if img != nil { name = "dd_dos" }
                
            if name != "" && dfn.hasProtectedDisk { name += "_protected" }

        case Format.vol:
            
            name = "NSFolder"
            
        default:
            
            name = ""
        }
                
        icon.image = NSImage(named: name != "" ? name : "biohazard")
    }
    
    func updateTitleText() {
                
        title.stringValue =
        hdf != nil ? "Amiga Hard Drive" :
        adf != nil ? "Amiga Disk" :
        ext != nil ? "Extended Amiga Disk" :
        img != nil ? "PC Disk" : "Unrecognized device"
    }

    func updateTrackAndSectorInfo() {
        
        var text = "Unknown track and sector format"
        var color = NSColor.warningColor
        
        if hdf != nil {
            
            text = hdf!.layoutInfo
            color = NSColor.secondaryLabelColor
            
        } else if adf != nil {
            
            text = adf!.layoutInfo
            color = NSColor.secondaryLabelColor
            
        } else if img != nil {
            
            text = img!.layoutInfo
            color = NSColor.secondaryLabelColor
        }

        layoutInfo.stringValue = text
        layoutInfo.textColor = color
    }
    
    func updateVolumeInfo() {
        
        var text = "No compatible file system"
        var color = NSColor.warningColor
        
        if vol != nil {
            
            text = vol!.dos.description
            color = .secondaryLabelColor
        }
        
        volumeInfo.stringValue = text
        volumeInfo.textColor = color
    }
    
    func updateBootInfo() {
                
        if adf == nil {
            bootInfo.stringValue = ""
            return
        }
        
        bootInfo.stringValue = adf!.bootInfo
        bootInfo.textColor = adf!.hasVirus ? .warningColor : .secondaryLabelColor
    }

    //
    // Action methods
    //
    
    @IBAction func formatAction(_ sender: NSPopUpButton!) {
        
        update()
    }
    
    @IBAction func exportAction(_ sender: NSButton!) {
        
        switch formatPopup.selectedTag() {

        case Format.hdf: openExportToFilePanel(allowedTypes: ["hdf", "HDF"])
        case Format.adf: openExportToFilePanel(allowedTypes: ["adf", "ADF"])
        case Format.ext: openExportToFilePanel(allowedTypes: ["adf", "ADF"])
        case Format.img: openExportToFilePanel(allowedTypes: ["img", "IMG"])
        case Format.ima: openExportToFilePanel(allowedTypes: ["ima", "IMA"])
        case Format.vol: openExportToFolderPanel()

        default: fatalError()
        }
    }
    
    //
    // Exporting
    //
        
    func openExportToFilePanel(allowedTypes: [String]) {
     
        // TODO: allowedTypes is not used
        savePanel = NSSavePanel()
        savePanel.prompt = "Export"
        savePanel.title = "Export"
        savePanel.nameFieldLabel = "Export As:"
        savePanel.canCreateDirectories = true

        savePanel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = self.savePanel.url {
                    // self.savePanel.close()
                    self.export(url: url)
                }
            }
        })
    }

    func openExportToFolderPanel() {

        openPanel = NSOpenPanel()
        openPanel.prompt = "Export"
        openPanel.title = "Export"
        openPanel.nameFieldLabel = "Export As:"
        openPanel.canChooseDirectories = true
        openPanel.canCreateDirectories = true
        
        openPanel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = self.openPanel.url {
                    // self.openPanel.close()
                    self.export(url: url)
                }
            }
        })
    }
    
    func export(url: URL) {

        track("url = \(url)")
        
        do {
            
            switch formatPopup.selectedTag() {

            case Format.adf:
                track("Exporting ADF")
                try parent.mydocument.export(fileProxy: adf!, to: url)

            case Format.hdf:
                track("Exporting HDF")
                try parent.mydocument.export(fileProxy: hdf!, to: url)

            case Format.ext:
                track("Exporting Extended ADF")
                try parent.mydocument.export(fileProxy: ext!, to: url)

            case Format.img:
                track("Exporting IMG")
                try parent.mydocument.export(fileProxy: img!, to: url)

            case Format.ima:
                track("Exporting IMA")
                try parent.mydocument.export(fileProxy: img!, to: url)

            case Format.vol:
                track("Exporting file system")
                try vol!.export(url: url)
                
            default:
                fatalError()
            }

            if hdf != nil {

                dhn.markDiskAsUnmodified()
                myAppDelegate.noteNewRecentlyExportedHdrURL(url, drive: nr)

            } else {

                dfn.markDiskAsUnmodified()
                myAppDelegate.noteNewRecentlyExportedDiskURL(url, drive: nr)
            }
            hideSheet()

        } catch let error as VAError {
            error.warning("Cannot export disk")
        } catch {
            fatalError()
        }
    }
}

//
// Protocols
//

extension DiskExporter: NSFilePromiseProviderDelegate {
   
    func filePromiseProvider(_ filePromiseProvider: NSFilePromiseProvider, fileNameForType fileType: String) -> String {
        
        var name: String
        
        switch formatPopup.selectedTag() {
            
        case Format.hdf: name = "Untitled.hdf"
        case Format.adf: name = "Untitled.adf"
        case Format.ext: name = "Untitled.adf"
        case Format.img: name = "Untitled.img"
        case Format.ima: name = "Untitled.ima"
        case Format.vol: name = "Untitled"
            
        default: fatalError()
        }
        
        track("NSFilePromiseProviderDelegate: name = \(name)")
        return name
    }
    
    func filePromiseProvider(_ filePromiseProvider: NSFilePromiseProvider, writePromiseTo url: URL, completionHandler: @escaping (Error?) -> Void) {
        
        track("NSFilePromiseProviderDelegate: url = \(url)")
        export(url: url)
    }
}
