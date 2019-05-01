// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import AppKit

public extension MetalView {

    //
    // Keyboard events
    //
    
    override func keyDown(with event: NSEvent)
    {
        controller.keyboardcontroller.keyDown(with: event)
    }
    
    override func keyUp(with event: NSEvent)
    {
        controller.keyboardcontroller.keyUp(with: event)
    }
    
    override func flagsChanged(with event: NSEvent) {
        
        controller.keyboardcontroller.flagsChanged(with: event)
    }
    
    
    //
    // Mouse events
    //
    
    func retainMouse() {
        
        NSCursor.hide()
        CGAssociateMouseAndMouseCursorPosition(boolean_t(truncating: false))
        gotMouse = true
    }
    
    func releaseMouse() {
        
        NSCursor.unhide()
        CGAssociateMouseAndMouseCursorPosition(boolean_t(truncating: true))
        gotMouse = false
    }
    
    override func updateTrackingAreas() {

        track()
        
        let options : NSTrackingArea.Options = [ .activeInKeyWindow, .mouseEnteredAndExited ]
        
        if trackingArea != nil {
            removeTrackingArea(trackingArea!)
        }
        trackingArea = NSTrackingArea(rect: self.bounds, options: options, owner: self, userInfo: nil)
        addTrackingArea(trackingArea!)
    }
    
    override func mouseEntered(with event: NSEvent)
    {
        track()
        if controller.autoMouseSharing {
            
            // Only retain the mouse if the last shake is some time ago
            let dt = DispatchTime.now().uptimeNanoseconds - lastShake.uptimeNanoseconds
            if dt / 1_00_000_000 > 5 {
                retainMouse()
            } else {
                track("Last shake too recent")
            }
        }
    }
    
    override func mouseExited(with event: NSEvent)
    {
        track()
    }
    
    override func mouseDown(with event: NSEvent)
    {
        if gotMouse {
            amigaProxy?.mouse.setLeftButton(true)
        } else {
            retainMouse()
        }
    }
    
    override func mouseUp(with event: NSEvent)
    {
        if gotMouse {
            amigaProxy?.mouse.setLeftButton(false)
        }
    }
    
    override func rightMouseDown(with event: NSEvent)
    {
        if gotMouse {
            amigaProxy?.mouse.setRightButton(true)
        }
    }

    override func rightMouseUp(with event: NSEvent)
    {
        if gotMouse {
            amigaProxy?.mouse.setRightButton(false)
        }
    }
    
    override func mouseMoved(with event: NSEvent) {
        
        // Only proceed if the Amiga has access to the mouse
        // if !gotMouse { return }
        
        // Determine delta movement steps
        let dx = event.deltaX
        let dy = event.deltaY
        
        // Check for a shaking mouse movement if auto sharing is on
        if controller.autoMouseSharing {
            
            dxsum = (0.8 * dxsum) + dx
            dxabssum = (0.8 * dxabssum) + abs(dx)
            // track("\(dx) \(dy) \(dxsum) \(dxabssum)")
     
            if dxabssum - abs(dxsum) > 100 {
                track("Mouse shake detected")
                lastShake = DispatchTime.now()
                releaseMouse()
            }
        }
        
        controller.mouseXY.x += dx
        controller.mouseXY.y += dy

        // Make coordinate independent of window size
        let scaleX = (256.0 * 400.0) / frame.width
        let scaleY = (256.0 * 300.0) / frame.height
        let newX = controller.mouseXY.x * scaleX
        let newY = controller.mouseXY.y * scaleY

        let newLocation = NSMakePoint(newX, newY)
        
        if gotMouse {
            amigaProxy?.mouse.setXY(newLocation)
        }
        //track("\(dx) \(dy)\n");
    }
    
    override func mouseDragged(with event: NSEvent)
    {
        mouseMoved(with: event)
    }
    
    override func rightMouseDragged(with event: NSEvent)
    {
        mouseMoved(with: event)
    }
}
