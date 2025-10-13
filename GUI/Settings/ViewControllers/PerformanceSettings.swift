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
    @IBOutlet weak var warpMode: NSPopUpButton!
    @IBOutlet weak var warpBoot: NSTextField!

    // Threading
    @IBOutlet weak var vSync: NSButton!
    @IBOutlet weak var speedBoost: NSSlider!
    @IBOutlet weak var speedBoostInfo: NSTextField!
    @IBOutlet weak var runAheadLabel: NSTextField!
    @IBOutlet weak var runAheadSlider: NSSlider!
    @IBOutlet weak var runAheadInfo: NSTextField!

    // Boosters
    @IBOutlet weak var ciaIdleSleep: NSButton!
    @IBOutlet weak var frameSkipping: NSButton!
    @IBOutlet weak var audioFastPath: NSButton!

    // Compression
    @IBOutlet weak var wsCompressor: NSPopUpButton!
    @IBOutlet weak var snapCompressor: NSPopUpButton!

    override func viewDidLoad() {

        log(.lifetime)
    }

    //
    // Methods from SettingsViewController
    //

    override func refresh() {

        super.refresh()

        guard let config = config else { return }

        // Warp
        warpMode.selectItem(withTag: config.warpMode)
        warpBoot.integerValue = config.warpBoot

        // Threading
        let vsync = config.vsync
        let runAhead = config.runAhead
        vSync.state = config.vsync ? .on : .off
        speedBoost.integerValue = config.speedBoost
        speedBoostInfo.stringValue = "\(config.speedBoost) %"
        speedBoost.isEnabled = !vsync
        speedBoostInfo.textColor = vsync ? .tertiaryLabelColor : .labelColor
        runAheadLabel.stringValue = runAhead >= 0 ? "Run ahead:" : "Run behind:"
        runAheadSlider.integerValue = runAhead
        runAheadInfo.stringValue = "\(abs(runAhead)) frame" + (abs(runAhead) == 1 ? "" : "s")

        // Boosters
        ciaIdleSleep.state = config.ciaIdleSleep ? .on : .off
        frameSkipping.state = config.frameSkipping > 0 ? .on : .off
        audioFastPath.state = config.audioFastPath ? .on : .off

        // Compression
        wsCompressor.selectItem(withTag: config.wsCompressor)
    }

    override func preset(tag: Int) {

        emu?.suspend()

        // Revert to standard settings
        EmulatorProxy.defaults.removePerformanceUserDefaults()

        // Update the configutation
        config?.applyPerformanceUserDefaults()

        emu?.resume()
    }

    override func save() {

        config?.savePerformanceUserDefaults()
    }

    //
    // Action methods (Warp)
    //

    @IBAction func warpModeAction(_ sender: NSPopUpButton!) {

        config?.warpMode = sender.selectedTag()
    }

    @IBAction func warpBootAction(_ sender: NSTextField!) {

        config?.warpBoot = sender.integerValue
    }

    //
    // Action methods (Threading)
    //

    @IBAction func vSyncAction(_ sender: NSButton!) {

        config?.vsync = sender.state == .on
    }

    @IBAction func speedBoostAction(_ sender: NSSlider!) {

        config?.speedBoost = sender.integerValue
    }

    @IBAction func runAheadAction(_ sender: NSSlider!) {

        config?.runAhead = sender.integerValue
    }


    //
    // Action methods (Boosters)
    //

    @IBAction func ciaIdleSleepAction(_ sender: NSButton!) {

        config?.ciaIdleSleep = sender.state == .on
    }

    @IBAction func frameSkippingAction(_ sender: NSButton!) {

        config?.frameSkipping = sender.state == .on ? 16 : 0
    }

    @IBAction func audioFastPathAction(_ sender: NSButton!) {

        config?.audioFastPath = sender.state == .on
    }

    //
    // Action methods (Compressor)
    //

    @IBAction func wsCompressorAction(_ sender: NSPopUpButton!) {

        config?.wsCompressor = sender.selectedTag()
    }
}
