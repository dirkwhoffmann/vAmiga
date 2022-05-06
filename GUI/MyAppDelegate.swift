// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Cocoa

var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }

@NSApplicationMain
@objc public class MyAppDelegate: NSObject, NSApplicationDelegate {
    
    @IBOutlet weak var df0Menu: NSMenuItem!
    @IBOutlet weak var df1Menu: NSMenuItem!
    @IBOutlet weak var df2Menu: NSMenuItem!
    @IBOutlet weak var df3Menu: NSMenuItem!
    
    @IBOutlet weak var hd0Menu: NSMenuItem!
    @IBOutlet weak var hd1Menu: NSMenuItem!
    @IBOutlet weak var hd2Menu: NSMenuItem!
    @IBOutlet weak var hd3Menu: NSMenuItem!

    // Replace the old document controller by instantiating a custom controller
    let myDocumentController = MyDocumentController()

    /* An event tap for interception CGEvents. CGEvents are intercepted to
     * establish a direct mapping of the Command keys to the Amiga keys. To
     * make such a mapping work, we have to disable all keyboard shortcuts,
     * even the system-wide ones.
     */
    var eventTap: CFMachPort?

    // Preferences
    var pref: Preferences!
    var prefController: PreferencesController?
    
    // Information provider for connected HID devices
    var database = DeviceDatabase()
    
    // List of recently inserted floppy disks (all drives share the same list)
    var insertedFloppyDisks: [URL] = []
    
    // List of recently exported floppy disks (one list for each drive)
    var exportedFloppyDisks: [[URL]] = [[URL]](repeating: [URL](), count: 4)
    
    // List of recently attached hard drive URLs
    var attachedHardDrives: [URL] = []
    
    // List of recently exported hard drive URLs (one list for each drive)
    var exportedHardDrives: [[URL]] = [[URL]](repeating: [URL](), count: 4)

    override init() {
                
        super.init()
        pref = Preferences()
    }
    
    public func applicationDidFinishLaunching(_ aNotification: Notification) {
                        
        debug(.lifetime)
    }
    
    public func applicationWillTerminate(_ aNotification: Notification) {

        debug(.lifetime)
    }
    
    //
    // Handling lists of recently used URLs
    //
    
    private func noteRecentlyUsedURL(_ url: URL, to list: inout [URL], size: Int) {
        
        if !list.contains(url) {

            // Shorten the list if it is too large
            if list.count == size { list.remove(at: size - 1) }
            
            // Add new item at the beginning
            list.insert(url, at: 0)
        }
    }
    
    func getRecentlyUsedURL(_ pos: Int, from list: [URL]) -> URL? {
        return (pos < list.count) ? list[pos] : nil
    }
    
    func noteNewRecentlyInsertedDiskURL(_ url: URL) {
        noteRecentlyUsedURL(url, to: &insertedFloppyDisks, size: 10)
    }
    
    func getRecentlyInsertedDiskURL(_ pos: Int) -> URL? {
        return getRecentlyUsedURL(pos, from: insertedFloppyDisks)
    }
    
    func clearRecentlyInsertedDiskURLs() {
        insertedFloppyDisks = []
    }
    func noteNewRecentlyExportedDiskURL(_ url: URL, df n: Int) {
        noteRecentlyUsedURL(url, to: &exportedFloppyDisks[n], size: 1)
    }
    
    func getRecentlyExportedDiskURL(_ pos: Int, df n: Int) -> URL? {
        return getRecentlyUsedURL(pos, from: exportedFloppyDisks[n])
    }
    
    func clearRecentlyExportedDiskURLs(df n: Int) {
        exportedFloppyDisks[n] = [URL]()
    }
    
    func noteNewRecentlyAttachedHdrURL(_ url: URL) {
        noteRecentlyUsedURL(url, to: &attachedHardDrives, size: 10)
    }
    
    func getRecentlyAttachedHdrURL(_ pos: Int) -> URL? {
        return getRecentlyUsedURL(pos, from: attachedHardDrives)
    }
    
    func clearRecentlyAttachedHdrURLs() {
        attachedHardDrives = []
    }
    
    func noteNewRecentlyExportedHdrURL(_ url: URL, hd n: Int) {
        noteRecentlyUsedURL(url, to: &exportedHardDrives[n], size: 1)
    }
    
    func getRecentlyExportedHdrURL(_ pos: Int, hd n: Int) -> URL? {
        return getRecentlyUsedURL(pos, from: exportedHardDrives[n])
    }
    
    func clearRecentlyExportedHdrURLs(hd n: Int) {
        exportedHardDrives[n] = []
    }
}

//
// Personal delegation methods
//

extension MyAppDelegate {
    
    var documents: [MyDocument] {
        return NSDocumentController.shared.documents as? [MyDocument] ?? []
    }
    var windows: [NSWindow] {
        return documents.compactMap({ $0.windowForSheet })
    }
    var controllers: [MyController] {
        return documents.compactMap({ $0.windowForSheet?.windowController as? MyController })
    }
    var proxies: [AmigaProxy] {
        return documents.map({ $0.amiga })
    }
    
    func windowDidBecomeMain(_ window: NSWindow) {
        
        for c in controllers {
            
            if c.window == window {
                
                // Start playback
                if !c.macAudio!.isRunning {
                    c.macAudio!.startPlayback()
                    if !c.amiga.warpMode { c.amiga.paula.rampUpFromZero() }
                }
                
                // Update the visibility of all drive menus
                c.hideOrShowDriveMenus()

            } else {
                
                // Stop playback
                if c.macAudio!.isRunning {
                    c.macAudio!.stopPlayback()
                    c.amiga.paula.rampDown()
                }
            }
        }
    }
    
    // Callen when a HID device has been added
    func deviceAdded() {
        prefController?.refresh()
    }
    
    // Callen when a HID device has been removed
    func deviceRemoved() {
        prefController?.refresh()
    }

    // Callen when a HID device has been pulled
    func devicePulled(events: [GamePadAction]) {
        prefController?.refreshDeviceEvents(events: events)
    }

    // Use this variable to switch direct mapping of the Command keys on or off
    var mapCommandKeys: Bool {
        
        get {
            return eventTap != nil
        }
        
        set {
            if newValue == false && eventTap != nil {
                
                debug(.events, "Reenabling keyboard shortcuts...")
                CGEvent.tapEnable(tap: eventTap!, enable: false)
                eventTap = nil
            }
            
            if newValue == true && eventTap == nil {
                
                debug(.events, "Trying to disable keyboard shortcuts...")
                
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
                    
                    warn("Aborting. Access denied")
                    return
                }
                
                // Set up an event mask that matches keyDown and keyUp events
                let mask = CGEventMask(
                    (1 << CGEventType.keyDown.rawValue) |
                        (1 << CGEventType.keyUp.rawValue) |
                        (1 << CGEventType.mouseMoved.rawValue))
                
                // Try to create the event tap
                eventTap = CGEvent.tapCreate(tap: .cgSessionEventTap,
                                             place: .headInsertEventTap,
                                             options: .defaultTap,
                                             eventsOfInterest: mask,
                                             callback: cgEventCallback,
                                             userInfo: nil)
                
                if eventTap == nil {
                    
                    warn("Aborting. Failed to create the event tap.")
                    return
                }
                
                // Add the event tap to the run loop and enable it
                let runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0)
                CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, .commonModes)
                CGEvent.tapEnable(tap: eventTap!, enable: true)
                debug(.events, "Success")
            }
        }
    }
}

// To establish a direct mapping of the Command keys to the Amiga keys, this
// callback is registered. It intercepts keyDown and keyUp events and filters
// out the Command key modifier flag. As a result, all keyboard shortcuts are
// disabled and all keys that are pressed in combination with the Command key
// will trigger a standard Cocoa key event.
func cgEventCallback(proxy: CGEventTapProxy,
                     type: CGEventType,
                     event: CGEvent,
                     refcon: UnsafeMutableRawPointer?) -> Unmanaged<CGEvent>? {
        
    event.flags.remove(.maskCommand)
    return Unmanaged.passRetained(event)
}
