// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {

    func cacheCPU(count: Int = 0) {

        cpuInfo = amiga!.cpu.getInfo()
    }

    func refreshCPUValues() {

        cacheCPU()

        cpuPC.integerValue = Int(cpuInfo!.pc)

        cpuD0.integerValue = Int(cpuInfo!.d.0)
        cpuD1.integerValue = Int(cpuInfo!.d.1)
        cpuD2.integerValue = Int(cpuInfo!.d.2)
        cpuD3.integerValue = Int(cpuInfo!.d.3)
        cpuD4.integerValue = Int(cpuInfo!.d.4)
        cpuD5.integerValue = Int(cpuInfo!.d.5)
        cpuD6.integerValue = Int(cpuInfo!.d.6)
        cpuD7.integerValue = Int(cpuInfo!.d.7)

        cpuA0.integerValue = Int(cpuInfo!.a.0)
        cpuA1.integerValue = Int(cpuInfo!.a.1)
        cpuA2.integerValue = Int(cpuInfo!.a.2)
        cpuA3.integerValue = Int(cpuInfo!.a.3)
        cpuA4.integerValue = Int(cpuInfo!.a.4)
        cpuA5.integerValue = Int(cpuInfo!.a.5)
        cpuA6.integerValue = Int(cpuInfo!.a.6)
        cpuA7.integerValue = Int(cpuInfo!.a.7)

        cpuUSP.integerValue = Int(cpuInfo!.usp)
        cpuSSP.integerValue = Int(cpuInfo!.ssp)

        let sr = Int(cpuInfo!.sr)

        cpuT.state  = (sr & 0b1000000000000000 != 0) ? .on : .off
        cpuS.state  = (sr & 0b0010000000000000 != 0) ? .on : .off
        cpuI2.state = (sr & 0b0000010000000000 != 0) ? .on : .off
        cpuI1.state = (sr & 0b0000001000000000 != 0) ? .on : .off
        cpuI0.state = (sr & 0b0000000100000000 != 0) ? .on : .off
        cpuX.state  = (sr & 0b0000000000010000 != 0) ? .on : .off
        cpuN.state  = (sr & 0b0000000000001000 != 0) ? .on : .off
        cpuZ.state  = (sr & 0b0000000000000100 != 0) ? .on : .off
        cpuV.state  = (sr & 0b0000000000000010 != 0) ? .on : .off
        cpuC.state  = (sr & 0b0000000000000001 != 0) ? .on : .off
    }

    func refreshCPUFormatters() {

        let elements = [ cpuPC: fmt32,

                         cpuD0: fmt32, cpuD1: fmt32, cpuD2: fmt32,
                         cpuD3: fmt32, cpuD4: fmt32, cpuD5: fmt32,
                         cpuD6: fmt32, cpuD7: fmt32,

                         cpuA0: fmt32, cpuA1: fmt32, cpuA2: fmt32,
                         cpuA3: fmt32, cpuA4: fmt32, cpuA5: fmt32,
                         cpuA6: fmt32, cpuA7: fmt32,

                         cpuUSP: fmt32, cpuSSP: fmt32 ]

        for (c, f) in elements { assignFormatter(f, c!) }

        if parent!.amiga.isRunning() {
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

    func fullRefreshCPU() {

        instrTableView.fullRefresh()
        traceTableView.fullRefresh()
        breakTableView.fullRefresh()
        watchTableView.fullRefresh()

        refreshCPUFormatters()
        refreshCPUValues()
    }

    func periodicRefreshCPU(count: Int) {

        instrTableView.periodicRefresh(count: count)
        traceTableView.periodicRefresh(count: count)
        breakTableView.periodicRefresh(count: count)
        watchTableView.periodicRefresh(count: count)

        refreshCPUValues()
    }

    @IBAction func cpuStopAndGoAction(_ sender: NSButton!) {

        lockAmiga()

        amiga?.stopAndGo()
        fullRefresh()

        unlockAmiga()
    }
    
    @IBAction func cpuStepIntoAction(_ sender: NSButton!) {

        lockAmiga()

        amiga?.stepInto()
        fullRefresh()

        unlockAmiga()
    }
    
    @IBAction func cpuStepOverAction(_ sender: NSButton!) {

        lockAmiga()

        amiga?.stepOver()
        fullRefresh()

        unlockAmiga()
    }

    @IBAction func cpuClearTraceBufferAction(_ sender: NSButton!) {

        lockAmiga()

        amiga?.cpu.clearLog()
        fullRefresh()

        unlockAmiga()
    }
    
    @IBAction func cpuGotoAction(_ sender: NSSearchField!) {
        
        lockAmiga()

        if sender.stringValue == "" {
            instrTableView.jumpToPC()
        } else if let addr = UInt32(sender.stringValue, radix: 16) {
            instrTableView.jumpTo(addr: addr)
        } else {
            sender.stringValue = ""
        }
        fullRefresh()

        unlockAmiga()
    }
}
