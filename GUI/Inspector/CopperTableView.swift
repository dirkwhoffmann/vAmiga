// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class CopperTableView: NSTableView {

    @IBOutlet weak var inspector: Inspector!
    
    var amiga: AmigaProxy { return inspector.amiga }

    // Copper list (1 or 2)
    var nr = 1

    // Length of the currently displayed Copper list
    var length = 0
    
    // Number of additional rows to displays
    var extraRows = 0
    
    // Data caches
    // var copperInfo: CopperInfo!
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

        assert(nr == 1 || nr == 2)

        addrInRow = [:]
        data1InRow = [:]
        data2InRow = [:]
        instrInRow = [:]
        illegalInRow = [:]

        var start, end, addr, count: Int
            
        if nr == 1 {
            start = Int(inspector.copperInfo.copList1Start)
            end = Int(inspector.copperInfo.copList1End)
        } else {
            start = Int(inspector.copperInfo.copList2Start)
            end = Int(inspector.copperInfo.copList2End)
        }
        length = (end - start) / 4
        addr = start
        count = min(length, 100) + extraRows

        for i in 0 ..< count {

            addrInRow[i] = addr
            data1InRow[i] = amiga.mem.spypeek16(.AGNUS, addr: addr)
            data2InRow[i] = amiga.mem.spypeek16(.AGNUS, addr: addr + 2)
            instrInRow[i] = amiga.copper.disassemble(addr)
            illegalInRow[i] = amiga.copper.isIllegalInstr(addr)

            addr += 4
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

    func scrollToBottom() {
        scrollRowToVisible(numberOfRows(in: self) - 1)
    }
}

extension CopperTableView: NSTableViewDataSource {

    func numberOfRows(in tableView: NSTableView) -> Int {

        return addrInRow.count
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
            
            if row >= length {
                cell.textColor = .secondaryLabelColor
                return
            }
            
            if tableColumn?.identifier.rawValue == "instr" {
                if illegalInRow[row] == true {
                    cell.textColor = .warningColor
                    return
                }
            }
            cell.textColor = .textColor
        }
    }
}
