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

    // The application delegate
    var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }
    
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
    private(set) var autoSnapshots = ManagedArray<SnapshotProxy>.init(capacity: 32)
    private(set) var userSnapshots = ManagedArray<SnapshotProxy>.init(capacity: Int.max)

    // Screenshots (DEPRECATED)
    private(set) var autoScreenshots = ManagedArray<Screenshot>.init(capacity: 32)
    private(set) var userScreenshots = ManagedArray<Screenshot>.init(capacity: Int.max)

    // Fingerprint of the first disk inserted into df0 after reset
    private var bootDiskID = UInt64(0)
        
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
 
    deinit {
        
        track()
        amiga.kill()
    }
    
    override open func makeWindowControllers() {
        
        track()
        
        let nibName = NSNib.Name("MyDocument")
        let controller = MyController.init(windowNibName: nibName)
        controller.amiga = amiga
        self.addWindowController(controller)
    }

    //
    // Opening files
    //
    
    func openFile(url: URL, allowedTypes: [FileType]) -> (AmigaFileProxy?, FileError) {
        
        track("Opening URL \(url.lastPathComponent)")
        
        var err: FileError = .ERR_FILE_OK
        
        // If the provided URL points to compressed file, decompress it
        let path = url.unpacked.path
        
        // Iterate through all allowed file types
        for type in allowedTypes {
            
            switch type {
            
            case .SNAPSHOT:
                if let file = SnapshotProxy.make(withFile: path, error: &err) {
                    return (file, .ERR_FILE_OK)
                }
                
            case .ADF:
                if let file = ADFFileProxy.make(withFile: path, error: &err) {
                    return (file, .ERR_FILE_OK)
                }
                
            case .EXT:
                if let file = EXTFileProxy.make(withFile: path, error: &err) {
                    return (file, .ERR_FILE_OK)
                }
                
            case .IMG:
                if let file = IMGFileProxy.make(withFile: path, error: &err) {
                    return (file, .ERR_FILE_OK)
                }
                
            case .DMS:
                if let file = DMSFileProxy.make(withFile: path, error: &err) {
                    return (file, .ERR_FILE_OK)
                }
                
            case .EXE:
                if let file = EXEFileProxy.make(withFile: path, error: &err) {
                    return (file, .ERR_FILE_OK)
                }
                
            case .DIR:
                if let file = DIRFileProxy.make(withFile: path, error: &err) {
                    return (file, .ERR_FILE_OK)
                }
                
            case .HDF:
                if let file = HDFFileProxy.make(withFile: path, error: &err) {
                    return (file, .ERR_FILE_OK)
                }
                
            default:
                fatalError()
            }
            
            // Analyze the error code
            if err != .ERR_INVALID_TYPE { return (nil, err) }
        }
        
        // None of the allowed typed matched the file
        return (nil, .ERR_INVALID_TYPE)
    }
    
    //
    // Working with attachments
    //
        
    func createAttachment(url: URL, allowedTypes: [FileType]) -> FileError {
        
        track("Creating attachment from URL: \(url.lastPathComponent)")
        
        let (file, err) = openFile(url: url, allowedTypes: allowedTypes)
        
        if file != nil {
            amigaAttachment = file

            // Remember the URL
            if (file as? DiskFileProxy) != nil {
                myAppDelegate.noteNewRecentlyInsertedDiskURL(url)
            }
        }
        
        return err
    }
    
    @discardableResult
    func mountAttachment() -> Bool {
        
        switch amigaAttachment {

        case _ as SnapshotProxy:
            
            let proxy = amigaAttachment as! SnapshotProxy
            parent.load(snapshot: proxy)
            userSnapshots.append(proxy)
            
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
        
        let err = createAttachment(url: url, allowedTypes: [.SNAPSHOT])

        if err != .ERR_FILE_OK {
            throw NSError.fileError(err, url: url)
        }
    }
    
    //
    // Saving
    //
    
    override open func data(ofType typeName: String) throws -> Data {
        
        track("\(typeName)")
        
        if typeName == "vAmiga" {
            
            // Take snapshot
            if let snapshot = SnapshotProxy.make(withAmiga: amiga) {

                // Write to data buffer
                if let data = NSMutableData.init(length: snapshot.sizeOnDisk) {
                    snapshot.write(toBuffer: data.mutableBytes)
                    return data as Data
                }
            }
        }
        
        throw NSError(domain: NSOSStatusErrorDomain, code: unimpErr, userInfo: nil)
    }
    
    //
    // Exporting disks
    //
    
    @discardableResult
    func export(drive nr: Int, to url: URL) -> Bool {
                
        var df: DiskFileProxy?
        switch url.pathExtension.uppercased() {
        case "ADF":
            df = ADFFileProxy.make(withDrive: amiga.df(nr)!)
        case "IMG", "IMA":
            df = IMGFileProxy.make(withDrive: amiga.df(nr)!)
        default:
            break
        }
        
        if df != nil {
            return export(drive: nr, to: url, diskFileProxy: df!)
        } else {
            showExportDecodingAlert(driveNr: nr)
            return false
        }
    }
    
    @discardableResult
    func export(drive nr: Int, to url: URL, diskFileProxy df: DiskFileProxy) -> Bool {
                        
        track("Exporting disk to \(url)")
        
        // Serialize data
        let data = NSMutableData.init(length: df.sizeOnDisk)!
        df.write(toBuffer: data.mutableBytes)
        
        // Write to file
        if !data.write(to: url, atomically: true) {
            showExportAlert(url: url)
            return false
        }

        // Mark disk as "not modified"
        amiga.df(nr)!.isModifiedDisk = false
        
        // Remember export URL
        myAppDelegate.noteNewRecentlyExportedDiskURL(url, drive: nr)

        track("Export complete")
        return true
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
        
        if bootDiskID == 0 {
            bootDiskID = id
            try? loadScreenshots()
            return true
        }
        return false
    }
    
    // Writes screenshots back to disk if needed
    func persistScreenshots() throws {

        if userScreenshots.modified { try saveScreenshots() }
    }

    func saveScreenshots() throws {
        
        track("Saving user screenshots to disk (\(bootDiskID))")
        
        let format = parent.pref.screenshotTarget
        
        Screenshot.deleteFolder(forDisk: bootDiskID)
        for n in 0 ..< userScreenshots.count {
            let data = userScreenshots.element(at: n)?.screen?.representation(using: format)
            if let url = Screenshot.newUrl(diskID: bootDiskID, using: format) {
                try data?.write(to: url, options: .atomic)
            }
        }
    }
        
    func loadScreenshots() throws {
        
        track("Seeking screenshots for disk with id \(bootDiskID)")
        
        userScreenshots.clear()
        for url in Screenshot.collectFiles(forDisk: bootDiskID) {
            if let screenshot = Screenshot.init(fromUrl: url) {
                userScreenshots.append(screenshot)
            }
        }
        
        track("\(userScreenshots.count) screenshots loaded")
    }
}
