// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class CopperTableView: NSTableView {

    // Copper list (1 or 2)
    var nr = 1

    // Data caches
    var copperInfo: CopperInfo?
    var addrInRow: [Int: Int] = [:]
    var data1InRow: [Int: Int] = [:]
    var data2InRow: [Int: Int] = [:]
    var instrInRow: [Int: String] = [:]
    var illegalInRow: [Int: Bool] = [:]

    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
    }

    private func cache() {

        addrInRow = [:]
        data1InRow = [:]
        data2InRow = [:]
        instrInRow = [:]
        illegalInRow = [:]

        if amiga != nil {

            copperInfo = amiga!.agnus.getCopperInfo()

            assert(nr == 1 || nr == 2)
            var addr = nr == 1 ? Int(copperInfo!.cop1lc) : Int(copperInfo!.cop2lc)
            let count = nr == 1 ? Int(copperInfo!.length1) : Int(copperInfo!.length2)

            for i in 0 ..< count {

                addrInRow[i] = addr
                data1InRow[i] = amiga!.mem.spypeek16(addr)
                data2InRow[i] = amiga!.mem.spypeek16(addr + 2)
                instrInRow[i] = amiga!.agnus.disassemble(addr)
                illegalInRow[i] = amiga!.agnus.isIllegalInstr(addr)

                addr += 4
            }
        }
    }

    func refresh(count: Int = 0, full: Bool = false) {

        if count % 4 != 0 { return }

        if full {
            for (c, f) in ["addr": fmt24, "data1": fmt16, "data2": fmt16] {
                let columnId = NSUserInterfaceItemIdentifier(rawValue: c)
                if let column = tableColumn(withIdentifier: columnId) {
                    if let cell = column.dataCell as? NSCell {
                        cell.formatter = f
                    }
                }
            }
        }

        cache()
        reloadData()
    }
}

extension CopperTableView: NSTableViewDataSource {

    func numberOfRows(in tableView: NSTableView) -> Int {

        if let info = copperInfo {
            return  nr == 1 ? Int(info.length1) : Int(info.length2)
        }
        return 0
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
