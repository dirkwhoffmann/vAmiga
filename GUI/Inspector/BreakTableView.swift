// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class BreakTableView: NSTableView {
    
    @IBOutlet weak var inspector: Inspector!

    // Data caches
    var disabledCache: [Int: Bool] = [:]
    var symbCache: [Int: String] = [:]
    var addrCache: [Int: UInt32] = [:]
    var breakCnt = 0

    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
        
        action = #selector(clickAction(_:))
    }

    func cache() {

        if amiga == nil { return }

        breakCnt = amiga!.cpu.numberOfBreakpoints()

        for i in 0 ..< breakCnt {
            if amiga!.cpu.breakpointIsDisabled(i) {
                disabledCache[i] = true
                symbCache[i] = "\u{26AA}" /* ⚪ */
            } else {
                disabledCache[i] = false
                symbCache[i] = "\u{26D4}" /* ⛔ */
            }
            addrCache[i] = amiga!.cpu.breakpointAddr(i)
        }
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

        // Update display cache
        cache()

        // Refresh display with cached values
        reloadData()
    }

    @IBAction func clickAction(_ sender: NSTableView!) {

        let row = sender.clickedRow
        let col = sender.clickedColumn

        lockAmiga()

        if col == 0 { // Enable / Disable
            let disabled = amiga?.cpu.breakpointIsDisabled(row) ?? false
            amiga?.cpu.breakpointSetEnable(row, value: disabled)
            inspector.needsRefresh()
        }

        if col == 0 || col == 1 { // Jump to breakpoint address
            if let addr = amiga?.cpu.breakpointAddr(row), addr <= 0xFFFFFF { inspector.instrTableView.jumpTo(addr: addr)
            }
        }

        if col == 2 { // Delete
            amiga?.cpu.removeBreakpoint(row)
            inspector.needsRefresh()
        }

        unlockAmiga()
    }
}

extension BreakTableView: NSTableViewDataSource {

    func numberOfRows(in tableView: NSTableView) -> Int {

        return breakCnt + 1
    }

    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {

        let last = row == breakCnt

        switch tableColumn?.identifier.rawValue {

        case "break": return last ? "" : (symbCache[row] ?? "?")
        case "addr": return last ? "Add address" : (addrCache[row] ?? "?")
        case "delete": return last ? "" : "\u{1F5D1}" // "🗑"
        default: return ""
        }
    }
}

extension BreakTableView: NSTableViewDelegate {

    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {

        if tableColumn?.identifier.rawValue == "addr" {
            if let cell = cell as? NSTextFieldCell {

                let last = row == breakCnt
                let disabled = !last && disabledCache[row] == true
                let selected = tableView.selectedRow == row
                let edited = tableView.editedRow == row

                cell.textColor =
                    disabled ? NSColor.disabledControlTextColor :
                    edited ? NSColor.textColor :
                    selected ? NSColor.white :
                    last ? NSColor.disabledControlTextColor :
                    NSColor.textColor
            }
        }
    }

    func tableView(_ tableView: NSTableView, shouldEdit tableColumn: NSTableColumn?, row: Int) -> Bool {

        if tableColumn?.identifier.rawValue == "addr" {
            return row == numberOfRows(in: tableView) - 1
        }

        return false
    }

    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {

        if tableColumn?.identifier.rawValue != "addr" { NSSound.beep(); return }
        guard let addr = object as? UInt32 else { NSSound.beep(); return }

        lockAmiga()

        if amiga?.cpu.breakpointIsSet(at: addr) == false {
            amiga?.cpu.addBreakpoint(at: addr)
        } else {
            NSSound.beep()
        }
        inspector.needsRefresh()

        unlockAmiga()
    }
}
