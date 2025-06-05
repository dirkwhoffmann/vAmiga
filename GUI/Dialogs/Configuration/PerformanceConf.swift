// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
extension ConfigurationController {

    func refreshPerformanceTab() {

        let poweredOff = emu.poweredOff

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
        
        // Lock
        prfLockImage.isHidden = poweredOff
        prfLockInfo1.isHidden = poweredOff
        prfLockInfo2.isHidden = poweredOff

        // Buttons
        prfPowerButton.isHidden = !bootable
    }

    //
    // Action methods (Warp)
    //

    @IBAction func prfWarpModeAction(_ sender: NSPopUpButton!) {

        config.warpMode = sender.selectedTag()
    }

    @IBAction func prfWarpBootAction(_ sender: NSTextField!) {

        config.warpBoot = sender.integerValue
    }
    
    //
    // Action methods (Threading)
    //

    @IBAction func prfVSyncAction(_ sender: NSButton!) {

        config.vsync = sender.state == .on
    }

    @IBAction func prfspeedBoostAction(_ sender: NSSlider!) {

        config.speedBoost = sender.integerValue
    }

    @IBAction func prfRunAheadAction(_ sender: NSSlider!) {

        config.runAhead = sender.integerValue
    }


    //
    // Action methods (Boosters)
    //

    @IBAction func prfCiaIdleSleepAction(_ sender: NSButton!) {

        config.ciaIdleSleep = sender.state == .on
    }

    @IBAction func prfFrameSkippingAction(_ sender: NSButton!) {

        config.frameSkipping = sender.state == .on ? 16 : 0
    }

    @IBAction func prfAudioFastPathAction(_ sender: NSButton!) {

        config.audioFastPath = sender.state == .on
    }

    //
    // Action methods (Compressor)
    //

    @IBAction func prfWsCompressorAction(_ sender: NSPopUpButton!) {

        config.wsCompressor = sender.selectedTag()
    }

    @IBAction func prfSnapCompressorAction(_ sender: NSPopUpButton!) {

        config.snapCompressor = sender.selectedTag()
    }

    //
    // Presets
    //

    @IBAction func prfPresetAction(_ sender: NSPopUpButton!) {

        emu.suspend()

        // Revert to standard settings
        EmulatorProxy.defaults.removePerformanceUserDefaults()

        // Update the configutation
        config.applyPerformanceUserDefaults()

        emu.resume()
    }

    @IBAction func prfDefaultsAction(_ sender: NSButton!) {

        config.savePerformanceUserDefaults()
    }
}
