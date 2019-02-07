// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

class InstrTableView : NSTableView {
    
    @IBOutlet weak var inspector: Inspector!
    
    var memory = amigaProxy?.mem
    var cpu = amigaProxy?.cpu
    
    // Display caches
    var addrInRow  : [Int:UInt32] = [:]
    var instrInRow : [Int:String] = [:]
    var dataInRow  : [Int:String] = [:]
    var rowForAddr : [UInt32:Int] = [:]
    var hex = true
    
    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
        
        doubleAction = #selector(doubleClickAction(_:))
    }
        
    @IBAction func doubleClickAction(_ sender: Any!) {
        
        let sender = sender as! NSTableView
        let row = sender.selectedRow
        
        if let addr = addrInRow[row] {
            track("Toggling breakpoint at \(addr)")
            amigaProxy?.cpu.toggleBreakpoint(at: addr)
            reloadData()
        }
    }
    
    func disassemble() {
        
        if let pc = amigaProxy?.cpu.getInfo().pc {
            disassemble(startAddr: pc)
        }
    }
    
    func disassemble(startAddr: UInt32) {
        
        guard let amiga = amigaProxy else { return }
        
        var addr = startAddr
        var buffer = Array<Int8>(repeating: 0, count: 64)
        
        instrInRow = [:]
        addrInRow = [:]
        rowForAddr = [:]
        
        for i in 0...255 {
            if (addr <= 0xFFFFFF) {
          
                let bytes = amiga.cpu.disassemble(&buffer, pc: Int(addr))
                instrInRow[i] = String.init(cString: buffer)
                addrInRow[i]  = addr
                dataInRow[i]  = amiga.mem.hex(Int(addr), bytes: bytes)
                rowForAddr[addr] = i
                
                addr += UInt32(bytes)
                
            } else {
                
                instrInRow[i] = nil;
            }
        }
        
        reloadData()
    }
    
    func refresh(everything: Bool) {
    
        track()
        
        if (everything) {
        
            memory = amigaProxy?.mem
            cpu = amigaProxy?.cpu
        
            for (c,f) in ["addr" : fmt24] {
                let columnId = NSUserInterfaceItemIdentifier(rawValue: c)
                if let column = tableColumn(withIdentifier: columnId) {
                    if let cell = column.dataCell as? NSCell {
                        cell.formatter = f
                    }
                }
            }
            
            reloadData()
        }
        
        if let pc = amigaProxy?.cpu.getInfo().pc {
                
            if let row = rowForAddr[pc] {
                
                // If the requested address is already displayed, we simply
                // select the corresponding row.
                scrollRowToVisible(row)
                selectRowIndexes([row], byExtendingSelection: false)
                
            } else {
                
                 // If the requested address is not displayed, we update the
                // whole view and display it in the first row.
                disassemble(startAddr: pc)
                scrollRowToVisible(0)
                selectRowIndexes([0], byExtendingSelection: false)
            }
        }
    }
}

extension InstrTableView : NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        return 256;
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        switch tableColumn?.identifier.rawValue {
            
        case "break":
            if cpu!.hasBreakpoint(at: addrInRow[row]!) {
                return "⛔"
            } else {
                return " "
            }
        case "addr":
            return addrInRow[row]
        case "data":
            return dataInRow[row]
        case "instr":
            return instrInRow[row]
        default:
            return "?"
        }
    }
}

extension InstrTableView : NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        let cell = cell as! NSTextFieldCell
        
        if let addr = addrInRow[row], let c = cpu {
            
            if c.hasConditionalBreakpoint(at: addr) {
                cell.textColor = NSColor.systemOrange
            } else if c.hasBreakpoint(at: addr) {
                cell.textColor = NSColor.systemRed
            } else {
                cell.textColor = NSColor.textColor
            }
        }
    }
}
