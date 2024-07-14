// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class MyDocument: NSDocument {

    var pref: Preferences { return myAppDelegate.pref }
    
    // The window controller for this document
    var parent: MyController { return windowControllers.first as! MyController }

    // Optional media URL provided on app launch
    var launchUrl: URL?

    // Gateway to the core emulator
    var amiga: EmulatorProxy!

    // Snapshots
    private(set) var snapshots = ManagedArray<MediaFileProxy>(maxSize: 512 * 1024 * 1024)

    //
    // Initializing
    //
    
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
        amiga = EmulatorProxy()
    }
 
    override open func makeWindowControllers() {
                
        debug(.lifetime)
        
        let controller = MyController(windowNibName: "MyDocument")
        controller.emu = amiga
        self.addWindowController(controller)
    }
  
    //
    // Creating file proxys
    //

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

    /*
    func createFileProxy(from url: URL, allowedTypes: [FileType]) throws -> AmigaFileProxy? {
            
        debug(.media, "Reading file \(url.lastPathComponent)")
        
        // If the provided URL points to compressed file, decompress it first
        let newUrl = url.unpacked(maxSize: 2048 * 1024)
        
        // Iterate through all allowed file types
        for type in allowedTypes {
            
            do {
                switch type {
                
                case .SNAPSHOT:
                    return try SnapshotProxy.make(with: newUrl)
                    
                case .SCRIPT:
                    return try ScriptProxy.make(with: newUrl)
                    
                case .ADF:
                    return try ADFFileProxy.make(with: newUrl)
                    
                case .EADF:
                    return try EADFFileProxy.make(with: newUrl)
                    
                case .IMG:
                    return try IMGFileProxy.make(with: newUrl)

                case .ST:
                    return try STFileProxy.make(with: newUrl)

                case .DMS:
                    return try DMSFileProxy.make(with: newUrl)
                    
                case .EXE:
                    return try EXEFileProxy.make(with: newUrl)
                    
                case .DIR:
                    return try FolderProxy.make(with: newUrl)
                    
                case .HDF:
                    return try HDFFileProxy.make(with: newUrl)
                    
                default:
                    fatalError()
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
    */

    //
    // Loading
    //
    
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
    
    override func write(to url: URL, ofType typeName: String) throws {
            
        debug(.media)
        
        if typeName == "vAmiga" {

            if let snapshot = SnapshotProxy.make(withAmiga: amiga.amiga) {

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

            // TODO: Uncomment
            // parent.renderer.console.runScript(script: proxy)
            break

        case .HDF:

            // TODO: Uncomment
            try attach(hd: hd, file: proxy, force: force)
            break

        case .ADF, .DMS, .EXE, .EADF, .IMG, .ST:

            try insert(df: df, file: proxy, force: force)

        default:
            break
        }
    }

    /*
    @available(*, deprecated, message: "Use addMedia:(proxy: MediaFileProxy instead")
    func addMedia(proxy: AmigaFileProxy,
                  df: Int = 0,
                  hd: Int = 0,
                  force: Bool = false) throws {
        
        if let proxy = proxy as? SnapshotProxy {
            
            try processSnapshotFile(proxy)
        }
        if let proxy = proxy as? ScriptProxy {

            parent.renderer.console.runScript(script: proxy)
        }
        if let proxy = proxy as? HDFFileProxy {
            
            try attach(hd: hd, file: proxy, force: force)
        }
        if let proxy = proxy as? FloppyFileProxy {
            
            try insert(df: df, file: proxy, force: force)
        }
    }
    */

    func processSnapshotFile(_ proxy: MediaFileProxy, force: Bool = false) throws {

        try amiga.loadSnapshot(proxy)
        snapshots.append(proxy, size: proxy.size)
    }

    /*
    @available(*, deprecated)
    func processSnapshotFile(_ proxy: SnapshotProxy, force: Bool = false) throws {
        
        try amiga.loadSnapshot(proxy)
        snapshots.append(proxy, size: proxy.size)
    }
    */
    
    //
    // Exporting disks
    //
    
    func export(drive nr: Int, to url: URL) throws {
                        
        var df: FloppyFileProxy?
        switch url.pathExtension.uppercased() {
        case "ADF":
            df = try ADFFileProxy.make(with: amiga.df(nr)!)
        case "IMG", "IMA":
            df = try IMGFileProxy.make(with: amiga.df(nr)!)
        default:
            warn("Invalid path extension")
            return
        }
        
        try export(fileProxy: df!, to: url)
        amiga.df(nr)!.setFlag(.MODIFIED, value: false)
        myAppDelegate.noteNewRecentlyExportedDiskURL(url, df: nr)
        
        debug(.media, "Disk exported successfully")
    }

    func export(hardDrive nr: Int, to url: URL) throws {
        
        let hdn = amiga.hd(nr)!
        var dh: HDFFileProxy?

        switch url.pathExtension.uppercased() {
        case "HDF":
            dh = try HDFFileProxy.make(with: amiga.hd(nr)!)
        default:
            warn("Invalid path extension")
            return
        }
        
        try export(fileProxy: dh!, to: url)

        hdn.setFlag(.MODIFIED, value: false)
        myAppDelegate.noteNewRecentlyExportedHdrURL(url, hd: nr)

        debug(.media, "Hard Drive exported successfully")
    }
    
    func export(fileProxy: AmigaFileProxy, to url: URL) throws {
        
        debug(.media, "Exporting to \(url)")
        try fileProxy.writeToFile(url: url)        
    }        
}
