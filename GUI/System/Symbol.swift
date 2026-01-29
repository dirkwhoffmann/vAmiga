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
    case inspect
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
    case warpAutoOn
    case warpAutoOff
    case warpOn
    case warpOff

    // Pictograms
    case biohazard
    case exclamationmark
    case gear
    case magnifyingglass
    case shell
    case stearingwheel
    case wifi

    // Devices
    case amigaKey
    case cable
    case cableLoop
    case console
    case floppy35
    case floppy35wp
    case floppy525
    case floppy525wp
    case gamecontroller
    case harddrive
    case keyboard
    case midi
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
    
    var systemNames: [String] {

        switch self {

            // Actions
        case .inspect:          return [ "magnifyingglass" ]
        case .pause:            return [ "pause.circle" ]
        case .play:             return [ "play.circle" ]
        case .power:            return [ "power" ]
        case .reset:            return [ "arrow.counterclockwise.circle" ]
        case .trash:            return [ "trash" ]
        case .stepInto:         return [ "stepIntoTemplate" ]
        case .stepOver:         return [ "stepOverTemplate" ]
        case .stepCycle:        return [ "stepCycleTemplate" ]
        case .stepLine:         return [ "stepLineTemplate" ]
        case .stepFrame:        return [ "stepFrameTemplate" ]

            // Status
        case .serverListening:  return [ "point.3.connected.trianglepath.dotted" ]
        case .serverConnected:  return [ "point.3.filled.connected.trianglepath.dotted" ]
        case .warpAutoOn:       return [ "hourglass3Template" ]
        case .warpAutoOff:      return [ "hourglass1Template" ]
        case .warpOn:           return [ "warpOnTemplate" ]
        case .warpOff:          return [ "warpOffTemplate" ]

            // Pictograms
        case .biohazard:        return [ "biohazardTemplate" ]
        case .exclamationmark:  return [ "exclamationmark.octagon" ]
        case .gear:             return [ "gear" ]
        case .magnifyingglass:  return [ "magnifyingglass" ]
        case .shell:            return [ "fossil.shell" ]
        case .stearingwheel:    return [ "steeringwheel" ]
        case .wifi:             return [ "wifi.circle" ]

            // Devices
        case .amigaKey:         return [ "amigaKey.template" ]
        case .cable:            return [ "point.bottomleft.forward.to.point.topright.scurvepath" ]
        case .cableLoop:        return [ "point.forward.to.point.capsulepath" ]
        case .console:          return [ "fossil.shell", "text.justify.left", "text.rectangle", "apple.terminal", "text.alignleft" ]
        case .floppy35:         return [ "floppy35Template" ]
        case .floppy35wp:       return [ "floppy35wpTemplate" ]
        case .floppy525:        return [ "" ]
        case .floppy525wp:      return [ "" ]
        case .gamecontroller:   return [ "gamecontroller" ]
        case .harddrive:        return [ "hdrTemplate" ]
        case .keyboard:         return [ "keyboard" ]
        case .midi:             return [ "devMidiInterfaceTemplate" ]
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

        let config = NSImage.SymbolConfiguration(pointSize: size, weight: .light, scale: .small)
        let border = CGFloat(4)

        for name in symbol.systemNames {

            // Look up the image in the assets folder (custom images)
            if let img = get(name: name, size: size - border) {
                img.isTemplate = true
                return img
            }
            
            // Look up the image in the system SF library
            if let img = NSImage(systemSymbolName: name, accessibilityDescription: description) {
                if let result = img.withSymbolConfiguration(config) {
                    return result
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

