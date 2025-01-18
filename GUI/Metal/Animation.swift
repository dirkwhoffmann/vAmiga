// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import simd

struct AnimationType {
    
    static let color = 1
    static let texture = 2
}

struct AnimatedFloat {

    var current: Float
    var delta = Float(0.0)
    var steps = 1 { didSet { delta = (target - current) / Float(steps) } }
    var target: Float { didSet { delta = (target - current) / Float(steps) } }
    var animates: Bool { return current != target }
    var clamped: Float { return current < 0.0 ? 0.0 : current > 1.0 ? 1.0 : current }
    
    init(current: Float = 0.0, target: Float = 0.0) {

        self.current = current
        self.target = target
    }

    init(_ value: Float) {

        self.init(current: value, target: value)
    }

    mutating func set(_ value: Float) {

        self.current = value
        self.target = value
    }

    mutating func set(from: Float, to: Float, steps: Int) {
            
        self.current = from
        self.target = to
        self.steps = steps
    }
    
    mutating func move() {

        if abs(current - target) < abs(delta) {
            current = target
        } else {
            current += delta
        }
    }
}

extension Renderer {
    
    @MainActor
    func animate() {
                
        // Color animations
        if (animates & AnimationType.color) != 0 {

            white.move()
         
            let cont = white.animates
         
            if !cont {
                animates -= AnimationType.color
            }
        }
        
        // Texture animations
        if (animates & AnimationType.texture) != 0 {
            
            cutoutX1.move()
            cutoutY1.move()
            cutoutX2.move()
            cutoutY2.move()
            
            let cont = cutoutX1.animates || cutoutY1.animates || cutoutX2.animates || cutoutY2.animates
            
            let x = CGFloat(cutoutX1.current)
            let y = CGFloat(cutoutY1.current)
            let w = CGFloat(cutoutX2.current - cutoutX1.current)
            let h = CGFloat(cutoutY2.current - cutoutY1.current)
            
            // Update texture cutout
            canvas.textureRect = CGRect(x: x, y: y, width: w, height: h)
         
            if !cont {
                animates -= AnimationType.texture
            }
        }
    }
    
    //
    // Texture animations
    //

    @MainActor
    func zoomTextureIn(steps: Int = 30) {

        let current = canvas.textureRect
        let target = canvas.visibleNormalized

        cutoutX1.current = Float(current.minX)
        cutoutY1.current = Float(current.minY)
        cutoutX2.current = Float(current.maxX)
        cutoutY2.current = Float(current.maxY)
        
        cutoutX1.target = Float(target.minX)
        cutoutY1.target = Float(target.minY)
        cutoutX2.target = Float(target.maxX)
        cutoutY2.target = Float(target.maxY)

        cutoutX1.steps = steps
        cutoutY1.steps = steps
        cutoutX2.steps = steps
        cutoutY2.steps = steps

        animates |= AnimationType.texture
    }

    @MainActor
    func zoomTextureOut(steps: Int = 30) {
        
        let current = canvas.textureRect
        let target = canvas.entireNormalized
        
        cutoutX1.current = Float(current.minX)
        cutoutY1.current = Float(current.minY)
        cutoutX2.current = Float(current.maxX)
        cutoutY2.current = Float(current.maxY)

        cutoutX1.target = Float(target.minX)
        cutoutY1.target = Float(target.minY)
        cutoutX2.target = Float(target.maxX)
        cutoutY2.target = Float(target.maxY)

        cutoutX1.steps = steps
        cutoutY1.steps = steps
        cutoutX2.steps = steps
        cutoutY2.steps = steps

        animates |= AnimationType.texture
    }
    
    //
    // Color animation
    //
    
    @MainActor
    func flash(steps: Int = 20) {
                
        white.current = 1.0
        white.target = 0.0
        white.steps = steps
        
        animates |= AnimationType.color
    }

    @MainActor 
    func blend(steps: Int = 20) {

        canvas.alpha.current = 0
        canvas.alpha.steps = steps
    }
}
