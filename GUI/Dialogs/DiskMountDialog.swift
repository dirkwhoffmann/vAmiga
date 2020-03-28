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

    let carouselType = iCarouselType.coverFlow2
    
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
        carousel.scrollToItem(at: screenshots.count / 2, animated: false)
        
        UserDefaults.standard.register(defaults: ["VAmigaPreviewImages": true])

        update()
    }
    
    override func windowDidLoad() {

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            self.carousel.type = self.carouselType
            self.carousel.isHidden = false
        }
        track()
    }
         
    func updateScreenshots() {
        
        let fingerprint = disk.fnv()
        let folder = String(format: "%02X", fingerprint)
        let urls = parent.screenshotFolderContents(folder)
        
        screenshots = []
        if recordPreviewImages {
            for url in urls {
                if let image = NSImage.init(contentsOf: url) {
                    screenshots.append(image.roundCorners(withRadius: 10.0))
                }
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
        // carousel.type = carouselType
        update()
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
        
        track()
        var itemView: NSImageView
        
        // Reuse view if available, otherwise create a new view
        if let view = view as? NSImageView {
            
            itemView = view
            
        } else {
            
            let height = carousel.frame.height
            let width = carousel.frame.width
            track("h w : \(height) \(width)")
            
            itemView = NSImageView(frame: CGRect(x: 0, y: 0, width: height * 4 / 3, height: height))
            
            // itemView = NSImageView(frame: CGRect(x: 0, y: 0, width: 266, height: 200))
            itemView.image = screenshots[index % screenshots.count]
        }
        
        track("iCarousel: \(itemView)")
        return itemView
    }
}
