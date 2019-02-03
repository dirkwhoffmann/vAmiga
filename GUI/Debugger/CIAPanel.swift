// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

extension Inspector {
    
    private var selectedCia: Int {
        get { return ciaSelector.indexOfSelectedItem }
    }
    
    func refreshCIA() {
        
        guard let amiga = amigaProxy else { return }
        let info = selectedCia == 0 ? amiga.ciaA.getInfo() : amiga.ciaB.getInfo()
        
        ciaTA.intValue = Int32(info.timerA.count)
        ciaTAlatch.intValue = Int32(info.timerA.latch)
        ciaTArunning.state = info.timerA.running ? .on : .off
        ciaTAtoggle.state = info.timerA.toggle ? .on : .off
        ciaTApbout.state = info.timerA.pbout ? .on : .off
        ciaTAoneShot.state = info.timerA.oneShot ? .on : .off

        ciaTB.intValue = Int32(info.timerB.count)
        ciaTBlatch.intValue = Int32(info.timerB.latch)
        ciaTBrunning.state = info.timerB.running ? .on : .off
        ciaTBtoggle.state = info.timerB.toggle ? .on : .off
        ciaTBpbout.state = info.timerB.pbout ? .on : .off
        ciaTBoneShot.state = info.timerB.oneShot ? .on : .off
        
        ciaPRA.intValue = Int32(info.portA.reg)
        ciaPRAbinary.intValue = Int32(info.portA.reg)
        ciaDDRA.intValue = Int32(info.portA.dir)
        ciaDDRAbinary.intValue = Int32(info.portA.dir)

        var bits = info.portA.port
        ciaPA7.state = (bits & 0b10000000) != 0 ? .on : .off
        ciaPA6.state = (bits & 0b01000000) != 0 ? .on : .off
        ciaPA5.state = (bits & 0b00100000) != 0 ? .on : .off
        ciaPA4.state = (bits & 0b00010000) != 0 ? .on : .off
        ciaPA3.state = (bits & 0b00001000) != 0 ? .on : .off
        ciaPA2.state = (bits & 0b00000100) != 0 ? .on : .off
        ciaPA1.state = (bits & 0b00000010) != 0 ? .on : .off
        ciaPA0.state = (bits & 0b00000001) != 0 ? .on : .off

        ciaPRB.intValue = Int32(info.portB.reg)
        ciaPRBbinary.intValue = Int32(info.portB.reg)
        ciaPRB.intValue = Int32(info.portB.reg)
        ciaDDRB.intValue = Int32(info.portB.dir)
        
        bits = info.portB.port
        ciaPB7.state = (bits & 0b10000000) != 0 ? .on : .off
        ciaPB6.state = (bits & 0b01000000) != 0 ? .on : .off
        ciaPB5.state = (bits & 0b00100000) != 0 ? .on : .off
        ciaPB4.state = (bits & 0b00010000) != 0 ? .on : .off
        ciaPB3.state = (bits & 0b00001000) != 0 ? .on : .off
        ciaPB2.state = (bits & 0b00000100) != 0 ? .on : .off
        ciaPB1.state = (bits & 0b00000010) != 0 ? .on : .off
        ciaPB0.state = (bits & 0b00000001) != 0 ? .on : .off
   
        ciaICR.intValue = Int32(info.icr)
        ciaICRbinary.intValue = Int32(info.icr)
        ciaIMR.intValue = Int32(info.imr)
        ciaIMRbinary.intValue = Int32(info.imr)
        ciaIntLineLow.state = info.intLine ? .off : .on
        
        ciaCntHi.intValue = Int32(info.cnt.value.hi)
        ciaCntMid.intValue = Int32(info.cnt.value.mid)
        ciaCntLo.intValue = Int32(info.cnt.value.lo)
        ciaCntIntEnable.state = info.cntIntEnable ? .on : .off
        ciaAlarmHi.intValue = Int32(info.cnt.alarm.hi)
        ciaAlarmMid.intValue = Int32(info.cnt.alarm.mid)
        ciaAlarmLo.intValue = Int32(info.cnt.alarm.lo)
        
        ciaSDR.intValue = Int32(info.sdr)
        ciaSDRbinary.intValue = Int32(info.sdr)
        
        let idlePercentage = Int(info.idlePercentage * 100)
        ciaIdleCycles.integerValue = Int(info.idleCycles)
        ciaIdleLevel.integerValue = idlePercentage
        ciaIdleLevelText.stringValue = "\(idlePercentage) %"

   
    }
}

