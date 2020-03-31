// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class SnapshotTableCellView: NSTableCellView {
    
    @IBOutlet weak var preview: NSImageView!
    @IBOutlet weak var text1: NSTextField!
    @IBOutlet weak var text2: NSTextField!
    @IBOutlet weak var text3: NSTextField!
    @IBOutlet weak var button1: NSButton!
    @IBOutlet weak var button2: NSButton!
    @IBOutlet weak var button3: NSButton!
}

class SnapshotDialog: DialogController {
    
    var now: Date = Date()

    // Outlets
    @IBOutlet weak var autoTableHeadline: NSTextField!
    @IBOutlet weak var userTableHeadline: NSTextField!
    @IBOutlet weak var autoTableView: NSTableView!
    @IBOutlet weak var userTableView: NSTableView!
    @IBOutlet weak var selector: NSSegmentedControl!
    
    // Auto snapshot cache
    var numAutoSnapshots = -1
    var autoSnapshotImage: [Int: NSImage] = [:]
    var autoSnapshotText1: [Int: String] = [:]
    var autoSnapshotText2: [Int: String] = [:]
    var autoSnapshotText3: [Int: String] = [:]
    var autoSnapshotSlotForRow: [Int: Int] = [:]
    
    // User snapshot cache
    var numUserSnapshots = -1
    var userSnapshotImage: [Int: NSImage] = [:]
    var userSnapshotText1: [Int: String] = [:]
    var userSnapshotText2: [Int: String] = [:]
    var userSnapshotText3: [Int: String] = [:]
    var userSnapshotSlotForRow: [Int: Int] = [:]
    
    // Auto screenshot cache
    var numAutoScreenshots = -1
    var autoScreenshotUrls: [URL] = []
    var autoScreenshotImage: [Int: NSImage] = [:]
    var autoScreenshotText1: [Int: String] = [:]
    var autoScreenshotText2: [Int: String] = [:]
    var autoScreenshotText3: [Int: String] = [:]

    // User screenshot cache
    var numUserScreenshots = -1
    var userScreenshotUrls: [URL] = []
    var userScreenshotImage: [Int: NSImage] = [:]
    var userScreenshotText1: [Int: String] = [:]
    var userScreenshotText2: [Int: String] = [:]
    var userScreenshotText3: [Int: String] = [:]

    // Fingerprint of disk in df0
    var checksum = UInt64(0)
    
    var snapshots: Bool { return selector.selectedSegment == 0}
    
    override public func awakeFromNib() {
        
        track("awakeFromNib")
        
        // Disable auto snapshot saving while dialog is open (DEPRECATED)
        amiga.suspendAutoSnapshots()
        
        if numAutoSnapshots != -1 { return }
        
        // Start with the snapshot view
        selector.selectedSegment = 0
        
        // Fill caches
        reloadSnapshotCache()
        reloadScreenshotCache()
        
        // Update table views
        userTableView.reloadData()
        autoTableView.reloadData()
        
        update()
    }
    
    func update() {
    
        if snapshots {
            autoTableHeadline.stringValue = "Auto-taken snapshots"
            userTableHeadline.stringValue = "User-taken snapshots"
        } else {
            autoTableHeadline.stringValue = "Auto-taken screenshots"
            userTableHeadline.stringValue = "User-taken screenshots"
        }
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
            return "\(seconds / 60):\(seconds % 60) minutes ago"
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
        
        var image: NSImage
        var type: String
        
        if auto {
            image = autoScreenshotImage[nr]!
            type = autoScreenshotUrls[nr].pathExtension
        } else {
            image = userScreenshotImage[nr]!
            type = userScreenshotUrls[nr].pathExtension
        }

        track("\(type)")
        let w = Int(image.size.width)
        let h = Int(image.size.height)
        
        return "\(type.uppercased()) (\(w) x \(h))"
    }
    
    func reloadSnapshotCache() {

        track()
        now = Date()

        amiga.suspend()

        numAutoSnapshots = amiga.numAutoSnapshots()
        for n in 0..<numAutoSnapshots {
            let takenAt = amiga.autoSnapshotTimestamp(n)
            let image = amiga.autoSnapshotImage(n)
            autoSnapshotImage[n] = image.roundCorners()
            autoSnapshotText1[n] = timeDiffInfo(time: takenAt)
            autoSnapshotText2[n] = timeInfo(time: takenAt)
            autoSnapshotText3[n] = ""
        }

        numUserSnapshots = amiga.numUserSnapshots()
        for n in 0..<numUserSnapshots {
            let takenAt = amiga.userSnapshotTimestamp(n)
            let image = amiga.userSnapshotImage(n)
            userSnapshotImage[n] = image.roundCorners()
            userSnapshotText1[n] = timeDiffInfo(time: takenAt)
            userSnapshotText2[n] = timeInfo(time: takenAt)
            userSnapshotText3[n] = ""
        }
        amiga.resume()
        
        autoTableView.reloadData()
        userTableView.reloadData()
    }

    func reloadScreenshotCache() {

        track()
        now = Date()

        autoScreenshotUrls = Screenshot.collectAutoFiles(checksum: checksum)
        userScreenshotUrls = Screenshot.collectUserFiles(checksum: checksum)
        
        track("URLS")
        track("\(autoScreenshotUrls)")
        track("\(userScreenshotUrls)")

        numAutoScreenshots = autoScreenshotUrls.count
        for n in 0..<numAutoScreenshots {
 
            let url = autoScreenshotUrls[n]
            let takenAt = url.modificationDate()
            
            if let image = NSImage.init(contentsOf: url) {
                autoScreenshotImage[n] = image.roundCorners()
                autoScreenshotText1[n] = timeDiffInfo(date: takenAt)
                autoScreenshotText2[n] = timeInfo(date: takenAt)
                autoScreenshotText3[n] = imageInfo(nr: n, auto: true)
            }
        }
        
        numUserScreenshots = userScreenshotUrls.count
        for n in 0..<numUserScreenshots {
            
            let url = userScreenshotUrls[n]
            let takenAt = url.modificationDate()

            if let image = NSImage.init(contentsOf: url) {
                userScreenshotImage[n] = image.roundCorners()
                userScreenshotText1[n] = timeDiffInfo(date: takenAt)
                userScreenshotText2[n] = timeInfo(date: takenAt)
                userScreenshotText3[n] = imageInfo(nr: n, auto: false)

            }
        }
    }

    @IBAction func selectAction(_ sender: NSSegmentedControl!) {
        
        snapshots ? reloadSnapshotCache() : reloadScreenshotCache()
        autoTableView.reloadData()
        userTableView.reloadData()
        update()
    }

    @IBAction func autoButton1Action(_ sender: NSButton!) {
        
        if snapshots {
            
            track("\(sender.tag)")
            reloadSnapshotCache()
            
        } else {
            
            track("\(sender.tag)")
            reloadScreenshotCache()
        }
    }

    @IBAction func userButton1Action(_ sender: NSButton!) {
        
        if snapshots {
            
            track("\(sender.tag)")
            reloadSnapshotCache()
            
        } else {
            
            track("\(sender.tag)")
            reloadScreenshotCache()
        }
    }

    @IBAction func autoButton2Action(_ sender: NSButton!) {
        
        if snapshots {
            saveSnapshotAs(item: sender.tag, auto: true)
        } else {
            if let url = Screenshot.autoFolder(checksum: checksum) {
                NSWorkspace.shared.open(url)
            }
        }
    }
    
    @IBAction func userButton2Action(_ sender: NSButton!) {
        
        if snapshots {
            saveSnapshotAs(item: sender.tag, auto: false)
        } else {
            if let url = Screenshot.userFolder(checksum: checksum) {
                NSWorkspace.shared.open(url)
            }
        }
    }

    @IBAction func autoButton3Action(_ sender: NSButton!) {
        
        if snapshots {

             track("\(sender.tag)")
             amiga.deleteAutoSnapshot(sender.tag)
             reloadSnapshotCache()
         }
    }

    @IBAction func userButton3Action(_ sender: NSButton!) {
        
        if snapshots {

             track("\(sender.tag)")
             amiga.deleteUserSnapshot(sender.tag)
             reloadSnapshotCache()
         }
    }

    @IBAction override func cancelAction(_ sender: Any!) {
        
        track()
        amiga.resumeAutoSnapshots()
        hideSheet()
    }
    
    @IBAction func autoDoubleClick(_ sender: NSTableView!) {

        amiga.restoreAutoSnapshot(sender.selectedRow)
        cancelAction(self)
    }
    
    @IBAction func userDoubleClick(_ sender: NSTableView!) {

        amiga.restoreUserSnapshot(sender.selectedRow)
        cancelAction(self)
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
                        self.amiga.autoSnapshotData(item) :
                        self.amiga.userSnapshotData(item)
                    try? data?.write(to: url)
                }
            }
        })
    }
}

//
// NSTableViewDataSource, NSTableViewDelegate
//

extension SnapshotDialog: NSTableViewDataSource, NSTableViewDelegate {
    
    func numberOfRows(in tableView: NSTableView) -> Int {

        if snapshots {
            if tableView == autoTableView { return numAutoSnapshots }
            if tableView == userTableView { return numUserSnapshots }
        } else {
            if tableView == autoTableView { return numAutoScreenshots }
            if tableView == userTableView { return numUserScreenshots }
        }
        
        fatalError()
    }
    
    func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        
        if snapshots {
            track("snapshots: \(numAutoSnapshots) \(numUserSnapshots)")
            return snapshotTableView(tableView, row: row)
        } else {
            track("screenshots: \(numAutoScreenshots) \(numUserScreenshots)")
            return screenshotTableView(tableView, row: row)
        }
    }
    
    func createView(_ tableView: NSTableView) -> SnapshotTableCellView? {

        assert(tableView == autoTableView || tableView == userTableView)

        let id = NSUserInterfaceItemIdentifier(rawValue: "defaultRow")
        let view = tableView.makeView(withIdentifier: id, owner: self)
        return view as? SnapshotTableCellView
    }
    
    func snapshotTableView(_ tableView: NSTableView, row: Int) -> NSView? {
        
        guard let view = createView(tableView) else { return nil }
        
        view.button2.image = NSImage.init(named: "saveTemplate")
        view.button1.toolTip = "Restore snapshot"
        view.button2.toolTip = "Save snapshot to disk"
        view.button3.toolTip = "Discard snapshot"
        view.button1.tag = row
        view.button2.tag = row
        view.button3.tag = row
        
        if tableView == autoTableView {
            
            view.preview.image = autoSnapshotImage[row]
            view.text1.stringValue = autoSnapshotText1[row]!
            view.text2.stringValue = autoSnapshotText2[row]!
            view.text3.stringValue = autoSnapshotText3[row]!
            
        } else {
            
            view.preview.image = userSnapshotImage[row]
            view.text1.stringValue = userSnapshotText1[row]!
            view.text2.stringValue = userSnapshotText2[row]!
            view.text3.stringValue = userSnapshotText3[row]!
        }
        
        return view
    }
    
    func screenshotTableView(_ tableView: NSTableView, row: Int) -> NSView? {
        
        guard let view = createView(tableView) else { return nil }
        
        view.button1.isHidden = true
        view.button2.image = NSImage.init(named: "folderTemplate")
        view.button2.toolTip = nil
        view.button2.toolTip = "Open in Finder"
        view.button3.toolTip = "Discard screenshot"
        view.button1.tag = row
        view.button2.tag = row
        view.button3.tag = row

        if tableView == autoTableView {
            
            view.preview.image = autoScreenshotImage[row]
            view.text1.stringValue = autoScreenshotText1[row]!
            view.text2.stringValue = autoScreenshotText2[row]!
            view.text3.stringValue = autoScreenshotText3[row]!
            
        } else {
            track("screenshotTableView \(row)")
            view.preview.image = userScreenshotImage[row]
            view.text1.stringValue = userScreenshotText1[row]!
            view.text2.stringValue = userScreenshotText2[row]!
            view.text3.stringValue = userScreenshotText3[row]!
        }
        
        return view
    }
}

//
// Drag and drop
//

extension SnapshotDialog {
    
    func tableView(_ tableView: NSTableView,
                   writeRowsWith rowIndexes: IndexSet,
                   to pboard: NSPasteboard) -> Bool {
                
        // Get index of dragged item
        guard let index = rowIndexes.first else {
            track("Cannot get table index for drag and drop")
            return false
        }
        
        // Get snapshot data
        var data: Data
        if tableView == autoTableView {
            data = amiga.autoSnapshotData(index)
        } else {
            assert(tableView == userTableView)
            data = amiga.userSnapshotData(index)
        }
        
        let pboardType = NSPasteboard.PasteboardType.fileContents
        pboard.declareTypes([pboardType], owner: self)
        let fileWrapper = FileWrapper.init(regularFileWithContents: data)
        fileWrapper.preferredFilename = "Snapshot.vam"
        pboard.write(fileWrapper)

        return true
    }
}
