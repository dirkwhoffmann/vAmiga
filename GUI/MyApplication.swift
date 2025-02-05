// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Cocoa

@MainActor @objc(MyApplication)
class MyApplication: NSApplication {

    /* Set this variable to true to take away the control of the Command key
     * from the application. This becomes necessary once the command key
     * is meant to be operate the Amiga and not the Mac. If control is not taken
     * away, pressing the Command key in combination with another key will
     * trigger unwanted actions (e.g., Cmd+Q will quit the application).
     *
     * Like for all other secondary keys (Control, Option, etc.), function
     * 'flagsChanged' is invoked when the Command key is pressed or released.
     * However, this method is called too late in the command chain to
     * intercept, i.e., menu actions will already be carried out.
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

@MainActor @main @objc
public class MyAppDelegate: NSObject, NSApplicationDelegate {
    
    @IBOutlet weak var df0Menu: NSMenuItem!
    @IBOutlet weak var df1Menu: NSMenuItem!
    @IBOutlet weak var df2Menu: NSMenuItem!
    @IBOutlet weak var df3Menu: NSMenuItem!

    @IBOutlet weak var df0OpenRecent: NSMenuItem!
    @IBOutlet weak var df1OpenRecent: NSMenuItem!
    @IBOutlet weak var df2OpenRecent: NSMenuItem!
    @IBOutlet weak var df3OpenRecent: NSMenuItem!

    @IBOutlet weak var df0ExportRecent: NSMenuItem!
    @IBOutlet weak var df1ExportRecent: NSMenuItem!
    @IBOutlet weak var df2ExportRecent: NSMenuItem!
    @IBOutlet weak var df3ExportRecent: NSMenuItem!
    
    @IBOutlet weak var hd0Menu: NSMenuItem!
    @IBOutlet weak var hd1Menu: NSMenuItem!
    @IBOutlet weak var hd2Menu: NSMenuItem!
    @IBOutlet weak var hd3Menu: NSMenuItem!
    
    @IBOutlet weak var hd0OpenRecent: NSMenuItem!
    @IBOutlet weak var hd1OpenRecent: NSMenuItem!
    @IBOutlet weak var hd2OpenRecent: NSMenuItem!
    @IBOutlet weak var hd3OpenRecent: NSMenuItem!

    @IBOutlet weak var hd0ExportRecent: NSMenuItem!
    @IBOutlet weak var hd1ExportRecent: NSMenuItem!
    @IBOutlet weak var hd2ExportRecent: NSMenuItem!
    @IBOutlet weak var hd3ExportRecent: NSMenuItem!

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
    
    // Command line arguments
    var argv: [String] = []

    // User activity token obtained in applicationDidFinishLaunching()
    var token: NSObjectProtocol!
    
    // List of recently inserted floppy disk URLs (all drives share the same list)
    var insertedFloppyDisks: [URL] = [] {
        didSet {
            updateRecentUrlMenus([df0OpenRecent, df1OpenRecent, df2OpenRecent, df3OpenRecent],
                                 urls: insertedFloppyDisks)
        }
    }
    var exportedFloppyDisks0: [URL] = [] {
        didSet { updateRecentUrlMenu(df0ExportRecent, urls: insertedFloppyDisks, nr: 0) }
    }
    var exportedFloppyDisks1: [URL] = [] {
        didSet { updateRecentUrlMenu(df1ExportRecent, urls: insertedFloppyDisks, nr: 1) }
    }
    var exportedFloppyDisks2: [URL] = [] {
        didSet { updateRecentUrlMenu(df2ExportRecent, urls: insertedFloppyDisks, nr: 2) }
    }
    var exportedFloppyDisks3: [URL] = [] {
        didSet { updateRecentUrlMenu(df3ExportRecent, urls: insertedFloppyDisks, nr: 3) }
    }
    
    // List of recently exported floppy disk URLs (one list for each drive)
    // var exportedFloppyDisks: [[URL]] = [[URL]](repeating: [URL](), count: 4)
    
    // List of recently attached hard drive URLs
    var attachedHardDrives: [URL] = [] {
        didSet {
            updateRecentUrlMenus([hd0OpenRecent, hd1OpenRecent, hd2OpenRecent, hd3OpenRecent],
                                 urls: attachedHardDrives)
        }
    }
    var exportedHardDrives0: [URL] = [] {
        didSet { updateRecentUrlMenu(hd0ExportRecent, urls: exportedHardDrives0, nr: 0) }
    }
    var exportedHardDrives1: [URL] = [] {
        didSet { updateRecentUrlMenu(hd1ExportRecent, urls: exportedHardDrives1, nr: 1) }
    }
    var exportedHardDrives2: [URL] = [] {
        didSet { updateRecentUrlMenu(hd2ExportRecent, urls: exportedHardDrives2, nr: 2) }
    }
    var exportedHardDrives3: [URL] = [] {
        didSet { updateRecentUrlMenu(hd3ExportRecent, urls: exportedHardDrives3, nr: 3) }
    }
    
    // List of recently exported hard drive URLs (one list for each drive)
    // var exportedHardDrives: [[URL]] = [[URL]](repeating: [URL](), count: 4)
    
    // Pictograms used in menu items
    let diskMenuImage = NSImage(named: "diskTemplate")!.resize(width: 16.0, height: 16.0)
    let hdrMenuImage = NSImage(named: "hdrTemplate")!.resize(width: 16.0, height: 16.0)
    
    override init() {
        
        super.init()
        pref = Preferences()
    }
    
    public func applicationDidFinishLaunching(_ aNotification: Notification) {
        
        debug(.lifetime)
        
        token = ProcessInfo.processInfo.beginActivity(options: [ .userInitiated ],
                                                      reason: "Running vAmiga")
        
        argv = Array(CommandLine.arguments.dropFirst())
        
        /*
        insertedFloppyDisks = []
        exportedFloppyDisks0 = []
        exportedFloppyDisks1 = []
        exportedFloppyDisks2 = []
        exportedFloppyDisks3 = []
        
        attachedHardDrives = []
        exportedHardDrives0 = []
        exportedHardDrives1 = []
        exportedHardDrives2 = []
        exportedHardDrives3 = []
        */
    }
    
    public func applicationWillTerminate(_ aNotification: Notification) {

        debug(.lifetime)
        ProcessInfo.processInfo.endActivity(token)
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
    
    /*
    @available(*, deprecated, message: "Use at() instead.")
    func getRecentlyUsedURL(_ pos: Int, from list: [URL]) -> URL? {
        return list.at(pos)
    }
    */
    
    func noteNewRecentlyInsertedDiskURL(_ url: URL) {
        noteRecentlyUsedURL(url, to: &insertedFloppyDisks, size: 10)
    }
    
    func getRecentlyInsertedDiskURL(_ pos: Int) -> URL? {
        return insertedFloppyDisks.at(pos)
    }
    
    func clearRecentlyInsertedDiskURLs() {
        insertedFloppyDisks = []
        
    }
    func noteNewRecentlyExportedDiskURL(_ url: URL, df n: Int) {

        switch n {
        case 0: noteRecentlyUsedURL(url, to: &exportedFloppyDisks0, size: 1)
        case 1: noteRecentlyUsedURL(url, to: &exportedFloppyDisks1, size: 1)
        case 2: noteRecentlyUsedURL(url, to: &exportedFloppyDisks2, size: 1)
        case 3: noteRecentlyUsedURL(url, to: &exportedFloppyDisks3, size: 1)
        default: fatalError()
        }
    }
    
    func getRecentlyExportedDiskURL(_ pos: Int, df n: Int) -> URL? {
        switch n {
        case 0: return exportedFloppyDisks0.at(pos)
        case 1: return exportedFloppyDisks1.at(pos)
        case 2: return exportedFloppyDisks2.at(pos)
        case 3: return exportedFloppyDisks3.at(pos)
        default: fatalError()
        }
    }
    
    func clearRecentlyExportedDiskURLs(df n: Int) {
        switch n {
        case 0: exportedFloppyDisks0 = []
        case 1: exportedFloppyDisks1 = []
        case 2: exportedFloppyDisks2 = []
        case 3: exportedFloppyDisks3 = []
        default: fatalError()
        }
    }
    
    func noteNewRecentlyAttachedHdrURL(_ url: URL) {
        noteRecentlyUsedURL(url, to: &attachedHardDrives, size: 10)
    }
    
    func getRecentlyAttachedHdrURL(_ pos: Int) -> URL? {
        return attachedHardDrives.at(pos)
    }
    
    func clearRecentlyAttachedHdrURLs() {
        attachedHardDrives = []
    }
    
    func noteNewRecentlyExportedHdrURL(_ url: URL, hd n: Int) {
        switch n {
        case 0: noteRecentlyUsedURL(url, to: &exportedHardDrives0, size: 1)
        case 1: noteRecentlyUsedURL(url, to: &exportedHardDrives1, size: 1)
        case 2: noteRecentlyUsedURL(url, to: &exportedHardDrives2, size: 1)
        case 3: noteRecentlyUsedURL(url, to: &exportedHardDrives3, size: 1)
        default: fatalError()
        }
    }
    
    func getRecentlyExportedHdrURL(_ pos: Int, hd n: Int) -> URL? {
        switch n {
        case 0: return exportedHardDrives0.at(pos)
        case 1: return exportedHardDrives1.at(pos)
        case 2: return exportedHardDrives2.at(pos)
        case 3: return exportedHardDrives3.at(pos)
        default: fatalError()
        }
    }
    
    func clearRecentlyExportedHdrURLs(hd n: Int) {
        switch n {
        case 0: exportedHardDrives0 = []
        case 1: exportedHardDrives1 = []
        case 2: exportedHardDrives2 = []
        case 3: exportedHardDrives3 = []
        default: fatalError()
        }
    }
    
    func noteNewRecentlyOpenedURL(_ url: URL, type: FileType) {
        
        switch type {
            
        case .ADF, .EADF, .EXE, .IMG, .ST: noteNewRecentlyInsertedDiskURL(url)
        case .HDF:                         noteNewRecentlyAttachedHdrURL(url)
            
        default:
            break
        }
    }

    func noteNewRecentlyExportedURL(_ url: URL, nr: Int, type: FileType) {
        
        switch type {
            
        case .ADF, .EADF, .EXE, .IMG, .ST: noteNewRecentlyExportedDiskURL(url, df: nr)
        case .HDF:                         noteNewRecentlyExportedHdrURL(url, hd: nr)
            
        default:
            break
        }
    }

    func updateRecentUrlMenu(_ menuItem: NSMenuItem, urls: [URL], nr: Int) {
        
        let menu = menuItem.submenu!
        menu.removeAllItems()
        
        if !urls.isEmpty {

            for (index, url) in urls.enumerated() {
                
                let isHdf = url.pathExtension == "hdf" || url.pathExtension == "hdz"
                
                let item = NSMenuItem(title:  url.lastPathComponent,
                                      action: #selector(MyController.insertRecentDiskAction(_ :)),
                                      keyEquivalent: "")
                
                
                item.tag = nr << 16 | index
                item.image = isHdf ? hdrMenuImage : diskMenuImage
                menu.addItem(item)
            }
            
            menu.addItem(NSMenuItem.separator())
            menu.addItem(NSMenuItem(title: "Clear Menu",
                                    action: #selector(MyController.clearRecentlyInsertedDisksAction(_ :)),
                                    keyEquivalent: ""))
        }
    }
    
    func updateRecentUrlMenus(_ menus: [NSMenuItem], urls: [URL]) {
        
        for (index, menu) in menus.enumerated() {
     
            updateRecentUrlMenu(menu, urls: urls, nr: index)
        }
    }
}

//
// Personal delegation methods
//

@MainActor
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
    var proxies: [EmulatorProxy] {
        return documents.map({ $0.emu })
    }
    
    func windowDidBecomeMain(_ window: NSWindow) {
        
        for c in controllers {
            
            if c.window == window {
                
                c.emu?.put(.FOCUS, value: 1)
                c.hideOrShowDriveMenus()

            } else {
                
                c.emu?.put(.FOCUS, value: 0)
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

    // Callen when a HID event comes in
    func hidEvent(event: HIDEvent, nr: Int, value: Int) {
        prefController?.refreshDeviceEvent(event: event, nr: nr, value: value)
    }

    // Callen when a HID device triggers joystick actions
    func devicePulled(events: [GamePadAction]) {
        prefController?.refreshDeviceActions(actions: events)
    }
}

@MainActor var myApp: MyApplication { return NSApp as! MyApplication }
@MainActor var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }
