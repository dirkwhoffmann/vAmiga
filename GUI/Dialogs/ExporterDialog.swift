// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class ExporterDialog: DialogController {
        
    @IBOutlet weak var diskIcon: NSImageView!
    @IBOutlet weak var title: NSTextField!
    @IBOutlet weak var subtitle: NSTextField!
    @IBOutlet weak var warning: NSTextField!

    @IBOutlet weak var disclosureButton: NSButton!
    @IBOutlet weak var previewScrollView: NSScrollView!
    @IBOutlet weak var previewTable: NSTableView!
    @IBOutlet weak var cylinderText: NSTextField!
    @IBOutlet weak var cylinderField: NSTextField!
    @IBOutlet weak var cylinderStepper: NSStepper!
    @IBOutlet weak var headText: NSTextField!
    @IBOutlet weak var headField: NSTextField!
    @IBOutlet weak var headStepper: NSStepper!
    @IBOutlet weak var trackText: NSTextField!
    @IBOutlet weak var trackField: NSTextField!
    @IBOutlet weak var trackStepper: NSStepper!
    @IBOutlet weak var sectorText: NSTextField!
    @IBOutlet weak var sectorField: NSTextField!
    @IBOutlet weak var sectorStepper: NSStepper!
    @IBOutlet weak var blockText: NSTextField!
    @IBOutlet weak var blockField: NSTextField!
    @IBOutlet weak var blockStepper: NSStepper!

    @IBOutlet weak var info1: NSTextField!
    @IBOutlet weak var info2: NSTextField!
    
    @IBOutlet weak var formatPopup: NSPopUpButton!
    @IBOutlet weak var exportButton: NSButton!
    
    var savePanel: NSSavePanel!  // Used to export to files
    var openPanel: NSOpenPanel!  // Used to export to directories

    let shrinkedHeight = CGFloat(176)
    let expandedHeight = CGFloat(446)
    
    var myDocument: MyDocument { return parent.mydocument! }
    var size: CGSize { return window!.frame.size }
    var shrinked: Bool { return size.height < 300 }

    var drive: DriveProxy!
    var driveNr = 0
    var disk: DiskFileProxy?
    
    var numCylinders: Int { return disk?.numCylinders ?? 0 }
    var numSides: Int { return disk?.numSides ?? 0 }
    var numTracks: Int { return disk?.numTracks ?? 0 }
    var numSectors: Int { return disk?.numSectors ?? 0 }
    var numBlocks: Int { return disk?.numBlocks ?? 0 }

    // var type: AmigaFileType?
    var volume: FSVolumeProxy?
    
    // Block preview
    var _cylinder = 0
    var _side = 0
    var _track = 0
    var _sector = 0
    var _block = 0
    var sectorData: [String] = []
    let bytesPerRow = 32
    
    func setCylinder(_ newValue: Int) {

        if newValue >= 0 && newValue < numCylinders {
                        
            _cylinder = newValue
            _track    = _cylinder * 2 + _side
            _block    = _track * numSectors + _sector
        }
    }
    
    func setHead(_ newValue: Int) {
        
        if newValue >= 0 && newValue < numSides {
                        
            _side     = newValue
            _track    = _cylinder * 2 + _side
            _block    = _track * numSectors + _sector
        }
    }
    
    func setTrack(_ newValue: Int) {
        
        if newValue >= 0 && newValue < numTracks {
                        
            _track    = newValue
            _cylinder = _track / 2
            _side     = _track % 2
            _block    = _track * numSectors + _sector
        }
    }

    func setSector(_ newValue: Int) {
        
        if newValue >= 0 && newValue < numSectors {
                        
            _sector   = newValue
            _block    = _track * numSectors + _sector
        }
    }

    func setBlock(_ newValue: Int) {
        
        if newValue >= 0 && newValue < numBlocks {
                        
            _block    = newValue
            _track    = _block / numSectors
            _sector   = _block % numSectors
            _cylinder = _track / 2
            _side     = _track % 2
        }
    }

    var diskIconImage: NSImage? {
        
        var name: String!

        if drive.hasDDDisk { name = "dd" }
        if drive.hasHDDisk { name = "hd" }
        if name == nil { return nil }

        switch disk?.type {
        case .FILETYPE_ADF: name += "_adf"
        case .FILETYPE_IMG: name += "_dos"
        default: name += "_other"
        }
        
        if drive.hasWriteProtectedDisk() { name += "_protected" }

        return NSImage.init(named: name)
    }

    var titleText: String {
        
        if disk?.type == .FILETYPE_ADF { return "Amiga Disk" }
        if disk?.type == .FILETYPE_IMG { return "PC Disk" }

        return "This disk contains an unrecognized MFM stream"
    }

    var subtitleText: String {
        
        if disk != nil {
            
            let t = numTracks
            let s = numSectors
            let n = numSides == 1 ? "Single" : "Double"
            
            let den = disk!.diskDensity
            let d = den == .DISK_SD ? "single" : den == .DISK_DD ? "double" : "high"
            
            return "\(n) sided, \(d) density disk, \(t) tracks with \(s) sectors each"
        }

        return "None of the supported formats is able to store this disk"
    }
    
    var warningText: String {
        
        if disk?.type == .FILETYPE_ADF {
            if volume != nil {
                // let ofs = volume.
                return "OFS or FFS formatted"
            } else {
                return "No compatible file system detected"
            }
        }
        return ""
    }
    
    func showSheet(forDrive nr: Int) {
        
        track()
        
        drive = amiga.df(nr)!
        driveNr = nr
        
        // Try to decode the disk with the ADF decoder
        disk = ADFFileProxy.make(withDrive: drive)
        
        // It it is an ADF, try to extract the file system
        if disk != nil {
            volume = FSVolumeProxy.make(withADF: disk as? ADFFileProxy)
        }
        // If it is not an ADF, try the DOS decoder
        if disk == nil {
            disk = IMGFileProxy.make(withDrive: drive)
        }
        
        super.showSheet()
    }
    
    override public func awakeFromNib() {
        
        track()
        super.awakeFromNib()
        
        sectorData = Array(repeating: "", count: 512 / bytesPerRow)

        // doubleAction = #selector(doubleClickAction(_:))
        previewTable.action = #selector(clickAction(_:))

        // Start with a shrinked window
        var rect = window!.contentRect(forFrameRect: window!.frame)
        rect.size = CGSize(width: 606, height: shrinkedHeight)
        let frame = window!.frameRect(forContentRect: rect)
        window!.setFrame(frame, display: true)
        
        update()
    }
    
    override func windowDidLoad() {
        
        let adf = disk?.type == .FILETYPE_ADF
        let dos = disk?.type == .FILETYPE_IMG
            
        // Enable compatible file formats in the format selector popup
        formatPopup.autoenablesItems = false
        formatPopup.item(at: 0)!.isEnabled = adf
        formatPopup.item(at: 1)!.isEnabled = dos
        formatPopup.item(at: 2)!.isEnabled = dos
        formatPopup.item(at: 3)!.isEnabled = volume != nil
        
        // Preselect a suitable export format
        if adf { formatPopup.selectItem(at: 0) }
        if dos { formatPopup.selectItem(at: 1) }
        
        // Run a file system check
        if let report = volume?.check() {
            
            let errors = report.numErrors
            let blocks = report.numErroneousBlocks
            
            if errors == 0 {
                info1.stringValue = "The integrity of this volume has been verified successfully."
                info2.stringValue = "No errors found."
            } else {
                info1.stringValue = "The file system on this volume appears to be corrupt."
                info2.stringValue = "\(errors) errors in \(blocks) blocks"
                info1.textColor = .red
                info2.textColor = .red
            }

        } else {
            info1.stringValue = "No file system check has been performed."
            info2.stringValue = ""
        }
    }
    
    override func sheetDidShow() {
        
        // shrink()
    }
    
    func setHeight(_ newHeight: CGFloat) {
                
        var rect = window!.frame
        rect.origin.y += rect.size.height - newHeight
        rect.size.height = newHeight
        
        window!.setFrame(rect, display: true)

        // Force the preview table to appear at the correct vertical position
        var r = previewScrollView.frame
        r.origin.y = 81
        previewScrollView.frame = r

        update()
    }

    func shrink() { setHeight(shrinkedHeight) }
    func expand() { setHeight(expandedHeight) }

    func update() {
                
        // Update icon and text fields
        diskIcon.image = diskIconImage
        title.stringValue = titleText
        subtitle.stringValue = subtitleText
        warning.stringValue = warningText
        if volume == nil { warning.textColor = .red }

        // Disable the export button if no export is possible
        exportButton.isEnabled = disk != nil

        // Update the disclosure button state
        disclosureButton.state = shrinked ? .off : .on

        // Hide some elements if window is shrinked
        let items: [NSView] = [
            
            previewScrollView,
            cylinderText, cylinderField, cylinderStepper,
            headText, headField, headStepper,
            trackText, trackField, trackStepper,
            sectorText, sectorField, sectorStepper,
            blockText, blockField, blockStepper
        ]
        for item in items { item.isHidden = shrinked }
        
        // Only proceed if window is expanded
        if shrinked { return }
            
        // Update all elements
        cylinderField.integerValue   = _cylinder
        cylinderStepper.integerValue = _cylinder
        headField.integerValue       = _side
        headStepper.integerValue     = _side
        trackField.integerValue      = _track
        trackStepper.integerValue    = _track
        sectorField.integerValue     = _sector
        sectorStepper.integerValue   = _sector
        blockField.integerValue      = _block
        blockStepper.integerValue    = _block

        buildStrings()
        previewTable.reloadData()
    }
    
    //
    // Export functions
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
        parent.mydocument.export(drive: driveNr, to: url, diskFileProxy: disk!)
        hideSheet()
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
                    self.exportToDirectory(url: url)
                }
            }
        })

    }
    
    func exportToDirectory(url: URL) {
        
        track("url = \(url)")
        track("IMPLEMENTATION MISSING")
        hideSheet()
    }

    func export(url: URL) {
    
        track("url = \(url)")
        parent.mydocument.export(drive: driveNr, to: url, diskFileProxy: disk!)

        hideSheet()
    }

    //
    // Action methods
    //

    @IBAction func cylinderAction(_ sender: NSTextField!) {
        
        setCylinder(sender.integerValue)
        update()
    }
    
    @IBAction func cylinderStepperAction(_ sender: NSStepper!) {
        
        setCylinder(sender.integerValue)
        update()
    }
    
    @IBAction func headAction(_ sender: NSTextField!) {
        
        setHead(sender.integerValue)
        update()
    }
    
    @IBAction func headStepperAction(_ sender: NSStepper!) {
        
        setHead(sender.integerValue)
        update()
    }
    
    @IBAction func trackAction(_ sender: NSTextField!) {
        
        setTrack(sender.integerValue)
        update()
    }
    
    @IBAction func trackStepperAction(_ sender: NSStepper!) {
        
        setTrack(sender.integerValue)
        update()
    }
    
    @IBAction func sectorAction(_ sender: NSTextField!) {
        
        setSector(sender.integerValue)
        update()
    }
    
    @IBAction func sectorStepperAction(_ sender: NSStepper!) {
        
        setSector(sender.integerValue)
        update()
    }
    
    @IBAction func blockAction(_ sender: NSTextField!) {
        
        setBlock(sender.integerValue)
        update()
    }
    
    @IBAction func blockStepperAction(_ sender: NSStepper!) {
        
        setBlock(sender.integerValue)
        update()
    }
    
    @IBAction func disclosureAction(_ sender: NSButton!) {
        
        shrinked ? expand() : shrink()
    }
    
    @IBAction func clickAction(_ sender: NSTableView!) {

        track("row = \(sender.clickedRow) col = \(sender.clickedColumn)")
    }

    @IBAction func exportAction(_ sender: NSButton!) {
        
        track("selected item = \(formatPopup.indexOfSelectedItem)")
        
        switch formatPopup.indexOfSelectedItem {
        case 0: exportToFile(allowedTypes: ["adf", "ADF"])
        case 1: exportToFile(allowedTypes: ["img", "IMG"])
        case 2: exportToFile(allowedTypes: ["ima", "IMA"])
        case 3: exportToDirectory()
        default: fatalError()
        }
    }
        
    @IBAction override func cancelAction(_ sender: Any!) {
         
         track()
         hideSheet()
     }
}

extension ExporterDialog: NSWindowDelegate {
    
    func windowDidResize(_ notification: Notification) {
        
        track()
    }
    
    func windowWillStartLiveResize(_ notification: Notification) {
         
         track()
     }
     
     func windowDidEndLiveResize(_ notification: Notification) {
        
        track()
     }
}

extension ExporterDialog: NSTableViewDataSource {
    
    func buildHex(p: UnsafeMutablePointer<UInt8>, count: Int) -> String {
        
        let hexDigits = Array(("0123456789ABCDEF ").utf16)
        var chars: [unichar] = []
        chars.reserveCapacity(3 * count)
        
        for i in 1 ... count {
            
            let byte = p[i]
            chars.append(hexDigits[Int(byte / 16)])
            chars.append(hexDigits[Int(byte % 16)])
            chars.append(hexDigits[16])
        }
        
        return String(utf16CodeUnits: chars, count: chars.count)
    }
    
    func buildStrings() {
        
        track("Building strings for block \(_block)")
        
        let dst = UnsafeMutablePointer<UInt8>.allocate(capacity: 512)
        disk?.readSector(dst, block: _block)
        
        for i in 0 ..< numberOfRows(in: previewTable) {
            
            let str = buildHex(p: dst + i * bytesPerRow, count: bytesPerRow)
            sectorData[i] = str
        }
    }
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        
        return 512 / bytesPerRow
    }
    
    func tableView(_ tableView: NSTableView,
                   objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        if let id = tableColumn?.identifier.rawValue {

            var offset = 16 * row
            switch id {
            case "0": offset += 0
            case "1": offset += 1
            case "2": offset += 2
            case "3": offset += 3
            case "4": offset += 4
            case "5": offset += 5
            case "6": offset += 6
            case "7": offset += 7
            case "8": offset += 8
            case "9": offset += 9
            case "10": offset += 10
            case "11": offset += 11
            case "12": offset += 12
            case "13": offset += 13
            case "14": offset += 14
            case "15": offset += 15
            case "16": offset += 16
            default: fatalError()
            }
            
            assert(offset < 512)
            if let byte = disk?.readByte(_block, offset: row * 16 + offset) {
                return String(format: "%02X", byte)
            }
        }
        return ""
    }
}

extension ExporterDialog: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        let cell = cell as? NSTextFieldCell

        // REMOVE ASAP
        if (row == 4) {
            cell?.textColor = NSColor.systemRed
        } else {
            cell?.textColor = NSColor.labelColor
        }
    }
    
    func tableView(_ tableView: NSTableView, shouldSelectRow row: Int) -> Bool {
        return false
    }
}
