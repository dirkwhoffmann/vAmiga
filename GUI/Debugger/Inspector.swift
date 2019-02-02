// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

let fmt8 = MyFormatter.init(radix: 16, min: 0, max: 0xFF)
let fmt16 = MyFormatter.init(radix: 16, min: 0, max: 0xFFFF)
let fmt24 = MyFormatter.init(radix: 16, min: 0, max: 0xFFFFFF)

class Inspector : NSWindowController
{
    // Debug panel (Commons)
    @IBOutlet weak var debugPanel: NSTabView!
    @IBOutlet weak var stopAndGoButton: NSButton!
    @IBOutlet weak var stepIntoButton: NSButton!
    @IBOutlet weak var stepOverButton: NSButton!
    
    // Debug panel (Memory)
    @IBOutlet weak var bankTableView: BankTableView!
    @IBOutlet weak var memTableView: MemTableView!
    // @IBOutlet weak var memBankField: NSTextField!
    // @IBOutlet weak var memBankStepper: NSStepper!
    // @IBOutlet weak var memAreaPopup: NSPopUpButton!
    // @IBOutlet weak var memSearchPopup: NSTextField!
    @IBOutlet weak var memLayoutButton: NSButton!
    @IBOutlet weak var memLayoutSlider: NSSlider!
    @IBOutlet weak var memChipRamButton: NSButton!
    @IBOutlet weak var memChipRamText: NSTextField!
    @IBOutlet weak var memSlowRamButton: NSButton!
    @IBOutlet weak var memSlowRamText: NSTextField!
    @IBOutlet weak var memFastRamButton: NSButton!
    @IBOutlet weak var memFastRamText: NSTextField!
    @IBOutlet weak var memIOButton: NSButton!
    @IBOutlet weak var memRomButton: NSButton!

    var bank = 0
    var memSrc = MEM_CHIP
    var selected = -1
    
    // Debug panel (CIA)
    @IBOutlet weak var ciaSelector: NSSegmentedControl!
    
    @IBOutlet weak var ciaPA: NSTextField!
    @IBOutlet weak var ciaPAbinary: NSTextField!
    @IBOutlet weak var ciaPRA: NSTextField!
    @IBOutlet weak var ciaDDRA: NSTextField!
    
    @IBOutlet weak var ciaPB: NSTextField!
    @IBOutlet weak var ciaPBbinary: NSTextField!
    @IBOutlet weak var ciaPRB: NSTextField!
    @IBOutlet weak var ciaDDRB: NSTextField!
    
    @IBOutlet weak var ciaTimerA: NSTextField!
    @IBOutlet weak var ciaLatchA: NSTextField!
    @IBOutlet weak var ciaRunningA: NSButton!
    @IBOutlet weak var ciaToggleA: NSButton!
    @IBOutlet weak var ciaPBoutA: NSButton!
    @IBOutlet weak var ciaOneShotA: NSButton!
    
    @IBOutlet weak var ciaTimerB: NSTextField!
    @IBOutlet weak var ciaLatchB: NSTextField!
    @IBOutlet weak var ciaRunningB: NSButton!
    @IBOutlet weak var ciaToggleB: NSButton!
    @IBOutlet weak var ciaPBoutB: NSButton!
    @IBOutlet weak var ciaOneShotB: NSButton!
    
    @IBOutlet weak var todHours: NSTextField!
    @IBOutlet weak var todMinutes: NSTextField!
    @IBOutlet weak var todSeconds: NSTextField!
    @IBOutlet weak var todTenth: NSTextField!
    @IBOutlet weak var todIntEnable: NSButton!
    @IBOutlet weak var alarmHours: NSTextField!
    @IBOutlet weak var alarmMinutes: NSTextField!
    @IBOutlet weak var alarmSeconds: NSTextField!
    @IBOutlet weak var alarmTenth: NSTextField!
    
    @IBOutlet weak var ciaIcr: NSTextField!
    @IBOutlet weak var ciaIcrBinary: NSTextField!
    @IBOutlet weak var ciaImr: NSTextField!
    @IBOutlet weak var ciaImrBinary: NSTextField!
    @IBOutlet weak var ciaIntLineLow: NSButton!

    // Currently shown memory bank
    // var bank = 0
    
    // Factory method
    static func make() -> Inspector? {
        
        return Inspector.init(windowNibName: "Inspector")
    }
    
    override func awakeFromNib() {
        
        track()
        
        // memSearchPopup.formatter = fmt24
        refresh()
        refreshMemoryLayout()
    }
    
    // Updates the currently shown panel
    func refresh() {
        
        track()
        if let id = debugPanel.selectedTabViewItem?.label {
            
            track("\(id)")
            switch id {
                
            case "CIA":
                track()
                break
                
            case "Memory":
                track()
                refreshMemory()
                
            default:
                break
            }
        }
    }
}

extension Inspector : NSTabViewDelegate {
    
    func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {
        refresh()
    }
}
