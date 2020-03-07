// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class DiskMountDialog: DialogController {
    
    var disk: ADFFileProxy!
    var writeProtect = false
    
    let bytesPerRow = 32
    
    // Cylinder, Head, Track, and Sector
    var _cylinder = 0
    var _head = 0
    var _track = 0
    var _sector = 0
    
    // Preview data
    var sectorData: [String] = []
    
    var shrinked: Bool { return window!.frame.size.height < 300 }
    
    func setCylinder(_ newValue: Int) {

        if newValue >= 0 && newValue < disk.numCylinders() {
            
            let spt = disk.numSectorsPerTrack()
            
            _cylinder = newValue
            _track    = _cylinder * 2 + _head
            _sector   = (_track * spt) + (_sector % spt)
        }
    }
    
    func setHead(_ newValue: Int) {
        
        if newValue >= 0 && newValue < disk.numHeads() {
            
            let spt = disk.numSectorsPerTrack()
            
            _head     = newValue
            _track    = _cylinder * 2 + _head
            _sector   = (_track * spt) + (_sector % spt)
        }
    }
    
    func setTrack(_ newValue: Int) {
        
        if newValue >= 0 && newValue < disk.numTracks() {
            
            let spt = disk.numSectorsPerTrack()
            
            _track    = newValue
            _cylinder = _track / 2
            _head     = _track % 2
            _sector   = (_track * spt) + (_sector % spt)
        }
    }

    func setSector(_ newValue: Int) {
        
        if newValue >= 0 && newValue < disk.numSectors() {
            
            let spt = disk.numSectorsPerTrack()
            
            _sector   = newValue
            _track    = _sector / spt
            _cylinder = _track / 2
            _head     = _track % 2
        }
    }
    
    // Outlets
    @IBOutlet weak var diskIcon: NSImageView!
    @IBOutlet weak var disclosureButton: NSButton!
    @IBOutlet weak var infoText: NSTextField!
    @IBOutlet weak var warningText: NSTextField!
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
    @IBOutlet weak var df0Button: NSButton!
    @IBOutlet weak var df1Button: NSButton!
    @IBOutlet weak var df2Button: NSButton!
    @IBOutlet weak var df3Button: NSButton!

    override func showSheet(completionHandler handler:(() -> Void)? = nil) {
    
        if let attachment = myDocument?.amigaAttachment as? ADFFileProxy {
            
            disk = attachment
            super.showSheet(completionHandler: handler)

            if let sha1 = disk.sha1() {
                track("Checksum: \(sha1)")
            } else {
                track("Failed to compute SHA1 checksum")
            }
        }
    }
    
    override public func awakeFromNib() {

        track()
        sectorData = Array(repeating: "", count: 512 / bytesPerRow)

        var rect = window!.frame
        rect.size.height = 176 + 20
        window!.setFrame(rect, display: true)
    }
    
    override func windowDidLoad() {
        
    }
 
    func setHeight(_ newHeight: CGFloat) {

        var rect = window!.frame
        rect.origin.y += rect.size.height - newHeight
        rect.size.height = newHeight
        
        track("\(rect)")
        window?.setFrame(rect, display: true)
        update()
    }
    
    func shrink() { setHeight(176) }
    func expand() { setHeight(378) }
    
    func update() {
        
        let size = window!.frame.size
        let hide = size.height < 300
        
        // Update disk icon
        diskIcon.image = NSImage.init(named: writeProtect ? "adf_protected" : "adf")
        
        // Update text fields
        let typeName = [
            DISK_35_DD.rawValue: "3.5\"DD Amiga",
            DISK_35_DD_PC.rawValue: "3.5\"DD PC",
            DISK_35_HD.rawValue: "3.5\"HD Amiga",
            DISK_35_HD_PC.rawValue: "3.5\"HD PC",
            DISK_525_SD.rawValue: "5.25\"SD PC"
        ]
        let str = typeName[disk.diskType().rawValue]!
        infoText.stringValue = "A byte-accurate image of \(str) diskette."
        let compatible = disk.diskType() == DISK_35_DD
            
        // Update the disclosure button state
        disclosureButton.state = shrinked ? .off : .on
        warningText.isHidden = compatible
        
        // Check for available drives
        let dc = amiga.diskController.getConfig()

        let connected0 = dc.connected.0
        let connected1 = dc.connected.1
        let connected2 = dc.connected.2
        let connected3 = dc.connected.3
        df0Button.isEnabled = compatible && connected0
        df1Button.isEnabled = compatible && connected1
        df2Button.isEnabled = compatible && connected2
        df3Button.isEnabled = compatible && connected3

        // Hide some elements if window is shrinked
        let items: [NSView] = [
            
            previewScrollView,
            cylinderText, cylinderField, cylinderStepper,
            headText, headField, headStepper,
            trackText, trackField, trackStepper,
            sectorText, sectorField, sectorStepper
        ]
        for item in items { item.isHidden = hide }
        
        // Only proceed if window is expanded
        if hide { return }
    
        // Compute size of preview table
        let w = size.width - 40
        let h = size.height - 224
        previewScrollView.frame = NSRect.init(x: 20, y: 61, width: w, height: h)
   
        // Update all elements
        cylinderField.integerValue   = _cylinder
        cylinderStepper.integerValue = _cylinder
        headField.integerValue       = _head
        headStepper.integerValue     = _head
        trackField.integerValue      = _track
        trackStepper.integerValue    = _track
        sectorField.integerValue     = _sector
        sectorStepper.integerValue   = _sector

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
    
    @IBAction func disclosureAction(_ sender: NSButton!) {
        
        shrinked ? expand() : shrink()
    }

    @IBAction func insertDiskAction(_ sender: NSButton!) {
        
        track("insertDiskAction df\(sender.tag)")
        
        amigaProxy?.diskController.insert(sender.tag, adf: disk)
        amigaProxy?.diskController.setWriteProtection(sender.tag, value: writeProtect)

        myController?.renderer.rotateDown()
        hideSheet()
    }
    
    @IBAction func writeProtectAction(_ sender: NSButton!) {
        
        writeProtect = sender.state == .on
        update()
    }
}

//
// NSTableView delegate and data source
//

extension DiskMountDialog: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView,
                   willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        // let c = cell as! NSTextFieldCell
        // c.font = cbmfont
        // c.textColor = .red
    }
}
extension DiskMountDialog: NSWindowDelegate {
    
    func windowDidResize(_ notification: Notification) {
        
        update()
    }
}

extension DiskMountDialog: NSTableViewDataSource {
    
    func buildHex(count: Int) -> String {
        
        let hexDigits = Array(("0123456789ABCDEF ").utf16)
        var chars: [unichar] = []
        chars.reserveCapacity(3 * count)
        
        for _ in 1 ... count {
            
            let byte = disk.read()
            chars.append(hexDigits[Int(byte / 16)])
            chars.append(hexDigits[Int(byte % 16)])
            chars.append(hexDigits[16])
        }
        
        return String(utf16CodeUnits: chars, count: chars.count)
    }
    
    func buildStrings() {
        
        disk.seekSector(_sector)
        for i in 0 ... ((512 / bytesPerRow) - 1) {
            sectorData[i] = buildHex(count: bytesPerRow)
        }
    }
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        
        return 8
    }
    
    func tableView(_ tableView: NSTableView,
                   objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        if (tableColumn?.identifier)!.rawValue == "data" {
            
            return sectorData[row]
        }
        
        return "???"
    }
}
