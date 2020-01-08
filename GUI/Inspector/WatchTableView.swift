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
        }
        reloadData()
    }

    @IBAction func clickAction(_ sender: NSTableView!) {

        let row = sender.clickedRow
        let col = sender.clickedColumn

        if col == 0 {

            // Delete watchpoint
            track("Deleting breakpoint \(row)")
            amigaProxy?.cpu.removeWatchpoint(row)
            refresh(everything: false)

        } else {

            // Enable or disable
            // TODO
        }
    }
}

extension WatchTableView: NSTableViewDataSource {

    func numberOfRows(in tableView: NSTableView) -> Int {

        track("numberOfWatchpoints = \(cpu.numberOfWatchpoints())")
        return cpu.numberOfWatchpoints() + 1
    }

    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {

        switch tableColumn?.identifier.rawValue {

        case "break":
            return "\u{1F5D1}" // "🗑"

        case "addr":
            return cpu.watchpointAddr(row)

        default:
            fatalError()
        }
    }
}

extension WatchTableView: NSTableViewDelegate {

    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {

        if let cell = cell as? NSTextFieldCell {

            let disabled = cpu.isDisabled(row)

            switch tableColumn?.identifier.rawValue {

            case "addr" where disabled:
                cell.textColor = NSColor.secondaryLabelColor

            case "addr":
                cell.textColor = NSColor.labelColor

            default:
                break
            }
        }
    }

    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {

        switch tableColumn?.identifier.rawValue {

        case "addr":
            if let value = object as? UInt32 {
                if cpu.setBreakpointAddr(row, addr: value) { return }
            }
        default:
            break
        }

        NSSound.beep()
    }
}
