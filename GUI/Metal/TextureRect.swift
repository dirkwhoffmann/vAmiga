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
        let y2 = Int(VPOS_CNT) - 1
        
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

        /*
        log("config.hZoom: \(renderer.config.hZoom)")
        log("config.vZoom: \(renderer.config.vZoom)")
        log("config.hCenter: \(renderer.config.hCenter)")
        log("config.vCenter: \(renderer.config.vCenter)")
        */
        
        let max = largestVisible
        
        let hscale = CGFloat(1.0 - 0.2 * renderer.config.hZoom)
        let vscale = CGFloat(1.0 - 0.2 * renderer.config.vZoom)
        let width = hscale * max.width
        let height = vscale * max.height

        var bw: CGFloat
        var bh: CGFloat
        
        if x1 != nil && x2 != nil {
            
            let diww = x2! - x1!
            bw = x1! - 0.5 * (width - diww)
            if bw < max.minX { bw = max.minX }
            // log("Adaptive viewport: x = \(diww)")
            
        } else {
            
            bw = max.minX + CGFloat(renderer.config.hCenter) * (max.width - width)
        }
        
        if y1 != nil && y2 != nil {
            
            let diwh = y2! - y1!
            bh = y1! - 0.5 * (height - diwh)
            
            if bh < max.minY { bh = max.minY }
            // log("Adaptive viewport: y = \(diwh)")
            
        } else {
            
            bh = max.minY + CGFloat(renderer.config.vCenter) * (max.height - height)
        }
        
        /*
         log("width: \(width)")
         log("bw: \(bw)")
         log("height: \(height)")
         log("bh: \(bh)")
         */
        
        return CGRect(x: bw, y: bh, width: width, height: height)
    }
    
    var visibleNormalized: CGRect {
        
        return normalize(visible)
    }
    
    func updateTextureRect() {
        
        textureRect = visibleNormalized
    }

    func updateTextureRect(hstrt: Int, vstrt: Int, hstop: Int, vstop: Int) {

        log("updateTextureRect")

        // Convert to pixel coordinates
        x1 = 2 * CGFloat(hstrt)
        x2 = 2 * CGFloat(hstop)
        y1 = CGFloat(vstrt)
        y2 = CGFloat(vstop)

        // Crop
        let max = largestVisible
        if x1! < max.minX { x1 = max.minX }
        if y1! < max.minY { y1 = max.minY }
        if x2! > max.maxX { x2 = max.maxX }
        if y2! > max.maxY { y2 = max.maxY }

        log("(\(x1!),\(y1!)) - \(x2!),\(y2!))")

        // Compensate the texture shift
        x1! -= CGFloat(HBLANK_MIN) * 4
        x2! -= CGFloat(HBLANK_MIN) * 4
        
        updateTextureRect()
    }
    
    var textureRectAbs: CGRect {
        
        return CGRect(x: textureRect.origin.x * texW,
                      y: textureRect.origin.y * texH,
                      width: textureRect.width * texW,
                      height: textureRect.height * texH)
    }
}
