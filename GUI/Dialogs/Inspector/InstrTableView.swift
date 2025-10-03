// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
class InstrTableView: NSTableView {
    
    @IBOutlet weak var inspector: Inspector!
    
    var amiga: EmulatorProxy? { return inspector.parent.emu }
    var cpu: CPUProxy? { return amiga?.cpu }
    var breakpoints: GuardsProxy? { return amiga?.breakpoints }

    enum BreakpointType {
        
        case none
        case enabled
        case disabled
    }

    // The first address to disassemble
    var addrInFirstRow = 0

    // Data caches
    var numRows = 0
    var bpInRow: [Int: BreakpointType] = [:]
    var addrInRow: [Int: Int] = [:]
    var addrStrInRow: [Int: String] = [:]
    var dataInRow: [Int: String] = [:]
    var instrInRow: [Int: String] = [:]
    var rowForAddr: [Int: Int] = [:]
        
    // Optional address to be highlighted by an alert symbol
    var alertAddr: Int?
    
    override init(frame frameRect: NSRect) { super.init(frame: frameRect); commonInit() }
    required init?(coder: NSCoder) { super.init(coder: coder); commonInit() }
    
    func commonInit() {

        delegate = self
        dataSource = self
        target = self

        doubleAction = #selector(doubleClickAction(_:))
        action = #selector(clickAction(_:))
    }
                
    private func cache(addrInFirstRow addr: Int) {

        addrInFirstRow = addr
        cache()
    }
    
    private func cache() {
        
        guard let cpu = cpu else { return }

        numRows = 256
        rowForAddr = [:]

        var addr = addrInFirstRow
        for i in 0 ..< numRows {
            
            var bytes = 0
            instrInRow[i] = cpu.disassembleInstr(addr, length: &bytes)
            dataInRow[i] = cpu.disassembleWords(addr, length: bytes / 2)
                        
            if breakpoints!.isDisabled(at: addr) {
                bpInRow[i] = BreakpointType.disabled
            } else if breakpoints!.isSet(at: addr) {
                bpInRow[i] = BreakpointType.enabled
            } else {
                bpInRow[i] = BreakpointType.none
            }
            
            addrInRow[i] = addr
            addrStrInRow[i] = cpu.disassembleAddr(addr)
            rowForAddr[addr] = i
            addr += bytes
        }
    }
        
    func refresh(count: Int = 0, full: Bool = false, addr: Int = 0) {

        if full {
            for (c, f) in ["addr": inspector.fmt24] {
                let columnId = NSUserInterfaceItemIdentifier(rawValue: c)
                if let column = tableColumn(withIdentifier: columnId) {
                    if let cell = column.dataCell as? NSCell {
                        cell.formatter = f
                    }
                }
            }

            cache()
            reloadData()
        }

        // In animation mode, jump to the currently executed instruction
        if count != 0 || full { jumpTo(addr: addr) }
    }

    func jumpTo(addr: Int) {

        if let row = rowForAddr[addr] {

            // If the requested address is already displayed, we simply select
            // the corresponding row.
            reloadData()
            jumpTo(row: row)

        } else {

            // If the requested address is not displayed, we update the data
            // cache and display the address in row 0.
            cache(addrInFirstRow: addr)
            reloadData()
            jumpTo(row: 0)
        }
    }

    func jumpTo(row: Int) {

        scrollRowToVisible(row)
        selectRowIndexes([row], byExtendingSelection: false)
    }

    @IBAction func clickAction(_ sender: NSTableView!) {

        if sender.clickedColumn == 0 {

            clickAction(row: sender.clickedRow)
        }
    }

    func clickAction(row: Int) {

        guard let breakpoints = breakpoints else { return }

        if let addr = addrInRow[row] {

            if !breakpoints.isSet(at: addr) {
                breakpoints.setAt(addr)
            } else if breakpoints.isDisabled(at: addr) {
                breakpoints.enable(at: addr)
            } else if breakpoints.isEnabled(at: addr) {
                breakpoints.disable(at: addr)
            }

            inspector.fullRefresh()
        }
    }

    @IBAction func doubleClickAction(_ sender: NSTableView!) {

        if sender.clickedColumn != 0 {

            doubleClickAction(row: sender.clickedRow)
        }
    }

    func doubleClickAction(row: Int) {

        guard let breakpoints = breakpoints else { return }
        
        if let addr = addrInRow[row] {

            if breakpoints.isSet(at: addr) {
                breakpoints.remove(at: addr)
            } else {
                breakpoints.setAt(addr)
            }

            inspector.fullRefresh()
        }
    }
}

extension InstrTableView: NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        
        return numRows
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        switch tableColumn?.identifier.rawValue {
            
        case "break" where addrInRow[row] == alertAddr:
            return "⚠️"
        case "break" where bpInRow[row] == .enabled:
            return "\u{26D4}" // "⛔" ("\u{1F534}" // "🔴")
        case "break" where bpInRow[row] == .disabled:
            return "\u{26AA}" // "⚪" ("\u{2B55}" // "⭕")
        case "addr":
            return addrStrInRow[row]
        case "data":
            return dataInRow[row]
        case "instr":
            return instrInRow[row]
        default:
            return ""
        }
    }
}

extension InstrTableView: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        let cell = cell as? NSTextFieldCell

        if bpInRow[row] == .enabled {
            cell?.textColor = .warning
        } else if bpInRow[row] == .disabled {
            cell?.textColor = .disabledControlTextColor
        } else {
            cell?.textColor = .labelColor
        }
    }
}
