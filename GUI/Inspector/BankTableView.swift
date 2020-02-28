// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class BankTableView: NSTableView {
    
    @IBOutlet weak var inspector: Inspector!

    // Displayed memory bank
    var bank = 0

    // Data caches
    var bankCache: [Int: MemorySource] = [:]

    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
        action = #selector(clickAction(_:))
    }

    func cache() {

        for i in 0 ..< 256 {
            bankCache[i] = amiga!.mem.memSrc(i << 16)
        }
    }

    func refreshValues() {

        cache()
        reloadData()
    }

    func fullRefresh() {

        refreshValues()
    }

    func periodicRefresh(count: Int) {

        // Increase the update interval
        if count % 8 != 0 { return }

        refreshValues()
    }

    @IBAction func clickAction(_ sender: NSTableView!) {

        lockAmiga()
        inspector.setBank(sender.clickedRow)
        unlockAmiga()
    }
}

extension BankTableView: NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int { return 256; }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        switch tableColumn?.identifier.rawValue {

        case "source":

            switch bankCache[row]?.rawValue {
                
            case MEM_UNMAPPED.rawValue:
                return "Unmapped"
            case MEM_CHIP.rawValue:
                return "Chip Ram"
            case MEM_FAST.rawValue:
                return "Fast Ram"
            case MEM_SLOW.rawValue:
                return "Slow Ram"
            case MEM_CIA.rawValue:
                return "CIA"
            case MEM_RTC.rawValue:
                return "Clock"
            case MEM_OCS.rawValue:
                return "OCS"
            case MEM_AUTOCONF.rawValue:
                return "Autoconf"
            case MEM_ROM.rawValue:
                return "Rom"
            case MEM_WOM.rawValue:
                return "Wom"
            case MEM_EXT.rawValue:
                return "Extended Rom"
            default:
                return "???"
            }

        default:
            return row
        }
    }
}

extension BankTableView: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        if let cell = cell as? NSTextFieldCell {

            if bankCache[row] == MEM_UNMAPPED {
                cell.textColor = .gray
            } else {
                cell.textColor = .textColor
            }
        }
    }
}
