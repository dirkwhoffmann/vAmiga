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
    @IBOutlet weak var title: NSTextField!
    @IBOutlet weak var subtitle: NSTextField!
    @IBOutlet weak var warning: NSTextField!
    @IBOutlet weak var df0Button: NSButton!
    @IBOutlet weak var df1Button: NSButton!
    @IBOutlet weak var df2Button: NSButton!
    @IBOutlet weak var df3Button: NSButton!
    @IBOutlet weak var carousel: iCarousel!
    
    var disk: DiskFileProxy?
    var type: AmigaFileType?
    var writeProtect = false
    var screenshots: [Screenshot] = []
    
    var myDocument: MyDocument { return parent.mydocument! }
    var numItems: Int { return carousel.numberOfItems }
    var currentItem: Int { return carousel.currentItemIndex }
    var centerItem: Int { return numItems / 2 }
    var lastItem: Int { return numItems - 1 }
    var empty: Bool { return numItems == 0 }
    
    var diskIconImage: NSImage? {
        
        switch type {
        
        case .FILETYPE_ADF, .FILETYPE_DMS:
            return NSImage.init(named: writeProtect ? "adf_protected" : "adf")
        case .FILETYPE_IMG:
            return NSImage.init(named: writeProtect ? "dos_protected" : "dos")
        default:
            return nil
        }
    }

    var titleText: String {
        
        switch type {
        
        case .FILETYPE_ADF:
            return "Amiga Disk File (ADF)"
        case .FILETYPE_DMS:
            return "Disk Masher System (DMS)"
        case .FILETYPE_IMG:
            return "PC Disk Image"
        default:
            return "???"
        }
    }

    var subtitleText: String {
        
        let t = disk?.numTracks ?? 0
        let s = disk?.numSectorsPerTrack ?? 0
        let h = disk?.numSides == 1 ? "Single" : "Double"
        
        var d: String
        switch disk?.diskType {
        case .DISK_35_DD: d = "Double"
        case .DISK_35_HD: d = "High"
        case .DISK_525_SD: d = "Single"
        default: d = "???"
        }
        
        return "\(h) sided, \(d) density, \(t) tracks with \(s) sectors each"
    }

    override func showSheet(completionHandler handler:(() -> Void)? = nil) {
    
        track()

        type = myDocument.amigaAttachment?.type

        if let attachment = myDocument.amigaAttachment as? ADFFileProxy {
            disk = attachment
        }
        if let attachment = myDocument.amigaAttachment as? DMSFileProxy {
            disk = attachment.adf()
        }
        if let attachment = myDocument.amigaAttachment as? IMGFileProxy {
            disk = attachment
        }
        
        if disk != nil {
            
            // Load screenshots (if any)
            for url in Screenshot.collectFiles(forDisk: disk!.fnv) {
                if let screenshot = Screenshot.init(fromUrl: url) {
                    screenshots.append(screenshot)
                }
            }
            
            super.showSheet(completionHandler: handler)
        }
    }
    
    override public func awakeFromNib() {
        
        track()
        super.awakeFromNib()
        window?.makeFirstResponder(carousel)
        
        update()
        updateCarousel(goto: centerItem, animated: false)
    }
    
    override func windowDidLoad() {

        /*
        switch type {
            
        case .FILETYPE_ADF:
            title.stringValue = "Amiga Disk File (ADF)"
        case .FILETYPE_DMS:
            title.stringValue = "Disk Masher System (DMS)"
        default:
            title.stringValue = "???"
        }
        */
        
        if empty {

            setHeight(196)
            
        } else {
            
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                self.carousel.type = iCarouselType.coverFlow
                self.carousel.isHidden = false
                self.carousel.scrollToItem(at: self.centerItem, animated: false)
            }
        }
    }
    
    func setHeight(_ newHeight: CGFloat) {
        
        var rect = window!.frame
        rect.origin.y += rect.size.height - newHeight
        rect.size.height = newHeight
        
        window?.setFrame(rect, display: true)
        update()
    }

    func update() {
        
        // Update icon and text fields
        diskIcon.image = diskIconImage
        title.stringValue = titleText
        subtitle.stringValue = subtitleText
        
        let compatible = disk!.diskType == .DISK_35_DD
        warning.isHidden = compatible
                
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
    
    func updateCarousel(goto item: Int = -1, animated: Bool = false) {
        
        carousel.reloadData()
        if item != -1 {
            self.carousel.scrollToItem(at: item, animated: animated)
        }
        carousel.layOutItemViews()
    }

    //
    // Action methods
    //
    
    @IBAction func writeProtectAction(_ sender: NSButton!) {
        
        writeProtect = sender.state == .on
        update()
    }

    @IBAction func insertDiskAction(_ sender: NSButton!) {
        
        track("insertDiskAction df\(sender.tag)")
        
        switch disk {
        
        case _ as ADFFileProxy:
            
            amiga.diskController.insert(sender.tag, adf: disk as? ADFFileProxy)
            
        case _ as IMGFileProxy:
            
            // amiga.diskController.insert(sender.tag, img: disk as? ImgFileProxy)
            break
            
        default:
            break
        }

        amiga.diskController.setWriteProtection(sender.tag, value: writeProtect)     
        hideSheet()
    }
    
    @IBAction override func cancelAction(_ sender: Any!) {
         
         track()
                         
         hideSheet()
         try? myDocument.persistScreenshots()
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
        
        itemView.image =
            screenshots[index].screen?.roundCorners()
        
        return itemView
    }
}
