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
    
    // Alpha channel
    var alpha: AnimatedFloat = AnimatedFloat.init()
    var mix: AnimatedFloat = AnimatedFloat.init()

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
    var isVisible: Bool { return alpha.current > 0.0 }
    var isOpaque: Bool { return alpha.current == 1.0 }
    var isTransparent: Bool { return alpha.current < 1.0 }
    var isAnimating: Bool { return alpha.animates() }
    var isFadingIn: Bool { return alpha.target > alpha.current }
    var isFadingOut: Bool { return alpha.target < alpha.current }
        
    //
    // Opening and closing
    //
    
    func open(delay: Double) { alpha.steps = Int(60 * delay); open(); }
    func close(delay: Double) { alpha.steps = Int(60 * delay); close(); }
    func open() { alpha.target = 1.0 }
    func close() { alpha.target = 0.0 }
    func toggle() { isVisible ? close() : open(); }

    //
    // Performing continuous tasks
    //
    
    func update(frames: Int64) {
        
        if alpha.animates() {

            alpha.move()
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
