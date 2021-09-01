// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {

    private func cacheCPU() {

        cpuInfo = amiga.cpu.info
    }

    func refreshCPU(count: Int = 0, full: Bool = false) {

        cacheCPU()

        if full {
            let elements = [ cpuPC: fmt32,

                             cpuD0: fmt32, cpuD1: fmt32, cpuD2: fmt32,
                             cpuD3: fmt32, cpuD4: fmt32, cpuD5: fmt32,
                             cpuD6: fmt32, cpuD7: fmt32,

                             cpuA0: fmt32, cpuA1: fmt32, cpuA2: fmt32,
                             cpuA3: fmt32, cpuA4: fmt32, cpuA5: fmt32,
                             cpuA6: fmt32, cpuA7: fmt32,

                             cpuUSP: fmt32, cpuSSP: fmt32 ]

            for (c, f) in elements { assignFormatter(f, c!) }
        }

        cpuPC.integerValue = Int(cpuInfo.pc0)

        cpuD0.integerValue = Int(cpuInfo.d.0)
        cpuD1.integerValue = Int(cpuInfo.d.1)
        cpuD2.integerValue = Int(cpuInfo.d.2)
        cpuD3.integerValue = Int(cpuInfo.d.3)
        cpuD4.integerValue = Int(cpuInfo.d.4)
        cpuD5.integerValue = Int(cpuInfo.d.5)
        cpuD6.integerValue = Int(cpuInfo.d.6)
        cpuD7.integerValue = Int(cpuInfo.d.7)

        cpuA0.integerValue = Int(cpuInfo.a.0)
        cpuA1.integerValue = Int(cpuInfo.a.1)
        cpuA2.integerValue = Int(cpuInfo.a.2)
        cpuA3.integerValue = Int(cpuInfo.a.3)
        cpuA4.integerValue = Int(cpuInfo.a.4)
        cpuA5.integerValue = Int(cpuInfo.a.5)
        cpuA6.integerValue = Int(cpuInfo.a.6)
        cpuA7.integerValue = Int(cpuInfo.a.7)

        cpuUSP.integerValue = Int(cpuInfo.usp)
        cpuSSP.integerValue = Int(cpuInfo.ssp)

        let sr = Int(cpuInfo.sr)

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

        cpuInstrView.refresh(count: count, full: full, addr: Int(cpuInfo.pc0))
        cpuTraceView.refresh(count: count, full: full)
        cpuBreakView.refresh(count: count, full: full)
        cpuWatchView.refresh(count: count, full: full)
    }

    @IBAction func cpuClearTraceBufferAction(_ sender: NSButton!) {

        amiga.cpu.clearLog()
        refreshCPU(full: true)
    }
    
    @IBAction func cpuGotoAction(_ sender: NSSearchField!) {

        if sender.stringValue == "" {
            cpuInstrView.jumpTo(addr: Int(cpuInfo.pc0))
        } else if let addr = Int(sender.stringValue, radix: 16) {
            cpuInstrView.jumpTo(addr: addr)
        } else {
            sender.stringValue = ""
        }
    }
}
