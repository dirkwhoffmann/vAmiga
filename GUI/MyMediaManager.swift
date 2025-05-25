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
    var mycontroller: MyController { return mydocument.controller }
    var console: Console { return mycontroller.renderer.console }
    
    //
    // Initializing
    //

    init(with document: MyDocument) {

        debug(.lifetime, "Creating media manager")
        self.mydocument = document
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
            
            throw CoreError(.FILE_TYPE_MISMATCH,
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
        
            myAppDelegate.noteNewRecentlyInsertedDiskURL(url)
            myAppDelegate.noteNewRecentlyExportedDiskURL(url, df: n)
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
        
            myAppDelegate.noteNewRecentlyAttachedHdrURL(url)
            myAppDelegate.noteNewRecentlyExportedHdrURL(url, hd: n)
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
