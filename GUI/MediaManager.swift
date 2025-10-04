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
    var emu: EmulatorProxy? { return mydocument.emu }
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

    // Shared list of recently inserted floppy disk URLs
    static var insertedFloppyDisks: [URL] = []

    // Unshared list of export URLs (one for each floppy drive)
    var exportedFloppyDisks0: [URL] = []
    var exportedFloppyDisks1: [URL] = []
    var exportedFloppyDisks2: [URL] = []
    var exportedFloppyDisks3: [URL] = []

    // Shared list of recently attached hard drive URLs
    static var attachedHardDrives: [URL] = []

    // Unshared list of export URLs (one for each hard drive)
    var exportedHardDrives0: [URL] = []
    var exportedHardDrives1: [URL] = []
    var exportedHardDrives2: [URL] = []
    var exportedHardDrives3: [URL] = []

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

        initUrlMenus([df0OpenRecent, df1OpenRecent, df2OpenRecent, df3OpenRecent], count: 10,
                     action: #selector(MyController.insertRecentDiskAction(_:)))
        initUrlMenus([df0ExportRecent, df1ExportRecent, df2ExportRecent, df3ExportRecent], count: 1,
                     action: #selector(MyController.exportRecentDiskAction(_:)))
        initUrlMenus([hd0OpenRecent, hd1OpenRecent, hd2OpenRecent, hd3OpenRecent], count: 10,
                     action: #selector(MyController.attachRecentHdrAction(_:)))
        initUrlMenus([hd0ExportRecent, hd1ExportRecent, hd2ExportRecent, hd3ExportRecent], count: 1,
                     action: #selector(MyController.exportRecentHdrAction(_:)))
    }

    func initUrlMenus(_ menus: [NSMenuItem], count: Int,
                      action: Selector?, clearAction: Selector? = nil) {

        for (index, menu) in menus.enumerated() {

            initUrlMenu(menu, count: count, tag: index, action: action, clearAction: clearAction)
        }
    }

    func initUrlMenu(_ menuItem: NSMenuItem, count: Int, tag: Int,
                     action: Selector?, clearAction: Selector? = nil) {

        let menu = menuItem.submenu!
        menu.removeAllItems()

        for index in 0..<count {

            let item = NSMenuItem(title: "\(index)", action: action, keyEquivalent: "")
            item.tag = tag << 16 | index
            menu.addItem(item)
        }

        if let clearAction = clearAction {

            menu.addItem(NSMenuItem.separator())
            menu.addItem(NSMenuItem(title: "Clear Menu", action: clearAction, keyEquivalent: ""))
        }
    }

    //
    // Handling lists of recently used URLs
    //

    static func noteRecentlyUsedURL(_ url: URL, to list: inout [URL], size: Int) {

        if !list.contains(url) {

            // Shorten the list if it is too large
            if list.count == size { list.remove(at: size - 1) }

            // Add new item at the beginning
            list.insert(url, at: 0)
        }
    }

    static func getRecentlyUsedURL(_ pos: Int, from list: [URL]) -> URL? {
        return (pos < list.count) ? list[pos] : nil
    }
    
    static func noteNewRecentlyInsertedDiskURL(_ url: URL) {
        noteRecentlyUsedURL(url, to: &insertedFloppyDisks, size: 10)
    }

    static func getRecentlyInsertedDiskURL(_ pos: Int) -> URL? {
        return insertedFloppyDisks.at(pos)
    }

    static func clearRecentlyInsertedDiskURLs() {
        insertedFloppyDisks = []

    }
    func noteNewRecentlyExportedDiskURL(_ url: URL, df n: Int) {

        switch n {
        case 0: MediaManager.noteRecentlyUsedURL(url, to: &exportedFloppyDisks0, size: 1)
        case 1: MediaManager.noteRecentlyUsedURL(url, to: &exportedFloppyDisks1, size: 1)
        case 2: MediaManager.noteRecentlyUsedURL(url, to: &exportedFloppyDisks2, size: 1)
        case 3: MediaManager.noteRecentlyUsedURL(url, to: &exportedFloppyDisks3, size: 1)
        default: fatalError()
        }
    }

    func getRecentlyExportedDiskURLs(df n: Int) -> [URL] {
        switch n {
        case 0: return exportedFloppyDisks0
        case 1: return exportedFloppyDisks1
        case 2: return exportedFloppyDisks2
        case 3: return exportedFloppyDisks3
        default: fatalError()
        }
    }

    func getRecentlyExportedDiskURL(_ pos: Int, df n: Int) -> URL? {
        return getRecentlyExportedDiskURLs(df: n).at(pos)
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

    static func noteNewRecentlyAttachedHdrURL(_ url: URL) {
        MediaManager.noteRecentlyUsedURL(url, to: &attachedHardDrives, size: 10)
    }

    static func getRecentlyAttachedHdrURL(_ pos: Int) -> URL? {
        return attachedHardDrives.at(pos)
    }

    static func clearRecentlyAttachedHdrURLs() {
        attachedHardDrives = []
    }

    func noteNewRecentlyExportedHdrURL(_ url: URL, hd n: Int) {
        switch n {
        case 0: MediaManager.noteRecentlyUsedURL(url, to: &exportedHardDrives0, size: 1)
        case 1: MediaManager.noteRecentlyUsedURL(url, to: &exportedHardDrives1, size: 1)
        case 2: MediaManager.noteRecentlyUsedURL(url, to: &exportedHardDrives2, size: 1)
        case 3: MediaManager.noteRecentlyUsedURL(url, to: &exportedHardDrives3, size: 1)
        default: fatalError()
        }
    }

    func getRecentlyExportedHdrURLs(hd n: Int) -> [URL] {
        switch n {
        case 0: return exportedHardDrives0
        case 1: return exportedHardDrives1
        case 2: return exportedHardDrives2
        case 3: return exportedHardDrives3
        default: fatalError()
        }
    }

    func getRecentlyExportedHdrURL(_ pos: Int, hd n: Int) -> URL? {
        return getRecentlyExportedHdrURLs(hd: n).at(pos)
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

        case .ADF, .ADZ, .EADF, .DMS, .EXE, .IMG, .ST: MediaManager.noteNewRecentlyInsertedDiskURL(url)
        case .HDF, .HDZ:                               MediaManager.noteNewRecentlyAttachedHdrURL(url)

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

        guard let emu = emu else { return }

        var dfn: FloppyDriveProxy { return emu.df(n)! }

        if force || proceedWithUnsavedFloppyDisk(drive: dfn) {

            try dfn.swap(url: url)
        }

        if remember {

            MediaManager.noteNewRecentlyInsertedDiskURL(url)
            noteNewRecentlyExportedDiskURL(url, df: n)
        }
    }

    func addMedia(hd n: Int, url: URL, force: Bool = false, remember: Bool = true) throws {

        guard let emu = emu else { return }

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

            MediaManager.noteNewRecentlyAttachedHdrURL(url)
            noteNewRecentlyExportedHdrURL(url, hd: n)
        }
    }

    func importFolder(hd n: Int, url: URL, force: Bool = false) throws {

        guard let emu = emu else { return }

        var hdn: HardDriveProxy { return emu.hd(n)! }

        if force || proceedWithUnsavedHardDisk(drive: hdn) {

            try hdn.importFiles(url: url)
        }
    }

    func detach(hd n: Int, force: Bool = false) throws {

        guard let emu = emu else { return }

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

@MainActor
extension MediaManager {

    func installAros() {

        installAros(crc32: vamiga.CRC32_AROS_55696)
    }

    func installAros(crc32: UInt32 = vamiga.CRC32_AROS_20250219) {

        switch crc32 {

        case vamiga.CRC32_AROS_54705:       // Taken from UAE
            installAros(rom: "aros-svn54705-rom", ext: "aros-svn54705-ext")

        case vamiga.CRC32_AROS_55696:       // Taken from SAE
            installAros(rom: "aros-svn55696-rom", ext: "aros-svn55696-ext")

        case vamiga.CRC32_AROS_20250219:    // 2025 version
            installAros(rom: "aros-20250219-rom", ext: "aros-20250219-ext")

        default:
            fatalError()
        }
    }

    func installDiagRom(crc32: UInt32 = vamiga.CRC32_DIAG13) {

        switch crc32 {

        case vamiga.CRC32_DIAG121:
            install(rom: "diagrom-121")

        case vamiga.CRC32_DIAG13:
            install(rom: "diagrom-13")

        default:
            fatalError()
        }
    }

    func installAros(rom: String, ext: String) {

        guard let config = mycontroller.config, let emu = emu else { return }

        // Install both Roms
        install(rom: rom)
        install(ext: ext)

        // Configure the location of the exansion Rom
        config.extStart = 0xE0

        // Make sure the machine has enough Ram to run Aros
        let chip = emu.get(.MEM_CHIP_RAM)
        let slow = emu.get(.MEM_SLOW_RAM)
        let fast = emu.get(.MEM_FAST_RAM)
        if chip + slow + fast < 1024*1024 { config.slowRam = 512 }
    }

    func install(rom: String) {

        let data = NSDataAsset(name: rom)!.data
        try? emu?.mem.loadRom(buffer: data)
    }

    func install(ext: String) {

        let data = NSDataAsset(name: ext)!.data
        try? emu?.mem.loadExt(buffer: data)
    }
}
