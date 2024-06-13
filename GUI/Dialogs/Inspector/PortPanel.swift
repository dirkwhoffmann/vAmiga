// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

let serDatFont = NSFontManager.shared.font(withFamily: "Courier New",
                                           traits: .boldFontMask,
                                           weight: 0, size: 10)

extension Inspector {

    private func cachePorts() {

        if amiga != nil {

            if amiga.paused {

                port1Info = amiga.controlPort1.info
                port2Info = amiga.controlPort2.info
                serInfo   = amiga.serialPort.info
                uartInfo  = amiga.paula.uartInfo

            } else {

                port1Info = amiga.controlPort1.cachedInfo
                port2Info = amiga.controlPort2.cachedInfo
                serInfo   = amiga.serialPort.cachedInfo
                uartInfo  = amiga.paula.cachedUartInfo
            }
        }
    }

    func refreshPorts(count: Int = 0, full: Bool = false) {

        cachePorts()
        
        if full {
            let elements = [  poPOTGO: fmt16,
                             poPOTGOR: fmt16,
                            po0JOYDAT: fmt16,
                            po0POTDAT: fmt16,
                            po1JOYDAT: fmt16,
                            po1POTDAT: fmt16,
                             poSERPER: fmt16,
                           poRecShift: fmt16,
                          poRecBuffer: fmt16,
                         poTransShift: fmt16,
                        poTransBuffer: fmt16
            ]

            for (c, f) in elements { assignFormatter(f, c!) }
        }

        // Control port commons
        let potgo = Int(port1Info.potgo)
        let potgor = Int(port1Info.potgor)

        poPOTGO.integerValue = potgo
        poOUTRY.state = ((potgo & 0x8000) != 0) ? .on : .off
        poDATRY.state = ((potgo & 0x4000) != 0) ? .on : .off
        poOUTRX.state = ((potgo & 0x2000) != 0) ? .on : .off
        poDATRX.state = ((potgo & 0x1000) != 0) ? .on : .off
        poOUTLY.state = ((potgo & 0x0800) != 0) ? .on : .off
        poDATLY.state = ((potgo & 0x0400) != 0) ? .on : .off
        poOUTLX.state = ((potgo & 0x0200) != 0) ? .on : .off
        poDATLX.state = ((potgo & 0x0100) != 0) ? .on : .off

        poPOTGOR.integerValue = potgor
        poDATRYR.state = ((potgor & 0x4000) != 0) ? .on : .off
        poDATRXR.state = ((potgor & 0x1000) != 0) ? .on : .off
        poDATLYR.state = ((potgor & 0x0400) != 0) ? .on : .off
        poDATLXR.state = ((potgor & 0x0100) != 0) ? .on : .off

        // Control port 1
        po0JOYDAT.integerValue = Int(port1Info.joydat)
        po0M0V.state = port1Info.m0v ? .on : .off
        po0M0H.state = port1Info.m0h ? .on : .off
        po0M1V.state = port1Info.m1v ? .on : .off
        po0M1H.state = port1Info.m1h ? .on : .off
        po0POTDAT.integerValue = Int(port1Info.potdat)

        // Control port 2
        po1JOYDAT.integerValue = Int(port2Info.joydat)
        po1M0V.state = port2Info.m0v ? .on : .off
        po1M0H.state = port2Info.m0h ? .on : .off
        po1M1V.state = port2Info.m1v ? .on : .off
        po1M1H.state = port2Info.m1h ? .on : .off
        po1POTDAT.integerValue = Int(port2Info.potdat)

        // Serial port
        poSERPER.integerValue = Int(uartInfo.serper)
        poBaud.stringValue = "\(uartInfo.baudRate) Baud"
        poLONG.state = (uartInfo.serper & 0x8000) != 0 ? .on : .off
        poTXD.state = serInfo.txd ? .on : .off
        poRXD.state = serInfo.rxd ? .on : .off
        poCTS.state = serInfo.cts ? .on : .off
        poDSR.state = serInfo.dsr ? .on : .off
        poCD.state = serInfo.cd ? .on : .off
        poDTR.state = serInfo.dtr ? .on : .off
        poRecShift.integerValue = Int(uartInfo.receiveShiftReg)
        poRecBuffer.integerValue = Int(uartInfo.receiveBuffer)
        poTransShift.integerValue = Int(uartInfo.transmitShiftReg)
        poTransBuffer.integerValue = Int(uartInfo.transmitBuffer)
        
        // Logging windows
        if count % 2 == 0 {

            let serialInSize = parent!.serialIn.count

            // Truncate logging info if it has become too large
            if serialInSize > 8192 {
                parent!.serialIn = "...\n" + parent!.serialIn.dropFirst( serialInSize - 8000)
            }

            // Update text view if necessary
            if poSerialIn.string.count != serialInSize {
                poSerialIn.string = parent!.serialIn
                poSerialIn.scrollToEndOfDocument(nil)
                poSerialIn.font = serDatFont
            }

            let serialOutSize = parent!.serialOut.count

            // Truncate logging info if it has become too large
            if serialOutSize > 8192 {
                parent!.serialOut = "...\n" + parent!.serialOut.dropFirst( serialOutSize - 8000)
            }

            // Update text view if necessary
            if poSerialOut.string.count != serialOutSize {
                poSerialOut.string = parent!.serialOut
                poSerialOut.scrollToEndOfDocument(nil)
                poSerialOut.font = serDatFont
            }
        }
    }
}
