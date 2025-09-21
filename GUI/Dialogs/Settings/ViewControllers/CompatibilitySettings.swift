// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class CompatibilitySettingsViewController: SettingsViewController {

    // Blitter
    @IBOutlet weak var compBltAccuracy: NSSlider!
    @IBOutlet weak var compBltLevel2: NSTextField!
    @IBOutlet weak var compBltLevel1: NSTextField!

    // Chipset features
    @IBOutlet weak var compSlowRamMirror: NSButton!
    @IBOutlet weak var compSlowRamDelay: NSButton!
    @IBOutlet weak var compTodBug: NSButton!
    @IBOutlet weak var compPtrDrops: NSButton!

    // Timing
    @IBOutlet weak var compEClockSyncing: NSButton!

    // Disk controller
    @IBOutlet weak var compDriveSpeed: NSPopUpButton!
    @IBOutlet weak var compMechanics: NSButton!
    @IBOutlet weak var compLockDskSync: NSButton!
    @IBOutlet weak var compAutoDskSync: NSButton!

    // Keyboard
    @IBOutlet weak var compAccurateKeyboard: NSButton!

    // Collision detection
    @IBOutlet weak var compClxSprSpr: NSButton!
    @IBOutlet weak var compClxSprPlf: NSButton!
    @IBOutlet weak var compClxPlfPlf: NSButton!

    // Buttons
    @IBOutlet weak var compOKButton: NSButton!
    @IBOutlet weak var compPowerButton: NSButton!
    
    override func viewDidLoad() {

        print("CompatibilitySettingsViewController::viewDidLoad")
    }

    //
    // Refresh
    //

    override func refresh() {

    }

    //
    // Action methods
    //

    override func preset(tag: Int) {

        print("CompatibilitySettingsViewController::preset")
    }

    override func save() {

        print("CompatibilitySettingsViewController::preset")
    }
}
