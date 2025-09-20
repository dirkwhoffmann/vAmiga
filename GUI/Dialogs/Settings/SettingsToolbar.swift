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

    override func validateVisibleItems() {

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
