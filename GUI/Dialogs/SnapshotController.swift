// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

class SnapshotTableCellView: NSTableCellView {
    
    @IBOutlet weak var preview: NSImageView!
    @IBOutlet weak var text: NSTextField!
    @IBOutlet weak var subText: NSTextField!
    @IBOutlet weak var delete: NSButton!
}

class SnapshotDialog : DialogController  {
    
    // Outlets
    
    @IBOutlet weak var autoTableView: NSTableView!
    @IBOutlet weak var userTableView: NSTableView!

    var now: Date = Date()
    
    // Auto-saved snapshot cache
    var numAutoSnapshots = -1
    var autoSnapshotImage: [Int:NSImage] = [:]
    var autoTimeStamp: [Int:String] = [:]
    var autoTimeDiff: [Int:String] = [:]
    var autoSlotForRow: [Int:Int] = [:]
    
    // User-saved snapshot cache
    var numUserSnapshots = -1
    var userSnapshotImage: [Int:NSImage] = [:]
    var userTimeStamp: [Int:String] = [:]
    var userTimeDiff: [Int:String] = [:]
    var userSlotForRow: [Int:Int] = [:]
    
    override public func awakeFromNib() {
        
        track("awakeFromNib")
        
        if numAutoSnapshots == -1 {
            
            // Disable auto snapshot saving while dialog is open
            amigaProxy?.suspendAutoSnapshots()
            
            // Setup snapshot caches
            reloadAutoSnapshotCache()
            reloadUserSnapshotCache()
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
        
        var diff = Int(round(now.timeIntervalSince1970 - Double(timeStamp)))
        let min = diff / 60;
        let hrs = diff / 3600;
        if (diff) < 60 {
            let s = (diff == 1) ? "" : "s"
            return "\(diff) second\(s) ago"
        } else if (diff) < 3600 {
            diff = diff % 60
            return "\(min):\(diff) minutes ago"
        } else {
            diff = diff % 60
            return "\(hrs):\(min) hours ago"
        }
    }
    
    func reloadAutoSnapshotCache() {
        
        if let amiga = amigaProxy {
            
            track()
            
            amiga.suspend()
            numAutoSnapshots = amiga.numAutoSnapshots()
            for n in 0..<numAutoSnapshots {
                let takenAt = TimeInterval(amiga.autoSnapshotTimestamp(n))
                autoSnapshotImage[n] = amiga.autoSnapshotImage(n)
                autoTimeStamp[n] = timeInfo(timeStamp: takenAt)
                autoTimeDiff[n] = timeDiffInfo(timeStamp: takenAt)
            }
            amiga.resume()
            autoTableView.reloadData()
        }
    }
    
    func reloadUserSnapshotCache() {
        
        if let amiga = amigaProxy {
        
            track()
            
            amiga.suspend()
            numUserSnapshots = amiga.numUserSnapshots()
            for n in 0..<numUserSnapshots {
                let takenAt = TimeInterval(amiga.userSnapshotTimestamp(n))
                userSnapshotImage[n] = amiga.userSnapshotImage(n)
                userTimeStamp[n] = timeInfo(timeStamp: takenAt)
                userTimeDiff[n] = timeDiffInfo(timeStamp: takenAt)
            }
            amiga.resume()
            userTableView.reloadData()
        }
    }
    
    @IBAction func deleteAction(_ sender: Any!) {
        
        track()
        
        let sender = sender as! NSButton
        amigaProxy?.deleteUserSnapshot(sender.tag)
        reloadUserSnapshotCache()
    }
    
    @IBAction override func cancelAction(_ sender: Any!) {
        
        track()
        amigaProxy?.resumeAutoSnapshots()
        hideSheet()
    }
    
    @IBAction func autoDoubleClick(_ sender: Any!) {
        
        let sender = sender as! NSTableView
        amigaProxy?.restoreAutoSnapshot(sender.selectedRow)
        cancelAction(self)
    }
    
    @IBAction func userDoubleClick(_ sender: Any!) {
        
        let sender = sender as! NSTableView
        amigaProxy?.restoreUserSnapshot(sender.selectedRow)
        cancelAction(self)
    }
}

//
// NSTableViewDataSource, NSTableViewDelegate
//

extension SnapshotDialog : NSTableViewDataSource, NSTableViewDelegate {
    
    func numberOfRows(in tableView: NSTableView) -> Int {

        if (tableView == autoTableView) {
            return numAutoSnapshots
        }
            
        else if (tableView == userTableView) {
            return numUserSnapshots
        }
        
        fatalError();
    }
    
    func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView?{
        
        let id = NSUserInterfaceItemIdentifier(rawValue: "defaultRow")
        let result = tableView.makeView(withIdentifier: id, owner: self) as! SnapshotTableCellView
        
        if (tableView == autoTableView) {
            
            result.preview.image = autoSnapshotImage[row]
            result.text.stringValue = autoTimeStamp[row]!
            result.subText.stringValue = autoTimeDiff[row]!
            result.delete.isHidden = true
            result.delete.tag = row
            return result;
        }
        
        else if (tableView == userTableView) {
            
            result.preview.image = userSnapshotImage[row]
            result.text.stringValue = userTimeStamp[row]!
            result.subText.stringValue = userTimeDiff[row]!
            result.delete.isHidden = false
            result.delete.tag = row
            return result;
        }
    
        fatalError();
    }
}

//
// Drag and drop
//

extension SnapshotDialog {
    
    func tableView(_ tableView: NSTableView,
                   writeRowsWith rowIndexes: IndexSet,
                   to pboard: NSPasteboard) -> Bool {
        
        // Get active emulator instance
        guard let amiga = amigaProxy else {
            return false
        }
        
        // Get index of dragged item
        guard let index = rowIndexes.first else {
            track("Cannot get table index for drag and drop")
            return false
        }
        
        // Get snapshot data
        var data : Data
        if (tableView == autoTableView) {
            data = amiga.autoSnapshotData(index)
        }
        else {
            assert(tableView == userTableView)
            data = amiga.userSnapshotData(index)
        }
        
        let pboardType = NSPasteboard.PasteboardType.fileContents
        pboard.declareTypes([pboardType], owner: self)
        let fileWrapper = FileWrapper.init(regularFileWithContents: data)
        fileWrapper.preferredFilename = "Snapshot.vam"
        pboard.write(fileWrapper)

        return true;
    }
}

