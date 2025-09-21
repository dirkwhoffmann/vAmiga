// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class PerformanceSettingsViewController: SettingsViewController {

    // Warp
    @IBOutlet weak var prfWarpMode: NSPopUpButton!
    @IBOutlet weak var prfWarpBoot: NSTextField!

    // Threading
    @IBOutlet weak var prfVSync: NSButton!
    @IBOutlet weak var prfSpeedBoost: NSSlider!
    @IBOutlet weak var prfSpeedBoostInfo: NSTextField!
    @IBOutlet weak var prfRunAheadLabel: NSTextField!
    @IBOutlet weak var prfRunAhead: NSSlider!
    @IBOutlet weak var prfRunAheadInfo: NSTextField!

    // Boosters
    @IBOutlet weak var prfCiaIdleSleep: NSButton!
    @IBOutlet weak var prfFrameSkipping: NSButton!
    @IBOutlet weak var prfAudioFastPath: NSButton!

    // Compression
    @IBOutlet weak var prfWsCompressor: NSPopUpButton!
    @IBOutlet weak var prfSnapCompressor: NSPopUpButton!

    // Lock
    @IBOutlet weak var prfLockImage: NSButton!
    @IBOutlet weak var prfLockInfo1: NSTextField!
    @IBOutlet weak var prfLockInfo2: NSTextField!

    // Buttons
    @IBOutlet weak var prfFactorySettingsPopup: NSPopUpButton!
    @IBOutlet weak var prfOKButton: NSButton!
    @IBOutlet weak var prfPowerButton: NSButton!
    
    override func viewDidLoad() {

        print("PerformanceSettingsViewController::viewDidLoad")
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

        print("PerformanceSettingsViewController::preset")
    }

    override func save() {

        print("PerformanceSettingsViewController::preset")
    }
}
