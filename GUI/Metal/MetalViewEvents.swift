//
// This file is part of VirtualC64 - A cycle accurate Commodore 64 emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
//


import AppKit
import CoreGraphics

//
// CGEvent handler
//

func myCGEventCallback(proxy: CGEventTapProxy, type: CGEventType, event: CGEvent, refcon: UnsafeMutableRawPointer?) -> Unmanaged<CGEvent>? {
    
    track("Catching CGEvent")
    var keyCode = event.getIntegerValueField(.keyboardEventKeycode)
    var flags = event.flags
    
    let shift = flags.contains(.maskShift)
    let control = flags.contains(.maskControl)
    let option = flags.contains(.maskAlternate)
    let command = flags.contains(.maskCommand)
   
    
    if type == .keyDown {
        track("INTERCEPTED: keyDown \(keyCode)")
    }
    
    if type == .keyUp {
        track("INTERCEPTED keyUp \(keyCode)")
    }
    
    if type == .flagsChanged {
    
        track("INTERCEPTED flagsChanged \(keyCode)")
    }
    
    track("Shift = \(shift)")
    track("Control = \(control)")
    track("option = \(option)")
    track("command = \(command)")
    
    return Unmanaged.passRetained(event)
}

public extension MetalView {

    //
    // Keyboard event interception
    //

    @discardableResult
    func interceptKeyboardEvents() -> Bool {
        
        track("Trying to install a CGEvent interceptor...")

        func acquirePrivileges() {
            let trusted = kAXTrustedCheckOptionPrompt.takeUnretainedValue()
            let privOptions = [trusted: true] as CFDictionary
            let accessEnabled = AXIsProcessTrustedWithOptions(privOptions)
            
            if accessEnabled == true {
                track("Access granted")
            } else {
                track("Access DISABLED")

            }
        }
        
        acquirePrivileges()
        

        let eventMask =
            (1 << CGEventType.keyDown.rawValue) |
            (1 << CGEventType.keyUp.rawValue) |
            (1 << CGEventType.flagsChanged.rawValue)
        
        let eventTap = CGEvent.tapCreate(tap: .cgSessionEventTap,
                                         place: .headInsertEventTap,
                                         options: .defaultTap,
                                         eventsOfInterest: CGEventMask(eventMask),
                                         callback: myCGEventCallback,
                                         userInfo: nil)
        
        if eventTap == nil {
            track("Failed to create event tap. Won't be able to catch CGEvents.")
            return false
        }
        
        track("Success")
        
        let runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0)
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, .commonModes)
        CGEvent.tapEnable(tap: eventTap!, enable: true)
        // CFRunLoopRun()
        
        
        return true
    }
    
    //
    // Keyboard events
    //
    
    /*
    override public func performKeyEquivalent(with event: NSEvent) -> Bool {
        return true
    }
    */
    
    override public func keyDown(with event: NSEvent)
    {
        // track()
        controller.keyboardcontroller.keyDown(with: event)
    }
    
    override public func keyUp(with event: NSEvent)
    {
        controller.keyboardcontroller.keyUp(with: event)
    }
    
    override public func flagsChanged(with event: NSEvent) {
        
        track()
        
        // Save modifier flags. They are needed in TouchBar code
        controller.modifierFlags = event.modifierFlags
        controller.keyboardcontroller.flagsChanged(with: event)
    }
    
    //
    // Mouse events
    //
    
    func scaledMouseCoordinate(with event: NSEvent) -> NSPoint
    {
        // Get coordinate relative to view
        let locationInView = convert(event.locationInWindow, from: nil)
        
        // Scale into range 0..1
        var x = (frame.width == 0) ? 0.0 : (locationInView.x / frame.width)
        var y = (frame.height == 0) ? 0.0 : (locationInView.y / frame.height)

        // Clamp
        x = (x < 0.0) ? 0.0 : (x > 1.0) ? 1.0 : x
        y = (y < 0.0) ? 0.0 : (y > 1.0) ? 1.0 : y
        
        return NSMakePoint(x, y)
    }
    
    /*
    override public func mouseEntered(with event: NSEvent)
    {
        NSCursor.hide();
    }
    
    override public func mouseExited(with event: NSEvent)
    {
        NSCursor.unhide()
    }
    */
    
    override public func mouseDown(with event: NSEvent)
    {
        controller.c64.mouse.setLeftButton(true)
    }
    
    override public func mouseUp(with event: NSEvent)
    {
        controller.c64.mouse.setLeftButton(false)
    }
    
    override public func rightMouseUp(with event: NSEvent)
    {
        controller.c64.mouse.setRightButton(false)
    }
    
    override public func rightMouseDown(with event: NSEvent)
    {
        controller.c64.mouse.setRightButton(true)
    }
    
    override public func mouseMoved(with event: NSEvent) {
        
        let dx = event.deltaX
        let dy = -event.deltaY
        
        controller.mouseXY.x += dx
        controller.mouseXY.y += dy

        // Make coordinate independent of window size
        let scaleX = (256.0 * 400.0) / frame.width
        let scaleY = (256.0 * 300.0) / frame.height
        let newX = controller.mouseXY.x * scaleX
        let newY = controller.mouseXY.y * scaleY

        let newLocation = NSMakePoint(newX, newY)
        controller.c64.mouse.setXY(newLocation)
        //track("\(dx) \(dy)\n");
    }
    
    override public func mouseDragged(with event: NSEvent)
    {
        mouseMoved(with: event)
    }
    
    override public func rightMouseDragged(with event: NSEvent)
    {
        mouseMoved(with: event)
    }
}
