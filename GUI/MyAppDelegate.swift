// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Cocoa

/* The delegate object of this application.
 * This variable is global and can be accessed from anywhere in the Swift code.
 */
var myAppDelegate: MyAppDelegate {
    get {
        return NSApp.delegate as! MyAppDelegate
    }
}

/* The document of the currently active emulator instance.
 * This variable is global and can be accessed from anywhere in the Swift code.
 */
var myDocument: MyDocument? {
    get {
        if let doc = NSApplication.shared.orderedDocuments.first as? MyDocument {
            return doc
        } else {
            track("No document object found. Returning nil.")
            return nil
        }
    }
}

/* The window controller of the currently active emulator instance.
 * This variable is global and can be accessed from anywhere in the Swift code.
 */
var myController: MyController? {
    get {
        return myDocument?.windowControllers.first as? MyController
    }
}

/* The window of the currently active emulator instance.
 * This variable is global and can be accessed from anywhere in the Swift code.
 */
var myWindow: NSWindow? {
    get {
        return myController?.window
    }
}

/* The Amiga proxy of the currently active emulator instance.
 * This variable is global and can be accessed from anywhere in the Swift code.
 */
var amigaProxy: AmigaProxy? {
    get {
        return myDocument?.amiga
    }
}



/* An event tap for interception CGEvents
 * CGEvents are intercepted to establish a direct mapping of the Command keys
 * to the Amiga keys. To make such a mapping work, we have to disable all
 * keyboard shortcuts, even the system-wide ones.
 */
var eventTap : CFMachPort?

// Use this variable to switch direct mapping of the Command keys on or off
var mapCommandKeys : Bool {
    
    get {
        return eventTap != nil
    }
    
    set {
        if newValue == false && eventTap != nil {
            
            track("Reenabling keyboard shortcuts...")
            CGEvent.tapEnable(tap: eventTap!, enable: false)
            eventTap = nil
        }
        
        if newValue == true && eventTap == nil {
            
            track("Trying to disable keyboard shortcuts...")
            
            /* To disable keyboard shortcuts, we are going to filter out the
             * Command flag from all keyUp and keyDown CGEvents by installing
             * a CGEvent callback. Doing so requires accessability priviledges.
             * Hence, we first check if we have the priviledges and prompt the
             * user if we don't. In this case, the operation will fail and we
             * won't be able to disable shortcuts. In the meantime however, the
             * user has the ability to grant us access in the system
             * preferences. If he trust us, we'll pass this chechpoint in the
             * next function call.
             */
            let trusted = kAXTrustedCheckOptionPrompt.takeUnretainedValue()
            let privOptions = [trusted: true] as CFDictionary
            
            if !AXIsProcessTrustedWithOptions(privOptions) {
                
                track("Aborting. Access denied")
                return
            }
            
            // Set up an event mask that matches keyDown and keyUp events
            let mask = CGEventMask(
                (1 << CGEventType.keyDown.rawValue) | (1 << CGEventType.keyUp.rawValue))
            
            // Try to create the event tap
            eventTap = CGEvent.tapCreate(tap: .cgSessionEventTap,
                                         place: .headInsertEventTap,
                                         options: .defaultTap,
                                         eventsOfInterest: mask,
                                         callback: cgEventCallback,
                                         userInfo: nil)
            
            if eventTap == nil {
                
                track("Aborting. Failed to create the event tap.")
                return
            }
            
            // Add the event tap to the run loop and enable it
            let runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0)
            CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, .commonModes)
            CGEvent.tapEnable(tap: eventTap!, enable: true)
            track("Success")
        }
    }
}

/* To establish a direct mapping of the Command keys to the Amiga keys, this
 * callback is registered. It intercepts keyDown and keyUp events and filters
 * out the Command key modifier flag. As a result, all keyboard shortcuts are
 * disabled and all keys that are pressed in combination with the Command key
 * will trigger a standard Cocoa key event.
 */
func cgEventCallback(proxy: CGEventTapProxy,
                     type: CGEventType,
                     event: CGEvent,
                     refcon: UnsafeMutableRawPointer?) -> Unmanaged<CGEvent>? {
    
    event.flags.remove(.maskCommand)
    return Unmanaged.passRetained(event)
}


//
// Application delegate
//

@NSApplicationMain
@objc public class MyAppDelegate: NSObject, NSApplicationDelegate {
    
    @IBOutlet weak var df0Menu: NSMenuItem!
    @IBOutlet weak var df1Menu: NSMenuItem!
    @IBOutlet weak var df2Menu: NSMenuItem!
    @IBOutlet weak var df3Menu: NSMenuItem!

    // Inspector (opened as a separate window)
    var inspector: Inspector? = nil
    
    // Virtual keyboard (opened as a separate window)
    var virtualKeyboard: VirtualKeyboardController? = nil
    
    // The list of recently inserted disk URLs.
    var recentlyInsertedDiskURLs: [URL] = []
    
    // The list of recently exported disk URLs.
    var recentlyExportedDisk0URLs: [URL] = []
    var recentlyExportedDisk1URLs: [URL] = []
    var recentlyExportedDisk2URLs: [URL] = []
    var recentlyExportedDisk3URLs: [URL] = []
    
    public func applicationDidFinishLaunching(_ aNotification: Notification) {
        
        track()
        
        // Make touch bar customizable
        if #available(OSX 10.12.2, *) {
            NSApplication.shared.isAutomaticCustomizeTouchBarMenuItemEnabled = true
        }
        
        //
        // Add observers (for remote controlling the app)
        //
        
        let dc = DistributedNotificationCenter.default
        dc.addObserver(self, selector: #selector(vamResetCommand(_:)),
                       name: Notification.Name("VAMReset"),
                       object: nil)
        dc.addObserver(self, selector: #selector(vamConfigureCommand(_:)),
                       name: Notification.Name("VAMConfigure"),
                       object: nil)
        dc.addObserver(self, selector: #selector(vamMountCommand(_:)),
                       name: Notification.Name("VAMMount"),
                       object: nil)
        dc.addObserver(self, selector: #selector(vamTypeTextCommand(_:)),
                       name: Notification.Name("VAMTypeText"),
                       object: nil)
        dc.addObserver(self, selector: #selector(vamTakeScreenshotCommand(_:)),
                       name: Notification.Name("VAMTakeScreenshot"),
                       object: nil)
        dc.addObserver(self, selector: #selector(vamQuitCommand(_:)),
                       name: Notification.Name("VAMQuit"),
                       object: nil)
    }
    
    public func applicationWillTerminate(_ aNotification: Notification) {

        // Close auxiliary windows
        inspector?.close()
        virtualKeyboard?.close()
        
        track()
    }
    
    //
    // Handling the lists of recently used URLs
    //
    
    func noteRecentlyUsedURL(_ url: URL, to list: inout [URL], size: Int) {
        if !list.contains(url) {
            if list.count == size {
                list.remove(at: size - 1)
            }
            list.insert(url, at: 0)
        }
    }
    
    func getRecentlyUsedURL(_ pos: Int, from list: [URL]) -> URL? {
        return (pos < list.count) ? list[pos] : nil
    }
    
    func noteNewRecentlyInsertedDiskURL(_ url: URL) {
        noteRecentlyUsedURL(url, to: &recentlyInsertedDiskURLs, size: 10)
    }
    
    func getRecentlyInsertedDiskURL(_ pos: Int) -> URL? {
        return getRecentlyUsedURL(pos, from: recentlyInsertedDiskURLs)
    }
    
    func noteNewRecentlyExportedDiskURL(_ url: URL, drive nr: Int) {
        
        switch(nr) {
            
        case 0: noteRecentlyUsedURL(url, to: &recentlyExportedDisk0URLs, size: 1)
        case 1: noteRecentlyUsedURL(url, to: &recentlyExportedDisk1URLs, size: 1)
            
        default: fatalError()
        }
    }
    
    func getRecentlyExportedDiskURL(_ pos: Int, drive nr: Int) -> URL? {
        
        switch(nr) {
            
        case 0: return getRecentlyUsedURL(pos, from: recentlyExportedDisk0URLs)
        case 1: return getRecentlyUsedURL(pos, from: recentlyExportedDisk1URLs)
            
        default: fatalError()
        }
    }
    
    func clearRecentlyExportedDiskURLs(drive nr: Int) {
        
        switch(nr) {
            
        case 0: recentlyExportedDisk0URLs = []
        case 1: recentlyExportedDisk1URLs = []
        case 2: recentlyExportedDisk2URLs = []
        case 3: recentlyExportedDisk3URLs = []
        default: fatalError()
        }
    }
    
    func noteNewRecentlyUsedURL(_ url: URL) {
        
        switch (url.pathExtension.uppercased()) {
            
        case "ADF":
            noteNewRecentlyInsertedDiskURL(url)
            
        default:
            break
        }
    }
}


//
// Personal delegation methods
//

extension MyAppDelegate {
    
    func windowDidBecomeMain(_ window: NSWindow) {
        
        // Iterate through all controllers
        for case let document as MyDocument in NSApplication.shared.orderedDocuments {
            if let controller = document.windowControllers.first as? MyController {
                
                let audioEngine = controller.audioEngine!
                if window == controller.window {
                    
                    // Turn on audio
                    // track("Turning on audio for window \(controller.window)")
                    if !audioEngine.isRunning {
                        audioEngine.paula.rampUpFromZero()
                        audioEngine.startPlayback()
                    }

                } else {
                    
                    // Turn off audio
                    // track("Turning off audio for window \(controller.window)")
                    if audioEngine.isRunning {
                        audioEngine.stopPlayback()
                    }
                }
            }
        }
    }
}
