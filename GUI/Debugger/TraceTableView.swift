// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

class TraceTableView : NSTableView {
    
    @IBOutlet weak var inspector: Inspector!
    
    var cpu = amigaProxy?.cpu
    
    override func awakeFromNib() {
        
        dataSource = self
        target = self
    }
    
    func refresh() {
        
        reloadData()        
    }
    
    @IBAction func clickAction(_ sender: NSTableView!) {
        
        let row = sender.clickedRow
        track("\(row)")
    }
}

extension TraceTableView : NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int { return Int(CPUINFO_INSTR_COUNT); }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        if var info = cpu?.getTracedInstrInfo(row) {
            
            switch tableColumn?.identifier.rawValue {
                
            case "addr":
                return String(cString: &info.addr.0)
            case "flags":
                return String(cString: &info.flags.0)
            case "instr":
                return String(cString: &info.instr.0)
            default:
                return "???"
            }
        }
        return "??"
    }
}
