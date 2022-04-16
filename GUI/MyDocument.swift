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
    
    // Gateway to the core emulator
    var amiga: AmigaProxy!

    // Snapshots
    private(set) var snapshots = ManagedArray<SnapshotProxy>(capacity: 32)
        
    //
    // Initializing
    //
    
    override init() {
        
        log()
        
        super.init()

        // Check for Metal support
        if MTLCreateSystemDefaultDevice() == nil {

            showAlert(.noMetalSupport)
            NSApp.terminate(self)
            return
        }
        
        // Create an emulator instance
        amiga = AmigaProxy()
        
        // Register GUI related user defaults
        AmigaProxy.defaults.registerUserDefaults()
        
        // Load the user default settings
        AmigaProxy.defaults.load()
    }
 
    override open func makeWindowControllers() {
                
        log()
        
        let controller = MyController(windowNibName: "MyDocument")
        controller.amiga = amiga
        self.addWindowController(controller)
    }
  
    //
    // Creating attachments
    //

    func createFileProxy(from url: URL, allowedTypes: [FileType]) throws -> AmigaFileProxy? {
            
        log("Reading file \(url.lastPathComponent)")
        
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
                    
                case .EXT:
                    return try EXTFileProxy.make(with: newUrl)
                    
                case .IMG:
                    return try IMGFileProxy.make(with: newUrl)
                    
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

    //
    // Loading
    //
    
    override open func read(from url: URL, ofType typeName: String) throws {
             
        log()

        let types: [FileType] =
        [ .SNAPSHOT, .SCRIPT, .ADF, .HDF, .EXT, .IMG, .DMS, .EXE, .DIR ]

        do {

            try addMedia(url: url, allowedTypes: types)
            
        } catch let error as VAError {
            
            throw NSError(error: error)
        }
    }
    
    override open func revert(toContentsOf url: URL, ofType typeName: String) throws {
        
        log()
        
        do {
            let proxy = try createFileProxy(from: url, allowedTypes: [.SNAPSHOT])
            if let snapshot = proxy as? SnapshotProxy {
                try processSnapshotFile(snapshot)
            }
            
        } catch let error as VAError {
            
            throw NSError(error: error)
        }
    }
    
    //
    // Saving
    //
    
    override func write(to url: URL, ofType typeName: String) throws {
            
        log()
        
        if typeName == "vAmiga" {
            
            // Take snapshot
            if let snapshot = SnapshotProxy.make(withAmiga: amiga) {

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
                  allowedTypes types: [FileType],
                  df: Int = 0,
                  hd: Int = 0,
                  force: Bool = false,
                  remember: Bool = true) throws {
        
        let proxy = try createFileProxy(from: url, allowedTypes: types)
        
        if remember && proxy is FloppyFileProxy {
            myAppDelegate.noteNewRecentlyInsertedDiskURL(url)
        }
        if remember && proxy is HDFFileProxy {
            myAppDelegate.noteNewRecentlyAttachedHdrURL(url)
        }
        
        try addMedia(proxy: proxy!, df: df, hd: hd, force: force)
    }
    
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
    
    func processSnapshotFile(_ proxy: SnapshotProxy, force: Bool = false) throws {
        
        try amiga.loadSnapshot(proxy)
        snapshots.append(proxy)
    }
    
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
            log(warning: "Invalid path extension")
            return
        }
        
        try export(fileProxy: df!, to: url)
        amiga.df(nr)!.markDiskAsUnmodified()
        myAppDelegate.noteNewRecentlyExportedDiskURL(url, df: nr)
        
        log("Disk exported successfully")
    }

    func export(hardDrive nr: Int, to url: URL) throws {
                        
        var dh: HDFFileProxy?
        switch url.pathExtension.uppercased() {
        case "HDF":
            dh = try HDFFileProxy.make(with: amiga.hd(nr)!)
        default:
            log(warning: "Invalid path extension")
            return
        }
        
        try export(fileProxy: dh!, to: url)

        amiga.hd(nr)!.markDiskAsUnmodified()
        myAppDelegate.noteNewRecentlyExportedHdrURL(url, hd: nr)

        log("Hard Drive exported successfully")
    }
    
    func export(fileProxy: AmigaFileProxy, to url: URL) throws {
        
        log("Exporting to \(url)")
        try fileProxy.writeToFile(url: url)        
    }        
}
