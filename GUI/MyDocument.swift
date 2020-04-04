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

    /*
     Emulator proxy object. This object is an Objective-C bridge between
     the GUI (written in Swift) an the core emulator (written in C++).
     */
    var amiga: AmigaProxy!

    /*
     An otional media object attached to this document.
     This variable is checked by the GUI, e.g., when the READY_TO_RUN message
     is received. If an attachment is present, e.g., a T64 archive,
     is displays a user dialog. The user can then choose to mount the archive
     as a disk or to flash a single file into memory. If the attachment is a
     snapshot, it is read into the emulator without asking the user.
     This variable is also used when the user selects the "Insert Disk",
     "Insert Tape" or "Attach Cartridge" menu items. In that case, the selected
     URL is translated into an attachment and then processed. The actual
     post-processing depends on the attachment type and user options. E.g.,
     snapshots are flashed while T64 archives are converted to a disk and
     inserted into the disk drive.
     */
    var amigaAttachment: AmigaFileProxy?
    
    // Snapshots
    var autoSnapshots: [SnapshotProxy] = []
    var userSnapshots: [SnapshotProxy] = []
    var autoSnapshotCounter = 0
    
    // Screenshots
    private(set) var autoScreenshots: [Screenshot] = []
    private(set) var userScreenshots: [Screenshot] = []
    private var autoScreenshotCounter = 0
    private var autoScreenshotsModified = false
    private var userScreenshotsModified = false
    
    // Screenshots are associated the ADF file with this fingerprint
    var adfChecksum = UInt64(0) {
        willSet {
            if newValue != adfChecksum { try? persistScreenshots() }
        }
        didSet {
            if oldValue != adfChecksum { try? loadScreenshots() }
        }
    }
        
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
        MyController.registerUserDefaults()
        
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
        
        // Try to create a file wrapper
        let fileWrapper = try FileWrapper.init(url: url)
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
        
        // Try to create the attachment
        let fileWrapper = try FileWrapper.init(url: url)
        let pathExtension = url.pathExtension.uppercased()
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
            
            amiga.load(fromSnapshot: amigaAttachment as? SnapshotProxy)
       
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
        
        track("url = \(url) typeName = \(typeName)")
        assert(["ADF"].contains(typeName))
        
        let drive = amiga.df(nr)
        
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
    
    private func thinOutAutoSnapshots() {
        
        if let index = thinOut(numItems: autoSnapshots.count,
                               counter: &autoSnapshotCounter) {
            autoSnapshots.remove(at: index)
        }
    }
    
    func appendSnapshot(_ snapshot: SnapshotProxy, auto: Bool) {
        
        if auto {
            autoSnapshots.append(snapshot)
            thinOutAutoSnapshots()
        } else {
            userSnapshots.append(snapshot)
        }
    }
    
    //
    // Screenshots
    //
    
    func removeAutoScreenshot(at index: Int) {
        
        autoScreenshots.remove(at: index)
        autoScreenshotsModified = true
    }
    
    func removeUserScreenshot(at index: Int) {
        
        userScreenshots.remove(at: index)
        userScreenshotsModified = true
    }
    
    func swapAutoScreenshots(_ i: Int, _ j: Int) {
        
        autoScreenshots.swapAt(i, i)
        autoScreenshotsModified = true
    }

    func swapUserScreenshots(_ i: Int, _ j: Int) {
        
        userScreenshots.swapAt(i, i)
        userScreenshotsModified = true
    }

    func appendAutoScreenshot(_ newElement: Screenshot) {
        
        autoScreenshots.append(newElement)
        autoScreenshotsModified = true
        
        // Thin out screenshots to limit growth
        if let index = thinOut(numItems: autoScreenshots.count,
                               counter: &autoScreenshotCounter) {
            autoScreenshots.remove(at: index)
        }
    }

    func appendUserScreenshot(_ newElement: Screenshot) {
        
        userScreenshots.append(newElement)
        userScreenshotsModified = true
    }
             
    // Writes screenshots back to disk if needed
    func persistScreenshots() throws {

        if autoScreenshotsModified { try saveAutoScreenshots() }
        if userScreenshotsModified { try saveUserScreenshots() }
    }

    func saveAutoScreenshots() throws {
        
        track("Saving auto screenshots to disk (\(adfChecksum))")

        let format = parent!.screenshotTarget
        
        Screenshot.deleteAutoFolder(checksum: adfChecksum)
        for n in 0 ..< autoScreenshots.count {
            let data = autoScreenshots[n].screen?.representation(using: format)
            if let url = Screenshot.newAutoUrl(checksum: adfChecksum, using: format) {
                try data?.write(to: url, options: .atomic)
            }
        }
    }
    
    func saveUserScreenshots() throws {
        
        track("Saving user screenshots to disk (\(adfChecksum))")
        
        let format = parent!.screenshotTarget
        
        Screenshot.deleteUserFolder(checksum: adfChecksum)
        for n in 0 ..< userScreenshots.count {
            let data = userScreenshots[n].screen?.representation(using: format)
            if let url = Screenshot.newUserUrl(checksum: adfChecksum, using: format) {
                try data?.write(to: url, options: .atomic)
            }
        }
    }
    
    func saveScreenshots() throws {

        try saveAutoScreenshots()
        try saveUserScreenshots()
    }
    
    func loadAutoScreenshots() throws {
        
        track("Loading auto screenshots from disk (\(adfChecksum))")
        
        autoScreenshots = []
        for url in Screenshot.collectAutoFiles(checksum: adfChecksum) {
            if let screenshot = Screenshot.init(fromUrl: url) {
                autoScreenshots.append(screenshot)
            }
        }
        
        track("\(autoScreenshots.count) auto screenshots loaded")
    }
    
    func loadUserScreenshots() throws {
        
        track("Loading user screenshots from disk (\(adfChecksum))")
        
        userScreenshots = []
        for url in Screenshot.collectUserFiles(checksum: adfChecksum) {
            if let screenshot = Screenshot.init(fromUrl: url) {
                userScreenshots.append(screenshot)
            }
        }
        
        track("\(userScreenshots.count) user screenshots loaded")
    }
    
    func loadScreenshots() throws {

        try loadAutoScreenshots()
        try loadUserScreenshots()
    }
}
