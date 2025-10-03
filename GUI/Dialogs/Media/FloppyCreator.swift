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
class FloppyCreator: DialogController {
        
    @IBOutlet weak var diskIcon: NSImageView!
    @IBOutlet weak var virusIcon: NSImageView!

    @IBOutlet weak var capacity: NSPopUpButton!
    @IBOutlet weak var fileSystem: NSPopUpButton!
    @IBOutlet weak var bootBlock: NSPopUpButton!
    @IBOutlet weak var bootBlockLabel: NSTextField!
    @IBOutlet weak var nameLabel: NSTextField!
    @IBOutlet weak var nameField: NSTextField!
    @IBOutlet weak var importLabel: NSTextField!
    @IBOutlet weak var importButton: NSButton!
    @IBOutlet weak var urlField: NSTextField!
    
    @IBOutlet weak var cylinderField: NSTextField!
    @IBOutlet weak var headField: NSTextField!
    @IBOutlet weak var sectorField: NSTextField!

    var nr = 0
    var diskType: String!

    var drive: FloppyDriveProxy? { emu.df(nr) }
    var hasVirus: Bool { return bootBlock.selectedTag() >= 3 }
    
    var defaultImage: NSImage?
    var importURL: URL?
    let myOpenPanel = MyOpenPanel()
    
    //
    // Starting up
    //
    
    func showSheet(forDrive nr: Int) {
                
        self.nr = nr
        super.showAsSheet()
    }
            
    override func dialogWillShow() {
        
        super.dialogWillShow()
        
        let type = emu.get(.DRIVE_TYPE, drive: nr)
        switch FloppyDriveType(rawValue: type) {
            
        case .DD_35:

            capacity.lastItem?.title = "3.5\" DD"
            cylinderField.integerValue = 80
            headField.integerValue = 2
            sectorField.integerValue = 11
            defaultImage = NSImage(named: "dd_adf")
            
        case .HD_35:
            
            capacity.lastItem?.title = "3.5\" HD"
            cylinderField.integerValue = 80
            headField.integerValue = 2
            sectorField.integerValue = 22
            defaultImage = NSImage(named: "hd_adf")
            
        case .DD_525:
            
            capacity.lastItem?.title = "5.25\" DD"
            cylinderField.integerValue = 80
            headField.integerValue = 2
            sectorField.integerValue = 11
            defaultImage = NSImage(named: "dd_adf")
            
        default:
            fatalError()
        }

        // Preselect OFS as default file system
        fileSystem.selectItem(withTag: 1)
        
        update()
    }
    
    //
    // Updating the displayed information
    //
    
    var icon: NSImage? {
        
        if importURL != nil { return NSImage(named: "NSFolder") }
        return defaultImage
    }
    
    func update() {
                  
        let nodos = fileSystem.selectedTag() == 0
        
        // Remove the import URL when no file system is selected
        if nodos { importURL = nil }
        
        // Update icon
        diskIcon.image = icon
        urlField.stringValue = importURL?.path ?? ""
        
        // Hide some controls
        let controls: [NSControl: Bool] = [
            
            virusIcon: !hasVirus,
            bootBlock: nodos,
            bootBlockLabel: nodos,
            nameField: nodos,
            nameLabel: nodos,
            importLabel: nodos,
            importButton: nodos
        ]
        
        for (control, hidden) in controls {
            control.isHidden = hidden
        }
    }
        
    //
    // Action methods
    //

    @IBAction func capacityAction(_ sender: NSPopUpButton!) {
        
        update()
    }

    @IBAction func fileSystemAction(_ sender: NSPopUpButton!) {
        
        update()
    }

    @IBAction func bootBlockAction(_ sender: NSPopUpButton!) {
        
        update()
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
    
    @IBAction func insertAction(_ sender: Any!) {
        
        let fs: FSVolumeType =
        fileSystem.selectedTag() == 1 ? .OFS :
        fileSystem.selectedTag() == 2 ? .FFS : .NODOS
        
        let bb: BootBlockId =
        bootBlock.selectedTag() == 1 ? .AMIGADOS_13 :
        bootBlock.selectedTag() == 2 ? .AMIGADOS_20 :
        bootBlock.selectedTag() == 3 ? .SCA :
        bootBlock.selectedTag() == 4 ? .BYTE_BANDIT : .NONE
        
        let name = nameField.stringValue
        debug(.media, "Dos = \(fs) Boot = \(bb) Name = \(name)")
        
        do {

            try drive?.insertNew(fileSystem: fs, bootBlock: bb, name: name, url: importURL)
            mm.clearRecentlyExportedDiskURLs(df: nr)
            hide()
            
        } catch {
            
            parent.showAlert(.cantInsert, error: error, window: window)
        }
    }
}

//
// Drop view
//

@MainActor
class DfDropView: NSImageView {
    
    @IBOutlet var parent: FloppyCreator!

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
            if parent.fileSystem.selectedTag() == 0 { parent.fileSystem.selectItem(withTag: 1); }
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

