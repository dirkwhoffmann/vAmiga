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
    
    // Indicates if the CapsLock key should control warp mode
    var mapCapsLockWarp = true
    
    // Preferences
    var pref: Preferences!
    var settingsController: SettingsWindowController?
    
    // Information provider for connected HID devices
    var database = DeviceDatabase()
    
    // Command line arguments
    var argv: [String] = []
    
    // User activity token obtained in applicationDidFinishLaunching()
    var token: NSObjectProtocol!
    
    override init() {
        
        super.init()
        pref = Preferences()
    }
    
    public func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {
        
        return true
    }
    
    public func application(_ application: NSApplication, open urls: [URL]) {
        
        loginfo(.lifetime, "application(open urls: \(urls))")
    }
    
    public func applicationDidFinishLaunching(_ aNotification: Notification) {
        
        token = ProcessInfo.processInfo.beginActivity(options: [ .idleSystemSleepDisabled, .suddenTerminationDisabled ], reason: "Running an emulator")
        argv = Array(CommandLine.arguments.dropFirst())
        
        loginfo(.lifetime, "Launched with arguments \(argv)")
    }
    
    public func applicationShouldTerminate(_ sender: NSApplication) -> NSApplication.TerminateReply {
        
        loginfo(.shutdown, "Delay a bit to let audio fade out...")
        usleep(250000)
        loginfo(.shutdown, "OK...")
        
        return .terminateNow
    }
    
    public func applicationWillTerminate(_ aNotification: Notification) {
        
        loginfo(.lifetime)
        ProcessInfo.processInfo.endActivity(token)
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
    var proxies: [EmulatorProxy?] {
        return documents.map({ $0.emu })
    }
    
    static var currentController: MyController? {
        didSet {
            if currentController !== oldValue {
                oldValue?.emu?.put(.FOCUS, value: 0)
                currentController?.emu?.put(.FOCUS, value: 1)
            }
        }
    }
    
    // Callen when a HID device has been added
    func deviceAdded() {
        settingsController?.refresh()
    }
    
    // Callen when a HID device has been removed
    func deviceRemoved() {
        settingsController?.refresh()
    }
}

@MainActor var myApp: MyApplication { return NSApp as! MyApplication }
@MainActor var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }
