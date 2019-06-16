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

    func refreshPorts(everything: Bool) {

        guard
            let controller = myController,
            let port1Info = amigaProxy?.controlPort1.getInfo(),
            let port2Info = amigaProxy?.controlPort2.getInfo(),
            let serialInfo = amigaProxy?.serialPort.getInfo(),
            let uartInfo = amigaProxy?.paula.getUARTInfo()
            else { return }

        if everything {

            let elements = [ po0POY: fmt16,
                             po0POX: fmt16,
                             po1POY: fmt16,
                             po1POX: fmt16,
                             poRecShift: fmt16,
                             poRecBuffer: fmt16,
                             poTransShift: fmt16,
                             poTransBuffer: fmt16
            ]

            for (c, f) in elements { assignFormatter(f, c!) }
        }

        // Control port 1
        po0M0V.state = port1Info.m0v ? .on : .off
        po0M0H.state = port1Info.m0h ? .on : .off
        po0M1V.state = port1Info.m1v ? .on : .off
        po0M1H.state = port1Info.m1h ? .on : .off
        po0POY.integerValue = Int(port1Info.poty)
        po0POX.integerValue = Int(port1Info.potx)

        // Control port 2
        po1M0V.state = port2Info.m0v ? .on : .off
        po1M0H.state = port2Info.m0h ? .on : .off
        po1M1V.state = port2Info.m1v ? .on : .off
        po1M1H.state = port2Info.m1h ? .on : .off
        po1POY.integerValue = Int(port2Info.poty)
        po1POX.integerValue = Int(port2Info.potx)

        // Serial port
        poTXD.state = serialInfo.txd ? .on : .off
        poRXD.state = serialInfo.rxd ? .on : .off
        poCTS.state = serialInfo.cts ? .on : .off
        poDSR.state = serialInfo.dsr ? .on : .off
        poCD.state = serialInfo.cd ? .on : .off
        poDTR.state = serialInfo.dtr ? .on : .off
        poRecShift.integerValue = Int(uartInfo.receiveShiftReg)
        poRecBuffer.integerValue = Int(uartInfo.receiveBuffer)
        poTransShift.integerValue = Int(uartInfo.transmitShiftReg)
        poTransBuffer.integerValue = Int(uartInfo.transmitBuffer)

        // Logging windows
        if refreshCounter % 2 == 0 {

            let serialInSize = controller.serialIn.count

            // Truncate logging info if it has become too large
            if serialInSize > 8192 {
                controller.serialIn = "...\n" + controller.serialIn.dropFirst( serialInSize - 8000)
            }

            // Update text view if necessary
            if poSerialIn.string.count != serialInSize {
                poSerialIn.string = controller.serialIn
                poSerialIn.scrollToEndOfDocument(nil)
                poSerialIn.font = serDatFont
            }

            let serialOutSize = controller.serialOut.count

            // Truncate logging info if it has become too large
            if serialOutSize > 8192 {
                controller.serialOut = "...\n" + controller.serialOut.dropFirst( serialOutSize - 8000)
            }

            // Update text view if necessary
            if poSerialOut.string.count != serialOutSize {
                poSerialOut.string = controller.serialOut
                poSerialOut.scrollToEndOfDocument(nil)
                poSerialOut.font = serDatFont
            }
        }
    }
}
