// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class SpriteTableView : NSTableView {
    
    var amiga = amigaProxy
    
    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
    }
    
    func refresh(everything: Bool) {

        reloadData()
    }
}

extension SpriteTableView : NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int { return 8; }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
                
        switch tableColumn?.identifier.rawValue {
            
        case "addr":
            return 42
            
        case "data":
            return "1010101010101010"
            
        default:
            return "???"
        }
    }
}

extension SpriteTableView : NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {
        
        switch(tableColumn?.identifier.rawValue) {
            
        case "data":
            track()
            
        default:
            NSSound.beep()
            return
        }

        /*
        if let value = object as? UInt16 {
            amigaProxy?.suspend()
            amigaProxy?.mem.poke16(addr, value: Int(value))
            amigaProxy?.resume()
        }
        */
    }
}
