// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class ScreenshotDialog: DialogController {
    
    var now: Date!

    @IBOutlet weak var selector: NSSegmentedControl!
    @IBOutlet weak var carousel: iCarousel!
    @IBOutlet weak var leftButton: NSButton!
    @IBOutlet weak var rightButton: NSButton!
    @IBOutlet weak var actionButton: NSButton!
    @IBOutlet weak var finderLabel: NSTextField!
    @IBOutlet weak var finderButton: NSButton!
    @IBOutlet weak var itemLabel: NSTextField!
    @IBOutlet weak var text1: NSTextField!
    @IBOutlet weak var text2: NSTextField!

    // Fingerprint of disk in df0
    var checksum = UInt64(0)
    
    var latestIndex = 0
    var favoriteIndex = 0
    
    // Computed variables
    var myDocument: MyDocument { return parent.mydocument! }
    var userView: Bool { return selector.selectedSegment == 0 }
    var autoView: Bool { return selector.selectedSegment == 1 }
    var numItems: Int { return carousel.numberOfItems }
    var currentItem: Int { return carousel.currentItemIndex }
    var centerItem: Int { return numItems / 2 }
    var lastItem: Int { return numItems - 1 }
    var empty: Bool { return numItems == 0 }

    override func windowWillLoad() {
   
        track()
    }
    
    override func sheetDidShow() {
  
        track()
        
        now = Date()
        
        parent.stopScreenshotTimer()
        updateLabels()
        
        self.carousel.type = iCarouselType.coverFlow
        self.carousel.isHidden = false
        self.updateCarousel(goto: self.centerItem, animated: false)
    }
    
    func updateLabels() {
        
        track("numItems = \(numItems)")
        
        carousel.isHidden = false
        itemLabel.isHidden = empty
        text1.isHidden = false
        text2.isHidden = false

        leftButton.isEnabled = currentItem > 0
        rightButton.isEnabled = currentItem >= 0 && currentItem < lastItem
        itemLabel.stringValue = "\(currentItem + 1) / \(numItems)"

        userView ? updateUserLabels() : updateAutoLabels()
    }
    
    func updateUserLabels() {
        
        actionButton.image = NSImage.init(named: "trashTemplate")
        actionButton.toolTip = "Delete screenshot from disk"
        actionButton.isHidden = empty
        leftButton.isHidden = empty
        rightButton.isHidden = empty
        finderLabel.isHidden = empty
        finderButton.isHidden = empty
        
        if let screenshot = myDocument.userScreenshots.element(at: currentItem) {
            text1.stringValue = screenshot.description
            text2.stringValue = screenshot.sizeString + " pixels"
        } else {
            text1.stringValue = "No screenshots available"
            text2.stringValue = ""
        }
    }
   
    func updateAutoLabels() {
        
        actionButton.image = NSImage.init(named: "starTemplate")
        actionButton.toolTip = "Move screenshot to favorites"
        actionButton.isHidden = empty
        leftButton.isHidden = true
        rightButton.isHidden = true
        finderLabel.isHidden = true
        finderButton.isHidden = true
        
        if let screenshot = myDocument.autoScreenshots.element(at: currentItem) {
            text1.stringValue = screenshot.description
            text2.stringValue = screenshot.sizeString + " pixels"
        } else {
            text1.stringValue = "No screenshots available"
            text2.stringValue = ""
        }
    }
    
    func updateCarousel(goto item: Int, animated: Bool) {
        
        carousel.reloadData()
        let index = min(item, lastItem)
        if index >= 0 { carousel.scrollToItem(at: index, animated: animated) }
        carousel.layOutItemViews()
        updateLabels()
    }

    func updateCarousel(animated: Bool = false) {
        
        updateCarousel(goto: -1, animated: animated)
    }

    @IBAction func selectorAction(_ sender: NSSegmentedControl!) {
                
        updateCarousel(goto: Int.max, animated: false)
    }
    
    @IBAction func leftAction(_ sender: NSButton!) {
        
        if currentItem > 0 {
            myDocument.userScreenshots.swapAt(currentItem, currentItem - 1)
            updateCarousel(goto: currentItem - 1, animated: true)
        }
    }

    @IBAction func rightAction(_ sender: NSButton!) {
        
        if currentItem < lastItem {
            myDocument.userScreenshots.swapAt(currentItem, currentItem + 1)
            updateCarousel(goto: currentItem + 1, animated: true)
        }
    }

    @IBAction func actionAction(_ sender: NSButton!) {
        
        if userView {
            
            myDocument.userScreenshots.remove(at: currentItem)
            
        } else {
            
            if let screenshot = myDocument.autoScreenshots.element(at: currentItem) {
                myDocument.userScreenshots.append(screenshot)
                myDocument.autoScreenshots.remove(at: currentItem)
            }
        }
        
        updateCarousel(goto: currentItem - 1, animated: true)
    }

    @IBAction func finderAction(_ sender: NSButton!) {
        
        if let url = Screenshot.folder(forDisk: checksum) {
            
            try? myDocument.persistScreenshots()
            NSWorkspace.shared.open(url)
        }
    }
    
    @IBAction override func cancelAction(_ sender: Any!) {
        
        track()
                        
        hideSheet()
        
        parent.startScreenshotTimer()
        
        carousel.isHidden = true
        leftButton.isHidden = true
        rightButton.isHidden = true
        actionButton.isHidden = true
        text1.stringValue = ""
        text2.stringValue = ""
        
        try? myDocument.persistScreenshots()
    }
    
    /*
    func saveSnapshotAs(item: Int, auto: Bool) {
         
         track()
         
         let panel = NSSavePanel()
         panel.prompt = "Save As"
         panel.allowedFileTypes = ["vAmiga"]
        
        panel.beginSheetModal(for: window!, completionHandler: { result in
            if result == .OK {
                if let url = panel.url {
                    
                    if auto {
                        if let s = self.myDocument.autoSnapshots.element(at: item) {
                            try? s.data()?.write(to: url)
                        }
                    } else {
                        if let s = self.myDocument.userSnapshots.element(at: item) {
                            try? s.data()?.write(to: url)
                        }
                    }
                }
            }
         })
     }
    */
}

//
// iCarousel data source and delegate
//

extension ScreenshotDialog: iCarouselDataSource, iCarouselDelegate {
    
    func numberOfItems(in carousel: iCarousel) -> Int {
                
        if userView {
            return myDocument.userScreenshots.count
        } else {
            return myDocument.autoScreenshots.count
        }
    }
    
    func carousel(_ carousel: iCarousel, viewForItemAt index: Int, reusing view: NSView?) -> NSView {
        
        let h = carousel.frame.height - 10
        let w = h * 4 / 3
        let itemView = NSImageView(frame: CGRect(x: 0, y: 0, width: w, height: h))
        
        itemView.image = userView ?
            myDocument.userScreenshots.element(at: index)?.screen?.roundCorners() :
            myDocument.autoScreenshots.element(at: index)?.screen?.roundCorners()
    
        /*
        itemView.wantsLayer = true
        itemView.layer?.cornerRadius = 10.0
        itemView.layer?.masksToBounds = true
        */
        
        return itemView
    }
    
    func carouselCurrentItemIndexDidChange(_ carousel: iCarousel) {
        
        track()
        updateLabels()
    }
}
