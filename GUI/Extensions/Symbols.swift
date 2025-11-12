// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import AppKit

enum SFSymbol {

    case alarm
    case arrowClock
    case arrowDown
    case arrowkeys
    case arrowUp
    case console
    case gamecontroller
    case gauge
    case gear
    case keyboard
    case magnifyingglass
    case mouse
    case serverListening
    case serverConnected
    case nosign
    case pause
    case play
    case power
    case reset
    case trash

    var systemNames: [String] {

        switch self {

        case .alarm:            return [ "alarm" ]
        case .arrowClock:       return [ "clock.arrow.trianglehead.counterclockwise.rotate.90",
                                         "clock.arrow.circlepath" ]
        case .arrowDown:        return [ "arrow.down.circle" ]
        case .arrowkeys:        return [ "arrowkeys" ]
        case .arrowUp:          return [ "arrow.up.circle" ]
        case .console:          return [ "fossil.shell", "text.justify.left", "text.rectangle", "apple.terminal", "text.alignleft" ]
        case .gamecontroller:   return [ "gamecontroller" ]
        case .gauge:            return [ "gauge.chart.lefthalf.righthalf", "gauge.with.needle" ]
        case .gear:             return [ "gear" ]
        case .keyboard:         return [ "keyboard" ]
        case .magnifyingglass:  return [ "magnifyingglass" ]
        case .mouse:            return [ "computermouse" ]
        case .serverListening:  return [ "point.3.connected.trianglepath.dotted" ]
        case .serverConnected:  return [ "point.3.filled.connected.trianglepath.dotted" ]
        case .nosign:           return [ "nosign" ]
        case .pause:            return [ "pause.circle" ]
        case .play:             return [ "play.circle" ]
        case .power:            return [ "power" ]
        case .reset:            return [ "arrow.counterclockwise.circle" ]
        case .trash:            return [ "trash" ]
        }
    }

    static func get(_ symbol: SFSymbol, size: CGFloat = 25, description: String? = nil) -> NSImage {

        let config = NSImage.SymbolConfiguration(pointSize: size, weight: .light, scale: .small)

        for name in symbol.systemNames {

            if let img = NSImage(systemSymbolName: name, accessibilityDescription: description) {
                if let result = img.withSymbolConfiguration(config) {
                    return result
                }
            }
        }

        return NSImage(systemSymbolName: "questionmark.circle", accessibilityDescription: description)!
    }
}

