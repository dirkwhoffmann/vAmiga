// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class MyDocument: NSDocument {

    // The window controller for this document
    var parent: MyController { return windowControllers.first as! MyController }
    
    // Gateway to the core emulator
    var amiga: AmigaProxy!

    /* An optional media object attached to this document. This variable is
     * checked in mountAttachment() which is called in windowDidLoad(). If an
     * attachment is present, e.g., an ADF, it is automatically attached to the
     * emulator.
     */
    var attachment: AmigaFileProxy?
    
    // Snapshots
    private(set) var snapshots = ManagedArray<SnapshotProxy>(capacity: 32)
        
    //
    // Initializing
    //
    
    override init() {
        
        super.init()

        // Check for Metal support
        if MTLCreateSystemDefaultDevice() == nil {
            showNoMetalSupportAlert()
            NSApp.terminate(self)
            return
        }

        // Register standard user defaults
        UserDefaults.registerUserDefaults()
        
        // Create an emulator instance
        amiga = AmigaProxy()
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
        
    func createAttachment(from url: URL) throws {
        
        let types: [FileType] =
            [ .SNAPSHOT, .SCRIPT, .ADF, .HDF, .EXT, .IMG, .DMS, .EXE, .DIR ]
        
        try createAttachment(from: url, allowedTypes: types)
    }
    
    func createAttachment(from url: URL, allowedTypes: [FileType]) throws {
                        
        try attachment = createFileProxy(from: url, allowedTypes: allowedTypes)
        myAppDelegate.noteNewRecentlyInsertedDiskURL(url)
        
        log("Attachment created successfully")
    }
    
    fileprivate
    func createFileProxy(from url: URL, allowedTypes: [FileType]) throws -> AmigaFileProxy? {
            
        log("Creating proxy object from URL: \(url.lastPathComponent)")
        
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
            
    func mountAttachment(destination: FloppyDriveProxy? = nil) throws {
        
        // Only proceed if an attachment is present
        if attachment == nil { return }
        
        if let proxy = attachment as? SnapshotProxy {
            try amiga.loadSnapshot(proxy)
            snapshots.append(proxy)
            return
        }
        
        if let proxy = attachment as? ScriptProxy {
            parent.renderer.console.runScript(script: proxy)
            return
        }
        
        if let proxy = attachment as? HDFFileProxy {
            
            // TODO: CLEAN THIS CASE UP
            log("HDF with \(proxy.numBlocks) blocks")
            return
        }
        
        // Try to mount the attachment as a disk in df0
        try mountAttachment(drive: 0)
    }
    
    func mountAttachment(drive: Int) throws {

        if let proxy = attachment as? FloppyFileProxy {
            
            do {
                try amiga.df(drive)!.swap(file: proxy)
            } catch {
                (error as? VAError)?.cantInsert()
            }
        }
        
        if let proxy = attachment as? HDFFileProxy {
            
            do {
                
                if parent.askToReboot() {
                    
                    amiga.powerOff()
                    amiga.configure(.HDR_CONNECT, drive: drive, enable: true)
                    try amiga.hd(drive)?.attach(hdf: proxy)
                    amiga.powerOn()
                    try amiga.run()
                }
                
            } catch {
                (error as? VAError)?.cantAttach()
            }
        }
    }
    
    //
    // Loading
    //
    
    override open func read(from url: URL, ofType typeName: String) throws {
                
        do {
            try createAttachment(from: url)
            
        } catch let error as VAError {
            
            throw NSError(error: error)
        }
    }
    
    override open func revert(toContentsOf url: URL, ofType typeName: String) throws {
        
        log()
        
        do {
            try createAttachment(from: url)
            try mountAttachment()
            
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
