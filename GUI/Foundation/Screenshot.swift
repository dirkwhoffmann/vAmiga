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
    
    // Creation date
    var data = Date()
    
    // Indicates if the upspaced texture has been recorded
    var upscaled = false
    
    init(screen: NSImage, upscaled: Bool) {
        
        self.screen = screen
        self.upscaled = upscaled
        data = Date()
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
    
    static func url(for item: Int, in folder: URL?) -> URL? {
        
        if folder == nil { return nil }

        let types: [NSBitmapImageRep.FileType] = [ .tiff, .bmp, .gif, .jpeg, .png ]
        let url = folder!.appendingPathComponent(String(format: "%03d", item))
        
        for type in types {
            if let url = fileExists(name: url, type: type) { return url }
        }
        
        return nil
    }
    
    static func userUrl(for item: Int, checksum: UInt64) -> URL? {
        return Screenshot.url(for: item, in: Screenshot.userFolder(checksum: checksum))
    }
    
    static func autoUrl(for item: Int, checksum: UInt64) -> URL? {
        return Screenshot.url(for: item, in: Screenshot.autoFolder(checksum: checksum))
    }
    
    static func newUrl(in folder: URL?) -> URL? {
        
        if folder == nil { return nil }
        
        for i in 0...999 {
            
            let filename = String(format: "%03d.jpeg", i)
            let url = folder!.appendingPathComponent(filename)
            
            if !FileManager.default.fileExists(atPath: url.path) {
                return url
            }
        }
        
        return nil
    }
    
    static func newUserUrl(checksum: UInt64) -> URL? {
        return Screenshot.newUrl(in: Screenshot.userFolder(checksum: checksum))
    }
    
    static func newAutoUrl(checksum: UInt64) -> URL? {
        return Screenshot.newUrl(in: Screenshot.autoFolder(checksum: checksum))
    }
    
    static func newUrl(checksum: UInt64, auto: Bool) -> URL? {
        return auto ? newAutoUrl(checksum: checksum) : newUserUrl(checksum: checksum)
    }
    
    static func collectFiles(in folder: URL?) -> [URL] {
        
        var result = [URL]()
        
        for i in 0...999 {
            
            if let url = Screenshot.url(for: i, in: folder) {
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

    static func swap(item: Int, with item2: Int, in folder: URL?) {
        
        if folder == nil { return }
                
        let oldUrl = Screenshot.url(for: item, in: folder)
        let newUrl = Screenshot.url(for: item2, in: folder)
        let tmpUrl = Screenshot.newUrl(in: folder)
        
        if oldUrl != nil && newUrl != nil && tmpUrl != nil {
            
            let fm = FileManager.default
            try? fm.moveItem(at: oldUrl!, to: tmpUrl!)
            try? fm.moveItem(at: newUrl!, to: oldUrl!)
            try? fm.moveItem(at: tmpUrl!, to: newUrl!)
        }
    }

    static func swapUser(item: Int, with item2: Int, checksum: UInt64) {
        
        Screenshot.swap(item: item, with: item2, in: Screenshot.userFolder(checksum: checksum))
    }

    static func swapAuto(item: Int, with item2: Int, checksum: UInt64) {
        
        Screenshot.swap(item: item, with: item2, in: Screenshot.autoFolder(checksum: checksum))
    }
    
    static func delete(item: Int, in folder: URL?) {
        
        if folder == nil { return }
                
        if var url = Screenshot.url(for: item, in: folder) {
        
            let fm = FileManager.default
            try? fm.removeItem(at: url)
            
            // Rename all items above the deleted one
            for i in item ... 998 {
                
                if let above = Screenshot.url(for: i + 1, in: folder) {
                    
                    track("Renaming \(above) to \(url)")
                    try? fm.moveItem(at: above, to: url)
                    url = above
                    
                } else { break }
            }
        }
    }
    
    static func deleteUser(item: Int, checksum: UInt64) {
        
        Screenshot.delete(item: item, in: userFolder(checksum: checksum))
    }

    static func deleteAuto(item: Int, checksum: UInt64) {

        Screenshot.delete(item: item, in: autoFolder(checksum: checksum))
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

    static func thinOut(folder: URL?, counter: Int) {
        
        if folder == nil { return }

        let count = collectFiles(in: folder!).count
        let max = 32

        if count > max {
            
            var itemToDelete = 0

            if counter % 2 == 0 {
                itemToDelete = 24
            } else if (counter >> 1) % 2 == 0 {
                itemToDelete = 16
            } else if (counter >> 2) % 2 == 0 {
                itemToDelete = 8
            }

            track("Thinning out item \(itemToDelete)")
            delete(item: itemToDelete, in: folder)
        }
    }
    
    static func thinOutAuto(checksum: UInt64, counter: Int) {
        let folder = Screenshot.autoFolder(checksum: checksum)
        Screenshot.thinOut(folder: folder, counter: counter)
    }

    static func thinOutUser(checksum: UInt64, counter: Int) {
        let folder = Screenshot.userFolder(checksum: checksum)
        Screenshot.thinOut(folder: folder, counter: counter)
    }

    static func moveToUser(item: Int, checksum: UInt64) {
                
        let oldUrl = Screenshot.autoUrl(for: item, checksum: checksum)
        let newUrl = Screenshot.newUserUrl(checksum: checksum)
                
        if oldUrl != nil && newUrl != nil {
            do {
                try FileManager.default.copyItem(at: oldUrl!, to: newUrl!)
                Screenshot.deleteAuto(item: item, checksum: checksum)
            } catch let error as NSError {
                print(error)
            }
        }
    }
}
