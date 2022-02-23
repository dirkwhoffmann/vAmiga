// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Darwin

class FloppyDiskCreator: DialogController {
        
    @IBOutlet weak var diskIcon: NSImageView!
    @IBOutlet weak var virusIcon: NSImageView!

    @IBOutlet weak var capacity: NSPopUpButton!
    @IBOutlet weak var fileSystem: NSPopUpButton!
    @IBOutlet weak var bootBlock: NSPopUpButton!
    @IBOutlet weak var bootBlockText: NSTextField!

    var nr = 0
        
    var drive: DriveProxy? { amiga.df(nr) }
    var hasVirus: Bool { return bootBlock.selectedTag() >= 3 }
    
    var diskType: String!
    
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
        
        switch DriveType.init(rawValue: type) {
            
        case .DD_35:    capacity.lastItem?.title = "3.5\" DD"
        case .HD_35:    capacity.lastItem?.title = "3.5\" HD"
        case .DD_525:   capacity.lastItem?.title = "5.25\" DD"

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
                  
        // Update icons
        virusIcon.isHidden = !hasVirus
                
        // Disable some controls
        let controls: [NSControl: Bool] = [
            
            bootBlock: fileSystem.selectedTag() != 0
        ]
        
        for (control, enabled) in controls {
            control.isEnabled = enabled
        }

        // Recolor some labels
        let labels: [NSTextField: Bool] = [
            
            bootBlockText: fileSystem.selectedTag() != 0
        ]
        
        for (label, enabled) in labels {
            label.textColor = enabled ? .labelColor : .secondaryLabelColor
        }
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
        fileSystem.selectedTag() == 0 ? .NODOS :
        fileSystem.selectedTag() == 1 ? .OFS : .FFS
        
        let bb: BootBlockId =
        bootBlock.selectedTag() == 0 ? .NONE :
        bootBlock.selectedTag() == 1 ? .AMIGADOS_13 :
        bootBlock.selectedTag() == 2 ? .AMIGADOS_20 :
        bootBlock.selectedTag() == 3 ? .SCA : .BYTE_BANDIT
        
        do {
            try drive?.insertNew(fileSystem: fs, bootBlock: bb)
            myAppDelegate.clearRecentlyExportedDiskURLs(drive: nr)
            hideSheet()
            
        } catch {
            (error as? VAError)?.cantInsert()
        }
    }
}
