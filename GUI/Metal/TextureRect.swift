// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
extension Canvas {
    
    var texW: CGFloat { return CGFloat(TextureSize.original.width) }
    var texH: CGFloat { return CGFloat(TextureSize.original.height) }
    
    fileprivate func normalize(_ rect: CGRect) -> CGRect {
        
        return CGRect(x: rect.origin.x / texW,
                      y: rect.origin.y / texH,
                      width: rect.width / texW,
                      height: rect.height / texH)
    }

    // Returns the entire texture area (including HBLANK and VBLANK)
    var entire: CGRect {
        
        return CGRect(x: 0, y: 0, width: 4 * Int(TPP) * VAMIGA.HPOS.CNT, height: VAMIGA.VPOS.CNT)
    }
    
    var entireNormalized: CGRect {
        
        return normalize(entire)
    }
    
    // Returns the largest visibile texture area (excluding HBLANK and VBLANK)
    var largestVisible: CGRect {

        let pal = emu.agnus.traits.isPAL
        
        let x1 = 4 * Int(TPP) * VAMIGA.HBLANK.CNT
        let x2 = 4 * Int(TPP) * VAMIGA.PAL.HPOS.CNT
        let y1 = pal ? VAMIGA.PAL.VBLANK.CNT : VAMIGA.NTSC.VBLANK.CNT
        let y2 = pal ? VAMIGA.PAL.VPOS.CNT : VAMIGA.NTSC.VPOS.CNT

        return CGRect(x: x1, y: y1, width: x2 - x1, height: y2 - y1 - 1)
    }
    
    var largestVisibleNormalized: CGRect {
        
        return normalize(largestVisible)
    }
    
    // Returns the visible texture area based on the zoom and center parameters
    var visible: CGRect {

        // Determine zoom factors
        var hZoom = Float(renderer.config.hZoom) / 100.0
        var vZoom = Float(renderer.config.vZoom) / 100.0

        // Determine offsets
        let hCenter = Float(renderer.config.hCenter) / 100.0
        let vCenter = Float(renderer.config.vCenter) / 100.0

        switch renderer.config.zoom {

        case 1: hZoom = 1.0; vZoom = 0.27       // Narrow
        case 2: hZoom = 0.747; vZoom = 0.032    // Wide
        case 3: hZoom = 0; vZoom = 0            // Extreme
        default: break
        }

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
        
        let hscale = CGFloat(1.0 - 0.2 * hZoom)
        let vscale = CGFloat(1.0 - 0.2 * vZoom)
        let width = hscale * largest.width
        let height = vscale * largest.height

        var bw: CGFloat
        var bh: CGFloat
        
        if renderer.parent.config.center == 1 {
            
            bw = x1 - 0.5 * (width - (x2 - x1))
            bw = max(bw, largest.minX)
            bw = min(bw, largest.maxX - width)
            debug(.events, "AutoShift x: \(bw)")

            bh = y1 - 0.5 * (height - (y2 - y1))
            bh = max(bh, largest.minY)
            bh = min(bh, largest.maxY - height)
            debug(.events, "AutoShift y: \(bh)")

        } else {
            
            bw = largest.minX + CGFloat(hCenter) * (largest.width - width)
            bh = largest.minY + CGFloat(vCenter) * (largest.height - height)
        }
                
        return CGRect(x: bw, y: bh, width: width, height: height)
    }
    
    var visibleNormalized: CGRect {
        
        return normalize(visible)
    }
    
    func updateTextureRect() {

        if emu.get(.DMA_DEBUG_ENABLE) != 0 {
            textureRect = entireNormalized
        } else {
            textureRect = visibleNormalized
        }
    }

    func updateTextureRect(hstrt: Int, vstrt: Int, hstop: Int, vstop: Int) {

        debug(.metal, "updateTextureRect(\(hstrt), \(vstrt), \(hstop), \(vstop))")

        // Convert to pixel coordinates
        x1 = CGFloat(2 * Int(TPP) * hstrt)
        x2 = CGFloat(2 * Int(TPP) * hstop)
        y1 = CGFloat(vstrt)
        y2 = CGFloat(vstop)

        // Compensate the texture shift
        x1 -= CGFloat(VAMIGA.HBLANK.MIN) * CGFloat(TPP) * 4
        x2 -= CGFloat(VAMIGA.HBLANK.MIN) * CGFloat(TPP) * 4

        // Crop
        let max = largestVisible
        if x1 < max.minX { x1 = max.minX }
        if y1 < max.minY { y1 = max.minY }
        if x2 > max.maxX { x2 = max.maxX }
        if y2 > max.maxY { y2 = max.maxY }

        debug(.metal, "New texture rect = (\(x1),\(y1)) - (\(x2),\(y2))")

        updateTextureRect()
    }
    
    var textureRectAbs: CGRect {
        
        return CGRect(x: textureRect.origin.x * texW,
                      y: textureRect.origin.y * texH,
                      width: textureRect.width * texW,
                      height: textureRect.height * texH)
    }
}
