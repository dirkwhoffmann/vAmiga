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

        // Warp
        prfWarpMode.selectItem(withTag: config.warpMode)
        // prfWarpBoot.integerValue = config.warpBoot

        // Collision detection
        prfClxSprSpr.state = config.clxSprSpr ? .on : .off
        prfClxSprPlf.state = config.clxSprPlf ? .on : .off
        prfClxPlfPlf.state = config.clxPlfPlf ? .on : .off

        // Boosters
        prfCiaIdleSleep.state = config.ciaIdleSleep ? .on : .off
        prfFrameSkipping.state = config.frameSkipping > 0 ? .on : .off
    }

    //
    // Action methods (warp)
    //

    @IBAction func prfWarpModeAction(_ sender: NSPopUpButton!) {

        config.warpMode = sender.selectedTag()
        refresh()
    }

    @IBAction func prfWarpBootAction(_ sender: NSTextField!) {

        // config.warpBoot = sender.integerValue
        refresh()
    }
    
    //
    // Action methods (collision detection)
    //

    @IBAction func prfClxSprSprAction(_ sender: NSButton!) {

        config.clxSprSpr = sender.state == .on
        refresh()
    }

    @IBAction func prfClxSprPlfAction(_ sender: NSButton!) {

        config.clxSprPlf = sender.state == .on
        refresh()
    }

    @IBAction func prfClxPlfPlfAction(_ sender: NSButton!) {

        config.clxPlfPlf = sender.state == .on
        refresh()
    }

    //
    // Action methods (performance boosters)
    //

    @IBAction func prfCiaIdleSleepAction(_ sender: NSButton!) {

        config.ciaIdleSleep = sender.state == .on
        refresh()
    }

    @IBAction func prfFrameSkippingAction(_ sender: NSButton!) {

        config.frameSkipping = sender.state == .on ? 16 : 0
        refresh()
    }

}
