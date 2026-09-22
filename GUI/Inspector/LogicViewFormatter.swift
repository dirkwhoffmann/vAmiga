// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// import Cocoa

@MainActor
class LogicViewFormatter {

    var hex = false
    var padding = false
    var symbolic = false

    /* Zero padding follows the inspector's format setting rather than the
     * radix. Hex used to be padded unconditionally here and decimal never,
     * which made the logic analyzer disagree with the format the rest of the
     * inspector was showing (Inspector.padding already drives fmt4...fmt32).
     * The width is the natural one for a 'bitWidth'-wide value in the
     * current radix.
     */
    func string(from value: Int, bitWidth: Int) -> String {

        if !padding { return hex ? String(format: "%X", value) : String(value) }

        let digits = hex ? (bitWidth + 3) / 4 : String((1 << bitWidth) - 1).count

        return hex ? String(format: "%0\(digits)X", value)
                   : String(format: "%0\(digits)u", value)
    }
}
