// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Cocoa

struct SidebarItem {

    let title: String
    let iconName: String
    let identifier: NSUserInterfaceItemIdentifier
}

class SidebarViewController: NSViewController {

    @IBOutlet weak var outlineView: SettingsOutlineView!

    var splitViewController: SettingsSplitViewController? {
        parent as? SettingsSplitViewController
    }

    let items: [SidebarItem] = [
        
        SidebarItem(title: "General", iconName: "generalPrefs", identifier: .init("general")),
        SidebarItem(title: "Controls", iconName: "controlsPrefs", identifier: .init("controls")),
        SidebarItem(title: "Devices", iconName: "devicesPrefs", identifier: .init("devices")),
        SidebarItem(title: "Hardware", iconName: "hardwarePrefs", identifier: .init("hardware"))
    ]

    var selectionHandler: ((SidebarItem) -> Void)?

    override func viewDidLoad() {

        super.viewDidLoad()
        outlineView.delegate = self
        outlineView.dataSource = self
        outlineView.usesAutomaticRowHeights = false
        outlineView.rowSizeStyle = .custom
        outlineView.backgroundColor = .clear
        outlineView.usesAlternatingRowBackgroundColors = false

        // Select first item by default
        outlineView.selectRowIndexes(IndexSet(integer: 0), byExtendingSelection: false)
    }
}

extension SidebarViewController: NSOutlineViewDataSource {

    func outlineView(_ outlineView: NSOutlineView, numberOfChildrenOfItem item: Any?) -> Int {
        return items.count
    }

    func outlineView(_ outlineView: NSOutlineView, heightOfRowByItem item: Any) -> CGFloat {

        return 48
    }

    func outlineView(_ outlineView: NSOutlineView, isItemExpandable item: Any) -> Bool {
        return false
    }

    func outlineView(_ outlineView: NSOutlineView, child index: Int, ofItem item: Any?) -> Any {
        return items[index]
    }
}

extension SidebarViewController: NSOutlineViewDelegate {

    func outlineView(_ outlineView: NSOutlineView, viewFor tableColumn: NSTableColumn?, item: Any) -> NSView? {

        let cell = outlineView.makeView(withIdentifier: NSUserInterfaceItemIdentifier("SidebarCell"), owner: self) as? NSTableCellView

        if let sidebarItem = item as? SidebarItem {
            cell?.textField?.stringValue = sidebarItem.title
            cell?.imageView?.image = NSImage(named: sidebarItem.iconName)
        } else {
            cell?.textField?.stringValue = "???"
            cell?.imageView?.image = nil
        }
        return cell
    }

    func outlineViewSelectionDidChange(_ notification: Notification) {
        
        let selectedIndex = outlineView.selectedRow
        if selectedIndex >= 0 {
            selectionHandler?(items[selectedIndex])
        }
    }
}
