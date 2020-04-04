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
    
    var myDocument: MyDocument { return parent.mydocument! }
    
    var numAutoScreenshots: Int {
        return myDocument.autoScreenshots.count
    }
    var numUserScreenshots: Int {
        return myDocument.userScreenshots.count
    }
    var numScreenshots: Int {
        if numUserScreenshots > 0 { return numUserScreenshots }
        if numAutoScreenshots > 0 { return numAutoScreenshots }
        return 0
    }
    var displaysUserScreenshots: Bool {
        return numUserScreenshots > 0
    }
    var displaysAutoScreenshots: Bool {
        return numUserScreenshots == 0 && numAutoScreenshots > 0
    }
    var screenshotsModified = false

    override func showSheet(completionHandler handler:(() -> Void)? = nil) {
    
        track()
        if let attachment = myDocument.amigaAttachment as? ADFFileProxy {
            
            disk = attachment

            // Force the screenshot cache to update
            myDocument.adfChecksum = disk.fnv()
            
            super.showSheet(completionHandler: handler)
        }
    }
    
    override public func awakeFromNib() {
        
        track()
        window?.makeFirstResponder(carousel)
        
        update()
        updateCarousel(goto: numScreenshots / 2, animated: false)
    }
    
    override func windowDidLoad() {

        if numScreenshots == 0 {

            setHeight(196)
            
        } else {
            
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                self.carousel.type = self.carouselType
                self.carousel.isHidden = false
                self.leftButton.isHidden = false
                self.middleButton.isHidden = false
                self.rightButton.isHidden = false
                self.carousel.scrollToItem(at: self.numScreenshots / 2, animated: false)
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

    @IBAction func leftAction(_ sender: NSButton!) {

        let index = carousel.currentItemIndex
        track("leftAction: \(index)")
        
        if displaysUserScreenshots, index > 0 {
            myDocument.swapUserScreenshots(index, index - 1)
        }
        if displaysAutoScreenshots, index > 0 {
            myDocument.swapAutoScreenshots(index, index - 1)
        }
        screenshotsModified = true
        updateCarousel(goto: index - 1, animated: true)
    }
    
    @IBAction func rightAction(_ sender: NSButton!) {
        
        let index = carousel.currentItemIndex
        track("rightAction: \(index)")
        
        if displaysUserScreenshots, index < numUserScreenshots - 1 {
            myDocument.swapUserScreenshots(index, index + 1)
        }
        if displaysAutoScreenshots, index < numAutoScreenshots - 1 {
            myDocument.swapAutoScreenshots(index, index + 1)
        }
        screenshotsModified = true
        updateCarousel(goto: index + 1, animated: true)
    }

    @IBAction func middleAction(_ sender: NSButton!) {
        
        let index = carousel.currentItemIndex
        track("middleAction: \(index)")
        
        if displaysUserScreenshots {
            myDocument.removeUserScreenshot(at: index)
        }
        if displaysAutoScreenshots {
            myDocument.removeAutoScreenshot(at: index)
        }
        screenshotsModified = true
        updateCarousel()
    }
    
    @IBAction func insertDiskAction(_ sender: NSButton!) {
        
        track("insertDiskAction df\(sender.tag)")
                
        amiga.diskController.insert(sender.tag, adf: disk)
        amiga.diskController.setWriteProtection(sender.tag, value: writeProtect)
        parent.renderer.rotateDown()
     
        hideSheet()
        try? myDocument.persistScreenshots()
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
        
        return numScreenshots
    }
    
    func carousel(_ carousel: iCarousel, viewForItemAt index: Int, reusing view: NSView?) -> NSView {
        
        let h = carousel.frame.height - 10
        let w = h * 4 / 3
        let itemView = NSImageView(frame: CGRect(x: 0, y: 0, width: w, height: h))
        
        if numUserScreenshots > 0 {
            assert(index < numUserScreenshots)
            itemView.image =
                myDocument.userScreenshots[index].screen?.roundCorners()
        } else {
            assert(index < numAutoScreenshots)
            itemView.image =
                myDocument.autoScreenshots[index].screen?.roundCorners()
        }
        
        return itemView
    }
}
