// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

//
// URL
//

extension URL {

    enum FolderError: Error {
        case noAppSupportFolder
    }
    enum UnpackError: Error {
        case noSupportedFiles
    }

    //
    // Querying file attributes
    //
    
    var attributes: [FileAttributeKey: Any]? {
        
        return try? FileManager.default.attributesOfItem(atPath: path)
    }
    
    var fileSize: UInt64 {
        return attributes?[.size] as? UInt64 ?? UInt64(0)
    }
    
    var fileSizeString: String {
        return ByteCountFormatter.string(fromByteCount: Int64(fileSize), countStyle: .file)
    }
    
    var creationDate: Date? {
        return attributes?[.creationDate] as? Date
    }
    
    //
    // Working with folders
    //
    
    // Returns the URL of the application support folder of this application
    static func appSupportFolder() throws -> URL {
        
        let fm = FileManager.default
        
        let path = FileManager.SearchPathDirectory.applicationSupportDirectory
        let mask = FileManager.SearchPathDomainMask.userDomainMask
        
        let url = try fm.url(for: path, in: mask, appropriateFor: nil, create: true)
        return url.appendingPathComponent("vAmiga")
    }
    
    // Returns the URL of a subdirectory inside the application support folder
    static func appSupportFolder(_ name: String) throws -> URL {
        
        let support = try URL.appSupportFolder()
        
        let fm = FileManager.default
        let folder = support.appendingPathComponent("\(name)")
        var isDirectory: ObjCBool = false
        let folderExists = fm.fileExists(atPath: folder.path,
                                         isDirectory: &isDirectory)
        
        if !folderExists || !isDirectory.boolValue {
            
            try fm.createDirectory(at: folder,
                                   withIntermediateDirectories: true,
                                   attributes: nil)
        }
        
        return folder
    }
        
    // Returns the URL of an empty temporary folder
    static func tmpFolder() throws -> URL {
        
        let tmp = try appSupportFolder("tmp")
        try tmp.delete()
        return tmp
    }

    // Returns all files inside a folder
    func contents(allowedTypes: [String]? = nil) throws -> [URL] {
        
        // Collect files
        let urls = try FileManager.default.contentsOfDirectory(
            at: self, includingPropertiesForKeys: nil,
            options: [.skipsHiddenFiles, .skipsSubdirectoryDescendants]
        )

        // Filter out sub directories
        var filtered = urls.filter {
            $0.hasDirectoryPath == false
        }
        
        // Filter out all files with an unallowed type
        filtered = filtered.filter {
            allowedTypes?.contains($0.pathExtension.uppercased()) ?? true
        }
        
        return filtered
    }
    
    // Deletes all files inside a folder
    func delete() throws {
        
        let urls = try self.contents()
        for url in urls { try FileManager.default.removeItem(at: url) }
    }
    
    // Copies a file into the specified folder
    func copy(to folder: URL, replaceExtensionBy suffix: String) throws -> URL {
        
        // Create the destination URL
        var dest = folder.appendingPathComponent(self.lastPathComponent)
        dest.deletePathExtension()
        dest.appendPathExtension(suffix)
        
        // Copy the file
        try FileManager.default.copyItem(at: self, to: dest)
        
        return dest
    }
    
    func unpacked(maxSize: Int) -> URL {
        
        if fileSize < maxSize { return unpacked }
        
        return self
    }

    var unpacked: URL {
        
        if self.pathExtension == "zip" || self.pathExtension == "adz" {
            
            do { return try unpackZip() } catch { }
        }
        
        if self.pathExtension == "gz" || self.pathExtension == "adz" {
            
            do { return try unpackGz() } catch { }
        }
        
        return self
    }
    
    func unpackZip() throws -> URL {
        
        let urls = try unpack(suffix: "zip")
        if let first = urls.first { return first }
        
        throw UnpackError.noSupportedFiles
    }
    
    func unpackGz() throws -> URL {
        
        let urls = try unpack(suffix: "gz")
        if let first = urls.first { return first }
        
        throw UnpackError.noSupportedFiles
    }
    
    func unpack(suffix: String) throws -> [URL] {
        
        // Request the URL of a tempory folder
        let tmp = try URL.tmpFolder()
        
        // Copy the compressed file into it and fix the extension
        let url = try self.copy(to: tmp, replaceExtensionBy: suffix)
        
        // Try to decompress the file
        var exec: String
        var args: [String]
        
        switch suffix {
            
        case "zip":
            exec = "/usr/bin/unzip"
            args = [ "-o", url.path, "-d", tmp.path ]
            
        case "gz":
            exec = "/usr/bin/gunzip"
            args = [ url.path ]
            
        default:
            fatalError()
        }
                
        if let result = FileManager.exec(launchPath: exec, arguments: args) {
            print("\(result)")
        }
        
        // Collect all extracted URLs with a supported file type
        let types = ["ADF", "DMS", "IMG", "IMA", "EXE", "ROM", "BIN", "VAMIGA"]
        let urls = try tmp.contents(allowedTypes: types)
        
        // Arrange the URLs in alphabetical order
        let sorted = urls.sorted { $0.path < $1.path }
        return sorted
    }
    
    var modificationDate: Date? {
        
        guard let resVal = try? resourceValues(forKeys: [.contentModificationDateKey]) else {
            return nil
        }
        
        return resVal.contentModificationDate
    }
    
    func byAddingTimeStamp() -> URL {
        
        let path = self.deletingPathExtension()
        var lastComp = path.lastPathComponent
        let suffix = self.pathExtension
        
        let date = Date()
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd"
        let dateString = formatter.string(from: date)
        formatter.dateFormat = "hh.mm.ss"
        let timeString = formatter.string(from: date)
        lastComp.append(contentsOf: " \(dateString) at \(timeString)")
        
        return self.deletingLastPathComponent().appendingPathComponent(lastComp, isDirectory: false).appendingPathExtension(suffix)
    }
    
    func makeUnique() -> URL {
        
        let path = self.deletingPathExtension().path
        let suffix = self.pathExtension
        let fileManager = FileManager.default
        
        for i in 0...127 {
            
            let numberStr = (i == 0) ? "." : " \(i)."
            let url = URL(fileURLWithPath: path + numberStr + suffix)
            
            if !fileManager.fileExists(atPath: url.path) {
                return url
            }
        }
        return self
    }
    
    func byAddingExtension(for format: NSBitmapImageRep.FileType) -> URL {
        
        let extensions: [NSBitmapImageRep.FileType: String] =
            [ .tiff: "tiff", .bmp: "bmp", .gif: "gif", .jpeg: "jpeg", .png: "png" ]
        
        guard let ext = extensions[format] else {
            log(warning: "Unsupported image format: \(format)")
            return self
        }
        
        return self.appendingPathExtension(ext)
    }
    
    var imageFormat: NSBitmapImageRep.FileType? {
        
        switch pathExtension {
        case "tiff": return .tiff
        case "bmp": return .bmp
        case "gif": return .gif
        case "jpg", "jpeg": return .jpeg
        case "png": return .png
        default: return nil
        }
    }
}

//
// FileManager
//

extension FileManager {
    
    static func exec(launchPath: String, arguments: [String]) -> String? {
        
        let task = Process()
        task.launchPath = launchPath
        task.arguments = arguments
        
        let pipe = Pipe()
        task.standardOutput = pipe
        task.standardError = pipe
        task.launch()
        
        task.waitUntilExit()
        let data = pipe.fileHandleForReading.readDataToEndOfFile()
        let result = String(data: data, encoding: .utf8)
        
        return result
    }
    
    @discardableResult
    static func copy(from source: URL, to dest: URL) -> Bool {
        
        do {
            if FileManager.default.fileExists(atPath: dest.path) {
                try FileManager.default.removeItem(at: dest)
            }
            try FileManager.default.copyItem(at: source, to: dest)
        } catch {
            return false
        }
        return true
    }
}
