// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

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
        prfTimeLapse.integerValue = config.timeLapse
        prfTimeLapseInfo.stringValue = "\(config.timeLapse) %"
        prfTimeLapse.isEnabled = !vsync
        prfTimeLapseInfo.textColor = vsync ? .tertiaryLabelColor : .labelColor
        prfRunAheadSlider.integerValue = runAhead
        prfRunAheadInfo.stringValue = "\(runAhead) frame" + (runAhead == 1 ? "" : "s")

        // Collision detection
        prfClxSprSpr.state = config.clxSprSpr ? .on : .off
        prfClxSprPlf.state = config.clxSprPlf ? .on : .off
        prfClxPlfPlf.state = config.clxPlfPlf ? .on : .off

        // Boosters
        prfCiaIdleSleep.state = config.ciaIdleSleep ? .on : .off
        prfFrameSkipping.state = config.frameSkipping > 0 ? .on : .off
        prfAudioFastPath.state = config.audioFastPath ? .on : .off

        // Lock
        prfLockImage.isHidden = poweredOff
        prfLockInfo1.isHidden = poweredOff
        prfLockInfo2.isHidden = poweredOff

        // Buttons
        prfPowerButton.isHidden = !bootable
    }

    //
    // Action methods (warp)
    //

    @IBAction func prfWarpModeAction(_ sender: NSPopUpButton!) {

        config.warpMode = sender.selectedTag()
    }

    @IBAction func prfWarpBootAction(_ sender: NSTextField!) {

        config.warpBoot = sender.integerValue
    }
    
    //
    // Action methods (threading)
    //

    @IBAction func prfVSyncAction(_ sender: NSButton!) {

        config.vsync = sender.state == .on
    }

    @IBAction func prfTimeLapseAction(_ sender: NSSlider!) {

        config.timeLapse = sender.integerValue
    }

    @IBAction func prfRunAheadAction(_ sender: NSSlider!) {

        config.runAhead = sender.integerValue
    }

    //
    // Action methods (collision detection)
    //

    @IBAction func prfClxSprSprAction(_ sender: NSButton!) {

        config.clxSprSpr = sender.state == .on
    }

    @IBAction func prfClxSprPlfAction(_ sender: NSButton!) {

        config.clxSprPlf = sender.state == .on
    }

    @IBAction func prfClxPlfPlfAction(_ sender: NSButton!) {

        config.clxPlfPlf = sender.state == .on
    }

    //
    // Action methods (performance boosters)
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
