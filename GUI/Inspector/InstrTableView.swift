// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class InstrTableView: NSTableView {
    
    @IBOutlet weak var inspector: Inspector!

    enum BreakpointType {
        case none
        case enabled
        case disabled
    }

    // Data caches
    var addrInRow: [Int: UInt32] = [:]
    var instrInRow: [Int: String] = [:]
    var dataInRow: [Int: String] = [:]
    var bpInRow: [Int: BreakpointType] = [:]
    var rowForAddr: [UInt32: Int] = [:]

    var hex = true
    
    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
        
        doubleAction = #selector(doubleClickAction(_:))
        action = #selector(clickAction(_:))
    }
    
    @IBAction func clickAction(_ sender: NSTableView!) {

        if sender.clickedColumn == 0 {

            lockAmiga()
            clickAction(row: sender.clickedRow)
            unlockAmiga()
        }
    }

    func clickAction(row: Int) {

        if let addr = addrInRow[row], let cpu = amiga?.cpu {

            if !cpu.breakpointIsSet(at: addr) {
                cpu.addBreakpoint(at: addr)
            } else if cpu.breakpointIsSetAndDisabled(at: addr) {
                cpu.breakpointSetEnable(at: addr, value: true)
            } else if cpu.breakpointIsSetAndEnabled(at: addr) {
                cpu.breakpointSetEnable(at: addr, value: false)
            }

            inspector.needsRefresh()
        }
    }

    @IBAction func doubleClickAction(_ sender: NSTableView!) {

        if sender.clickedColumn == 0 {

             lockAmiga()
             doubleClickAction(row: sender.clickedRow)
             unlockAmiga()
         }
    }

    func doubleClickAction(row: Int) {

        if let addr = addrInRow[row], let cpu = amiga?.cpu {

            if cpu.breakpointIsSet(at: addr) {
                cpu.removeBreakpoint(at: addr)
            } else {
                cpu.addBreakpoint(at: addr)
            }

            inspector.needsRefresh()
        }
    }

    func jumpToPC() {
        
        if let pc = inspector.parent?.amiga.cpu.getInfo().pc { jumpTo(addr: pc) }
    }

    func jumpTo(addr: UInt32?) {
    
        guard let addr = addr else { return }
        
        if let row = rowForAddr[addr] {
            
            // If the requested address is already displayed, we simply
            // select the corresponding row.
            scrollRowToVisible(row)
            selectRowIndexes([row], byExtendingSelection: false)
            reloadData()
            
        } else {
            
            // If the requested address is not displayed, we update the
            // whole view and display it in the first row.
            scrollRowToVisible(0)
            selectRowIndexes([0], byExtendingSelection: false)
            update(addr: addr)
        }
    }
    
    // Updates the currently displayed instructions
    func update() {
        update(addr: addrInRow[0])
    }
    
    // Updates the displayed instructions, starting at the specified address
    func update(addr: UInt32?) {
        
        guard var addr = addr else { return }
        
        instrInRow = [:]
        addrInRow = [:]
        dataInRow = [:]
        bpInRow = [:]
        rowForAddr = [:]

        for i in 0 ..< Int(CPUINFO_INSTR_COUNT) where addr <= 0xFFFFFF {

            var info = amiga!.cpu.getInstrInfo(i)

            instrInRow[i] = String(cString: &info.instr.0)
            addrInRow[i] = addr
            dataInRow[i] = String(cString: &info.data.0)
            if amiga!.cpu.breakpointIsSetAndDisabled(at: addr) {
                bpInRow[i] = BreakpointType.disabled
            } else if amiga!.cpu.breakpointIsSet(at: addr) {
                bpInRow[i] = BreakpointType.enabled
            } else {
                bpInRow[i] = BreakpointType.none
            }
            rowForAddr[addr] = i
            addr += UInt32(info.bytes)
        }
        
        reloadData()
    }

    func refreshFormatters() {

        for (c, f) in ["addr": fmt24] {
            let columnId = NSUserInterfaceItemIdentifier(rawValue: c)
            if let column = tableColumn(withIdentifier: columnId) {
                if let cell = column.dataCell as? NSCell {
                    cell.formatter = f
                }
            }
        }
    }

    func refresh(count: Int) {

        // Perform a full refresh if needed
        if count == 0 { refreshFormatters() }
        
        // Update display cache and refresh display with cached values
        jumpToPC()
    }
}

extension InstrTableView: NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        return Int(CPUINFO_INSTR_COUNT)
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {

        switch tableColumn?.identifier.rawValue {
            
        case "break" where bpInRow[row] == .enabled:
            return "\u{26D4}" // "⛔" ("\u{1F534}" // "🔴")
        case "break" where bpInRow[row] == .disabled:
            return "\u{26AA}" // "⚪" ("\u{2B55}" // "⭕")
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

extension InstrTableView: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        let cell = cell as? NSTextFieldCell

        if bpInRow[row] == .enabled {
            cell?.textColor = NSColor.systemRed
        } else if bpInRow[row] == .disabled {
            cell?.textColor = NSColor.disabledControlTextColor
        } else {
            cell?.textColor = NSColor.labelColor
        }
    }
}
