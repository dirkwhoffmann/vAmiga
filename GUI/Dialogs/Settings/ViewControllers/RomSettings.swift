// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class RomSettingsViewController: SettingsViewController {

    // Rom info
    @IBOutlet weak var romDropView: RomDropView!
    @IBOutlet weak var romTitle: NSTextField!
    @IBOutlet weak var romSubtitle: NSTextField!
    @IBOutlet weak var romSubsubtitle: NSTextField!
    @IBOutlet weak var romModel: NSTextField!
    @IBOutlet weak var romDeleteButton: NSButton!

    @IBOutlet weak var extDropView: ExtRomDropView!
    @IBOutlet weak var extTitle: NSTextField!
    @IBOutlet weak var extSubtitle: NSTextField!
    @IBOutlet weak var extSubsubtitle: NSTextField!
    @IBOutlet weak var extModel: NSTextField!
    @IBOutlet weak var extDeleteButton: NSButton!
    @IBOutlet weak var extMapText: NSTextField!
    @IBOutlet weak var extMapAddr: NSPopUpButton!

    // Explanation
    @IBOutlet weak var romExpImage: NSButton!
    @IBOutlet weak var romExpInfo1: NSTextField!
    @IBOutlet weak var romExpInfo2: NSTextField!

    // Lock
    @IBOutlet weak var romLockImage: NSButton!
    @IBOutlet weak var romLockInfo1: NSTextField!
    @IBOutlet weak var romLockInfo2: NSTextField!

    // Buttons
    @IBOutlet weak var romArosPopup: NSPopUpButton!
    @IBOutlet weak var romOKButton: NSButton!
    @IBOutlet weak var romPowerButton: NSButton!

    override func viewDidLoad() {

        print("RomSettingsViewController::viewDidLoad")
    }

    //
    // Refresh
    //

    override func refresh() {

        super.refresh()
    }

    //
    // Action methods
    //

    override func preset(tag: Int) {

        print("RomSettingsViewController::preset")
    }

    override func save() {

        print("RomSettingsViewController::preset")
    }
}
