// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Darwin

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
    @IBOutlet weak var nameInfo: NSTextField!
    @IBOutlet weak var dosInfo: NSTextField!
    @IBOutlet weak var creationInfo: NSTextField!
    @IBOutlet weak var modificationInfo: NSTextField!
    @IBOutlet weak var capacityInfo: NSTextField!
    @IBOutlet weak var blocksInfo: NSTextField!
    @IBOutlet weak var usageInfo: NSTextField!
    @IBOutlet weak var bootblockInfo: NSTextField!

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
    
    var nr = 0
    
    // Returns the inspected floppy or hard drive
    var dfn: DriveProxy { return amiga.df(nr)! }
    var dhn: HardDriveProxy { return amiga.dh(nr)! }
    
    // The analyzed file system
    var vol: FileSystemProxy!
    
    // Result of the consistency checker
    var errorReport: FSErrorReport?
    
    var selection: Int?
    var selectedRow: Int? { return selection == nil ? nil : selection! / 16 }
    var selectedCol: Int? { return selection == nil ? nil : selection! % 16 }
    var strict: Bool { return strictButton.state == .on }

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

            let color = colors[vol.getDisplayType(x)]!
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

        do {
            let dfn = amiga.df(nr)!
            let adf = try ADFFileProxy.make(drive: dfn) as ADFFileProxy
            vol = try FileSystemProxy.make(withADF: adf)
            showSheet(fs: vol)
            
        } catch {
            
            (error as? VAError)?.warning("Unable to decode the file system.")
        }
    }
    
    func showSheet(hardDrive nr: Int) {
        
        track()
        self.nr = nr

        if dhn.partitions == 1 {
            
            // Analyze the first partition
            showSheet(hardDrive: nr, partition: 0)
            
        } else {
            
            // Ask the user to select a partition
            let nibName = NSNib.Name("PartitionSelector")
            let panel = PartitionSelector.make(parent: parent, nibName: nibName)
            panel?.showSheet(hardDrive: nr, completionHandler: {

                track("Completion handler")
                if let part = panel?.userSelection {
                    track("Selected partition = \(part)")
                    self.showSheet(hardDrive: nr, partition: part)
                }
            })
        }
    }
    
    func showSheet(hardDrive nr: Int, partition: Int) {
                
        do {
        
            let hdn = amiga.dh(nr)!
            let hdf = try HDFFileProxy.make(hdr: hdn) as HDFFileProxy
            vol = try FileSystemProxy.make(withHDF: hdf, partition: partition)
            showSheet(fs: vol)
            
        } catch {
            
            (error as? VAError)?.warning("Unable to decode the file system.")
        }
    }
    
    func showSheet(fs: FileSystemProxy) {
        
        vol = fs
        super.showSheet()
    }
    
    override public func awakeFromNib() {
        
        track()
        super.awakeFromNib()
        
        // Register to receive mouse click events
        previewTable.action = #selector(clickAction(_:))
        
        // Configure elements
        blockStepper.maxValue = .greatestFiniteMagnitude
        
        // Run a file system check
        errorReport = vol.check(strict)
        
        update()
    }
    
    override func windowDidLoad() {
        
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
        virus.isHidden = !vol.hasVirus

        // Update disk description
        updateVolumeInfo()
                
        // Hide some elements
        strictButton.isHidden = vol == nil
        
        // Update elements
        blockField.stringValue         = String(format: "%d", blockNr)
        blockStepper.integerValue      = blockNr
        corruptionStepper.integerValue = blockNr
        
        // Inform about corrupted blocks
        updateCorruptionInfo()
        
        // Update the block view table
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
        
    func updateVolumeInfo() {
                
        title.stringValue = vol.dos.description
        nameInfo.stringValue = vol.name
        dosInfo.stringValue = "???"
        creationInfo.stringValue = vol.creationDate
        modificationInfo.stringValue = vol.modificationDate
        capacityInfo.stringValue = vol.capacityString
        blocksInfo.integerValue = vol.numBlocks
        usageInfo.stringValue = String(format: "%d (%.2f%%)", vol.usedBlocks, vol.fillLevel)
        bootblockInfo.stringValue = vol.bootBlockName
        
        /*
        var text = "No compatible file system"
        var color = NSColor.warningColor
        
        if vol != nil {
            
            text = vol.dos.description
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
    
    func updateCorruptionInfo() {
     
        var label = ""
        let total = errorReport?.corruptedBlocks ?? 0
        
        if total > 0 {
                  
            let nr = vol.getCorrupted(blockNr)
            
            if nr > 0 {
                label = "Corrupted block \(nr) out of \(total)"
            } else {
                // let blocks = total == 1 ? "block" : "blocks"
                label = "\(total) corrupted block" + (total == 1 ? "s" : "")
            }
        }
        
        corruptionText.stringValue = label
        corruptionStepper.isHidden = total == 0
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
                
        if selection == nil {
            updateBlockInfoUnselected()
            updateErrorInfoUnselected()
        } else {
            updateBlockInfoSelected()
            updateErrorInfoSelected()
        }
    }
    
    func updateBlockInfoUnselected() {
        
        let type = vol.blockType(blockNr)
        info1.stringValue = type.description
    }
    
    func updateBlockInfoSelected() {
        
        let usage = vol.itemType(blockNr, pos: selection!)
        info1.stringValue = usage.description
    }

    func updateErrorInfoUnselected() {

        info2.stringValue = ""
    }

    func updateErrorInfoSelected() {
        
        var exp = UInt8(0)
        let error = vol.check(blockNr, pos: selection!, expected: &exp, strict: strict)
        info2.stringValue = error.description(expected: Int(exp))
    }
      
    //
    // Helper methods
    //
    
    func setBlock(_ newValue: Int) {
        
        if newValue != blockNr {
                        
            blockNr = newValue.clamped(0, vol.numBlocks - 1)
            selection = nil
            update()
        }
    }

    func setCorruptedBlock(_ newValue: Int) {
        
        var jump: Int
         
        if newValue > blockNr {
            jump = vol.nextCorrupted(blockNr)
        } else {
            jump = vol.prevCorrupted(blockNr)
        }

        corruptionStepper.integerValue = jump
        setBlock(jump)
    }
    
    //
    // Action methods
    //

    @IBAction func clickAction(_ sender: NSTableView!) {
        
        if sender.clickedColumn >= 1 && sender.clickedRow >= 0 {
            
            let newValue = 16 * sender.clickedRow + sender.clickedColumn - 1
            selection = selection != newValue ? newValue : nil
            update()
        }
    }

    @IBAction func blockTypeAction(_ sender: NSButton!) {
        
        var type = FSBlockType(rawValue: sender.tag)!

        // Make sure we search the correct data block type
        if type == .DATA_BLOCK_OFS && vol.isFFS { type = .DATA_BLOCK_FFS }
        if type == .DATA_BLOCK_FFS && vol.isOFS { type = .DATA_BLOCK_OFS }

        // Goto the next block of the requested type
        let nextBlock = vol.nextBlock(of: type, after: blockNr)
        if nextBlock != -1 { setBlock(nextBlock) }
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
        errorReport = vol.check(strict)
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

            let byte = vol.readByte(blockNr, offset: 16 * row + col)
            return String(format: "%02X", byte)

        } else {
            
            return String(format: "%X", row)
        }
    }
}

extension VolumeInspector: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {

        var exp = UInt8(0)
        let cell = cell as? NSTextFieldCell

        if let col = columnNr(tableColumn) {
            
            let offset = 16 * row + col
            let error = vol.check(blockNr, pos: offset, expected: &exp, strict: strict)
            
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
