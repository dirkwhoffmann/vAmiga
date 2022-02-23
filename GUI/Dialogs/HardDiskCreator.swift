// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class HardDiskCreator: DialogController {
        
    @IBOutlet weak var diskIcon: NSImageView!
    @IBOutlet weak var virusIcon: NSImageView!
    @IBOutlet weak var capacity: NSPopUpButton!
    @IBOutlet weak var fileSystem: NSPopUpButton!
    @IBOutlet weak var bootBlock: NSPopUpButton!

    @IBOutlet weak var bootBlockText: NSTextField!
    @IBOutlet weak var cylinderText: NSTextField!
    @IBOutlet weak var headText: NSTextField!
    @IBOutlet weak var sectorText: NSTextField!
    @IBOutlet weak var bsizeText: NSTextField!

    @IBOutlet weak var cylinderField: NSTextField!
    @IBOutlet weak var headField: NSTextField!
    @IBOutlet weak var sectorField: NSTextField!
    @IBOutlet weak var bsizeField: NSTextField!
    
    @IBOutlet weak var cylinderStepper: NSStepper!
    @IBOutlet weak var headStepper: NSStepper!
    @IBOutlet weak var sectorStepper: NSStepper!
    @IBOutlet weak var bsizeStepper: NSStepper!

    var nr = 0

    var cylinders = 0
    var heads = 0
    var sectors = 0
    var bsize = 0
        
    var drive: HardDriveProxy? { amiga.dh(nr) }
    var hasVirus: Bool { return bootBlock.selectedTag() >= 3 }
    
    //
    // Selecting a block
    //
    
    func setCylinder(_ newValue: Int) {
        
        if newValue != cylinders {

            cylinders = newValue.clamped(1, 65536)
            update()
        }
    }
    
    func setHead(_ newValue: Int) {
        
        if newValue != heads {

            heads = newValue.clamped(1, 64)
            update()
        }
    }
    
    func setSector(_ newValue: Int) {
        
        if newValue != sectors {
              
            sectors = newValue.clamped(1, 128)
            update()
        }
    }

    func setBsize(_ newValue: Int) {
        
        if newValue != bsize {
              
            bsize = newValue.clamped(1, 1024)
            update()
        }
    }

    //
    // Starting up
    //
    
    func showSheet(forDrive nr: Int) {
        
        track()
        
        self.nr = nr
        super.showSheet()
    }
            
    override public func awakeFromNib() {
        
        track()
        super.awakeFromNib()
                
        // Configure elements
        cylinderStepper.maxValue = .greatestFiniteMagnitude
        headStepper.maxValue = .greatestFiniteMagnitude
        sectorStepper.maxValue = .greatestFiniteMagnitude
        bsizeStepper.maxValue = .greatestFiniteMagnitude
        
        setCapacity(mb: capacity.selectedTag())
        update()
    }
    
    override func windowDidLoad() {

        track()
    }
    
    override func sheetDidShow() {
     
        track()
    }
    
    func setCapacity(mb: Int) {
        
        if mb != 0 {
            
            heads = 2
            sectors = 32
            bsize = 512
            cylinders = (mb * 1024 * 1024) / (heads * sectors * bsize)
        }
    }
    
    //
    // Updating the displayed information
    //
    
    func update() {
          
        let custom = capacity.selectedTag() == 0
        
        // Update icons
        virusIcon.isHidden = !hasVirus
        
        // Update text fields and steppers
        cylinderField.stringValue      = String(format: "%d", cylinders)
        cylinderStepper.integerValue   = cylinders
        headField.stringValue          = String(format: "%d", heads)
        headStepper.integerValue       = heads
        sectorField.stringValue        = String(format: "%d", sectors)
        sectorStepper.integerValue     = sectors
        bsizeField.stringValue         = String(format: "%d", bsize)
        bsizeStepper.integerValue      = bsize
        
        // Disable some controls
        let controls: [NSControl: Bool] = [
            
            bootBlock: fileSystem.selectedTag() != 0,
            cylinderField: custom,
            cylinderStepper: custom,
            headField: custom,
            headStepper: custom,
            sectorField: custom,
            sectorStepper: custom,
            bsizeField: custom,
            bsizeStepper: custom
        ]
        
        for (control, enabled) in controls {
            control.isEnabled = enabled
        }

        // Recolor some labels
        let labels: [NSTextField: Bool] = [
            
            bootBlockText: fileSystem.selectedTag() != 0,
            cylinderText: custom,
            headText: custom,
            sectorText: custom,
            bsizeText: custom
        ]
        
        for (label, enabled) in labels {
            label.textColor = enabled ? .labelColor : .secondaryLabelColor
        }
    }
        
    //
    // Action methods
    //

    @IBAction func capacityAction(_ sender: NSPopUpButton!) {
        
        setCapacity(mb: sender.selectedTag())
        update()
    }

    @IBAction func fileSystemAction(_ sender: NSPopUpButton!) {
        
        track()
        update()
    }

    @IBAction func bootBlockAction(_ sender: NSPopUpButton!) {
        
        track()
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
    
    @IBAction func bsizeAction(_ sender: NSTextField!) {
        
        setBsize(sender.integerValue)
    }
    
    @IBAction func bsizeStepperAction(_ sender: NSStepper!) {
        
        setBsize(sender.integerValue)
    }
    
    @IBAction func attachAction(_ sender: Any!) {
        
        let fs: FSVolumeType =
        fileSystem.selectedTag() == 0 ? .NODOS :
        fileSystem.selectedTag() == 1 ? .OFS : .FFS
        
        let bb: BootBlockId =
        bootBlock.selectedTag() == 0 ? .NONE :
        bootBlock.selectedTag() == 1 ? .AMIGADOS_13 :
        bootBlock.selectedTag() == 2 ? .AMIGADOS_20 :
        bootBlock.selectedTag() == 3 ? .SCA : .BYTE_BANDIT
        
        do {
            
            try drive?.attach(c: cylinders, h: heads, s: sectors, b: bsize)
            try drive?.format(fs: fs, bb: bb)
            hideSheet()
            
        } catch let error as VAError {
            error.warning("Unable to attach hard drive.")
        } catch {
            fatalError()
        }
    }
}
