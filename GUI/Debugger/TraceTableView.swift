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
    
    var memory = amigaProxy?.mem
    
    // Data caches
    var pc    : [Int:String] = [:]
    var flags : [Int:String] = [:]
    var instr : [Int:String] = [:]
    
    override func awakeFromNib() {
        
        dataSource = self
        target = self
    }
    
    func refresh() {
        
        track()

        guard let cpu = amigaProxy?.cpu else { return }

        pc = [:]
        flags = [:]
        instr = [:]
        
        let entries = cpu.recordedInstructions() - 1
        if entries > 0 {
            for i in 0...(entries - 1) {
                
                var rec = cpu.readRecordedInstruction(i)
                
                let pcStr = String(rec.pc, radix: 16, uppercase: true)
                let flagsStr = String.init(utf8String:&rec.flags.0)!
                let commandStr = String.init(utf8String:&rec.instr.0)!
                pc[i] = pcStr
                flags[i] = flagsStr
                instr[i] = commandStr
            }
            scrollRowToVisible(entries - 1)
        }
    
        reloadData()
        
    }
    
    @IBAction func clickAction(_ sender: NSTableView!) {
        
        let row = sender.clickedRow
        track("\(row)")
    }
}

extension TraceTableView : NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int { return 254; }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        switch tableColumn?.identifier.rawValue {
            
        case "addr":
            return pc[row]
        case "flags":
            return flags[row] ?? ""
        case "instr":
            return instr[row] ?? ""
        default:
            return "???"
        }
    }
}

