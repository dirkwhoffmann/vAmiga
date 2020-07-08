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
    var parent: MyController? { return windowControllers.first as? MyController }

    // The application delegate
    var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }
    
    /*
     Emulator proxy object. This object is an Objective-C bridge between
     the GUI (written in Swift) an the core emulator (written in C++).
     */
    var amiga: AmigaProxy!

    /*
     An otional media object attached to this document.
     This variable is checked in mountAmigaAttachment() which is called in
     windowDidLoad(). If an attachment is present, e.g., an ADF archive, it
     is automatically attached to the emulator.
     */
    var amigaAttachment: AmigaFileProxy?
    
    // Snapshots
    private(set) var autoSnapshots = ManagedArray<SnapshotProxy>.init(capacity: 32)
    private(set) var userSnapshots = ManagedArray<SnapshotProxy>.init(capacity: Int.max)

    // Screenshots
    private(set) var autoScreenshots = ManagedArray<Screenshot>.init(capacity: 32)
    private(set) var userScreenshots = ManagedArray<Screenshot>.init(capacity: Int.max)

    // Fingerprint of the first disk inserted into df0 after a reset
    // The value is used to determine the screenshot save folder
    private var bootDiskID = UInt64(0)
        
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
        
        // Shut down the emulator
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
    // Creating attachments
    //
    
    // Creates an ADF file proxy from a URL
    func createADF(from url: URL) throws -> ADFFileProxy? {

        track("Creating ADF proxy from URL \(url.lastPathComponent).")
        
        // If the URL points to an ADZ, unzip it
        let newUrl = url.adfFromAdz ?? url

        // Try to create a file wrapper
        let fileWrapper = try FileWrapper.init(url: newUrl)
        guard let data = fileWrapper.regularFileContents else {
            throw NSError(domain: "vAmiga", code: 0, userInfo: nil)
        }
        
        // Try to create ADF file proxy
        let buffer = (data as NSData).bytes
        let length = data.count
        let proxy = ADFFileProxy.make(withBuffer: buffer, length: length)
        
        if proxy != nil {
            myAppDelegate.noteNewRecentlyUsedURL(url)
        }
        
        return proxy
    }
    
    // Creates an attachment from a URL
    func createAmigaAttachment(from url: URL) throws {
        
        track("Creating attachment from URL \(url.lastPathComponent).")
        
        // If the URL points to an ADZ, unzip it
        let newUrl = url.adfFromAdz ?? url
        
        // Try to create the attachment
        let fileWrapper = try FileWrapper.init(url: newUrl)
        let pathExtension = newUrl.pathExtension.uppercased()
        try createAmigaAttachment(from: fileWrapper, ofType: pathExtension)
        
        // Put URL in recently used URL lists
        myAppDelegate.noteNewRecentlyUsedURL(url)
    }

    // Creates an attachment from a file wrapper
    fileprivate func createAmigaAttachment(from fileWrapper: FileWrapper,
                                           ofType typeName: String) throws {
        
        guard let filename = fileWrapper.filename else {
            throw NSError(domain: "vAmiga", code: 0, userInfo: nil)
        }
        guard let data = fileWrapper.regularFileContents else {
            throw NSError(domain: "vAmiga", code: 0, userInfo: nil)
        }
        
        let buffer = (data as NSData).bytes
        let length = data.count
        var openAsUntitled = true
        
        track("Read \(length) bytes from file \(filename) [\(typeName)].")
        
        switch typeName {
            
        case "VAMIGA":
            // Check for outdated snapshot formats
            if SnapshotProxy.isUnsupportedSnapshot(buffer, length: length) {
                throw NSError.snapshotVersionError(filename: filename)
            }
            amigaAttachment = SnapshotProxy.make(withBuffer: buffer, length: length)
            openAsUntitled = false
            
        case "ADF":
            amigaAttachment = ADFFileProxy.make(withBuffer: buffer, length: length)
            
        default:
            throw NSError.unsupportedFormatError(filename: filename)
        }
        
        if amigaAttachment == nil {
            throw NSError.corruptedFileError(filename: filename)
        }
        if openAsUntitled {
            fileURL = nil
        }
        amigaAttachment!.setPath(filename)
    }

    //
    // Processing attachments
    //
    
    @discardableResult
    func mountAmigaAttachment() -> Bool {
        
        switch amigaAttachment {

        case _ as SnapshotProxy:
            
            parent?.load(snapshot: amigaAttachment as? SnapshotProxy)
       
        case _ as ADFFileProxy:
            
            if let df = parent?.dragAndDropDrive?.nr() {
                amiga.diskController.insert(df, adf: amigaAttachment as? ADFFileProxy)
            } else {
                runDiskMountDialog()
            }
            
        default:
            break
        }
        
        return true
    }
    
    func runDiskMountDialog() {
        let name = NSNib.Name("DiskMountDialog")
        let controller = DiskMountDialog.make(parent: parent!, nibName: name)
        controller?.showSheet()
    }

    //
    // Loading
    //
    
    override open func read(from url: URL, ofType typeName: String) throws {
        
        try createAmigaAttachment(from: url)
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
                if let data = NSMutableData.init(length: snapshot.sizeOnDisk()) {
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
    
    func export(drive nr: Int, to url: URL, ofType typeName: String) -> Bool {
        
        // track("url = \(url) typeName = \(typeName)")
        assert(["ADF"].contains(typeName))
        
        let drive = amiga.df(nr)!
        
        // Convert disk to ADF format
        guard let adf = ADFFileProxy.make(withDrive: drive) else {
            track("ADF conversion failed")
            return false
        }

        // Serialize data
        let data = NSMutableData.init(length: adf.sizeOnDisk())!
        adf.write(toBuffer: data.mutableBytes)
        
        // Write to file
        if !data.write(to: url, atomically: true) {
            showExportErrorAlert(url: url)
            return false
        }

        // Mark disk as "not modified"
        drive.setModifiedDisk(false)
        
        // Remember export URL
        myAppDelegate.noteNewRecentlyExportedDiskURL(url, drive: nr)
        return true
    }
    
    @discardableResult
    func export(drive nr: Int, to url: URL?) -> Bool {
        
        if let suffix = url?.pathExtension {
            return export(drive: nr, to: url!, ofType: suffix.uppercased())
        } else {
            return false
        }
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
        
        let format = parent!.prefs.screenshotTarget
        
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
