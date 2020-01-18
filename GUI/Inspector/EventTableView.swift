// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class EventTableView: NSTableView {

    override func awakeFromNib() {
        
        delegate = self
        dataSource = self
        target = self
    }
    
    func refresh(count: Int) {
                
        reloadData()
    }
}

extension EventTableView: NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {

        return SLOT_COUNT.rawValue
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        guard let info = amiga?.agnus.getEventSlotInfo(row) else { return nil }
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
            } else if info.frameRel == -1 {
                return "previous"
            } else if info.frameRel == 0 {
                return "current"
            } else {
                return "upcoming"
            }
        case "vpos":
            if willTrigger && info.frameRel == 0 {
                return info.vpos
            } else {
                return ""
            }
        case "hpos":
            if willTrigger && info.frameRel == 0 {
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

extension EventTableView: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        if let cell = cell as? NSTextFieldCell {
            if tableColumn?.identifier.rawValue != "slot" {
                if amiga?.agnus.getEventSlotInfo(row).trigger == INT64_MAX {
                    cell.textColor = .secondaryLabelColor
                    return
                }
            }
            
            cell.textColor = .textColor
        }
    }
}
