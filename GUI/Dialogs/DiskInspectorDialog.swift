// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class DiskInspectorDialog: DialogController {
        
    @IBOutlet weak var diskIcon: NSImageView!
    @IBOutlet weak var virusIcon: NSImageView!
    @IBOutlet weak var title: NSTextField!
    @IBOutlet weak var layoutInfo: NSTextField!
    @IBOutlet weak var volumeInfo: NSTextField!
    @IBOutlet weak var bootInfo: NSTextField!
    @IBOutlet weak var decontaminationButton: NSButton!

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
    @IBOutlet weak var corruptionStepper: NSStepper!

    @IBOutlet weak var info1: NSTextField!
    @IBOutlet weak var info2: NSTextField!

    @IBOutlet weak var strictButton: NSButton!
    
    var savePanel: NSSavePanel!  // Used to export to files
    var openPanel: NSOpenPanel!  // Used to export to directories

    var driveNr: Int?
    var drive: DriveProxy? { return driveNr == nil ? nil : amiga.df(driveNr!) }
    
    // Results of the different decoders
    var adf: ADFFileProxy?
    var img: IMGFileProxy?
    var ext: EXTFileProxy?
    var vol: FSDeviceProxy?

    var errorReport: FSErrorReport?
    
    var selection: Int?
    var selectedRow: Int? { return selection == nil ? nil : selection! / 16 }
    var selectedCol: Int? { return selection == nil ? nil : selection! % 16 }
    var strict: Bool { return strictButton.state == .on }
        
    var myDocument: MyDocument { return parent.mydocument! }
    // var size: CGSize { return window!.frame.size }
        
    var numCyls: Int {
        return adf?.numCyls ?? img?.numCyls ?? ext?.numCyls ?? vol?.numCyls ?? 0
    }
    var numSides: Int {
        return adf?.numSides ?? img?.numSides ?? ext?.numSides ?? vol?.numHeads ?? 0
    }
    var numTracks: Int {
        return adf?.numTracks ?? img?.numTracks ?? ext?.numTracks ?? vol?.numTracks ?? 0
    }
    var numSectors: Int {
        return adf?.numSectors ?? img?.numSectors ?? ext?.numSectors ?? vol?.numSectors ?? 0
    }
    var numBlocks: Int {
        return adf?.numBlocks ?? img?.numBlocks ?? ext?.numBlocks ?? vol?.numBlocks ?? 0
    }
    var isDD: Bool {
        return adf?.isDD ?? img?.isDD ?? ext?.isDD ?? false
    }
    var isHD: Bool {
        return adf?.isHD ?? img?.isHD ?? ext?.isHD ?? false
    }
    var hasVirus: Bool {
        return adf?.hasVirus ?? false
    }

    // Block preview
    var cylinderNr = 0
    var headNr = 0
    var trackNr = 0
    var sectorNr = 0
    var blockNr = 0
    
    //
    // Selecting a block
    //
    
    func setCylinder(_ newValue: Int) {
        
        if newValue != cylinderNr {

            let value = newValue.clamped(0, numCyls - 1)

            cylinderNr = value
            trackNr    = cylinderNr * 2 + headNr
            blockNr    = trackNr * numSectors + sectorNr
            
            selection = nil
            update()
        }
    }
    
    func setHead(_ newValue: Int) {
        
        if newValue != headNr {
                        
            let value = newValue.clamped(0, numSides - 1)

            headNr     = value
            trackNr    = cylinderNr * 2 + headNr
            blockNr    = trackNr * numSectors + sectorNr
            
            selection = nil
            update()
        }
    }
    
    func setTrack(_ newValue: Int) {
        
        if newValue != trackNr {
                   
            let value = newValue.clamped(0, numTracks - 1)
            
            trackNr    = value
            cylinderNr = trackNr / 2
            headNr     = trackNr % 2
            blockNr    = trackNr * numSectors + sectorNr

            selection = nil
            update()
        }
    }

    func setSector(_ newValue: Int) {
        
        if newValue != sectorNr {
                  
            let value = newValue.clamped(0, numSectors - 1)
            
            sectorNr   = value
            blockNr    = trackNr * numSectors + sectorNr
            
            selection = nil
            update()
        }
    }

    func setBlock(_ newValue: Int) {
        
        if newValue != blockNr {
                        
            let value = newValue.clamped(0, numBlocks - 1)

            blockNr    = value
            trackNr    = blockNr / numSectors
            sectorNr   = blockNr % numSectors
            cylinderNr = trackNr / 2
            headNr     = trackNr % 2
            
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
    // Starting up
    //
    
    func showSheet(forDrive nr: Int) {
        
        track()
        
        driveNr = nr

        // Run the ADF decoder
        adf = try? ADFFileProxy.make(drive: drive!) as ADFFileProxy

        // Run the extended ADF decoder
        ext = try? EXTFileProxy.make(drive: drive!) as EXTFileProxy

        // Run the DOS decoder
        img = try? IMGFileProxy.make(drive: drive!) as IMGFileProxy
                        
        // Try to decode the file system from the ADF
        if adf != nil { vol = try? FSDeviceProxy.make(withADF: adf!) }

        super.showSheet()
    }
    
    func showSheet(forVolume volume: FSDeviceProxy) {
        
        vol = volume
        super.showSheet()
    }
        
    override public func awakeFromNib() {
        
        track()
        super.awakeFromNib()
        
        // Register to receive mouse click events
        previewTable.action = #selector(clickAction(_:))
        
        // Configure elements
        sectorStepper.maxValue = .greatestFiniteMagnitude
        blockStepper.maxValue = .greatestFiniteMagnitude

        // Run a file system check
        errorReport = vol?.check(strict)
        
        update()
    }
    
    override func windowDidLoad() {
                            
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
        cylinderField.stringValue      = String(format: "%d", cylinderNr)
        cylinderStepper.integerValue   = cylinderNr
        headField.stringValue          = String(format: "%d", headNr)
        headStepper.integerValue       = headNr
        trackField.stringValue         = String(format: "%d", trackNr)
        trackStepper.integerValue      = trackNr
        sectorField.stringValue        = String(format: "%d", sectorNr)
        sectorStepper.integerValue     = sectorNr
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
    
    func updateDiskIcon() {

        if driveNr == nil {
            
            diskIcon.image = NSImage(named: "hdf")!
            virusIcon.isHidden = true
            decontaminationButton.isHidden = true
            return
        }
            
        var name = ""
            
        if ext != nil { name = isHD ? "hd_other" : "dd_other" }
        if adf != nil { name = isHD ? "hd_adf" : "dd_adf" }
        if img != nil { name = "dd_dos" }
                
        if drive!.hasWriteProtectedDisk() { name += "_protected" }
        
        diskIcon.image = NSImage(named: name)!
        virusIcon.isHidden = !hasVirus
        decontaminationButton.isHidden = !hasVirus
    }
    
    func updateTitleText() {
        
        var text = "Raw MFM stream"
        var color = NSColor.textColor
        
        if driveNr == nil {
            
            text = "Amiga Hard Drive"
            color = .textColor
            
        } else if adf != nil {
            
            text = "Amiga Disk"
            color = .textColor
                
        } else if img != nil {
                
            text = "PC Disk"
            color = .textColor
        }
        
        title.stringValue = text
        title.textColor = color
    }

    func updateTrackAndSectorInfo() {
        
        var text = "Unknown track and sector format"
        var color = NSColor.warningColor
        
        if driveNr == nil {
            
            let blocks = vol!.numBlocks
            let capacity = blocks / 2000
            text = "\(capacity) MB (\(blocks) sectors)"
            color = NSColor.secondaryLabelColor
            
        } else if adf != nil {
            
            text = adf!.layoutInfo
            color = NSColor.secondaryLabelColor
            
        } else if adf != nil {
            
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
            
            if let errors = errorReport?.corruptedBlocks, errors > 0 {
                
                let blocks = errors == 1 ? "block" : "blocks"
                text += " with \(errors) corrupted \(blocks)"
                color = .warningColor
            }
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
    // Action methods
    //

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

extension DiskInspectorDialog: NSWindowDelegate {
    
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

extension DiskInspectorDialog: NSTableViewDataSource {
    
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
            if let byte = adf?.readByte(blockNr, offset: 16 * row + col) {
                return String(format: "%02X", byte)
            }
            if let byte = img?.readByte(blockNr, offset: 16 * row + col) {
                return String(format: "%02X", byte)
            }
        } else {
            return String(format: "%X", row)
        }
        
        return ""
    }
}

extension DiskInspectorDialog: NSTableViewDelegate {
    
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