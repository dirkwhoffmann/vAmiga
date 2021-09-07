// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

let image0 = NSImage(named: "state0Template")!
let image1 = NSImage(named: "state1Template")!
let image2 = NSImage(named: "state2Template")!
let image3 = NSImage(named: "state3Template")!
let image5 = NSImage(named: "state5Template")!

extension Inspector {

    private func cachePaula() {

        paulaInfo = amiga.paula.info
        audioInfo0 = amiga.paula.audioInfo0
        audioInfo1 = amiga.paula.audioInfo1
        audioInfo2 = amiga.paula.audioInfo2
        audioInfo3 = amiga.paula.audioInfo3
        diskInfo = amiga.diskController.info
    }

    func refreshPaula(count: Int = 0, full: Bool = false) {

        cachePaula()

        if full {
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
                             audioLen1: fmt16,
                             audioPer1: fmt16,
                             audioVol1: fmt16,
                             audioDat1: fmt16,
                             audioLen2: fmt16,
                             audioPer2: fmt16,
                             audioVol2: fmt16,
                             audioDat2: fmt16,
                             audioLen3: fmt16,
                             audioPer3: fmt16,
                             audioVol3: fmt16,
                             audioDat3: fmt16
            ]

            for (c, f) in elements { assignFormatter(f, c!) }
        }

        // Interrupt controller
        let intena = Int(paulaInfo.intena)
        let intreq = Int(paulaInfo.intreq)

        paulaIntena.integerValue = intena
        paulaEna14.state = (intena & 0b0100000000000000 != 0) ? .on : .off
        paulaEna13.state = (intena & 0b0010000000000000 != 0) ? .on : .off
        paulaEna12.state = (intena & 0b0001000000000000 != 0) ? .on : .off
        paulaEna11.state = (intena & 0b0000100000000000 != 0) ? .on : .off
        paulaEna10.state = (intena & 0b0000010000000000 != 0) ? .on : .off
        paulaEna9.state  = (intena & 0b0000001000000000 != 0) ? .on : .off
        paulaEna8.state  = (intena & 0b0000000100000000 != 0) ? .on : .off
        paulaEna7.state  = (intena & 0b0000000010000000 != 0) ? .on : .off
        paulaEna6.state  = (intena & 0b0000000001000000 != 0) ? .on : .off
        paulaEna5.state  = (intena & 0b0000000000100000 != 0) ? .on : .off
        paulaEna4.state  = (intena & 0b0000000000010000 != 0) ? .on : .off
        paulaEna3.state  = (intena & 0b0000000000001000 != 0) ? .on : .off
        paulaEna2.state  = (intena & 0b0000000000000100 != 0) ? .on : .off
        paulaEna1.state  = (intena & 0b0000000000000010 != 0) ? .on : .off
        paulaEna0.state  = (intena & 0b0000000000000001 != 0) ? .on : .off

        paulaIntreq.integerValue = intreq
        paulaReq14.state = (intreq & 0b0100000000000000 != 0) ? .on : .off
        paulaReq13.state = (intreq & 0b0010000000000000 != 0) ? .on : .off
        paulaReq12.state = (intreq & 0b0001000000000000 != 0) ? .on : .off
        paulaReq11.state = (intreq & 0b0000100000000000 != 0) ? .on : .off
        paulaReq10.state = (intreq & 0b0000010000000000 != 0) ? .on : .off
        paulaReq9.state  = (intreq & 0b0000001000000000 != 0) ? .on : .off
        paulaReq8.state  = (intreq & 0b0000000100000000 != 0) ? .on : .off
        paulaReq7.state  = (intreq & 0b0000000010000000 != 0) ? .on : .off
        paulaReq6.state  = (intreq & 0b0000000001000000 != 0) ? .on : .off
        paulaReq5.state  = (intreq & 0b0000000000100000 != 0) ? .on : .off
        paulaReq4.state  = (intreq & 0b0000000000010000 != 0) ? .on : .off
        paulaReq3.state  = (intreq & 0b0000000000001000 != 0) ? .on : .off
        paulaReq2.state  = (intreq & 0b0000000000000100 != 0) ? .on : .off
        paulaReq1.state  = (intreq & 0b0000000000000010 != 0) ? .on : .off
        paulaReq0.state  = (intreq & 0b0000000000000001 != 0) ? .on : .off

        // Disk controller
        switch diskInfo.state {
        case .OFF:
            dskStateText.stringValue = "Idle"
        case .WAIT:
            dskStateText.stringValue = "Waiting for sync signal"
        case .READ:
            dskStateText.stringValue = "Reading"
        case .WRITE:
            dskStateText.stringValue = "Writing"
        default:
            dskStateText.stringValue = "UNKNOWN"
        }

        let selectedDrive = diskInfo.selectedDrive
        let dsklen = Int(diskInfo.dsklen)
        let dskbytr = Int(diskInfo.dskbytr)
        let adkcon = Int(paulaInfo.adkcon)

        dskSelectDf0.state = (selectedDrive == 0) ? .on : .off
        dskSelectDf1.state = (selectedDrive == 1) ? .on : .off
        dskSelectDf2.state = (selectedDrive == 2) ? .on : .off
        dskSelectDf3.state = (selectedDrive == 3) ? .on : .off

        dskDsklen.integerValue = dsklen
        dskDmaen.state = (dsklen & 0x8000 != 0) ? .on : .off
        dskWrite.state = (dsklen & 0x4000 != 0) ? .on : .off

        dskDskbytr.integerValue = dskbytr
        dskByteready.state = (dskbytr & 0x8000 != 0) ? .on : .off
        dskDmaon.state     = (dskbytr & 0x4000 != 0) ? .on : .off
        dskDiskwrite.state = (dskbytr & 0x2000 != 0) ? .on : .off
        dskWordequal.state = (dskbytr & 0x1000 != 0) ? .on : .off

        dskAdkconHi.integerValue = adkcon >> 8
        dskPrecomp1.state = (adkcon & 0x4000 != 0) ? .on : .off
        dskPrecomp0.state = (adkcon & 0x2000 != 0) ? .on : .off
        dskMfmprec.state  = (adkcon & 0x1000 != 0) ? .on : .off
        dskUartbrk.state  = (adkcon & 0x0800 != 0) ? .on : .off
        dskWordsync.state = (adkcon & 0x0400 != 0) ? .on : .off
        dskMsbsync.state  = (adkcon & 0x0200 != 0) ? .on : .off
        dskFast.state     = (adkcon & 0x0100 != 0) ? .on : .off

        let dsksync = Int(diskInfo.dsksync)
        dskDsksync.integerValue = dsksync
        dskDsksync.textColor = (dsksync == 0x4489) ? .textColor : .warningColor
        
        switch diskInfo.fifoCount {
        case 0: dskFifo0.stringValue = ""; fallthrough
        case 1: dskFifo1.stringValue = ""; fallthrough
        case 2: dskFifo2.stringValue = ""; fallthrough
        case 3: dskFifo3.stringValue = ""; fallthrough
        case 4: dskFifo4.stringValue = ""; fallthrough
        case 5: dskFifo5.stringValue = ""; fallthrough
        default: break
        }

        switch diskInfo.fifoCount {
        case 6: dskFifo5.stringValue = String(format: "%02X", diskInfo.fifo.5); fallthrough
        case 5: dskFifo4.stringValue = String(format: "%02X", diskInfo.fifo.4); fallthrough
        case 4: dskFifo3.stringValue = String(format: "%02X", diskInfo.fifo.3); fallthrough
        case 3: dskFifo2.stringValue = String(format: "%02X", diskInfo.fifo.2); fallthrough
        case 2: dskFifo1.stringValue = String(format: "%02X", diskInfo.fifo.1); fallthrough
        case 1: dskFifo0.stringValue = String(format: "%02X", diskInfo.fifo.0); fallthrough
        default: break
        }

        // Audio
        func assignImage(_ button: NSButton,
                         info: StateMachineInfo, displayState: inout Int) {

            // Determine the state to display
            switch displayState {
            case 0:
                if info.state != 0 { displayState = info.dma ? 1 : 2 }
            case 1:
                displayState = 5
            case 5:
                displayState = 2
            case 2:
                displayState = 3
            case 3:
                displayState = (info.state == 2 || info.state == 3) ? 2 : 0
            default:
                fatalError()
            }

            // Assign the correct image
            switch displayState {
            case 0:  button.image = image0
            case 1:  button.image = image1
            case 2:  button.image = image2
            case 3:  button.image = image3
            case 5:  button.image = image5
            default: button.image = nil
            }
        }

        audioLen0.intValue = Int32(audioInfo0.audlenLatch)
        audioPer0.intValue = Int32(audioInfo0.audperLatch)
        audioVol0.intValue = Int32(audioInfo0.audvolLatch)
        audioDat0.intValue = Int32(audioInfo0.auddat)

        audioLen1.intValue = Int32(audioInfo1.audlenLatch)
        audioPer1.intValue = Int32(audioInfo1.audperLatch)
        audioVol1.intValue = Int32(audioInfo1.audvolLatch)
        audioDat1.intValue = Int32(audioInfo1.auddat)

        audioLen2.intValue = Int32(audioInfo2.audlenLatch)
        audioPer2.intValue = Int32(audioInfo2.audperLatch)
        audioVol2.intValue = Int32(audioInfo2.audvolLatch)
        audioDat2.intValue = Int32(audioInfo2.auddat)
        
        audioLen3.intValue = Int32(audioInfo3.audlenLatch)
        audioPer3.intValue = Int32(audioInfo3.audperLatch)
        audioVol3.intValue = Int32(audioInfo3.audvolLatch)
        audioDat3.intValue = Int32(audioInfo3.auddat)

        assignImage(audioImg0, info: audioInfo0, displayState: &displayState0)
        assignImage(audioImg1, info: audioInfo1, displayState: &displayState1)
        assignImage(audioImg2, info: audioInfo2, displayState: &displayState2)
        assignImage(audioImg3, info: audioInfo3, displayState: &displayState3)
    }
}
