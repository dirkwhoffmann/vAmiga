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
    @IBOutlet weak var title: NSTextField!
    @IBOutlet weak var subtitle1: NSTextField!
    @IBOutlet weak var subtitle2: NSTextField!
    @IBOutlet weak var subtitle3: NSTextField!
    @IBOutlet weak var warning: NSTextField!
    @IBOutlet weak var biohazardIcon: NSImageView!
    @IBOutlet weak var decontaminate: NSButton!
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
        
        let density = disk?.diskDensity
                
        var name: String
        switch type {
        case .FILETYPE_ADF, .FILETYPE_DMS, .FILETYPE_EXE, .FILETYPE_DIR:
            name = density == .DISK_HD ? "hd_adf" : "dd_adf"
        case .FILETYPE_IMG:
            name = "dd_dos"
        default:
            name = ""
        }
        
        if writeProtect { name += "_protected" }
        return NSImage.init(named: name)
    }

    var titleText: String {
        
        switch type {
        
        case .FILETYPE_ADF:
            return "Amiga Disk File (ADF)"
        case .FILETYPE_IMG:
            return "PC Disk Image"
        case .FILETYPE_DMS:
            return "Disk Masher System (DMS)"
        case .FILETYPE_EXE:
            return "ADF containing a single Amiga executable"
        case .FILETYPE_DIR:
            return "ADF (converted host directory)"
        default:
            return "???"
        }
    }

    func updateSubTitle1() {
                
        let t = disk?.numTracks ?? 0
        let s = disk?.numSectors ?? 0
        let n = disk?.numSides == 1 ? "Single" : "Double"

        let den = disk?.diskDensity
        let d = den == .DISK_SD ? "single" : den == .DISK_DD ? "double" : "high"

        subtitle1.stringValue = "\(n) sided, \(d) density disk, \(t) tracks with \(s) sectors each"
    }

    func updateSubTitle2() {
        
        let dos = disk!.dos
        
        subtitle2.stringValue = dos.description

    }

    func updateSubTitle3() {

        let virus = disk!.bootBlockType == .BB_VIRUS
        let name = disk!.bootBlockName!

        subtitle3.stringValue = virus ? "Contagious boot block detected (\(name))" : name
        subtitle3.textColor = virus ? .systemRed : .secondaryLabelColor
    }
    
    override func showSheet(completionHandler handler:(() -> Void)? = nil) {
    
        track()

        type = myDocument.amigaAttachment?.type

        if let attachment = myDocument.amigaAttachment as? ADFFileProxy {
            disk = attachment
        }
        if let attachment = myDocument.amigaAttachment as? IMGFileProxy {
            disk = attachment
        }
        if let attachment = myDocument.amigaAttachment as? DMSFileProxy {
            disk = attachment
        }
        if let attachment = myDocument.amigaAttachment as? EXEFileProxy {
            disk = attachment
        }
        if let attachment = myDocument.amigaAttachment as? DIRFileProxy {
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
        updateSubTitle1()
        updateSubTitle2()
        updateSubTitle3()
        biohazardIcon.isHidden = disk?.bootBlockType != .BB_VIRUS
        decontaminate.isHidden = disk?.bootBlockType != .BB_VIRUS

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
        
        track("insertDiskAction df\(sender.tag)")
                
        switch disk {
        
        case _ as ADFFileProxy:
            amiga.diskController.insert(sender.tag, adf: disk as? ADFFileProxy)

        case _ as IMGFileProxy:
            amiga.diskController.insert(sender.tag, img: disk as? IMGFileProxy)

        case _ as DMSFileProxy:
            amiga.diskController.insert(sender.tag, dms: disk as? DMSFileProxy)

        case _ as EXEFileProxy:
            amiga.diskController.insert(sender.tag, exe: disk as? EXEFileProxy)

        case _ as DIRFileProxy:
            amiga.diskController.insert(sender.tag, dir: disk as? DIRFileProxy)

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
