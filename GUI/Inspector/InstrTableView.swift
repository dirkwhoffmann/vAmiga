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

    // Display caches
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

        let row = sender.clickedRow
        let col = sender.clickedColumn

        inspector.lockParent()
        if let amiga = inspector.parent?.amiga {
            clickAction(amiga, row: row, col: col)
        }
        inspector.unlockParent()
    }

    func clickAction(_ amiga: AmigaProxy, row: Int, col: Int) {

        if col == 0, let addr = addrInRow[row] {

            let cpu = amiga.cpu!

            if !cpu.breakpointIsSet(at: addr) {
                cpu.addBreakpoint(at: addr)
            } else if cpu.breakpointIsSetAndDisabled(at: addr) {
                cpu.breakpointSetEnable(at: addr, value: true)
            } else if cpu.breakpointIsSetAndEnabled(at: addr) {
                cpu.breakpointSetEnable(at: addr, value: false)
            }
            update()
            reloadData()
            inspector.breakTableView.reloadData()
        }
    }

    @IBAction func doubleClickAction(_ sender: NSTableView!) {

         let row = sender.clickedRow
         let col = sender.clickedColumn

         inspector.lockParent()
         if let amiga = inspector.parent?.amiga {
             doubleClickAction(amiga, row: row, col: col)
         }
         inspector.unlockParent()
     }

    func doubleClickAction(_ amiga: AmigaProxy, row: Int, col: Int) {

        if col == 0, let addr = addrInRow[row] {

            let cpu = amiga.cpu!
            
            if cpu.breakpointIsSet(at: addr) {
                cpu.removeBreakpoint(at: addr)
            } else {
                cpu.addBreakpoint(at: addr)
            }
            inspector.refresh(everything: true)
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

            if let cpu = inspector.parent?.amiga.cpu {

                var info = cpu.getInstrInfo(i)

                instrInRow[i] = String(cString: &info.instr.0)
                addrInRow[i] = addr
                dataInRow[i] = String(cString: &info.data.0)
                if cpu.breakpointIsSetAndDisabled(at: addr) {
                    bpInRow[i] = BreakpointType.disabled
                } else if cpu.breakpointIsSet(at: addr) {
                    bpInRow[i] = BreakpointType.enabled
                } else {
                    bpInRow[i] = BreakpointType.none
                }
                rowForAddr[addr] = i
                addr += UInt32(info.bytes)
            }
        }
        
        reloadData()
    }
    
    func refresh(everything: Bool) {

        if everything {

            for (c, f) in ["addr": fmt24] {
                let columnId = NSUserInterfaceItemIdentifier(rawValue: c)
                if let column = tableColumn(withIdentifier: columnId) {
                    if let cell = column.dataCell as? NSCell {
                        cell.formatter = f
                    }
                }
            }
        }
        
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
