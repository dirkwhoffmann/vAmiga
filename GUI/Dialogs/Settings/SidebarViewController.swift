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

    @IBOutlet weak var outlineView: NSOutlineView!

    let items: [SidebarItem] = [
        SidebarItem(title: "General", iconName: "CategoryGeneral", identifier: .init("general")),
        /*
        SidebarItem(title: "Shader", iconName: "CategoryShader", identifier: .init("shader")),
        SidebarItem(title: "Recorder", iconName: "CategoryRecorder", identifier: .init("recorder"))
        */
    ]

    var selectionHandler: ((SidebarItem) -> Void)?

    override func viewDidLoad() {

        super.viewDidLoad()
        outlineView.delegate = self
        outlineView.dataSource = self
        outlineView.usesAutomaticRowHeights = false
        outlineView.rowHeight = 48
        outlineView.rowSizeStyle = .custom
        // tableView.reloadData()

        // Select first item by default
        // outlineView.selectRowIndexes(IndexSet(integer: 0), byExtendingSelection: false)
    }
}

extension SidebarViewController: NSOutlineViewDataSource, NSOutlineViewDelegate {

    func outlineView(_ outlineView: NSOutlineView, numberOfChildrenOfItem item: Any?) -> Int {
        return items.count
    }

    func outlineView(_ outlineView: NSOutlineView, isItemExpandable item: Any) -> Bool {
        return false
    }

    func outlineView(_ outlineView: NSOutlineView, child index: Int, ofItem item: Any?) -> Any {
        return "Item \(index)"
    }

    func outlineView(_ outlineView: NSOutlineView, viewFor tableColumn: NSTableColumn?, item: Any) -> NSView? {

        let cell = outlineView.makeView(withIdentifier: NSUserInterfaceItemIdentifier("SidebarCell"), owner: self) as? NSTableCellView
        cell?.textField?.stringValue = item as? String ?? ""
        return cell
    }

    func outlineViewSelectionDidChange(_ notification: Notification) {

        let selectedIndex = outlineView.selectedRow
        if selectedIndex >= 0 {
            selectionHandler?(items[selectedIndex])
        }
    }
}
