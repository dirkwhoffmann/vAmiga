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
    @IBOutlet weak var controlsSegCtrl: NSSegmentedControl!
    
    var emu: EmulatorProxy! { return inspector.parent.emu }
    
    override func validateVisibleItems() {

    }
    
    func updateToolbar(info: AmigaInfo, full: Bool) {
        
        if full {
            
            let running = emu.running
            
            let label = running ? "Pause" : "Run"
            let image = running ? "pauseTemplate" : "runTemplate"

            controlsSegCtrl.setToolTip(label, forSegment: 0)
            controlsSegCtrl.setImage(NSImage(named: image), forSegment: 0)
            for i in 1...4 { controlsSegCtrl.setEnabled(!running, forSegment: i) }
            timeStamp.font = NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
        }
        
        timeStamp.title = String(format: "%d:%03d:%03d", info.frame, info.vpos, info.hpos)
    }
    
    //
    // Action methods
    //
    
    @IBAction func panelAction(_ sender: NSPopUpButton) {

        inspector.selectPanel(sender.selectedTag())
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
