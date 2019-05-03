// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

class MyDocument : NSDocument {

    /*
     Emulator proxy object. This object is an Objective-C bridge between
     the GUI (written in Swift) an the core emulator (written in C++).
     */
    var amiga: AmigaProxy!

    /*
     Emulator proxy object. This object is an Objective-C bridge between
     the GUI (written in Swift) an the core emulator (written in C++).
     */
    // var c64: C64Proxy!
    
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
    var amigaAttachment: AmigaFileProxy? = nil
    
    override init() {
        
        track()
        super.init()
        
        // Register standard user defaults
        MyController.registerUserDefaults()
        
        // Create emulator instance
        amiga = AmigaProxy()
        // amiga.makeActiveInstance()
        
        // Install the AROS Kickstart replacement per default
        amiga.mem.loadKickRom(fromBuffer: NSDataAsset(name: "aros.rom")?.data)
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
            throw NSError(domain: "VirtualC64", code: 0, userInfo: nil)
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
    
    /// Creates an attachment from a URL
    func createAmigaAttachment(from url: URL) throws {
        
        track("Creating attachment from URL \(url.lastPathComponent).")
        
        // Try to create the attachment
        let fileWrapper = try FileWrapper.init(url: url)
        let pathExtension = url.pathExtension.uppercased()
        try createAmigaAttachment(from: fileWrapper, ofType: pathExtension)
        
        // Put URL in recently used URL lists
        myAppDelegate.noteNewRecentlyUsedURL(url)
    }
    
    // OLD CODE:
    /*
    func createAttachment(from url: URL) throws {
    
        track("Creating attachment from URL \(url.lastPathComponent).")

        // Try to create the attachment
        let fileWrapper = try FileWrapper.init(url: url)
        let pathExtension = url.pathExtension.uppercased()
        try createAttachment(from: fileWrapper, ofType: pathExtension)

        // Put URL in recently used URL lists
        noteNewRecentlyUsedURL(url)
    }
    */
    
    /// Creates an attachment from a file wrapper
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
        
        switch (typeName) {
            
        case "VAMIGA":
            // Check for outdated snapshot formats
            if AmigaSnapshotProxy.isUnsupportedSnapshot(buffer, length: length) {
                throw NSError.snapshotVersionError(filename: filename)
            }
            amigaAttachment = AmigaSnapshotProxy.make(withBuffer: buffer, length: length)
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

    // OLD CODE
    /*
    fileprivate func createAttachment(from fileWrapper: FileWrapper,
                                      ofType typeName: String) throws {
        
        guard let filename = fileWrapper.filename else {
            throw NSError(domain: "VirtualC64", code: 0, userInfo: nil)
        }
        guard let data = fileWrapper.regularFileContents else {
            throw NSError(domain: "VirtualC64", code: 0, userInfo: nil)
        }
        
        let buffer = (data as NSData).bytes
        let length = data.count
        var openAsUntitled = true
        
        track("Read \(length) bytes from file \(filename).")
        
        switch (typeName) {
    
        case "CRT":
            // Check for unsupported cartridge types
            if CRTFileProxy.isUnsupportedCRTBuffer(buffer, length: length) {
                let type = CRTFileProxy.typeName(ofCRTBuffer: buffer, length: length)!
                throw NSError.unsupportedCartridgeError(filename: filename, type: type)
            }
            attachment = CRTFileProxy.make(withBuffer: buffer, length: length)
            
        case "TAP":
            attachment = TAPFileProxy.make(withBuffer: buffer, length: length)
            
        case "T64":
            attachment = T64FileProxy.make(withBuffer: buffer, length: length)
            
        case "PRG":
            attachment = PRGFileProxy.make(withBuffer: buffer, length: length)
            
        case "D64":
            attachment = D64FileProxy.make(withBuffer: buffer, length: length)
            
        case "P00":
            attachment = P00FileProxy.make(withBuffer: buffer, length: length)
            
        case "G64":
            attachment = G64FileProxy.make(withBuffer: buffer, length: length)

        default:
            throw NSError.unsupportedFormatError(filename: filename)
        }
        
        if attachment == nil {
            throw NSError.corruptedFileError(filename: filename)
        }
        if openAsUntitled {
            fileURL = nil
        }
        attachment!.setPath(filename)
    }
    */
    
    //
    // Processing attachments
    //
    
    @discardableResult
    func mountAmigaAttachment() -> Bool {
        
        // guard let controller = myController else { return false }
        
        switch(amigaAttachment) {

        case _ as AmigaSnapshotProxy:
            
            amiga.load(fromSnapshot: amigaAttachment as? AmigaSnapshotProxy)
            return true
       
        case _ as ADFFileProxy:
            
            if let drive = myController?.dragAndDropDrive {
                drive.insertDisk(amigaAttachment as? ADFFileProxy)
            } else {
                runDiskMountDialog()
            }
            
        default:
            break
        }
        
        return true
    }
    
    // OLD CODE
    /*
    @discardableResult
    func mountAttachment() -> Bool {

        guard let controller = myController else { return false }
        
        // Determine action to perform and text to type
        var action = AutoMountAction.openBrowser
        var autoTypeText: String?

        func getAction(_ type: String) {
            action = controller.autoMountAction[type] ?? action
            if action != .openBrowser && (controller.autoType[type] ?? false) {
                autoTypeText = controller.autoTypeText[type]
            }
        }

        switch(attachment) {
        case _ as SnapshotProxy: c64.flash(attachment); return true
        case _ as D64FileProxy, _ as G64FileProxy: getAction("D64")
        case _ as PRGFileProxy, _ as P00FileProxy: getAction("PRG")
        case _ as T64FileProxy: getAction("T64")
        case _ as TAPFileProxy: getAction("TAP")
        case _ as CRTFileProxy: getAction("CRT")
        default: return false
        }
    
        // Check if the emulator has just been startet. In that case, we have
        // to wait until the Kernal boot routine has been executed. Otherwise,
        // the C64 would ignore everything we are doing here.
        let delay = (c64.cpu.cycle() < 3000000) ? 2.0 : 0.0

        // Execute asynchronously ...
        DispatchQueue.main.asyncAfter(deadline: .now() + delay, execute: {
            self.mountAttachment(action: action, text: autoTypeText)
        })
        
        return true
    }
    
    func mountAttachment(action: AutoMountAction, text: String?) {
        
        // Perform action
        track("Action = \(action)")
        switch action {
        case .insertIntoDrive8: mountAttachmentAsDisk(drive: 1)
        case .insertIntoDrive9: mountAttachmentAsDisk(drive: 2)
        case .flashFirstFile: flashAttachmentIntoMemory()
        case .insertIntoDatasette: mountAttachmentAsTape()
        case .attachToExpansionPort: mountAttachmentAsCartridge()
        default: break
        }
        
        // Type text
        if text != nil {
            track("Auto typing: \(text!)")
            myController?.keyboardcontroller.type(text! + "\n")
        }
    }
    */
    
    func runDiskMountDialog() {
        let nibName = NSNib.Name("DiskMountDialog")
        let controller = DiskMountController.init(windowNibName: nibName)
        controller.showSheet()
    }
    
    /*
    @discardableResult
    func mountAttachmentAsDisk(drive nr: Int) -> Bool {
        
        if let archive = attachment as? AnyArchiveProxy {
            
            if proceedWithUnexportedDisk(drive: nr) {
                
                let parent = windowForSheet!.windowController as! MyController
                parent.changeDisk(archive, drive: nr)
                return true
            }
        }
        return false
    }
    
     */
    
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
        
        track("Trying to write \(typeName) file.")
        
        if typeName == "vAmiga" {
            
            // Take snapshot
            if let snapshot = AmigaSnapshotProxy.make(withAmiga: amiga) {

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

        // TODO: Convert disk to ADF format
        // guard let adf = ADFFileProxy.make(withDisk: drive.disk) else {
        guard let adf = ADFFileProxy.make(with: DISK_35_DD) else {
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
    
    /*
    func exportOld(drive nr: Int, to url: URL, ofType typeName: String) -> Bool {
        
        track("url = \(url) typeName = \(typeName)")
        assert(["ADF", "D64", "T64", "PRG", "P00", "G64"].contains(typeName))
        
        let drive = c64.drive(nr)!
        
        // Convert disk to a D64 archive
        // guard let d64archive = D64Proxy.make(withDrive: drive) else {
        guard let d64archive = D64FileProxy.make(withDisk: drive.disk) else {
            return false
        }
        
        // Convert the D64 archive into the target format
        var archive: AnyArchiveProxy?
        switch typeName.uppercased() {
        case "D64":
            track("Exporting to D64 format")
            archive = d64archive
        
        case "G64":
            track("Exporting to G64 format")
            archive = G64FileProxy.make(withDisk: drive.disk)

        case "ADF":
            
            //
            // TODO: IMPLEMENT THIS
            //
            
            track("Exporting to ADF format. TO BE IMPLEMENTED")
            // archive = ADFFileProxy.make(withDisk: drive.disk)

        case "T64":
            track("Exporting to T64 format")
            archive = T64FileProxy.make(withAnyArchive: d64archive)
            
        case "PRG":
            track("Exporting to PRG format")
            if d64archive.numberOfItems() > 1  {
                showDiskHasMultipleFilesAlert(format: "PRG")
            }
            archive = PRGFileProxy.make(withAnyArchive: d64archive)
            
        case "P00":
            track("Exporting to P00 format")
            if d64archive.numberOfItems() > 1  {
                showDiskHasMultipleFilesAlert(format: "P00")
            }
            archive = P00FileProxy.make(withAnyArchive: d64archive)
            
        default:
            fatalError()
        }
        
        // Serialize archive
        let data = NSMutableData.init(length: archive!.sizeOnDisk())!
        archive!.write(toBuffer: data.mutableBytes)
        
        // Write to file
        if !data.write(to: url, atomically: true) {
            showExportErrorAlert(url: url)
            return false
        }
        
        // Mark disk as "not modified"
        drive.setModifiedDisk(false)
        
        // Remember export URL
        noteNewRecentlyExportedDiskURL(url, drive: nr)
        return true
    }
    */
    
    @discardableResult
    func export(drive nr: Int, to url: URL?) -> Bool {
        
        if let suffix = url?.pathExtension {
            return export(drive: nr, to: url!, ofType: suffix.uppercased())
        } else {
            return false
        }
    }
}
