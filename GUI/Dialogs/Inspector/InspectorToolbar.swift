// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class InspectorToolbar: NSToolbar {
    
    @IBOutlet weak var inspector: Inspector!
    @IBOutlet weak var timeStamp: NSButton!
    @IBOutlet weak var selector: NSPopUpButton!
    @IBOutlet weak var formatSegCtrl: NSSegmentedControl!
    @IBOutlet weak var execSegCtrl: NSSegmentedControl!

    var emu: EmulatorProxy! { return inspector.parent.emu }
    
    override func validateVisibleItems() {

    }
    
    func updateToolbar(info: AmigaInfo, full: Bool) {
        
        let frame = inspector.agnusInfo.frame
        let vpos = inspector.agnusInfo.vpos
        let hpos = inspector.agnusInfo.hpos
        
        if full {
            
            let running = emu.running
            
            let label = running ? "Pause" : "Run"
            let image = running ? "pauseTemplate" : "runTemplate"

            execSegCtrl.setToolTip(label, forSegment: 0)
            execSegCtrl.setImage(NSImage(named: image), forSegment: 0)
            for i in 1...4 { execSegCtrl.setEnabled(!running, forSegment: i) }
            timeStamp.font = NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
        }
        
        var format = ""
        if inspector.hex {
            format = inspector.padding ? "%X:%03X:%03X" : "%X:%X:%X"
        } else {
            format = inspector.padding ? "%d:%03d:%03d" : "%d:%d:%d"
        }
        timeStamp.title = String(format: format, frame, vpos, hpos)
    }
    
    //
    // Action methods
    //
    
    @IBAction func panelAction(_ sender: NSPopUpButton) {

        inspector.selectPanel(sender.selectedTag())
    }
 
    @IBAction func formatAction(_ sender: NSPopUpButton) {

        inspector.format = sender.selectedTag()
    }

    @IBAction func execAction(_ sender: NSSegmentedControl) {
        
        switch sender.selectedSegment {
            
        case 0: emu.running ? emu.pause() : try? emu.run()
        case 1: emu.stepOver()
        case 2: emu.stepInto()
        case 3: emu.finishLine()
        case 4: emu.finishFrame()
            
        default:
            fatalError()
        }
    }
    
    @IBAction func plusAction(_ sender: NSButton) {

        inspector.parent.addInspector()
    }
    
    @IBAction func hexAction(_ sender: NSButton) {
        
        if sender.state == .on {
            
            emu.set(.CPU_DASM_NUMBERS, value: DasmNumbers.HEX.rawValue)
            inspector.hex = true
            
        } else {
            
            emu.set(.CPU_DASM_NUMBERS, value: DasmNumbers.DEC.rawValue)
            inspector.hex = false
        }
    }
}
