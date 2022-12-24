// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Cocoa

@objc(MyApplication)
class MyApplication: NSApplication {

    /* Set this variable to true to take away the control of the Command key
     * from the application. This becomes necessary once the command key
     * is meant to be operate the Amiga and not the Mac. If control is not taken
     * away, pressing the Command key in combination with another key will
     * trigger unwanted actions (e.g., Cmd+Q will quit the application).
     *
     * Like for all other secondary keys (Control, Option, etc.), function
     * 'flagsChanged' is invoked when the Command key is pressed or released.
     * However, this method is called too late in the command chain to intercept,
     * i.e., menu actions will already be carried out.
     *
     * The solution taken here is to override function sendEvent in the
     * Application class. This delegation function is called early enough in
     * the command chain to hide any Command key events from the Mac app.
     */
    var disableCmdKey = false

    override func sendEvent(_ event: NSEvent) {

        if disableCmdKey {

            if event.type == NSEvent.EventType.keyUp {

                if event.modifierFlags.contains(.command) {

                    debug(.events, "keyUp: Removing CMD flag")
                    event.cgEvent!.flags.remove(.maskCommand)
                    super.sendEvent(NSEvent(cgEvent: event.cgEvent!)!)
                    return
                }
            }
            if event.type == NSEvent.EventType.keyDown {

                if event.modifierFlags.contains(.command) {

                    debug(.events, "keyDown: Removing CMD flag")
                    event.cgEvent!.flags.remove(.maskCommand)
                    super.sendEvent(NSEvent(cgEvent: event.cgEvent!)!)
                    return
                }
            }
        }
        super.sendEvent(event)
    }
}

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

    // Indicates if the Command keys should be mapped to the Amiga keys
    var mapLeftCmdKey = false
    var mapRightCmdKey = false

    // Indicates if the CapsLock key should control warp mode
    var mapCapsLockWarp = true

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
}

var myApp: MyApplication { return NSApp as! MyApplication }
var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }
