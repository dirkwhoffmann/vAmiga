// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class VolumeInspector: DialogController {
        
    var myDocument: MyDocument { return parent.mydocument! }

    @IBOutlet weak var layoutButton: NSButton!
    @IBOutlet weak var layoutSlider: NSSlider!
    @IBOutlet weak var bootBlockButton: NSButton!
    @IBOutlet weak var rootBlockButton: NSButton!
    @IBOutlet weak var bmBlockButton: NSButton!
    @IBOutlet weak var bmExtBlockButton: NSButton!
    @IBOutlet weak var fileHeaderBlockButton: NSButton!
    @IBOutlet weak var fileListBlockButton: NSButton!
    @IBOutlet weak var userDirBlockButton: NSButton!
    @IBOutlet weak var dataBlockButton: NSButton!

    @IBOutlet weak var icon: NSImageView!
    @IBOutlet weak var virus: NSImageView!
    @IBOutlet weak var title: NSTextField!
    @IBOutlet weak var capacityInfo: NSTextField!
    @IBOutlet weak var numBlocksInfo: NSTextField!
    @IBOutlet weak var bsizeInfo: NSTextField!
    @IBOutlet weak var usageInfo: NSTextField!
    @IBOutlet weak var nameInfo: NSTextField!
    @IBOutlet weak var creationInfo: NSTextField!
    @IBOutlet weak var modificationInfo: NSTextField!
    @IBOutlet weak var bootblockInfo: NSTextField!

    @IBOutlet weak var decontaminationButton: NSButton!

    @IBOutlet weak var previewScrollView: NSScrollView!
    @IBOutlet weak var previewTable: NSTableView!
    @IBOutlet weak var blockText: NSTextField!
    @IBOutlet weak var blockField: NSTextField!
    @IBOutlet weak var blockStepper: NSStepper!
    @IBOutlet weak var corruptionText: NSTextField!
    @IBOutlet weak var corruptionStepper: NSStepper!
    @IBOutlet weak var strictButton: NSButton!

    @IBOutlet weak var info1: NSTextField!
    @IBOutlet weak var info2: NSTextField!
    
    var nr = -1
    
    // Returns the inspected floppy or hard drive
    var dfn: DriveProxy { return amiga.df(nr)! }
    var dhn: HardDriveProxy { return amiga.dh(nr)! }
    
    // Number of blocks in this volume (DEPRECATED)
    var b = 0
    
    var upperBlock: Int { return b > 0 ? b - 1 : 0 }

    // Results of the different decoders
    var hdf: HDFFileProxy?
    var adf: ADFFileProxy?
    var vol: FileSystemProxy?
    
    // Result of the consistency checker
    var errorReport: FSErrorReport?
    
    var selection: Int?
    var selectedRow: Int? { return selection == nil ? nil : selection! / 16 }
    var selectedCol: Int? { return selection == nil ? nil : selection! % 16 }
    var strict: Bool { return strictButton.state == .on }

    var numPartitions: Int { return hdf?.numPartitions ?? 1 }

    var hasVirus: Bool {
        return adf?.hasVirus ?? false
    }

    // Block preview
    var blockNr = 0
    
    let colors: [FSBlockType: NSColor] = [
        
        .UNKNOWN_BLOCK: NSColor.white,
        .EMPTY_BLOCK: NSColor.gray,
        .BOOT_BLOCK: NSColor(r: 0xFF, g: 0xCC, b: 0x99, a: 0xFF),
        .ROOT_BLOCK: NSColor(r: 0xFF, g: 0x99, b: 0x99, a: 0xFF),
        .BITMAP_BLOCK: NSColor(r: 0xCC, g: 0x99, b: 0xFF, a: 0xFF),
        .BITMAP_EXT_BLOCK: NSColor(r: 0xE5, g: 0xCC, b: 0xFF, a: 0xFF),
        .USERDIR_BLOCK: NSColor(r: 0x99, g: 0xCC, b: 0xFF, a: 0xFF),
        .FILEHEADER_BLOCK: NSColor(r: 0xFF, g: 0xFF, b: 0x99, a: 0xFF),
        .FILELIST_BLOCK: NSColor(r: 0xFF, g: 0xFF, b: 0xCC, a: 0xFF),
        .DATA_BLOCK_OFS: NSColor(r: 0x99, g: 0xFF, b: 0x99, a: 0xFF),
        .DATA_BLOCK_FFS: NSColor(r: 0x99, g: 0xFF, b: 0x99, a: 0xFF)
    ]

    var layoutImage: NSImage? {
        
        // Create image representation in memory
        let width = 1760
        let height = 16
        let size = CGSize(width: width, height: height)
        let cap = Int(size.width) * Int(size.height)
        let mask = calloc(cap, MemoryLayout<UInt32>.size)!
        let ptr = mask.bindMemory(to: UInt32.self, capacity: cap)

        // Create image data
        for x in 0..<width {

            let color = colors[vol?.getDisplayType(x) ?? .UNKNOWN_BLOCK]!
            let ciColor = CIColor(color: color)!
            
            for y in 0...height-1 {
                
                var r, g, b, a: Int
                
                r = Int(ciColor.red * CGFloat(255 - 2*y))
                g = Int(ciColor.green * CGFloat(255 - 2*y))
                b = Int(ciColor.blue * CGFloat(255 - 2*y))
                a = Int(ciColor.alpha)
                
                let abgr = UInt32(r | g << 8 | b << 16 | a << 24)
                ptr[y*width + x] = abgr
            }
        }

        // Create image
        let image = NSImage.make(data: mask, rect: size)
        let resizedImage = image?.resizeSharp(width: CGFloat(width), height: CGFloat(height))
        return resizedImage
    }
    
    //
    // Starting up
    //
    
    func showSheet(diskDrive nr: Int) {
        
        track()
        
        self.nr = nr

        // Run the ADF decoder
        adf = try? ADFFileProxy.make(drive: dfn) as ADFFileProxy
                        
        // Try to extract the file system
        initVolume(partition: 0)
                
        super.showSheet()
    }
    
    func showSheet(hardDrive nr: Int) {
        
        track()
        
        self.nr = nr

        // Run the HDF decoder
        hdf = try? HDFFileProxy.make(hdr: dhn) as HDFFileProxy
                                
        // Try to decide the file system
        initVolume(partition: 0)

        super.showSheet()
    }
    
    func initVolume(partition nr: Int) {
    
        track("partition = \(nr)")
        
        vol = nil
        if adf != nil { vol = try? FileSystemProxy.make(withADF: adf!) }
        if hdf != nil { vol = try? FileSystemProxy.make(withHDF: hdf!, partition: nr) }
        
        b = vol?.numBlocks ?? 0
        blockNr = blockNr.clamped(0, upperBlock)
    }
        
    override public func awakeFromNib() {
        
        track()
        super.awakeFromNib()
        
        // Register to receive mouse click events
        previewTable.action = #selector(clickAction(_:))
        
        // Configure elements
        blockStepper.maxValue = .greatestFiniteMagnitude
        
        // Run a file system check
        errorReport = vol?.check(strict)
        
        update()
    }
    
    override func windowDidLoad() {

        /*
        // Initialize the partition selector
        partitionPopup.removeAllItems()
                
        for i in 1...numPartitions {
                    
            partitionPopup.addItem(withTitle: "Partition \(i)")
            partitionPopup.lastItem!.tag = i - 1
        }
        partitionPopup.autoenablesItems = false
        */
        
        // Update the block allocation map
        updateLayoutImage()
        
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
    // Updating the displayed information
    //

    func update() {
          
        // Update icons
        updateDiskIcon()

        // Update disk description
        updateTitleText()
        updateTrackAndSectorInfo()
        updateVolumeInfo()
        updateBootInfo()
                
        // Hide some elements
        strictButton.isHidden = vol == nil
        
        // Update all elements
        blockField.stringValue         = String(format: "%d", blockNr)
        blockStepper.integerValue      = blockNr
        corruptionStepper.integerValue = blockNr
        
        if let total = errorReport?.corruptedBlocks, total > 0 {
                     
            if let corr = vol?.getCorrupted(blockNr), corr > 0 {
                track("total = \(total) corr = \(corr)")
                corruptionText.stringValue = "Corrupted block \(corr) out of \(total)"
            } else {
                let blocks = total == 1 ? "block" : "blocks"
                corruptionText.stringValue = "\(total) corrupted \(blocks)"
            }
            
            corruptionText.textColor = .labelColor
            corruptionText.textColor = .warningColor
            corruptionStepper.isHidden = false
        
        } else {
            corruptionText.stringValue = ""
            corruptionStepper.isHidden = true
        }
        
        updateBlockInfo()
        previewTable.reloadData()
    }
    
    func updateLayoutImage() {
             
        // Update the layout image
        layoutButton.image = layoutImage

        // Update color images
        let size = NSSize(width: 16, height: 16)
        bootBlockButton.image = NSImage(color: colors[.BOOT_BLOCK]!, size: size)
        rootBlockButton.image = NSImage(color: colors[.ROOT_BLOCK]!, size: size)
        bmBlockButton.image = NSImage(color: colors[.BITMAP_BLOCK]!, size: size)
        bmExtBlockButton.image = NSImage(color: colors[.BITMAP_EXT_BLOCK]!, size: size)
        fileListBlockButton.image = NSImage(color: colors[.FILELIST_BLOCK]!, size: size)
        fileHeaderBlockButton.image = NSImage(color: colors[.FILEHEADER_BLOCK]!, size: size)
        userDirBlockButton.image = NSImage(color: colors[.USERDIR_BLOCK]!, size: size)
        dataBlockButton.image = NSImage(color: colors[.DATA_BLOCK_OFS]!, size: size)
    }
    
    func updateDiskIcon() {

        /*
        let name = ""
    
        icon.image = NSImage(named: name != "" ? name : "biohazard")
        virus.isHidden = !hasVirus
        decontaminationButton.isHidden = !hasVirus
        */
    }
    
    func updateTitleText() {
        
        /*
        let text = "Amiga File System"
        let color = NSColor.textColor
                
        title.stringValue = text
        title.textColor = color
        */
    }

    func updateTrackAndSectorInfo() {
        
        /*
        let text = "Lorem ipsum"
        let color = NSColor.warningColor
        
        layoutInfo.stringValue = text
        layoutInfo.textColor = color
        */
    }
    
    func updateVolumeInfo() {
        
        /*
        var text = "No compatible file system"
        var color = NSColor.warningColor
        
        if vol != nil {
            
            text = vol!.dos.description
            color = .secondaryLabelColor
            
            if let errors = errorReport?.corruptedBlocks, errors > 0 {
                
                let blocks = errors == 1 ? "block" : "blocks"
                text += " with \(errors) corrupted \(blocks)"
                color = .warningColor
            }
        }
        
        volumeInfo.stringValue = text
        volumeInfo.textColor = color
        */
    }
    
    func updateBootInfo() {
               
        /*
        if adf == nil {
            bootInfo.stringValue = ""
            return
        }
        
        bootInfo.stringValue = adf!.bootInfo
        bootInfo.textColor = adf!.hasVirus ? .warningColor : .secondaryLabelColor
        */
    }
    
    func updateBlockInfo() {
        
        if vol == nil {
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
        
        let type = vol!.blockType(blockNr)
        info1.stringValue = type.description
    }
    
    func updateBlockInfoSelected() {
        
        let usage = vol!.itemType(blockNr, pos: selection!)
        info1.stringValue = usage.description
    }

    func updateErrorInfoUnselected() {

        info2.stringValue = ""
    }

    func updateErrorInfoSelected() {
        
        var exp = UInt8(0)
        let error = vol!.check(blockNr, pos: selection!, expected: &exp, strict: strict)
        info2.stringValue = error.description(expected: Int(exp))
    }
      
    //
    // Helper methods
    //
    
    func setBlock(_ newValue: Int) {
        
        if newValue != blockNr {
                        
            blockNr = newValue.clamped(0, upperBlock)
            selection = nil
            update()
        }
    }

    func setCorruptedBlock(_ newValue: Int) {
        
        var jump: Int
         
        if newValue > blockNr {
            jump = vol!.nextCorrupted(blockNr)
        } else {
            jump = vol!.prevCorrupted(blockNr)
        }

        corruptionStepper.integerValue = jump
        setBlock(jump)
    }
    
    //
    // Action methods
    //

    @IBAction func partitionAction(_ sender: NSPopUpButton!) {
        
        track()
        
        initVolume(partition: sender.selectedTag())
        update()
        updateLayoutImage()
    }

    @IBAction func decontaminationAction(_ sender: NSButton!) {
        
        track()
        adf?.killVirus()
        vol?.killVirus()
        update()
    }

    @IBAction func clickAction(_ sender: NSTableView!) {
        
        if sender.clickedColumn >= 1 && sender.clickedRow >= 0 {
            
            let newValue = 16 * sender.clickedRow + sender.clickedColumn - 1
            selection = selection != newValue ? newValue : nil
            update()
        }
    }
    
    @IBAction func blockAction(_ sender: NSTextField!) {
        
        setBlock(sender.integerValue)
    }
    
    @IBAction func blockStepperAction(_ sender: NSStepper!) {
        
        setBlock(sender.integerValue)
    }
        
    @IBAction func corruptedBlockStepperAction(_ sender: NSStepper!) {
    
        setCorruptedBlock(sender.integerValue)
    }

    @IBAction func strictAction(_ sender: NSButton!) {
        
        track()
        errorReport = vol?.check(strict)
        update()
    }
}

//
// Extensions
//

extension VolumeInspector: NSWindowDelegate {
    
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

extension VolumeInspector: NSTableViewDataSource {
    
    func columnNr(_ column: NSTableColumn?) -> Int? {
        
        return column == nil ? nil : Int(column!.identifier.rawValue)
    }
        
    func numberOfRows(in tableView: NSTableView) -> Int {
        
        return 512 / 16
    }
    
    func tableView(_ tableView: NSTableView,
                   objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        if let col = columnNr(tableColumn) {

            if let byte = vol?.readByte(blockNr, offset: 16 * row + col) {
                return String(format: "%02X", byte)
            }

        } else {
            return String(format: "%X", row)
        }
        
        return ""
    }
}

extension VolumeInspector: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {

        var exp = UInt8(0)
        let cell = cell as? NSTextFieldCell

        if let col = columnNr(tableColumn) {
            
            let offset = 16 * row + col
            let error = vol?.check(blockNr, pos: offset, expected: &exp, strict: strict) ?? .OK
            
            if row == selectedRow && col == selectedCol {
                cell?.textColor = .white
                cell?.backgroundColor = error == .OK ? .selectedContentBackgroundColor : .warningColor
            } else {
                cell?.textColor = error == .OK ? .textColor : .warningColor
                cell?.backgroundColor = NSColor.alternatingContentBackgroundColors[row % 2]
            }
        } else {
            cell?.backgroundColor = .windowBackgroundColor
        }
    }
    
    func tableView(_ tableView: NSTableView, shouldSelectRow row: Int) -> Bool {
        return false
    }
}
