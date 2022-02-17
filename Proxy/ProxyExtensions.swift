// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

//
// Factory extensions
//

extension Proxy {
    
    static func make<T: MakeWithBuffer>(buffer: UnsafeRawPointer, length: Int) throws -> T {
        
        track()
        
        let exc = ExceptionWrapper()
        let obj = T.make(withBuffer: buffer, length: length, exception: exc)
        if exc.errorCode != ErrorCode.OK { throw VAError(exc) }
        return obj!
    }
    
    static func make<T: MakeWithFile>(url: URL) throws -> T {
        
        let exc = ExceptionWrapper()
        let obj = T.make(withFile: url.path, exception: exc)
        if exc.errorCode != ErrorCode.OK { throw VAError(exc) }
        return obj!
    }

    static func make<T: MakeWithDrive>(drive: DriveProxy) throws -> T {
        
        let exc = ExceptionWrapper()
        let obj = T.make(withDrive: drive, exception: exc)
        if exc.errorCode != ErrorCode.OK { throw VAError(exc) }
        return obj!
    }

    static func make<T: MakeWithFileSystem>(fs: FSDeviceProxy) throws -> T {
        
        let exc = ExceptionWrapper()
        let obj = T.make(withFileSystem: fs, exception: exc)
        if exc.errorCode != ErrorCode.OK { throw VAError(exc) }
        return obj!
    }
}

//
// Exception passing
//

extension AmigaProxy {

    func isReady() throws {
        
        let exception = ExceptionWrapper()
        isReady(exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    /*
    func powerOn() throws {
        
        let exception = ExceptionWrapper()
        power(on: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    */
    
    func run() throws {
        
        let exception = ExceptionWrapper()
        run(exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    
    func loadSnapshot(_ proxy: SnapshotProxy) throws {

        let exception = ExceptionWrapper()
        loadSnapshot(proxy, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
}

extension MemProxy {
 
    func loadRom(_ proxy: RomFileProxy) throws {

        let exception = ExceptionWrapper()
        loadRom(proxy, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    
    func loadRom(buffer: Data) throws {

        let exception = ExceptionWrapper()
        loadRom(fromBuffer: buffer, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    
    func loadRom(_ url: URL) throws {

        let exception = ExceptionWrapper()
        loadRom(fromFile: url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    
    func loadExt(_ proxy: ExtendedRomFileProxy) throws {

        let exception = ExceptionWrapper()
        loadExt(proxy, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func loadExt(buffer: Data) throws {

        let exception = ExceptionWrapper()
        loadExt(fromBuffer: buffer, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    
    func loadExt(_ url: URL) throws {

        let exception = ExceptionWrapper()
        loadExt(fromFile: url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func saveRom(_ url: URL) throws {

        let exception = ExceptionWrapper()
        saveRom(url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func saveWom(_ url: URL) throws {

        let exception = ExceptionWrapper()
        saveWom(url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func saveExt(_ url: URL) throws {

        let exception = ExceptionWrapper()
        saveExt(url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
}

extension DriveProxy {

    /*
    func insert(file: DiskFileProxy) throws {
        
        let exception = ExceptionWrapper()
        insert(file, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    */
    
    func swap(file: DiskFileProxy) throws {
        
        let exception = ExceptionWrapper()
        swap(file, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func insertNew() throws {
        
        let exception = ExceptionWrapper()
        insertNew(exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
}

extension AmigaFileProxy {
    
    @discardableResult
    func writeToFile(url: URL) throws -> Int {
        
        let exception = ExceptionWrapper()
        let result = write(toFile: url.path, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
        
        return result
    }
}

extension ADFFileProxy {

    static func make (diameter: DiskDiameter, density: DiskDensity) throws -> ADFFileProxy {

        let exception = ExceptionWrapper()
        let result = ADFFileProxy.make(with: diameter, density: density, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
        
        return result!

    }
}

extension FSDeviceProxy {
            
    static func make(withADF adf: ADFFileProxy) throws -> FSDeviceProxy {
        
        let exception = ExceptionWrapper()
        let result = FSDeviceProxy.make(withADF: adf, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
        
        return result!
    }
    
    func export(url: URL) throws {
            
        let exception = ExceptionWrapper()
        export(url.path, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
}

//
// Other extensions
//

public extension AmigaProxy {
    
    func df(_ nr: Int) -> DriveProxy? {
        
        switch nr {
            
        case 0: return df0
        case 1: return df1
        case 2: return df2
        case 3: return df3
            
        default:
            return nil
        }
    }

    func dh(_ nr: Int) -> HardDriveProxy? {
        
        switch nr {
            
        case 0: return dh0

        default:
            return nil
        }
    }

    func df(_ item: NSButton!) -> DriveProxy? {
        
        return df(item.tag)
    }
    
    func df(_ item: NSMenuItem!) -> DriveProxy? {
        
        return df(item.tag)
    }
    
    func image(data: UnsafeMutablePointer<UInt8>?, size: NSSize) -> NSImage {
        
        var bitmap = data
        let width = Int(size.width)
        let height = Int(size.height)
                
        let imageRep = NSBitmapImageRep(bitmapDataPlanes: &bitmap,
                                        pixelsWide: width,
                                        pixelsHigh: height,
                                        bitsPerSample: 8,
                                        samplesPerPixel: 4,
                                        hasAlpha: true,
                                        isPlanar: false,
                                        colorSpaceName: NSColorSpaceName.calibratedRGB,
                                        bytesPerRow: 4 * width,
                                        bitsPerPixel: 32)
        
        let image = NSImage(size: (imageRep?.size)!)
        image.addRepresentation(imageRep!)
        // image.makeGlossy()
        
        return image
    }
}

public extension DriveProxy {
    
    var icon: NSImage {

        var name: String

        if hasWriteProtectedDisk() {
            name = isModifiedDisk ? "diskUPTemplate" : "diskPTemplate"
        } else {
            name = isModifiedDisk ? "diskUTemplate" : "diskTemplate"
        }

        return NSImage(named: name)!
    }
}

extension DiskFileProxy {
    
    func icon(protected: Bool) -> NSImage {
        
        let density = diskDensity

        let name = (density == .HD ? "hd" : "dd") +
        (type == .IMG ? "_dos" : dos == .NODOS ? "_other" : "_adf") +
        (protected ? "_protected" : "")

        return NSImage(named: name)!
    }
    
    var layoutInfo: String {
                
        var result = numSides == 1 ? "Single sided" : "Double sided"

        if diskDensity == .SD { result += ", single density" }
        if diskDensity == .DD { result += ", double density" }
        if diskDensity == .HD { result += ", high density" }

        result += " disk, \(numTracks) tracks"
        if numSectors > 0 { result += " with \(numSectors) sectors each" }
        
        return result
    }
    
    var bootInfo: String {
        
        let name = bootBlockName!
        
        if bootBlockType == .VIRUS {
            return "Contagious boot block (\(name))"
        } else {
            return name
        }
    }
}

extension HDFFileProxy {
    
    func icon() -> NSImage {
        
        return NSImage(named: "hdf")!
    }
    
    var layoutInfo: String {
        
        let capacity = numBlocks / 2000
        return "\(capacity) MB (\(numBlocks) sectors)"
    }
    
    var bootInfo: String {

        return ""
    }
}

public extension RemoteManagerProxy {
    
    var icon: NSImage? {

        if numConnected > 0 {
            return NSImage(named: "srvConnectTemplate")!
        }
        if numListening > 0 {
            return NSImage(named: "srvListenTemplate")!
        }
        if numLaunching > 0 {
            return NSImage(named: "srvLaunchTemplate")!
        }
        if numErroneous > 0 {
            return NSImage(named: "srvErrorTemplate")!
        }

        return nil
    }
}
