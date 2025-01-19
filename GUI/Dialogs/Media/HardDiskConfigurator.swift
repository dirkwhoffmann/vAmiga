// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
class HardDiskConfigurator: DialogController {
        
    @IBOutlet weak var geometryPopup: NSPopUpButton!

    @IBOutlet weak var cylinderText: NSTextField!
    @IBOutlet weak var headText: NSTextField!
    @IBOutlet weak var sectorText: NSTextField!

    @IBOutlet weak var cylinderField: NSTextField!
    @IBOutlet weak var headField: NSTextField!
    @IBOutlet weak var sectorField: NSTextField!
    
    @IBOutlet weak var cylinderStepper: NSStepper!
    @IBOutlet weak var headStepper: NSStepper!
    @IBOutlet weak var sectorStepper: NSStepper!

    @IBOutlet weak var warningText: NSTextField!
    @IBOutlet weak var okButton: NSButton!

    var nr = 0

    // Original geometry
    var origCyls = 0
    var origHeads = 0
    var origSectors = 0
    var origBsize = 0

    // Selected geometry
    var cyls = 0
    var heads = 0
    var sectors = 0
    var bsize = 0
    
    var drive: HardDriveProxy { emu.hd(nr)! }
    
    //
    // Selecting a block
    //
    
    func setCylinder(_ newValue: Int) {
        
        if newValue != cyls {

            cyls = newValue.clamped(1, 65536)
            update()
        }
    }
    
    func setHead(_ newValue: Int) {
        
        if newValue != heads {

            heads = newValue.clamped(1, 16)
            update()
        }
    }
    
    func setSector(_ newValue: Int) {
        
        if newValue != sectors {
              
            sectors = newValue.clamped(16, 64)
            update()
        }
    }

    //
    // Starting up
    //
    
    func show(forDrive nr: Int) {

        self.nr = nr
        
        let traits = drive.traits
        origCyls = traits.cylinders
        origHeads = traits.heads
        origSectors = traits.sectors
        origBsize = traits.bsize

        cyls = origCyls
        heads = origHeads
        sectors = origSectors
        bsize = origBsize

        super.showAsSheet()
    }
            
    override func dialogWillShow() {
        
        super.dialogWillShow()
                
        cylinderStepper.maxValue = .greatestFiniteMagnitude
        headStepper.maxValue = .greatestFiniteMagnitude
        sectorStepper.maxValue = .greatestFiniteMagnitude
        warningText.textColor = .warning

        geometryPopup.removeAllItems()
        geometryPopup.addItem(withTitle: "Custom")
        geometryPopup.item(at: 0)!.tag = 0
        
        if let geometries = drive.geometries() as? [Int] {
            
            for (i, geo) in geometries.enumerated() {
                
                let c = (geo >> 32)
                let h = (geo >> 16) & 0xFFFF
                let s = geo & 0xFFFF
                                
                geometryPopup.addItem(withTitle: "\(c) - \(h) - \(s)")
                geometryPopup.item(at: i + 1)!.tag = geo
            }
            
            geometryPopup.autoenablesItems = false
        }
        
        update()
    }
        
    //
    // Updating the displayed information
    //
    
    func update() {
                  
        let traits = drive.traits
        let custom = geometryPopup.selectedTag() == 0

        // Update text fields and steppers
        cylinderField.stringValue      = String(format: "%d", cyls)
        cylinderStepper.integerValue   = cyls
        headField.stringValue          = String(format: "%d", heads)
        headStepper.integerValue       = heads
        sectorField.stringValue        = String(format: "%d", sectors)
        sectorStepper.integerValue     = sectors
        
        // Disable some controls
        let controls: [NSControl: Bool] = [
            
            cylinderField: custom,
            cylinderStepper: custom,
            headField: custom,
            headStepper: custom,
            sectorField: custom,
            sectorStepper: custom
        ]
        
        for (control, enabled) in controls {
            control.isEnabled = enabled
        }

        // Recolor some labels
        let labels: [NSTextField: Bool] = [
            
            cylinderText: custom,
            headText: custom,
            sectorText: custom
        ]
        
        for (label, enabled) in labels {
            label.textColor = enabled ? .labelColor : .secondaryLabelColor
        }
        
        // Check if the geometry is consistent with the HDF
        let matching = cyls * heads * sectors * bsize == traits.bytes
        warningText.isHidden = matching
        okButton.isEnabled = matching
    }
        
    //
    // Action methods
    //

    @IBAction func geometryAction(_ sender: NSPopUpButton!) {
                
        let tag = sender.selectedTag()
        
        if tag == 0 {
            
            cyls = origCyls
            heads = origHeads
            sectors = origSectors
            
        } else {

            cyls = (tag >> 32)
            heads = (tag >> 16) & 0xFFFF
            sectors = tag & 0xFFFF
        }
        
        update()
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

    @IBAction func sectorAction(_ sender: NSTextField!) {
        
        setSector(sender.integerValue)
    }
    
    @IBAction func sectorStepperAction(_ sender: NSStepper!) {
        
        setSector(sender.integerValue)
    }
        
    @IBAction override func okAction(_ sender: Any!) {
        
        do {
            try drive.changeGeometry(c: cyls, h: heads, s: sectors)
            hide()
            
        } catch {
            parent.showAlert(.cantChangeGeometry, error: error, window: window)
        }
    }
}
