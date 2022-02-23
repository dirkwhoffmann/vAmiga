// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// swiftlint:disable force_try

class DiskInspector: DialogController {
        
    var myDocument: MyDocument { return parent.mydocument! }

    @IBOutlet weak var icon: NSImageView!
    @IBOutlet weak var virus: NSImageView!
    @IBOutlet weak var title: NSTextField!
    @IBOutlet weak var layoutInfo: NSTextField!
    @IBOutlet weak var volumeInfo: NSTextField!
    @IBOutlet weak var bootInfo: NSTextField!
    @IBOutlet weak var decontaminationButton: NSButton!
    @IBOutlet weak var geometryText: NSTextField!
    @IBOutlet weak var geometryPopup: NSPopUpButton!

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
    
    var nr = -1
    
    // Returns the inspected floppy or hard drive
    var dfn: DriveProxy { return amiga.df(nr)! }
    var dhn: HardDriveProxy { return amiga.dh(nr)! }
    
    // The drive geometry (cylinders, heads, sectors, tracks, blocks)
    var c = 0
    var h = 0
    var s = 0
    var t = 0
    var b = 0
    
    var upperCyl: Int { return c > 0 ? c - 1 : 0 }
    var upperHead: Int { return h > 0 ? h - 1 : 0 }
    var upperSector: Int { return s > 0 ? s - 1 : 0 }
    var upperTrack: Int { return t > 0 ? t - 1 : 0 }
    var upperBlock: Int { return b > 0 ? b - 1 : 0 }

    // Results of the different decoders
    var hdf: HDFFileProxy?
    var adf: ADFFileProxy?
    var img: IMGFileProxy?
    var ext: EXTFileProxy?
    var vol: FSDeviceProxy?

    // Geometry
    var errorReport: FSErrorReport?
    
    var selection: Int?
    var selectedRow: Int? { return selection == nil ? nil : selection! / 16 }
    var selectedCol: Int? { return selection == nil ? nil : selection! % 16 }
    var strict: Bool { return strictButton.state == .on }
                
    /*
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
    */
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
    // Starting up
    //
    
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

        initDriveGeometry()
        
        super.showSheet()
    }
    
    func showSheet(hardDrive nr: Int) {
        
        track()
        
        self.nr = nr

        // Run the HDF decoder
        hdf = try? HDFFileProxy.make(hdr: dhn) as HDFFileProxy
                        
        // Try to decode the file system from the HDF
        if hdf != nil { vol = try? FSDeviceProxy.make(withHDF: hdf!) }
        
        initDriveGeometry()
        
        super.showSheet()
    }
    
    func initDriveGeometry() {
        
        // Read CHS geometry (cylinders, heads, sectors)
        if hdf != nil {
            c = hdf!.numCyls; h = hdf!.numSectors; s = hdf!.numSectors
            
        } else if adf != nil {
            c = adf!.numCyls; h = adf!.numSectors; s = adf!.numSectors
            
        } else if img != nil {
            c = img!.numCyls; h = img!.numSectors; s = img!.numSectors
            
        } else if ext != nil {
            c = ext!.numCyls; h = ext!.numSectors; s = ext!.numSectors
        }
        
        // Update derived values (tracks, blocks)
        t = c * h
        b = t * s
    }
    
    override public func awakeFromNib() {
        
        track()
        super.awakeFromNib()
        
        // Register to receive mouse click events
        previewTable.action = #selector(clickAction(_:))

        // Hide some elements
        geometryText.isHidden = hdf == nil
        geometryPopup.isHidden = hdf == nil
        
        // Configure elements
        sectorStepper.maxValue = .greatestFiniteMagnitude
        blockStepper.maxValue = .greatestFiniteMagnitude
        
        // Run a file system check
        errorReport = vol?.check(strict)
        
        update()
    }
    
    override func windowDidLoad() {
                 
        // Initialize the geometry selector
        geometryPopup.removeAllItems()
        geometryPopup.addItem(withTitle: "Custom")
        geometryPopup.item(at: 0)!.tag = 0
        
        if hdf != nil {
            
            if let geometries = dhn.test() as? [Int] {
                
                for (i, geo) in geometries.enumerated() {
                    
                    let c = (geo >> 32)
                    let h = (geo >> 16) & 0xFFFF
                    let s = geo & 0xFFFF
                    
                    geometryPopup.addItem(withTitle: "\(c) - \(h) - \(s)")
                    geometryPopup.item(at: i + 1)!.tag = geo
                }
                
                track()
                geometryPopup.autoenablesItems = false
            }
        }
        
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

        if hdf != nil {
            
            icon.image = NSImage(named: "hdf")!
            virus.isHidden = true
            decontaminationButton.isHidden = true
            return
        }
            
        var name = ""
            
        if ext != nil { name = isHD ? "hd_other" : "dd_other" }
        if adf != nil { name = isHD ? "hd_adf" : "dd_adf" }
        if img != nil { name = "dd_dos" }
        
        if name != "" && dfn.hasWriteProtectedDisk() { name += "_protected" }
        
        icon.image = NSImage(named: name != "" ? name : "biohazard")
        virus.isHidden = !hasVirus
        decontaminationButton.isHidden = !hasVirus
    }
    
    func updateTitleText() {
        
        var text = "Raw MFM stream"
        var color = NSColor.textColor
        
        if hdf != nil {
            
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
        
        if hdf != nil {
            
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
    // Helper methods
    //
    
    func setCylinder(_ newValue: Int) {
        
        if newValue != cylinderNr {

            let value = newValue.clamped(0, upperCyl)

            cylinderNr = value
            trackNr    = cylinderNr * 2 + headNr
            blockNr    = trackNr * s + sectorNr
            
            selection = nil
            update()
        }
    }
    
    func setHead(_ newValue: Int) {
        
        if newValue != headNr {
                        
            let value = newValue.clamped(0, upperHead)

            headNr     = value
            trackNr    = cylinderNr * 2 + headNr
            blockNr    = trackNr * s + sectorNr
            
            selection = nil
            update()
        }
    }
    
    func setTrack(_ newValue: Int) {
        
        if newValue != trackNr {
                   
            let value = newValue.clamped(0, upperTrack)
            
            trackNr    = value
            cylinderNr = trackNr / 2
            headNr     = trackNr % 2
            blockNr    = trackNr * s + sectorNr

            selection = nil
            update()
        }
    }

    func setSector(_ newValue: Int) {
        
        if newValue != sectorNr {
                  
            let value = newValue.clamped(0, upperSector)
            
            sectorNr   = value
            blockNr    = trackNr * s + sectorNr
            
            selection = nil
            update()
        }
    }

    func setBlock(_ newValue: Int) {
        
        if newValue != blockNr {
                        
            let value = newValue.clamped(0, upperBlock)

            blockNr    = value
            trackNr    = blockNr / s
            sectorNr   = blockNr % s
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
    // Action methods
    //

    @IBAction func geometryAction(_ sender: NSPopUpButton!) {
        
        track()
        
        let tag = sender.selectedTag()
        c = (tag >> 32)
        h = (tag >> 16) & 0xFFFF
        s = tag & 0xFFFF
            
        update()
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
    
    @IBAction override func okAction(_ sender: Any!) {
        
        // Update geometry if a hard drive has been analyzed
        if hdf != nil { try! dhn.changeGeometry(c: c, h: h, s: h) }
        
        track()
        hideSheet()
    }
    
        @IBAction override func cancelAction(_ sender: Any!) {
        
        track()
        hideSheet()
    }
}

//
// Extensions
//

extension DiskInspector: NSWindowDelegate {
    
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

extension DiskInspector: NSTableViewDataSource {
    
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

extension DiskInspector: NSTableViewDelegate {
    
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
