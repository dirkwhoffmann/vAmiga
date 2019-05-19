// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class SpriteTableView: NSTableView {
    
    @IBOutlet weak var inspector: Inspector!
    
    var amiga = amigaProxy
    
    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
    }
    
    func refresh(everything: Bool) {

        if everything {
           
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
}

extension SpriteTableView: NSTableViewDataSource {
    
    func colorIndex(tableColumn: NSTableColumn?, row: Int) -> Int? {
       
        if let id = tableColumn?.identifier.rawValue, let nr = Int(id) {

            let sprInfo = amiga!.denise.getSpriteInfo(inspector.selectedSprite)
            let addr = Int(sprInfo.ptr) + 4 * row
            let data = (amiga!.mem.spypeek16(addr) & (0x8000 >> nr)) != 0
            let datb = (amiga!.mem.spypeek16(addr + 2) & (0x8000 >> nr)) != 0
            
            return (data ? 1 : 0) + (datb ? 2 : 0)
        }
        return nil
    }
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        
        if let sprInfo = amiga?.denise.getSpriteInfo(inspector.selectedSprite) {
            
            let sprLines = sprInfo.vstop - sprInfo.vstrt
            if sprLines >= 0 && sprLines < 128 {
                return Int(sprLines)
            }
        }
        return 0
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        return nil
    }
}

extension SpriteTableView: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        let cell = cell as? NSTextFieldCell
        let info = amiga!.denise.getInfo()
        
         if let index = colorIndex(tableColumn: tableColumn, row: row) {
            
            var color = NSColor.white
            
            switch inspector.selectedSprite {
                
            case 0, 1:
                switch index {
                case 1: color = NSColor.init(rgba: info.color.17)
                case 2: color = NSColor.init(rgba: info.color.18)
                case 3: color = NSColor.init(rgba: info.color.19)
                default: break
                }
                
            case 2, 3:
                switch index {
                case 1: color = NSColor.init(rgba: info.color.21)
                case 2: color = NSColor.init(rgba: info.color.22)
                case 3: color = NSColor.init(rgba: info.color.23)
                default: break
                }
                
            case 4, 5:
                switch index {
                case 1: color = NSColor.init(rgba: info.color.25)
                case 2: color = NSColor.init(rgba: info.color.26)
                case 3: color = NSColor.init(rgba: info.color.27)
                default: break
                }
                
            default:
                switch index {
                case 1: color = NSColor.init(rgba: info.color.29)
                case 2: color = NSColor.init(rgba: info.color.30)
                case 3: color = NSColor.init(rgba: info.color.31)
                default: break
                }
            }
            
            cell?.backgroundColor = color
        }
    }
    
    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {
        
        switch tableColumn?.identifier.rawValue {
            
        default:
            NSSound.beep()
            return
        }

        /*
        if let value = object as? UInt16 {
            amigaProxy?.suspend()
            amigaProxy?.mem.poke16(addr, value: Int(value))
            amigaProxy?.resume()
        }
        */
    }
}
