// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class ImporterDialog: DialogController {
        
    @IBOutlet weak var diskIcon: NSImageView!
    @IBOutlet weak var virusIcon: NSImageView!
    @IBOutlet weak var title: NSTextField!
    @IBOutlet weak var layoutInfo: NSTextField!
    @IBOutlet weak var volumeInfo: NSTextField!
    @IBOutlet weak var bootInfo: NSTextField!
    @IBOutlet weak var warning: NSTextField!
    @IBOutlet weak var decontaminationButton: NSButton!
    @IBOutlet weak var df0Button: NSButton!
    @IBOutlet weak var df1Button: NSButton!
    @IBOutlet weak var df2Button: NSButton!
    @IBOutlet weak var df3Button: NSButton!
    @IBOutlet weak var carousel: iCarousel!
    
    var disk: DiskFileProxy?
    var type: FileType?
    var writeProtect = false
    var screenshots: [Screenshot] = []
    
    var myDocument: MyDocument { return parent.mydocument! }
    var numItems: Int { return carousel.numberOfItems }
    var currentItem: Int { return carousel.currentItemIndex }
    var centerItem: Int { return numItems / 2 }
    var lastItem: Int { return numItems - 1 }
    var empty: Bool { return numItems == 0 }
        
    var titleText: String {
        
        switch type {
        
        case .ADF:
            return "Amiga Disk File (ADF)"
        case .EXT:
            return "Amiga Disk File (Extended ADF)"
        case .IMG:
            return "PC Disk Image"
        case .DMS:
            return "Disk Masher System (DMS)"
        case .EXE:
            return "ADF (Converted Amiga executable)"
        case .DIR:
            return "ADF (Converted Host Directory)"
        default:
            return "???"
        }
    }
    
    override func showSheet(completionHandler handler:(() -> Void)? = nil) {
    
        track()

        type = myDocument.attachment?.type

        if let attachment = myDocument.attachment as? ADFFileProxy {
            disk = attachment
        }
        if let attachment = myDocument.attachment as? EXTFileProxy {
            disk = attachment
        }
        if let attachment = myDocument.attachment as? IMGFileProxy {
            disk = attachment
        }
        if let attachment = myDocument.attachment as? DMSFileProxy {
            disk = attachment
        }
        if let attachment = myDocument.attachment as? EXEFileProxy {
            disk = attachment
        }
        if let attachment = myDocument.attachment as? FolderProxy {
            disk = attachment
        }

        if disk != nil {
            
            // Load screenshots (if any)
            for url in Screenshot.collectFiles(forDisk: disk!.fnv) {
                if let screenshot = Screenshot(fromUrl: url) {
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
        
        if empty {

            setHeight(222)
            
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
                    
        // Update icons
        diskIcon.image = disk!.icon(protected: writeProtect)
        virusIcon.isHidden = disk!.hasVirus == false
        decontaminationButton.isHidden = disk?.hasVirus == false

        // Update disk description
        title.stringValue = titleText
        layoutInfo.stringValue = disk!.layoutInfo
        volumeInfo.stringValue = disk!.dos.description
        bootInfo.stringValue = disk!.bootInfo
        bootInfo.textColor = disk!.hasVirus ? .warningColor : .secondaryLabelColor
        
        // Determine enabled drives
        let t = disk!.diskType
        let d = disk!.diskDensity
        let dc = amiga.diskController.getConfig()
        let df0 = dc.connected.0 && amiga.df0.isInsertable(t, density: d)
        let df1 = dc.connected.1 && amiga.df1.isInsertable(t, density: d)
        let df2 = dc.connected.2 && amiga.df2.isInsertable(t, density: d)
        let df3 = dc.connected.3 && amiga.df3.isInsertable(t, density: d)
        df0Button.isEnabled = df0
        df1Button.isEnabled = df1
        df2Button.isEnabled = df2
        df3Button.isEnabled = df3
        
        // Show warning message if no compatible drives are available
        warning.isHidden = df0 || df1 || df2 || df3
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

    @IBAction func decontaminateAction(_ sender: NSButton!) {
        
        disk!.killVirus()
        update()
    }

    @IBAction func insertDiskAction(_ sender: NSButton!) {
        
        let drive = amiga.df(sender.tag)!
                    
        do {
            try drive.swap(file: disk!)
            drive.setWriteProtection(writeProtect)
            hideSheet()
        } catch {
            (error as? VAError)?.warning("Failed to insert disk")
        }
    }
    
    @IBAction override func cancelAction(_ sender: Any!) {
         
         track()
         hideSheet()
     }
}

extension ImporterDialog: NSWindowDelegate {
    
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

extension ImporterDialog: iCarouselDataSource, iCarouselDelegate {
    
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
