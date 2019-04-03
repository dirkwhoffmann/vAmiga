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
    
    var memory = amigaProxy?.mem // REMOVE
    var cpu = amigaProxy?.cpu
    
    // Data caches
    var pc    : [Int:String] = [:]
    var flags : [Int:String] = [:]
    var instr : [Int:String] = [:]
    
    override func awakeFromNib() {
        
        dataSource = self
        target = self
    }
    
    func refresh() {

        // guard let cpu = amigaProxy?.cpu else { return }

        pc = [:]
        flags = [:]
        instr = [:]

        /*
        amigaProxy?.suspend()
        
        let count = cpu.recordedInstructions() - 1
        
        if count > 0 {
            for i in 0...(count - 1) {
                
                // var rec = cpu.readRecordedInstruction(i)
                
                let pcStr = "** 42 **" //  String(rec.pc, radix: 16, uppercase: true)
                // let flagsStr = String.init(utf8String:&rec.flags.0)!
                // let commandStr = String.init(utf8String:&rec.instr.0)!
                pc[i] = pcStr
                flags[i] = "" // flagsStr
                instr[i] = "" // commandStr
            }
            scrollRowToVisible(count - 1)
        }
    
        amigaProxy?.resume()
        */
        
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
        
        if var info = cpu?.getTraceInstr(row) {
            
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

