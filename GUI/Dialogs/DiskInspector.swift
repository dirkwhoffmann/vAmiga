// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class DiskInspector: DialogController {
        
    var myDocument: MyDocument { return parent.mydocument! }

    @IBOutlet weak var icon: NSImageView!
    @IBOutlet weak var title: NSTextField!
    @IBOutlet weak var subTitle1: NSTextField!
    @IBOutlet weak var subTitle2: NSTextField!
    @IBOutlet weak var subTitle3: NSTextField!
    @IBOutlet weak var cylindersInfo: NSTextField!
    @IBOutlet weak var headsInfo: NSTextField!
    @IBOutlet weak var sectorsInfo: NSTextField!
    @IBOutlet weak var blocksInfo: NSTextField!
    @IBOutlet weak var bsizeInfo: NSTextField!
    @IBOutlet weak var capacityInfo: NSTextField!

    @IBOutlet weak var tabView: NSTabView!

    @IBOutlet weak var previewScrollView: NSScrollView!
    @IBOutlet weak var previewTable: NSTableView!
    @IBOutlet weak var cylinderField: NSTextField!
    @IBOutlet weak var cylinderStepper: NSStepper!
    @IBOutlet weak var headField: NSTextField!
    @IBOutlet weak var headStepper: NSStepper!
    @IBOutlet weak var trackField: NSTextField!
    @IBOutlet weak var trackStepper: NSStepper!
    @IBOutlet weak var sectorField: NSTextField!
    @IBOutlet weak var sectorStepper: NSStepper!
    @IBOutlet weak var blockField: NSTextField!
    @IBOutlet weak var blockStepper: NSStepper!

    @IBOutlet weak var mfmView: NSScrollView!

    // Title and icon of the info section
    var titleString = ""
    var image: NSImage?
    
    // The floppy drive to get MFM data from
    var drive: DriveProxy?
        
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
        
    // The decoder to get the displayed information from
    var decoder: DiskFileProxy?
    
    var selection: Int?
    var selectedRow: Int? { return selection == nil ? nil : selection! / 16 }
    var selectedCol: Int? { return selection == nil ? nil : selection! % 16 }

    var isDD: Bool {
        return adf?.isDD ?? img?.isDD ?? ext?.isDD ?? false
    }
    var isHD: Bool {
        return adf?.isHD ?? img?.isHD ?? ext?.isHD ?? false
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
        
    func show(diskDrive nr: Int) {
        
        track()
        
        drive = amiga.df(nr)
        let protected = drive!.hasWriteProtectedDisk()
        
        titleString = "DF\(nr) - Amiga Floppy Drive"

        // Run the ADF decoder
        adf = try? ADFFileProxy.make(drive: drive!) as ADFFileProxy
        decoder = adf
        
        if decoder == nil {

            // Run the DOS decoder
            img = try? IMGFileProxy.make(drive: drive!) as IMGFileProxy
            decoder = img
        }
        if decoder == nil {
            
            // Run the extended ADF decoder
            ext = try? EXTFileProxy.make(drive: drive!) as EXTFileProxy
            decoder = ext
        }

        image = (decoder as? FloppyFileProxy)?.icon(protected: protected)
        initDriveGeometry()
        showWindow()
    }
    
    func show(hardDrive nr: Int) {
        
        track()
        
        titleString = "HD\(nr) - Amiga Hard Drive"

        // Run the HDF decoder
        hdf = try? HDFFileProxy.make(hdr: amiga.dh(nr)!) as HDFFileProxy
        decoder = hdf
        
        image = NSImage(named: "hdf")!
        initDriveGeometry()
        showWindow()
    }
        
    func initDriveGeometry() {
        
        // Read CHS geometry (cylinders, heads, sectors)
        c = decoder!.numCyls
        h = decoder!.numHeads
        s = decoder!.numSectors
        
        // Update derived values (tracks, blocks)
        t = c * h
        b = t * s
    }
    
    override public func awakeFromNib() {
        
        track()
        super.awakeFromNib()
        
        // Register to receive mouse click events
        previewTable.action = #selector(clickAction(_:))
        
        // Configure elements
        sectorStepper.maxValue = .greatestFiniteMagnitude
        blockStepper.maxValue = .greatestFiniteMagnitude
                
        update()
    }
    
    override func windowDidLoad() {
                 
    }
    
    override func sheetDidShow() {
        
    }
     
    //
    // Updating the displayed information
    //

    func update() {
          
        updateInfo()
                                
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
                
        previewTable.reloadData()
        
        let textView = mfmView.documentView as? NSTextView
        // let storage = textView?.textStorage

        let textStorage = NSTextStorage(string: drive?.readTrackBits(trackNr) ?? "???")
        textView?.layoutManager?.replaceTextStorage(textStorage)
    }

    func updateInfo() {
        
        icon.image = image
        title.stringValue = titleString

        switch decoder {
            
        case is HDFFileProxy: subTitle1.stringValue = "Standard Hard Drive"
        case is ADFFileProxy: subTitle1.stringValue = "Amiga Floppy Disk"
        case is IMGFileProxy: subTitle1.stringValue = "PC Disk"
        case is EXTFileProxy: subTitle1.stringValue = "Amiga Floppy Disk (Ext)"
            
        default:
            subTitle1.stringValue = "Raw MFM stream"
            subTitle2.stringValue = ""
            subTitle3.stringValue = ""
            cylindersInfo.stringValue = "-"
            headsInfo.stringValue = "-"
            sectorsInfo.stringValue = "-"
            capacityInfo.stringValue = ""
            blocksInfo.stringValue = "-"
            bsizeInfo.stringValue = "-"
            return
        }
        
        if let hdf = decoder as? HDFFileProxy {
                        
            let num = hdf.numPartitions
            let rdb = hdf.hasRDB
            subTitle2.stringValue = "\(num) Partition" + (num != 1 ? "s" : "")
            subTitle3.stringValue = (rdb ? "" : "No ") + "Rigid Disk Block found"
        }
        
        if let floppy = decoder as? FloppyFileProxy {
            
            subTitle2.stringValue = floppy.typeInfo + " " + floppy.layoutInfo
            subTitle3.stringValue = ""
            cylindersInfo.integerValue = floppy.numCyls
            headsInfo.integerValue = floppy.numHeads
            sectorsInfo.integerValue = floppy.numSectors
            blocksInfo.integerValue = floppy.numBlocks
            bsizeInfo.integerValue = floppy.bsize
            capacityInfo.stringValue = floppy.describeCapacity
        }
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
    
    //
    // Action methods
    //

    @IBAction func clickAction(_ sender: NSTableView!) {

        track()
        
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
}

//
// Extensions
//

extension DiskInspector: NSTableViewDataSource {
    
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
            return decoder?.asciidump(blockNr, offset: row * 16, len: 16) ?? ""
            
        default:
            if let col = columnNr(tableColumn) {
                
                if let byte = adf?.readByte(blockNr, offset: 16 * row + col) {
                    return String(format: "%02X", byte)
                }
                if let byte = img?.readByte(blockNr, offset: 16 * row + col) {
                    return String(format: "%02X", byte)
                }
            }
            fatalError()
        }
    }
}

extension DiskInspector: NSTableViewDelegate {
    
    /*
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
    */
    
    func tableView(_ tableView: NSTableView, shouldSelectRow row: Int) -> Bool {
        return false
    }
}
