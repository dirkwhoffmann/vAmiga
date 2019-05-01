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
        // NSCursor.hide()
    }
    
    override func mouseExited(with event: NSEvent)
    {
        track()
        // NSCursor.unhide()
    }
    
    override func mouseDown(with event: NSEvent)
    {
        amigaProxy?.mouse.setLeftButton(true)
    }
    
    override func mouseUp(with event: NSEvent)
    {
        amigaProxy?.mouse.setLeftButton(false)
    }
    
    override func rightMouseDown(with event: NSEvent)
    {
        amigaProxy?.mouse.setRightButton(true)
    }

    override func rightMouseUp(with event: NSEvent)
    {
        amigaProxy?.mouse.setRightButton(false)
    }
    
    override func mouseMoved(with event: NSEvent) {
        
        let dx = event.deltaX
        let dy = event.deltaY
        
        controller.mouseXY.x += dx
        controller.mouseXY.y += dy

        // Make coordinate independent of window size
        let scaleX = (256.0 * 400.0) / frame.width
        let scaleY = (256.0 * 300.0) / frame.height
        let newX = controller.mouseXY.x * scaleX
        let newY = controller.mouseXY.y * scaleY

        let newLocation = NSMakePoint(newX, newY)
        
        amigaProxy?.mouse.setXY(newLocation)
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
