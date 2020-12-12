// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Carbon.HIToolbox

//
// Logging / Debugging
// 

public func track(_ message: String = "",
                  path: String = #file, function: String = #function, line: Int = #line ) {
    
    if let file = URL.init(string: path)?.deletingPathExtension().lastPathComponent {
        if message == "" {
            print("\(file).\(line)::\(function)")
        } else {
            print("\(file).\(line)::\(function): \(message)")
        }
    }
}

//
// Extensions to comparable
//

extension Comparable {
    
    func clamped(_ f: Self, _ t: Self) -> Self {
        
        var r = self
        if r < f { r = f }
        if r > t { r = t }
        return r
    }
}

//
// String class extensions
//

extension String {
    
    init?(keyCode: UInt16, carbonFlags: Int) {
        
        let source = TISCopyCurrentASCIICapableKeyboardLayoutInputSource().takeUnretainedValue()
        let layoutData = TISGetInputSourceProperty(source, kTISPropertyUnicodeKeyLayoutData)
        let dataRef = unsafeBitCast(layoutData, to: CFData.self)
        let keyLayout = UnsafePointer<CoreServices.UCKeyboardLayout>.self
        let keyLayoutPtr = unsafeBitCast(CFDataGetBytePtr(dataRef), to: keyLayout)
        let modifierKeyState = (carbonFlags >> 8) & 0xFF
        let keyTranslateOptions = OptionBits(CoreServices.kUCKeyTranslateNoDeadKeysBit)
        var deadKeyState: UInt32 = 0
        let maxChars = 1
        var length = 0
        var chars = [UniChar](repeating: 0, count: maxChars)
        
        let error = CoreServices.UCKeyTranslate(keyLayoutPtr,
                                                keyCode,
                                                UInt16(CoreServices.kUCKeyActionDisplay),
                                                UInt32(modifierKeyState),
                                                UInt32(LMGetKbdType()),
                                                keyTranslateOptions,
                                                &deadKeyState,
                                                maxChars,
                                                &length,
                                                &chars)
        if error == noErr {
            self.init(NSString(characters: &chars, length: length))
        } else {
            return nil
        }
    }
}

extension NSAttributedString {
    
    convenience init(_ text: String, size: CGFloat, color: NSColor) {
        
        let paraStyle = NSMutableParagraphStyle()
        paraStyle.alignment = .center

        let attr: [NSAttributedString.Key: Any] = [
            .font: NSFont.systemFont(ofSize: size),
            .foregroundColor: color,
            .paragraphStyle: paraStyle
        ]
        
        self.init(string: text, attributes: attr)
    }
}

//
// URL class extensions
//

extension URL {

    enum FolderError: Error {
        case noAppSupportFolder
    }
    enum UnpackError: Error {
        case noSupportedFiles
    }

    //
    // Working with folders
    //
    
    // Returns the URL of the application support folder of this application
    static func appSupportFolder() throws -> URL {
        
        let fm = FileManager.default
        let path = FileManager.SearchPathDirectory.applicationSupportDirectory
        let mask = FileManager.SearchPathDomainMask.userDomainMask
        
        if let url = fm.urls(for: path, in: mask).first {
            return url.appendingPathComponent("vAmiga")
        } else {
            throw FolderError.noAppSupportFolder
        }
    }
    
    // Returns the URL of a sub directory inside the application support folder
    static func appSupportFolder(_ name: String, create: Bool = false) throws -> URL {

        let support = try URL.appSupportFolder()
        
        let fm = FileManager.default
        let folder = support.appendingPathComponent("\(name)")
        
        // Check if the folder already exists
        var isDirectory: ObjCBool = false
        let folderExists = fm.fileExists(atPath: folder.path,
                                         isDirectory: &isDirectory)
        
        // If not, create the folder (if requested)
        if create && (!folderExists || !isDirectory.boolValue) {
            
            try fm.createDirectory(at: folder,
                                   withIntermediateDirectories: true,
                                   attributes: nil)
        }
        
        return folder
    }
        
    // Return the URL to an empty temporary folder
    static func tmpFolder() throws -> URL {
        
        let tmp = try appSupportFolder("tmp", create: true)
        try tmp.delete()
        return tmp
    }

    // Returns all files inside a folder
    func contents(allowedTypes: [String]? = nil) throws -> [URL] {
        
        let urls = try FileManager.default.contentsOfDirectory(
            at: self, includingPropertiesForKeys: nil,
            options: [.skipsHiddenFiles, .skipsSubdirectoryDescendants]
        )
        
        let filtered = urls.filter {
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
        
        track("exec = \(exec)")
        track("args = \(args)")
        
        if let result = FileManager.exec(launchPath: exec, arguments: args) {
            print("\(result)")
        }
        
        // Collect all extracted URLs with a supported file type
        let types = ["ADF", "DMS", "IMG", "IMA", "ROM", "BIN", "VAMIGA"]
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
            track("Unsupported image format: \(format)")
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
}

//
// Data class extensions
//

extension Data {
    var bitmap: NSBitmapImageRep? {
        return NSBitmapImageRep(data: self)
    }
}

//
// Managing time and date
//

extension DispatchTime {

    static func diffNano(_ t: DispatchTime) -> UInt64 {
        return DispatchTime.now().uptimeNanoseconds - t.uptimeNanoseconds
    }

    static func diffMicroSec(_ t: DispatchTime) -> UInt64 { return diffNano(t) / 1_000 }
    static func diffMilliSec(_ t: DispatchTime) -> UInt64 { return diffNano(t) / 1_000_000 }
    static func diffSec(_ t: DispatchTime) -> UInt64 { return diffNano(t) / 1_000_000_000 }
}

extension Date {

    func diff(_ date: Date) -> TimeInterval {
        
        let interval1 = self.timeIntervalSinceReferenceDate
        let interval2 = date.timeIntervalSinceReferenceDate

        return interval2 - interval1
    }
}

//
// Controls
//

extension NSTabView {
    
    var selectedIndex: Int {
        
        guard let selected = self.selectedTabViewItem else {
            return -1
        }
        return self.indexOfTabViewItem(selected)
    }
}
