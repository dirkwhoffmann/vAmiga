// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class CopperTableView: NSTableView {

    enum BreakpointType {
        
        case none
        case enabled
        case disabled
    }
    
    @IBOutlet weak var inspector: Inspector!
    
    var amiga: AmigaProxy { return inspector.amiga }
    var copper: CopperProxy { return amiga.copper }
    var breakpoints: GuardsProxy { return amiga.copperBreakpoints }
    
    // Length of the Copper list as proposed by the Copper debugger
    var nativeLength = 0

    // Number of additional rows to displays
    var extraRows = 0
    
    // Actual length of the displayed Copper list
    var actualLength: Int { return nativeLength + extraRows }

    // Data caches
    var bpInRow: [Int: BreakpointType] = [:]
    var addrInRow: [Int: Int] = [:]
    var instrInRow: [Int: String] = [:]
    var illegalInRow: [Int: Bool] = [:]
    var rowForAddr: [Int: Int] = [:]
    
    override func awakeFromNib() {

        delegate = self
        dataSource = self
        target = self
        
        doubleAction = #selector(doubleClickAction(_:))
        action = #selector(clickAction(_:))
    }
    
    private func cache(list nr: Int, symbolic: Bool) {

        assert(nr == 1 || nr == 2)

        addrInRow = [:]
        instrInRow = [:]
        illegalInRow = [:]
        rowForAddr = [:]
        
        var start, end, addr: Int
            
        if nr == 1 {
            start = Int(inspector.copperInfo.copList1Start)
            end = Int(inspector.copperInfo.copList1End)
        } else {
            start = Int(inspector.copperInfo.copList2Start)
            end = Int(inspector.copperInfo.copList2End)
        }
        nativeLength = min((end - start) / 4, 500)
                
        addr = start

        for i in 0 ..< actualLength {

            instrInRow[i] = copper.disassemble(addr, symbolic: symbolic)
            illegalInRow[i] = copper.isIllegalInstr(addr)

            if breakpoints.isDisabled(at: addr) {
                bpInRow[i] = BreakpointType.disabled
            } else if breakpoints.isSet(at: addr) {
                bpInRow[i] = BreakpointType.enabled
            } else {
                bpInRow[i] = BreakpointType.none
            }

            addrInRow[i] = addr
            rowForAddr[addr] = i
            addr += 4
        }
    }

    func refresh(count: Int, full: Bool, list nr: Int, symbolic: Bool) {

        if count % 4 != 0 { return }
        
        if full {
            for (c, f) in ["addr": fmt24] {
                let columnId = NSUserInterfaceItemIdentifier(rawValue: c)
                if let column = tableColumn(withIdentifier: columnId) {
                    if let cell = column.dataCell as? NSCell {
                        cell.formatter = f
                    }
                }
            }
        }
        
        cache(list: nr, symbolic: symbolic)
        reloadData()
        
        // In animation mode, jump to the currently executed instruction
        
        if count != 0 || full {
            jumpTo(addr: Int(inspector.copperInfo.coppc0))
        }
    }

    func jumpTo(addr: Int, focus: Bool = false) {

        if let row = rowForAddr[addr] {

            reloadData()
            jumpTo(row: row, focus: focus)

        } else {

            deselectAll(self)
        }
    }

    func jumpTo(row: Int, focus: Bool = false) {

        if focus { window?.makeFirstResponder(self) }
        scrollRowToVisible(row)
        selectRowIndexes([row], byExtendingSelection: false)
    }
    
    func scrollToBottom() {
        
        scrollRowToVisible(numberOfRows(in: self) - 1)
    }
    
    @IBAction func clickAction(_ sender: NSTableView!) {
        
        if sender.clickedColumn == 0 {
            
            clickAction(row: sender.clickedRow)
        }
    }
    
    func clickAction(row: Int) {
        
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
        
        if let addr = addrInRow[row] {
            
            log("Double-clicked in row \(row) addr = \(addr)")
            
            if breakpoints.isSet(at: addr) {
                breakpoints.remove(at: addr)
            } else {
                breakpoints.setAt(addr)
            }
            
            inspector.fullRefresh()
        }
    }
}

extension CopperTableView: NSTableViewDataSource {

    func numberOfRows(in tableView: NSTableView) -> Int {

        return addrInRow.count
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        switch tableColumn?.identifier.rawValue {

        case "break" where bpInRow[row] == .enabled:
            return "\u{26D4}" // "⛔"
        case "break" where bpInRow[row] == .disabled:
            return "\u{26AA}" // "⚪"
        case "addr":
            return addrInRow[row]
        case "instr":
            return instrInRow[row]
        default:
            return ""
        }
    }
}

extension CopperTableView: NSTableViewDelegate {

    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        if let cell = cell as? NSTextFieldCell {
            
            if row >= nativeLength {
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
