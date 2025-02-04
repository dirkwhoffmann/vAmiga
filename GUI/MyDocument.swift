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
    static let adf = UTType("de.dirkwhoffmann.retro.adf")!
    static let adz = UTType("de.dirkwhoffmann.retro.adf")!
    static let dms = UTType("de.dirkwhoffmann.retro.dms")!
    static let exe = UTType("de.dirkwhoffmann.retro.exe")!
    static let hdf = UTType("de.dirkwhoffmann.retro.hdf")!
    static let hdz = UTType("de.dirkwhoffmann.retro.hdz")!
    static let img = UTType("de.dirkwhoffmann.retro.img")!
}

@MainActor
class MyDocument: NSDocument {

    var pref: Preferences { return myAppDelegate.pref }
    var parent: MyController { return windowControllers.first as! MyController }
    var console: Console { return parent.renderer.console }
    var canvas: Canvas { return parent.renderer.canvas }
    
    // Gateway to the core emulator
    var emu: EmulatorProxy!

    // Snapshots
    private(set) var snapshots = ManagedArray<MediaFileProxy>(maxSize: 512 * 1024 * 1024)

    //
    // Initializing
    //

    override init() {
        
        debug(.lifetime)
        
        super.init()

        // Check for Metal support
        if MTLCreateSystemDefaultDevice() == nil {

            showAlert(.noMetalSupport)
            NSApp.terminate(self)
            return
        }
                
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

        // If the provided URL points to compressed file, decompress it first
        let newUrl = url.unpacked(maxSize: 2048 * 1024)

        // Iterate through all allowed file types
        for type in allowedTypes {

            do {
                switch type {

                case .WORKSPACE, .SNAPSHOT, .SCRIPT, .ADF, .EADF, .IMG, .ST, .DMS, .EXE, .DIR, .HDF:

                    return try MediaFileProxy.make(with: newUrl, type: type)

                default:
                    break
                }

            } catch let error as VAException {
                if error.errorCode != .FILE_TYPE_MISMATCH {
                    throw error
                }
            }
        }

        // None of the allowed types matched the file
        throw VAException(.FILE_TYPE_MISMATCH,
                      "The type of this file is not known to the emulator.")
    }


    //
    // Loading
    //
    
    
    override open func read(from url: URL, ofType typeName: String) throws {
             
        debug(.media)
    }
    
    override open func revert(toContentsOf url: URL, ofType typeName: String) throws {
        
        debug(.media)
        
        do {
            try addMedia(url: url, allowedTypes: [.WORKSPACE])

        } catch let error as VAException {

            throw NSError(error: error)
        }
    }
    
    //
    // Saving
    //
  
    override func save(to url: URL, ofType typeName: String, for saveOperation: NSDocument.SaveOperationType) async throws {
            
        debug(.media)

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
                
                saveMachineDescription(to: url.appendingPathComponent("machine.plist"))
                
            } catch let error as VAException {
                
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
    // Handling media files
    //

    func addMedia(url: URL,
                  allowedTypes types: [FileType] = FileType.all,
                  df: Int = 0,
                  hd: Int = 0,
                  force: Bool = false,
                  remember: Bool = true) throws {
        
        let file = try createMediaFileProxy(from: url, allowedTypes: types)

        // Remember the URL if requested
        if remember {

            switch file.type {

            case .WORKSPACE, .SNAPSHOT:
                break

            case .ADF, .EADF, .HDF, .EXE, .IMG, .ST:
                myAppDelegate.noteNewRecentlyInsertedDiskURL(url)

            default:
                break
            }
        }

        try addMedia(proxy: file, url: url, df: df, hd: hd, force: force)
    }
    
    func addMedia(proxy: MediaFileProxy,
                  url: URL? = nil,
                  df: Int = 0,
                  hd: Int = 0,
                  force: Bool = false) throws {

        switch proxy.type {

        case .WORKSPACE:

            try processWorkspaceFile(url: url!)

        case .SNAPSHOT:

            try processSnapshotFile(proxy)

        case .SCRIPT:

            console.runScript(script: proxy)
            break

        case .HDF:

            try attach(hd: hd, file: proxy, force: force)
            break

        case .ADF, .DMS, .EXE, .EADF, .IMG, .ST:

            try insert(df: df, file: proxy, force: force)

        default:
            break
        }
    }

    func processWorkspaceFile(url: URL, force: Bool = false) throws {

        Swift.print("processWorkspaceFile \(url) force: \(force)")

        try emu.amiga.loadWorkspace(url: url)
    }

    func processSnapshotFile(_ proxy: MediaFileProxy, force: Bool = false) throws {

        try emu.amiga.loadSnapshot(proxy)
        snapshots.append(proxy, size: proxy.size)
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
        myAppDelegate.noteNewRecentlyExportedDiskURL(url, df: nr)
        
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
        myAppDelegate.noteNewRecentlyExportedHdrURL(url, hd: nr)

        debug(.media, "Hard Drive exported successfully")
    }

    func export(fileProxy: MediaFileProxy, to url: URL) throws {

        debug(.media, "Exporting to \(url)")
        try fileProxy.writeToFile(url: url)
    }
}
