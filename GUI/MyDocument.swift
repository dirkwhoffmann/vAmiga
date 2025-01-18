// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import UniformTypeIdentifiers

extension UTType {

    static let adf = UTType("de.dirkwhoffmann.retro.adf")!
    static let adz = UTType("de.dirkwhoffmann.retro.adf")!
    static let dms = UTType("de.dirkwhoffmann.retro.dms")!
    static let exe = UTType("de.dirkwhoffmann.retro.exe")!
    static let hdf = UTType("de.dirkwhoffmann.retro.hdf")!
    static let hdz = UTType("de.dirkwhoffmann.retro.hdz")!
    static let img = UTType("de.dirkwhoffmann.retro.img")!
    static let ini = UTType("de.dirkwhoffmann.retro.ini")!
    static let vamiga = UTType("de.dirkwhoffmann.retro.vamiga")!
}

class MyDocument: NSDocument {

    var pref: Preferences { return myAppDelegate.pref }
    
    // The window controller for this document
    var parent: MyController { return windowControllers.first as! MyController }
    var console: Console { return parent.renderer.console }

    // Optional media URL provided on app launch
    var launchUrl: URL?

    // Gateway to the core emulator
    var emu: EmulatorProxy!

    // Snapshots
    private(set) var snapshots = ManagedArray<MediaFileProxy>(maxSize: 512 * 1024 * 1024)

    //
    // Initializing
    //
    
    @MainActor
    override init() {
        
        debug(.lifetime)
        
        super.init()

        // Check for Metal support
        if MTLCreateSystemDefaultDevice() == nil {

            showAlert(.noMetalSupport)
            NSApp.terminate(self)
            return
        }
                
        // Register all GUI related user defaults
        EmulatorProxy.defaults.registerUserDefaults()
        
        // Load the user default settings
        EmulatorProxy.defaults.load()
        
        // Create an emulator instance
        emu = EmulatorProxy()
    }
 
    @MainActor
    override open func makeWindowControllers() {
                
        debug(.lifetime)
        
        let controller = MyController(windowNibName: "MyDocument")
        controller.emu = emu
        self.addWindowController(controller)
    }
  
    //
    // Creating file proxys
    //

    @MainActor
    func createMediaFileProxy(from url: URL, allowedTypes: [FileType]) throws -> MediaFileProxy {

        debug(.media, "Reading file \(url.lastPathComponent)")

        // If the provided URL points to compressed file, decompress it first
        let newUrl = url.unpacked(maxSize: 2048 * 1024)

        // Iterate through all allowed file types
        for type in allowedTypes {

            do {
                switch type {

                case .SNAPSHOT, .SCRIPT, .ADF, .EADF, .IMG, .ST, .DMS, .EXE, .DIR, .HDF:

                    return try MediaFileProxy.make(with: newUrl, type: type)

                default:
                    break
                }

            } catch let error as VAError {
                if error.errorCode != .FILE_TYPE_MISMATCH {
                    throw error
                }
            }
        }

        // None of the allowed types matched the file
        throw VAError(.FILE_TYPE_MISMATCH,
                      "The type of this file is not known to the emulator.")
    }


    //
    // Loading
    //
    
    nonisolated
    override open func read(from url: URL, ofType typeName: String) throws {
             
        debug(.media)

        launchUrl = url
        
        /*
        let types: [FileType] =
        [ .SNAPSHOT, .SCRIPT, .ADF, .EADF, .HDF, .IMG, .ST, .DMS, .EXE, .DIR ]

        do {
            try addMedia(url: url, allowedTypes: types)
            
        } catch let error as VAError {
            
            throw NSError(error: error)
        }
        */
    }
    
    @MainActor
    override open func revert(toContentsOf url: URL, ofType typeName: String) throws {
        
        debug(.media)
        
        do {
            try addMedia(url: url, allowedTypes: [.SNAPSHOT])

        } catch let error as VAError {

            throw NSError(error: error)
        }
    }
    
    //
    // Saving
    //
    
    /*
    nonisolated
    override func write(to url: URL, ofType typeName: String) throws {
            
        debug(.media)
        
        if typeName == "vAmiga" {

            if let snapshot = emu.amiga.takeSnapshot() {

                do {
                    try snapshot.writeToFile(url: url)

                } catch let error as VAError {
                    
                    throw NSError(error: error)
                }
            }
        }
    }
    */
    
    @MainActor
    override func save(to url: URL, ofType typeName: String, for saveOperation: NSDocument.SaveOperationType) async throws {
            
        debug(.media)
        
        if typeName == "vAmiga" {

            if let snapshot = emu.amiga.takeSnapshot() {

                do {
                    try snapshot.writeToFile(url: url)

                } catch let error as VAError {
                    
                    throw NSError(error: error)
                }
            }
        }
    }
    
    //
    // Handling media files
    //

    func addMedia(url: URL,
                  allowedTypes types: [FileType] = FileType.all,
                  df: Int = 0,
                  hd: Int = 0,
                  force: Bool = false,
                  remember: Bool = true) throws {
        
        let file = try createMediaFileProxy(from: url, allowedTypes: types)

        // Remember the URL if requested
        if remember {

            switch file.type {

            case .SNAPSHOT:
                // document.snapshots.append(file)
                break

            case .ADF, .EADF, .HDF, .EXE, .IMG, .ST:
                myAppDelegate.noteNewRecentlyInsertedDiskURL(url)

            default:
                break
            }
        }

        try addMedia(proxy: file, df: df, hd: hd, force: force)
    }
    
    func addMedia(proxy: MediaFileProxy,
                  df: Int = 0,
                  hd: Int = 0,
                  force: Bool = false) throws {

        switch proxy.type {

        case .SNAPSHOT:

            try processSnapshotFile(proxy)

        case .SCRIPT:

            console.runScript(script: proxy)
            break

        case .HDF:

            try attach(hd: hd, file: proxy, force: force)
            break

        case .ADF, .DMS, .EXE, .EADF, .IMG, .ST:

            try insert(df: df, file: proxy, force: force)

        default:
            break
        }
    }

    func processSnapshotFile(_ proxy: MediaFileProxy, force: Bool = false) throws {

        try emu.amiga.loadSnapshot(proxy)
        snapshots.append(proxy, size: proxy.size)
    }

    
    //
    // Exporting disks
    //
    
    func export(drive nr: Int, to url: URL) throws {
                        
        var df: MediaFileProxy?
        switch url.pathExtension.uppercased() {
        case "ADF":
            df = try MediaFileProxy.make(with: emu.df(nr)!, type: .ADF)
        case "IMG", "IMA":
            df = try MediaFileProxy.make(with: emu.df(nr)!, type: .IMG)
        default:
            warn("Invalid path extension")
            return
        }
        
        try export(fileProxy: df!, to: url)
        emu.df(nr)!.setFlag(.MODIFIED, value: false)
        myAppDelegate.noteNewRecentlyExportedDiskURL(url, df: nr)
        
        debug(.media, "Disk exported successfully")
    }

    func export(hardDrive nr: Int, to url: URL) throws {
        
        let hdn = emu.hd(nr)!
        var dh: MediaFileProxy?

        switch url.pathExtension.uppercased() {
        case "HDF":
            dh = try MediaFileProxy.make(with: emu.hd(nr)!, type: .HDF)
        default:
            warn("Invalid path extension")
            return
        }
        
        try export(fileProxy: dh!, to: url)

        hdn.setFlag(.MODIFIED, value: false)
        myAppDelegate.noteNewRecentlyExportedHdrURL(url, hd: nr)

        debug(.media, "Hard Drive exported successfully")
    }

    func export(fileProxy: MediaFileProxy, to url: URL) throws {

        debug(.media, "Exporting to \(url)")
        try fileProxy.writeToFile(url: url)
    }
}
