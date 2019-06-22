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

    var memory = amigaProxy?.mem
    var bank = 0
    
    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
        action = #selector(clickAction(_:))
    }
    
    func refresh() {
        
        track()
        memory = amigaProxy?.mem
        reloadData()
    }
    
    @IBAction func clickAction(_ sender: NSTableView!) {
        
        let row = sender.clickedRow
        inspector.setBank(row)
    }
}

extension BankTableView: NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int { return 256; }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        switch tableColumn?.identifier.rawValue {
            
        case "bank":
            return row
            
        case "source":
            
            let src = memory?.memSrc(row << 16).rawValue
            
            switch src {
                
            case MEM_UNMAPPED.rawValue:
                return "Unmapped"
            case MEM_CHIP.rawValue:
                return "Chip Ram"
            case MEM_FAST.rawValue:
                return "Fast Ram"
            case MEM_CIA.rawValue:
                return "CIA"
            case MEM_SLOW.rawValue:
                return "Slow Ram"
            case MEM_RTC.rawValue:
                return "Clock"
            case MEM_OCS.rawValue:
                return "OCS"
            case MEM_BOOT.rawValue:
                return "Boot Rom"
            case MEM_KICK.rawValue:
                return "Kickstart"
            case MEM_EXTROM.rawValue:
                return "Extended Rom"
            default:
                return "???"
            }
        default:
            return "???"
        }
    }
}

extension BankTableView: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        if let cell = cell as? NSTextFieldCell {
            
            let src = memory?.memSrc(row << 16).rawValue
            switch src {
            case MEM_UNMAPPED.rawValue:
                cell.textColor = .gray
            default:
                cell.textColor = .textColor
            }
        }
    }
}
