// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class CopperTableView : NSTableView {
    
    // @IBOutlet weak var inspector: Inspector!
    
    var memory = amigaProxy?.mem
    var numRows = 5
    
    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
    }
    
    func refresh() {
        
        track()
        memory = amigaProxy?.mem
        reloadData()
    }
}

extension CopperTableView : NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int { return numRows; }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        switch tableColumn?.identifier.rawValue {
            
        case "bank":
            return row
            
        default:
            return "???"
        }
    }
}

extension CopperTableView : NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        if let cell = cell as? NSTextFieldCell {
            
            // TODO: Make red when MOVE has an illegal target
            cell.textColor = .red
        }
    }
}
