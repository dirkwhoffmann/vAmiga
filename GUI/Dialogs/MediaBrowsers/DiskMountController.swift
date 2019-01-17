// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

class DiskMountController : UserDialogController {
    
    var disk: AnyDiskProxy!
    
    let bytesPerRow = 32
    
    // Cylinder, Head, Track, and Sector
    var _cylinder = 0
    var _head = 0
    var _track = 0
    var _sector = 0
        
    func setCylinder(_ newValue: Int) {

        if (newValue >= 0 && newValue <= 79) {
            
            _cylinder = newValue
            _track    = _cylinder * 2 + _head
            _sector   = (_track * 11) + (_sector % 11)
        }
    }
    
    func setHead(_ newValue: Int) {
        
        if (newValue >= 0 && newValue <= 1) {
            
            _head     = newValue
            _track    = _cylinder * 2 + _head
            _sector   = (_track * 11) + (_sector % 11)
        }
    }
    
    func setTrack(_ newValue: Int) {
        
        if (newValue >= 0 && newValue <= 159) {
            
            _track    = newValue
            _cylinder = _track / 2
            _head     = _track % 2
            _sector   = (_track * 11) + (_sector % 11)
        }
    }

    func setSector(_ newValue: Int) {
        
        if (newValue >= 0 && newValue <= 1759) {
            
            _sector   = newValue
            _track    = _sector % 11
            _cylinder = _track / 2
            _head     = _track % 2
        }
    }
    
    // Outlets
    @IBOutlet weak var previewTable: NSTableView!
    @IBOutlet weak var cylinderField: NSTextField!
    @IBOutlet weak var cylinderStepper: NSStepper!
    @IBOutlet weak var headField: NSTextField!
    @IBOutlet weak var headStepper: NSStepper!
    @IBOutlet weak var trackField: NSTextField!
    @IBOutlet weak var trackStepper: NSStepper!
    @IBOutlet weak var sectorField: NSTextField!
    @IBOutlet weak var sectorStepper: NSStepper!
    
    override func showSheet(completionHandler handler:(() -> Void)? = nil) {
    
        if let attachment = myDocument?.attachment as? AnyDiskProxy {
            
            disk = attachment
            super.showSheet(completionHandler: handler)
        }
    }
    
    override public func awakeFromNib() {

        update()
    }
    
    func update() {
    
        cylinderField.integerValue   = _cylinder
        cylinderStepper.integerValue = _cylinder
        headField.integerValue       = _head
        headStepper.integerValue     = _head
        trackField.integerValue      = _track
        trackStepper.integerValue    = _track
        sectorField.integerValue     = _sector
        sectorStepper.integerValue   = _sector

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
    @IBAction func df0Action(_ sender: Any!) {
        
        print("df0Action")
        // myController?.changeDisk(disk, drive: nr)
        myController?.metalScreen.rotateBack()
        hideSheet()
    }
    @IBAction func df1Action(_ sender: Any!) {
        
        print("df1Action")
        // myController?.changeDisk(disk, drive: nr)
        myController?.metalScreen.rotateBack()
        hideSheet()
    }
}

//
// NSTableView delegate and data source
//

extension DiskMountController : NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView,
                   willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        // let c = cell as! NSTextFieldCell
        // c.font = cbmfont
        // c.textColor = .red
    }
}


extension DiskMountController : NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        
        return 8
    }
    
    func tableView(_ tableView: NSTableView,
                   objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        if (tableColumn?.identifier)!.rawValue == "data" {
            
            return "42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42 42"
        }
        
        return "???"
    }
}

