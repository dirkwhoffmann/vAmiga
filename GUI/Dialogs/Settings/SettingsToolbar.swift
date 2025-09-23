// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/*
@MainActor
class SettingsToolbar: NSToolbar {

    @IBOutlet weak var presetsPopup: NSPopUpButton!

    var windowController: SettingsWindowController!
    var splitViewController: SettingsSplitViewController? {windowController.splitViewController }

    override func validateVisibleItems() {

    }

    func update(presets: [ (String, Int) ]) {

        print("updatePresets")

        // Remove any items set in Interface Builder
        presetsPopup.removeAllItems()

        // Add the first (disabled) row
        presetsPopup.menu?.addItem(withTitle: "Revert to...", action: nil, keyEquivalent: "")

        // Add new presets
        for (title, tag) in presets {

            if title == "" {
                presetsPopup.menu?.addItem(NSMenuItem.separator())
            } else {
                let item = NSMenuItem(title: title, action: nil, keyEquivalent: "")
                item.tag = tag
                presetsPopup.menu?.addItem(item)
            }
        }
    }

    //
    // Action methods
    //

    @IBAction
    func presetAction(_ sender: NSPopUpButton) {

        splitViewController?.presetAction(sender)

    }

    @IBAction
    func saveAction(_ sender: Any) {

        splitViewController?.saveAction(sender)
    }
}
*/
