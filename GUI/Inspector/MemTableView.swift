// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class MemTableView: NSTableView {

    @IBOutlet weak var inspector: Inspector!
    var amiga: AmigaProxy!

    // Displayed memory bank
    var bank = 0

    // Data caches
    var addrInRow: [Int: Int] = [:]
    var asciiInRow: [Int: String] = [:]
    var dataInAddr: [Int: Int] = [:]
    
    override func awakeFromNib() {

        amiga = inspector.amiga
        delegate = self
        dataSource = self
        target = self
    
        inspector.setBank(0)
    }

    private func cache() {

        addrInRow = [:]
        asciiInRow = [:]
        dataInAddr = [:]

        var addr = inspector.bank * 65536
        let rows = numberOfRows(in: self)

        for i in 0 ..< rows {

            addrInRow[i] = addr
            asciiInRow[i] = amiga.mem.ascii(addr)

            for _ in 0 ..< 8 {

                dataInAddr[addr] = amiga.mem.spypeek16(addr)
                addr += 2
            }
        }
    }

    func refresh(count: Int = 0, full: Bool = false) {

        if count % 8 != 0 { return }

        if full {
            let columnFormatters = [
                "addr": fmt24,
                "0": fmt16,
                "2": fmt16,
                "4": fmt16,
                "6": fmt16,
                "8": fmt16,
                "A": fmt16,
                "C": fmt16,
                "E": fmt16
            ]

            for (column, formatter) in columnFormatters {
                let columnId = NSUserInterfaceItemIdentifier(rawValue: column)
                if let tableColumn = tableColumn(withIdentifier: columnId) {
                    if let cell = tableColumn.dataCell as? NSCell {
                        cell.formatter = formatter
                    }
                }
            }
        }

        cache()
        reloadData()
    }
}

extension MemTableView: NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        
        return 65536 / 16
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {

        var addr = inspector.bank * 65536 + row * 16
     
        switch tableColumn?.identifier.rawValue {

        case "addr":
            return addrInRow[row]
            
        case "ascii":
            return asciiInRow[row]

        case "E": addr += 2; fallthrough
        case "C": addr += 2; fallthrough
        case "A": addr += 2; fallthrough
        case "8": addr += 2; fallthrough
        case "6": addr += 2; fallthrough
        case "4": addr += 2; fallthrough
        case "2": addr += 2; fallthrough
        case "0": return dataInAddr[addr]
      
        default:
            fatalError()
        }
    }
}

extension MemTableView: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        var addr = inspector.bank * 65536 + row * 16
        let cell = cell as? NSTextFieldCell
        
        if inspector.memSrc == MEM_UNMAPPED {
            cell?.textColor = NSColor.gray
        } else {
            cell?.textColor = NSColor.textColor
        }
        
        switch tableColumn?.identifier.rawValue {
            
        case "E": addr += 2; fallthrough
        case "C": addr += 2; fallthrough
        case "A": addr += 2; fallthrough
        case "8": addr += 2; fallthrough
        case "6": addr += 2; fallthrough
        case "4": addr += 2; fallthrough
        case "2": addr += 2; fallthrough
        case "0":
            if inspector.selected >> 1 == addr >> 1 {
                cell?.textColor = NSColor.red
            }
        default:
            break
        }
    }
    
    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {
        
    }
}
