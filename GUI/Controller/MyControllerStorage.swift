// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

//
// Screenshots
//

extension MyController {
    
    private func filename(nr: Int) -> String {
        
        return String(format: "%03d.jpeg", nr)
    }
    
    private func getFolderURL(base: String, disk: String) -> URL? {
                
        let fm = FileManager.default
        let path = FileManager.SearchPathDirectory.applicationSupportDirectory
        let mask = FileManager.SearchPathDomainMask.userDomainMask
        guard let support = fm.urls(for: path, in: mask).first else { return nil }
        let folder = support.appendingPathComponent("vAmiga/\(base)/\(disk)")
                
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
    
    func userScreenshotFolder(_ disk: String?) -> URL? {
        return disk != nil ? getFolderURL(base: "user", disk: disk!) : nil
    }

    func autoScreenshotFolder(_ disk: String?) -> URL? {
        return disk != nil ? getFolderURL(base: "auto", disk: disk!) : nil
    }

    func screenshotFolderContents(base: String, disk: String) -> [URL] {
        
        var result = [URL]()

        for i in 0...999 {
            
            if let url = screenshotURL(base: base, disk: disk, nr: i) {
                result.append(url)
            } else {
                break
            }
        }

        track("screenshotFolderContents: \(result)")
        return result
    }

    func userScreenshotFolderContents(disk: String?) -> [URL] {
       return disk != nil ? screenshotFolderContents(base: "user", disk: disk!) : []
    }
    func userScreenshotFolderContents(disk: UInt64) -> [URL] {
        return userScreenshotFolderContents(disk: String(format: "%X", disk))
    }
    func autoScreenshotFolderContents(disk: String?) -> [URL] {
       return disk != nil ? screenshotFolderContents(base: "auto", disk: disk!) : []
    }
    func autoScreenshotFolderContents(disk: UInt64) -> [URL] {
        return autoScreenshotFolderContents(disk: String(format: "%X", disk))
    }

    func screenshotURL(base: String, disk: String, nr: Int) -> URL? {
        
        if let url = getFolderURL(base: base, disk: disk) {
            
            let fileurl = url.appendingPathComponent(filename(nr: nr))
            if FileManager.default.fileExists(atPath: fileurl.path) { return fileurl }
        }
        
        return nil
    }
    func autoScreenshotURL(disk: String?, nr: Int) -> URL? {
        return disk != nil ? screenshotURL(base: "auto", disk: disk!, nr: nr) : nil
    }
    func userScreenshotURL(disk: String?, nr: Int) -> URL? {
        return disk != nil ? screenshotURL(base: "user", disk: disk!, nr: nr) : nil
    }

    func newScreenshotURL(base: String, disk: String) -> URL? {
        
        if let url = getFolderURL(base: base, disk: disk) {
            
            for i in 0...999 {
                
                let fileurl = url.appendingPathComponent(filename(nr: i))
                
                if !FileManager.default.fileExists(atPath: fileurl.path) {
                    return fileurl
                }
            }
        }
        
        return nil
    }
    
    func newUserScreenshotURL(disk: String?) -> URL? {
        return disk != nil ? newScreenshotURL(base: "user", disk: disk!) : nil
    }

    func newAutoScreenshotURL(disk: String?) -> URL? {
        return disk != nil ? newScreenshotURL(base: "auto", disk: disk!) : nil
    }
    
    func deleteScreenshot(base: String, disk: String, nr: Int) {
        
        track()
        let fm = FileManager.default
                
        if var url = screenshotURL(base: base, disk: disk, nr: nr) {
            
            try? FileManager.default.removeItem(at: url)
            
            // Rename all items above the deleted one
            for i in nr ... 998 {
                
                if let above = screenshotURL(base: base, disk: disk, nr: i + 1) {
                    
                    track("Renaming \(above) to \(above)")
                    try? fm.moveItem(at: above, to: url)
                    url = above
                    
                } else { break }
            }
        }
    }
    
    func swapScreenshots(base: String, disk: String, nr1: Int, nr2: Int) {
        
        let fm = FileManager.default
        
        let oldUrl = screenshotURL(base: base, disk: disk, nr: nr1)
        let newUrl = screenshotURL(base: base, disk: disk, nr: nr2)
        let tmpUrl = newScreenshotURL(base: base, disk: disk)

        /*
        track("swap:")
        track("\(oldUrl)")
        track("\(newUrl)")
        track("\(tmpUrl)")
        */
        
        if oldUrl != nil && newUrl != nil && tmpUrl != nil {
            
            try? fm.moveItem(at: oldUrl!, to: tmpUrl!)
            try? fm.moveItem(at: newUrl!, to: oldUrl!)
            try? fm.moveItem(at: tmpUrl!, to: newUrl!)
        }
    }
    
    func swapUserScreenshots(disk: String?, nr1: Int, nr2: Int) {
        if disk != nil { swapScreenshots(base: "user", disk: disk!, nr1: nr1, nr2: nr2) }
    }
    func swapUserScreenshots(disk: UInt64, nr1: Int, nr2: Int) {
        swapUserScreenshots(disk: String(format: "%X", disk), nr1: nr1, nr2: nr2)
    }
    func swapAutoScreenshots(disk: String?, nr1: Int, nr2: Int) {
        if disk != nil { swapScreenshots(base: "auto", disk: disk!, nr1: nr1, nr2: nr2) }
    }
    func swapAutoScreenshots(disk: UInt64, nr1: Int, nr2: Int) {
        swapAutoScreenshots(disk: String(format: "%X", disk), nr1: nr1, nr2: nr2)
    }

    func moveToUserScreenshots(disk: String, nr: Int) {
        
        let fm = FileManager.default
                    
        let oldUrl = autoScreenshotURL(disk: disk, nr: nr)
        let newUrl = newUserScreenshotURL(disk: disk)
            
        /*
        track("moveToUserScreenshots")
        track("\(oldUrl)")
        track("\(newUrl)")
        */
        
        if oldUrl != nil && newUrl != nil {
            do {
                try fm.copyItem(at: oldUrl!, to: newUrl!)
                deleteScreenshot(base: "auto", disk: disk, nr: nr)
            } catch let error as NSError {
                print(error)
            }
        }
    }
    func moveToUserScreenshots(disk: UInt64, nr: Int) {
        moveToUserScreenshots(disk: String(format: "%X", disk), nr: nr)
    }
    
    func saveAutoScreenshot(fingerprint: Int) {
        
        track("saveAutoScreenshot: \(fingerprint)")
        
        let num = amiga.numAutoScreenshots()
        if fingerprint == 0 || num == 0 { return }
        
        let image = amiga.autoScreenshotImage(num - 1)
        let disk = String(format: "%X", fingerprint)
        
        if let url = newAutoScreenshotURL(disk: disk) {
            
            track("Saving screenshot to \(url)")
            
            let type = NSBitmapImageRep.FileType.jpeg
            let data = image.representation(using: type)
            try? data?.write(to: url, options: .atomic)
        }
    }
}
