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
    
        guard let eventInfo = amiga?.dma.getEventInfo() else { return "" }
        
        var info : EventSlotInfo
        
        switch(row) {
        case 0:  info = primary ? eventInfo.primary.0 : eventInfo.secondary.0
        case 1:  info = primary ? eventInfo.primary.1 : eventInfo.secondary.1
        case 2:  info = primary ? eventInfo.primary.2 : eventInfo.secondary.2
        case 3:  info = primary ? eventInfo.primary.3 : eventInfo.secondary.3
        case 4:  info = primary ? eventInfo.primary.4 : eventInfo.secondary.4
        case 5:  info = primary ? eventInfo.primary.5 : eventInfo.secondary.5
        case 6:  info = primary ? eventInfo.primary.6 : eventInfo.secondary.6
        case 7:  info = eventInfo.secondary.7
        case 8:  info = eventInfo.secondary.8
        case 9:  info = eventInfo.secondary.9
        case 10: info = eventInfo.secondary.10
        case 11: info = eventInfo.secondary.11
        case 12: info = eventInfo.secondary.12
        case 13: info = eventInfo.secondary.13
        case 14: info = eventInfo.secondary.14
        default: return "???"
        }
    
        let never = (info.trigger == INT64_MAX)
        let currentFrame = (info.frame == 0)
        let cycleDiff = (info.trigger - eventInfo.dmaClock) / 2
   
        switch tableColumn?.identifier.rawValue {
            
        case "slot":    return String.init(cString: info.slotName)
        case "event":   return String.init(cString: info.eventName)
        case "trigger": return never ? "never" : String(format: "%lld", info.trigger)
        case "frame":   return currentFrame ? "current" : String(format: "%lld", info.frame)
        case "vpos":    return info.vpos
        case "hpos":    return info.hpos
        case "remark":  return never ? "" : "due in \(cycleDiff) DMA cycles"
        default:        return "???"
        }
    }
}

extension EventTableView : NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        if let cell = cell as? NSTextFieldCell {
            if tableColumn?.identifier.rawValue == "instr" {
                cell.textColor = .red
                return
            }
            cell.textColor = .textColor
        }
    }
}
