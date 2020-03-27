// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class KeyViewItem: NSCollectionViewItem {
    
    override func viewDidLoad() {
        
        super.viewDidLoad()
        view.wantsLayer = true
    }
}

class DiskMountDialog: DialogController {
        
    @IBOutlet weak var diskIcon: NSImageView!
    @IBOutlet weak var disclosureButton: NSButton!
    @IBOutlet weak var infoText: NSTextField!
    @IBOutlet weak var warningText: NSTextField!
    @IBOutlet weak var df0Button: NSButton!
    @IBOutlet weak var df1Button: NSButton!
    @IBOutlet weak var df2Button: NSButton!
    @IBOutlet weak var df3Button: NSButton!
    @IBOutlet weak var keyMatrixCollectionView: NSCollectionView!
    @IBOutlet weak var keyMatrixScrollView: NSScrollView!
    
    var disk: ADFFileProxy!
    var writeProtect = false
    var shrinked: Bool { return window!.frame.size.height < 300 }
    var screenshots: [NSImage] = []
    
    override func showSheet(completionHandler handler:(() -> Void)? = nil) {
    
        if let attachment = myDocument?.amigaAttachment as? ADFFileProxy {
            
            disk = attachment
            updateScreenshots()
            super.showSheet(completionHandler: handler)
        }
    }
    
    override public func awakeFromNib() {

        track()

        // var rect = window!.frame
        // rect.size.height = 176 + 20
        // window!.setFrame(rect, display: true)
        
        keyMatrixScrollView.drawsBackground = false
        keyMatrixScrollView.contentView.drawsBackground = false
        
        // keyMatrixScrollView.backgroundColor = .clear
        keyMatrixCollectionView.backgroundColors = [.clear]
        
        update()
        keyMatrixCollectionView.reloadData()
    }
    
    override func windowDidLoad() {
     
        if screenshots.count == 0 { shrink() }
        track()
    }
 
    func updateScreenshots() {
        
        let fingerprint = disk.fnv()
        let folder = String(format: "%02X", fingerprint)
        let urls = parent.screenshotFolderContents(folder)
        
        for url in urls {
            if let image = NSImage.init(contentsOf: url) {
                screenshots.append(image)
            }
        }

        track("Found \(screenshots.count) screenshots for this disk")
    }
    
    func setHeight(_ newHeight: CGFloat) {

        var rect = window!.frame
        rect.origin.y += rect.size.height - newHeight
        rect.size.height = newHeight
        
        track("\(rect)")
        window?.setFrame(rect, display: true)
        update()
    }
    
    func shrink() { setHeight(176) }
    func expand() { setHeight(358) }
    
    func update() {
        
        let size = window!.frame.size
        let hide = size.height < 300
        
        // Update disk icon
        diskIcon.image = NSImage.init(named: writeProtect ? "adf_protected" : "adf")
        
        // Update text fields
        let typeName = [
            DISK_35_DD.rawValue: "3.5\"DD Amiga",
            DISK_35_DD_PC.rawValue: "3.5\"DD PC",
            DISK_35_HD.rawValue: "3.5\"HD Amiga",
            DISK_35_HD_PC.rawValue: "3.5\"HD PC",
            DISK_525_SD.rawValue: "5.25\"SD PC"
        ]
        let str = typeName[disk.diskType().rawValue]!
        infoText.stringValue = "A byte-accurate image of \(str) diskette."
        let compatible = disk.diskType() == DISK_35_DD
            
        // Update the disclosure button state
        disclosureButton.state = shrinked ? .off : .on
        warningText.isHidden = compatible
        
        // Check for available drives
        let dc = amiga.diskController.getConfig()

        let connected0 = dc.connected.0
        let connected1 = dc.connected.1
        let connected2 = dc.connected.2
        let connected3 = dc.connected.3
        df0Button.isEnabled = compatible && connected0
        df1Button.isEnabled = compatible && connected1
        df2Button.isEnabled = compatible && connected2
        df3Button.isEnabled = compatible && connected3

        // Hide some elements if window is shrinked
        let items: [NSView] = [
            
            keyMatrixScrollView
        ]
        for item in items { item.isHidden = hide }
        
        // Only proceed if window is expanded
        if hide { return }
    }
    
    //
    // Action methods
    //

    @IBAction func disclosureAction(_ sender: NSButton!) {
        
        shrinked ? expand() : shrink()
    }

    @IBAction func insertDiskAction(_ sender: NSButton!) {
        
        track("insertDiskAction df\(sender.tag)")
        
        amiga.diskController.insert(sender.tag, adf: disk)
        amiga.diskController.setWriteProtection(sender.tag, value: writeProtect)

        parent.renderer.rotateDown()
        hideSheet()
    }
    
    @IBAction func writeProtectAction(_ sender: NSButton!) {
        
        writeProtect = sender.state == .on
        update()
    }
}

//
// NSTableView delegate and data source
//

extension DiskMountDialog: NSTableViewDelegate {
    
    func tableView(_ tableView: NSTableView,
                   willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {
        
        // let c = cell as! NSTextFieldCell
        // c.font = cbmfont
        // c.textColor = .red
    }
}
extension DiskMountDialog: NSWindowDelegate {
    
    func windowDidResize(_ notification: Notification) {
        
        update()
    }
}

//
// NSCollectionView data source and delegate
//

extension DiskMountDialog: NSCollectionViewDataSource {
    
    func numberOfSections(in collectionView: NSCollectionView) -> Int {
        
        return 1
    }
    
    func collectionView(_ collectionView: NSCollectionView,
                        numberOfItemsInSection section: Int) -> Int {
        
        return screenshots.count
    }
    
    func collectionView(_ itemForRepresentedObjectAtcollectionView: NSCollectionView,
                        itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {
        
        let id = NSUserInterfaceItemIdentifier(rawValue: "KeyViewItem")
        let item = keyMatrixCollectionView.makeItem(withIdentifier: id, for: indexPath)
        guard let keyViewItem = item as? KeyViewItem else {
            return item
        }
        
        // let row = indexPath.section
        // let col = indexPath.item
        keyViewItem.imageView?.image = screenshots[indexPath.item]
        return keyViewItem
    }
}

extension DiskMountDialog: NSCollectionViewDelegate {
    
    func collectionView(_ collectionView: NSCollectionView,
                        didSelectItemsAt indexPaths: Set<IndexPath>) {
        
    }
}
