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
    static let geometry = 1
    static let alpha = 2
    static let texture = 4
}

class AnimatedFloat {

    var current: Float
    var delta = Float(0.0)
    var steps = 1 { didSet { delta = (target - current) / Float(steps) } }
    var target: Float { didSet { delta = (target - current) / Float(steps) } }

    init(current: Float = 0.0, target: Float = 0.0) {

        self.current = current
        self.target = target
    }

    convenience init(_ value: Float) {

        self.init(current: value, target: value)
    }

    func set(_ value: Float) {

        current = value
        target = value
    }

    func animates() -> Bool {

        return current != target
    }

    func move() {

        if abs(current - target) < abs(delta) {
            current = target
        } else {
            current += delta
        }
    }
}

extension MetalView {
 
    // Returns true iff an animation is in progress
    func animatesDeprecated() -> Bool {

        return angleX.animates()
            || angleY.animates()
            || angleZ.animates()
            || shiftX.animates()
            || shiftY.animates()
            || shiftZ.animates()
            || alpha.animates()
    }

    func performAnimationStep() {

        assert(animates != 0)

        var cont: Bool

        // Check for geometry animation
        if (animates & AnimationType.geometry) != 0 {

            angleX.move()
            angleY.move()
            angleZ.move()
            cont = angleX.animates() || angleY.animates() || angleZ.animates()

            shiftX.move()
            shiftY.move()
            shiftZ.move()
            cont = cont || shiftX.animates() || shiftY.animates() || shiftZ.animates()

            // Check if animation has terminated
            if !cont {
                animates -= AnimationType.geometry
                angleX.set(0)
                angleY.set(0)
                angleZ.set(0)
            }

            buildMatrices3D()
        }

        // Check for alpha channel animation
        if (animates & AnimationType.alpha) != 0 {

            alpha.move()
            noise.move()
            cont = alpha.animates() || noise.animates()

            // Check if animation has terminated
            if !cont {
                animates -= AnimationType.alpha
            }
        }

        // Check for texture animation
        if (animates & AnimationType.texture) != 0 {

            cutoutX1.move()
            cutoutY1.move()
            cutoutX2.move()
            cutoutY2.move()
            cont = cutoutX1.animates() || cutoutY1.animates() || cutoutX2.animates() || cutoutY2.animates()

            // Update texture cutout
            textureRect = CGRect.init(x: CGFloat(cutoutX1.current),
                                      y: CGFloat(cutoutY1.current),
                                      width: CGFloat(cutoutX2.current - cutoutX1.current),
                                      height: CGFloat(cutoutY2.current - cutoutY1.current))
            buildVertexBuffer()

            // Check if animation has terminated
            if !cont {
                animates -= AnimationType.texture
            }
        }
    }

    //
    // Texture animations
    //

    func zoomTextureIn(steps: Int = 30) {

        track("Zooming texture in...")

        cutoutX1.target = MetalView.cutoutX1default
        cutoutY1.target = MetalView.cutoutY1default
        cutoutX2.target = MetalView.cutoutX2default
        cutoutY2.target = MetalView.cutoutY2default

        cutoutX1.steps = steps
        cutoutY1.steps = steps
        cutoutX2.steps = steps
        cutoutY2.steps = steps

        animates |= AnimationType.texture
    }

    func zoomTextureOut(steps: Int = 30) {

        track("Zooming texture out...")

        cutoutX1.target = 0.0
        cutoutY1.target = 0.0
        cutoutX2.target = 1.0
        cutoutY2.target = 1.0

        cutoutX1.steps = steps
        cutoutY1.steps = steps
        cutoutX2.steps = steps
        cutoutY2.steps = steps

        animates |= AnimationType.texture
    }

    //
    // Geometry animations
    //

    func zoomIn(steps: Int = 60) {
    
        track("Zooming in...")

        shiftZ.current = 6.0
        shiftZ.target = 0.0
        angleX.target = 0.0
        angleY.target = 0.0
        angleZ.target = 0.0
        alpha.current = 0.0
        alpha.target = 1.0
        noise.current = 1.0
        noise.target = 0.0

        shiftZ.steps = steps
        angleX.steps = steps
        angleY.steps = steps
        angleZ.steps = steps
        alpha.steps = steps
        noise.steps = steps

        animates |= AnimationType.geometry + AnimationType.alpha
    }

    func zoomOut(steps: Int = 40) {

        track("Zooming out...")

        shiftZ.target = 6.0
        angleX.target = 0.0
        angleY.target = 0.0
        angleZ.target = 0.0
        alpha.target = 0.0
        noise.target = 1.0

        shiftZ.steps = steps
        angleX.steps = steps
        angleY.steps = steps
        angleZ.steps = steps
        alpha.steps = steps
        noise.steps = steps

        animates |= AnimationType.geometry + AnimationType.alpha
    }

    func rotate(x: Float = 0.0, y: Float = 0.0, z: Float = 0.0) {

        track("Rotating x: \(x) y: \(y) z: \(z)...")

        angleX.target = x
        angleY.target = y
        angleZ.target = z

        let steps = 60
        angleX.steps = steps
        angleY.steps = steps
        angleZ.steps = steps

        animates |= AnimationType.geometry
    }

    func rotateRight() { rotate(y: -90) }
    func rotateLeft() { rotate(y: 90) }
    func rotateDown() { rotate(x: 90) }
    func rotateUp() { rotate(x: -90) }

    func scroll(steps: Int = 120) {
        
        track("Scrolling...")

        shiftY.current = -1.5
        shiftY.target = 0
        angleX.target = 0.0
        angleY.target = 0.0
        angleZ.target = 0.0
        alpha.target = 1.0

        shiftY.steps = steps
        angleX.steps = steps
        angleY.steps = steps
        angleZ.steps = steps
        alpha.steps = 1

        animates |= AnimationType.geometry + AnimationType.alpha
    }

    func snapToFront() {

        track("Snapping to front...")

        shiftZ.current = -0.05
        shiftZ.target = 0
        shiftZ.steps = 10

        animates |= AnimationType.geometry
    }

    //
    // Alpha channel animations
    //

    func blend(from: Float, to: Float, steps: Int) {

        track("Blending...")

        angleX.target = 0
        angleY.target = 0
        angleZ.target = 0
        alpha.current = from
        alpha.target = to

        angleX.steps = steps
        angleY.steps = steps
        angleZ.steps = steps
        alpha.steps = steps

        animates |= AnimationType.alpha
    }

    func blendIn(steps: Int = 40) {

        noise.target = 0.0
        noise.steps = steps
        blend(from: 0.0, to: 1.0, steps: steps)
    }

    func blendOut(steps: Int = 40) {

        noise.target = 1.0
        noise.steps = steps
        blend(from: 1.0, to: 0.0, steps: steps)
    }

    //
    // Matrix utilities
    //
    
    func perspectiveMatrix(fovY: Float,
                           aspect: Float,
                           nearZ: Float,
                           farZ: Float) -> matrix_float4x4 {
        
        // Variant 1: Keeps correct aspect ratio independent of window size
        let yscale = 1.0 / tanf(fovY * 0.5) // 1 / tan == cot
        let xscale = yscale / aspect
        let q = farZ / (farZ - nearZ)
    
        // Alternative: Adjust to window size
        // float yscale = 1.0f / tanf(fovY * 0.5f);
        // float xscale = 0.75 * yscale;
        // float q = farZ / (farZ - nearZ);
        
        var m = matrix_float4x4()
        m.columns.0 = SIMD4<Float>(xscale, 0.0, 0.0, 0.0)
        m.columns.1 = SIMD4<Float>(0.0, yscale, 0.0, 0.0)
        m.columns.2 = SIMD4<Float>(0.0, 0.0, q, 1.0)
        m.columns.3 = SIMD4<Float>(0.0, 0.0, q * -nearZ, 0.0)
    
        return m
    }
    
    func translationMatrix(x: Float,
                           y: Float,
                           z: Float) -> matrix_float4x4 {

        var m = matrix_identity_float4x4
        m.columns.3 = SIMD4<Float>(x, y, z, 1.0)
    
        return m
    }
    
    func rotationMatrix(radians: Float,
                        x: Float,
                        y: Float,
                        z: Float) -> matrix_float4x4 {

        var v = vector_float3(x, y, z)
        v = normalize(v)
        let cos = cosf(radians)
        let cosp = 1.0 - cos
        let sin = sinf(radians)
    
        var m = matrix_float4x4()
        m.columns.0 = SIMD4<Float>(cos + cosp * v.x * v.x,
                                   cosp * v.x * v.y + v.z * sin,
                                   cosp * v.x * v.z - v.y * sin,
                                   0.0)
        m.columns.1 = SIMD4<Float>(cosp * v.x * v.y - v.z * sin,
                                   cos + cosp * v.y * v.y,
                                   cosp * v.y * v.z + v.x * sin,
                                   0.0)
        m.columns.2 = SIMD4<Float>(cosp * v.x * v.z + v.y * sin,
                                   cosp * v.y * v.z - v.x * sin,
                                   cos + cosp * v.z * v.z,
                                   0.0)
        m.columns.3 = SIMD4<Float>(0.0,
                                   0.0,
                                   0.0,
                                   1.0)
        return m
    }
}
