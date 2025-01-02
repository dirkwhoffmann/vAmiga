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
        case 24 where hex:  return String(format: "%06X", value)
        case 32 where hex:  return String(format: "%08X", value)
        default:            return String(value)
        }
    }
    
    func string(from value: Int, probe: Probe) -> String {

        switch probe {

        case .BUS_OWNER:
            switch BusOwner(rawValue: Int8(value)) {
            case .CPU:      return "CPU"
            case .REFRESH:  return "REF"
            case .DISK:     return "DSK"
            case .AUD0:     return "AUD0"
            case .AUD1:     return "AUD1"
            case .AUD2:     return "AUD2"
            case .AUD3:     return "AUD3"
            case .BPL1:     return "BLP1"
            case .BPL2:     return "BPL2"
            case .BPL3:     return "BPL3"
            case .BPL4:     return "BPL4"
            case .BPL5:     return "BPL5"
            case .BPL6:     return "BPL6"
            case .SPRITE0:  return "SPR0"
            case .SPRITE1:  return "SPR1"
            case .SPRITE2:  return "SPR2"
            case .SPRITE3:  return "SPR3"
            case .SPRITE4:  return "SPR4"
            case .SPRITE5:  return "SPR5"
            case .SPRITE6:  return "SPR6"
            case .SPRITE7:  return "SPR7"
            case .COPPER:   return "COP"
            case .BLITTER:  return "BLT"
            case .BLOCKED:  return "BLK"
            default:        return ""
            }

        case .ADDR_BUS where hex:
            return String(format: "%06X", value)

        case .DATA_BUS where hex, .MEMORY where hex:
            return String(format: "%04X", value)

        default:
            return String(value)
        }
    }
}
