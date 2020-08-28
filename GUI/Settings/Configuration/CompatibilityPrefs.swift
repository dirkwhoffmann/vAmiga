// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ConfigurationController {

    func refreshCompatibilityTab() {

        let config = amiga.config

        // Graphics
        compClxSprSpr.state = config.denise.clxSprSpr ? .on : .off
        compClxSprPlf.state = config.denise.clxSprPlf ? .on : .off
        compClxPlfPlf.state = config.denise.clxPlfPlf ? .on : .off

        // Blitter
        let a = config.blitter.accuracy
        compBltAccuracy.intValue = a
        compBltLevel1.textColor = (a >= 1) ? .labelColor : .tertiaryLabelColor
        compBltLevel2.textColor = (a >= 2) ? .labelColor : .tertiaryLabelColor

        // CIAs
        compTodBug.state = config.ciaA.todBug ? .on : .off
        
        // Floppy drives
        let speed = config.df0.speed        
        assert(speed == config.df1.speed)
        assert(speed == config.df2.speed)
        assert(speed == config.df3.speed)
        compDriveSpeed.selectItem(withTag: Int(speed))
        compAsyncFifo.isHidden = speed == -1
        compAsyncFifo.state = config.diskController.asyncFifo ? .on : .off
        compLockDskSync.state = config.diskController.lockDskSync ? .on : .off
        compAutoDskSync.state = config.diskController.autoDskSync ? .on : .off

        // Keyboard
        compAccurateKeyboard.state = config.keyboard.accurate ? .on : .off

        // Buttons
        compPowerButton.isHidden = !bootable
    }

    @IBAction func compClxSprSprAction(_ sender: NSButton!) {

        config.clxSprSpr = sender.state == .on
        refresh()
    }

    @IBAction func compClxSprPlfAction(_ sender: NSButton!) {

        config.clxSprPlf = sender.state == .on
        refresh()
    }

    @IBAction func compClxPlfPlfAction(_ sender: NSButton!) {

        config.clxPlfPlf = sender.state == .on
        refresh()
    }

    @IBAction func compBltAccuracyAction(_ sender: NSSlider!) {

        config.blitterAccuracy = sender.integerValue
        refresh()
    }

    @IBAction func compTodBugAction(_ sender: NSButton!) {

        config.todBug = sender.state == .on
        refresh()
    }

    @IBAction func compDriveSpeedAction(_ sender: NSPopUpButton!) {

        config.driveSpeed = sender.selectedTag()
        refresh()
    }

    @IBAction func compAsyncFifoAction(_ sender: NSButton!) {

        config.asyncFifo = sender.state == .on
        refresh()
    }

    @IBAction func compLockDskSyncAction(_ sender: NSButton!) {
        
        config.lockDskSync = sender.state == .on
        refresh()
    }
    
    @IBAction func compAutoDskSyncAction(_ sender: NSButton!) {
        
        config.autoDskSync = sender.state == .on
        refresh()
    }

    @IBAction func compAccurateKeyboardAction(_ sender: NSButton!) {

        config.accurateKeyboard = sender.state == .on
        refresh()
    }

    @IBAction func compPresetAction(_ sender: NSPopUpButton!) {
         
         switch sender.selectedTag() {
         case 0: config.loadCompatibilityDefaults(CompatibilityDefaults.std)
         case 1: config.loadCompatibilityDefaults(CompatibilityDefaults.accurate)
         case 2: config.loadCompatibilityDefaults(CompatibilityDefaults.accelerated)
         default: fatalError()
         }
         refresh()
     }

     @IBAction func compDefaultsAction(_ sender: NSButton!) {
         
         config.saveCompatibilityUserDefaults()
     }
}
