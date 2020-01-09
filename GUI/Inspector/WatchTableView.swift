// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class WatchTableView: NSTableView {

    @IBOutlet weak var inspector: Inspector!

    var cpu = amigaProxy!.cpu!

    override func awakeFromNib() {

        delegate = self
        dataSource = self
        target = self

        action = #selector(clickAction(_:))
    }

    func refresh(everything: Bool) {

        if everything {
            cpu = amigaProxy!.cpu
            for (c, f) in ["addr": fmt24] {
                let columnId = NSUserInterfaceItemIdentifier(rawValue: c)
                if let column = tableColumn(withIdentifier: columnId) {
                    if let cell = column.dataCell as? NSCell {
                        cell.formatter = f
                    }
                }
            }
            reloadData()
        }
    }

    @IBAction func clickAction(_ sender: NSTableView!) {

        let row = sender.clickedRow
        let col = sender.clickedColumn

        if col == 0 {

            // Toggle enable status
            cpu.watchpointSetEnable(row, value: cpu.watchpointIsDisabled(row))
            reloadData()
        }
        
        if col == 2 {

            // Delete
            cpu.removeWatchpoint(row)
            reloadData()
        }
    }
}

extension WatchTableView: NSTableViewDataSource {

    func numberOfRows(in tableView: NSTableView) -> Int {

        return cpu.numberOfWatchpoints() + 1
    }

    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {

        let disabled = cpu.watchpointIsDisabled(row)
        let last = row == numberOfRows(in: tableView) - 1

        switch tableColumn?.identifier.rawValue {

        case "break" where disabled:
            return last ? "" : "\u{26AA}" // "⚪"

        case "break":
            return last ? "" : "\u{26D4}" // "⛔"

        case "addr": 
            return last ? "Add address" : cpu.watchpointAddr(row)

        case "delete":
            return last ? "" : "\u{1F5D1}" // "🗑"

        default:
            fatalError()
        }
    }
}

extension WatchTableView: NSTableViewDelegate {

    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {

        if tableColumn?.identifier.rawValue == "addr" {
            if let cell = cell as? NSTextFieldCell {

                let last = row == numberOfRows(in: tableView) - 1
                let disabled = !last && cpu.watchpointIsDisabled(row)
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

        track("Adding ")
        if tableColumn?.identifier.rawValue == "addr" {
            if let addr = object as? UInt32 {
                if !cpu.watchpointIsSet(at: addr) {
                    cpu.addWatchpoint(at: addr)
                    reloadData()
                    return
                }
            }
        }

        NSSound.beep()
    }
}
