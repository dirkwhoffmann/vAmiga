// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class StorageDialog: DialogController {
    
    @IBOutlet weak var selector: NSSegmentedControl!

    @IBOutlet weak var autoText1: NSTextField!
    @IBOutlet weak var autoText2: NSTextField!
    @IBOutlet weak var autoCarousel: iCarousel!
    @IBOutlet weak var autoFinderText: NSTextField!
    @IBOutlet weak var autoFinderButton: NSButton!
    @IBOutlet weak var autoButton1: NSButton!
    @IBOutlet weak var autoButton2: NSButton!
    @IBOutlet weak var autoButton3: NSButton!

    @IBOutlet weak var userText1: NSTextField!
    @IBOutlet weak var userText2: NSTextField!
    @IBOutlet weak var userCarousel: iCarousel!
    @IBOutlet weak var userFinderText: NSTextField!
    @IBOutlet weak var userFinderButton: NSButton!
    @IBOutlet weak var userButton1: NSButton!
    @IBOutlet weak var userButton2: NSButton!
    @IBOutlet weak var userButton3: NSButton!

    let carouselType = iCarouselType.coverFlow

    // Fingerprint of disk in df0
    var checksum = UInt64(0)

    var snapshots: Bool { return selector.selectedSegment == 0 }
    
    override func windowDidLoad() {
        
        track()

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            self.autoCarousel.type = self.carouselType
            self.autoCarousel.isHidden = false
            self.autoCarousel.scrollToItem(at: 0, animated: false)
            self.userCarousel.type = self.carouselType
            self.userCarousel.isHidden = false
            self.autoCarousel.scrollToItem(at: 0, animated: false)
        }
    }
    
    func update() {
         
        if snapshots {
            autoText1.stringValue = "Automatically saved snapshots"
            autoText2.stringValue = "Ipsum lorem"

            autoButton1.image = NSImage.init(named: "NSRefreshFreestandingTemplate")
            autoButton1.toolTip = "Restore snapshot"
            autoButton2.image = NSImage.init(named: "saveTemplate")
            autoButton2.toolTip = "Save snapshot"
            autoButton3.image = NSImage.init(named: "trashTemplate")
            autoButton3.toolTip = "Delete snapshot"

            userText1.stringValue = "Manually saved snapshots"
            userText2.stringValue = "Ipsum lorem"

            userButton1.image = NSImage.init(named: "NSRefreshFreestandingTemplate")
            userButton1.toolTip = "Restore snapshot"
            userButton2.image = NSImage.init(named: "saveTemplate")
            userButton2.toolTip = "Save snapshot"
            userButton3.image = NSImage.init(named: "trashTemplate")
            userButton3.toolTip = "Delete snapshot"

        } else {
            
            autoText1.stringValue = "Automatically saved screenshots"
            autoText2.stringValue = "Ipsum lorem"
            
            autoButton1.image = NSImage.init(named: "NSGoLeftTemplate")
            autoButton1.toolTip = "Move item left"
            autoButton2.image = NSImage.init(named: "starTemplate")
            autoButton2.toolTip = "Move to favorites"
            autoButton3.image = NSImage.init(named: "NSGoRightTemplate")
            autoButton3.toolTip = "Move item right"
            
            userText1.stringValue = "Manually saved screenshots"
            userText2.stringValue = "Ipsum lorem"
            
            userButton1.image = NSImage.init(named: "NSGoLeftTemplate")
            userButton1.toolTip = "Move item left"
            userButton2.image = NSImage.init(named: "trashTemplate")
            userButton2.toolTip = "Delete screenshot"
            userButton3.image = NSImage.init(named: "NSGoRightTemplate")
            userButton3.toolTip = "Move item right"
        }
        
        autoFinderText.isHidden = snapshots
        autoFinderButton.isHidden = snapshots
     }
    
    func updateAutoCarousel() {
        
        autoCarousel.reloadData()
        /*
         if scrollToCenter {
         let center = screenshots.count / 2
         self.carousel.scrollToItem(at: center, animated: false)
         }
         */
        autoCarousel.layOutItemViews()
    }
    
    func updateUserCarousel() {
        
        userCarousel.reloadData()
        /*
         if scrollToCenter {
         let center = screenshots.count / 2
         self.carousel.scrollToItem(at: center, animated: false)
         }
         */
        userCarousel.layOutItemViews()
    }
    
    @IBAction func selectorAction(_ sender: NSSegmentedControl!) {
        
        track()

        update()
        updateAutoCarousel()
        updateUserCarousel()
    }

    @IBAction func autoButton1Action(_ sender: NSButton!) {
        
        track()

        if snapshots {
            track("Restore auto snapshot")
        } else {
            track("Move auto screenshot left")
        }
    }

    @IBAction func autoButton2Action(_ sender: NSButton!) {
        
        track()
        
        if snapshots {
            track("Save auto snapshot")
        } else {
            track("Move to favorites")
        }
    }
    
    @IBAction func autoButton3Action(_ sender: NSButton!) {
        
        track()
        
        if snapshots {
            track("Delete auto snapshot")
        } else {
            track("Move auto screenshot right")
        }
    }

    @IBAction func autoFinderAction(_ sender: NSButton!) {
        
        track()
        
        if let url = Screenshot.autoFolder(checksum: checksum) {
            NSWorkspace.shared.open(url)
        }
    }

    @IBAction func userButton1Action(_ sender: NSButton!) {
        
        track()
        
        if snapshots {
            track("Restore user snapshot")
        } else {
            track("Move user screenshot left")
        }
    }

     @IBAction func userButton2Action(_ sender: NSButton!) {
        
        track()
        
        if snapshots {
            track("Save user snapshot")
        } else {
            track("Delete user screenshot")
        }
     }
    
    @IBAction func userButton3Action(_ sender: NSButton!) {
        
        track()
        
        if snapshots {
            track("Delete auto snapshot")
        } else {
            track("Move user screenshot right")
        }
     }

    @IBAction func userFinderAction(_ sender: NSButton!) {
        
        track()
        
        if let url = Screenshot.userFolder(checksum: checksum) {
            NSWorkspace.shared.open(url)
        }
    }
}

//
// iCarousel data source and delegate
//

extension StorageDialog: iCarouselDataSource, iCarouselDelegate {
    
    func numberOfItems(in carousel: iCarousel) -> Int {
        
        assert(carousel == autoCarousel || carousel == userCarousel)
        let auto = carousel == autoCarousel
        
        if snapshots {
            return 0
        } else {
            return auto ? 10 : 32 // autoScreenshots.count : userScreenshots.count
        }
    }
    
    func carousel(_ carousel: iCarousel, viewForItemAt index: Int, reusing view: NSView?) -> NSView {
        
        assert(carousel == autoCarousel || carousel == userCarousel)
        let auto = carousel == autoCarousel

        let h = carousel.frame.height - 10
        let w = h * 4 / 3
        let itemView = NSImageView(frame: CGRect(x: 0, y: 0, width: w, height: h))
                
        if snapshots {
            itemView.image = NSImage.init(named: "adf")
        } else {
            itemView.image = auto ? NSImage.init(named: "rom_alert") : NSImage.init(named: "rom_diag")
        }
        
        // track("iCarousel: \(itemView)")
        return itemView
    }
}
