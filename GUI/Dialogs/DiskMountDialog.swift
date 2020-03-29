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
    @IBOutlet weak var recordButton: NSButton!
    @IBOutlet weak var recordButtonText: NSTextField!
    @IBOutlet weak var listSelector: NSSegmentedControl!
    @IBOutlet weak var leftButton: NSButton!
    @IBOutlet weak var middleButton: NSButton!
    @IBOutlet weak var rightButton: NSButton!

    private var userList: Bool { return listSelector.indexOfSelectedItem == 0 }
    
    let carouselType = iCarouselType.coverFlow
    
    var disk: ADFFileProxy!
    var writeProtect = false
    var shrinked: Bool { return window!.frame.size.height < 300 }
    var screenshots: [NSImage] = []
    var recordPreviewImages: Bool {
        get {
            return UserDefaults.standard.bool(forKey: "VAmigaPreviewImages")
        }
        set {
            UserDefaults.standard.set(newValue, forKey: "VAmigaPreviewImages")
        }
    }

    override func showSheet(completionHandler handler:(() -> Void)? = nil) {
    
        track()
        if let attachment = myDocument?.amigaAttachment as? ADFFileProxy {
            
            disk = attachment
            updateScreenshots()
            super.showSheet(completionHandler: handler)
        }
    }
    
    override public func awakeFromNib() {

        track()
        window?.makeFirstResponder(carousel)
        
        UserDefaults.standard.register(defaults: ["VAmigaPreviewImages": true])

        update()
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
        
        let urls = parent.autoScreenshotFolderContents(disk: disk.fnv())
        
        screenshots = []
        for url in urls {
            track("Creating image for URL \(url)")
            if let image = NSImage.init(contentsOf: url) {
                screenshots.append(image.roundCorners(withRadius: 10.0))
            }
        }
        
        track("Found \(screenshots.count) screenshots for this disk")

        if screenshots.count == 0 {
            
            // Add a default image
            let name = recordPreviewImages ? "noise_camera" : "noise"
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
        
        // Buttons
        recordButton.state = recordPreviewImages ? .on : .off
        recordButtonText.isHidden = !recordPreviewImages
        
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

    @IBAction func recordPreviewImagesAction(_ sender: NSButton!) {
        
        recordPreviewImages = !recordPreviewImages
        updateScreenshots()
        carousel.reloadData()
        carousel.layOutItemViews()
        update()
    }
    
    @IBAction func listAction(_ sender: NSSegmentedControl!) {

        track("\(userList)")
    }
    
    @IBAction func leftAction(_ sender: NSButton!) {

        track("leftAction")
        
        let max = carousel.numberOfItems
        let index = carousel.currentItemIndex
        
        track("max = \(max) index = \(index)")
        
        if index > 0 {
            
            let fingerprint = disk.fnv()
            let disk = String(format: "%X", fingerprint)
            
            parent.swapAutoScreenshots(disk: disk, nr1: index, nr2: index - 1)
            
            updateScreenshots()
            carousel.reloadData()
            carousel.layOutItemViews()
        }
    }
    
    @IBAction func rightAction(_ sender: NSButton!) {

        track("rightAction")
    }

    @IBAction func middleAction(_ sender: NSButton!) {

        track("middleAction")
        
        let index = carousel.currentItemIndex
        track("currentItemIndex = \(index)")
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
        
        let h = carousel.frame.height
        let w = h * 4 / 3
        let itemView = NSImageView(frame: CGRect(x: 0, y: 0, width: w, height: h))
        
        assert(index < screenshots.count)
        itemView.image = screenshots[index % screenshots.count]

        track("iCarousel: \(itemView)")
        return itemView
    }
}
