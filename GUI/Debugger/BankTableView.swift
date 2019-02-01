// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

class BankTableView : NSTableView {
    
    @IBOutlet weak var inspector: Inspector!

    var memory = amigaProxy?.mem
    var bank = 0
    
    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
        action = #selector(clickAction(_:))
//        doubleAction = #selector(doubleClickAction(_:))
        
        // Assign formatters
        /*
        let columnFormatters = [
            "addr" : fmt24,
            "0" : fmt16,
            "2" : fmt16,
            "4" : fmt16,
            "6" : fmt16,
            "8" : fmt16,
            "A" : fmt16,
            "C" : fmt16,
            "E" : fmt16,
            ]
        
        for (column, formatter) in columnFormatters {
            let columnId = NSUserInterfaceItemIdentifier(rawValue: column)
            if let tableColumn = tableColumn(withIdentifier: columnId) {
                if let cell = tableColumn.dataCell as? NSCell {
                    cell.formatter = formatter
                }
            }
        }
        */
    }
    
    func refresh() {
        
        track()
        memory = amigaProxy?.mem
        reloadData()
    }
    
    @IBAction func clickAction(_ sender: NSTableView!) {
        
        let row = sender.clickedRow
        track("Switching to bank \(row)")
        
        inspector.bank = row
    }
}

extension BankTableView : NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int { return 255; }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        switch tableColumn?.identifier.rawValue {
            
        case "bank":
            return row
            
        case "source":
            
            let src = memory?.memSrc(row << 16).rawValue
            
            switch (src) {
                
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
            case MEM_CUSTOM.rawValue:
                return "OCS"
            case MEM_ROM.rawValue:
                return "Kickstart"
            case MEM_WOM.rawValue:
                return "WOM"
            default:
                return "???"
            }
        default:
            return "???"
        }
    }
}

extension BankTableView : NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        if let cell = cell as? NSTextFieldCell {
            
            if tableColumn?.identifier.rawValue == "source" {
                
                let src = memory?.memSrc(row << 16).rawValue
                switch (src) {
                case MEM_UNMAPPED.rawValue:
                    cell.textColor = .gray
                default:
                    cell.textColor = .textColor
                }
                
            } else {
                
                cell.textColor = .textColor
            }
        }
    }
}

