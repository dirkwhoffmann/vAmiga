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
    @IBOutlet weak var trackInfo: NSTextField!
    @IBOutlet weak var volumeInfo: NSTextField!

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
    @IBOutlet weak var errorInfo: NSTextField!
    @IBOutlet weak var errorStepper: NSStepper!

    @IBOutlet weak var formatPopup: NSPopUpButton!
    @IBOutlet weak var exportButton: NSButton!
    
    var errorReport: FSErrorReport?
    var selection: Int?
    var selectedRow: Int? { return selection == nil ? nil : selection! / 16 }
    var selectedCol: Int? { return selection == nil ? nil : selection! % 16 }
    
    var savePanel: NSSavePanel!  // Used to export to files
    var openPanel: NSOpenPanel!  // Used to export to directories

    let shrinkedHeight = CGFloat(176)
    let expandedHeight = CGFloat(450)
    
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
    var isDD: Bool { return disk?.diskDensity == .DISK_DD }
    var isHD: Bool { return disk?.diskDensity == .DISK_HD }

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

    func updateTrackAndSectorInfo() {
        
        var text = "This disk contains un unknown track and sector format."
        
        if disk != nil {
            
            let n = numSides == 1 ? "Single" : "Double"
            let d = isHD ? "high" : isDD ? "double" : "single"
            
            text = "\(n) sided, "
            text += "\(d) density disk, "
            text += "\(numTracks) tracks with \(numSectors) sectors each"
        } else {
            trackInfo.textColor = .systemRed
        }

        trackInfo.stringValue = text
    }
    
    func updateVolumeInfo() {
        
        var text = "No compatible file system has been detected."
        
        if disk?.type == .FILETYPE_ADF {
            
            if volume != nil {
                
                switch volume!.type {
                case .OFS:      text = "Original File System (OFS)"
                case .OFS_INTL: text = "Original File System (OFS-INTL)"
                case .OFS_DC:   text = "Original File System (OFS-DC)"
                case .OFS_LNFS: text = "Original File System (OFS-LNFS)"
                case .FFS:      text = "Original File System (FFS)"
                case .FFS_INTL: text = "Original File System (FFS-INTL)"
                case .FFS_DC:   text = "Original File System (FFS-DC)"
                case .FFS_LNFS: text = "Original File System (FFS-LNFS)"
                default:        text = "Unknown file system"
                }

                if let errors = errorReport?.numErrors, errors > 0 {
                    
                    let blocks = errorReport!.numErroneousBlocks
                    let text2 = " with \(errors) errors in \(blocks) blocks"
                    volumeInfo.stringValue = "Corrupted " + text + text2
                    volumeInfo.textColor = .systemRed
                    errorInfo.stringValue = "\(blocks) corrupted blocks"
                    return
                }
            }
        }

        /*
        let attr1 = [NSAttributedString.Key.foregroundColor: NSColor.textColor]
        let attr2 = [NSAttributedString.Key.foregroundColor: NSColor.systemRed]
        let str1 = NSMutableAttributedString(string: text1, attributes: attr1)
        let str2 = NSMutableAttributedString(string: text2, attributes: attr2)
        str1.append(str2)
        */
        
        volumeInfo.stringValue = text
    }
    
    func updateBlockInfo() {
        
        if volume == nil {
            info1.stringValue = ""
            info2.stringValue = ""
            return
        }
        
        if selection == nil {
            updateBlockInfoUnselected()
        } else {
            updateBlockInfoSelected()
            updateErrorInfoSelected()
        }
    }
    
    func updateBlockInfoUnselected() {
        
        let type = volume!.blockType(_block)
        var text: String
        
        switch type {
        case .UNKNOWN_BLOCK:    text = "has an unknown block type"
        case .EMPTY_BLOCK:      text = "is empty"
        case .BOOT_BLOCK:       text = "is a Boot Block"
        case .ROOT_BLOCK:       text = "is a Root Block"
        case .BITMAP_BLOCK:     text = "is a Bitmap Block"
        case .USERDIR_BLOCK:    text = "is a User Directory Block"
        case .FILEHEADER_BLOCK: text = "is a File Header Block"
        case .FILELIST_BLOCK:   text = "is a File List Block"
        case .DATA_BLOCK:       text = "is a Data Block"
        default: fatalError()
        }
        
        info1.stringValue = "This block " + text
    }
    
    func updateBlockInfoSelected() {
        
        let usage = volume!.itemType(_block, pos: selection!)
        var text: String
        
        switch usage {
        case .FSI_UNKNOWN:
            text = "Unknown"
        case .FSI_UNUSED:
            text = "Unused"
        case .FSI_DOS_HEADER:
            text = "AmigaDOS header signature"
        case .FSI_DOS_VERSION:
            text = "AmigaDOS version number"
        case .FSI_BOOTCODE:
            text = "Boot code instruction"
        case .FSI_TYPE_ID:
            text = "Type identifier"
        case .FSI_SUBTYPE_ID:
            text = "Subtype identifier"
        case .FSI_SELF_REF:
            text = "Block reference to itself"
        case .FSI_CHECKSUM:
            text = "Checksum"
        case .FSI_HASHTABLE_SIZE:
            text = "Hashtable size"
        case .FSI_HASH_REF:
            text = "Hashtable reference"
        case .FSI_PROT_BITS:
            text = "Protection status bits"
        case .FSI_NAME:
            text = "Part of the name string"
        case .FSI_COMMENT:
            text = "Part of the comment string"
        case .FSI_CREATED:
            text = "Part of the creation date"
        case .FSI_MODIFIED:
            text = "Part of th modification date"
        case .FSI_NEXT_HASH_REF:
            text = "Reference to the next block with the same hash"
        case .FSI_PARENT_DIR_REF:
            text = "Reference to the parent directory"
        case .FSI_FILEHEADER_REF:
            text = "Reference to the file header block"
        case .FSI_EXTBLOCK_REF:
            text = "Reference to the next extension block"
        case .FSI_BLOCK_COUNT:
            text = "Number of entries in the block reference table"
        case .FSI_FILESIZE_COUNT:
            text = "File size"
        case .FSI_DATA_BLOCK_NUMBER:
            text = "Position in the data block chain"
        case .FSI_DATA_BLOCK_REF:
            text = "Reference to a data block"
        case .FSI_FIRST_DATABLOCK_REF:
            text = "Reference to the first data block of this file"
        case .FSI_DATA_COUNT:
            text = "Number of bytes stored in this data block"
        case .FSI_DATA:
            text = "Data byte"
        case .FSI_BITMAP:
            text = "Block allocation bits"
        default:
            fatalError()
        }
        
        info1.stringValue = text
    }
    
    func updateErrorInfoSelected() {
        
        let error = volume!.check(_block, pos: selection!)
        let value = disk!.readByte(_block, offset: selection!)
        var text: String

        switch error {
        case .OK:
            text = ""
        case .BLOCK_TYPE_ID_MISMATCH:
            text = "Invalid block type"
        case .BLOCK_SUBTYPE_ID_MISMATCH:
            text = "Invalid type indentifer"
        case .BLOCK_EXPECTED_DOS_HEADER:
            text = "The signature doesn't match 'DOS'"
        case .BLOCK_INVALID_DOS_VERSION:
            text = "\(value) is not a valid DOS version number"
        case .BLOCK_MISSING_SELFREF:
            text = "Expected a self-reference"
        case .BLOCK_MISSING_FILEHEADER_REF:
            text = "Expected a reference to a file header block"
        case .BLOCK_NO_DATABLOCK_REF:
            text = "Expected a reference to a data block"
        case .BLOCK_MISSING_DATABLOCK_NUMBER:
            text = "This is not a valid data block number"
        case .BLOCK_HASHTABLE_SIZE_MISMATCH:
            text = "Invalid size"
        case .BLOCK_REF_MISSING:
            text = "Expected a block reference"
        case .BLOCK_UNEXPECTED_REF:
            text = "Unexpected block reference"
        case .BLOCK_REF_OUT_OF_RANGE:
            text = "Block reference is out of range"
        case .BLOCK_REF_TYPE_MISMATCH:
            text = "The referenced block has an invalid type"
        case .BLOCK_VALUE_TOO_LARGE:
            text = "Value is too large"
        case .EXPECTED_00:
            text = "Expected $00"
        case .EXPECTED_FF:
            text = "Expected $FF"
        case .BLOCK_CHECKSUM_ERROR:
            text = "Invalid block checksum"
        default:
            fatalError()
        }
        
        info2.stringValue = text
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
        errorReport = volume?.check()
        /*
        if errorReport != nil {
                    
            let errors = errorReport!.numErrors
            let blocks = errorReport!.numErroneousBlocks
            
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
        */
        
        update()
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
        r.origin.y = 87
        previewScrollView.frame = r

        update()
    }

    func shrink() { setHeight(shrinkedHeight) }
    func expand() { setHeight(expandedHeight) }

    func update() {
                
        // Update icon and text fields
        diskIcon.image = diskIconImage
        title.stringValue = titleText
        updateTrackAndSectorInfo()
        updateVolumeInfo()
        if volume == nil { volumeInfo.textColor = .red }

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
            blockText, blockField, blockStepper,
            errorInfo, errorStepper
        ]
        for item in items { item.isHidden = shrinked }
                
        // Only proceed if the window is expanded
        if shrinked { return }
        
        // Hide more elements if no errors are present
        if errorReport?.numErrors == 0 {
            errorInfo.isHidden = true
            errorStepper.isHidden = true
        }

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

        updateBlockInfo()
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
    
    @IBAction func errorStepperAction(_ sender: NSStepper!) {

        if selection == nil { return }

        var row = selectedRow!
        var col = selectedCol!
        
        if volume?.prevErrorLocation(&row, pos: &col) == true {
            track("row = \(row) col = \(col)")
        }
        
        track()
    }
    
    @IBAction func disclosureAction(_ sender: NSButton!) {
        
        shrinked ? expand() : shrink()
    }
    
    @IBAction func clickAction(_ sender: NSTableView!) {
        
        if sender.clickedColumn > 0 {
            
            let newValue = 16 * sender.clickedRow + sender.clickedColumn - 1
            selection = selection != newValue ? newValue : nil
            update()
        }
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
    
    func columnNr(_ column: NSTableColumn?) -> Int? {
        
        return column == nil ? nil : Int(column!.identifier.rawValue)
    }
    
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
        
        if let col = columnNr(tableColumn) {

            if let byte = disk?.readByte(_block, offset: 16 * row + col) {
                return String(format: "%02X", byte)
            }
        } else {
            return String(format: "%X", row)
        }
        
        return ""
    }
}

extension ExporterDialog: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {

        let cell = cell as? NSTextFieldCell

        if let col = columnNr(tableColumn) {
            
            let error = volume?.check(_block, pos: 16 * row + col) ?? .OK
            
            if row == selectedRow && col == selectedCol {
                cell?.backgroundColor = error == .OK ? .selectedContentBackgroundColor : .red
                cell?.textColor = .white
            } else {
                cell?.backgroundColor = NSColor.controlAlternatingRowBackgroundColors[row % 2]
                cell?.textColor = error == .OK ? .labelColor : .systemRed
            }
        } else {
            cell?.backgroundColor = NSColor.windowBackgroundColor
        }
    }
    
    func tableView(_ tableView: NSTableView, shouldSelectRow row: Int) -> Bool {
        return false
    }
}
