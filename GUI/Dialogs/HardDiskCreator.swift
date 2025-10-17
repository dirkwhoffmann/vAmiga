// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
class HardDiskCreator: DialogController {
        
    @IBOutlet weak var diskIcon: NSImageView!
    
    @IBOutlet weak var capacity: NSPopUpButton!
    @IBOutlet weak var fileSystem: NSPopUpButton!
    @IBOutlet weak var nameLabel: NSTextField!
    @IBOutlet weak var nameField: NSTextField!
    @IBOutlet weak var importLabel: NSTextField!
    @IBOutlet weak var importButton: NSButton!
    @IBOutlet weak var urlField: NSTextField!

    @IBOutlet weak var cylinderText: NSTextField!
    @IBOutlet weak var headText: NSTextField!
    @IBOutlet weak var sectorText: NSTextField!

    @IBOutlet weak var cylinderField: NSTextField!
    @IBOutlet weak var headField: NSTextField!
    @IBOutlet weak var sectorField: NSTextField!
    
    @IBOutlet weak var cylinderStepper: NSStepper!
    @IBOutlet weak var headStepper: NSStepper!
    @IBOutlet weak var sectorStepper: NSStepper!

    var nr = 0
    var cylinders = 0
    var heads = 0
    var sectors = 0
    var bsize = 0
    var fs = FSVolumeType.NODOS
    
    var drive: HardDriveProxy? { emu.hd(nr) }
    
    var importURL: URL?
    let myOpenPanel = MyOpenPanel()
    
    //
    // Selecting a block
    //
    
    func setCylinder(_ newValue: Int) {
        
        if newValue != cylinders {

            cylinders = newValue.clamped(Int(VAMIGA.HDR.C_MIN), Int(VAMIGA.HDR.C_MAX))
            update()
        }
    }
    
    func setHead(_ newValue: Int) {
        
        if newValue != heads {

            heads = newValue.clamped(Int(VAMIGA.HDR.H_MIN), Int(VAMIGA.HDR.H_MAX))
            update()
        }
    }
    
    func setSector(_ newValue: Int) {
        
        if newValue != sectors {
              
            sectors = newValue.clamped(Int(VAMIGA.HDR.S_MIN), Int(VAMIGA.HDR.S_MAX))
            update()
        }
    }
    
    func setFS(_ newValue: Int) {
        
        fs = newValue == 0 ? .NODOS : newValue == 1 ? .OFS : .FFS
        update()
    }
        
    //
    // Starting up
    //
    
    func show(forDrive nr: Int) {
                
        self.nr = nr
        super.showAsSheet()
    }
            
    override func dialogWillShow() {
        
        super.dialogWillShow()
                
        // Configure elements
        cylinderStepper.maxValue = .greatestFiniteMagnitude
        headStepper.maxValue = .greatestFiniteMagnitude
        sectorStepper.maxValue = .greatestFiniteMagnitude
        capacity.selectItem(withTag: 8)
        setCapacity(mb: capacity.selectedTag())

        update()
    }
    
    func setCapacity(mb: Int) {
        
        if mb > 0 {
            
            bsize = 512
            sectors = 32
            heads = 1
            cylinders = (mb * 1024 * 1024) / (heads * sectors * bsize)
            
            while cylinders > 1024 {
                
                cylinders /= 2
                heads *= 2
            }
        }

        update()
    }
    
    //
    // Updating the displayed information
    //
    
    func update() {
        
        let custom = capacity.selectedTag() == 0
        let nodos = fs == .NODOS
        
        // Remove the import URL when no file system is selected
        if nodos { importURL = nil }

        // Update icon
        diskIcon.image = NSImage(named: importURL != nil ? "NSFolder" : "hdf")
        urlField.stringValue = importURL?.path ?? ""
        
        // Update file system
        fileSystem.selectItem(withTag: fs == .NODOS ? 0 : fs == .OFS ? 1 : 2)
            
        // Update text fields and steppers
        cylinderField.stringValue      = String(format: "%d", cylinders)
        cylinderStepper.integerValue   = cylinders
        headField.stringValue          = String(format: "%d", heads)
        headStepper.integerValue       = heads
        sectorField.stringValue        = String(format: "%d", sectors)
        sectorStepper.integerValue     = sectors
        
        // Disable some controls
        let controls1: [NSControl: Bool] = [
            
            cylinderField: custom,
            cylinderStepper: custom,
            headField: custom,
            headStepper: custom,
            sectorField: custom,
            sectorStepper: custom
        ]
        
        for (control, enabled) in controls1 {
            control.isEnabled = enabled
        }

        // Hide some controls
        let controls2: [NSControl: Bool] = [
            
            nameLabel: nodos,
            nameField: nodos,
            importLabel: nodos,
            importButton: nodos
        ]

        for (control, hidden) in controls2 {
            control.isHidden = hidden
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
    }
        
    //
    // Action methods
    //

    @IBAction func capacityAction(_ sender: NSPopUpButton!) {
        
        setCapacity(mb: sender.selectedTag())
    }

    @IBAction func fileSystemAction(_ sender: NSPopUpButton!) {
        
        setFS(sender.selectedTag())
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
    
    @IBAction func importAction(_ sender: NSButton!) {
        
        myOpenPanel.configure(types: [ .directory ], prompt: "Import")
        myOpenPanel.panel.canChooseDirectories = true
        myOpenPanel.open(for: window, { result in
            
            if result == .OK, let url = self.myOpenPanel.url {
                
                self.importURL = url
                self.update()
            }
        })
    }
    
    @IBAction func attachAction(_ sender: Any!) {
        
        let fs: FSVolumeType =
        fileSystem.selectedTag() == 0 ? .NODOS :
        fileSystem.selectedTag() == 1 ? .OFS : .FFS
               
        let name = nameField.stringValue
        
        do {
            try drive?.attach(c: cylinders, h: heads, s: sectors, b: bsize)
            try drive?.format(fs: fs, name: name)
            if let url = importURL { try drive?.importFiles(url: url) }

            hide()
            
        } catch {
            
            parent.showAlert(.cantAttach, error: error, window: window)
        }
    }
}

//
// Drop view
//

@MainActor
class HdDropView: NSImageView {
    
    @IBOutlet var parent: HardDiskCreator!

    var oldImage: NSImage?
    
    override init(frame frameRect: NSRect) { super.init(frame: frameRect); commonInit() }
    required init?(coder: NSCoder) { super.init(coder: coder); commonInit() }
    
    func commonInit() {

        registerForDraggedTypes([NSPasteboard.PasteboardType.fileURL])
    }
    
    func acceptDragSource(url: URL) -> Bool { return false }

    override func draggingEntered(_ sender: NSDraggingInfo) -> NSDragOperation {

        if let url = sender.url, url.hasDirectoryPath {
            
            parent.diskIcon.image = NSImage(named: "NSFolder")
            return .copy
        }
        
        return NSDragOperation()
    }
    
    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        
        if let url = sender.url, url.hasDirectoryPath {
            
            parent.importURL = url
            if parent.fs == .NODOS { parent.fs = .OFS }
            parent.update()
            return true
        }
        
        return false
    }

    override func draggingExited(_ sender: NSDraggingInfo?) {

        parent.update()
    }
    
    override func prepareForDragOperation(_ sender: NSDraggingInfo) -> Bool {

        return true
    }
        
    override func concludeDragOperation(_ sender: NSDraggingInfo?) {

        parent.update()
    }
}
