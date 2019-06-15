// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {
    
    func refreshPaula(everything: Bool) {
        
        guard let paula = amigaProxy?.paula else { return }
        let info = paula.getInfo()
        let audInfo = paula.getAudioInfo()
        let dskInfo = paula.getDiskControllerInfo()
        
        // track("Refreshing Paula inspector tab")
        
        if everything {

            let elements = [ paulaIntena: fmt16,
                             paulaIntreq: fmt16,
                             dskDsklen: fmt16,
                             dskDskbytr: fmt16,
                             dskAdkconHi: fmt8,
                             dskDsksync: fmt16,

                             audioLen0: fmt16,
                             audioPer0: fmt16,
                             audioVol0: fmt16,
                             audioDat0: fmt16,
                             audioLoc0: fmt16,
                             audioLen1: fmt16,
                             audioPer1: fmt16,
                             audioVol1: fmt16,
                             audioDat1: fmt16,
                             audioLoc1: fmt16,
                             audioLen2: fmt16,
                             audioPer2: fmt16,
                             audioVol2: fmt16,
                             audioDat2: fmt16,
                             audioLoc2: fmt16,
                             audioLen3: fmt16,
                             audioPer3: fmt16,
                             audioVol3: fmt16,
                             audioDat3: fmt16,
                             audioLoc3: fmt16
                             ]

            for (c, f) in elements { assignFormatter(f, c!) }
        }
        
        // Interrupt controller
        paulaIntena.intValue = Int32(info.intena)
        paulaEna14.state = (info.intena & 0b0100000000000000 != 0) ? .on : .off
        paulaEna13.state = (info.intena & 0b0010000000000000 != 0) ? .on : .off
        paulaEna12.state = (info.intena & 0b0001000000000000 != 0) ? .on : .off
        paulaEna11.state = (info.intena & 0b0000100000000000 != 0) ? .on : .off
        paulaEna10.state = (info.intena & 0b0000010000000000 != 0) ? .on : .off
        paulaEna9.state  = (info.intena & 0b0000001000000000 != 0) ? .on : .off
        paulaEna8.state  = (info.intena & 0b0000000100000000 != 0) ? .on : .off
        paulaEna7.state  = (info.intena & 0b0000000010000000 != 0) ? .on : .off
        paulaEna6.state  = (info.intena & 0b0000000001000000 != 0) ? .on : .off
        paulaEna5.state  = (info.intena & 0b0000000000100000 != 0) ? .on : .off
        paulaEna4.state  = (info.intena & 0b0000000000010000 != 0) ? .on : .off
        paulaEna3.state  = (info.intena & 0b0000000000001000 != 0) ? .on : .off
        paulaEna2.state  = (info.intena & 0b0000000000000100 != 0) ? .on : .off
        paulaEna1.state  = (info.intena & 0b0000000000000010 != 0) ? .on : .off
        paulaEna0.state  = (info.intena & 0b0000000000000001 != 0) ? .on : .off
        
        paulaIntreq.intValue = Int32(info.intreq)
        paulaReq14.state = (info.intreq & 0b0100000000000000 != 0) ? .on : .off
        paulaReq13.state = (info.intreq & 0b0010000000000000 != 0) ? .on : .off
        paulaReq12.state = (info.intreq & 0b0001000000000000 != 0) ? .on : .off
        paulaReq11.state = (info.intreq & 0b0000100000000000 != 0) ? .on : .off
        paulaReq10.state = (info.intreq & 0b0000010000000000 != 0) ? .on : .off
        paulaReq9.state  = (info.intreq & 0b0000001000000000 != 0) ? .on : .off
        paulaReq8.state  = (info.intreq & 0b0000000100000000 != 0) ? .on : .off
        paulaReq7.state  = (info.intreq & 0b0000000010000000 != 0) ? .on : .off
        paulaReq6.state  = (info.intreq & 0b0000000001000000 != 0) ? .on : .off
        paulaReq5.state  = (info.intreq & 0b0000000000100000 != 0) ? .on : .off
        paulaReq4.state  = (info.intreq & 0b0000000000010000 != 0) ? .on : .off
        paulaReq3.state  = (info.intreq & 0b0000000000001000 != 0) ? .on : .off
        paulaReq2.state  = (info.intreq & 0b0000000000000100 != 0) ? .on : .off
        paulaReq1.state  = (info.intreq & 0b0000000000000010 != 0) ? .on : .off
        paulaReq0.state  = (info.intreq & 0b0000000000000001 != 0) ? .on : .off
        
        // Disk controller
        switch dskInfo.state {
        case DRIVE_DMA_OFF:
            dskStateText.stringValue = "Idle"
        case DRIVE_DMA_WAIT:
            dskStateText.stringValue = "Waiting for sync signal"
        case DRIVE_DMA_READ:
            dskStateText.stringValue = "Reading"
        case DRIVE_DMA_WRITE:
            dskStateText.stringValue = "Writing"
        default:
            dskStateText.stringValue = "UNKNOWN"
        }
    
        dskSelectDf0.state = (dskInfo.selectedDrive == 0) ? .on : .off
        dskSelectDf1.state = (dskInfo.selectedDrive == 1) ? .on : .off
        dskSelectDf2.state = (dskInfo.selectedDrive == 2) ? .on : .off
        dskSelectDf3.state = (dskInfo.selectedDrive == 3) ? .on : .off
        
        dskDsklen.integerValue = Int(dskInfo.dsklen)
        dskDmaen.state = (dskInfo.dsklen & 0x8000 != 0) ? .on : .off
        dskWrite.state = (dskInfo.dsklen & 0x4000 != 0) ? .on : .off

        dskDskbytr.integerValue = Int(dskInfo.dskbytr)
        dskByteready.state = (dskInfo.dskbytr & 0x8000 != 0) ? .on : .off
        dskDmaon.state     = (dskInfo.dskbytr & 0x4000 != 0) ? .on : .off
        dskDiskwrite.state = (dskInfo.dskbytr & 0x2000 != 0) ? .on : .off
        dskWordequal.state = (dskInfo.dskbytr & 0x1000 != 0) ? .on : .off

        dskAdkconHi.integerValue = Int(info.adkcon >> 8)
        dskPrecomp1.state = (info.adkcon & 0x4000 != 0) ? .on : .off
        dskPrecomp0.state = (info.adkcon & 0x2000 != 0) ? .on : .off
        dskMfmprec.state  = (info.adkcon & 0x1000 != 0) ? .on : .off
        dskUartbrk.state  = (info.adkcon & 0x0800 != 0) ? .on : .off
        dskWordsync.state = (info.adkcon & 0x0400 != 0) ? .on : .off
        dskMsbsync.state  = (info.adkcon & 0x0200 != 0) ? .on : .off
        dskFast.state     = (info.adkcon & 0x0100 != 0) ? .on : .off

        dskDsksync.integerValue = Int(dskInfo.dsksync)
       
        switch dskInfo.fifoCount {
        case 0: dskFifo0.stringValue = ""; fallthrough
        case 1: dskFifo1.stringValue = ""; fallthrough
        case 2: dskFifo2.stringValue = ""; fallthrough
        case 3: dskFifo3.stringValue = ""; fallthrough
        case 4: dskFifo4.stringValue = ""; fallthrough
        case 5: dskFifo5.stringValue = ""; fallthrough
        default: break
        }

        switch dskInfo.fifoCount {
        case 6: dskFifo5.stringValue = String(format: "%02X", dskInfo.fifo.5); fallthrough
        case 5: dskFifo4.stringValue = String(format: "%02X", dskInfo.fifo.4); fallthrough
        case 4: dskFifo3.stringValue = String(format: "%02X", dskInfo.fifo.3); fallthrough
        case 3: dskFifo2.stringValue = String(format: "%02X", dskInfo.fifo.2); fallthrough
        case 2: dskFifo1.stringValue = String(format: "%02X", dskInfo.fifo.1); fallthrough
        case 1: dskFifo0.stringValue = String(format: "%02X", dskInfo.fifo.0); fallthrough
        default: break
        }
        
        /*
        switch (dskInfo.fifoCount) {
        case 6: dskFifo5.intValue = dskInfo.fifo.5; fallthrough
        case 5: dskFifo4.intValue = dskInfo.fifo.4; fallthrough
        case 4: dskFifo3.intValue = dskInfo.fifo.3; fallthrough
        case 3: dskFifo2.intValue = dskInfo.fifo.2; fallthrough
        case 2: dskFifo1.intValue = dskInfo.fifo.1; fallthrough
        case 1: dskFifo0.intValue = dskInfo.fifo.0; fallthrough
        default: break
        }
        */

        // Audio section
        audioLen0.intValue = Int32(audInfo.channel.0.audlenLatch)
        audioPer0.intValue = Int32(audInfo.channel.0.audperLatch)
        audioVol0.intValue = Int32(audInfo.channel.0.audvolLatch)
        audioDat0.intValue = Int32(audInfo.channel.0.auddatLatch)
        audioLoc0.intValue = Int32(audInfo.channel.0.audlcLatch)

        audioLen1.intValue = Int32(audInfo.channel.1.audlenLatch)
        audioPer1.intValue = Int32(audInfo.channel.1.audperLatch)
        audioVol1.intValue = Int32(audInfo.channel.1.audvolLatch)
        audioDat1.intValue = Int32(audInfo.channel.1.auddatLatch)
        audioLoc1.intValue = Int32(audInfo.channel.1.audlcLatch)

        audioLen2.intValue = Int32(audInfo.channel.2.audlenLatch)
        audioPer2.intValue = Int32(audInfo.channel.2.audperLatch)
        audioVol2.intValue = Int32(audInfo.channel.2.audvolLatch)
        audioDat2.intValue = Int32(audInfo.channel.2.auddatLatch)
        audioLoc2.intValue = Int32(audInfo.channel.2.audlcLatch)

        audioLen3.intValue = Int32(audInfo.channel.3.audlenLatch)
        audioPer3.intValue = Int32(audInfo.channel.3.audperLatch)
        audioVol3.intValue = Int32(audInfo.channel.3.audvolLatch)
        audioDat3.intValue = Int32(audInfo.channel.3.auddatLatch)
        audioLoc3.intValue = Int32(audInfo.channel.3.audlcLatch)

        // Audio sample sequencer
        let fillLevel = Int32(paula.fillLevel() * 100)
        audioBufferLevel.intValue = fillLevel
        audioBufferLevelText.stringValue = "\(fillLevel) %"
        audioBufferUnderflows.intValue = Int32(paula.bufferUnderflows())
        audioBufferOverflows.intValue = Int32(paula.bufferOverflows())
        audioWaveformView.update()
    }

    @IBAction func paulaINTENAAction(_ sender: NSButton!) {

        let value = sender.state == .on
        amigaProxy?.paula.setINTENA(sender.tag, value: value)
        refreshPaula(everything: false)
    }

    @IBAction func paulaINTREQAction(_ sender: NSButton!) {

        let value = sender.state == .on
        amigaProxy?.paula.setINTREQ(sender.tag, value: value)
        refreshPaula(everything: false)
    }
}
