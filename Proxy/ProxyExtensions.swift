// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Darwin

//
// Factory extensions
//

extension MakeWithBuffer {
    
    static func makeWith(buffer: UnsafeRawPointer, length: Int) throws -> Self {
                
        let exc = ExceptionWrapper()
        let obj = make(withBuffer: buffer, length: length, exception: exc)
        if exc.errorCode != ErrorCode.OK { throw VAError(exc) }
        return obj!
    }

    static func make(with data: Data) throws -> Self {
        
        let exception = ExceptionWrapper()
        let obj = make(with: data, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
        return obj!
    }

    private static func make(with data: Data, exception: ExceptionWrapper) -> Self? {
        
        return data.withUnsafeBytes { uwbp -> Self? in
            
            return make(withBuffer: uwbp.baseAddress!, length: uwbp.count, exception: exception)
        }
    }
}

extension MakeWithFile {
    
    static func make(with url: URL) throws -> Self {
        
        let exc = ExceptionWrapper()
        let obj = make(withFile: url.path, exception: exc)
        if exc.errorCode != ErrorCode.OK { throw VAError(exc) }
        return obj!
    }
}

extension MakeWithDrive {
    
    static func make(with drive: FloppyDriveProxy) throws -> Self {
        
        let exc = ExceptionWrapper()
        let obj = make(withDrive: drive, exception: exc)
        if exc.errorCode != ErrorCode.OK { throw VAError(exc) }
        return obj!
    }
}

extension MakeWithHardDrive {
    
    static func make(with hdr: HardDriveProxy) throws -> Self {
        
        let exc = ExceptionWrapper()
        let obj = make(withHardDrive: hdr, exception: exc)
        if exc.errorCode != ErrorCode.OK { throw VAError(exc) }
        return obj!
    }
}

extension MakeWithFileSystem {
    
    static func make(with fs: FileSystemProxy) throws -> Self {
        
        let exc = ExceptionWrapper()
        let obj = make(withFileSystem: fs, exception: exc)
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

extension FloppyDriveProxy {

    func swap(file: FloppyFileProxy) throws {
        
        let exception = ExceptionWrapper()
        swap(file, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    
    func insertNew(fileSystem: FSVolumeType, bootBlock: BootBlockId, name: String) throws {
        
        let exception = ExceptionWrapper()
        insertNew(fileSystem, bootBlock: bootBlock, name: name, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
}

extension HardDriveProxy {

    func attach(hdf: HDFFileProxy) throws {
        
        let exception = ExceptionWrapper()
        attach(hdf, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func attach(c: Int, h: Int, s: Int, b: Int) throws {
        
        let exception = ExceptionWrapper()
        attach(c, h: h, s: s, b: b, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func format(fs: FSVolumeType, name: String) throws {
        
        let exception = ExceptionWrapper()
        format(fs, name: name, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func changeGeometry(c: Int, h: Int, s: Int, b: Int = 512) throws {
        
        let exception = ExceptionWrapper()
        changeGeometry(c, h: h, s: s, b: b, exception: exception)
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

    static func make (diameter: Diameter, density: Density) throws -> ADFFileProxy {

        let exception = ExceptionWrapper()
        let result = ADFFileProxy.make(with: diameter, density: density, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
        
        return result!

    }
}

extension HDFFileProxy {
    
    @discardableResult
    func writeToFile(url: URL, partition nr: Int) throws -> Int {
        
        let exception = ExceptionWrapper()
        let result = write(toFile: url.path, partition: nr, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
        
        return result
    }
}

extension FileSystemProxy {
            
    static func make(withADF adf: ADFFileProxy) throws -> FileSystemProxy {
        
        let exception = ExceptionWrapper()
        let result = FileSystemProxy.make(withADF: adf, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
        
        return result!
    }

    static func make(withHDF hdf: HDFFileProxy, partition nr: Int) throws -> FileSystemProxy {
        
        let exception = ExceptionWrapper()
        let result = FileSystemProxy.make(withHDF: hdf, partition: nr, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
        
        return result!
    }

    func export(url: URL) throws {
            
        let exception = ExceptionWrapper()
        export(url.path, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
}

extension RecorderProxy {
    
    func startRecording(rect: NSRect, rate: Int, ax: Int, ay: Int) throws {
        
        let exception = ExceptionWrapper()
        startRecording(rect, bitRate: rate, aspectX: ax, aspectY: ay, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
}

//
// Other extensions
//

public extension AmigaProxy {
    
    func df(_ nr: Int) -> FloppyDriveProxy? {
        
        switch nr {
            
        case 0: return df0
        case 1: return df1
        case 2: return df2
        case 3: return df3
            
        default:
            return nil
        }
    }

    func df(_ item: NSButton!) -> FloppyDriveProxy? { return df(item.tag) }
    func df(_ item: NSMenuItem!) -> FloppyDriveProxy? { return df(item.tag) }

    func hd(_ nr: Int) -> HardDriveProxy? {
        
        switch nr {

        case 0: return hd0
        case 1: return hd1
        case 2: return hd2
        case 3: return hd3

        default:
            return nil
        }
    }

    func hd(_ item: NSButton!) -> HardDriveProxy? { return hd(item.tag) }
    func hd(_ item: NSMenuItem!) -> HardDriveProxy? { return hd(item.tag) }
    
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

extension FloppyDriveProxy {
    
    var templateIcon: NSImage? {

        var name: String
        
        if !hasDisk { return nil }
        
        if hasProtectedDisk {
            name = hasModifiedDisk ? "diskUPTemplate" : "diskPTemplate"
        } else {
            name = hasModifiedDisk ? "diskUTemplate" : "diskTemplate"
        }
        
        return NSImage(named: name)!
    }
    
    var ledIcon: NSImage? {
        
        if !isConnected { return nil }

        if motor {
            if writing {
                return NSImage(named: "ledRed")
            } else {
                return NSImage(named: "ledGreen")
            }
        } else {
            return NSImage(named: "driveLedOff")
        }
    }
}

extension HardDriveProxy {
    
    var templateIcon: NSImage? {
        
        return NSImage(named: "hdrTemplate")!
    }
    
    var ledIcon: NSImage? {
        
        if !isConnected { return nil }
        
        switch state {
            
        case .IDLE: return NSImage(named: "ledGrey")
        case .READING: return NSImage(named: "ledGreen")
        case .WRITING: return NSImage(named: "ledRed")
            
        default: fatalError()
        }
    }
}

extension FloppyFileProxy {
    
    func icon(protected: Bool) -> NSImage {
        
        let name = (diskDensity == .HD ? "hd" : "dd") +
        (type == .IMG ? "_dos" : dos == .NODOS ? "_other" : "_adf") +
        (protected ? "_protected" : "")

        return NSImage(named: name)!
    }

    var typeInfo: String {

        var result = ""

        if diskType == .INCH_35 { result += "3.5\"" }
        if diskType == .INCH_525 { result += "5.25\"" }
        
        return result
    }
    
    var layoutInfo: String {
                
        var result = ""

        if numHeads == 1 { result += "Single sided, " }
        if numHeads == 2 { result += "Double sided, " }
        if diskDensity == .SD { result += "Single density" }
        if diskDensity == .DD { result += "Double density" }
        if diskDensity == .HD { result += "High density" }
        
        return result
    }

    /*
    var layoutInfo: String {
                
        var result = numHeads == 1 ? "Single sided" : "Double sided"

        if diskDensity == .SD { result += ", single density" }
        if diskDensity == .DD { result += ", double density" }
        if diskDensity == .HD { result += ", high density" }

        result += " disk, \(numTracks) tracks"
        if numSectors > 0 { result += " with \(numSectors) sectors each" }
        
        return result
    }
    */
        
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
        
        let capacity = sizeAsString!
        return "\(capacity), \(numBlocks) sectors"
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
