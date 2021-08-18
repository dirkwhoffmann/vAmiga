// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

//
// NSFont
//

extension NSFont {
 
    static func monospaced(ofSize fontSize: CGFloat, weight: Weight) -> NSFont {
        
        if #available(macOS 10.15, *) {
            return NSFont.monospacedSystemFont(ofSize: fontSize, weight: weight)
        } else {
            return NSFont.systemFont(ofSize: fontSize)
        }
    }
}
