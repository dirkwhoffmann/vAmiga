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
    @IBOutlet weak var autoTableView: NSTableView!
    @IBOutlet weak var userTableView: NSTableView!
    @IBOutlet weak var selector: NSSegmentedControl!
    
    // Auto snapshot cache
    var numAutoSnapshots = -1
    var autoSnapshotImage: [Int: NSImage] = [:]
    var autoSnapshotTimeStamp: [Int: String] = [:]
    var autoSnapshotTimeDiff: [Int: String] = [:]
    var autoSnapshotSlotForRow: [Int: Int] = [:]
    
    // User snapshot cache
    var numUserSnapshots = -1
    var userSnapshotImage: [Int: NSImage] = [:]
    var userSnapshotTimeStamp: [Int: String] = [:]
    var userSnapshotTimeDiff: [Int: String] = [:]
    var userSnapshotSlotForRow: [Int: Int] = [:]
    
    // Auto screenshot cache
    var numAutoScreenshots = -1
    var autoScreenshotImage: [Int: NSImage] = [:]
    var autoScreenshotTimeStamp: [Int: String] = [:]
    var autoScreenshotTimeDiff: [Int: String] = [:]

    // User screenshot cache
    var numUserScreenshots = -1
    var userScreenshotImage: [Int: NSImage] = [:]
    var userScreenshotTimeStamp: [Int: String] = [:]
    var userScreenshotTimeDiff: [Int: String] = [:]

    // Fingerprint of disk in df0
    var checksum = UInt64(0)
    
    var snapshots: Bool { return selector.selectedSegment == 0}
    
    override public func awakeFromNib() {
        
        track("awakeFromNib")
        
        if numAutoSnapshots == -1 {
            
            // Disable auto snapshot saving while dialog is open
            amiga.suspendAutoSnapshots()
            
            // Fill caches
            reloadSnapshotCache()
            reloadScreenshotCache()
        }
        
        track("awakeFromNib (exit)")
    }
    
    func timeInfo(timeStamp: TimeInterval) -> String {
        
        let formatter = DateFormatter()
        formatter.timeZone = TimeZone.current
        formatter.dateFormat = "HH:mm:ss" // "yyyy-MM-dd HH:mm"
        
        return formatter.string(from: Date(timeIntervalSince1970: timeStamp))
    }
    
    func timeDiffInfo(timeStamp: TimeInterval) -> String {
        
        let secPerMin = 60
        let secPerHour = secPerMin * 60
        let secPerDay = secPerHour * 24
        let secPerWeek = secPerDay * 7
        let secPerMonth = secPerWeek * 4
        let secPerYear = secPerWeek * 52

        let diff = Int(round(now.timeIntervalSince1970 - Double(timeStamp)))
                
        if diff < secPerMin {
            return "\(diff) second" + (diff == 1 ? "" : "s") + " ago"
        }
        if diff < secPerHour {
            return "\(diff / 60):\(diff % 60) minutes ago"
        }
        if diff < secPerDay {
            let h = diff / secPerHour
            return "\(h) hour" + (h == 1 ? "" : "s") + " ago"
        }
        if diff < secPerWeek {
            let d = diff / secPerDay
            return "\(d) day" + (d == 1 ? "" : "s") + " ago"
        }
        if diff < secPerMonth {
            let w = diff / secPerWeek
            return "\(w) week" + (w == 1 ? "" : "s") + " ago"
        }
        if diff < secPerYear {
            let m = diff / secPerMonth
            return "\(m) month" + (m == 1 ? "" : "s") + " ago"
        } else {
            let y = diff / secPerYear
            return "\(y) year" + (y == 1 ? "" : "s") + " ago"
        }
    }
    
    func reloadSnapshotCache() {

        track()

        amiga.suspend()
        
        numAutoSnapshots = amiga.numAutoSnapshots()
        for n in 0..<numAutoSnapshots {
            let takenAt = TimeInterval(amiga.autoSnapshotTimestamp(n))
            autoSnapshotImage[n] = amiga.autoSnapshotImage(n)
            autoSnapshotTimeStamp[n] = timeInfo(timeStamp: takenAt)
            autoSnapshotTimeDiff[n] = timeDiffInfo(timeStamp: takenAt)
        }

        numUserSnapshots = amiga.numUserSnapshots()
        for n in 0..<numUserSnapshots {
            let takenAt = TimeInterval(amiga.userSnapshotTimestamp(n))
            userSnapshotImage[n] = amiga.userSnapshotImage(n)
            userSnapshotTimeStamp[n] = timeInfo(timeStamp: takenAt)
            userSnapshotTimeDiff[n] = timeDiffInfo(timeStamp: takenAt)
        }
        amiga.resume()
        
        autoTableView.reloadData()
        userTableView.reloadData()
    }

    func reloadScreenshotCache() {

        track("checksum = \(checksum)")
        
        let autoUrls = Screenshot.collectAutoFiles(checksum: checksum)
        let userUrls = Screenshot.collectUserFiles(checksum: checksum)

        track("Found urls \(userUrls)")
        
        numAutoScreenshots = autoUrls.count
        for n in 0..<numAutoScreenshots {
            
            if let image = NSImage.init(contentsOf: autoUrls[n]) {
                autoScreenshotImage[n] = image.roundCorners(withRadius: 20.0)
                autoScreenshotTimeStamp[n] = "TODO"
                autoScreenshotTimeDiff[n] = "todo"
            }
        }
        
        numUserScreenshots = userUrls.count
        for n in 0..<numUserScreenshots {
            
            if let image = NSImage.init(contentsOf: userUrls[n]) {
                userScreenshotImage[n] = image.roundCorners(withRadius: 20.0)
                userScreenshotTimeStamp[n] = "TODO"
                userScreenshotTimeDiff[n] = "todo"
            }
        }
    }

    @IBAction func selectAction(_ sender: NSSegmentedControl!) {
        
        track("Snapshots: \(snapshots)")
        
        snapshots ? reloadSnapshotCache() : reloadScreenshotCache()
        autoTableView.reloadData()
        userTableView.reloadData()
    }

    @IBAction func button1Action(_ sender: NSButton!) {
        
        if snapshots {
            
            track("\(sender.tag)")
            amiga.deleteUserSnapshot(sender.tag)
            reloadSnapshotCache()
        }
    }

    @IBAction func button2Action(_ sender: NSButton!) {
        
        if snapshots {

            track("\(sender.tag)")
            amiga.deleteUserSnapshot(sender.tag)
            reloadSnapshotCache()
        }
    }

    @IBAction func button3Action(_ sender: NSButton!) {
        
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
            return snapshotTableView(tableView, row: row)
        } else {
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
        
        view.button1.image = NSImage.init(named: "saveTemplate")
        view.button2.image = NSImage.init(named: "folderTemplate")
        view.button2.image = NSImage.init(named: "trashTemplate")
        view.button1.tag = row
        view.button2.tag = row
        view.button3.tag = row
        
        if tableView == autoTableView {
            
            view.preview.image = autoSnapshotImage[row]
            view.text1.stringValue = autoSnapshotTimeStamp[row]!
            view.text2.stringValue = autoSnapshotTimeDiff[row]!
            view.text3.stringValue = "200 x 400 pixels" // placeholder
            
        } else {
            
            view.preview.image = userSnapshotImage[row]
            view.text1.stringValue = userSnapshotTimeStamp[row]!
            view.text2.stringValue = userSnapshotTimeDiff[row]!
            view.text3.stringValue = "100 x 200 pixels" // placeholder
        }
        
        return view
    }
    
    func screenshotTableView(_ tableView: NSTableView, row: Int) -> NSView? {
         
         guard let view = createView(tableView) else { return nil }
         
         view.button1.image = NSImage.init(named: "saveTemplate")
         view.button2.image = NSImage.init(named: "folderTemplate")
         view.button2.image = NSImage.init(named: "trashTemplate")
         view.button1.tag = row
         view.button2.tag = row
         view.button3.tag = row
         
         if tableView == autoTableView {
             
             view.preview.image = autoScreenshotImage[row]
             view.text1.stringValue = autoScreenshotTimeStamp[row]!
             view.text2.stringValue = autoScreenshotTimeDiff[row]!
             view.text3.stringValue = "200 x 400 pixels" // placeholder
             
         } else {
             
             view.preview.image = userScreenshotImage[row]
             view.text1.stringValue = userScreenshotTimeStamp[row]!
             view.text2.stringValue = userScreenshotTimeDiff[row]!
             view.text3.stringValue = "100 x 200 pixels" // placeholder
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
