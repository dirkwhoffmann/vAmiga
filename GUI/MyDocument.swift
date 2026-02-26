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
    
    // The window controller of this document
    var controller: MyController { return windowControllers.first as! MyController }
    
    var console: Console { return controller.renderer.console }
    var canvas: Canvas { return controller.renderer.canvas }
    var pref: Preferences { return myAppDelegate.pref }
    
    // Optional media URL provided on app launch
    var launchURL: URL?
    
    // The media manager for this document
    var mm: MediaManager!
    
    // Gateway to the core emulator
    var emu: EmulatorProxy?
    
    // Snapshots
    static let maxSnapshots: Int = 16
    private(set) var snapshots = ManagedArray<SnapshotProxy>(maxCount: maxSnapshots)

    //
    // Initializing
    //
    
    override init() {
        
        loginfo(.lifetime)
        
        super.init()
        
        // Check for OS compatibility
        if #available(macOS 27, *) {
            
            showAlert(.unsupportedOSVersion)
            NSApp.terminate(self)
            return
        }
        
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
    
    override class var autosavesInPlace: Bool {
        
        return false
    }
    
    override open func makeWindowControllers() {
        
        loginfo(.lifetime)
        
        let controller = MyController(windowNibName: "MyDocument")
        self.addWindowController(controller)
    }
    
    func shutDown() {
        
        loginfo(.shutdown, "Remove proxy...")
        
        emu?.kill()
        emu = nil
        
        loginfo(.shutdown, "Done")
    }
    
    //
    // Loading
    //
    
    override open func read(from url: URL, ofType typeName: String) throws {
        
        loginfo(.media)
    }
    
    override open func revert(toContentsOf url: URL, ofType typeName: String) throws {
        
        loginfo(.media)
        
        do {
            // try mm.mount(url: url, allowedTypes: [.WORKSPACE])
            try processWorkspaceFile(url: url)

        } catch let error as AppError {
            
            throw NSError(error: error)
        }
    }
    
    //
    // Saving
    //
    
    override func save(to url: URL, ofType typeName: String, for saveOperation: NSDocument.SaveOperationType) async throws {
        
        loginfo(.media, "url = \(url)")
        
        if typeName == "de.dirkwhoffmann.retro.vamiga" {
            
            do {
                // Save the workspace
                try emu?.amiga.saveWorkspace(url: url)
                
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
                
                throw NSError(error: error)
            }
        }
    }
    
    func saveMachineDescription(to url: URL) {
        
        guard let emu = emu else { return }
        
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

        // Load workspace
        try emu?.amiga.loadWorkspace(url: url)
        
        // Update the document's title and save status
        self.fileURL = url
        self.windowForSheet?.title = url.deletingPathExtension().lastPathComponent
        self.updateChangeCount(.changeCleared)
        
        // Scan directory for additional media files
        let supportedTypes: [String : ImageFormat] =
        ["adf": .ADF, "dms": .DMS, "exe": .EXE, "img": .IMG, "hdf": .HDF, "hdz": .HDZ, "st": .ST]
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

        try emu?.amiga.loadSnapshot(url: url)
    }

    @discardableResult
    func appendSnapshot(file: SnapshotProxy) -> Bool {
        
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

        try console.runScript(url: url)
     }

    //
    // Exporting disks
    //
    
    func export(drive nr: Int, to url: URL) throws {

        guard let dfn = emu?.df(nr) else { return }

        let ext = url.pathExtension.uppercased();
        if !["ADF", "IMG", "IMA"].contains(ext) {

            warn("Invalid path extension")
            return
        }

        try dfn.writeToFile(url: url)
        dfn.setFlag(.MODIFIED, value: false)
        mm.noteNewRecentlyExportedDiskURL(url, df: nr)

        loginfo(.media, "Disk exported successfully")
    }
    
    func export(hardDrive nr: Int, to url: URL) throws {
        
        guard let hdn = emu?.hd(nr) else { return }

        let ext = url.pathExtension.uppercased();
        if !["HDF"].contains(ext) {

            warn("Invalid path extension")
            return
        }

        try hdn.writeToFile(url: url)
        hdn.setFlag(.MODIFIED, value: false)
        mm.noteNewRecentlyExportedHdrURL(url, hd: nr)

        loginfo(.media, "Hard Drive exported successfully")
    }
}
