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

        guard let controller = myController else { return }

        if everything {

        }

        if refreshCounter % 2 == 0 {

            let serialInSize = controller.serialIn.count
            if serialInSize > 8192 {
                controller.serialIn = "...\n" + controller.serialIn.dropFirst( serialInSize - 8000)
            }
            poSerialIn.string = controller.serialIn
            poSerialIn.scrollToEndOfDocument(nil)
            poSerialIn.font = serDatFont

            let serialOutSize = controller.serialOut.count
            if serialOutSize > 8192 {
                controller.serialOut = "...\n" + controller.serialOut.dropFirst( serialOutSize - 8000)
            }
            poSerialOut.string = controller.serialOut
            poSerialOut.scrollToEndOfDocument(nil)
            poSerialOut.font = serDatFont
        }
    }
}
