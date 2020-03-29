// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class DiskMountDialog: DialogController {
        
    @IBOutlet weak var diskIcon: NSImageView!
    @IBOutlet weak var infoText: NSTextField!
    @IBOutlet weak var warningText: NSTextField!
    @IBOutlet weak var df0Button: NSButton!
    @IBOutlet weak var df1Button: NSButton!
    @IBOutlet weak var df2Button: NSButton!
    @IBOutlet weak var df3Button: NSButton!
    @IBOutlet weak var carousel: iCarousel!
    @IBOutlet weak var leftButton: NSButton!
    @IBOutlet weak var middleButton: NSButton!
    @IBOutlet weak var rightButton: NSButton!
    @IBOutlet weak var folderPopup: NSPopUpButton!
        
    let carouselType = iCarouselType.coverFlow
    
    var disk: ADFFileProxy!
    var writeProtect = false
    var shrinked: Bool { return window!.frame.size.height < 300 }
    var screenshots: [NSImage] = []
 
    override func showSheet(completionHandler handler:(() -> Void)? = nil) {
    
        track()
        if let attachment = myDocument?.amigaAttachment as? ADFFileProxy {
            
            disk = attachment
            super.showSheet(completionHandler: handler)
        }
    }
    
    override public func awakeFromNib() {

        track()
        window?.makeFirstResponder(carousel)
        
        if let userPath = userScreenshotFolder {
            folderPopup.item(at: 0)?.title = userPath.path
        }
        if let autoPath = autoScreenshotFolder {
            folderPopup.item(at: 1)?.title = autoPath.path
        }
        
        update()
        updateCarousel()
    }
    
    override func windowDidLoad() {

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            self.carousel.type = self.carouselType
            self.carousel.isHidden = false
            self.carousel.scrollToItem(at: self.screenshots.count / 2, animated: true)
        }
        track()
    }
         
    func updateScreenshots() {
                
        // Scan for filenames
        var urls: [URL]
        if favorites {
            urls = parent.userScreenshotFolderContents(disk: disk.fnv())
        } else {
            urls = parent.autoScreenshotFolderContents(disk: disk.fnv())
        }
        
        // Create images
        screenshots = []
        for url in urls {
            if let image = NSImage.init(contentsOf: url) {
                screenshots.append(image.roundCorners(withRadius: 10.0))
            }
        }
        
        track("Found \(screenshots.count) screenshots for this disk")

        // Add a default image if the list is empty
        if screenshots.count == 0 {
            let name = "noise_camera"
            screenshots.append(NSImage.init(named: name)!)
        }
    }
     
    func update() {
        
        // Update disk icon
        diskIcon.image = NSImage.init(named: writeProtect ? "adf_protected" : "adf")
        
        // Update text fields
        let typeName = [
            DISK_35_DD.rawValue: "3.5\"DD Amiga",
            DISK_35_DD_PC.rawValue: "3.5\"DD PC",
            DISK_35_HD.rawValue: "3.5\"HD Amiga",
            DISK_35_HD_PC.rawValue: "3.5\"HD PC",
            DISK_525_SD.rawValue: "5.25\"SD PC"
        ]
        let str = typeName[disk.diskType().rawValue]!
        infoText.stringValue = "A byte-accurate image of \(str) diskette."
        let compatible = disk.diskType() == DISK_35_DD
        warningText.isHidden = compatible
                
        // Preview images
        if favorites {
            middleButton.image = NSImage.init(named: "trashTemplate")
            middleButton.toolTip = "Delete permanently"
        } else {
            middleButton.image = NSImage.init(named: "star")
            middleButton.toolTip = "Move to favorites"
        }
        
        // Check for available drives
        let dc = amiga.diskController.getConfig()

        let connected0 = dc.connected.0
        let connected1 = dc.connected.1
        let connected2 = dc.connected.2
        let connected3 = dc.connected.3
        df0Button.isEnabled = compatible && connected0
        df1Button.isEnabled = compatible && connected1
        df2Button.isEnabled = compatible && connected2
        df3Button.isEnabled = compatible && connected3
    }
    
    func updateCarousel(scrollToCenter: Bool = false) {
        
        updateScreenshots()
        carousel.reloadData()
        if scrollToCenter {
            let center = screenshots.count / 2
            self.carousel.scrollToItem(at: center, animated: false)
        }
        carousel.layOutItemViews()
    }

    //
    // Action methods
    //

    @IBAction func insertDiskAction(_ sender: NSButton!) {
        
        track("insertDiskAction df\(sender.tag)")
        
        amiga.diskController.insert(sender.tag, adf: disk)
        amiga.diskController.setWriteProtection(sender.tag, value: writeProtect)

        parent.renderer.rotateDown()
        hideSheet()
    }
    
    @IBAction func writeProtectAction(_ sender: NSButton!) {
        
        writeProtect = sender.state == .on
        update()
    }

    @IBAction func folderAction(_ sender: NSPopUpButton!) {

        update()
        updateCarousel(scrollToCenter: true)
    }

    @IBAction func leftAction(_ sender: NSButton!) {

        let index = carousel.currentItemIndex
        track("leftAction: \(index)")
                
        if index > 0 {
            swapScreenshot(at: index, with: index - 1)
            updateCarousel()
            carousel.scrollToItem(at: index - 1, animated: true)
        }
    }
    
    @IBAction func rightAction(_ sender: NSButton!) {
        
        let index = carousel.currentItemIndex
        track("rightAction: \(index)")
        
        if index < carousel.numberOfItems - 1 {
            swapScreenshot(at: index, with: index + 1)
            updateCarousel()
            carousel.scrollToItem(at: index + 1, animated: true)
        }
    }

    @IBAction func middleAction(_ sender: NSButton!) {

        let index = carousel.currentItemIndex
        track("middleAction: \(index)")

        if favorites {
            track("Deleting screenshot")
            deleteUserScreenshot(at: index)
        } else {
            track("Moving screenshot to favorites")
            moveToFavorites(nr: index)
        }
 
        updateCarousel()
    }
}

extension DiskMountDialog: NSWindowDelegate {
    
    func windowDidResize(_ notification: Notification) {
        
        track()
    }
    
    func windowWillStartLiveResize(_ notification: Notification) {
         
         track()
     }
     
     func windowDidEndLiveResize(_ notification: Notification) {
        
        track()
     }
}

//
// Screenshot storage handling
//

extension DiskMountDialog {
    
    var favorites: Bool { return folderPopup.selectedTag() == 0 }
    
    func filename(forItem nr: Int) -> String {
        
        return String(format: "%03d.jpeg", nr)
    }
    
    static func folderURL(favorites: Bool, checksum: UInt64) -> URL? {
        
        let fm = FileManager.default
        let path = FileManager.SearchPathDirectory.applicationSupportDirectory
        let mask = FileManager.SearchPathDomainMask.userDomainMask
        guard let support = fm.urls(for: path, in: mask).first else { return nil }
        let dir = favorites ? "user/" : "auto/"
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
    
    var userScreenshotFolder: URL? {
        return DiskMountDialog.folderURL(favorites: true, checksum: disk.fnv())
    }
    var autoScreenshotFolder: URL? {
        return DiskMountDialog.folderURL(favorites: false, checksum: disk.fnv())
    }
    var screenshotFolder: URL? {
        return DiskMountDialog.folderURL(favorites: favorites, checksum: disk.fnv())
    }

    func screenshotURL(forItem nr: Int, inFolder folder: URL?) -> URL? {
        
        if (folder == nil) { return nil }
        
        let file = folder!.appendingPathComponent(filename(forItem: nr))
        return FileManager.default.fileExists(atPath: file.path) ? file : nil
    }

    func screenshotURL(forItem nr: Int) -> URL? {
        return screenshotURL(forItem: nr, inFolder: screenshotFolder)
    }

    func userScreenshotURL(forItem nr: Int) -> URL? {
        return screenshotURL(forItem: nr, inFolder: userScreenshotFolder)
    }

    func autoScreenshotURL(forItem nr: Int) -> URL? {
        return screenshotURL(forItem: nr, inFolder: autoScreenshotFolder)
    }

    func newScreenshotURL(inFolder folder: URL?) -> URL? {
        
        if (folder == nil) { return nil }
        
        for i in 0...999 {
            
            let file = folder!.appendingPathComponent(filename(forItem: i))
            
            if !FileManager.default.fileExists(atPath: file.path) {
                return file
            }
        }
        
        return nil
    }

    func newScreenshotURL() -> URL? {
        return newScreenshotURL(inFolder: screenshotFolder)
    }

    func newUserScreenshotURL() -> URL? {
        return newScreenshotURL(inFolder: userScreenshotFolder)
    }

    func newAutoScreenshotURL() -> URL? {
         return newScreenshotURL(inFolder: autoScreenshotFolder)
    }

    func swapScreenshot(at pos1: Int, with pos2: Int, in folder: URL) {
        
        let fm = FileManager.default
        
        let oldUrl = screenshotURL(forItem: pos1, inFolder: folder)
        let newUrl = screenshotURL(forItem: pos2, inFolder: folder)
        let tmpUrl = newScreenshotURL(inFolder: folder)

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
    
    func swapScreenshot(at pos1: Int, with pos2: Int) {
        
        if let folder = screenshotFolder {
            swapScreenshot(at: pos1, with: pos2, in: folder)
        }
    }
    
    func deleteScreenshot(at pos: Int, in folder: URL?) {
        
        if (folder == nil) { return }
        let fm = FileManager.default
        
        if var url = screenshotURL(forItem: pos, inFolder: folder) {
            
            try? FileManager.default.removeItem(at: url)
            
            // Rename all items above the deleted one
            for i in pos ... 998 {
                
                if let above = screenshotURL(forItem: i + 1, inFolder: folder) {
                    
                    track("Renaming \(above) to \(above)")
                    try? fm.moveItem(at: above, to: url)
                    url = above
                    
                } else { break }
            }
        }
    }
    
    func deleteScreenshot(at pos: Int) {
        deleteScreenshot(at: pos, in: screenshotFolder)
    }

    func deleteAutoScreenshot(at pos: Int) {
        deleteScreenshot(at: pos, in: autoScreenshotFolder)
    }

    func deleteUserScreenshot(at pos: Int) {
        deleteScreenshot(at: pos, in: userScreenshotFolder)
    }

    func moveToFavorites(nr: Int) {
        
        let fm = FileManager.default
        
        let oldUrl = autoScreenshotURL(forItem: nr)
        let newUrl = newUserScreenshotURL()
        
        /*
         track("moveToUserScreenshots")
         track("\(oldUrl)")
         track("\(newUrl)")
         */
        
        if oldUrl != nil && newUrl != nil {
            do {
                try fm.copyItem(at: oldUrl!, to: newUrl!)
                deleteAutoScreenshot(at: nr)
            } catch let error as NSError {
                print(error)
            }
        }
    }
}

//
// iCarousel data source and delegate
//

extension DiskMountDialog: iCarouselDataSource, iCarouselDelegate {
    
    func numberOfItems(in carousel: iCarousel) -> Int {
        
        return screenshots.count
    }
    
    func carousel(_ carousel: iCarousel, viewForItemAt index: Int, reusing view: NSView?) -> NSView {
        
        let h = carousel.frame.height
        let w = h * 4 / 3
        let itemView = NSImageView(frame: CGRect(x: 0, y: 0, width: w, height: h))
        
        assert(index < screenshots.count)
        itemView.image = screenshots[index % screenshots.count]

        // track("iCarousel: \(itemView)")
        return itemView
    }
}
