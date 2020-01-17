// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class CopperTableView: NSTableView {

    // Data caches
    var addrInRow: [Int: Int] = [:]
    var data1InRow: [Int: Int] = [:]
    var data2InRow: [Int: Int] = [:]
    var instrInRow: [Int: String] = [:]
    var illegalInRow: [Int: Bool] = [:]
    var numRows = 0
    
    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
    }

    func cache(count: Int = 0) {

        var addr = coplc

        addrInRow = [:]
        data1InRow = [:]
        data2InRow = [:]
        instrInRow = [:]
        illegalInRow = [:]

        if amiga != nil {

            numRows = amiga!.agnus.instrCount(tag)

            for i in 0 ..< numRows {

                addrInRow[i] = addr
                data1InRow[i] = amiga!.mem.spypeek16(addr)
                data2InRow[i] = amiga!.mem.spypeek16(addr + 2)
                instrInRow[i] = amiga!.agnus.disassemble(addr)
                illegalInRow[i] = amiga!.agnus.isIllegalInstr(addr)

                addr += 4
            }
        }
    }

    func refresh(everything: Bool) {
        
        if everything {

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
}

extension CopperTableView: NSTableViewDataSource {
    
    var coplc: Int {
        assert(tag == 1 || tag == 2)
        let info = amiga!.agnus.getCopperInfo()
        return (tag == 1) ? Int(info.cop1lc) : Int(info.cop2lc)
    }
    
    func numberOfRows(in tableView: NSTableView) -> Int {

        return numRows
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {

        switch tableColumn?.identifier.rawValue {
            
        case "addr":  return addrInRow[row]
        case "data1": return data1InRow[row]
        case "data2": return data2InRow[row]
        case "instr": return instrInRow[row]

        default: fatalError()
        }
    }
}

extension CopperTableView: NSTableViewDelegate {

    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        if let cell = cell as? NSTextFieldCell {
            if tableColumn?.identifier.rawValue == "instr" {
                if illegalInRow[row] == true { cell.textColor = .red; return }
            }
            cell.textColor = .textColor
        }
    }
}
