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
    @IBOutlet weak var cpuStepOverButton: NSButton!
    @IBOutlet weak var cpuTraceStopAndGoButton: NSButton!
    @IBOutlet weak var cpuTraceStepIntoButton: NSButton!
    @IBOutlet weak var cpuTraceStepOverButton: NSButton!
    @IBOutlet weak var cpuTraceClearButton: NSButton!

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
    
    // Debug panel (Agnus)
    @IBOutlet weak var dmaDMACON: NSTextField!
    @IBOutlet weak var dmaBLTPRI: NSButton!
    @IBOutlet weak var dmaDMAEN: NSButton!
    @IBOutlet weak var dmaBPLEN: NSButton!
    @IBOutlet weak var dmaCOPEN: NSButton!
    @IBOutlet weak var dmaBLTEN: NSButton!
    @IBOutlet weak var dmaSPREN: NSButton!
    @IBOutlet weak var dmaDSKEN: NSButton!
    @IBOutlet weak var dmaAUD3EN: NSButton!
    @IBOutlet weak var dmaAUD2EN: NSButton!
    @IBOutlet weak var dmaAUD1EN: NSButton!
    @IBOutlet weak var dmaAUD0EN: NSButton!

    @IBOutlet weak var dmaVPOS: NSTextField!
    @IBOutlet weak var dmaHPOS: NSTextField!

    @IBOutlet weak var dmaDIWSTRT: NSTextField!
    @IBOutlet weak var dmaDIWSTOP: NSTextField!
    @IBOutlet weak var dmaDDFSTRT: NSTextField!
    @IBOutlet weak var dmaDDFSTOP: NSTextField!
 
    @IBOutlet weak var dmaBPL1PT: NSTextField!
    @IBOutlet weak var dmaBPL2PT: NSTextField!
    @IBOutlet weak var dmaBPL3PT: NSTextField!
    @IBOutlet weak var dmaBPL4PT: NSTextField!
    @IBOutlet weak var dmaBPL5PT: NSTextField!
    @IBOutlet weak var dmaBPL6PT: NSTextField!

    @IBOutlet weak var dmaBPL1MOD: NSTextField!
    @IBOutlet weak var dmaBPL2MOD: NSTextField!

    // Debug panel (Copper)
    @IBOutlet weak var copActive: NSButton!
    @IBOutlet weak var copCOPPC: NSTextField!
    @IBOutlet weak var copCOPINS1: NSTextField!
    @IBOutlet weak var copCOPINS2: NSTextField!
    @IBOutlet weak var copCDANG: NSButton!

    @IBOutlet weak var copCOP1LC: NSTextField!
    @IBOutlet weak var copPlus1: NSButton!
    @IBOutlet weak var copMinus1: NSButton!
    @IBOutlet weak var copList1: CopperTableView!

    @IBOutlet weak var copCOP2LC: NSTextField!
    @IBOutlet weak var copPlus2: NSButton!
    @IBOutlet weak var copMinus2: NSButton!
    @IBOutlet weak var copList2: CopperTableView!

    // Debug panel (Denise)
    @IBOutlet weak var deniseBPLCON0: NSTextField!
    @IBOutlet weak var deniseHIRES: NSButton!
    @IBOutlet weak var deniseHOMOD: NSButton!
    @IBOutlet weak var deniseDBPLF: NSButton!
    @IBOutlet weak var deniseLACE: NSButton!
    @IBOutlet weak var deniseBPLCON1: NSTextField!
    @IBOutlet weak var deniseBPLCON2: NSTextField!

    @IBOutlet weak var dmaBpl1Enable: NSButton!
    @IBOutlet weak var dmaBpl2Enable: NSButton!
    @IBOutlet weak var dmaBpl3Enable: NSButton!
    @IBOutlet weak var dmaBpl4Enable: NSButton!
    @IBOutlet weak var dmaBpl5Enable: NSButton!
    @IBOutlet weak var dmaBpl6Enable: NSButton!

    // Debug panel (Paula)
    @IBOutlet weak var paulaIntena: NSTextField!
    @IBOutlet weak var paulaIntreq: NSTextField!
    @IBOutlet weak var paulaEna14: NSButton!
    @IBOutlet weak var paulaEna13: NSButton!
    @IBOutlet weak var paulaEna12: NSButton!
    @IBOutlet weak var paulaEna11: NSButton!
    @IBOutlet weak var paulaEna10: NSButton!
    @IBOutlet weak var paulaEna9: NSButton!
    @IBOutlet weak var paulaEna8: NSButton!
    @IBOutlet weak var paulaEna7: NSButton!
    @IBOutlet weak var paulaEna6: NSButton!
    @IBOutlet weak var paulaEna5: NSButton!
    @IBOutlet weak var paulaEna4: NSButton!
    @IBOutlet weak var paulaEna3: NSButton!
    @IBOutlet weak var paulaEna2: NSButton!
    @IBOutlet weak var paulaEna1: NSButton!
    @IBOutlet weak var paulaEna0: NSButton!
    @IBOutlet weak var paulaReq14: NSButton!
    @IBOutlet weak var paulaReq13: NSButton!
    @IBOutlet weak var paulaReq12: NSButton!
    @IBOutlet weak var paulaReq11: NSButton!
    @IBOutlet weak var paulaReq10: NSButton!
    @IBOutlet weak var paulaReq9: NSButton!
    @IBOutlet weak var paulaReq8: NSButton!
    @IBOutlet weak var paulaReq7: NSButton!
    @IBOutlet weak var paulaReq6: NSButton!
    @IBOutlet weak var paulaReq5: NSButton!
    @IBOutlet weak var paulaReq4: NSButton!
    @IBOutlet weak var paulaReq3: NSButton!
    @IBOutlet weak var paulaReq2: NSButton!
    @IBOutlet weak var paulaReq1: NSButton!
    @IBOutlet weak var paulaReq0: NSButton!
    @IBOutlet weak var audioWaveformView: WaveformView!
    @IBOutlet weak var audioBufferLevel: NSLevelIndicator!
    @IBOutlet weak var audioBufferLevelText: NSTextField!
    @IBOutlet weak var audioBufferUnderflows: NSTextField!
    @IBOutlet weak var audioBufferOverflows: NSTextField!
    
    // Debug Panel (Events)
    @IBOutlet weak var evMasterClock: NSTextField!
    @IBOutlet weak var evInCpuCycles: NSTextField!
    @IBOutlet weak var evInDmaCycles: NSTextField!
    @IBOutlet weak var evInCiaCycles: NSTextField!

    @IBOutlet weak var evCpuProgress: NSTextField!
    @IBOutlet weak var evDmaProgress: NSTextField!
    @IBOutlet weak var evCiaAProgress: NSTextField!
    @IBOutlet weak var evCiaBProgress: NSTextField!

    @IBOutlet weak var evFrame: NSTextField!
    @IBOutlet weak var evVPos: NSTextField!
    @IBOutlet weak var evHPos: NSTextField!

    @IBOutlet weak var evPrimTableView: EventTableView!
    @IBOutlet weak var evSecTableView: EventTableView!

    private var pendingRefreshWorkItem: DispatchWorkItem?
    

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
        
        // Configure the event tables
        evPrimTableView.primary = true
    }
    
    override func showWindow(_ sender: Any?) {

        track()
        super.showWindow(self)
        amigaProxy?.enableDebugging()
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
        
        if window?.isVisible == false { return }
        
        if let id = debugPanel.selectedTabViewItem?.label {
            
            switch id {

            case "CPU":
                refreshCPU(everything: everything)

            case "CIA":
                refreshCIA(everything: everything)
                
            case "Memory":
                refreshMemory(everything: everything)

            case "DMA":
                refreshAgnus(everything: everything)

            case "Copper":
                refreshCopper(everything: everything)

            case "Denise":
                refreshDenise(everything: everything)
                
            case "Paula":
                refreshPaula(everything: everything)

            case "Events":
                refreshEvents(everything: everything)
                
            default:
                break
            }
        }
    }
}

extension Inspector : NSWindowDelegate {
    
    func windowWillClose(_ notification: Notification) {
        
        track("Closing inspector")
        amigaProxy?.disableDebugging()
    }
}

extension Inspector : NSTabViewDelegate {
    
    func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {
        refresh(everything: true)
    }
}
