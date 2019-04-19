// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class SpriteTableView : NSTableView {
    
    @IBOutlet weak var inspector: Inspector!
    
    var amiga = amigaProxy
    
    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
    }
    
    func refresh(everything: Bool) {

        if (everything) {
           
            for (c,f) in ["addr" : fmt24] {
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

extension SpriteTableView : NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        
        if let sprInfo = amiga?.denise.getSpriteInfo(inspector.selectedSprite) {
            
            let sprLines = sprInfo.vstop - sprInfo.vstrt
            if (sprLines >= 0 && sprLines < 128) {
                return Int(sprLines)
            }
        }
        return 0
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        let sprInfo = amiga!.denise.getSpriteInfo(inspector.selectedSprite)
        
        let addr = Int(sprInfo.ptr) + 4 * row
        
        switch tableColumn?.identifier.rawValue {
            
        case "addr":
            return addr
            
        case "data":
            
            let data = amiga!.mem.spypeek16(addr)
            let datb = amiga!.mem.spypeek16(addr + 2)
            
            var result = [Character](repeating: " ", count: 16)

            for i in 0...15 {

                var col = (data & (1 << i) == 0) ? 0 : 1;
                col += (datb & (1 << i) == 0) ? 0 : 2;

                switch (col) {
                case 0: result[i] = "."
                case 1: result[i] = "1"
                case 2: result[i] = "2"
                case 3: result[i] = "3"
                default:  result[i] = "?"
                }
            }
            
            return String(result)
            
        default:
            return "???"
        }
    }
}

extension SpriteTableView : NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, setObjectValue object: Any?, for tableColumn: NSTableColumn?, row: Int) {
        
        switch(tableColumn?.identifier.rawValue) {
            
        case "data":
            track()
            
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
