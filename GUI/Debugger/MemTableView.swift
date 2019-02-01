// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation


struct MemoryHighlighting {
    static let none = 0
    static let paula = 1
    static let denise = 2
    static let agnus = 3
}

class MemTableView : NSTableView {
    
    var highlighting = MemoryHighlighting.none
    var memory = amigaProxy?.mem
    var bank = 0

    @IBOutlet weak var inspector: Inspector!
    
    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
    
        // Assign formatters
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
        
    }
    
    func refresh() {
        
        track()
        memory = amigaProxy?.mem
        reloadData()
    }
    
    func setHighlighting(_ value : Int) {
        
        highlighting = value
        refresh()
    }
    
    // Returns the memory source for the specified address


    // Return true if the specified memory address should be displayed
    func shouldDisplay(_ addr: UInt16) -> Bool {
        
        return true
    }
        
    // Return true if the specified memory address should be highlighted
    func shouldHighlight(_ addr: UInt16) -> Bool {
        
        return false
    }
}

extension MemTableView : NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        
        return 65536 / 16;
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        // guard let mem = amigaProxy?.mem else { return "" }
        
        // track("\(row) \(tableColumn?.identifier.rawValue)")
        
        var addr = inspector.bank * 65536 + row * 16
     
        switch tableColumn?.identifier.rawValue {
            
        case "src":
            return "??"
       
        case "addr":
            return addr
            
        case "ascii":
            return memory?.ascii(addr) ?? ""
            
        case "E": addr += 2; fallthrough
        case "C": addr += 2; fallthrough
        case "A": addr += 2; fallthrough
        case "8": addr += 2; fallthrough
        case "6": addr += 2; fallthrough
        case "4": addr += 2; fallthrough
        case "2": addr += 2; fallthrough
        case "0": return memory?.spypeek16(addr) ?? ""
      
        default:
            return 42
        }
    }
}

extension MemTableView : NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        let cell = cell as! NSTextFieldCell

        if (tableColumn?.identifier.rawValue == "src") {
            // cell.font = NSFont.systemFont(ofSize: 9)
            cell.textColor = .gray
        } else {
            cell.textColor = NSColor.textColor
        }
        
        if (tableColumn?.identifier.rawValue == "ascii") {
            // cell.font = cbmfont
        }
        
        if shouldHighlight(UInt16(4 * row)) {
            cell.textColor = .systemRed
        } 
    }
    
    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {
        
    }
}
