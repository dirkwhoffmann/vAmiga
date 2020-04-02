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

    let carouselType = iCarouselType.coverFlow
    
    var disk: ADFFileProxy!
    var writeProtect = false
    var shrinked: Bool { return window!.frame.size.height < 300 }
    var screenshots: [NSImage] = []
 
    var screenshotFolder: URL?
        
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
                    
            // Check for screenshots
            var userFolder: URL? { return Screenshot.userFolder(checksum: disk.fnv()) }
            var autoFolder: URL? { return Screenshot.autoFolder(checksum: disk.fnv()) }
            
            if Screenshot.collectUserFiles(checksum: disk.fnv()).count > 0 {
                track("Found images in user screenshot folder")
                screenshotFolder = userFolder
            } else if Screenshot.collectAutoFiles(checksum: disk.fnv()).count > 0 {
                track("Found images in auto screenshot folder")
                screenshotFolder = autoFolder
            }
            
            update()
            updateCarousel()
    }
    
    override func windowDidLoad() {

        if screenshotFolder == nil {

            setHeight(196)
            
        } else {
            
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                self.carousel.type = self.carouselType
                self.carousel.isHidden = false
                self.leftButton.isHidden = false
                self.middleButton.isHidden = false
                self.rightButton.isHidden = false
                self.carousel.scrollToItem(at: self.screenshots.count / 2, animated: false)
            }
        }
        track()
    }
    
    func setHeight(_ newHeight: CGFloat) {
        
        var rect = window!.frame
        rect.origin.y += rect.size.height - newHeight
        rect.size.height = newHeight
        
        window?.setFrame(rect, display: true)
        update()
    }

    func updateScreenshots() {

        // Get a list of filenames
        let urls = Screenshot.collectFiles(in: screenshotFolder)
        
        // Create images
        screenshots = []
        for url in urls {
            if let image = NSImage.init(contentsOf: url) {
                screenshots.append(image.roundCorners(withRadius: 20.0))
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

        Screenshot.deleteAutoFolder(checksum: disk.fnv())
        
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
    
    @IBAction func folderSelectorAction(_ sender: NSSegmentedControl!) {

        update()
        updateCarousel(scrollToCenter: true)
    }

    @IBAction func showInFinderAction(_ sender: NSButton!) {

        track()
        if let folder = screenshotFolder {
            NSWorkspace.shared.open(folder)
        }
    }

    @IBAction func leftAction(_ sender: NSButton!) {

        let index = carousel.currentItemIndex
        track("leftAction: \(index)")
                
        if index > 0 {
            Screenshot.swap(item: index, with: index - 1, in: screenshotFolder)
            updateCarousel()
            carousel.scrollToItem(at: index - 1, animated: true)
        }
    }
    
    @IBAction func rightAction(_ sender: NSButton!) {
        
        let index = carousel.currentItemIndex
        track("rightAction: \(index)")
        
        if index < carousel.numberOfItems - 1 {
            Screenshot.swap(item: index, with: index + 1, in: screenshotFolder)
            updateCarousel()
            carousel.scrollToItem(at: index + 1, animated: true)
        }
    }

    @IBAction func middleAction(_ sender: NSButton!) {

        let index = carousel.currentItemIndex
        track("middleAction: \(index)")

        track("Deleting screenshot")
        Screenshot.deleteUser(item: index, checksum: disk.fnv())
        
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
// iCarousel data source and delegate
//

extension DiskMountDialog: iCarouselDataSource, iCarouselDelegate {
    
    func numberOfItems(in carousel: iCarousel) -> Int {
        
        return screenshots.count
    }
    
    func carousel(_ carousel: iCarousel, viewForItemAt index: Int, reusing view: NSView?) -> NSView {
        
        let h = carousel.frame.height - 10
        let w = h * 4 / 3
        let itemView = NSImageView(frame: CGRect(x: 0, y: 0, width: w, height: h))
        
        assert(index < screenshots.count)
        itemView.image = screenshots[index % screenshots.count]

        // track("iCarousel: \(itemView)")
        return itemView
    }
}
