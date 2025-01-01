// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// import Cocoa

class LogicViewFormatter {

    var hex = false
    var symbolic = false

    func string(from value: Int, bitWidth: Int) -> String {

        /*
        if symbolic, bitWidth == 16 {

            if TIARegister.isTIAAddr(addr: value) {
                return TIARegister(rawValue: value)?.description ?? "???"
            }
            if PIARegister.isPIAAddr(addr: value) {
                return PIARegister(rawValue: value)?.description ?? "???"
            }
        }
        */
        
        switch bitWidth {

        case 8 where hex:   return String(format: "%02X", value)
        case 16 where hex:  return String(format: "%04X", value)
        default:            return String(value)
        }
    }
}
