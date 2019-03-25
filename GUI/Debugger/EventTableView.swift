// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class EventTableView : NSTableView {
    
    var amiga = amigaProxy
    var primary = false
    
    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
    }
    
    func refresh(everything: Bool) {
        
        if (everything) {
            
            amiga = amigaProxy
        }
        
        reloadData()
    }
}

extension EventTableView : NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {
        
        return primary ? primarySlotCount : secondarySlotCount;
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        let info = primary ?
            amiga!.dma.getPrimarySlotInfo(row) :
            amiga!.dma.getSecondarySlotInfo(row)
        
        let willTrigger = (info.trigger != INT64_MAX)
        
        switch tableColumn?.identifier.rawValue {
            
        case "slot":    return String.init(cString: info.slotName)
        case "event":   return String.init(cString: info.eventName)
        case "trigger":
            if willTrigger {
                return String(format: "%lld", info.trigger)
            } else {
                return "none"
            }
        case "frame":
            if !willTrigger {
                return ""
            } else if info.frame == 0 {
                return "current"
            } else {
                return String(format: "%lld", info.frame)
            }
        case "vpos":
            if willTrigger {
                return info.vpos
            } else {
                return ""
            }
        case "hpos":
            if willTrigger {
                return info.hpos
            } else {
                return ""
            }
        case "remark":
            if !willTrigger {
                return ""
            } else if info.triggerRel == 0 {
                return "due immediately"
            } else {
                return "due in \(info.triggerRel / 8) DMA cycles"
            }
        default:        return "???"
        }
    }
}

extension EventTableView : NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        if let cell = cell as? NSTextFieldCell {
            
            if tableColumn?.identifier.rawValue != "slot" {
                
                let info = primary ?
                    amiga!.dma.getPrimarySlotInfo(row) :
                    amiga!.dma.getSecondarySlotInfo(row)
                
                if info.trigger == INT64_MAX {
                    cell.textColor = .secondaryLabelColor
                    return
                }
            }
            
            cell.textColor = .textColor
        }
    }
}
