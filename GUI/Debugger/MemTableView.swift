//
// This file is part of VirtualC64 - A cycle accurate Commodore 64 emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
//

import Foundation

struct MemoryView {
    static let cpuView = 0
    static let ramView = 1
    static let romView = 2
    static let ioView = 3
}

struct MemoryHighlighting {
    static let none = 0
    static let rom = 1
    static let romBasic = 2
    static let romChar = 3
    static let romKernal = 4
    static let crt = 5
    static let io = 6
    static let ioVic = 7
    static let ioSid = 8
    static let ioCia = 9
}

class MemTableView : NSTableView {
    
    var c : MyController? = nil
    var cbmfont = NSFont.init(name: "C64ProMono", size: 9)
    private var memView = MemoryView.cpuView
    private var highlighting = MemoryHighlighting.none
    
    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
    }
    
    func refresh() {
        
        reloadData()
    }
    
    func setMemView(_ value : Int) {
        memView = value
        refresh()
    }
    
    func setHighlighting(_ value : Int) {
        highlighting = value
        refresh()
    }
    
    // Returns the memory source for the specified address


    // Return true if the specified memory address should be displayed
    func shouldDisplay(_ addr: UInt16) -> Bool {
        
        return false
        }
    }
        
    // Return true if the specified memory address should be highlighted
    func shouldHighlight(_ addr: UInt16) -> Bool {
        
        return false
    }

extension MemTableView : NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        
        return 0x10000 / 4;
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        return "";
    }
}

extension MemTableView : NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        let cell = cell as! NSTextFieldCell

        if (tableColumn?.identifier.rawValue == "src") {
            cell.font = NSFont.systemFont(ofSize: 9)
            cell.textColor = .gray
        } else {
            cell.textColor = NSColor.textColor
        }
        
        if (tableColumn?.identifier.rawValue == "ascii") {
            cell.font = cbmfont
        }
        
        if shouldHighlight(UInt16(4 * row)) {
            cell.textColor = .systemRed
        } 
    }
    
    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {
        
    }
}
