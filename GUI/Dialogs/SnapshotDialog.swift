// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class SnapshotDialog: DialogController {
    
    var now: Date!

    @IBOutlet weak var autoSeperator: NSBox!
    @IBOutlet weak var autoCarousel: iCarousel!
    @IBOutlet weak var autoPrev: NSButton!
    @IBOutlet weak var autoNext: NSButton!
    @IBOutlet weak var autoTrash: NSButton!
    @IBOutlet weak var autoAction1: NSButton!
    @IBOutlet weak var autoAction2: NSButton!
    @IBOutlet weak var autoNr: NSTextField!
    @IBOutlet weak var autoText1: NSTextField!
    @IBOutlet weak var autoText2: NSTextField!
    
    @IBOutlet weak var userSeperator: NSBox!
    @IBOutlet weak var userCarousel: iCarousel!
    @IBOutlet weak var userPrev: NSButton!
    @IBOutlet weak var userNext: NSButton!
    @IBOutlet weak var userTrash: NSButton!
    @IBOutlet weak var userAction1: NSButton!
    @IBOutlet weak var userAction2: NSButton!
    @IBOutlet weak var userNr: NSTextField!
    @IBOutlet weak var userText1: NSTextField!
    @IBOutlet weak var userText2: NSTextField!
 
    let carouselType = iCarouselType.timeMachine //   coverFlow
    
    // Fingerprint of disk in df0
    var checksum = UInt64(0)
    
    // Computed variables
    var myDocument: MyDocument { return parent.mydocument! }
    var autoIndex: Int { return autoCarousel.currentItemIndex }
    var userIndex: Int { return userCarousel.currentItemIndex }
    var autoSelection: Int { return autoCarousel.currentItemIndex }
    var userSelection: Int { return userCarousel.currentItemIndex }
    var numAutoItems: Int { return autoCarousel.numberOfItems }
    var numUserItems: Int { return userCarousel.numberOfItems }
    var lastAutoItem: Int { return numAutoItems - 1 }
    var lastUserItem: Int { return numUserItems - 1 }
    
    override func windowWillLoad() {
   
        track()
    }
    
    override func sheetDidShow() {
  
        track()
        
        now = Date()
        
        parent.stopSnapshotTimer()
        parent.stopScreenshotTimer()
        updateAutoLabels()
        updateUserLabels()
        
        self.autoCarousel.type = self.carouselType
        self.autoCarousel.isHidden = false
        self.updateAutoCarousel(goto: self.lastAutoItem, animated: false)
        
        self.userCarousel.type = self.carouselType
        self.userCarousel.isHidden = false
        self.updateUserCarousel(goto: self.lastUserItem, animated: false)
    }
    
    func updateAutoLabels() {
                
        autoNext.isEnabled = autoIndex >= 0 && autoIndex < lastAutoItem
        autoPrev.isEnabled = autoIndex > 0

        updateAutoSnapshotLabels()
        
        let autoItems: [NSView] = [
            autoSeperator,
            autoNext,
            autoPrev,
            autoAction1,
            autoAction2,
            autoTrash,
            autoText1,
            autoText2
        ]
        for item in autoItems { item.isHidden = (numAutoItems == 0) }
        autoNr.isHidden = false
    }
    
    func updateUserLabels() {
        
        userNext.isEnabled = userIndex >= 0 && userIndex < lastUserItem
        userPrev.isEnabled = userIndex > 0
        
        updateUserSnapshotLabels()

        let userItems: [NSView] = [
            userSeperator,
            userNext,
            userPrev,
            userAction1,
            userAction2,
            userTrash,
            userText1,
            userText2
        ]
        for item in userItems { item.isHidden = (numUserItems == 0) }
        userNr.isHidden = false
    }
    
    func updateAutoSnapshotLabels() {
                
        autoTrash.isEnabled = autoIndex >= 0
        autoAction1.isEnabled = autoIndex >= 0
        autoAction2.isEnabled = autoIndex >= 0
        
        if let snapshot = myDocument.autoSnapshots.element(at: autoIndex) {
            
            autoNr.stringValue = "\(autoIndex + 1) / \(numAutoItems)"
            autoNr.textColor = .labelColor
            let takenAt = snapshot.timeStamp()
            autoText1.stringValue = timeDiffInfo(time: takenAt)
            autoText2.stringValue = timeInfo(time: takenAt)
            
        } else {
            
            autoNr.stringValue = "No snapshots available"
            autoNr.textColor = .secondaryLabelColor
            autoText1.stringValue = ""
            autoText2.stringValue = ""
        }
    }
    
    func updateUserSnapshotLabels() {
        
        userTrash.isEnabled = userIndex >= 0
        userAction1.isEnabled = userIndex >= 0
        userAction2.isEnabled = userIndex >= 0
        
        if let snapshot = myDocument.userSnapshots.element(at: userIndex) {
            
            userNr.stringValue = "\(userIndex + 1) / \(numUserItems)"
            userNr.textColor = .labelColor
            let takenAt = snapshot.timeStamp()
            userText1.stringValue = timeDiffInfo(time: takenAt)
            userText2.stringValue = timeInfo(time: takenAt)
            
        } else {
            
            userNr.stringValue = "No snapshots available"
            userNr.textColor = .secondaryLabelColor
            userText1.stringValue = ""
            userText2.stringValue = ""
        }
    }
    
    func updateCarousel(carousel: iCarousel, goto item: Int, animated: Bool) {
        
        carousel.reloadData()
        
        let index = min(item, carousel.numberOfItems - 1)
        if index >= 0 { carousel.scrollToItem(at: index, animated: animated) }
        
        track("index = \(index) num = \(carousel.numberOfItems)")
        carousel.layOutItemViews()
    }

    func updateAutoCarousel(goto item: Int = -1, animated: Bool = false) {
        
        updateCarousel(carousel: autoCarousel, goto: item, animated: animated)
        updateAutoLabels()
    }

    func updateUserCarousel(goto item: Int = -1, animated: Bool = false) {
        
        updateCarousel(carousel: userCarousel, goto: item, animated: animated)
        updateUserLabels()
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
        
        if seconds == 0 {
            return "Now"
        }
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
        myDocument.autoSnapshots.remove(at: index)
        updateAutoCarousel()
    }
    
    @IBAction func userTrashAction(_ sender: NSButton!) {
        
        let index = userCarousel.currentItemIndex
        myDocument.userSnapshots.remove(at: index)
        updateUserCarousel()
    }
    
    @IBAction func auto1Action(_ sender: NSButton!) {
        
        track()
        let index = autoCarousel.currentItemIndex
        if !parent.restoreAutoSnapshot(item: index) {
            NSSound.beep()
        }
    }
    
    @IBAction func user1Action(_ sender: NSButton!) {
        
        track()
        let index = userCarousel.currentItemIndex
        if !parent.restoreUserSnapshot(item: index) {
            NSSound.beep()
        }
    }
    
    @IBAction func auto2Action(_ sender: NSButton!) {
        
        track()
        let index = autoCarousel.currentItemIndex
        saveSnapshotAs(item: index, auto: true)
    }
    
    @IBAction func user2Action(_ sender: NSButton!) {
        
        track()
        let index = userCarousel.currentItemIndex
        saveSnapshotAs(item: index, auto: false)
    }

    @IBAction func moveToUserAction(_ sender: NSButton!) {
        
        track()
        
        let index = autoCarousel.currentItemIndex
        if let screenshot = myDocument.autoScreenshots.element(at: index) {
            myDocument.userScreenshots.append(screenshot)
            myDocument.autoScreenshots.remove(at: index)
        }
        updateAutoCarousel(goto: index - 1, animated: true)
        updateUserCarousel(goto: Int.max, animated: true)
    }

    @IBAction override func cancelAction(_ sender: Any!) {
        
        track()
                        
        hideSheet()

        parent.startSnapshotTimer()
        parent.startScreenshotTimer()
        
        // Hide some controls
        let items: [NSView] = [
            
            autoSeperator,
            autoNr,
            autoNext,
            autoPrev,
            autoAction1,
            autoAction2,
            autoTrash,
            autoText1,
            autoText2,
            
            userSeperator,
            userNr,
            userNext,
            userPrev,
            userAction1,
            userAction2,
            userTrash,
            userText1,
            userText2
        ]
        
        for item in items { item.isHidden = true }

        try? myDocument.persistScreenshots()
    }
    
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
}

//
// iCarousel data source and delegate
//

extension SnapshotDialog: iCarouselDataSource, iCarouselDelegate {
    
    func numberOfItems(in carousel: iCarousel) -> Int {
        
        var numItems: Int
        
        if carousel == autoCarousel {
            numItems = myDocument.autoSnapshots.count
        } else {
            numItems = myDocument.userSnapshots.count
        }
        
        return numItems
    }
    
    func carousel(_ carousel: iCarousel, viewForItemAt index: Int, reusing view: NSView?) -> NSView {
        
        let h = carousel.frame.height - 10
        let w = h * 4 / 3
        let itemView = NSImageView(frame: CGRect(x: 0, y: 0, width: w, height: h))
        
        itemView.image = (carousel == autoCarousel) ?
            myDocument.autoSnapshots.element(at: index)?.previewImage()?.roundCorners() :
            myDocument.userSnapshots.element(at: index)?.previewImage()?.roundCorners()
        
        /*
        itemView.wantsLayer = true
        itemView.layer?.cornerRadius = 10.0
        itemView.layer?.masksToBounds = true
        */
        
        return itemView
    }
    
    func carouselCurrentItemIndexDidChange(_ carousel: iCarousel) {
        
        carousel == autoCarousel ? updateAutoLabels() : updateUserLabels()
    }
}
