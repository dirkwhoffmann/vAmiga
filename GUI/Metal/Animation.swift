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
    func animates() -> Bool {

        return angleX.animates()
            || angleY.animates()
            || angleZ.animates()
            || eyeX.animates()
            || eyeY.animates()
            || eyeZ.animates()
            || alpha.animates()
    }

    func textureAnimates() -> Bool {

        return cutoutX1.animates()
            || cutoutY1.animates()
            // currentTexOriginX != targetTexOriginX ||
            // currentTexOriginY != targetTexOriginY ||
            || currentTexWidth != targetTexWidth
            || currentTexHeight != targetTexHeight
    }
    
    func getEyeX() -> Float {
        
        return eyeX.current // currentEyeX
    }
    
    func setEyeX(_ newX: Float) {

        eyeX.set(newX)
        self.buildMatrices3D()
    }
    
    func getEyeY() -> Float {
        
        return eyeY.current // currentEyeY
    }
    
    func setEyeY(_ newY: Float) {

        eyeY.set(newY)
        self.buildMatrices3D()
    }
    
    func getEyeZ() -> Float {
        
        return eyeZ.current // currentEyeZ
    }
    
    func setEyeZ(_ newZ: Float) {

        eyeZ.set(newZ)
        self.buildMatrices3D()
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
        /*
        if abs(currentTexOriginX - targetTexOriginX) < abs(deltaTexOriginX) {
            currentTexOriginX = targetTexOriginX
        } else {
            currentTexOriginX += deltaTexOriginX
        }

        if abs(currentTexOriginY - targetTexOriginY) < abs(deltaTexOriginY) {
            currentTexOriginY = targetTexOriginY
        } else {
            currentTexOriginY += deltaTexOriginY
        }
        */

        if abs(currentTexWidth - targetTexWidth) < abs(deltaTexWidth) {
            currentTexWidth = targetTexWidth
        } else {
            currentTexWidth += deltaTexWidth
        }

        if abs(currentTexHeight - targetTexHeight) < abs(deltaTexHeight) {
            currentTexHeight = targetTexHeight
        } else {
            currentTexHeight += deltaTexHeight
        }
/*
        textureRect.origin.x = currentTexOriginX
        textureRect.origin.y = currentTexOriginY
        textureRect.size.width = currentTexWidth
        textureRect.size.height = currentTexHeight
*/
        updateScreenGeometry()
    }

    func computeTextureDeltaSteps(animationCycles: Int) {

        let cycles = Float(animationCycles)
        // deltaTexOriginX = (targetTexOriginX - currentTexOriginX) / cycles
        // deltaTexOriginY = (targetTexOriginY - currentTexOriginY) / cycles
        deltaTexWidth = (targetTexWidth - currentTexWidth) / cycles
        deltaTexHeight = (targetTexHeight - currentTexHeight) / cycles
    }

    func zoomTextureIn(cycles: Int = 60) {

        track("Zooming texture in...")

        cutoutX1.target = 0.0
        cutoutY1.target = 0.0
        // targetTexOriginX = 0.0
        // targetTexOriginY = 0.0
        targetTexWidth = 728.0 / 768.0
        targetTexHeight = 286.0 / 288.0

        self.computeTextureDeltaSteps(animationCycles: cycles)
    }

    func zoomTextureOut(cycles: Int = 60) {

        track("Zooming texture out...")

        cutoutX1.target = 0.0
        cutoutY1.target = 0.0
        // targetTexOriginX = 0.0
        // targetTexOriginY = 0.0
        targetTexWidth = 1.0
        targetTexHeight = 1.0

        self.computeTextureDeltaSteps(animationCycles: cycles)
    }

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
        // targetYAngle -= (targetYAngle >= 360) ? 360 : 0
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
        // targetYAngle += (targetYAngle < 0) ? 360 : 0
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
    }

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
    }
    
    func snapToFront() {
        
        track("Snapping to front...")

        eyeZ.current = -0.05
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
