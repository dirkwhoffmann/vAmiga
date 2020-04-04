// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Screenshot {
    
    // The actual screenshot
    var screen: NSImage?
    
    // Proposed file format for this screenshot
    var format = NSBitmapImageRep.FileType.jpeg
    
    // Creation date
    var data = Date()
    
    // Image width and heigt
    var width: Int { return Int(screen?.size.width ?? 0) }
    var height: Int { return Int(screen?.size.height ?? 0) }

    // Indicates if the upspaced texture has been recorded
    var upscaled: Bool { return height > 1000 }

    // Textual description of the image source
    var sourceString: String {
        return upscaled ? "upscaled texture" : "emulator texture"
    }
    
    // Textual description of the image format
    var formatString: String {
        switch format {
        case .tiff: return "TIFF image"
        case .bmp:  return "BMP image"
        case .gif:  return "GIF image"
        case .jpeg: return "JPEG image"
        case .png:  return "PNG image"
        default:    return "Image"
        }
    }
    
    // Combined textual description
    var description: String {
        return formatString + " from " + sourceString
    }
    
    // Textual representation of the image size
    var sizeString: String {
        return screen != nil ? "\(width) x \(height)" : ""
    }
    
    init(screen: NSImage, format: NSBitmapImageRep.FileType) {
        
        self.screen = screen
        self.format = format
        data = Date()
    }
    
    convenience init?(fromUrl url: URL) {
        
        guard let format = url.imageFormat else { return nil }
        guard let image = NSImage.init(contentsOf: url) else { return nil }

        self.init(screen: image, format: format)
    }
    
    func quicksave(format: NSBitmapImageRep.FileType) {
        
        // Get URL to desktop directory
        let paths = NSSearchPathForDirectoriesInDomains(.desktopDirectory,
                                                        .userDomainMask,
                                                        true)
        let desktop = NSURL.init(fileURLWithPath: paths[0])
        
        // Assemble filename
        if var url = desktop.appendingPathComponent("Screenshot.") {
            
            url = url.addExtension(for: format)
            url = url.addTimeStamp()
            url = url.makeUnique()
            
            do {
                try save(url: url, format: format)
            } catch {
                track("Failed to quicksave screenshot to \(url.path)")
            }
        }
    }
      
    func save(url: URL, format: NSBitmapImageRep.FileType) throws {
        
        // Convert to target format
        let data = screen?.representation(using: format)
        
        // Save to file
        try data?.write(to: url, options: .atomic)
    }
    
    static func folder(auto: Bool, checksum: UInt64) -> URL? {
        
        let fm = FileManager.default
        let path = FileManager.SearchPathDirectory.applicationSupportDirectory
        let mask = FileManager.SearchPathDomainMask.userDomainMask
        guard let support = fm.urls(for: path, in: mask).first else { return nil }
        let dir = auto ? "auto" : "user"
        let subdir = String(format: "%X", checksum)
        let folder = support.appendingPathComponent("vAmiga/\(dir)/\(subdir)")
        var isDirectory: ObjCBool = false
        let folderExists = fm.fileExists(atPath: folder.path,
                                         isDirectory: &isDirectory)
        
        if !folderExists || !isDirectory.boolValue {
            
            do {
                try fm.createDirectory(at: folder,
                                       withIntermediateDirectories: true,
                                       attributes: nil)
            } catch {
                return nil
            }
        }
        
        return folder
    }
    
    static func userFolder(checksum: UInt64) -> URL? {
        return Screenshot.folder(auto: false, checksum: checksum)
    }
    
    static func autoFolder(checksum: UInt64) -> URL? {
        return Screenshot.folder(auto: true, checksum: checksum)
    }
    
    static func fileExists(name: URL, type: NSBitmapImageRep.FileType) -> URL? {
        
        let url = name.addExtension(for: type)
        return FileManager.default.fileExists(atPath: url.path) ? url : nil
    }
    
    static func url(forItem item: Int, in folder: URL?) -> URL? {
        
        if folder == nil { return nil }

        let types: [NSBitmapImageRep.FileType] = [ .tiff, .bmp, .gif, .jpeg, .png ]
        let url = folder!.appendingPathComponent(String(format: "%03d", item))
        
        for type in types {
            if let url = fileExists(name: url, type: type) { return url }
        }
        
        return nil
    }
    
    static func userUrl(forItem item: Int, checksum: UInt64) -> URL? {
        return Screenshot.url(forItem: item, in: Screenshot.userFolder(checksum: checksum))
    }
    
    static func autoUrl(forItem item: Int, checksum: UInt64) -> URL? {
        return Screenshot.url(forItem: item, in: Screenshot.autoFolder(checksum: checksum))
    }
    
    static func newUrl(in folder: URL?,
                       using format: NSBitmapImageRep.FileType = .jpeg) -> URL? {
        
        if folder == nil { return nil }
        
        for i in 0...999 {
            
            let filename = String(format: "%03d", i)
            let url = folder!.appendingPathComponent(filename).addExtension(for: format)
            
            if !FileManager.default.fileExists(atPath: url.path) {
                return url
            }
        }
        
        return nil
    }
    
    static func newUserUrl(checksum: UInt64,
                           using format: NSBitmapImageRep.FileType = .jpeg) -> URL? {

        let folder = Screenshot.userFolder(checksum: checksum)
        return Screenshot.newUrl(in: folder, using: format)
    }
    
    static func newAutoUrl(checksum: UInt64,
                           using format: NSBitmapImageRep.FileType = .jpeg) -> URL? {
        
        let folder = Screenshot.autoFolder(checksum: checksum)
        return Screenshot.newUrl(in: folder, using: format)
    }
    
    static func newUrl(checksum: UInt64, auto: Bool) -> URL? {
        return auto ? newAutoUrl(checksum: checksum) : newUserUrl(checksum: checksum)
    }
    
    static func collectFiles(in folder: URL?) -> [URL] {
        
        var result = [URL]()
        
        for i in 0...999 {
            
            if let url = Screenshot.url(forItem: i, in: folder) {
                result.append(url)
            } else {
                break
            }
        }
        return result
    }
    
    static func collectUserFiles(checksum: UInt64) -> [URL] {
        
        return Screenshot.collectFiles(in: Screenshot.userFolder(checksum: checksum))
    }
    
    static func collectAutoFiles(checksum: UInt64) -> [URL] {
        
        return Screenshot.collectFiles(in: Screenshot.autoFolder(checksum: checksum))
    }

    static func delete(folder: URL?) {
        
        let files = Screenshot.collectFiles(in: folder)
        for file in files {
            try? FileManager.default.removeItem(at: file)
        }
    }
    
    static func deleteUserFolder(checksum: UInt64) {
        
        Screenshot.delete(folder: Screenshot.userFolder(checksum: checksum))
    }
    
    static func deleteAutoFolder(checksum: UInt64) {
        
        track()
        Screenshot.delete(folder: Screenshot.autoFolder(checksum: checksum))
    }
}
