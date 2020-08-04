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
    var amiga: AmigaProxy!

    // Data caches
    var disabledCache: [Int: Bool] = [:]
    var addrCache: [Int: UInt32] = [:]
    var numRows = 0

    override func awakeFromNib() {

        amiga = inspector.amiga
        delegate = self
        dataSource = self
        target = self

        action = #selector(clickAction(_:))
    }

    func cache() { }
    func click(row: Int, col: Int) { }
    func edit(row: Int, addr: UInt32) { }

    func refresh(count: Int = 0, full: Bool = false) {

        if full {
            for (c, f) in ["addr": fmt24] {
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
    }

    @IBAction func clickAction(_ sender: NSTableView!) {

        click(row: sender.clickedRow, col: sender.clickedColumn)
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
            // return row == numberOfRows(in: tableView) - 1
            return true
        }

        return false
    }

    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {

        if tableColumn?.identifier.rawValue != "addr" { NSSound.beep(); return }
        guard let addr = object as? UInt32 else { NSSound.beep(); return }

        edit(row: row, addr: addr)
        inspector.fullRefresh()
     }
}

class BreakTableView: PointTableView {

    override func cache() {

        numRows = amiga.cpu.numberOfBreakpoints

        for i in 0 ..< numRows {
            disabledCache[i] = amiga.cpu.breakpointIsDisabled(i)
            addrCache[i] = amiga.cpu.breakpointAddr(i)
        }
    }

    override func click(row: Int, col: Int) {
        
        amiga.suspend()
        
        if col == 0 {
            
            // Enable / Disable
            let disabled = amiga.cpu.breakpointIsDisabled(row)
            amiga.cpu.breakpointSetEnable(row, value: disabled)
            inspector.fullRefresh()
        }
        
        if col == 0 || col == 1 {
            
            // Jump to breakpoint address
            let addr = amiga.cpu.breakpointAddr(row)
            if addr <= 0xFFFFFF {
                inspector.fullRefresh()
                inspector.cpuInstrView.jumpTo(addr: addr)
            }
        }
        
        if col == 2 {
            
            // Delete
            amiga.cpu.removeBreakpoint(row)
            inspector.fullRefresh()
        }
        
        amiga.resume()
    }

    override func edit(row: Int, addr: UInt32) {

        // Abort if a breakpoint has been set already
        if amiga.cpu.breakpointIsSet(at: addr) { NSSound.beep(); return }
        
        amiga.suspend()
        
        if row == numRows {
            amiga.cpu.addBreakpoint(at: addr)
        } else {
            assert(row < numRows)
            amiga.cpu.replaceBreakpoint(row, addr: addr)
        }
        
        inspector.cpuInstrView.jumpTo(addr: addr)
        
        amiga.resume()
    }
}

class WatchTableView: PointTableView {

    override func cache() {

        numRows = amiga.cpu.numberOfWatchpoints()

        for i in 0 ..< numRows {
            disabledCache[i] = amiga.cpu.watchpointIsDisabled(i)
            addrCache[i] = amiga.cpu.watchpointAddr(i)
        }
    }

    override func click(row: Int, col: Int) {

        if col == 0 {

             // Toggle enable status
            let disabled = amiga.cpu.watchpointIsDisabled(row)
             amiga.cpu.watchpointSetEnable(row, value: disabled)
             inspector.fullRefresh()
         }

         if col == 2 {

             // Delete
             amiga.cpu.removeWatchpoint(row)
             inspector.fullRefresh()
         }
    }
    
    override func edit(row: Int, addr: UInt32) {
        
        // Abort if a watchpoint has been set already
        if amiga.cpu.watchpointIsSet(at: addr) { NSSound.beep(); return }
        
        amiga.suspend()
        
        if row == numRows {
            amiga.cpu.addWatchpoint(at: addr)
        } else {
            assert(row < numRows)
            amiga.cpu.replaceWatchpoint(row, addr: addr)
        }
        
        amiga.resume()
    }
}
