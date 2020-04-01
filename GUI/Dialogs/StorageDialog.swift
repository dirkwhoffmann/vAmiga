// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class StorageDialog: DialogController {
    
    var now: Date = Date()

    @IBOutlet weak var tabView: NSTabView!
    
    @IBOutlet weak var snapshotSelector: NSSegmentedControl!
    @IBOutlet weak var snapshotCarousel: iCarousel!
    @IBOutlet weak var snapshotRestore: NSButton!
    @IBOutlet weak var snapshotSave: NSButton!
    @IBOutlet weak var snapshotDelete: NSButton!
    @IBOutlet weak var snapshotDescription1: NSTextField!
    @IBOutlet weak var snapshotDescription2: NSTextField!
    
    @IBOutlet weak var screenshotSelector: NSSegmentedControl!
    @IBOutlet weak var screenshotCarousel: iCarousel!
    @IBOutlet weak var screenshotLeft: NSButton!
    @IBOutlet weak var screenshotMiddle: NSButton!
    @IBOutlet weak var screenshotRight: NSButton!
    @IBOutlet weak var screenshotDescription1: NSTextField!
    @IBOutlet weak var screenshotDescription2: NSTextField!
    
    let carouselType = iCarouselType.timeMachine //   coverFlow
    
    // Fingerprint of disk in df0
    var checksum = UInt64(0)
    
    var snapshots: Bool {
        if let id = tabView.selectedTabViewItem?.identifier as? String {
            return id == "Snapshots"
        } else {
            return true
        }
    }
    var autoSnapshots: Bool { return snapshotSelector.selectedSegment == 0 }
    var autoScreenshots: Bool { return screenshotSelector.selectedSegment == 0 }
    var carousel: iCarousel { return autoSnapshots ? snapshotCarousel : screenshotCarousel }
    var autoFolder: URL? { return Screenshot.autoFolder(checksum: checksum) }
    var userFolder: URL? { return Screenshot.userFolder(checksum: checksum) }
    var screenshotFolder: URL? { return autoScreenshots ? autoFolder : userFolder }

    // Auto snapshot cache
    var autoSnapshotImage: [Int: NSImage] = [:]
    var autoSnapshotDescription1: [Int: String] = [:]
    var autoSnapshotDescription2: [Int: String] = [:]
    var autoSnapshotSlotForRow: [Int: Int] = [:]
    
    // User snapshot cache
    var userSnapshotImage: [Int: NSImage] = [:]
    var userSnapshotDescription1: [Int: String] = [:]
    var userSnapshotDescription2: [Int: String] = [:]
    var userSnapshotSlotForRow: [Int: Int] = [:]
    
    // Auto screenshot cache
    var autoScreenshotUrls: [URL] = []
    var autoScreenshotImage: [Int: NSImage] = [:]
    var autoScreenshotDesc1: [Int: String] = [:]
    var autoScreenshotDesc2: [Int: String] = [:]
    
    // User screenshot cache
    var userScreenshotUrls: [URL] = []
    var userScreenshotImage: [Int: NSImage] = [:]
    var userScreenshotDesc1: [Int: String] = [:]
    var userScreenshotDesc2: [Int: String] = [:]
    
    var reenter = false // Remove ASAP
    
    override func windowDidLoad() {
        
        track()

        assert(reenter == false)
        reenter = true

        parent.stopSnapshotTimer()
        parent.stopScreenshotTimer()
        reloadSnapshotCache()
        reloadScreenshotCache()
        
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            
            self.snapshotCarousel.type = self.carouselType
            self.snapshotCarousel.isHidden = false
            self.snapshotCarousel.scrollToItem(at: 0, animated: false)
            
            self.screenshotCarousel.type = self.carouselType
            self.screenshotCarousel.isHidden = false
            self.screenshotCarousel.scrollToItem(at: 0, animated: false)
        }
    }
    
    func updateSnapshotView() {
        
        let index = snapshotCarousel.currentItemIndex

        if autoSnapshots {
            snapshotDescription1.stringValue = autoSnapshotDescription1[index] ?? ""
            snapshotDescription2.stringValue = autoSnapshotDescription2[index] ?? ""

        } else {
            snapshotDescription1.stringValue = userSnapshotDescription1[index] ?? ""
            snapshotDescription2.stringValue = userSnapshotDescription2[index] ?? ""
        }
    }
    
    func updateScreenshotView() {
        
        let index = screenshotCarousel.currentItemIndex
        
        if autoScreenshots {
            screenshotMiddle.image = NSImage.init(named: "starTemplate")
            screenshotMiddle.toolTip = "Move to favorites"
            screenshotDescription1.stringValue = autoScreenshotDesc1[index] ?? ""
            screenshotDescription2.stringValue = autoScreenshotDesc2[index] ?? ""
            
        } else {
            
            screenshotMiddle.image = NSImage.init(named: "trashTemplate")
            screenshotMiddle.toolTip = "Delete screenshot"
            screenshotDescription1.stringValue = userScreenshotDesc1[index] ?? ""
            screenshotDescription2.stringValue = userScreenshotDesc2[index] ?? ""
        }
    }
    
    func updateView() {
        
        snapshots ? updateSnapshotView() : updateScreenshotView()
    }
    
    func updateSnapshotCarousel(reload: Bool, moveTo item: Int = -1, animated: Bool = false) {
        
        // Update cache if requested
        if reload { reloadSnapshotCache() }
        
        // Force the carousel view to reload all items
        snapshotCarousel.reloadData()
        
        // Scroll to destination item if requested
        if item != -1 { snapshotCarousel.scrollToItem(at: item, animated: animated) }

        // Fix layout issues
        snapshotCarousel.layOutItemViews()
    }
    
    func updateScreenshotCarousel(reload: Bool, moveTo item: Int = -1, animated: Bool = false) {
        
        // Update cache if requested
        if reload { reloadScreenshotCache() }
        
        // Force the carousel view to reload all items
        screenshotCarousel.reloadData()
        
        // Scroll to destination item if requested
        if item != -1 { screenshotCarousel.scrollToItem(at: item, animated: animated) }
        
        // Fix layout issues
        screenshotCarousel.layOutItemViews()
    }
    
    func updateCarousel(reload: Bool, moveTo item: Int = -1, animated: Bool = false) {
        
        snapshots ?
            updateSnapshotCarousel(reload: reload, moveTo: item, animated: animated) :
            updateScreenshotCarousel(reload: reload, moveTo: item, animated: animated)
    }
    
    func timeInfo(date: Date?) -> String {
         
         if date == nil { return "" }
         
         let formatter = DateFormatter()
         formatter.timeZone = TimeZone.current
         formatter.dateFormat = "HH:mm:ss" // "yyyy-MM-dd HH:mm"
         
         return formatter.string(from: date!)
    }
    
    func timeInfo(time: time_t) -> String {
        
        return timeInfo(date: Date(timeIntervalSince1970: TimeInterval(time)))
    }
    
    func timeDiffInfo(seconds: Int) -> String {
        
        let secPerMin = 60
        let secPerHour = secPerMin * 60
        let secPerDay = secPerHour * 24
        let secPerWeek = secPerDay * 7
        let secPerMonth = secPerWeek * 4
        let secPerYear = secPerWeek * 52
        
        if seconds < secPerMin {
            return "\(seconds) second" + (seconds == 1 ? "" : "s") + " ago"
        }
        if seconds < secPerHour {
            let m = seconds / secPerMin
            return "\(m) minute" + (m == 1 ? "" : "s") + " ago"
        }
        if seconds < secPerDay {
            let h = seconds / secPerHour
            return "\(h) hour" + (h == 1 ? "" : "s") + " ago"
        }
        if seconds < secPerWeek {
            let d = seconds / secPerDay
            return "\(d) day" + (d == 1 ? "" : "s") + " ago"
        }
        if seconds < secPerMonth {
            let w = seconds / secPerWeek
            return "\(w) week" + (w == 1 ? "" : "s") + " ago"
        }
        if seconds < secPerYear {
            let m = seconds / secPerMonth
            return "\(m) month" + (m == 1 ? "" : "s") + " ago"
        } else {
            let y = seconds / secPerYear
            return "\(y) year" + (y == 1 ? "" : "s") + " ago"
        }
    }
    
    func timeDiffInfo(interval: TimeInterval?) -> String {
        
        return interval == nil ? "" : timeDiffInfo(seconds: Int(interval!))
    }
    
    func timeDiffInfo(date: Date?) -> String {
        
        return date == nil ? "" : timeDiffInfo(interval: date!.diff(now))
    }
    
    func timeDiffInfo(time: time_t) -> String {
        
        let date = Date(timeIntervalSince1970: TimeInterval(time))
        return timeDiffInfo(date: date)
    }
    
    func imageInfo(nr: Int, auto: Bool) -> String {
        
        var image: NSImage?
        var result: String
        
        if auto {
            image = autoScreenshotImage[nr]
            result = autoScreenshotUrls[nr].pathExtension.uppercased() + " image"
        } else {
            image = userScreenshotImage[nr]
            result = userScreenshotUrls[nr].pathExtension.uppercased() + " image"
        }
        
        if image != nil {
            result += " (\(Int(image!.size.width)) x \(Int(image!.size.height)))"
        }
        
        return result
    }
    
    func reloadSnapshotCache() {
        
        track()
        now = Date()
        
        amiga.suspend()
        
        autoSnapshotImage = [:]
        let numAutoSnapshots = amiga.numAutoSnapshots()
        for n in 0..<numAutoSnapshots {
            let takenAt = amiga.autoSnapshotTimestamp(n)
            let image = amiga.autoSnapshotImage(n)
            autoSnapshotImage[n] = image.roundCorners()
            autoSnapshotDescription1[n] = timeDiffInfo(time: takenAt) + " (" + timeInfo(time: takenAt) + ")"
            autoSnapshotDescription2[n] = ""
        }
        
        userSnapshotImage = [:]
        let numUserSnapshots = amiga.numUserSnapshots()
        for n in 0..<numUserSnapshots {
            let takenAt = amiga.userSnapshotTimestamp(n)
            let image = amiga.userSnapshotImage(n)
            userSnapshotImage[n] = image.roundCorners()
            userSnapshotDescription1[n] = timeDiffInfo(time: takenAt) + " (" + timeInfo(time: takenAt) + ")"
            userSnapshotDescription2[n] = ""
        }
        amiga.resume()
        
        // Add default images if the list is empty
        if autoSnapshotImage.count == 0 {
            autoSnapshotImage[0] = NSImage.init(named: "noise_camera")!
            autoSnapshotDescription1[0] = ""
            autoSnapshotDescription2[0] = ""
        }
        if userSnapshotImage.count == 0 {
            userSnapshotImage[0] = NSImage.init(named: "noise_camera")!
            userSnapshotDescription1[0] = ""
            userSnapshotDescription2[0] = ""
        }
    }
    
    func reloadScreenshotCache() {
        
        track()
        now = Date()
        
        autoScreenshotUrls = Screenshot.collectAutoFiles(checksum: checksum)
        userScreenshotUrls = Screenshot.collectUserFiles(checksum: checksum)
        
        track("URLS")
        track("\(autoScreenshotUrls)")
        track("\(userScreenshotUrls)")
        
        autoScreenshotImage = [:]
        for n in 0 ..< autoScreenshotUrls.count {
            
            let url = autoScreenshotUrls[n]
            let takenAt = url.modificationDate()
            
            if let image = NSImage.init(contentsOf: url) {
                autoScreenshotImage[n] = image.roundCorners()
                autoScreenshotDesc1[n] = timeDiffInfo(date: takenAt) + " (" + timeInfo(date: takenAt) + ")"
                autoScreenshotDesc2[n] = imageInfo(nr: n, auto: true)
            }
        }
        
        userScreenshotImage = [:]
        for n in 0 ..< userScreenshotUrls.count {
            
            let url = userScreenshotUrls[n]
            let takenAt = url.modificationDate()
            
            if let image = NSImage.init(contentsOf: url) {
                userScreenshotImage[n] = image.roundCorners()
                userScreenshotDesc1[n] = timeDiffInfo(date: takenAt) + " (" + timeInfo(date: takenAt) + ")"
                userScreenshotDesc2[n] = imageInfo(nr: n, auto: false)
                
            }
        }
        
        // Add default images if the list is empty
        if autoScreenshotImage.count == 0 {
            autoScreenshotImage[0] = NSImage.init(named: "noise_camera")!
            autoScreenshotDesc1[0] = ""
            autoScreenshotDesc2[0] = ""
        }
        if userScreenshotImage.count == 0 {
            userScreenshotImage[0] = NSImage.init(named: "noise_camera")!
            userScreenshotDesc1[0] = ""
            userScreenshotDesc2[0] = ""
        }
    }
        
    @IBAction func snapshotSelectorAction(_ sender: NSSegmentedControl!) {
        
        track()
        updateCarousel(reload: false)
        updateView()
    }
    
    @IBAction func screenshotSelectorAction(_ sender: NSSegmentedControl!) {
        
        track()
        updateCarousel(reload: false)
        updateView()
    }
    
    @IBAction func restoreButtonAction(_ sender: NSButton!) {
        
        track()
        
        if autoSnapshots {
            track("Restore auto snapshot")
        } else {
            track("Restore user snapshot")
        }
    }
    
    @IBAction func saveButtonAction(_ sender: NSButton!) {
        
        track()
        
        if autoSnapshots {
            track("Save auto snapshot")
        } else {
            track("Save user snapshot")
        }
    }
    
    @IBAction func deleteButtonAction(_ sender: NSButton!) {
        
        track()
        
        if autoSnapshots {
            track("Delete auto snapshot")
        } else {
            track("Delete user snapshot")
        }
        updateCarousel(reload: true)
    }
    
    @IBAction func finderAction(_ sender: NSButton!) {
        
        track()
        if let url = screenshotFolder {
            NSWorkspace.shared.open(url)
        }
    }
    
    @IBAction func leftButtonAction(_ sender: NSButton!) {
        
        let index = carousel.currentItemIndex
        track("\(index)")
        
        if index > 0 {
            Screenshot.swap(item: index, with: index - 1, in: screenshotFolder)
            updateCarousel(reload: true, moveTo: index - 1, animated: true)
        }
    }
    
    @IBAction func middleButtonAction(_ sender: NSButton!) {
        
        let index = screenshotCarousel.currentItemIndex
        track("\(index)")
        
        if autoScreenshots {
            track("Moving \(index) to user")
            Screenshot.moveToUser(item: index, checksum: checksum)
        } else {
            Screenshot.deleteUser(item: index, checksum: checksum)
        }
        updateCarousel(reload: true)
    }
    
    @IBAction func rightButtonAction(_ sender: NSButton!) {
        
        let index = carousel.currentItemIndex
        track("\(index)")
        
        if index < carousel.numberOfItems - 1 {
            Screenshot.swap(item: index, with: index + 1, in: screenshotFolder)
            updateCarousel(reload: true, moveTo: index + 1, animated: true)
        }
    }
    
    @IBAction override func cancelAction(_ sender: Any!) {
        
        track()
        parent.startScreenshotTimer()
        parent.startSnapshotTimer()
        hideSheet()
    }
}

//
// NTabView delegate
//

extension StorageDialog: NSTabViewDelegate {
    
    func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {
        
        track()
        let center = carousel.numberOfItems / 2
        updateCarousel(reload: false, moveTo: center, animated: false)
        updateView()
    }
}

//
// iCarousel data source and delegate
//

extension StorageDialog: iCarouselDataSource, iCarouselDelegate {
    
    func numberOfItems(in carousel: iCarousel) -> Int {
        
        assert(carousel == snapshotCarousel || carousel == screenshotCarousel)
        
        if carousel == snapshotCarousel {
            return autoSnapshots ? autoSnapshotImage.count : userSnapshotImage.count
        } else {
            return autoScreenshots ? autoScreenshotImage.count : userScreenshotImage.count
        }
    }
    
    func carousel(_ carousel: iCarousel, viewForItemAt index: Int, reusing view: NSView?) -> NSView {
        
        assert(carousel == snapshotCarousel || carousel == screenshotCarousel)
        
        let h = carousel.frame.height - 10
        let w = h * 4 / 3
        let itemView = NSImageView(frame: CGRect(x: 0, y: 0, width: w, height: h))
        
        if carousel == snapshotCarousel {
            
            if autoSnapshots {
                itemView.image = autoSnapshotImage[index]
            } else {
                itemView.image = userSnapshotImage[index]
            }
            
        } else {
            
            if autoScreenshots {
                itemView.image = autoScreenshotImage[index]
            } else {
                itemView.image = userScreenshotImage[index]
            }
        }
        
        return itemView
    }
    
    func carouselCurrentItemIndexDidChange(_ carousel: iCarousel) {
        
        track()
        updateView()
    }
}
