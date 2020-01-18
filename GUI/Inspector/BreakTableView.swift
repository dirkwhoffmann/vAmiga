// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class PointTableView: NSTableView {

    @IBOutlet weak var inspector: Inspector!

    // Data caches
    var disabledCache: [Int: Bool] = [:]
    var addrCache: [Int: UInt32] = [:]
    var numRows = 0

    override func awakeFromNib() {

        delegate = self
        dataSource = self
        target = self

        action = #selector(clickAction(_:))
    }

    func cache() { }
    func click(row: Int, col: Int) { }
    func edit(addr: UInt32) { }

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

        if count == 0 {

            refreshFormatters()
            cache()
            reloadData()
        }
    }

    @IBAction func clickAction(_ sender: NSTableView!) {

        lockAmiga()
        click(row: sender.clickedRow, col: sender.clickedColumn)
        unlockAmiga()
    }
}

extension PointTableView: NSTableViewDataSource {

    func numberOfRows(in tableView: NSTableView) -> Int {

        return numRows + 1
    }

    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {

        let last = row == numRows

        switch tableColumn?.identifier.rawValue {

        case "break" where disabledCache[row] == true:
            return last ? "" : "\u{26AA}" /* ⚪ */
        case "break":
            return last ? "" : "\u{26D4}" /* ⛔ */
        case "addr":
            return last ? "Add address" : (addrCache[row] ?? "?")
        case "delete":
            return last ? "" : "\u{1F5D1}" // "🗑"

        default: return ""
        }
    }
}

extension PointTableView: NSTableViewDelegate {

    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {

        if tableColumn?.identifier.rawValue == "addr" {
            if let cell = cell as? NSTextFieldCell {

                let last = row == numRows
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
        edit(addr: addr)
        inspector.needsRefresh()
        unlockAmiga()
     }
}

class BreakTableView: PointTableView {

    override func cache() {

        numRows = amiga!.cpu.numberOfBreakpoints()

        for i in 0 ..< numRows {
            disabledCache[i] = amiga!.cpu.breakpointIsDisabled(i)
            addrCache[i] = amiga!.cpu.breakpointAddr(i)
        }
    }

    override func click(row: Int, col: Int) {

        if col == 0 {

            // Enable / Disable
            let disabled = amiga?.cpu.breakpointIsDisabled(row) ?? false
            amiga?.cpu.breakpointSetEnable(row, value: disabled)
            inspector.needsRefresh()
        }

        if col == 0 || col == 1 {

            // Jump to breakpoint address
            if let addr = amiga?.cpu.breakpointAddr(row), addr <= 0xFFFFFF { inspector.instrTableView.jumpTo(addr: addr)
            }
        }

        if col == 2 {

            // Delete
            amiga?.cpu.removeBreakpoint(row)
            inspector.needsRefresh()
        }
    }

    override func edit(addr: UInt32) {

        if amiga?.cpu.breakpointIsSet(at: addr) == false {
            amiga?.cpu.addBreakpoint(at: addr)
        } else {
            NSSound.beep()
        }
    }
}

class WatchTableView: PointTableView {

    override func cache() {

        numRows = amiga!.cpu.numberOfWatchpoints()

        for i in 0 ..< numRows {
            disabledCache[i] = amiga!.cpu.watchpointIsDisabled(i)
            addrCache[i] = amiga!.cpu.watchpointAddr(i)
        }
    }

    override func click(row: Int, col: Int) {

        if col == 0 {

             // Toggle enable status
             let disabled = amiga?.cpu.watchpointIsDisabled(row) ?? false
             amiga?.cpu.watchpointSetEnable(row, value: disabled)
             inspector.needsRefresh()
         }

         if col == 2 {

             // Delete
             amiga?.cpu.removeWatchpoint(row)
             inspector.needsRefresh()
         }
    }

    override func edit(addr: UInt32) {

        if amiga?.cpu.watchpointIsSet(at: addr) == false {
            amiga?.cpu.addWatchpoint(at: addr)
        } else {
            NSSound.beep()
        }
    }
}
