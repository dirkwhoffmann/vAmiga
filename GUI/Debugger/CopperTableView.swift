// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class CopperTableView: NSTableView {
    
    // @IBOutlet weak var inspector: Inspector!
    
    var amiga = amigaProxy
    var numRows = 5
    
    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
    }
    
    func refresh(everything: Bool) {
        
        if everything {
            
            amiga = amigaProxy
            
            for (c, f) in ["addr": fmt24, "data1": fmt16, "data2": fmt16] {
                let columnId = NSUserInterfaceItemIdentifier(rawValue: c)
                if let column = tableColumn(withIdentifier: columnId) {
                    if let cell = column.dataCell as? NSCell {
                        cell.formatter = f
                    }
                }
            }
        }
        
        reloadData()
    }
    
    @IBAction func copPlusAction(_ sender: NSButton!) {
        
        if numRows < 128 {
            numRows += 1
        } else {
            NSSound.beep()
        }
        reloadData()
    }
    
    @IBAction func copMinusAction(_ sender: NSButton!) {
        
        if numRows > 0 {
            numRows -= 1
        } else {
            NSSound.beep()
        }
        reloadData()
    }
}

extension CopperTableView: NSTableViewDataSource {
    
    var coplc: Int {
        assert(tag == 1 || tag == 2)
        let info = amiga!.dma.getCopperInfo()
        return (tag == 1) ? Int(info.cop1lc) : Int(info.cop2lc)
    }
    
    func numberOfRows(in tableView: NSTableView) -> Int { return numRows; }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        let addr = coplc + 4 * row
        
        switch tableColumn?.identifier.rawValue {
            
        case "addr":
            return addr

        case "data1":
            return amiga?.mem.spypeek16(addr)

        case "data2":
            return amiga?.mem.spypeek16(addr + 2)

        case "instr":
            return amiga?.dma.disassemble(addr)

        default:
            return "???"
        }
    }
}

extension CopperTableView: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {
        
        var addr = 0
        
        switch tableColumn?.identifier.rawValue {
        
        case "data1":
            addr = coplc + 4 * row
            
        case "data2":
            addr = coplc + 4 * row + 2
        
        default:
            NSSound.beep()
            return
        }
        
        if let value = object as? UInt16 {
            amigaProxy?.suspend()
            amigaProxy?.mem.poke16(addr, value: Int(value))
            amigaProxy?.resume()
        }
    }
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        if let cell = cell as? NSTextFieldCell {
            if tableColumn?.identifier.rawValue == "instr" {
                if amiga?.dma.isIllegalInstr(coplc + 4 * row) == true {
                    cell.textColor = .red
                    return
                }
            }
            cell.textColor = .textColor
        }
    }
}
