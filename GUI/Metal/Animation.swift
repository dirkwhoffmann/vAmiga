// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation
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
            || eyeX.animates()
            || eyeY.animates()
            || eyeZ.animates()
            || alpha.animates()
    }

    func textureAnimatesDeprecated() -> Bool {

        return cutoutX1.animates()
            || cutoutY1.animates()
            || cutoutX2.animates()
            || cutoutY2.animates()
    }
    
    func getEyeX() -> Float {
        
        return eyeX.current // currentEyeX
    }
    
    func setEyeX(_ newX: Float) {

        eyeX.set(newX)
        self.buildMatrices3D()
    }
    
    func getEyeY() -> Float {
        
        return eyeY.current
    }
    
    func setEyeY(_ newY: Float) {

        eyeY.set(newY)
        self.buildMatrices3D()
    }
    
    func getEyeZ() -> Float {
        
        return eyeZ.current
    }
    
    func setEyeZ(_ newZ: Float) {

        eyeZ.set(newZ)
        self.buildMatrices3D()
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

            eyeX.move()
            eyeY.move()
            eyeZ.move()
            cont = cont || eyeX.animates() || eyeY.animates() || eyeZ.animates()

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
            cont = alpha.animates()

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

            // Check if animation has terminated
            if !cont {
                animates -= AnimationType.texture
            }
        }
    }

    func updateAngles() {

        angleX.move()
        angleY.move()
        angleZ.move()

        eyeX.move()
        eyeY.move()
        eyeZ.move()

        alpha.move()

        // DEPRECATED
        angleX.current -= (angleX.current >= 360) ? 360 : 0
        angleX.current += (angleX.current < 0) ? 360 : 0
        angleY.current -= (angleY.current >= 360.0) ? 360 : 0
        angleY.current += (angleY.current < 0.0) ? 360 : 0
        angleZ.current -= (angleZ.current >= 360.0) ? 360 : 0
        angleZ.current += (angleZ.current < 0.0) ? 360 : 0
    }

    func updateTextureRect() {

        cutoutX1.move()
        cutoutY1.move()
        cutoutX2.move()
        cutoutY2.move()

        updateScreenGeometry()
    }

    //
    // Texture animations
    //

    func zoomTextureIn(cycles: Int = 60) {

        track("Zooming texture in...")

        cutoutX1.target = 0.0
        cutoutY1.target = 0.0
        cutoutX2.target = 728.0 / 768.0
        cutoutY2.target = 286.0 / 288.0

        cutoutX1.steps = cycles
        cutoutY1.steps = cycles
        cutoutX2.steps = cycles
        cutoutY2.steps = cycles

        animates |= AnimationType.texture
    }

    func zoomTextureOut(cycles: Int = 60) {

        track("Zooming texture out...")

        cutoutX1.target = 0.0
        cutoutY1.target = 0.0
        cutoutX2.target = 1.0
        cutoutY2.target = 1.0

        cutoutX1.steps = cycles
        cutoutY1.steps = cycles
        cutoutX2.steps = cycles
        cutoutY2.steps = cycles

        animates |= AnimationType.texture
    }

    //
    // Geometry animations
    //

    func zoom() {
    
        track("Zooming in...")

        eyeZ.current = 6.0
        angleX.target = 0.0
        angleY.target = 0.0
        angleZ.target = 0.0

        let steps = 120
        angleX.steps = steps
        angleY.steps = steps
        angleZ.steps = steps

        animates |= AnimationType.geometry
    }
    
    func rotateBack() {
    
        track("Rotating back...")

        angleX.target = 0.0
        angleY.target += 90.0
        angleZ.target = 0.0

        let steps = 60
        angleX.steps = steps
        angleY.steps = steps
        angleZ.steps = steps

        angleY.target -= (angleY.target >= 360) ? 360 : 0

        animates |= AnimationType.geometry
    }
    
    func rotate() {
    
        track("Rotating...")

        angleX.target = 0.0
        angleY.target -= 90.0
        angleZ.target = 0.0

        let steps = 60
        angleX.steps = steps
        angleY.steps = steps
        angleZ.steps = steps

        angleY.target += (angleY.target < 0) ? 360 : 0

        animates |= AnimationType.geometry
    }
    
    func scroll() {
        
        track("Scrolling...")

        eyeY.current = -1.5
        angleX.target = 0.0
        angleY.target = 0.0
        angleZ.target = 0.0

        let steps = 120
        angleX.steps = steps
        angleY.steps = steps
        angleZ.steps = steps

        animates |= AnimationType.geometry
    }

    func snapToFront() {

        track("Snapping to front...")

        eyeZ.current = -0.05

        animates |= AnimationType.geometry
    }

    //
    // Alpha channel animations
    //

    func blendIn() {
        
        track("Blending in...")

        angleX.target = 0.0
        angleY.target = 0
        angleZ.target = 0
        alpha.target = 1.0

        let steps = 10
        angleX.steps = steps
        angleY.steps = steps
        angleZ.steps = steps
        alpha.steps = steps

        animates |= AnimationType.alpha
    }

    func blendOut() {
        
        track("Blending out...")

        angleX.target = 0.0
        angleY.target = 0.0
        angleZ.target = 0.0
        alpha.target = 0.0

        let steps = 40
        angleX.steps = steps
        angleY.steps = steps
        angleZ.steps = steps
        alpha.steps = steps

        animates |= AnimationType.alpha
    }

    //
    // Matrix utilities
    //
    
    func matrix_from_perspective(fovY: Float,
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
        m.columns.0 = float4(xscale, 0.0, 0.0, 0.0)
        m.columns.1 = float4(0.0, yscale, 0.0, 0.0)
        m.columns.2 = float4(0.0, 0.0, q, 1.0)
        m.columns.3 = float4(0.0, 0.0, q * -nearZ, 0.0)
    
        return m
    }
    
    func matrix_from_translation(x: Float,
                                 y: Float,
                                 z: Float) -> matrix_float4x4 {
    
        var m = matrix_identity_float4x4
        m.columns.3 = float4(x, y, z, 1.0)
    
        return m
    }
    
    func matrix_from_rotation(radians: Float,
                              x: Float,
                              y: Float,
                              z: Float) -> matrix_float4x4 {
    
        var v = vector_float3(x, y, z)
        v = normalize(v)
        let cos = cosf(radians)
        let cosp = 1.0 - cos
        let sin = sinf(radians)
    
        var m = matrix_float4x4()
        m.columns.0 = float4(cos + cosp * v.x * v.x,
                             cosp * v.x * v.y + v.z * sin,
                             cosp * v.x * v.z - v.y * sin,
                             0.0)
        m.columns.1 = float4(cosp * v.x * v.y - v.z * sin,
                             cos + cosp * v.y * v.y,
                             cosp * v.y * v.z + v.x * sin,
                             0.0)
        m.columns.2 = float4(cosp * v.x * v.z + v.y * sin,
                             cosp * v.y * v.z - v.x * sin,
                             cos + cosp * v.z * v.z,
                             0.0)
        m.columns.3 = float4(0.0,
                             0.0,
                             0.0,
                             1.0)
        return m
    }
}
