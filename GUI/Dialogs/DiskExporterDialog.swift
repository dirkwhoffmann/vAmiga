// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class DiskExporterDialog: DialogController {

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
    var dfn: DriveProxy { return amiga.df(nr)! }
    var dhn: HardDriveProxy { return amiga.dh(nr)! }

    // Results of the different decoders
    var hdf: HDFFileProxy?
    var adf: ADFFileProxy?
    var ext: EXTFileProxy?
    var img: IMGFileProxy?
    var vol: FSDeviceProxy?

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
        if adf != nil { vol = try? FSDeviceProxy.make(withADF: adf!) }

        super.showSheet()
    }

    func showSheet(hardDrive nr: Int) {
        
        track()
        
        self.nr = nr

        /*
        // Run the HDF decoder
        hdf = try? HDFFileProxy.make(drive: dhn) as HDFFileProxy
                        
        // Try to decode the file system from the HDF
        if hdf != nil { vol = try? FSDeviceProxy.make(withHDF: hdf!) }
        */
        
        super.showSheet()
    }
    
    override public func awakeFromNib() {
        
        track()
        super.awakeFromNib()
        
        update()
    }

    override func windowDidLoad() {
                    
        func addItem(_ title: String, tag: Int) {
            
            formatPopup.addItem(withTitle: title)
            formatPopup.lastItem?.tag = tag
        }
        
        formatPopup.autoenablesItems = false
        formatPopup.removeAllItems()

        if adf != nil { addItem("ADF", tag: Format.adf) }
        if hdf != nil { addItem("HDF", tag: Format.hdf) }
        if ext != nil { addItem("ADF", tag: Format.ext) }
        if img != nil { addItem("IMG", tag: Format.img) }
        if img != nil { addItem("IMA", tag: Format.ima) }
        if vol != nil { addItem("Directory", tag: Format.vol) }

        if formatPopup.numberOfItems > 0 {

            exportButton.isEnabled = true
            formatPopup.selectItem(at: 0)

        } else {
                
            exportButton.isEnabled = false
        }
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

        if hdf != nil {
            
            name = "hdf"
            
        } else {
            
            if ext != nil { name = isHD ? "hd_other" : "dd_other" }
            if adf != nil { name = isHD ? "hd_adf" : "dd_adf" }
            if img != nil { name = "dd_dos" }
                
            if dfn.hasWriteProtectedDisk() { name += "_protected" }
        }
        
        track("name = \(name)")
        icon.image = NSImage(named: name)
        if icon.image == nil { track(); icon.image = NSImage(named: "biohazard") }
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
    // Exporting
    //
    
    func exportToFile(allowedTypes: [String]) {
     
        savePanel = NSSavePanel()
        savePanel.prompt = "Export"
        savePanel.title = "Export"
        savePanel.nameFieldLabel = "Export As:"
        savePanel.canCreateDirectories = true
        
        savePanel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = self.savePanel.url {
                    track("url = \(url)")
                    self.exportToFile(url: url)
                }
            }
        })
    }

    func exportToFile(url: URL) {

        track("url = \(url)")
        
        do {
            
            switch formatPopup.selectedTag() {

            case Format.adf:
                track("Exporting ADF")
                try parent.mydocument.export(diskFileProxy: adf!, to: url)

            case Format.hdf:
                track("Exporting HDF")
                // try parent.mydocument.export(diskFileProxy: hdf!, to: url)

            case Format.ext:
                track("Exporting Extended ADF")
                try parent.mydocument.export(diskFileProxy: ext!, to: url)

            case Format.img:
                track("Exporting IMG")
                try parent.mydocument.export(diskFileProxy: img!, to: url)

            case Format.ima:
                track("Exporting IMA")
                try parent.mydocument.export(diskFileProxy: img!, to: url)

            default:
                fatalError()
            }

            if hdf != nil {

                dhn.modified = false
                myAppDelegate.noteNewRecentlyExportedHdrURL(url, drive: nr)

            } else {

                dfn.modified = false
                myAppDelegate.noteNewRecentlyExportedDiskURL(url, drive: nr)
            }
            hideSheet()

        } catch let error as VAError {
            error.warning("Cannot export disk")
        } catch {
            fatalError()
        }
    }

    func exportToDirectory() {

        openPanel = NSOpenPanel()
        openPanel.prompt = "Export"
        openPanel.title = "Export"
        openPanel.nameFieldLabel = "Export As:"
        openPanel.canChooseDirectories = true
        openPanel.canCreateDirectories = true
        
        openPanel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = self.openPanel.url {
                    track("url = \(url)")
                    self.openPanel.close()
                    self.exportToDirectory(url: url)
                }
            }
        })
    }
    
    func exportToDirectory(url: URL) {
        
        track("url = \(url)")
        
        do {
            try vol!.export(url: url)
            hideSheet()

        } catch let error as VAError {
            error.warning("Failed to export disk.")
        } catch {
            fatalError()
        }
    }

    //
    // Action methods
    //

    @IBAction func exportAction(_ sender: NSButton!) {
        
        switch formatPopup.indexOfSelectedItem {

        case Format.hdf: exportToFile(allowedTypes: ["hdf", "HDF"])
        case Format.adf: exportToFile(allowedTypes: ["adf", "ADF"])
        case Format.ext: exportToFile(allowedTypes: ["adf", "ADF"])
        case Format.img: exportToFile(allowedTypes: ["img", "IMG"])
        case Format.ima: exportToFile(allowedTypes: ["ima", "IMA"])
        case Format.vol: exportToDirectory()

        default: fatalError()
        }
    }
}
