// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

let fmt8  = MyFormatter.init(radix: 16, min: 0, max: 0xFF)
let fmt16 = MyFormatter.init(radix: 16, min: 0, max: 0xFFFF)
let fmt24 = MyFormatter.init(radix: 16, min: 0, max: 0xFFFFFF)
let fmt32 = MyFormatter.init(radix: 16, min: 0, max: 0xFFFFFFFF)

class Inspector : NSWindowController
{
    // Debug panel (Commons)
    @IBOutlet weak var debugPanel: NSTabView!
    
    // Debug panel (CPU)
    @IBOutlet weak var instrTableView: InstrTableView!
    @IBOutlet weak var traceTableView: TraceTableView!
    @IBOutlet weak var breakTableView: BreakTableView!
    @IBOutlet weak var cpuPC: NSTextField!
    @IBOutlet weak var cpuD0: NSTextField!
    @IBOutlet weak var cpuD1: NSTextField!
    @IBOutlet weak var cpuD2: NSTextField!
    @IBOutlet weak var cpuD3: NSTextField!
    @IBOutlet weak var cpuD4: NSTextField!
    @IBOutlet weak var cpuD5: NSTextField!
    @IBOutlet weak var cpuD6: NSTextField!
    @IBOutlet weak var cpuD7: NSTextField!
    @IBOutlet weak var cpuA0: NSTextField!
    @IBOutlet weak var cpuA1: NSTextField!
    @IBOutlet weak var cpuA2: NSTextField!
    @IBOutlet weak var cpuA3: NSTextField!
    @IBOutlet weak var cpuA4: NSTextField!
    @IBOutlet weak var cpuA5: NSTextField!
    @IBOutlet weak var cpuA6: NSTextField!
    @IBOutlet weak var cpuA7: NSTextField!
    @IBOutlet weak var cpuSSP: NSTextField!

    @IBOutlet weak var cpuT: NSButton!
    @IBOutlet weak var cpuS: NSButton!
    @IBOutlet weak var cpuI2: NSButton!
    @IBOutlet weak var cpuI1: NSButton!
    @IBOutlet weak var cpuI0: NSButton!
    @IBOutlet weak var cpuX: NSButton!
    @IBOutlet weak var cpuN: NSButton!
    @IBOutlet weak var cpuZ: NSButton!
    @IBOutlet weak var cpuV: NSButton!
    @IBOutlet weak var cpuC: NSButton!

    @IBOutlet weak var cpuStopAndGoButton: NSButton!
    @IBOutlet weak var cpuStepIntoButton: NSButton!
    @IBOutlet weak var cupStepOverButton: NSButton!

    
    // Debug panel (Memory)
    @IBOutlet weak var memSearchField: NSSearchField!
    @IBOutlet weak var memBankTableView: BankTableView!
    @IBOutlet weak var memTableView: MemTableView!
    @IBOutlet weak var memLayoutButton: NSButton!
    @IBOutlet weak var memLayoutSlider: NSSlider!
    @IBOutlet weak var memChipRamButton: NSButton!
    @IBOutlet weak var memChipRamText: NSTextField!
    @IBOutlet weak var memSlowRamButton: NSButton!
    @IBOutlet weak var memSlowRamText: NSTextField!
    @IBOutlet weak var memFastRamButton: NSButton!
    @IBOutlet weak var memFastRamText: NSTextField!
    @IBOutlet weak var memCIAButton: NSButton!
    @IBOutlet weak var memRTCButton: NSButton!
    @IBOutlet weak var memOCSButton: NSButton!
    @IBOutlet weak var memRomButton: NSButton!

    var bank = 0
    var memSrc = MEM_CHIP
    var selected = -1
    
    // Debug panel (CIA)
    @IBOutlet weak var ciaSelector: NSSegmentedControl!
    
    @IBOutlet weak var ciaPRA: NSTextField!
    @IBOutlet weak var ciaPRAbinary: NSTextField!
    @IBOutlet weak var ciaDDRA: NSTextField!
    @IBOutlet weak var ciaDDRAbinary: NSTextField!
    @IBOutlet weak var ciaPA7: NSButton!
    @IBOutlet weak var ciaPA6: NSButton!
    @IBOutlet weak var ciaPA5: NSButton!
    @IBOutlet weak var ciaPA4: NSButton!
    @IBOutlet weak var ciaPA3: NSButton!
    @IBOutlet weak var ciaPA2: NSButton!
    @IBOutlet weak var ciaPA1: NSButton!
    @IBOutlet weak var ciaPA0: NSButton!

    @IBOutlet weak var ciaPRB: NSTextField!
    @IBOutlet weak var ciaPRBbinary: NSTextField!
    @IBOutlet weak var ciaDDRB: NSTextField!
    @IBOutlet weak var ciaDDRBbinary: NSTextField!
    @IBOutlet weak var ciaPB7: NSButton!
    @IBOutlet weak var ciaPB6: NSButton!
    @IBOutlet weak var ciaPB5: NSButton!
    @IBOutlet weak var ciaPB4: NSButton!
    @IBOutlet weak var ciaPB3: NSButton!
    @IBOutlet weak var ciaPB2: NSButton!
    @IBOutlet weak var ciaPB1: NSButton!
    @IBOutlet weak var ciaPB0: NSButton!
    
    @IBOutlet weak var ciaTA: NSTextField!
    @IBOutlet weak var ciaTAlatch: NSTextField!
    @IBOutlet weak var ciaTArunning: NSButton!
    @IBOutlet weak var ciaTAtoggle: NSButton!
    @IBOutlet weak var ciaTApbout: NSButton!
    @IBOutlet weak var ciaTAoneShot: NSButton!
    
    @IBOutlet weak var ciaTB: NSTextField!
    @IBOutlet weak var ciaTBlatch: NSTextField!
    @IBOutlet weak var ciaTBrunning: NSButton!
    @IBOutlet weak var ciaTBtoggle: NSButton!
    @IBOutlet weak var ciaTBpbout: NSButton!
    @IBOutlet weak var ciaTBoneShot: NSButton!
    
    @IBOutlet weak var ciaICR: NSTextField!
    @IBOutlet weak var ciaICRbinary: NSTextField!
    @IBOutlet weak var ciaIMR: NSTextField!
    @IBOutlet weak var ciaIMRbinary: NSTextField!
    @IBOutlet weak var ciaIntLineLow: NSButton!

    @IBOutlet weak var ciaCntHi: NSTextField!
    @IBOutlet weak var ciaCntMid: NSTextField!
    @IBOutlet weak var ciaCntLo: NSTextField!
    @IBOutlet weak var ciaCntIntEnable: NSButton!
    @IBOutlet weak var ciaAlarmHi: NSTextField!
    @IBOutlet weak var ciaAlarmMid: NSTextField!
    @IBOutlet weak var ciaAlarmLo: NSTextField!

    @IBOutlet weak var ciaSDR: NSTextField!
    @IBOutlet weak var ciaSDRbinary: NSTextField!
    
    @IBOutlet weak var ciaIdleCycles: NSTextField!
    @IBOutlet weak var ciaIdleLevelText: NSTextField!
    @IBOutlet weak var ciaIdleLevel: NSLevelIndicator!
    
    // Factory method
    static func make() -> Inspector? {
        
        return Inspector.init(windowNibName: "Inspector")
    }
    
    override func awakeFromNib() {
        
        track()
        
        // Create and assign binary number formatter
        let bF = MyFormatter.init(radix: 2, min: 0, max: 255)
        ciaPRAbinary.formatter = bF
        ciaDDRAbinary.formatter = bF
        ciaPRBbinary.formatter = bF
        ciaDDRBbinary.formatter = bF
        ciaSDRbinary.formatter = bF
        ciaICRbinary.formatter = bF
        ciaIMRbinary.formatter = bF
        
        // Refresh the currently shown panel
        refresh(everything: true)
    }
    
    // Assigns a number formatter to a control
    func assignFormatter(_ formatter: Formatter, _ control: NSControl) {
        control.abortEditing()
        control.formatter = formatter
        control.needsDisplay = true
    }
    /*
    func assignFormatter(_ formatter: Formatter, _ controls: [NSControl]) {
        for control in controls {
            assignFormatter(formatter, control)
        }
    }
    */
    
    // Updates the currently shown panel
    func refresh(everything: Bool) {
        
        track()
        if let id = debugPanel.selectedTabViewItem?.label {
            
            track("\(id)")
            switch id {

            case "CPU":
                track()
                refreshCPU(everything: everything)

            case "CIA":
                track()
                refreshCIA(everything: everything)
                
            case "Memory":
                track()
                refreshMemory(everything: everything)
                
            default:
                break
            }
        }
    }
}

/*
extension Inspector : NSWindowDelegate {
    
}
*/

extension Inspector : NSTabViewDelegate {
    
    func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {
        refresh(everything: true)
    }
}
