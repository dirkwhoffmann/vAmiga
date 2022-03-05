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
    @IBOutlet weak var info1: NSTextField!
    @IBOutlet weak var info2: NSTextField!
    @IBOutlet weak var partitionPopup: NSPopUpButton!
    @IBOutlet weak var formatPopup: NSPopUpButton!
    @IBOutlet weak var exportButton: NSButton!
    
    // Panel for exporting files
    var savePanel: NSSavePanel!
    
    // Panel for exporting directories
    var openPanel: NSOpenPanel!

    // Reference to the export drive
    var dfn: FloppyDriveProxy?
    var dhn: HardDriveProxy?

    // The partition to export
    var partition: Int?

    // Number of available partitions
    var numPartitions: Int { return hdf?.numPartitions ?? 1 }
    
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
        
        dfn = amiga.df(nr)

        // Run the ADF decoder
        adf = try? ADFFileProxy.make(drive: dfn!) as ADFFileProxy

        // Run the extended ADF decoder
        ext = try? EXTFileProxy.make(drive: dfn!) as EXTFileProxy

        // Run the DOS decoder
        img = try? IMGFileProxy.make(drive: dfn!) as IMGFileProxy
                        
        // Select the export partition
        select(partition: 0)
        
        super.showSheet()
    }

    func showSheet(hardDrive nr: Int) {
        
        track()
        
        dhn = amiga.hd(nr)

        // Run the HDF decoder
        hdf = try? HDFFileProxy.make(hdr: dhn!) as HDFFileProxy
                                
        // Select the export partition
        select(partition: numPartitions == 1 ? 0 : nil)
        
        super.showSheet()
    }
    
    func select(partition nr: Int?) {

        if hdf != nil && nr != nil {

            // Try to decode the file system from the HDF
            vol = try? FileSystemProxy.make(withHDF: hdf!, partition: nr!)
        
        } else if adf != nil {

            // Try to decode the file system from the ADF
            vol = try? FileSystemProxy.make(withADF: adf!)

        } else {
               
            // Exporting to a folder is not offered
            vol = nil
        }
    }
    
    func updatePartitionPopup() {
        
        func addItem(_ title: String, tag: Int) {
            
            partitionPopup.addItem(withTitle: title)
            partitionPopup.lastItem?.tag = tag
        }

        partitionPopup.autoenablesItems = false
        partitionPopup.removeAllItems()

        if hdf != nil {

            if numPartitions > 1 {
                addItem("All partitions", tag: -1)
            }
            for i in 1...numPartitions {
                addItem("Partition \(i)", tag: i - 1)
            }
        }
        if adf != nil {

            addItem("Entire disk", tag: -1)
        }
    }
    
    func updateFormatPopup() {
        
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
        if vol != nil { addItem("Folder", tag: Format.vol) }
    }
        
    override public func awakeFromNib() {

        track()
        super.awakeFromNib()
        updatePartitionPopup()
        updateFormatPopup()
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
        updateDiskInfo()
        updateFileSystemInfo()
    }
    
    func updateIcon() {
            
        track("formatPopup.selectedTag() = \(formatPopup.selectedTag())")
        
        switch formatPopup.selectedTag() {

        case Format.hdf:
            
            icon.image = hdf!.icon()
            
        case Format.adf, Format.ext, Format.img, Format.ima:
            
            let wp = dfn!.hasProtectedDisk
            
            icon.image =
            adf?.icon(protected: wp) ??
            img?.icon(protected: wp) ??
            ext?.icon(protected: wp) ?? nil

        case Format.vol:
            
            icon.image = NSImage(named: "NSFolder")
                        
        default:
            
            icon.image = nil
        }
            
        if icon.image == nil {
            icon.image = NSImage(named: "biohazard")
        }
    }
    
    func updateTitleText() {
                
        title.stringValue =
        hdf != nil ? "Amiga Hard Drive" :
        adf != nil ? "Amiga Floppy Disk" :
        ext != nil ? "Extended Amiga Disk" :
        img != nil ? "PC Disk" : "Unrecognized device"
    }

    func updateDiskInfo() {
        
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

        info1.stringValue = text
        info1.textColor = color
    }
    
    func updateFileSystemInfo() {
        
        var text = "No compatible file system"
        var color = NSColor.warningColor
        
        if vol != nil {
            
            text = vol!.dos.description
            color = .secondaryLabelColor
        }
        
        info2.stringValue = text
        info2.textColor = color
    }

    //
    // Action methods
    //
    
    @IBAction func formatAction(_ sender: NSPopUpButton!) {
        
        update()
    }

    @IBAction func partitionAction(_ sender: NSButton!) {

        track("Partition: \(partitionPopup.selectedTag())")
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
                    self.export(url: url)
                }
            }
        })
    }
    
    func export(url: URL) {

        if hdf != nil {
            exportHardDisk(url: url)
        } else {
            exportFloppyDisk(url: url)
        }
    }
    
    func exportFloppyDisk(url: URL) {

        track("url = \(url)")

        do {
            
            switch formatPopup.selectedTag() {

            case Format.adf:
                track("Exporting ADF")
                try parent.mydocument.export(fileProxy: adf!, to: url)

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
            
            dfn!.markDiskAsUnmodified()
            myAppDelegate.noteNewRecentlyExportedDiskURL(url, drive: dfn!.nr)
            
            hideSheet()

        } catch let error as VAError {
            error.warning("Cannot export floppy disk")
        } catch {
            fatalError()
        }
    }
    
    func exportHardDisk(url: URL) {
        
        track("url = \(url)")

        do {
            
            track("Exporting HDF")
            try parent.mydocument.export(fileProxy: hdf!, to: url)
            
            dhn!.markDiskAsUnmodified()
            myAppDelegate.noteNewRecentlyExportedHdrURL(url, drive: dhn!.nr)
            
            hideSheet()
            
        } catch let error as VAError {
            error.warning("Cannot export hard disk")
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
