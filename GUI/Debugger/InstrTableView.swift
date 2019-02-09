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
    
    var memory = amigaProxy!.mem!
    var cpu = amigaProxy!.cpu!
    
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
        action = #selector(clickAction(_:))
    }
    
    @IBAction func clickAction(_ sender: NSTableView!) {
        
        let row = sender.clickedRow
        let col = sender.clickedColumn

        track("row = \(row) col = \(col)")
        
        if col == 0, let addr = addrInRow[row] {
            
            if !cpu.hasBreakpoint(at: addr) {
                cpu.setBreakpointAt(addr)
                return
            }
            if cpu.hasDisabledBreakpoint(at: addr) {
                cpu.enableBreakpoint(at: addr)
                return
            }
            if cpu.hasBreakpoint(at: addr) {
                cpu.disableBreakpoint(at: addr)
                return
            }
        }
    }
    
    @IBAction func doubleClickAction(_ sender: NSTableView!) {
        
        let row = sender.clickedRow
        let col = sender.clickedColumn

        if col > 0, let addr = addrInRow[row] {
            
            if cpu.hasBreakpoint(at: addr) {
                cpu.deleteBreakpoint(at: addr)
            } else {
                cpu.setBreakpointAt(addr)
            }
        }
    }
    
    func disassemblePC() {
        
        if let pc = amigaProxy?.cpu.getInfo().pc {
            disassemble(startAddr: pc)
        }
    }
    
    func disassemble(startAddr: UInt32) {
        
        addrInRow[0] = startAddr;
        disassemble()
        scrollRowToVisible(0)
        selectRowIndexes([0], byExtendingSelection: false)
    }
    
    func disassemble() {
    
        guard let amiga = amigaProxy else { return }
        
        let startAddr = addrInRow[0] ?? 0
        track("disassemble: \(startAddr)")
        
        var addr = startAddr
        var buffer = Array<Int8>(repeating: 0, count: 64)
        
        instrInRow = [:]
        addrInRow = [:]
        dataInRow = [:]
        rowForAddr = [:]
        
        for i in 0...255 {
            if (addr <= 0xFFFFFF) {
          
                let bytes = amiga.cpu.disassemble(&buffer, pc: Int(addr))
                instrInRow[i] = String.init(cString: buffer)
                addrInRow[i]  = addr
                dataInRow[i]  = amiga.mem.hex(Int(addr), bytes: bytes)
                rowForAddr[addr] = i
                
                addr += UInt32(bytes)
            }
        }
        
        reloadData()
    }
    
    func refresh(everything: Bool) {
    
        track()
        
        if (everything) {
        
            memory = amigaProxy!.mem
            cpu = amigaProxy!.cpu
        
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
            }
        }
    }
}

extension InstrTableView : NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        return 256;
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        guard let addr = addrInRow[row] else { return nil }
        
        switch tableColumn?.identifier.rawValue {
            
        case "break" where cpu.hasDisabledBreakpoint(at: addr):
            return "\u{26AA}" // "⚪"
        case "break" where cpu.hasBreakpoint(at: addr):
            return "\u{1F534}" // "🔴"
            // return "\u{26D4}" // "⛔"
        case "addr":
            return addrInRow[row]
        case "data":
            return dataInRow[row]
        case "instr":
            return instrInRow[row]
        default:
            return ""
        }
    }
}

extension InstrTableView : NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        let cell = cell as! NSTextFieldCell
        
        if let addr = addrInRow[row] {

            // let disabled = cpu.hasDisabledBreakpoint(at: addr)
            // let conditional = cpu.hasConditionalBreakpoint(at: addr)

            if cpu.hasDisabledBreakpoint(at: addr) {
                cell.textColor = NSColor.disabledControlTextColor
            } else if cpu.hasConditionalBreakpoint(at: addr) {
                cell.textColor = NSColor.systemOrange
            } else if cpu.hasBreakpoint(at: addr) {
                cell.textColor = NSColor.systemRed
            } else {
                cell.textColor = NSColor.labelColor
            }
        }
    }
}
