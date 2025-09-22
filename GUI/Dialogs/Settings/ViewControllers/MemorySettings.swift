// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class MemorySettingsViewController: SettingsViewController {

    // RAM
    @IBOutlet weak var memChipRamPopup: NSPopUpButton!
    @IBOutlet weak var memSlowRamPopup: NSPopUpButton!
    @IBOutlet weak var memFastRamPopup: NSPopUpButton!

    // Memory properties
    @IBOutlet weak var memRamInitPattern: NSPopUpButton!
    @IBOutlet weak var memBankMap: NSPopUpButton!
    @IBOutlet weak var memUnmappingType: NSPopUpButton!

    // Warning
    @IBOutlet weak var memWarnImage: NSButton!
    @IBOutlet weak var memWarnInfo1: NSTextField!
    @IBOutlet weak var memWarnInfo2: NSTextField!

    override func viewDidLoad() {

        print("MemorySettingsViewController::viewDidLoad")
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

        print("MemorySettingsViewController::preset")
    }

    override func save() {

        print("MemorySettingsViewController::preset")
    }
}
