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

    @IBOutlet weak var selector: NSSegmentedControl!

    @IBOutlet weak var autoLabel: NSTextField!
    @IBOutlet weak var autoCarousel: iCarousel!
    @IBOutlet weak var autoPrev: NSButton!
    @IBOutlet weak var autoNext: NSButton!
    @IBOutlet weak var autoTrash: NSButton!
    @IBOutlet weak var autoAction1: NSButton!
    @IBOutlet weak var autoAction2: NSButton!
    @IBOutlet weak var autoNr: NSTextField!
    @IBOutlet weak var autoText1: NSTextField!
    @IBOutlet weak var autoText2: NSTextField!
    @IBOutlet weak var autoFinderLabel: NSTextField!
    @IBOutlet weak var autoFinderButton: NSButton!
    
    @IBOutlet weak var userLabel: NSTextField!
    @IBOutlet weak var userCarousel: iCarousel!
    @IBOutlet weak var userPrev: NSButton!
    @IBOutlet weak var userNext: NSButton!
    @IBOutlet weak var userTrash: NSButton!
    @IBOutlet weak var userAction1: NSButton!
    @IBOutlet weak var userAction2: NSButton!
    @IBOutlet weak var userNr: NSTextField!
    @IBOutlet weak var userText1: NSTextField!
    @IBOutlet weak var userText2: NSTextField!
    @IBOutlet weak var userFinderLabel: NSTextField!
    @IBOutlet weak var userFinderButton: NSButton!

    @IBOutlet weak var moveToUser: NSButton!

    let carouselType = iCarouselType.timeMachine //   coverFlow
    
    // Fingerprint of disk in df0
    var checksum = UInt64(0)
    
    // Computed variables
    var snapshotView: Bool { return selector.selectedSegment == 0 }
    var screenshotView: Bool { return selector.selectedSegment == 1 }
    var autoFolder: URL? { return Screenshot.autoFolder(checksum: checksum) }
    var userFolder: URL? { return Screenshot.userFolder(checksum: checksum) }
    var autoSelection: Int { return autoCarousel.currentItemIndex }
    var userSelection: Int { return userCarousel.currentItemIndex }
    var numAutoItems: Int { return autoCarousel.numberOfItems }
    var numUserItems: Int { return userCarousel.numberOfItems }
    var lastAutoItem: Int { return numAutoItems - 1 }
    var lastUserItem: Int { return numUserItems - 1 }

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
    
    override func windowDidLoad() {
        
        track()

        parent.stopSnapshotTimer()
        parent.stopScreenshotTimer()
        updateCaches()
        updateLabels()
        
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            
            track("lastAutoItem = \(self.lastAutoItem)")
            self.autoCarousel.type = self.carouselType
            self.autoCarousel.isHidden = false
            self.updateAutoCarousel(goto: self.lastAutoItem, animated: false)

            track("lastUserItem = \(self.lastUserItem)")
            self.userCarousel.type = self.carouselType
            self.userCarousel.isHidden = false
            self.updateUserCarousel(goto: self.lastUserItem, animated: false)
        }
    }
    
    func updateLabels() {
        
        let autoIndex = autoCarousel.currentItemIndex
        let userIndex = userCarousel.currentItemIndex
        
        autoNext.isEnabled = autoIndex >= 0 && autoIndex < autoCarousel.numberOfItems - 1
        autoPrev.isEnabled = autoIndex > 0
        userNext.isEnabled = userIndex >= 0 && userIndex < userCarousel.numberOfItems - 1
        userPrev.isEnabled = userIndex > 0

        if snapshotView {
            
            moveToUser.isHidden = true
            
            autoLabel.stringValue = "Automatically saved snapshots"
            autoTrash.image = NSImage.init(named: "trashTemplate")
            autoTrash.toolTip = "Delete snapshot"
            autoTrash.isEnabled = autoIndex >= 0
            autoAction1.image = NSImage.init(named: "restoreTemplate")
            autoAction1.toolTip = "Restore snapshot"
            autoAction1.isEnabled = autoIndex >= 0
            autoAction2.image = NSImage.init(named: "saveTemplate")
            autoAction2.toolTip = "Save snapshot to disk"
            autoAction2.isEnabled = autoIndex >= 0
            autoFinderButton.isHidden = true
            autoFinderLabel.isHidden = true

            if autoIndex >= 0 {
                
                let snapshot = parent.mydocument!.autoSnapshots[autoIndex]
                autoNr.stringValue = "\(autoIndex + 1) / \(numAutoItems)"
                autoNr.textColor = .labelColor
                autoText1.stringValue = timeDiffInfo(time: snapshot.timeStamp())
                autoText2.stringValue = ""

            } else {

                autoNr.stringValue = "No snapshots available"
                autoNr.textColor = .secondaryLabelColor
                autoText1.stringValue = ""
                autoText2.stringValue = ""
            }
            
            userLabel.stringValue = "Manually saved snapshots"
            userTrash.image = NSImage.init(named: "trashTemplate")
            userTrash.toolTip = "Delete snapshot"
            userTrash.isEnabled = userIndex >= 0
            userAction1.image = NSImage.init(named: "restoreTemplate")
            userAction1.toolTip = "Restore snapshot"
            userAction1.isEnabled = userIndex >= 0
            userAction2.image = NSImage.init(named: "saveTemplate")
            userAction2.toolTip = "Save snapshot to disk"
            userAction2.isEnabled = userIndex >= 0
            userFinderButton.isHidden = true
            userFinderLabel.isHidden = true
            
            if userIndex >= 0 {
                
                let snapshot = parent.mydocument!.userSnapshots[userIndex]
                userNr.stringValue = "\(userIndex + 1) / \(numUserItems)"
                userNr.textColor = .labelColor
                userText1.stringValue = timeDiffInfo(time: snapshot.timeStamp())
                userText2.stringValue = ""
                
            } else {
                
                userNr.stringValue = "No snapshots available"
                userNr.textColor = .secondaryLabelColor
                userText1.stringValue = ""
                userText2.stringValue = ""
            }

        } else {
            
            moveToUser.isHidden = false

            autoLabel.stringValue = "Automatically saved screenshots"
            autoNr.stringValue = "\(autoIndex + 1) / \(numAutoItems)"
            autoText1.stringValue = autoScreenshotDesc1[autoIndex] ?? ""
            autoText2.stringValue = autoScreenshotDesc2[autoIndex] ?? ""
            
            autoTrash.toolTip = "Delete screenshot"
            autoAction1.image = NSImage.init(named: "backTemplate")
            autoAction1.toolTip = "Change order"
            autoAction2.image = NSImage.init(named: "frontTemplate")
            autoAction2.toolTip = "Change order"

            userLabel.stringValue = "Manually saved screenshots"
            userNr.stringValue = "\(userIndex + 1) / \(numUserItems)"
            userText1.stringValue = userScreenshotDesc1[userIndex] ?? ""
            userText2.stringValue = userScreenshotDesc2[userIndex] ?? ""

            userTrash.image = NSImage.init(named: "trashTemplate")
            userTrash.toolTip = "Delete screenshot"
            userAction1.image = NSImage.init(named: "backTemplate")
            userAction1.toolTip = "Move screenshot inwards"
            userAction2.image = NSImage.init(named: "frontTemplate")
            userAction2.toolTip = "Move screenshot outwards"

            autoFinderButton.isHidden = false
            autoFinderLabel.isHidden = false
            userFinderButton.isHidden = false
            userFinderLabel.isHidden = false
        }
    }

    func updateCarousel(carousel: iCarousel, goto item: Int, animated: Bool) {
        
        carousel.reloadData()
        
        let index = min(item, carousel.numberOfItems - 1)
        if index >= 0 { carousel.scrollToItem(at: index, animated: animated) }
        
        carousel.layOutItemViews()
        updateLabels()
    }

    func updateAutoCarousel(goto item: Int = -1, animated: Bool = false) {
        
        updateCarousel(carousel: autoCarousel, goto: item, animated: animated)
    }

    func updateUserCarousel(goto item: Int = -1, animated: Bool = false) {
        
        updateCarousel(carousel: userCarousel, goto: item, animated: animated)
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

    func timeDiffInfo(url: URL) -> String {
        
        return timeDiffInfo(date: url.modificationDate())
    }

    func imageSizeInfo(image: NSImage?) -> String {

        let w = image?.size.width ?? 0
        let h = image?.size.height ?? 0
        return w != 0 && h != 0 ? " (\(w) x \(h))" : ""
    }

    func imageInfo(image: NSImage?, url: URL?) -> String {

        if image == nil || url == nil { return "" }
        
        let suffix = url!.pathExtension.uppercased()
        let width  = Int(image!.size.width)
        let height = Int(image!.size.height)
           
        return "\(suffix) image (\(width) x \(height))"
    }

    func autoImageInfo(_ nr: Int) -> String {

        return imageInfo(image: autoScreenshotImage[nr], url: autoScreenshotUrls[nr])
    }

    func userImageInfo(_ nr: Int) -> String {

        return imageInfo(image: userScreenshotImage[nr], url: userScreenshotUrls[nr])
    }

    func updateAutoScreenshotCache() {
        
        track()
        now = Date()
        
        autoScreenshotImage = [:]
        autoScreenshotDesc1 = [:]
        autoScreenshotDesc2 = [:]
        autoScreenshotUrls = Screenshot.collectAutoFiles(checksum: checksum)
        
        for n in 0 ..< autoScreenshotUrls.count {
            
            let url = autoScreenshotUrls[n]
            
            if let image = NSImage.init(contentsOf: url) {
                autoScreenshotImage[n] = image.roundCorners()
                autoScreenshotDesc1[n] = "Taken " + timeDiffInfo(url: url)
                autoScreenshotDesc2[n] = autoImageInfo(n)
            }
        }
        
        if autoScreenshotUrls.count == 0 {
            autoScreenshotImage[0] = NSImage.init(named: "noise_camera")!
        }
    }
    
    func updateUserScreenshotCache() {
        
        track()
        now = Date()
        
        userScreenshotImage = [:]
        userScreenshotDesc1 = [:]
        userScreenshotDesc2 = [:]
        userScreenshotUrls = Screenshot.collectUserFiles(checksum: checksum)
        
        for n in 0 ..< userScreenshotUrls.count {
            
            let url = userScreenshotUrls[n]
            
            if let image = NSImage.init(contentsOf: url) {
                userScreenshotImage[n] = image.roundCorners()
                userScreenshotDesc1[n] = "Taken " + timeDiffInfo(url: url)
                userScreenshotDesc2[n] = userImageInfo(n)
            }
        }
    
        if userScreenshotUrls.count == 0 {
            userScreenshotImage[0] = NSImage.init(named: "noise_camera")!
        }
    }
    
    func updateScreenshotCaches() {
        
        updateAutoScreenshotCache()
        updateUserScreenshotCache()
    }
        
    func updateCaches() {
        
        updateScreenshotCaches()
    }
    
    @IBAction func selectorAction(_ sender: NSSegmentedControl!) {
                
        updateAutoCarousel(goto: Int.max, animated: false)
        updateUserCarousel(goto: Int.max, animated: false)
    }
    
    func nextAction(carousel: iCarousel) {
        
        track("numberOfItems = \(carousel.numberOfItems)")
        
        let index = carousel.currentItemIndex
        
        if index < carousel.numberOfItems - 1 {
            carousel.scrollToItem(at: index + 1, animated: true)
        }
    }

    func prevAction(carousel: iCarousel) {
        
        track("numberOfItems = \(carousel.numberOfItems)")
        
        let index = carousel.currentItemIndex
        
        if index > 0 {
            carousel.scrollToItem(at: index - 1, animated: true)
        }
    }

    @IBAction func autoNextAction(_ sender: NSButton!) {
        
        track()
        nextAction(carousel: autoCarousel)
    }
    
    @IBAction func autoPrevAction(_ sender: NSButton!) {
        
        track()
        prevAction(carousel: autoCarousel)
    }

    @IBAction func userNextAction(_ sender: NSButton!) {
        
        track()
        nextAction(carousel: userCarousel)
    }

    @IBAction func userPrevAction(_ sender: NSButton!) {
        
        track()
        prevAction(carousel: userCarousel)
    }

    @IBAction func autoTrashAction(_ sender: NSButton!) {
        
        let index = autoCarousel.currentItemIndex
        track("index = \(index)")

        if snapshotView {
            parent.mydocument!.autoSnapshots.remove(at: index)
            updateAutoCarousel()
        } else {
            Screenshot.deleteAuto(item: index, checksum: checksum)
            updateAutoScreenshotCache()
        }
    }
    
    @IBAction func userTrashAction(_ sender: NSButton!) {
        
        let index = userCarousel.currentItemIndex
        track("index = \(index)")
        
        if snapshotView {
            parent.mydocument!.userSnapshots.remove(at: index)
        } else {
            Screenshot.deleteUser(item: index, checksum: checksum)
            updateUserScreenshotCache()
        }
        updateUserCarousel()
    }
    
    @IBAction func auto1Action(_ sender: NSButton!) {
        
        track()
        let index = autoCarousel.currentItemIndex
        
        if snapshotView {
            
            if !parent.restoreAutoSnapshot(item: index) { NSSound.beep() }
            
        } else if index > 0 {
            
            Screenshot.swap(item: index, with: index - 1, in: autoFolder)
            updateAutoScreenshotCache()
            updateAutoCarousel(goto: index - 1, animated: true)
        }
    }
    
    @IBAction func user1Action(_ sender: NSButton!) {
        
        track()
        let index = userCarousel.currentItemIndex
        
        if snapshotView {
            
            if !parent.restoreUserSnapshot(item: index) { NSSound.beep() }
            
        } else if index > 0 {
            
            Screenshot.swap(item: index, with: index - 1, in: userFolder)
            updateUserScreenshotCache()
            updateUserCarousel(goto: index - 1, animated: true)
        }
    }
    
    @IBAction func auto2Action(_ sender: NSButton!) {
        
        track()
        let index = autoCarousel.currentItemIndex
        
        if snapshotView {
            
            saveSnapshotAs(item: index, auto: true)

        } else if index < autoCarousel.numberOfItems - 1 {
            
            Screenshot.swap(item: index, with: index + 1, in: autoFolder)
            updateAutoScreenshotCache()
            updateAutoCarousel(goto: index + 1, animated: true)
        }
    }
    
    @IBAction func user2Action(_ sender: NSButton!) {
        
        track()
        let index = userCarousel.currentItemIndex
        
        if snapshotView {
            
            saveSnapshotAs(item: index, auto: false)
            
        } else if index < userCarousel.numberOfItems - 1 {
            
            Screenshot.swap(item: index, with: index + 1, in: userFolder)
            updateUserScreenshotCache()
            updateUserCarousel(goto: index + 1, animated: true)
        }
    }

    @IBAction func moveToUserAction(_ sender: NSButton!) {
        
        track()
        assert(screenshotView)
        
        let index = autoCarousel.currentItemIndex
        Screenshot.moveToUser(item: index, checksum: checksum)
        
        updateAutoScreenshotCache()
        updateUserScreenshotCache()
        updateAutoCarousel(goto: index - 1, animated: true)
        updateUserCarousel(goto: Int.max, animated: true)
    }

    @IBAction func autoFinderAction(_ sender: NSButton!) {
        
        track()
        assert(screenshotView)
        if let url = autoFolder {
            NSWorkspace.shared.open(url)
        }
    }
    
    @IBAction func userFinderAction(_ sender: NSButton!) {
        
        track()
        assert(screenshotView)
        if let url = userFolder {
            NSWorkspace.shared.open(url)
        }
    }
    
    @IBAction override func cancelAction(_ sender: Any!) {
        
        track()
        parent.startSnapshotTimer()
        parent.startScreenshotTimer()
        hideSheet()
    }
    
    func saveSnapshotAs(item: Int, auto: Bool) {
         
         track()
         
         let panel = NSSavePanel()
         panel.prompt = "Save As"
         panel.allowedFileTypes = ["vAmiga"]
         
         panel.beginSheetModal(for: window!, completionHandler: { result in
             if result == .OK {
                 if let url = panel.url {
                     
                     let data = auto ?
                        self.parent.mydocument!.autoSnapshots[item].data() :
                        self.parent.mydocument!.userSnapshots[item].data()
                    try? data?.write(to: url)
                 }
             }
         })
     }
}

//
// iCarousel data source and delegate
//

extension StorageDialog: iCarouselDataSource, iCarouselDelegate {
    
    func numberOfItems(in carousel: iCarousel) -> Int {
        
        var numItems: Int
        
        if carousel == autoCarousel {
            numItems = snapshotView ?
                (parent.mydocument?.autoSnapshots.count ?? 0) :
                autoScreenshotImage.count
        } else {
            numItems = snapshotView ?
                (parent.mydocument?.userSnapshots.count ?? 0) :
                userScreenshotImage.count
        }
        
        return numItems
    }
    
    func carousel(_ carousel: iCarousel, viewForItemAt index: Int, reusing view: NSView?) -> NSView {
        
        let h = carousel.frame.height - 10
        let w = h * 4 / 3
        let itemView = NSImageView(frame: CGRect(x: 0, y: 0, width: w, height: h))
        
        if snapshotView {
            
            itemView.image = (carousel == autoCarousel) ?
                parent.mydocument!.autoSnapshots[index].previewImage() :
                parent.mydocument!.userSnapshots[index].previewImage()

            itemView.wantsLayer = true
            itemView.layer?.cornerRadius = 10.0
            itemView.layer?.masksToBounds = true
            
        } else {
            
            if carousel == autoCarousel {
                itemView.image = autoScreenshotImage[index]
            } else {
                itemView.image = userScreenshotImage[index]
            }
        }
        
        return itemView
    }
    
    func carouselCurrentItemIndexDidChange(_ carousel: iCarousel) {
        
        updateLabels()
    }
}
