// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Darwin

class FloppyCreator: DialogController {
        
    @IBOutlet weak var diskIcon: NSImageView!
    @IBOutlet weak var virusIcon: NSImageView!

    @IBOutlet weak var capacity: NSPopUpButton!
    @IBOutlet weak var fileSystem: NSPopUpButton!
    @IBOutlet weak var bootBlock: NSPopUpButton!
    @IBOutlet weak var bootBlockLabel: NSTextField!

    @IBOutlet weak var cylinderField: NSTextField!
    @IBOutlet weak var headField: NSTextField!
    @IBOutlet weak var sectorField: NSTextField!

    var nr = 0
    var diskType: String!

    var drive: DriveProxy? { amiga.df(nr) }
    var hasVirus: Bool { return bootBlock.selectedTag() >= 3 }
    
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
        
        let type = amiga.getConfig(.DRIVE_TYPE, drive: nr)
        switch FloppyDriveType(rawValue: type) {
            
        case .DD_35:

            capacity.lastItem?.title = "3.5\" DD"
            cylinderField.integerValue = 80
            headField.integerValue = 2
            sectorField.integerValue = 11
            
        case .HD_35:
            
            capacity.lastItem?.title = "3.5\" HD"
            cylinderField.integerValue = 80
            headField.integerValue = 2
            sectorField.integerValue = 22

        case .DD_525:
            
            capacity.lastItem?.title = "5.25\" DD"
            cylinderField.integerValue = 80
            headField.integerValue = 2
            sectorField.integerValue = 11

        default:
            fatalError()
        }

        update()
    }
    
    override func windowDidLoad() {

        track()
    }
    
    override func sheetDidShow() {
     
        track()
    }
    
    //
    // Updating the displayed information
    //
    
    func update() {
                  
        let formatted = fileSystem.selectedTag() != 0
        
        // Update icons
        virusIcon.isHidden = !hasVirus
                
        // Update boot block selector
        bootBlock.isEnabled = formatted
        bootBlockLabel.textColor = formatted ? .labelColor : .secondaryLabelColor
    }
        
    //
    // Action methods
    //

    @IBAction func capacityAction(_ sender: NSPopUpButton!) {
        
        track()
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
    
    @IBAction func insertAction(_ sender: Any!) {
        
        let fs: FSVolumeType =
        fileSystem.selectedTag() == 1 ? .OFS :
        fileSystem.selectedTag() == 2 ? .FFS : .NODOS
        
        let bb: BootBlockId =
        bootBlock.selectedTag() == 1 ? .AMIGADOS_13 :
        bootBlock.selectedTag() == 2 ? .AMIGADOS_20 :
        bootBlock.selectedTag() == 3 ? .SCA :
        bootBlock.selectedTag() == 4 ? .BYTE_BANDIT : .NONE
        
        track("Dos = \(fs) Boot = \(bb)")
        do {
            try drive?.insertNew(fileSystem: fs, bootBlock: bb)
            myAppDelegate.clearRecentlyExportedDiskURLs(drive: nr)
            hideSheet()
            
        } catch {
            (error as? VAError)?.cantInsert()
        }
    }
}
