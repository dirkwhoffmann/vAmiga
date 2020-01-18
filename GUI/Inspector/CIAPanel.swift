// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {
    
    private var selectedCia: Int { return ciaSelector.indexOfSelectedItem }

    func cacheCIA(count: Int = 0) {

        let ciaA = selectedCia == 0
        ciaInfo = ciaA ? amiga!.ciaA.getInfo() : amiga!.ciaB.getInfo()
    }

    func refreshCIAFormatters() {

        let ciaA = selectedCia == 0

        ciaPA7.title = "PA7: " + (ciaA ? "GAME1" : "/DTR")
        ciaPA6.title = "PA6: " + (ciaA ? "GAME0" : "/RTS")
        ciaPA5.title = "PA5: " + (ciaA ? "/RDY"  : "/CD")
        ciaPA4.title = "PA4: " + (ciaA ? "/TK0"  : "/CTS")
        ciaPA3.title = "PA3: " + (ciaA ? "/WPRO" : "/DSR")
        ciaPA2.title = "PA2: " + (ciaA ? "/CHNG" : "SEL")
        ciaPA1.title = "PA1: " + (ciaA ? "LED"   : "POUT")
        ciaPA0.title = "PA0: " + (ciaA ? "OVL"   : "BUSY")

        ciaPB7.title = "PB7: " + (ciaA ? "DATA7" : "/MTR")
        ciaPB6.title = "PB6: " + (ciaA ? "DATA6" : "/SEL3")
        ciaPB5.title = "PB5: " + (ciaA ? "DATA5" : "/SEL2")
        ciaPB4.title = "PB4: " + (ciaA ? "DATA4" : "/SEL1")
        ciaPB3.title = "PB3: " + (ciaA ? "DATA3" : "/SEL0")
        ciaPB2.title = "PB2: " + (ciaA ? "DATA2" : "/SIDE")
        ciaPB1.title = "PB1: " + (ciaA ? "DATA1" : "DIR")
        ciaPB0.title = "PB0: " + (ciaA ? "DATA0" : "/STEP")

        let elements = [ ciaPRA: fmt8,
                         ciaDDRA: fmt8,
                         ciaPRB: fmt8,
                         ciaDDRB: fmt8,
                         ciaCntHi: fmt8,
                         ciaCntMid: fmt8,
                         ciaCntLo: fmt8,
                         ciaAlarmHi: fmt8,
                         ciaAlarmMid: fmt8,
                         ciaAlarmLo: fmt8,
                         ciaIMR: fmt8,
                         ciaICR: fmt8,
                         ciaSDR: fmt8,
                         ciaTA: fmt16,
                         ciaTAlatch: fmt16,
                         ciaTB: fmt16,
                         ciaTBlatch: fmt16 ]

        for (c, f) in elements { assignFormatter(f, c!) }
    }

    func refreshCIA(count: Int) {

        // Perform a full refresh if needed
        if count == 0 { refreshCIAFormatters() }

        // Update display cache
        cacheCIA()

        // Refresh display with cached values
        ciaTA.intValue = Int32(ciaInfo!.timerA.count)
        ciaTAlatch.intValue = Int32(ciaInfo!.timerA.latch)
        ciaTArunning.state = ciaInfo!.timerA.running ? .on : .off
        ciaTAtoggle.state = ciaInfo!.timerA.toggle ? .on : .off
        ciaTApbout.state = ciaInfo!.timerA.pbout ? .on : .off
        ciaTAoneShot.state = ciaInfo!.timerA.oneShot ? .on : .off

        ciaTB.intValue = Int32(ciaInfo!.timerB.count)
        ciaTBlatch.intValue = Int32(ciaInfo!.timerB.latch)
        ciaTBrunning.state = ciaInfo!.timerB.running ? .on : .off
        ciaTBtoggle.state = ciaInfo!.timerB.toggle ? .on : .off
        ciaTBpbout.state = ciaInfo!.timerB.pbout ? .on : .off
        ciaTBoneShot.state = ciaInfo!.timerB.oneShot ? .on : .off

        ciaPRA.intValue = Int32(ciaInfo!.portA.reg)
        ciaPRAbinary.intValue = Int32(ciaInfo!.portA.reg)
        ciaDDRA.intValue = Int32(ciaInfo!.portA.dir)
        ciaDDRAbinary.intValue = Int32(ciaInfo!.portA.dir)

        var bits = ciaInfo!.portA.port
        ciaPA7.state = (bits & 0b10000000) != 0 ? .on : .off
        ciaPA6.state = (bits & 0b01000000) != 0 ? .on : .off
        ciaPA5.state = (bits & 0b00100000) != 0 ? .on : .off
        ciaPA4.state = (bits & 0b00010000) != 0 ? .on : .off
        ciaPA3.state = (bits & 0b00001000) != 0 ? .on : .off
        ciaPA2.state = (bits & 0b00000100) != 0 ? .on : .off
        ciaPA1.state = (bits & 0b00000010) != 0 ? .on : .off
        ciaPA0.state = (bits & 0b00000001) != 0 ? .on : .off

        ciaPRB.intValue = Int32(ciaInfo!.portB.reg)
        ciaPRBbinary.intValue = Int32(ciaInfo!.portB.reg)
        ciaPRB.intValue = Int32(ciaInfo!.portB.reg)
        ciaDDRB.intValue = Int32(ciaInfo!.portB.dir)

        bits = ciaInfo!.portB.port
        ciaPB7.state = (bits & 0b10000000) != 0 ? .on : .off
        ciaPB6.state = (bits & 0b01000000) != 0 ? .on : .off
        ciaPB5.state = (bits & 0b00100000) != 0 ? .on : .off
        ciaPB4.state = (bits & 0b00010000) != 0 ? .on : .off
        ciaPB3.state = (bits & 0b00001000) != 0 ? .on : .off
        ciaPB2.state = (bits & 0b00000100) != 0 ? .on : .off
        ciaPB1.state = (bits & 0b00000010) != 0 ? .on : .off
        ciaPB0.state = (bits & 0b00000001) != 0 ? .on : .off

        ciaICR.intValue = Int32(ciaInfo!.icr)
        ciaICRbinary.intValue = Int32(ciaInfo!.icr)
        ciaIMR.intValue = Int32(ciaInfo!.imr)
        ciaIMRbinary.intValue = Int32(ciaInfo!.imr)
        ciaIntLineLow.state = ciaInfo!.intLine ? .off : .on

        ciaCntHi.intValue = Int32(ciaInfo!.cnt.value.hi)
        ciaCntMid.intValue = Int32(ciaInfo!.cnt.value.mid)
        ciaCntLo.intValue = Int32(ciaInfo!.cnt.value.lo)
        ciaCntIntEnable.state = ciaInfo!.cntIntEnable ? .on : .off
        ciaAlarmHi.intValue = Int32(ciaInfo!.cnt.alarm.hi)
        ciaAlarmMid.intValue = Int32(ciaInfo!.cnt.alarm.mid)
        ciaAlarmLo.intValue = Int32(ciaInfo!.cnt.alarm.lo)

        ciaSDR.intValue = Int32(ciaInfo!.sdr)
        ciaSDRbinary.intValue = Int32(ciaInfo!.sdr)

        let idlePercentage = Int(ciaInfo!.idlePercentage * 100)
        ciaIdleCycles.stringValue = "\(ciaInfo!.idleCycles) cycles"
        ciaIdleLevel.integerValue = idlePercentage
        ciaIdleLevelText.stringValue = "\(idlePercentage) %"
    }

    @IBAction func selectCIAAction(_ sender: Any!) {

        needsRefresh()
    }
}
