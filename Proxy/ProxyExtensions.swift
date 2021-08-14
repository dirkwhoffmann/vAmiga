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

    func powerOn() throws {
        
        let exception = ExceptionWrapper()
        power(on: exception)
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

extension DiskControllerProxy {

    func insert(_ nr: Int, file: DiskFileProxy) throws {
        
        let exception = ExceptionWrapper()
        insert(nr, file: file, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
}

extension AmigaFileProxy {
    
    @discardableResult
    func writeToFile(url: URL) throws -> Int {
        
        var err = ErrorCode.OK
        let result = write(toFile: url.path, error: &err)
        if err != .OK { throw VAError(err) }
        
        return result
    }
}

extension FSDeviceProxy {
        
    /*
    func exportDirectory(url: URL) throws {
            
        var err = ErrorCode.OK
        if exportDirectory(url.path, error: &err) == false {
            throw VAError(err)
        }
    }
    */
}

//
//
//

public extension AmigaProxy {
    
    func df(_ nr: Int) -> DriveProxy? {
        
        switch nr {
            
        case 0: return df0
        case 1: return df1
        case 2: return df2
        case 3: return df3
        default:return nil
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
                                        bytesPerRow: 4*width,
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
        
        var name: String
        switch type {
        case .ADF, .DMS, .EXE, .DIR:
            name = density == .HD ? "hd_adf" : "dd_adf"
        case .IMG:
            name = "dd_dos"
        default:
            name = ""
        }
        
        if protected { name += "_protected" }
        return NSImage(named: name)!
    }
    
    var layoutInfo: String {
        
        var result = numSides == 1 ? "Single sided" : "Double sided"

        if diskDensity == .SD { result += ", single density" }
        if diskDensity == .DD { result += ", double density" }
        if diskDensity == .HD { result += ", high density" }

        result += " disk, \(numTracks) tracks with \(numSectors) sectors each"
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

extension NSError {
    
    static func fileError(_ ec: ErrorCode, url: URL) -> NSError {
        
        let str = "\"" + url.lastPathComponent + "\""
        var info1, info2: String

        switch ec {
                    
        case .OK:
            fatalError()
        
        case .FILE_NOT_FOUND:
            info1 = "File " + str + " could not be opened."
            info2 = "The file does not exist."
            
        case .FILE_TYPE_MISMATCH:
            info1 = "File " + str + " could not be opened."
            info2 = "The file format does not match."
            
        case .FILE_CANT_READ:
            info1 = "Can't read from file " + str + "."
            info2 = "The file cannot be opened."
            
        case .FILE_CANT_WRITE:
            info1 = "Can't write to file " + str + "."
            info2 = "The file cannot be opened."
            
        case .OUT_OF_MEMORY:
            info1 = "The file operation cannot be performed."
            info2 = "Not enough memory."
                        
        case .MISSING_ROM_KEY:
            info1 = "Failed to decrypt the selected Rom image."
            info2 = "A rom.key file is required to process this file."
            
        case .INVALID_ROM_KEY:
            info1 = "Failed to decrypt the selected Rom image."
            info2 = "Decrypting the Rom with the provided rom.key file did not produce a valid Rom image."
            
        default:
            info1 = "The operation cannot be performed."
            info2 = "An uncategorized error exception has been thrown."
        }
        
        return NSError(domain: "vAmiga", code: ec.rawValue,
                       userInfo: [NSLocalizedDescriptionKey: info1,
                                  NSLocalizedRecoverySuggestionErrorKey: info2,
                                  NSURLErrorKey: url])
    }
}

extension NSAlert {

    convenience init(fileError ec: ErrorCode, url: URL) {
        
        self.init()
     
        let err = NSError.fileError(ec, url: url)
        
        let msg1 = err.userInfo[NSLocalizedDescriptionKey] as! String
        let msg2 = err.userInfo[NSLocalizedRecoverySuggestionErrorKey] as! String

        alertStyle = .warning
        messageText = msg1
        informativeText = msg2
        addButton(withTitle: "OK")
    }
            
    /// DEPRECATED
    @available(macOS, deprecated)
    static func warning(_ msg1: String, _ msg2: String, icon: String? = nil) {

        alert(msg1, msg2, style: .warning, icon: icon)
    }

    /// DEPRECATED
    @available(macOS, deprecated)
    static func critical(_ msg1: String, _ msg2: String, icon: String? = nil) {
        
        alert(msg1, msg2, style: .critical, icon: icon)
    }
    
    /// DEPRECATED
    @available(macOS, deprecated)
    static func alert(_ msg1: String, _ msg2: String, style: NSAlert.Style, icon: String?) {
        
        let alert = NSAlert()
        alert.alertStyle = style
        if let icon = icon { alert.icon = NSImage(named: icon) }
        alert.messageText = msg1
        alert.informativeText = msg2
        alert.addButton(withTitle: "OK")
        alert.runModal()
    }
}

extension ErrorCode {
    
    func showAlert(url: URL) {
        
        let alert = NSAlert(fileError: self, url: url)
        alert.runModal()
    }
}
