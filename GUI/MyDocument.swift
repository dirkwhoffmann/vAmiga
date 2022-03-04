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
     * attachment is present, e.g., a D64 archive, it is automatically attached
     * to the emulator.
     */
    var attachment: AmigaFileProxy?
    
    // Snapshots
    private(set) var snapshots = ManagedArray<SnapshotProxy>(capacity: 32)

    // Fingerprint of the first disk inserted into df0 after reset
    var bootDiskID = UInt64(0)
        
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
                
        track()
        
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
        
        track("Attachment created successfully")
    }
    
    fileprivate
    func createFileProxy(from url: URL, allowedTypes: [FileType]) throws -> AmigaFileProxy? {
            
        track("Creating proxy object from URL: \(url.lastPathComponent)")
        
        // If the provided URL points to compressed file, decompress it first
        let newUrl = url.unpacked(maxSize: 2048 * 1024)
        
        // Iterate through all allowed file types
        for type in allowedTypes {
            
            do {
                switch type {
                
                case .SNAPSHOT:
                    return try Proxy.make(url: newUrl) as SnapshotProxy
                    
                case .SCRIPT:
                    return try Proxy.make(url: newUrl) as ScriptProxy
                    
                case .ADF:
                    return try Proxy.make(url: newUrl) as ADFFileProxy
                    
                case .EXT:
                    return try Proxy.make(url: newUrl) as EXTFileProxy
                    
                case .IMG:
                    return try Proxy.make(url: newUrl) as IMGFileProxy
                    
                case .DMS:
                    return try Proxy.make(url: newUrl) as DMSFileProxy
                    
                case .EXE:
                    return try Proxy.make(url: newUrl) as EXEFileProxy
                    
                case .DIR:
                    return try Proxy.make(url: newUrl) as FolderProxy
                    
                case .HDF:
                    return try Proxy.make(url: newUrl) as HDFFileProxy
                    
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
            track("HDF with \(proxy.numBlocks) blocks")
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
            
            if parent.askToReboot() {
            
                amiga.powerOff()
                try? amiga.hd(drive)?.attach(hdf: proxy)
                amiga.powerOn()
                try? amiga.run()
            }
        }
    }
    
    func runImportDialog() {
        
        let name = NSNib.Name("ImporterDialog")
        let controller = ImporterDialog.make(parent: parent, nibName: name)
        controller?.showSheet()
    }

    //
    // Loading
    //
    
    override open func read(from url: URL, ofType typeName: String) throws {
        
        do {
            try createAttachment(from: url)
            
        } catch let error as VAError {
            
            error.cantOpen(url: url)
        }
    }
    
    override open func revert(toContentsOf url: URL, ofType typeName: String) throws {
        
        track()
        
        do {
            try createAttachment(from: url)
            try mountAttachment()
            
        } catch let error as VAError {
            
            error.cantOpen(url: url)
        }
    }
    
    //
    // Saving
    //
    
    override func write(to url: URL, ofType typeName: String) throws {
            
        track()
        
        if typeName == "vAmiga" {
            
            // Take snapshot
            if let snapshot = SnapshotProxy.make(withAmiga: amiga) {

                // Write to data buffer
                do {
                    _ = try snapshot.writeToFile(url: url)
                } catch {
                    throw NSError(domain: NSOSStatusErrorDomain, code: unimpErr, userInfo: nil)
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
            df = try Proxy.make(drive: amiga.df(nr)!) as ADFFileProxy
        case "IMG", "IMA":
            df = try Proxy.make(drive: amiga.df(nr)!) as IMGFileProxy
        default:
            track("Invalid path extension")
            return
        }
        
        try export(fileProxy: df!, to: url)
        amiga.df(nr)!.markDiskAsUnmodified()
        myAppDelegate.noteNewRecentlyExportedDiskURL(url, drive: nr)
        
        track("Disk exported successfully")
    }

    func export(hardDrive nr: Int, to url: URL) throws {
                        
        var dh: HDFFileProxy?
        switch url.pathExtension.uppercased() {
        case "HDF":
            dh = try Proxy.make(hdr: amiga.hd(nr)!) as HDFFileProxy
        default:
            track("Invalid path extension")
            return
        }
        
        try export(fileProxy: dh!, to: url)

        amiga.hd(nr)!.markDiskAsUnmodified()
        myAppDelegate.noteNewRecentlyExportedHdrURL(url, drive: nr)

        track("Hard Drive exported successfully")
    }
    
    func export(fileProxy: AmigaFileProxy, to url: URL) throws {
        
        track("Exporting to \(url)")
        try fileProxy.writeToFile(url: url)        
    }
        
    //
    // Screenshots
    //
    
    func deleteBootDiskID() {
     
        bootDiskID = 0
    }

    @discardableResult
    func setBootDiskID(_ id: UInt64) -> Bool {
        
        // Only proceed if no disk has been inserted, yet
        if bootDiskID != 0 { return false }
        
        // Remember the disk ID
        bootDiskID = id
        return true
    }
}
