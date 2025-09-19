// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
class SettingsToolbar: NSToolbar {

    var windowController: SettingsWindowController!
    var splitViewController: SettingsSplitViewController? {windowController.splitViewController }

    /*
    @IBOutlet weak var timeStamp: NSButton!
    @IBOutlet weak var execSegCtrl: NSSegmentedControl!
    @IBOutlet weak var selectorPopup: NSPopUpButton!
    @IBOutlet weak var selectorToolbarItem: NSToolbarItem!
    @IBOutlet weak var formatPopup: NSPopUpButton!
    @IBOutlet weak var formatToolbarItem: NSToolbarItem!
    */

    override func validateVisibleItems() {

        // print("validateVisibleItems")
    }

    //
    // Action methods
    //

    @IBAction
    func presetAction(_ sender: NSPopUpButton) {

        print("Toolbar: presetAction")
        if splitViewController == nil { print("splitViewController is nil") }
        splitViewController?.presetAction(sender)

    }

    @IBAction
    func saveAction(_ sender: Any) {

        print("saveAction")
    }
}
