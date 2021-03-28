// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Renderer {
    
    var texW: CGFloat { return CGFloat(TextureSize.original.width) }
    var texH: CGFloat { return CGFloat(TextureSize.original.height) }
    
    func normalized(_ rect: CGRect) -> CGRect {
        
        return CGRect.init(x: rect.origin.x / texW,
                           y: rect.origin.y / texH,
                           width: rect.width / texW,
                           height: rect.height / texH)
    }

    // Returns the used texture area (including HBLANK and VBLANK)
    var entire: CGRect {
        
        return CGRect.init(x: 0, y: 0, width: 4 * Int(HPOS_CNT), height: Int(VPOS_CNT))
    }
    
    var entireNormalized: CGRect {
        
        return normalized(entire)
    }
    
    // Returns the largest visibile texture area (excluding HBLANK and VBLANK)
    var largestVisible: CGRect {
        
        let x1 = Int(HBLANK_CNT) * 4
        let x2 = Int(HPOS_CNT) * 4
        let y1 = Int(VBLANK_CNT)
        let y2 = Int(VPOS_CNT) - 1
        
        return CGRect.init(x: x1, y: y1, width: x2 - x1, height: y2 - y1)
    }
    
    var largestVisibleNormalized: CGRect {
        
        return normalized(largestVisible)
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
        
        let max = largestVisible
        
        let width = (1 - CGFloat(config.hZoom)) * max.width
        let bw = max.minX + CGFloat(config.hCenter) * (max.width - width)
        let height = (1 - CGFloat(config.vZoom)) * max.height
        let bh = max.minY + CGFloat(config.vCenter) * (max.height - height)
        
        return CGRect.init(x: bw, y: bh, width: width, height: height)
    }
    
    var visibleNormalized: CGRect {
        
        return normalized(visible)
    }
    
    func updateTextureRect() {
        
        textureRect = visibleNormalized
    }
    
    var textureRectAbs: CGRect {
        
        return CGRect.init(x: textureRect.origin.x * texW,
                           y: textureRect.origin.y * texH,
                           width: textureRect.width * texW,
                           height: textureRect.height * texH)
    }
}
