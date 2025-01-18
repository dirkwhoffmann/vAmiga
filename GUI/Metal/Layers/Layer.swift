// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Layer: NSObject {
    
    let renderer: Renderer
    
    var ressourceManager: RessourceManager { return renderer.ressourceManager }
    var device: MTLDevice { return renderer.device }
    var view: MTKView { return renderer.view }
    var amiga: EmulatorProxy { return renderer.parent.emu }
    
    // Alpha channel of this layer
    var alpha: AnimatedFloat = AnimatedFloat(0.0)

    //
    // Initializing
    //
    
    init(renderer: Renderer) {
        
        self.renderer = renderer
        super.init()
    }
    
    //
    // Querying the visual state
    //
        
    @MainActor var isVisible: Bool { return alpha.current > 0.0 }
    @MainActor var isOpaque: Bool { return alpha.current == 1.0 }
    @MainActor var isTransparent: Bool { return alpha.current < 1.0 }
    @MainActor var isAnimating: Bool { return alpha.animates }
    @MainActor var isFadingIn: Bool { return alpha.target > alpha.current }
    @MainActor var isFadingOut: Bool { return alpha.target < alpha.current }
        
    //
    // Opening and closing
    //
    
    @MainActor func open(delay: Double) { alpha.steps = Int(60 * delay); open(); }
    @MainActor func close(delay: Double) { alpha.steps = Int(60 * delay); close(); }
    @MainActor func open() { alpha.target = 1.0 }
    @MainActor func close() { alpha.target = 0.0 }
    @MainActor func toggle() { isVisible ? close() : open(); }

    //
    // Performing continuous tasks
    //
    
    @MainActor
    func update(frames: Int64) {
        
        if alpha.animates {

            alpha.move()
            alphaDidChange()
            
            if !alpha.animates {
                isVisible ? layerDidOpen() : layerDidClose()
            }
        }
    }
    
    @MainActor func alphaDidChange() { }
    @MainActor func layerDidOpen() { }
    @MainActor func layerDidClose() { }
}
