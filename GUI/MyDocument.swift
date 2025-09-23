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

    static let workspace = UTType("de.dirkwhoffmann.retro.vamiga")!
    static let snapshot = UTType("de.dirkwhoffmann.retro.vasnap")!
    static let retrosh = UTType("de.dirkwhoffmann.retro.retrosh")!
    static let adf = UTType("public.retro.adf")!
    static let adz = UTType("public.retro.adz")!
    static let dms = UTType("public.retro.dms")!
    static let exe = UTType("public.retro.exe")!
    static let hdf = UTType("public.retro.hdf")!
    static let hdz = UTType("public.retro.hdz")!
    static let img = UTType("public.retro.img")!
}

@MainActor
class MyDocument: NSDocument {

    var pref: Preferences { return myAppDelegate.pref }

    // The window controller for this document
    var controller: MyController { return windowControllers.first as! MyController }
    
    var console: Console { return controller.renderer.console }
    var canvas: Canvas { return controller.renderer.canvas }
    
    // Optional media URL provided on app launch
    var mediaURL: URL?
    
    // The media manager for this document
    var mm: MediaManager!

    // Gateway to the core emulator
    var emu: EmulatorProxy!

    // Snapshots
    static let maxSnapshots: Int = 4
    private(set) var snapshots = ManagedArray<MediaFileProxy>(maxCount: maxSnapshots)
    // private(set) var snapshots = ManagedArray<MediaFileProxy>(maxSize: 512 * 1024 * 1024)

    //
    // Initializing
    //

    override init() {

        super.init()

        // Check for OS compatibility
        /*
        if #available(macOS 26, *) {

            showAlert(.unsupportedOSVersion)
            NSApp.terminate(self)
            return
        }
        */
        
        // Check for Metal support
        if MTLCreateSystemDefaultDevice() == nil {

            showAlert(.noMetalSupport)
            NSApp.terminate(self)
            return
        }

        // Create the media manager
        mm = MediaManager(with: self)

        // Register all GUI related user defaults
        EmulatorProxy.defaults.registerUserDefaults()
        
        // Load the user default settings
        EmulatorProxy.defaults.load()
        
        // Create an emulator instance
        emu = EmulatorProxy()
    }
 
    override open func makeWindowControllers() {
                
        debug(.lifetime)
        
        let controller = MyController(windowNibName: "MyDocument")
        controller.emu = emu
        self.addWindowController(controller)
    }
  
    //
    // Creating file proxys
    //

    func createMediaFileProxy(from url: URL, allowedTypes: [FileType]) throws -> MediaFileProxy {

        debug(.media, "Reading file \(url.lastPathComponent)")

        // Iterate through all allowed file types
        for type in allowedTypes {
            
            do {
                
                return try MediaFileProxy.make(with: url, type: type)
                
            } catch let error as AppError {
               
                if error.errorCode != .FILE_TYPE_MISMATCH { throw error }
            }
        }
        
        // None of the allowed types matched the file
        throw AppError(.FILE_TYPE_MISMATCH,
                      "The type of this file is not known to the emulator.")
    }


    //
    // Loading
    //
    
    
    override open func read(from url: URL, ofType typeName: String) throws {
             
        Swift.print("read(from url: \(url.path))")
        debug(.media)
    }
    
    override open func revert(toContentsOf url: URL, ofType typeName: String) throws {
        
        debug(.media)
        
        do {
            try mm.addMedia(url: url, allowedTypes: [.WORKSPACE])

        } catch let error as AppError {

            throw NSError(error: error)
        }
    }
    
    //
    // Saving
    //
  
    override func save(to url: URL, ofType typeName: String, for saveOperation: NSDocument.SaveOperationType) async throws {
            
        debug(.media, "url = \(url)")

        if typeName == "de.dirkwhoffmann.retro.vamiga" {

            do {
                // Save the workspace
                try emu.amiga.saveWorkspace(url: url)

                // Add a screenshot to the workspace bundle
                if let image = canvas.screenshot(source: .emulator, cutout: .visible) {
                
                    // Convert to target format
                    let data = image.representation(using: .png)
                    
                    // Save to file
                    try data?.write(to: url.appendingPathComponent("preview.png"))
                }
                
                // Save a plist file containing the machine properties
                saveMachineDescription(to: url.appendingPathComponent("machine.plist"))
                
                // Update the document's title and save status
                self.fileURL = url
                self.windowForSheet?.title = url.deletingPathExtension().lastPathComponent
                self.updateChangeCount(.changeCleared)
                
            } catch let error as AppError {
                
                // Swift.print("Error: \(error.what)")
                throw NSError(error: error)
            }
        }
    }
    
    func saveMachineDescription(to url: URL) {
        
        var dictionary: [String: Any] = [:]

        let bankMap = BankMap(rawValue: emu.get(.MEM_BANKMAP))
        let agnusRev = AgnusRevision(rawValue: emu.get(.AGNUS_REVISION))
        let deniseRev = DeniseRevision(rawValue: emu.get(.DENISE_REVISION))
        let agnusType = agnusRev == .OCS || agnusRev == .OCS_OLD ? "OCS" : "ECS"
        let deniseType = deniseRev == .OCS ? "OCS" : "ECS"
        let model = "Commodore Amiga " + (bankMap == .A500 ? "500" : bankMap == .A1000 ? "1000" : "2000")
        
        // Collect some info about the emulated machine
        dictionary["Model"] = model
        dictionary["Kickstart"] = String(cString: emu.mem.romTraits.title)
        dictionary["Agnus"] = agnusType
        dictionary["Denise"] = deniseType
        dictionary["Chip"] = emu.get(.MEM_CHIP_RAM)
        dictionary["Slow"] = emu.get(.MEM_SLOW_RAM)
        dictionary["Fast"] = emu.get(.MEM_FAST_RAM)
        dictionary["Version"] = EmulatorProxy.version()
        do {
            
            let data = try PropertyListSerialization.data(fromPropertyList: dictionary, format: .xml, options: 0)
            try data.write(to: url)
            
        } catch { }
    }
    
    //
    // Handling workspaces
    //
    
    func processWorkspaceFile(url: URL, force: Bool = false) throws {
        
        Swift.print("processWorkspaceFile \(url) force: \(force)")
        
        // Load workspace
        try emu.amiga.loadWorkspace(url: url)
        
        // Update the document's title and save status
        self.fileURL = url
        self.windowForSheet?.title = url.deletingPathExtension().lastPathComponent
        self.updateChangeCount(.changeCleared)
        
        // Scan directory for additional media files
        let supportedTypes: [String : FileType] =
        ["adf": .ADF, "adz": .ADZ, "dms": .DMS, "exe": .EXE, "img": .IMG, "hdf": .HDF, "hdz": .HDZ, "st": .ST]
        let exclude = ["df0", "df1", "df2", "df3", "hd0", "hd1", "hd2", "hd3"]

        let contents = try FileManager.default.contentsOfDirectory(at: url, includingPropertiesForKeys: nil)
        for file in contents {
            if !exclude.contains(url.deletingPathExtension().lastPathComponent) {
                if let type = supportedTypes[file.pathExtension.lowercased()] {
                    mm.noteNewRecentlyOpenedURL(file, type: type)
                }
            }
        }
    }

    //
    // Handling snapshots
    //

    func processSnapshotFile(url: URL) throws {

        let file = try createMediaFileProxy(from: url, allowedTypes: [.SNAPSHOT])
        try processSnapshotFile(file: file)
    }
    
    func processSnapshotFile(file: MediaFileProxy) throws {

        try emu.amiga.loadSnapshot(file)
        appendSnapshot(file: file)
    }

    @discardableResult
    func appendSnapshot(file: MediaFileProxy) -> Bool {

        // Remove the oldest entry if applicable
        if snapshots.full && pref.snapshotAutoDelete { snapshots.remove(at: 0) }

        // Only proceed if there is space left
        if snapshots.full { return false }

        // Append the snapshot
        snapshots.append(file, size: file.size)
        return true
    }

    //
    // Handling scripts
    //

    func processScriptFile(url: URL, force: Bool = false) throws {
        
        let file = try createMediaFileProxy(from: url, allowedTypes: [.SCRIPT])
        try processScriptFile(file: file, force: force)
    }

    func processScriptFile(file: MediaFileProxy, force: Bool = false) throws {
        
        console.runScript(script: file)
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
        mm.noteNewRecentlyExportedDiskURL(url, df: nr)
        
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
        mm.noteNewRecentlyExportedHdrURL(url, hd: nr)

        debug(.media, "Hard Drive exported successfully")
    }

    func export(fileProxy: MediaFileProxy, to url: URL) throws {

        debug(.media, "Exporting to \(url)")
        try fileProxy.writeToFile(url: url)
    }
}
