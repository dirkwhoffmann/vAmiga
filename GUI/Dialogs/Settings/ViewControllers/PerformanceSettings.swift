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
        prfWarpMode.selectItem(withTag: config.warpMode)
        prfWarpBoot.integerValue = config.warpBoot

        // Threading
        let vsync = config.vsync
        let runAhead = config.runAhead
        prfVSync.state = config.vsync ? .on : .off
        prfSpeedBoost.integerValue = config.speedBoost
        prfSpeedBoostInfo.stringValue = "\(config.speedBoost) %"
        prfSpeedBoost.isEnabled = !vsync
        prfSpeedBoostInfo.textColor = vsync ? .tertiaryLabelColor : .labelColor
        prfRunAheadLabel.stringValue = runAhead >= 0 ? "Run ahead:" : "Run behind:"
        prfRunAhead.integerValue = runAhead
        prfRunAheadInfo.stringValue = "\(abs(runAhead)) frame" + (abs(runAhead) == 1 ? "" : "s")

        // Boosters
        prfCiaIdleSleep.state = config.ciaIdleSleep ? .on : .off
        prfFrameSkipping.state = config.frameSkipping > 0 ? .on : .off
        prfAudioFastPath.state = config.audioFastPath ? .on : .off

        // Compression
        prfWsCompressor.selectItem(withTag: config.wsCompressor)
        prfSnapCompressor.selectItem(withTag: config.snapCompressor)
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

    @IBAction func prfWarpModeAction(_ sender: NSPopUpButton!) {

        config?.warpMode = sender.selectedTag()
    }

    @IBAction func prfWarpBootAction(_ sender: NSTextField!) {

        config?.warpBoot = sender.integerValue
    }

    //
    // Action methods (Threading)
    //

    @IBAction func prfVSyncAction(_ sender: NSButton!) {

        config?.vsync = sender.state == .on
    }

    @IBAction func prfspeedBoostAction(_ sender: NSSlider!) {

        config?.speedBoost = sender.integerValue
    }

    @IBAction func prfRunAheadAction(_ sender: NSSlider!) {

        config?.runAhead = sender.integerValue
    }


    //
    // Action methods (Boosters)
    //

    @IBAction func prfCiaIdleSleepAction(_ sender: NSButton!) {

        config?.ciaIdleSleep = sender.state == .on
    }

    @IBAction func prfFrameSkippingAction(_ sender: NSButton!) {

        config?.frameSkipping = sender.state == .on ? 16 : 0
    }

    @IBAction func prfAudioFastPathAction(_ sender: NSButton!) {

        config?.audioFastPath = sender.state == .on
    }

    //
    // Action methods (Compressor)
    //

    @IBAction func prfWsCompressorAction(_ sender: NSPopUpButton!) {

        config?.wsCompressor = sender.selectedTag()
    }

    @IBAction func prfSnapCompressorAction(_ sender: NSPopUpButton!) {

        config?.snapCompressor = sender.selectedTag()
    }
}
