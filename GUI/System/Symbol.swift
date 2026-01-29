// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import AppKit

@MainActor
enum Symbol {
    
    // Actions
    case pause
    case play
    case power
    case reset
    case trash
    case stepInto
    case stepOver
    case stepCycle
    case stepLine
    case stepFrame

    // Status
    case serverListening
    case serverConnected
    case wifi

    // Devices
    case console
    case gamecontroller
    case keyboard
    case mouse
    case nosign

    // Indicators
    case arrowClock
    case arrowUp
    case arrowDown
    case arrowLeft
    case arrowRight
    case arrowkeys
    case gauge
    case gear
    case magnifyingglass
    
    var systemNames: [String] {

        switch self {

            // Actions
        case .pause:            return [ "pause.circle" ]
        case .play:             return [ "play.circle" ]
        case .power:            return [ "power" ]
        case .reset:            return [ "arrow.counterclockwise.circle" ]
        case .trash:            return [ "trash" ]

            // Status
        case .serverListening:  return [ "point.3.connected.trianglepath.dotted" ]
        case .serverConnected:  return [ "point.3.filled.connected.trianglepath.dotted" ]
        case .wifi:             return [ "wifi.circle" ]

            // Devices
        case .console:          return [ "fossil.shell", "text.justify.left", "text.rectangle", "apple.terminal", "text.alignleft" ]
        case .gamecontroller:   return [ "gamecontroller" ]
        case .keyboard:         return [ "keyboard" ]
        case .mouse:            return [ "computermouse" ]
        case .nosign:           return [ "nosign" ]

            // Indicators
        case .arrowClock:       return [ "clock.arrow.trianglehead.counterclockwise.rotate.90",
                                         "clock.arrow.circlepath" ]
        case .arrowUp:          return [ "arrowshape.up" ]
        case .arrowDown:        return [ "arrowshape.down" ]
        case .arrowLeft:        return [ "arrowshape.left" ]
        case .arrowRight:       return [ "arrowshape.right" ]
        case .arrowkeys:        return [ "arrowkeys" ]
        case .gauge:            return [ "gauge.chart.lefthalf.righthalf", "gauge.with.needle" ]
        case .gear:             return [ "gear" ]
        case .magnifyingglass:  return [ "magnifyingglass" ]
            
        default:                return [ ]
        }
    }

    var customIcon: String? {
        
        switch self {
            
        case .stepInto:         return "stepIntoTemplate"
        case .stepOver:         return "stepOverTemplate"
        case .stepCycle:        return "stepCycleTemplate"
        case .stepLine:         return "stepLineTemplate"
        case .stepFrame:        return "stepFrameTemplate"
            
        default:                return nil
        }
    }
    
    private struct CacheKey: Hashable {
     
        let symbol: Symbol
        let size: CGFloat
    }

    private static var imageCache = [CacheKey: NSImage]()

    static func get(_ symbol: Symbol, size: CGFloat = 25, description: String? = nil) -> NSImage {

        // Return cached image if available
        if let cached = imageCache[CacheKey(symbol: symbol, size: size)] { return cached }

        if let custom = symbol.customIcon {

            // Get the custom image
            let border = CGFloat(4)
            if let img = get(name: custom, size: size - border) {
                return img
            }

        } else {
            
            // Get a symbol from the SF library
            let config = NSImage.SymbolConfiguration(pointSize: size, weight: .light, scale: .small)
            for name in symbol.systemNames {
                if let img = NSImage(systemSymbolName: name, accessibilityDescription: description) {
                    if let result = img.withSymbolConfiguration(config) {
                        return result
                    }
                }
            }
        }

        // No image found
        return NSImage(systemSymbolName: "questionmark.circle", accessibilityDescription: description)!
    }
    
    static func get(name: String, size: CGFloat = 25) -> NSImage? {

        guard let img = NSImage(named: name) else { return nil }
        return img.resize(size: CGSize(width: size, height: size))
    }
}

