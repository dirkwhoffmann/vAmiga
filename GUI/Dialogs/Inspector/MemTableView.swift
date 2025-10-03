// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
class MemTableView: NSTableView {

    @IBOutlet weak var inspector: Inspector!

    var emu: EmulatorProxy { return inspector.emu}

    // Displayed memory bank
    var bank = 0

    // Data caches
    var addrInRow: [Int: Int] = [:]
    var asciiInRow: [Int: String] = [:]
    var dataInAddr: [Int: Int] = [:]
    
    override init(frame frameRect: NSRect) { super.init(frame: frameRect); commonInit() }
    required init?(coder: NSCoder) { super.init(coder: coder); commonInit() }
    
    func commonInit() {

        delegate = self
        dataSource = self
        target = self
    }
        
    private func cache() {

        addrInRow = [:]
        asciiInRow = [:]
        dataInAddr = [:]

        let bank = inspector.displayedBank
        var addr = bank * 65536
        let rows = numberOfRows(in: self)

        // Continously update the RTC to get live register updates
        if emu.mem.memSrc(inspector.accessor, addr: addr) == .RTC {
            emu.rtc.update()
        }
        
        for i in 0 ..< rows {

            addrInRow[i] = addr
            asciiInRow[i] = emu.mem.ascDump(inspector.accessor, addr: addr, bytes: 16)

            for _ in 0 ..< 8 {

                dataInAddr[addr] = emu.mem.spypeek16(inspector.accessor, addr: addr)
                addr += 2
            }
        }
    }

    func refresh(count: Int = 0, full: Bool = false) {

        if count % 8 != 0 { return }

        if full {
            let columnFormatters = [
                "addr": inspector.fmt24,
                "0": inspector.fmt16,
                "2": inspector.fmt16,
                "4": inspector.fmt16,
                "6": inspector.fmt16,
                "8": inspector.fmt16,
                "A": inspector.fmt16,
                "C": inspector.fmt16,
                "E": inspector.fmt16
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

        var addr = inspector.displayedBank * 65536 + row * 16
     
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
        
        var addr = inspector.displayedBank * 65536 + row * 16
        let cell = cell as? NSTextFieldCell
        
        if inspector.displayedBankType == .NONE {
            cell?.textColor = .gray
        } else {
            cell?.textColor = .labelColor
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
            if inspector.searchAddress >> 1 == addr >> 1 {
                cell?.textColor = NSColor.red
            }
        default:
            break
        }
    }
    
    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {
        
    }
}
