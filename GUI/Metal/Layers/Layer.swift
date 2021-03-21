// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Layer: NSObject {
    
    let mtkView: MTKView
    let device: MTLDevice
    let renderer: Renderer
    
    var amiga: AmigaProxy { return renderer.parent.amiga }
    var commandEncoder: MTLRenderCommandEncoder { return renderer.commandEncoder }
    
    // Alpha channel parameters
    var alpha = 0
    var targetAlpha = 0
    
    // Time until alpha is supposed to reach targetAlpha in seconds
    var delay = 0.2
    
    //
    // Initializing
    //
    
    init(view: MTKView, device: MTLDevice, renderer: Renderer) {
        
        self.mtkView = view
        self.device = device
        self.renderer = renderer
        
        super.init()
    }
    
    //
    // Working with the layer
    //
        
    // Informs about the visual state of this layer
    var isVisible: Bool { return alpha > 0 }
    var isOpaque: Bool { return alpha == 0xFF; }
    var isTransparent: Bool { return alpha < 0xFF; }
    var isAnimating: Bool { return alpha != targetAlpha; }
    var isFadingIn: Bool { return targetAlpha > alpha; }
    var isFadingOut: Bool { return targetAlpha < alpha; }
    
    var floatAlpha: Float { return alpha < 0 ? 0 : alpha > 0xFF ? 1.0 : Float(alpha) / 0xFF }
    
    //
    // Opening and closing
    //
    
    func open(delay: Double) { self.delay = delay; open(); }
    func close(delay: Double) { self.delay = delay; close(); }
    func open() { targetAlpha = 0xFF }
    func close() { targetAlpha = 0x00 }
    func toggle() { isVisible ? close() : open(); }

    //
    // Performing continuous tasks
    //
    
    func update(frames: Int64) {
        
        if alpha != targetAlpha {
            
            let delta = 1.0 / 60.0
            
            if alpha < targetAlpha {
                alpha += Int(0xFF * delta / delay)
                if alpha > targetAlpha { alpha = targetAlpha }
            }
            if alpha > targetAlpha {
                alpha -= Int(0xFF * delta / delay)
                if alpha < targetAlpha { alpha = targetAlpha }
            }
            
            alphaDidChange()
        }
    }

    func render(buffer: MTLCommandBuffer) { }
    func render(encoder: MTLRenderCommandEncoder, flat: Bool) { }
    
    //
    // Responding to events
    //
    
    func alphaDidChange() { }
}
