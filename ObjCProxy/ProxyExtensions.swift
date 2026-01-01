// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension AmigaProxy {

    func loadSnapshot(_ proxy: SnapshotProxy) throws {

        let exception = ExceptionWrapper()
        loadSnapshot(proxy, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func loadSnapshot(url: URL) throws {

        let exception = ExceptionWrapper()
        loadSnapshot(from: url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func saveSnapshot(url: URL) throws {

        let exception = ExceptionWrapper()
        saveSnapshot(to: url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func loadWorkspace(url: URL) throws {
        
        let exception = ExceptionWrapper()
        loadWorkspace(url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func saveWorkspace(url: URL) throws {
        
        let exception = ExceptionWrapper()
        saveWorkspace(url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }
}

extension MakeWithBuffer {
    
    static func makeWith(buffer: UnsafeRawPointer, length: Int) throws -> Self {
                
        let exc = ExceptionWrapper()
        let obj = make(withBuffer: buffer, length: length, exception: exc)
        if exc.fault != 0 { throw AppError(exc) }
        return obj!
    }

    static func make(with data: Data) throws -> Self {
        
        let exc = ExceptionWrapper()
        let obj = make(with: data, exception: exc)
        if exc.fault != 0 { throw AppError(exc) }
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
        if exc.fault != 0 { throw AppError(exc) }
        return obj!
    }
}

extension MakeWithDrive {
    
    static func make(with drive: FloppyDriveProxy, format: ImageFormat) throws -> Self {

        let exc = ExceptionWrapper()
        let obj = make(withDrive: drive, format: format, exception: exc)
        if exc.fault != 0 { throw AppError(exc) }
        return obj!
    }
}

extension MakeWithHardDrive {

    static func make(with drive: HardDriveProxy, format: ImageFormat) throws -> Self {

        let exc = ExceptionWrapper()
        let obj = make(withDrive: drive, format: format,  exception: exc)
        if exc.fault != 0 { throw AppError(exc) }
        return obj!
    }
}

/*
extension HardDiskImageProxy {

    static func make(with hdr: HardDriveProxy, format: ImageFormat) throws -> Self {

        let exc = ExceptionWrapper()
        let obj = make(withDrive: hdr, format: format,  exception: exc)
        if exc.fault != 0 { throw AppError(exc) }
        return obj!
    }
}
*/

extension MakeWithFileSystem {
    
    static func make(with fs: FileSystemProxy) throws -> Self {
        
        let exc = ExceptionWrapper()
        let obj = make(withFileSystem: fs, exception: exc)
        if exc.fault != 0 { throw AppError(exc) }
        return obj!
    }
}

extension EmulatorProxy {

    func launch() throws {
        
        let exception = ExceptionWrapper()
        launch(exception)
        if exception.fault != 0 { throw AppError(exception) }
    }
    
    func launch(_ listener: UnsafeRawPointer, _ callback: @escaping @convention(c) (UnsafeRawPointer?, Message) -> Void) throws
    {
        let exception = ExceptionWrapper()
        launch(listener, function: callback, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }
    
    func isReady() throws {
        
        let exception = ExceptionWrapper()
        isReady(exception)
        if exception.fault != 0 { throw AppError(exception) }
    }
    
    func run() throws {
        
        let exception = ExceptionWrapper()
        run(exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func exportConfig(url: URL) throws {

        let exception = ExceptionWrapper()
        exportConfig(url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }
}

extension MemProxy {


    func loadRom(buffer: Data) throws {

        let exception = ExceptionWrapper()
        loadRom(fromBuffer: buffer, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }
    
    func loadRom(_ url: URL) throws {

        let exception = ExceptionWrapper()
        loadRom(fromFile: url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func loadExt(buffer: Data) throws {

        let exception = ExceptionWrapper()
        loadExt(fromBuffer: buffer, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }
    
    func loadExt(_ url: URL) throws {

        let exception = ExceptionWrapper()
        loadExt(fromFile: url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func saveRom(_ url: URL) throws {

        let exception = ExceptionWrapper()
        saveRom(url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func saveWom(_ url: URL) throws {

        let exception = ExceptionWrapper()
        saveWom(url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func saveExt(_ url: URL) throws {

        let exception = ExceptionWrapper()
        saveExt(url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }
    
    func symbolize(addr: Int, accessor: Accessor = .CPU) -> String? {
        
        switch memSrc(accessor, addr: addr) {
            
        case .NONE:                     return "-"
        case .CHIP, .CHIP_MIRROR:       return "CHIP"
        case .SLOW, .SLOW_MIRROR:       return "SLOW"
        case .FAST:                     return "FAST"
        case .CIA, .CIA_MIRROR:         return "CIA"
        case .RTC:                      return "RTC"
        case .CUSTOM, .CUSTOM_MIRROR:   return regName(addr)
        case .AUTOCONF:                 return "ACONF"
        case .ZOR:                      return "ZORRO"
        case .ROM, .ROM_MIRROR:         return "ROM"
        case .WOM:                      return "WOM"
        case .EXT:                      return "ROM"
        default:                        return nil
        }
    }
}

extension FloppyDriveProxy {

    func swap(url: URL) throws {

        let exception = ExceptionWrapper()
        insertFile(url, protected: false, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func insertNew(fileSystem: FSVolumeType, bootBlock: BootBlockId, name: String, url: URL?) throws {
        
        let exception = ExceptionWrapper()
        insertBlankDisk(fileSystem, bootBlock: bootBlock, name: name, url: url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func writeToFile(url: URL) throws {

        let exception = ExceptionWrapper()
        write(toFile: url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }
}

extension HardDriveProxy {

    func attach(url: URL) throws {

        let exception = ExceptionWrapper()
        attachFile(url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func attach(c: Int, h: Int, s: Int, b: Int) throws {

        let exception = ExceptionWrapper()
        attach(c, h: h, s: s, b: b, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func importFiles(url: URL) throws {

        let exception = ExceptionWrapper()
        importFiles(url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }
    
    func format(fs: FSVolumeType, name: String) throws {

        let exception = ExceptionWrapper()
        format(fs, name: name, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func changeGeometry(c: Int, h: Int, s: Int, b: Int = 512) throws {

        let exception = ExceptionWrapper()
        changeGeometry(c, h: h, s: s, b: b, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }

    func writeToFile(url: URL) throws {

        let exception = ExceptionWrapper()
        write(toFile: url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }
}

extension AnyFileProxy {
    
    @discardableResult
    func writeToFile(url: URL) throws -> Int {
        
        let exception = ExceptionWrapper()
        let result = write(toFile: url.path, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
        
        return result
    }
}

extension DiskImageProxy {

    @discardableResult
    func writeToFile(url: URL) throws -> Int {

        let exception = ExceptionWrapper()
        let result = write(toFile: url, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }

        return result
    }
}

extension DiskImageProxy {

    func icon(protected: Bool = false) -> NSImage? {

        if let proxy = self as? FloppyDiskImageProxy {

            return icon(diameter: proxy.diameter,
                        density: proxy.density,
                        protected: protected)
        }

        switch format {

        case .HDF, .HDZ:

            var name = "hdf"
            if protected { name += "_protected" }
            return NSImage(named: name)!

        default:
            return nil
        }
    }

    func icon(diameter: Diameter, density: Density, protected: Bool = false) -> NSImage? {

        switch format {

        case .ADF, .ADZ, .EADF, .IMG:

            var name = (density == .HD ? "hd" : "dd") + (format == .IMG ? "_dos" : "_adf")
            if protected { name += "_protected" }
            return NSImage(named: name)!

        default:
            return nil
        }
    }
}

extension FloppyDiskImageProxy {

    /*
    func icon(protected: Bool = false) -> NSImage? {

        var name = ""

        switch format {

        case .ADF, .ADZ, .EADF, .IMG:

            name = (density == .HD ? "hd" : "dd") + (format == .IMG ? "_dos" : "_adf")
            // (format == .IMG ? "_dos" : info.dos == .NODOS ? "_other" : "_adf")

        default:

            name = ""
        }

        if protected { name += "_protected" }
        return NSImage(named: name)!
    }
    */
}

extension HardDiskImageProxy {

    /*
    func icon(protected: Bool = false) -> NSImage {

        var name = ""

        switch format {

        case .HDF, .HDZ:

            name = "hdf"

        default:

            name = ""
        }

        if protected { name += "_protected" }
        return NSImage(named: name)!
    }
    */

    @discardableResult
    func writePartitionToFile(url: URL, partition nr: Int) throws -> Int {

        let exception = ExceptionWrapper()
        let result = write(toFile: url, partition: nr, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }

        return result
    }
}

extension FileSystemProxy {

    static func make(with file: FloppyDiskImageProxy) throws -> FileSystemProxy {

        let exception = ExceptionWrapper()
        let result = FileSystemProxy.make(withImage: file, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }

        return result!
    }

    static func make(with file: HardDiskImageProxy, partition: Int = 0) throws -> FileSystemProxy {

        let exception = ExceptionWrapper()
        let result = FileSystemProxy.make(withImage: file, partition: partition, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }

        return result!
    }

    func export(url: URL, recursive: Bool = true, contents: Bool = true) throws {

        let exception = ExceptionWrapper()
        export(url.path, recursive: recursive, contents: contents, exception: exception)
        if exception.fault != 0 { throw AppError(exception) }
    }
}


//
// Other extensions
//

@MainActor
extension EmulatorProxy {
    
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

        let info = info
        var name: String

        if !info.hasDisk { return nil }

        if info.hasProtectedDisk {
            name = info.hasModifiedDisk ? "diskUPTemplate" : "diskPTemplate"
        } else {
            name = info.hasModifiedDisk ? "diskUTemplate" : "diskTemplate"
        }
        
        return NSImage(named: name)!
    }
    
    var toolTip: String? {
        
        return nil
    }
    
    var ledIcon: NSImage? {
        
        let info = info

        if !info.isConnected { return nil }

        if info.motor {
            if info.writing {
                return NSImage(named: "ledRed")
            } else {
                return NSImage(named: "ledGreen")
            }
        } else {
            return NSImage(named: "ledGrey")
        }
    }
}

extension HardDriveProxy {
    
    var templateIcon: NSImage? {
        
        var name: String
                
        switch controller.info.state {

        case .UNDETECTED, .INITIALIZING:
            name = "hdrETemplate"
            
        default:
            name = info.hasModifiedDisk ? "hdrUTemplate" : "hdrTemplate"
        }
        
        return NSImage(named: name)!
    }
    
    var toolTip: String? {
        
        switch controller.info.state {
            
        case .UNDETECTED:
            return "The hard drive is waiting to be initialized by the OS."
            
        case .INITIALIZING:
            return "The OS has started to initialize the hard drive. If the " +
            "condition persists the hard drive is not valid or incompatible " +
            "with the chosen setup."
            
        default:
            return nil
        }
    }
    
    func ledIcon(info: HardDriveInfo) -> NSImage? {

        if !info.isConnected { return nil }

        switch info.state {
            
        case .IDLE: return NSImage(named: "ledGrey")
        case .READING: return NSImage(named: "ledGreen")
        case .WRITING: return NSImage(named: "ledRed")
            
        default: fatalError()
        }
    }
}

extension RemoteManagerProxy {
    
    var icon: NSImage? {

        var numActive = 0
        var numConnected = 0

        func count(_ info: RemoteServerInfo) {

            if info.state != .OFF { numActive += 1 }
            if info.state == .CONNECTED { numConnected += 1 }
        }

        let info = info
        count(info.rshInfo)
        count(info.gdbInfo)
        count(info.promInfo)
        count(info.serInfo)

        if numConnected > 0 { return SFSymbol.get(.serverConnected) }
        if numActive > 0 { return SFSymbol.get(.serverListening) }

        return nil
    }
}
