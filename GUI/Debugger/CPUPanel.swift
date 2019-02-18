// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {
    
    func refreshCPU(everything: Bool) {
        
        track()
        
        guard let amiga = amigaProxy else { return }
        let info = amiga.cpu.getInfo()
        
        if everything {
            
            for (c,f) in [ cpuPC: fmt32,
                           
                           cpuD0: fmt32, cpuD1: fmt32, cpuD2: fmt32, cpuD3: fmt32,
                           cpuD4: fmt32, cpuD5: fmt32, cpuD6: fmt32, cpuD7: fmt32,
                           
                           cpuA0: fmt32, cpuA1: fmt32, cpuA2: fmt32, cpuA3: fmt32,
                           cpuA4: fmt32, cpuA5: fmt32, cpuA6: fmt32, cpuA7: fmt32,
                           
                           cpuSSP: fmt32 ]
            {
                assignFormatter(f, c!)
            }
            
            if amiga.isRunning() {
                cpuStopAndGoButton.image = NSImage.init(named: "pauseTemplate")
                cpuStepIntoButton.isEnabled = false
                cpuStepOverButton.isEnabled = false
                cpuTraceStopAndGoButton.image = NSImage.init(named: "pauseTemplate")
                cpuTraceStepIntoButton.isEnabled = false
                cpuTraceStepOverButton.isEnabled = false
            } else {
                cpuStopAndGoButton.image = NSImage.init(named: "continueTemplate")
                cpuStepIntoButton.isEnabled = true
                cpuStepOverButton.isEnabled = true
                cpuTraceStopAndGoButton.image = NSImage.init(named: "continueTemplate")
                cpuTraceStepIntoButton.isEnabled = true
                cpuTraceStepOverButton.isEnabled = true
            }
        }

        cpuPC.integerValue = Int(info.pc)

        cpuD0.integerValue = Int(info.d.0)
        cpuD1.integerValue = Int(info.d.1)
        cpuD2.integerValue = Int(info.d.2)
        cpuD3.integerValue = Int(info.d.3)
        cpuD4.integerValue = Int(info.d.4)
        cpuD5.integerValue = Int(info.d.5)
        cpuD6.integerValue = Int(info.d.6)
        cpuD7.integerValue = Int(info.d.7)

        cpuA0.integerValue = Int(info.a.0)
        cpuA1.integerValue = Int(info.a.1)
        cpuA2.integerValue = Int(info.a.2)
        cpuA3.integerValue = Int(info.a.3)
        cpuA4.integerValue = Int(info.a.4)
        cpuA5.integerValue = Int(info.a.5)
        cpuA6.integerValue = Int(info.a.6)
        cpuA7.integerValue = Int(info.a.7)
        
        cpuSSP.integerValue = Int(info.ssp)

        let flags = Int(info.flags)
        
        cpuT.state  = (flags & 0b1000000000000000 != 0) ? .on : .off
        cpuS.state  = (flags & 0b0010000000000000 != 0) ? .on : .off
        cpuI2.state = (flags & 0b0000010000000000 != 0) ? .on : .off
        cpuI1.state = (flags & 0b0000001000000000 != 0) ? .on : .off
        cpuI0.state = (flags & 0b0000000100000000 != 0) ? .on : .off
        cpuX.state  = (flags & 0b0000000000010000 != 0) ? .on : .off
        cpuN.state  = (flags & 0b0000000000001000 != 0) ? .on : .off
        cpuZ.state  = (flags & 0b0000000000000100 != 0) ? .on : .off
        cpuV.state  = (flags & 0b0000000000000010 != 0) ? .on : .off
        cpuC.state  = (flags & 0b0000000000000001 != 0) ? .on : .off

        instrTableView.refresh(everything: everything)
        traceTableView.refresh()
        breakTableView.refresh(everything: everything)
    }
    
    @IBAction func cpuStopAndGoAction(_ sender: NSButton!) {
        
        myController?.stopAndGoAction(sender)
    }
    
    @IBAction func cpuStepIntoAction(_ sender: NSButton!) {
        
         myController?.stepIntoAction(sender)
    }
    
    @IBAction func cpuStepOverAction(_ sender: NSButton!) {
        
         myController?.stepOverAction(sender)
    }

    @IBAction func cpuClearTraceBufferAction(_ sender: NSButton!) {
        
        amigaProxy?.cpu.clearTraceBuffer()
        traceTableView.refresh()
    }
    
    @IBAction func cpuGotoAction(_ sender: NSSearchField!) {
        
        let input = sender.stringValue
        
        if (input == "") {
            instrTableView.jumpToPC()
            return
        }
        
        if let addr = UInt32(input, radix: 16) {
            instrTableView.jumpTo(addr: addr)
            return
        }
        
        sender.stringValue = ""
    }
}
