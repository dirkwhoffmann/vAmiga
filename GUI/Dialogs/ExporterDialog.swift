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
    @IBOutlet weak var corruptionText: NSTextField!
    @IBOutlet weak var corruptionField: NSTextField!
    @IBOutlet weak var corruptionStepper: NSStepper!

    @IBOutlet weak var info1: NSTextField!
    @IBOutlet weak var info2: NSTextField!

    @IBOutlet weak var strictButton: NSButton!
    @IBOutlet weak var formatPopup: NSPopUpButton!
    @IBOutlet weak var exportButton: NSButton!
    
    var driveNr = 0 // DEPRECATED
    var drive: DriveProxy!
    var disk: DiskFileProxy?
    var errorReport: FSErrorReport?
    
    var selection: Int?
    var selectedRow: Int? { return selection == nil ? nil : selection! / 16 }
    var selectedCol: Int? { return selection == nil ? nil : selection! % 16 }
    var strict: Bool { return strictButton.state == .on }
    
    var savePanel: NSSavePanel!  // Used to export to files
    var openPanel: NSOpenPanel!  // Used to export to directories

    let shrinkedHeight = CGFloat(176)
    let expandedHeight = CGFloat(450)
    
    var myDocument: MyDocument { return parent.mydocument! }
    var size: CGSize { return window!.frame.size }
    var shrinked: Bool { return size.height < 300 }
        
    var numCylinders: Int { return disk?.numCylinders ?? 0 }
    var numSides: Int { return disk?.numSides ?? 0 }
    var numTracks: Int { return disk?.numTracks ?? 0 }
    var numSectors: Int { return disk?.numSectors ?? 0 }
    var numBlocks: Int { return disk?.numBlocks ?? 0 }
    var isDD: Bool { return disk?.diskDensity == .DISK_DD }
    var isHD: Bool { return disk?.diskDensity == .DISK_HD }

    var volume: FSVolumeProxy?
    
    // Block preview
    var _cylinder = 0
    var _side = 0
    var _track = 0
    var _sector = 0
    var _block = 0
    var _corruption = 0
    
    var sectorData: [String] = []
    let bytesPerRow = 16
    
    //
    // Checking the file system
    //

    func checkVolume() {
                
        errorReport = volume?.check(strict)
        let errors = errorReport?.corruptedBlocks ?? 0
        
        corruptionStepper.minValue = Double(1)
        corruptionStepper.maxValue = Double(max(1, errors))
    }

    //
    // Selecting a block
    //
    
    func setCylinder(_ newValue: Int) {

        if newValue != _cylinder && newValue >= 0 && newValue < numCylinders {
                        
            _cylinder = newValue
            _track    = _cylinder * 2 + _side
            _block    = _track * numSectors + _sector
            
            selection = nil
            update()
        }
    }
    
    func setHead(_ newValue: Int) {
        
        if newValue != _side && newValue >= 0 && newValue < numSides {
                        
            _side     = newValue
            _track    = _cylinder * 2 + _side
            _block    = _track * numSectors + _sector
            
            selection = nil
            update()
        }
    }
    
    func setTrack(_ newValue: Int) {
        
        if newValue != _track && newValue >= 0 && newValue < numTracks {
                        
            _track    = newValue
            _cylinder = _track / 2
            _side     = _track % 2
            _block    = _track * numSectors + _sector

            selection = nil
            update()
        }
    }

    func setSector(_ newValue: Int) {
        
        if newValue != _sector && newValue >= 0 && newValue < numSectors {
                        
            _sector   = newValue
            _block    = _track * numSectors + _sector
            
            selection = nil
            update()
        }
    }

    func setBlock(_ newValue: Int) {
        
        if newValue != _block && newValue >= 0 && newValue < numBlocks {
                        
            _block    = newValue
            _track    = _block / numSectors
            _sector   = _block % numSectors
            _cylinder = _track / 2
            _side     = _track % 2
            
            selection = nil
            update()
        }
    }

    func setCorruptedBlock(_ newValue: Int) {
        
        guard let blockNr = volume?.seekCorruptedBlock(newValue) else { return }
        
        if newValue != _corruption && blockNr >= 0 && blockNr < numBlocks {
                        
            _corruption = newValue
            _block      = blockNr
            _track      = _block / numSectors
            _sector     = _block % numSectors
            _cylinder   = _track / 2
            _side       = _track % 2
            
            selection = nil
            update()
        }
    }
    
    //
    // Getting the ball rolling
    //
    
    func showSheet(forDrive nr: Int) {
        
        track()
        
        drive = amiga.df(nr)!
        driveNr = nr
        
        // Try to decode the disk with the ADF decoder
        disk = ADFFileProxy.make(withDrive: drive)
        
        // It it is an ADF, try to extract the file system
        if disk != nil { volume = FSVolumeProxy.make(withADF: disk as? ADFFileProxy) }
        
        // If it is not an ADF, try the DOS decoder
        if disk == nil { disk = IMGFileProxy.make(withDrive: drive) }
                
        super.showSheet()
    }
    
    override public func awakeFromNib() {
        
        track()
        super.awakeFromNib()
        
        sectorData = Array(repeating: "", count: 512 / bytesPerRow)

        // Register to receive mouse click events
        previewTable.action = #selector(clickAction(_:))
        
        // Start with a shrinked window
        var rect = window!.contentRect(forFrameRect: window!.frame)
        rect.size = CGSize(width: 606, height: shrinkedHeight)
        let frame = window!.frameRect(forContentRect: rect)
        window!.setFrame(frame, display: true)
        
        // Run a file system check
        checkVolume()

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
        
        // Jump to the first corrupted block if an error was found
        if errorReport != nil && errorReport!.corruptedBlocks > 0 {
            setCorruptedBlock(1)
        } else {
            update()
        }
    }
    
    override func sheetDidShow() {
        
    }
    
    //
    // Expanding and shrinking the window
    //
    
    func shrink() { setHeight(shrinkedHeight) }
    func expand() { setHeight(expandedHeight) }
    
    func setHeight(_ newHeight: CGFloat) {
                
        var rect = window!.frame
        rect.origin.y += rect.size.height - newHeight
        rect.size.height = newHeight
        
        window!.setFrame(rect, display: true)
        
        // Force the preview table to appear at the correct vertical position
        var r = previewScrollView.frame
        r.origin.y = 91
        previewScrollView.frame = r

        exportButton.keyEquivalent = shrinked ? "\r" : ""

        update()
    }

    //
    // Updating the displayed information
    //
    
    func update() {
                
        updateDiskIcon()
        updateTitleText()
        updateTrackAndSectorInfo()
        updateVolumeInfo()
        if volume == nil { volumeInfo.textColor = .red }

        // Disable the export button if no export is possible
        exportButton.isEnabled = disk != nil

        // Update the disclosure button state
        disclosureButton.state = shrinked ? .off : .on

        // Hide elements that make no sense if no file system is present
        strictButton.isHidden = volume == nil

        // Hide some elements if the window is shrinked
        let items: [NSView] = [
            previewScrollView,
            cylinderText, cylinderField, cylinderStepper,
            headText, headField, headStepper,
            trackText, trackField, trackStepper,
            sectorText, sectorField, sectorStepper,
            blockText, blockField, blockStepper,
            corruptionText, corruptionField, corruptionStepper
        ]
        for item in items { item.isHidden = shrinked }

        // Only proceed if the window is expanded
        if shrinked { return }
        
        // Hide more elements if no errors are present
        if errorReport?.corruptedBlocks == 0 {
            corruptionText.isHidden = true
            corruptionField.isHidden = true
            corruptionStepper.isHidden = true
        }

        // Update all elements
        cylinderField.integerValue     = _cylinder
        cylinderStepper.integerValue   = _cylinder
        headField.integerValue         = _side
        headStepper.integerValue       = _side
        trackField.integerValue        = _track
        trackStepper.integerValue      = _track
        sectorField.integerValue       = _sector
        sectorStepper.integerValue     = _sector
        blockField.integerValue        = _block
        blockStepper.integerValue      = _block
        corruptionField.integerValue   = _corruption
        corruptionStepper.integerValue = _corruption

        if volume?.seekCorruptedBlock(_corruption) == _block {
            corruptionField.textColor = .labelColor
        } else {
            corruptionField.textColor = .tertiaryLabelColor
        }
        
        updateBlockInfo()
        buildStrings()
        previewTable.reloadData()
    }
    
    func updateDiskIcon() {
        
        var name = ""

        if drive.hasDDDisk { name = "dd" }
        if drive.hasHDDisk { name = "hd" }

        switch disk?.type {
        case .FILETYPE_ADF: name += "_adf"
        case .FILETYPE_IMG: name += "_dos"
        default: name += "_other"
        }
        
        if drive.hasWriteProtectedDisk() { name += "_protected" }

        diskIcon.image = NSImage.init(named: name)
    }

    func updateTitleText() {
        
        var name = "This disk contains an unrecognized MFM stream"
        
        if disk?.type == .FILETYPE_ADF { name = "Amiga Disk" }
        if disk?.type == .FILETYPE_IMG { name = "PC Disk" }

        title.stringValue = name
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

                if let errors = errorReport?.corruptedBlocks, errors > 0 {
                    
                    let blocks = errors == 1 ? "block" : "blocks"
                    let text2 = " with \(errors) corrupted \(blocks)"
                    volumeInfo.stringValue = text + text2
                    volumeInfo.textColor = .systemRed
                    return
                }
            }
        }
        
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
            updateErrorInfoUnselected()
        } else {
            updateBlockInfoSelected()
            updateErrorInfoSelected()
        }
    }
    
    func updateBlockInfoUnselected() {
        
        let type = volume!.blockType(_block)
        var text: String
        
        switch type {
        case .UNKNOWN_BLOCK:    text = "Unknown block type"
        case .EMPTY_BLOCK:      text = "Empty Block"
        case .BOOT_BLOCK:       text = "Boot Block"
        case .ROOT_BLOCK:       text = "Root Block"
        case .BITMAP_BLOCK:     text = "Bitmap Block"
        case .USERDIR_BLOCK:    text = "User Directory Block"
        case .FILEHEADER_BLOCK: text = "File Header Block"
        case .FILELIST_BLOCK:   text = "File List Block"
        case .DATA_BLOCK:       text = "Data Block"
        default: fatalError()
        }
        
        info1.stringValue = text
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
            text = "AmigaDOS Header Signature"
        case .FSI_DOS_VERSION:
            text = "AmigaDOS Version Number"
        case .FSI_BOOTCODE:
            text = "Boot Code Instruction"
        case .FSI_TYPE_ID:
            text = "Type Identifier"
        case .FSI_SUBTYPE_ID:
            text = "Subtype Identifier"
        case .FSI_SELF_REF:
            text = "Block Reference to itself"
        case .FSI_CHECKSUM:
            text = "Checksum"
        case .FSI_HASHTABLE_SIZE:
            text = "Hashtable Size"
        case .FSI_HASH_REF:
            text = "Hashtable Entry"
        case .FSI_PROT_BITS:
            text = "Protection Status Bits"
        case .FSI_BCPL_STRING_LENGTH:
            text = "BCPL String Length"
        case .FSI_BCPL_DISK_NAME:
            text = "Disk Name (BCPL Character)"
        case .FSI_BCPL_DIR_NAME:
            text = "Directory Name (BCPL Character)"
        case .FSI_BCPL_FILE_NAME:
            text = "File Name (BCPL Character)"
        case .FSI_BCPL_COMMENT:
            text = "Comment (BCPL Character)"
        case .FSI_CREATED_DAY:
            text = "Creation Date (Days)"
        case .FSI_CREATED_MIN:
            text = "Creation date (Minutes)"
        case .FSI_CREATED_TICKS:
            text = "Creation Date (Ticks)"
        case .FSI_MODIFIED_DAY:
            text = "Modification Date (Day)"
        case .FSI_MODIFIED_MIN:
            text = "Modification Date (Minutes)"
        case .FSI_MODIFIED_TICKS:
            text = "Modification Date (Ticks)"
        case .FSI_NEXT_HASH_REF:
            text = "Reference to the next block with the same hash value"
        case .FSI_PARENT_DIR_REF:
            text = "Parent Directory Block Reference"
        case .FSI_FILEHEADER_REF:
            text = "File Header Block Reference"
        case .FSI_EXT_BLOCK_REF:
            text = "Next Extension Block Reference"
        case .FSI_BITMAP_BLOCK_REF:
            text = "Bitmap Block Reference"
        case .FSI_BITMAP_VALIDITY:
            text = "Bitmap Validity Bits"
        case .FSI_DATA_BLOCK_REF_COUNT:
            text = "Number of Stored Data Block References"
        case .FSI_FILESIZE:
            text = "File Size"
        case .FSI_DATA_BLOCK_NUMBER:
            text = "Position in the Data Block Chain"
        case .FSI_DATA_BLOCK_REF:
            text = "Data Block Reference"
        case .FSI_FIRST_DATA_BLOCK_REF:
            text = "Reference to First Element in the Data Block Chain"
        case .FSI_NEXT_DATA_BLOCK_REF:
            text = "Reference to Next Element in the Data Block Chain"
        case .FSI_DATA_COUNT:
            text = "Number of Stored Data Bytes"
        case .FSI_DATA:
            text = "Data Byte"
        case .FSI_BITMAP:
            text = "Block Allocation Table"
        default:
            fatalError()
        }
        
        info1.stringValue = text
    }

    func updateErrorInfoUnselected() {

        info2.stringValue = ""
    }

    func updateErrorInfoSelected() {
        
        let error = volume!.check(_block, pos: selection!, strict: strict)
        var text: String

        switch error {
        case .OK:
            text = ""
        case .EXPECTED_D:
            text = "Expected 'D'"
        case .EXPECTED_O:
            text = "Expected 'O'"
        case .EXPECTED_S:
            text = "Expected 'S'"
        case .EXPECTED_00:
            text = "Expected $00"
        case .EXPECTED_01:
            text = "Expected $01"
        case .EXPECTED_02:
            text = "Expected $02"
        case .EXPECTED_03:
            text = "Expected $03"
        case .EXPECTED_08:
            text = "Expected $08"
        case .EXPECTED_10:
            text = "Expected $10"
        case .EXPECTED_FD:
            text = "Expected $FD"
        case .EXPECTED_FF:
            text = "Expected $FF"
        case .EXPECTED_DOS_REVISION:
            text = "Expected a value between 0 and 7"
        case .EXPECTED_NO_REF:
            text = "Did not expect a block reference here"
        case .EXPECTED_REF:
            text = "Expected a block reference"
        case .EXPECTED_SELFREF:
            text = "Expected a self-reference"
            
        case .PTR_TO_UNKNOWN_BLOCK:
            text = "This reference points to a block of unknown type"
        case .PTR_TO_EMPTY_BLOCK:
            text = "This reference points to an empty block"
        case .PTR_TO_BOOT_BLOCK:
            text = "This reference points to a boot block"
        case .PTR_TO_ROOT_BLOCK:
            text = "This reference points to the root block"
        case .PTR_TO_BITMAP_BLOCK:
            text = "This reference points to a bitmap block"
        case .PTR_TO_USERDIR_BLOCK:
            text = "This reference points to a user directory block"
        case .PTR_TO_FILEHEADER_BLOCK:
            text = "This reference points to a file header block"
        case .PTR_TO_FILELIST_BLOCK:
            text = "This reference points to a file header block"
        case .PTR_TO_DATA_BLOCK:
            text = "This reference points to a data block"
        case .EXPECTED_DATABLOCK_NR:
            text = "Invalid data block position number"
        case .INVALID_HASHTABLE_SIZE:
            text = "Expected $48 (72 hash table entries)"
        case .INVALID_CHECKSUM:
            text = "Invalid block checksum"
        case .OUT_OF_RANGE:
            text = "Value is out of range"
        default:
            track("error = \(error)")
            fatalError()
        }
        
        info2.stringValue = text
    }
        
    //
    // Exporting the disk
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

    @IBAction func disclosureAction(_ sender: NSButton!) {
        
        shrinked ? expand() : shrink()
    }
    
    @IBAction func clickAction(_ sender: NSTableView!) {
        
        if sender.clickedColumn >= 1 && sender.clickedRow >= 0 {
            
            let newValue = 16 * sender.clickedRow + sender.clickedColumn - 1
            selection = selection != newValue ? newValue : nil
            update()
        }
    }

    @IBAction func cylinderAction(_ sender: NSTextField!) {
        
        setCylinder(sender.integerValue)
    }
    
    @IBAction func cylinderStepperAction(_ sender: NSStepper!) {
        
        setCylinder(sender.integerValue)
    }
    
    @IBAction func headAction(_ sender: NSTextField!) {
        
        setHead(sender.integerValue)
    }
    
    @IBAction func headStepperAction(_ sender: NSStepper!) {
        
        setHead(sender.integerValue)
    }
    
    @IBAction func trackAction(_ sender: NSTextField!) {
        
        setTrack(sender.integerValue)
    }
    
    @IBAction func trackStepperAction(_ sender: NSStepper!) {
        
        setTrack(sender.integerValue)
    }
    
    @IBAction func sectorAction(_ sender: NSTextField!) {
        
        setSector(sender.integerValue)
    }
    
    @IBAction func sectorStepperAction(_ sender: NSStepper!) {
        
        setSector(sender.integerValue)
    }
    
    @IBAction func blockAction(_ sender: NSTextField!) {
        
        setBlock(sender.integerValue)
    }
    
    @IBAction func blockStepperAction(_ sender: NSStepper!) {
        
        setBlock(sender.integerValue)
    }
    
    @IBAction func corruptedBlockAction(_ sender: NSTextField!) {
        
        setCorruptedBlock(sender.integerValue)
    }
    
    @IBAction func corruptedBlockStepperAction(_ sender: NSStepper!) {
        
        setCorruptedBlock(sender.integerValue)
    }

    @IBAction func strictAction(_ sender: NSButton!) {
        
        track()
        
        checkVolume()
        update()
    }
    
    @IBAction func exportAction(_ sender: NSButton!) {
        
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

//
// Extensions
//

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
            
            let offset = 16 * row + col
            let error = volume?.check(_block, pos: offset, strict: strict) ?? .OK
            
            if row == selectedRow && col == selectedCol {
                cell?.textColor = .white
                cell?.backgroundColor = error == .OK ? .selectedContentBackgroundColor : .red
            } else {
                cell?.textColor = error == .OK ? .textColor : .systemRed
                cell?.backgroundColor = NSColor.alternatingContentBackgroundColors[row % 2]
            }
        } else {
            cell?.backgroundColor = NSColor.windowBackgroundColor
        }
    }
    
    func tableView(_ tableView: NSTableView, shouldSelectRow row: Int) -> Bool {
        return false
    }
}
