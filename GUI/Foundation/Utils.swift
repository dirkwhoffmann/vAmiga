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
    
    static var appSupportFolder: URL? {
        
        let fm = FileManager.default
        let path = FileManager.SearchPathDirectory.applicationSupportDirectory
        let mask = FileManager.SearchPathDomainMask.userDomainMask
        let url = fm.urls(for: path, in: mask).first
        return url?.appendingPathComponent("vAmiga")
    }
    
    static func appSupportFolder(_ name: String) -> URL? {
    
        guard let support = URL.appSupportFolder else { return nil }

        let fm = FileManager.default
        let folder = support.appendingPathComponent("\(name)")
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
    
    static var tmpFolder: URL? {

        return appSupportFolder("tmp")
    }
    
    func moveToTmpFolder() throws -> URL? {
        
        // Get the tmp folder
        guard let tmp = URL.tmpFolder else { return nil }
        
        // Compose destination URL
        let file = self.lastPathComponent
        let dest = tmp.appendingPathComponent(file)
        
        track("moveToTmpFolder: \(tmp)")
        
        // Clear all existing items
        try tmp.clear()
                
        // Copy the file
        track("Copying \(self) to \(dest)")
        try FileManager.default.copyItem(at: self, to: dest)
        track("Item copied")
        
        return dest
    }
    
    func clear(except file: URL? = nil) throws {
        
        track("clear: \(self)")
        let urls = try FileManager.default.contentsOfDirectory(
            at: self, includingPropertiesForKeys: nil,
            options: [.skipsHiddenFiles, .skipsSubdirectoryDescendants])

        track("urls = \(urls)")

        for url in urls where url.lastPathComponent != file?.lastPathComponent {
            try FileManager.default.removeItem(at: url)
        }
        track("cleared")
    }
        
    func unpack(allowedFileTypes: [String]) throws -> URL? {
                
        // Check if this file is compressed
        let extensions = ["zip", "gz", "adz"]
        if !extensions.contains( self.pathExtension) { return nil }
        
        // Get a temporary folder
        guard let tmp = URL.tmpFolder else { return nil }
        track("tmp folder = \(tmp)")
                    
        // Copy file to a temporary directory
        guard let tmpFile = try self.moveToTmpFolder() else { return nil }
        
        // Get path to the temporary as a string
        print("\(tmpFile.path)")
        let path = tmpFile.path // .replacingOccurrences(of: " ", with: #"\ "#)
        print("\(path)")

        // Try to unzip
        let exec = "/usr/bin/gunzip"
        let args = [ path ]
        
        track("exec = \(exec)")
        track("args = \(args)")
        print("\(args)")
        if let result = FileManager.exec(launchPath: exec, arguments: args) {
            print("\(result)")
        }
        
        // Get all files of the allowed file types
        let urls = try FileManager.default.contentsOfDirectory(
             at: tmp, includingPropertiesForKeys: nil,
             options: [.skipsHiddenFiles, .skipsSubdirectoryDescendants])
        track("urls = \(urls)")
        var result: [URL] = []
        for url in urls {
            if allowedFileTypes.contains(url.pathExtension) {
                result.append(url)
            }
        }
        
        // Pick the first one
        let first = result.first
        
        return first
    }
        
    func modificationDate() -> Date? {
        
        let attr = try? FileManager.default.attributesOfItem(atPath: self.path)
        
        if attr != nil {
            return attr![.creationDate] as? Date
        } else {
            return nil
        }
    }
        
    func addTimeStamp() -> URL {
        
        let path = self.deletingPathExtension().path
        let suffix = self.pathExtension
        
        let date = Date.init()
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd"
        let dateString = formatter.string(from: date)
        formatter.dateFormat = "hh.mm.ss"
        let timeString = formatter.string(from: date)
        let timeStamp = dateString + " at " + timeString

        return URL(fileURLWithPath: path + " " + timeStamp + "." + suffix)
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
    
    func addExtension(for format: NSBitmapImageRep.FileType) -> URL {
    
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

    var adfFromAdz: URL? {
        
        if self.pathExtension.uppercased() != "ADZ" { return nil }
        
        let name = self.deletingPathExtension().lastPathComponent
        let adfname = name + ".adf"
        
        if let support = URL.appSupportFolder {
            
            let exec = "/usr/bin/unzip"
            let args = [
                "-o",           // Overwrite existing file
                self.path,      // Zipped archive
                adfname,        // File to extract
                "-d",           // Extract file to...
                support.path    // ...the application support folder
            ]
            
            track("exec = \(exec)")
            track("args = \(args)")
            
            if let result = FileManager.exec(launchPath: exec, arguments: args) {
                
                print("\(result)")
                return support.appendingPathComponent(adfname)
            }
        }
        return nil
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
        
        let data = pipe.fileHandleForReading.readDataToEndOfFile()
        let result = String(data: data, encoding: .utf8)
        task.waitUntilExit()
        
        return result
    }
    
    /*
    static func moveToTmpFolder(url: URL) throws {
        
        // Get the tmp folder
        guard let tmp = URL.tmpFolder else { return }
        
        // Clear all existing items
        try tmp.erase() //  clearFolder(folder: tmp)
        
        // Copy the file
        try FileManager.default.copyItem(at: url, to: tmp)
    }
    */
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
    
    func selectedIndex() -> Int {
        
        let selected = self.selectedTabViewItem
        return selected != nil ? self.indexOfTabViewItem(selected!) : -1
    }
}
