// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
class MediaManager {
    
    struct Option: OptionSet {
        
        let rawValue: Int
        
        static let force       = Option(rawValue: 1 << 0)
        static let remember    = Option(rawValue: 1 << 1)
        static let protect     = Option(rawValue: 1 << 2)
    }
    
    // References to other objects
    var mydocument: MyDocument!
    var emu: EmulatorProxy! { return mydocument.emu }
    var mm: MediaManager! { return mydocument.mm }
    var mycontroller: MyController { return mydocument.controller }
    var console: Console { return mycontroller.renderer.console }

    // References to menu outlets
    var df0OpenRecent: NSMenuItem! { return myAppDelegate.df0OpenRecent }
    var df1OpenRecent: NSMenuItem! { return myAppDelegate.df1OpenRecent }
    var df2OpenRecent: NSMenuItem! { return myAppDelegate.df2OpenRecent }
    var df3OpenRecent: NSMenuItem! { return myAppDelegate.df3OpenRecent }

    var df0ExportRecent: NSMenuItem! { return myAppDelegate.df0ExportRecent }
    var df1ExportRecent: NSMenuItem! { return myAppDelegate.df1ExportRecent }
    var df2ExportRecent: NSMenuItem! { return myAppDelegate.df2ExportRecent }
    var df3ExportRecent: NSMenuItem! { return myAppDelegate.df3ExportRecent }
    
    var hd0OpenRecent: NSMenuItem! { return myAppDelegate.hd0OpenRecent }
    var hd1OpenRecent: NSMenuItem! { return myAppDelegate.hd1OpenRecent }
    var hd2OpenRecent: NSMenuItem! { return myAppDelegate.hd2OpenRecent }
    var hd3OpenRecent: NSMenuItem! { return myAppDelegate.hd3OpenRecent }

    var hd0ExportRecent: NSMenuItem! { return myAppDelegate.hd0ExportRecent }
    var hd1ExportRecent: NSMenuItem! { return myAppDelegate.hd1ExportRecent }
    var hd2ExportRecent: NSMenuItem! { return myAppDelegate.hd2ExportRecent }
    var hd3ExportRecent: NSMenuItem! { return myAppDelegate.hd3ExportRecent }
    
    // List of recently inserted floppy disk URLs (all drives share the same list)
    var insertedFloppyDisks: [URL] = [] {
        didSet {
            updateRecentUrlMenus([df0OpenRecent, df1OpenRecent, df2OpenRecent, df3OpenRecent],
                                 urls: insertedFloppyDisks,
                                 action: #selector(MyController.insertRecentDiskAction(_ :)))
        }
    }
    // List of export URLs  (one for each floppy drive)
    var exportedFloppyDisks0: [URL] = [] {
        didSet { updateRecentUrlMenu(df0ExportRecent, urls: exportedFloppyDisks0, nr: 0,
                                     action: #selector(MyController.exportRecentDiskAction(_:))) }
    }
    var exportedFloppyDisks1: [URL] = [] {
        didSet { updateRecentUrlMenu(df1ExportRecent, urls: exportedFloppyDisks1, nr: 1,
                                     action: #selector(MyController.exportRecentDiskAction(_:))) }
    }
    var exportedFloppyDisks2: [URL] = [] {
        didSet { updateRecentUrlMenu(df2ExportRecent, urls: exportedFloppyDisks2, nr: 2,
                                     action: #selector(MyController.exportRecentDiskAction(_:))) }
    }
    var exportedFloppyDisks3: [URL] = [] {
        didSet { updateRecentUrlMenu(df3ExportRecent, urls: exportedFloppyDisks3, nr: 3,
                                     action: #selector(MyController.exportRecentDiskAction(_:))) }
    }
        
    // List of recently attached hard drive URLs
    var attachedHardDrives: [URL] = [] {
        didSet {
            updateRecentUrlMenus([hd0OpenRecent, hd1OpenRecent, hd2OpenRecent, hd3OpenRecent],
                                 urls: attachedHardDrives,
                                 action: #selector(MyController.insertRecentDiskAction(_ :)))
        }
    }
    // List of export URLs (one for each hard drive)
    var exportedHardDrives0: [URL] = [] {
        didSet { updateRecentUrlMenu(hd0ExportRecent, urls: exportedHardDrives0, nr: 0,
                                     action: #selector(MyController.exportRecentHdrAction(_ :))) }
    }
    var exportedHardDrives1: [URL] = [] {
        didSet { updateRecentUrlMenu(hd1ExportRecent, urls: exportedHardDrives1, nr: 1,
                                     action: #selector(MyController.exportRecentHdrAction(_ :))) }
    }
    var exportedHardDrives2: [URL] = [] {
        didSet { updateRecentUrlMenu(hd2ExportRecent, urls: exportedHardDrives2, nr: 2,
                                     action: #selector(MyController.exportRecentHdrAction(_ :))) }
    }
    var exportedHardDrives3: [URL] = [] {
        didSet { updateRecentUrlMenu(hd3ExportRecent, urls: exportedHardDrives3, nr: 3,
                                     action: #selector(MyController.exportRecentHdrAction(_ :))) }
    }
    
    // Pictograms used in menu items
    var diskMenuImage = NSImage(named: "diskTemplate")!.resize(width: 16.0, height: 16.0)
    var hdrMenuImage = NSImage(named: "hdrTemplate")!.resize(width: 16.0, height: 16.0)

    //
    // Initializing
    //

    init(with document: MyDocument) {

        debug(.lifetime, "Creating media manager")
        self.mydocument = document
        
        diskMenuImage.isTemplate = true
        hdrMenuImage.isTemplate = true
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
            
        case .ADF, .ADZ, .EADF, .DMS, .EXE, .IMG, .ST: noteNewRecentlyInsertedDiskURL(url)
        case .HDF, .HDZ:                               noteNewRecentlyAttachedHdrURL(url)
            
        default:
            break
        }
    }

    func noteNewRecentlyExportedURL(_ url: URL, nr: Int, type: FileType) {
        
        switch type {
            
        case .ADF, .ADZ, .EADF, .DMS, .EXE, .IMG, .ST: noteNewRecentlyExportedDiskURL(url, df: nr)
        case .HDF, .HDZ:                               noteNewRecentlyExportedHdrURL(url, hd: nr)
            
        default:
            break
        }
    }

    func updateRecentUrlMenu(_ menuItem: NSMenuItem, urls: [URL], nr: Int, action selector: Selector?) {

        let menu = menuItem.submenu!
        menu.removeAllItems()
        
        if !urls.isEmpty {

            for (index, url) in urls.enumerated() {
                
                let isHdf = url.pathExtension == "hdf" || url.pathExtension == "hdz"
                
                let item = NSMenuItem(title:  url.lastPathComponent, action: selector, keyEquivalent: "")
                item.tag = nr << 16 | index
                item.image = isHdf ? hdrMenuImage : diskMenuImage
                menu.addItem(item)
            }

            /*
            if clearMenu {
                
                menu.addItem(NSMenuItem.separator())
                menu.addItem(NSMenuItem(title: "Clear Menu",
                                        action: #selector(MyController.clearRecentlyInsertedDisksAction(_ :)),
                                        keyEquivalent: ""))
            }
            */
        }
    }
    
    func updateRecentUrlMenus(_ menus: [NSMenuItem], urls: [URL], action selector: Selector?) {

        for (index, menu) in menus.enumerated() {
     
            updateRecentUrlMenu(menu, urls: urls, nr: index, action: selector)
        }
    }

    //
    // Adding media files
    //
    
    func addMedia(url: URL,
                  allowedTypes types: [FileType] = FileType.all,
                  drive: Int = 0,
                  force: Bool = false,
                  remember: Bool = true) throws {
        
        let type = MediaFileProxy.type(of: url)
        if !types.contains(type) {
            
            throw AppError(.FILE_TYPE_MISMATCH,
                            "The type of this file is not known to the emulator.")
        }
        
        switch type {

        case .WORKSPACE:
            try mydocument.processWorkspaceFile(url: url)

        case .SNAPSHOT:
            try mydocument.processSnapshotFile(url: url)

        case .SCRIPT:
            try mydocument.processScriptFile(url: url)
            
        case .HDF, .HDZ:
            try addMedia(hd: drive, url: url, force: force, remember: remember)

        case .ADF, .ADZ, .DMS, .EXE, .EADF, .IMG, .ST, .DIR:
            try addMedia(df: drive, url: url, force: force, remember: remember)

        default:
            break
        }
    }
    
    func addMedia(df n: Int, url: URL, force: Bool = false, remember: Bool = true) throws {

        var dfn: FloppyDriveProxy { return emu.df(n)! }

        if force || proceedWithUnsavedFloppyDisk(drive: dfn) {

            try dfn.swap(url: url)
        }
        
        if remember {
        
            mm.noteNewRecentlyInsertedDiskURL(url)
            mm.noteNewRecentlyExportedDiskURL(url, df: n)
        }
    }
    
    func addMedia(hd n: Int, url: URL, force: Bool = false, remember: Bool = true) throws {

        var hdn: HardDriveProxy { return emu.hd(n)! }

        func attach() throws {

            emu.set(.HDC_CONNECT, drive: n, enable: true)
            try hdn.attach(url: url)
        }

        if force || proceedWithUnsavedHardDisk(drive: hdn) {

            if emu.poweredOff {

                try attach()

            } else if force || askToPowerOff() {

                emu.powerOff()
                try attach()
                emu.powerOn()
                try emu.run()
            }
        }
        
        if remember {
        
            mm.noteNewRecentlyAttachedHdrURL(url)
            mm.noteNewRecentlyExportedHdrURL(url, hd: n)
        }
    }
    
    func importFolder(hd n: Int, url: URL, force: Bool = false) throws {

        var hdn: HardDriveProxy { return emu.hd(n)! }

        if force || proceedWithUnsavedHardDisk(drive: hdn) {

            try hdn.importFiles(url: url)
        }
    }
    
    func detach(hd n: Int, force: Bool = false) throws {
        
        var hdn: HardDriveProxy { return emu.hd(n)! }

        func detach() throws {
                      
            emu.set(.HDC_CONNECT, drive: n, enable: false)
        }
        
        if force || proceedWithUnsavedHardDisk(drive: hdn) {
            
            if emu.poweredOff {
                
                try detach()
                
            } else if force || askToPowerOff() {
                
                emu.powerOff()
                try detach()
                emu.powerOn()
                try emu.run()
            }
        }
    }
}
