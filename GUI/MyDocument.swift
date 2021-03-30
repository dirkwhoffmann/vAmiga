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
    
    /* Emulator proxy. This object is an Objective-C bridge between the Swift
     * GUI an the core emulator which is written in C++.
     */
    var amiga: AmigaProxy!

    /* An otional media object attached to this document. This variable is
     * checked in mountAttachment() which is called in windowDidLoad(). If an
     * attachment is present, e.g., an ADF archive, it is automatically
     * attached to the emulator.
     */
    var amigaAttachment: AmigaFileProxy?
    
    // Snapshots
    private(set) var snapshots = ManagedArray<SnapshotProxy>.init(capacity: 32)

    // Fingerprint of the first disk inserted into df0 after reset
    var bootDiskID = UInt64(0)
        
    //
    // Initializing
    //
    
    override init() {
        
        track()
        super.init()

        // Check for Metal support
        if MTLCreateSystemDefaultDevice() == nil {
            showNoMetalSupportAlert()
            NSApp.terminate(self)
            return
        }

        // Register standard user defaults
        UserDefaults.registerUserDefaults()
        
        // Create emulator instance
        amiga = AmigaProxy()
    }
 
    override open func makeWindowControllers() {
        
        track()
        
        let nibName = NSNib.Name("MyDocument")
        let controller = MyController.init(windowNibName: nibName)
        controller.amiga = amiga
        self.addWindowController(controller)
    }
  
    //
    // Creating attachments
    //
        
    func createAttachment(from url: URL) throws {
        
        let types: [FileType] =
            [ .SNAPSHOT, .ADF, .HDF, .EXT, .IMG, .DMS, .EXE, .DIR ]
        
        try createAttachment(from: url, allowedTypes: types)
    }
    
    func createAttachment(from url: URL, allowedTypes: [FileType]) throws {
        
        track("Creating attachment from URL: \(url.lastPathComponent)")
        
        try amigaAttachment = createFileProxy(from: url, allowedTypes: allowedTypes)
        myAppDelegate.noteNewRecentlyInsertedDiskURL(url)
        
        track("Attachment created successfully")
    }
    
    fileprivate
    func createFileProxy(from url: URL, allowedTypes: [FileType]) throws -> AmigaFileProxy? {
        
        var result: AmigaFileProxy?
        
        track("Creating proxy object from URL: \(url.lastPathComponent)")
        
        // If the provided URL points to compressed file, decompress it first
        let newUrl = url.unpacked

        // Iterate through all allowed file types
        for type in allowedTypes {
            
            switch type {
            
            case .SNAPSHOT:
                try? result = Proxy.make(url: newUrl) as SnapshotProxy
                
            case .ADF:
                try? result = Proxy.make(url: newUrl) as ADFFileProxy
                                
            case .EXT:
                try? result = Proxy.make(url: newUrl) as EXTFileProxy
                
            case .IMG:
                try? result = Proxy.make(url: newUrl) as IMGFileProxy
                
            case .DMS:
                try? result = Proxy.make(url: newUrl) as DMSFileProxy
                
            case .EXE:
                try? result = Proxy.make(url: newUrl) as EXEFileProxy
                
            case .DIR:
                try? result = Proxy.make(url: newUrl) as FolderProxy
                
            case .HDF:
                try? result = Proxy.make(url: newUrl) as HDFFileProxy
                
            default:
                fatalError()
            }
            
            if result != nil { return result }
        }
        
        // None of the allowed typed matched the file
        throw VAError(.FILE_TYPE_MISMATCH)
    }

    @discardableResult
    func mountAttachment() -> Bool {
        
        switch amigaAttachment {

        case _ as SnapshotProxy:
            
            let proxy = amigaAttachment as! SnapshotProxy
            parent.load(snapshot: proxy)
            snapshots.append(proxy)
            
        case _ as HDFFileProxy:
            
            track()
            
            parent.warning("This file is a hard drive image (HDF)",
                           "Hard drive emulation is not supported yet.",
                           icon: "hdf")
        
        // Experimental code: The current version of vAmiga does not
        // support hard drives. If a HDF file is dragged in, we open the
        // disk export dialog for debugging purposes. It enables us to
        // examine the contents of the HDF and to check for file system
        // errors.
        /*
            if let vol = FSDeviceProxy.make(withHDF: amigaAttachment as? HDFFileProxy) {

                vol.dump()
            
                let nibName = NSNib.Name("ExporterDialog")
                let exportPanel = ExporterDialog.make(parent: parent, nibName: nibName)
                exportPanel?.showSheet(forVolume: vol)
            }
        */

        case _ as DiskFileProxy:

            if let df = parent.dragAndDropDrive?.nr {
                amiga.diskController.insert(df, file: amigaAttachment as? DiskFileProxy)
            } else {
                runImporterDialog()
            }
                    
        default:
            break
        }
        
        return true
    }
    
    func runImporterDialog() {
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
            mountAttachment()
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
                        
        var df: DiskFileProxy?
        switch url.pathExtension.uppercased() {
        case "ADF":
            df = try Proxy.make(drive: amiga.df(nr)!) as ADFFileProxy
        case "IMG", "IMA":
            df = try Proxy.make(drive: amiga.df(nr)!) as IMGFileProxy
        default:
            break
        }
        
        try export(diskFileProxy: df!, to: url)
        
        // Mark disk as "not modified"
        amiga.df(nr)!.isModifiedDisk = false
        
        // Remember export URL
        myAppDelegate.noteNewRecentlyExportedDiskURL(url, drive: nr)
    }
    
    func export(diskFileProxy df: DiskFileProxy, to url: URL) throws {
        
        track("Exporting disk to \(url)")
        
        try df.writeToFile(url: url)        
    }
    
    //
    // Snapshots
    //
        
    private func thinOut(numItems: Int, counter: inout Int) -> Int? {
        
        if numItems < 32 { return nil }
              
        var itemToDelete = 0
        
        if counter % 2 == 0 {
            itemToDelete = 24
        } else if (counter >> 1) % 2 == 0 {
            itemToDelete = 16
        } else if (counter >> 2) % 2 == 0 {
            itemToDelete = 8
        }
        counter += 1
        
        return itemToDelete
    }
    
    //
    // Screenshots
    //
    
    func deleteBootDiskID() {
     
        bootDiskID = 0
    }

    @discardableResult
    func setBootDiskID(_ id: UInt64) -> Bool {
        
        track("setBootDiskID(\(id))")
        
        if bootDiskID == 0 {
            track("Assigning new ID")
            bootDiskID = id
            return true
        }
        return false
    }
}
