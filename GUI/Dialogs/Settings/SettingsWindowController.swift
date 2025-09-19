// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Cocoa

class SettingsWindowController: NSWindowController {

    var splitViewController: SettingsSplitViewController? {
        self.contentViewController as? SettingsSplitViewController
    }
    var currentVC: SettingsViewController? { splitViewController?.currentVC }

    required init?(coder: NSCoder) {

        super.init(coder: coder)
    }

    override func windowDidLoad() {

        super.windowDidLoad()

        if let toolbar = window?.toolbar as? SettingsToolbar {
            toolbar.windowController = self
        }
    }

    func show() {

        self.showWindow(nil)
        self.window?.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
    }

    /*
    @IBAction func presetAction(_ sender: Any) {

        print("presetAction")
        // currentVC?.preset(tag: sender.selectedTag())
        // currentVC?.refresh()
    }
    */
}
