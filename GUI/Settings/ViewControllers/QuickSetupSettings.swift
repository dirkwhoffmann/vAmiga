// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class QuickSetupSettingsViewController: SettingsViewController {

    // Model
    @IBOutlet weak var modelPopup: NSPopUpButton!
    @IBOutlet weak var modelText: NSTextField!
    @IBOutlet weak var modelApply: NSButton!

    // Monitor
    @IBOutlet weak var monitorPopup: NSPopUpButton!
    @IBOutlet weak var monitorText: NSTextField!
    @IBOutlet weak var monitorApply: NSButton!

    // Profile
    @IBOutlet weak var profilePopup: NSPopUpButton!
    @IBOutlet weak var profileText: NSTextField!
    @IBOutlet weak var profileApply: NSButton!

    override func viewDidLoad() {

        log(.lifetime)
    }

    //
    // Methods from SettingsViewController
    //

    override func refresh() {

        super.refresh()
    }
}
