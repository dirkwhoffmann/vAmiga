// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Canvas {
    
    var texW: CGFloat { return CGFloat(TextureSize.original.width) }
    var texH: CGFloat { return CGFloat(TextureSize.original.height) }
    
    fileprivate func normalize(_ rect: CGRect) -> CGRect {
        
        return CGRect(x: rect.origin.x / texW,
                      y: rect.origin.y / texH,
                      width: rect.width / texW,
                      height: rect.height / texH)
    }

    // Returns the used texture area (including HBLANK and VBLANK)
    var entire: CGRect {
        
        return CGRect(x: 0, y: 0, width: 4 * Int(HPOS_CNT), height: Int(VPOS_CNT))
    }
    
    var entireNormalized: CGRect {
        
        return normalize(entire)
    }
    
    // Returns the largest visibile texture area (excluding HBLANK and VBLANK)
    var largestVisible: CGRect {
        
        let x1 = Int(HBLANK_CNT) * 4
        let x2 = Int(HPOS_CNT) * 4
        let y1 = Int(VBLANK_CNT)
        let y2 = Int(VPOS_CNT) - 2
        
        return CGRect(x: x1, y: y1, width: x2 - x1, height: y2 - y1)
    }
    
    var largestVisibleNormalized: CGRect {
        
        return normalize(largestVisible)
    }
    
    // Returns the visible texture area based on the zoom and center parameters
    var visible: CGRect {
        
        /*
         *       aw <--------- maxWidth --------> dw
         *    ah |-----|---------------------|-----|
         *     ^ |     bw                   cw     |
         *     | -  bh *<----- width  ------>*     -
         *     | |     ^                     ^     |
         *     | |     |                     |     |
         *     | |   height                height  |
         *     | |     |                     |     |
         *     | |     v                     v     |
         *     | -  ch *<----- width  ------>*     -
         *     v |                                 |
         *    dh |-----|---------------------|-----|
         *
         *      aw/ah - dw/dh = largest posible texture cutout
         *      bw/bh - cw/ch = currently used texture cutout
         */
        
        let largest = largestVisible
        
        let hscale = CGFloat(1.0 - 0.2 * renderer.config.hZoom)
        let vscale = CGFloat(1.0 - 0.2 * renderer.config.vZoom)
        let width = hscale * largest.width
        let height = vscale * largest.height

        var bw: CGFloat
        var bh: CGFloat
        
        if renderer.parent.config.hAutoCenter {
            
            bw = x1 - 0.5 * (width - (x2 - x1))
            bw = max(bw, largest.minX)
            bw = min(bw, largest.maxX - width)
            // log("AutoShift x: \(bw)")
            
        } else {
            
            bw = largest.minX + CGFloat(renderer.config.hCenter) * (largest.width - width)
        }
        
        if renderer.parent.config.vAutoCenter {
            
            bh = y1 - 0.5 * (height - (y2 - y1))
            bh = max(bh, largest.minY)
            bh = min(bh, largest.maxY - height)
            // log("AutoShift y: \(bh)")
            
        } else {
            
            bh = largest.minY + CGFloat(renderer.config.vCenter) * (largest.height - height)
        }
                
        return CGRect(x: bw, y: bh, width: width, height: height)
    }
    
    var visibleNormalized: CGRect {
        
        return normalize(visible)
    }
    
    func updateTextureRect() {
        
        textureRect = visibleNormalized
    }

    func updateTextureRect(hstrt: Int, vstrt: Int, hstop: Int, vstop: Int) {

        // log("updateTextureRect \(hstrt) \(vstrt) \(hstop) \(vstop)")

        // Convert to pixel coordinates
        x1 = 2 * CGFloat(hstrt)
        x2 = 2 * CGFloat(hstop)
        y1 = CGFloat(vstrt)
        y2 = CGFloat(vstop)

        // Crop
        let max = largestVisible
        if x1 < max.minX { x1 = max.minX }
        if y1 < max.minY { y1 = max.minY }
        if x2 > max.maxX { x2 = max.maxX }
        if y2 > max.maxY { y2 = max.maxY }

        // log("(\(x1),\(y1)) - \(x2),\(y2))")

        // Compensate the texture shift
        x1 -= CGFloat(HBLANK_MIN) * 4
        x2 -= CGFloat(HBLANK_MIN) * 4
        
        updateTextureRect()
    }
    
    var textureRectAbs: CGRect {
        
        return CGRect(x: textureRect.origin.x * texW,
                      y: textureRect.origin.y * texH,
                      width: textureRect.width * texW,
                      height: textureRect.height * texH)
    }
}
