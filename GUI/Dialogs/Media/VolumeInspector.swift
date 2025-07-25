// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// import Darwin

@MainActor
class VolumeInspector: DialogController {
        
    @IBOutlet weak var icon: NSImageView!
    @IBOutlet weak var virus: NSImageView!
    @IBOutlet weak var title: NSTextField!
    @IBOutlet weak var nameInfo: NSTextField!
    @IBOutlet weak var creationInfo: NSTextField!
    @IBOutlet weak var modificationInfo: NSTextField!
    @IBOutlet weak var bootblockInfo: NSTextField!
    @IBOutlet weak var capacityInfo: NSTextField!
    @IBOutlet weak var blocksInfo: NSTextField!
    @IBOutlet weak var usageInfo: NSTextField!
    @IBOutlet weak var virusInfo: NSTextField!

    @IBOutlet weak var blockImageButton: NSButton!
    @IBOutlet weak var blockSlider: NSSlider!
    @IBOutlet weak var bootBlockButton: NSButton!
    @IBOutlet weak var rootBlockButton: NSButton!
    @IBOutlet weak var bmBlockButton: NSButton!
    @IBOutlet weak var bmExtBlockButton: NSButton!
    @IBOutlet weak var fileHeaderBlockButton: NSButton!
    @IBOutlet weak var fileListBlockButton: NSButton!
    @IBOutlet weak var userDirBlockButton: NSButton!
    @IBOutlet weak var dataBlockButton: NSButton!

    @IBOutlet weak var allocImageButton: NSButton!
    @IBOutlet weak var allocSlider: NSSlider!
    @IBOutlet weak var allocInfo: NSTextField!
    @IBOutlet weak var allocGreenButton: NSButton!
    @IBOutlet weak var allocYellowButton: NSButton!
    @IBOutlet weak var allocRedButton: NSButton!
    @IBOutlet weak var allocRectifyInfo: NSTextField!
    @IBOutlet weak var allocRectifyButton: NSButton!

    @IBOutlet weak var diagnoseImageButton: NSButton!
    @IBOutlet weak var diagnoseSlider: NSSlider!
    @IBOutlet weak var diagnoseInfo: NSTextField!
    @IBOutlet weak var diagnosePassButton: NSButton!
    @IBOutlet weak var diagnoseFailButton: NSButton!
    @IBOutlet weak var diagnoseNextButton: NSButton!
    @IBOutlet weak var diagnoseNextInfo: NSTextField!

    @IBOutlet weak var previewScrollView: NSScrollView!
    @IBOutlet weak var previewTable: NSTableView!
    @IBOutlet weak var blockField: NSTextField!
    @IBOutlet weak var blockStepper: NSStepper!
    @IBOutlet weak var strictButton: NSButton!
    @IBOutlet weak var info1: NSTextField!
    @IBOutlet weak var info2: NSTextField!
            
    var myDocument: MyDocument { return parent.mydocument! }

    struct Palette {
        
        static let white = NSColor.white
        static let gray = NSColor.gray
        static let black = NSColor.black
        static let red = NSColor(r: 0xff, g: 0x66, b: 0x66, a: 0xff)
        static let orange = NSColor(r: 0xff, g: 0xb2, b: 0x66, a: 0xff)
        static let yellow = NSColor(r: 0xff, g: 0xff, b: 0x66, a: 0xff)
        static let green = NSColor(r: 0x66, g: 0xff, b: 0x66, a: 0xff)
        static let dgreen = NSColor(r: 0x00, g: 0x99, b: 0x00, a: 0xff)
        static let cyan = NSColor(r: 0x66, g: 0xff, b: 0xff, a: 0xff)
        static let blue = NSColor(r: 0x66, g: 0xb2, b: 0xff, a: 0xff)
        static let purple = NSColor(r: 0xb2, g: 0x66, b: 0xff, a: 0xff)
        static let pink = NSColor(r: 0xff, g: 0x66, b: 0xff, a: 0xff)
    }

    // The analyzed file system
    var vol: FileSystemProxy!
    
    // Result of the consistency checker
    var erroneousBlocks: [NSNumber] = []
    var bitMapErrors: [NSNumber] = []

    var selection: Int?
    var selectedRow: Int? { return selection == nil ? nil : selection! / 16 }
    var selectedCol: Int? { return selection == nil ? nil : selection! % 16 }
    var strict: Bool { return strictButton.state == .on }

    // Block preview
    var blockNr = 0
    
    let palette: [FSBlockType: NSColor] = [
        
        .UNKNOWN: Palette.white,
        .EMPTY: Palette.gray,
        .BOOT: Palette.orange,
        .ROOT: Palette.red,
        .BITMAP: Palette.purple,
        .BITMAP_EXT: Palette.pink,
        .USERDIR: Palette.yellow,
        .FILEHEADER: Palette.blue,
        .FILELIST: Palette.dgreen, //  Palette.cyan,
        .DATA_OFS: Palette.green,
        .DATA_FFS: Palette.green
    ]

    func layoutImage(size: NSSize) -> NSImage? {
        
        var data = Data(count: Int(size.width))
                
        data.withUnsafeMutableBytes { ptr in
            if let baseAddress = ptr.baseAddress {
                vol.createUsageMap(baseAddress, length: Int(size.width))
            }
        }
                
        return createImage(data: data, size: size, colorize: { (x: UInt8) -> NSColor in
            
            return palette[FSBlockType(rawValue: Int(x)) ?? .UNKNOWN]!
        })
    }

    func allocImage(size: NSSize) -> NSImage? {
        
        var data = Data(count: Int(size.width))
                
        data.withUnsafeMutableBytes { ptr in
            if let baseAddress = ptr.baseAddress {
                vol.createAllocationMap(baseAddress, length: Int(size.width))
            }
        }
        
        return createImage(data: data, size: size, colorize: { (x: UInt8) -> NSColor in
            
            switch x {
            case 0: return Palette.gray
            case 1: return Palette.green
            case 2: return Palette.yellow
            case 3: return Palette.red
            default: fatalError()
            }
        })
    }
    
    func diagnoseImage(size: NSSize) -> NSImage? {
        
        var data = Data(count: Int(size.width))
                
        data.withUnsafeMutableBytes { ptr in
            if let baseAddress = ptr.baseAddress {
                vol.createHealthMap(baseAddress, length: Int(size.width))
            }
        }
        
        return createImage(data: data, size: size, colorize: { (x: UInt8) -> NSColor in
            
            switch x {
            case 0: return Palette.gray
            case 1: return Palette.green
            case 2: return Palette.red
            default: return Palette.white
            }
        })
    }
 
    func createImage(data: Data, size: NSSize, colorize: (UInt8) -> NSColor) -> NSImage? {
        
        precondition(data.count == Int(size.width))
        
        // Create image representation in memory
        let width = Int(size.width)
        let height = Int(size.height)
        let cap = width * height
        let mask = calloc(cap, MemoryLayout<UInt32>.size)!
        let ptr = mask.bindMemory(to: UInt32.self, capacity: cap)

        // Create image data
        for x in 0..<width {

            // let color = colors[vol.getDisplayType(x)]!
            let color = colorize(data[x])
            let ciColor = CIColor(color: color)!
            
            for y in 0...height-1 {
                
                var r, g, b, a: Int
                
                r = Int(ciColor.red * CGFloat(255 - 2*y))
                g = Int(ciColor.green * CGFloat(255 - 2*y))
                b = Int(ciColor.blue * CGFloat(255 - 2*y))
                a = Int(ciColor.alpha * CGFloat(255))

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
    
    func show(diskDrive nr: Int) throws {
        
        let dfn = emu.df(nr)!
        let adf = try MediaFileProxy.make(with: dfn, type: .ADF)
        vol = try FileSystemProxy.make(with: adf)

        showAsWindow()
    }
    
    func show(hardDrive nr: Int) throws {
        
        let hdn = emu.hd(nr)!
        var partition: Int?
        
        if hdn.info.partitions == 1 {
        
            // Analyze the first partition
            partition = 0
            
        } else {
            
            // Ask the user to select a partition
            let panel = PartitionSelector(with: parent, nibName: "PartitionSelector")
            panel?.showSheet(hardDrive: nr, completionHandler: {
                partition = panel?.userSelection
            })
        }
        
        if partition != nil {
            try show(hardDrive: nr, partition: partition!)
        }
    }
    
    func show(hardDrive nr: Int, partition: Int) throws {
        
        let hdn = emu.hd(nr)!
        let hdf = try MediaFileProxy.make(with: hdn, type: .HDF)
        vol = try FileSystemProxy.make(with: hdf, partition: partition)
        
        showAsWindow()
    }
    
    func showSheet(fs: FileSystemProxy) {
     
        vol = fs
        showAsWindow()
    }
        
    override func windowDidLoad() {
                
    }

    override func dialogWillShow() {

        super.dialogWillShow()

        // Register to receive mouse click events
        previewTable.action = #selector(clickAction(_:))
        
        // Configure elements
        blockStepper.maxValue = .greatestFiniteMagnitude
        blockSlider.minValue = 0
        blockSlider.maxValue = Double(vol.numBlocks - 1)
        allocSlider.minValue = 0
        allocSlider.maxValue = Double(vol.numBlocks - 1)
        diagnoseSlider.minValue = 0
        diagnoseSlider.maxValue = Double(vol.numBlocks - 1)

        // Run a file system check
        erroneousBlocks = vol.xrayBlocks
        bitMapErrors = vol.xrayBitmap

        // Compute images
        updateUsageImage()
        updateAllocImage()
        updateHealthImage()
        
        update()
    }
     
    //
    // Updating the displayed information
    //

    func update() {
          
        updateVolumeInfo()
        updateVirusInfo()
        updateAllocInfo()
        updateHealthInfo()
        
        // Update elements
        blockField.stringValue         = String(format: "%d", blockNr)
        blockStepper.integerValue      = blockNr
        blockSlider.integerValue       = blockNr
        allocSlider.integerValue       = blockNr
        diagnoseSlider.integerValue    = blockNr
        
        // Update the block view table
        updateBlockInfo()
        previewTable.reloadData()
    }
    
    func updateUsageImage() {
             
        let size = NSSize(width: 16, height: 16)
        bootBlockButton.image = NSImage(color: palette[.BOOT]!, size: size)
        rootBlockButton.image = NSImage(color: palette[.ROOT]!, size: size)
        bmBlockButton.image = NSImage(color: palette[.BITMAP]!, size: size)
        bmExtBlockButton.image = NSImage(color: palette[.BITMAP_EXT]!, size: size)
        fileListBlockButton.image = NSImage(color: palette[.FILELIST]!, size: size)
        fileHeaderBlockButton.image = NSImage(color: palette[.FILEHEADER]!, size: size)
        userDirBlockButton.image = NSImage(color: palette[.USERDIR]!, size: size)
        dataBlockButton.image = NSImage(color: palette[.DATA_OFS]!, size: size)
        blockImageButton.image = layoutImage(size: blockImageButton.bounds.size.scaled(x: 2.0))
    }

    func updateAllocImage() {
            
        let size = NSSize(width: 16, height: 16)
        allocGreenButton.image = NSImage(color: Palette.green, size: size)
        allocYellowButton.image = NSImage(color: Palette.yellow, size: size)
        allocRedButton.image = NSImage(color: Palette.red, size: size)
        allocImageButton.image = allocImage(size: allocImageButton.bounds.size)
    }

    func updateHealthImage() {
            
        let size = NSSize(width: 16, height: 16)
        diagnosePassButton.image = NSImage(color: Palette.green, size: size)
        diagnoseFailButton.image = NSImage(color: Palette.red, size: size)
        diagnoseImageButton.image = diagnoseImage(size: diagnoseImageButton.bounds.size)
    }
    
    func updateVolumeInfo() {
                
        title.stringValue = vol.dos.description
        nameInfo.stringValue = vol.name
        bootblockInfo.stringValue = vol.bootBlockName
        creationInfo.stringValue = vol.creationDate
        modificationInfo.stringValue = vol.modificationDate
        capacityInfo.stringValue = vol.capacityString
        blocksInfo.integerValue = vol.numBlocks
        usageInfo.stringValue = "\(vol.usedBlocks) (" + vol.fillLevelString + ")"
    }
    
    func updateVirusInfo() {
        
        if vol.hasVirus {
            
            virus.isHidden = false
            virusInfo.stringValue = "Positive"
            virusInfo.textColor = .warning
            bootblockInfo.textColor = .warning

        } else {

            virus.isHidden = true
            virusInfo.stringValue = "Passed - No findings"
            virusInfo.textColor = .secondaryLabelColor
            bootblockInfo.textColor = .secondaryLabelColor
        }
    }
    
    func updateAllocInfo() {
     
        // let total = errorReport?.bitmapErrors ?? 0
        let total = bitMapErrors.count

        if total > 0 {
            
            let blocks = total == 1 ? "block" : "blocks"
            diagnoseInfo.stringValue = "\(total) suspicious \(blocks) found"
        }

        allocInfo.isHidden = total == 0
        allocRectifyInfo.isHidden = total == 0
        allocRectifyButton.isHidden = total == 0
    }
    
    func updateHealthInfo() {
     
        // let total = errorReport?.corruptedBlocks ?? 0
        let total = erroneousBlocks.count

        if total > 0 {
            
            let blocks = total == 1 ? "block" : "blocks"
            diagnoseInfo.stringValue = "\(total) corrupted \(blocks) found"
        }

        diagnoseInfo.isHidden = total == 0
        diagnoseNextInfo.isHidden = total == 0
        diagnoseNextButton.isHidden = total == 0
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
    
    //
    // Action methods
    //

    @IBAction func sliderAction(_ sender: NSSlider!) {

        setBlock(sender.integerValue)
    }
    
    @IBAction func blockTypeAction(_ sender: NSButton!) {
        
        var type = FSBlockType(rawValue: sender.tag)!

        // Make sure we search the correct data block type
        if type == .DATA_OFS && vol.isFFS { type = .DATA_FFS }
        if type == .DATA_FFS && vol.isOFS { type = .DATA_OFS }

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
            
    @IBAction func gotoNextCorruptedBlockAction(_ sender: NSButton!) {

        var low = 0
        var high = erroneousBlocks.count

        while low < high {

            let mid = (low + high) / 2
            if erroneousBlocks[mid].intValue > blockNr {
                high = mid
            } else {
                low = mid + 1
            }
        }

        if low < erroneousBlocks.count {
            setBlock(erroneousBlocks[low].intValue)
        } else if erroneousBlocks.count > 0 {
            setBlock(erroneousBlocks[0].intValue)
        }
    }

    @IBAction func rectifyAction(_ sender: NSButton!) {
        
        vol.rectifyAllocationMap()
        updateAllocImage()
        update()
    }

    @IBAction func strictAction(_ sender: NSButton!) {

        // Examine all blocks
        vol.xrayBlocks(strict)
        erroneousBlocks = vol.xrayBlocks

        // Examime the bitmap
        vol.xrayBitmap(strict)
        bitMapErrors = vol.xrayBitmap

        updateHealthImage()
        update()
    }
    
    @IBAction func clickAction(_ sender: NSTableView!) {
        
        if sender.clickedColumn >= 1 && sender.clickedRow >= 0 {
            
            let newValue = 16 * sender.clickedRow + sender.clickedColumn - 1
            selection = selection != newValue ? newValue : nil
            update()
        }
    }
}

@MainActor
extension VolumeInspector: NSTableViewDataSource {
    
    func columnNr(_ column: NSTableColumn?) -> Int? {
        
        return column == nil ? nil : Int(column!.identifier.rawValue)
    }
        
    func numberOfRows(in tableView: NSTableView) -> Int {
        
        return 512 / 16
    }
    
    func tableView(_ tableView: NSTableView,
                   objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
                
        switch tableColumn?.identifier.rawValue {

        case "Offset":
            return String(format: "%X", row)
            
        case "Ascii":
            return vol.ascii(blockNr, offset: 16 * row, length: 16)
            
        default:
            if let col = columnNr(tableColumn) {

                let byte = vol.readByte(blockNr, offset: 16 * row + col)
                return String(format: "%02X", byte)
            }
        }
        fatalError()
    }
}

@MainActor
extension VolumeInspector: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {

        var exp = UInt8(0)
        let cell = cell as? NSTextFieldCell

        if let col = columnNr(tableColumn) {
            
            let offset = 16 * row + col
            let error = vol.check(blockNr, pos: offset, expected: &exp, strict: strict)
            
            if row == selectedRow && col == selectedCol {
                cell?.textColor = .white
                cell?.backgroundColor = error == .OK ? .selectedContentBackgroundColor : .warning
            } else {
                cell?.textColor = error == .OK ? .textColor : .warning
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

@MainActor
extension VolumeInspector: NSTabViewDelegate {
    
    func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {
        
        update()
    }
}
