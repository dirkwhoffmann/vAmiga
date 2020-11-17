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
    
    @IBOutlet weak var formatPopup: NSPopUpButton!
    @IBOutlet weak var exportButton: NSButton!
    
    var panel = NSSavePanel()

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

        return "???"
    }

    var subtitleText: String {
                
        let t = numTracks
        let s = numSectors
        let n = numSides == 1 ? "Single" : "Double"

        let den = disk?.diskDensity
        let d = den == .DISK_SD ? "single" : den == .DISK_DD ? "double" : "high"

        return "\(n) sided, \(d) density disk, \(t) tracks with \(s) sectors each"
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
        
        // It it's an ADF, try to extract the file system
        if disk != nil {
            volume = FSVolumeProxy.make(withADF: disk as? ADFFileProxy)
        }
        // If it's not an ADF, try the DOS decoder
        if disk == nil {
            disk = IMGFileProxy.make(withDrive: drive)
        }
        
        // Update text labels
        /*
        if adf != nil {
            selectFormat(0)
            text1.stringValue = "This disk has been identified as an Amiga disk."
            if vol != nil {
                text2.stringValue = "The File System has been parsed successfully."
            } else {
                text2.stringValue = "An unsupported or corrupted File System has been detected."
                text2.textColor = .red
            }
        }
        if img != nil {
            selectFormat(1)
            text1.stringValue = "This disk has been identifies as a DOS disk."
            text2.stringValue = ""
        }
        if adf == nil && img == nil {
            text1.stringValue = "This disk cannot be exported."
            text2.stringValue = "The MFM stream doesn't comply to the Amiga or MS-DOS format."
            text1.textColor = .red
            text2.textColor = .red
        }
        */
                
        /*
        if df == nil {
            df = ADFFileProxy.make(withDrive: proxy)
            if df != nil { selectFormat(0) }
        }
        // Try to decode the disk with the DOS decoder if the ADF decoder failed
        if df == nil {
            df = IMGFileProxy.make(withDrive: proxy)
            if df != nil { selectFormat(1) }
        }

        // Abort if both decoders failed
        if df == nil {
            parent.mydocument.showExportDecodingAlert(driveNr: nr)
            return
        }
        */
        
        // Run panel as sheet
        /*
        if let win = parent.window {
            savePanel.beginSheetModal(for: win, completionHandler: { result in
                if result == .OK {
                    if let url = self.savePanel.url {
                        track("url = \(url)")
                        self.parent.mydocument.export(drive: nr,
                                                      to: url,
                                                      diskFileProxy: self.df!)
                    }
                }
            })
        }
        */
        
        super.showSheet()
    }
    
    override public func awakeFromNib() {
        
        track()
        super.awakeFromNib()
        
        sectorData = Array(repeating: "", count: 512 / bytesPerRow)

        // Start with a shrinked window
        var rect = window!.contentRect(forFrameRect: window!.frame)
        rect.size = CGSize(width: 606, height: shrinkedHeight)
        let frame = window!.frameRect(forContentRect: rect)
        window!.setFrame(frame, display: true)
        
        update()
    }
    
    override func windowDidLoad() {
        
        // Disable unsupported formats in the format selector popup
        formatPopup.autoenablesItems = false
        /*
        formatPopup.item(at: 0)!.isEnabled = adf != nil
        formatPopup.item(at: 1)!.isEnabled = img != nil
        formatPopup.item(at: 2)!.isEnabled = img != nil
        formatPopup.item(at: 3)!.isEnabled = vol != nil
        */

        // shrink()
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
        // r.origin.x = 20
        r.origin.y = 61
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
    @IBAction func exportAction(_ sender: NSButton!) {
        
        track()
        
        // Configure panel
        // panel = NSOpenPanel()
        panel.prompt = "Export"
        panel.title = "Export"
        panel.nameFieldLabel = "Export As:"
        // panel.canChooseDirectories = true
        panel.canCreateDirectories = true
        
        // Open panel as a sheet
        panel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let u = self.panel.url {
                    track("url = \(u)")
                    self.export(url: u)
                }
            }
        })
    }
    
    func export(url: URL) {
    
        track("url = \(url)")
        parent.mydocument.export(drive: driveNr, to: url, diskFileProxy: disk!)

        hideSheet()
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
    
    /*
    func buildHex(count: Int) -> String {
        
        let hexDigits = Array(("0123456789ABCDEF ").utf16)
        var chars: [unichar] = []
        chars.reserveCapacity(3 * count)
        
        for _ in 1 ... count {
            
            if let byte = disk?.read() {
                chars.append(hexDigits[Int(byte / 16)])
                chars.append(hexDigits[Int(byte % 16)])
            } else {
                chars.append(hexDigits[16])
                chars.append(hexDigits[16])
            }
            chars.append(hexDigits[16])
        }
        
        return String(utf16CodeUnits: chars, count: chars.count)
    }
    */
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
        
        if (tableColumn?.identifier)!.rawValue == "data" {
            
            return row < sectorData.count ? sectorData[row] : ""
            // return sectorData[row]
        }
        
        return "???"
    }
}
