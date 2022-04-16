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

        // Graphics
        compClxSprSpr.state = config.clxSprSpr ? .on : .off
        compClxSprPlf.state = config.clxSprPlf ? .on : .off
        compClxPlfPlf.state = config.clxPlfPlf ? .on : .off

        // Blitter
        let level = config.blitterAccuracy
        compBltAccuracy.integerValue = level
        compBltLevel1.textColor = (level >= 1) ? .labelColor : .tertiaryLabelColor
        compBltLevel2.textColor = (level >= 2) ? .labelColor : .tertiaryLabelColor
        
        // Chipset features
        compTodBug.state = config.todBug ? .on : .off

        // Floppy drives
        let speed = config.driveSpeed
        compDriveSpeed.selectItem(withTag: Int(speed))
        compMechanicalDelays.state = config.mechanicalDelays ? .on : .off
        compLockDskSync.state = config.lockDskSync ? .on : .off
        compAutoDskSync.state = config.autoDskSync ? .on : .off

        // Timing
        compEClockSyncing.state = config.eClockSyncing ? .on : .off

        // Keyboard
        compAccurateKeyboard.state = config.accurateKeyboard ? .on : .off

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

    @IBAction func compMechanicalDelaysAction(_ sender: NSButton!) {

        config.mechanicalDelays = sender.state == .on
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

    @IBAction func compEClockSyncingAction(_ sender: NSButton!) {
        
        config.eClockSyncing = sender.state == .on
        refresh()
    }

    @IBAction func compAccurateKeyboardAction(_ sender: NSButton!) {

        config.accurateKeyboard = sender.state == .on
        refresh()
    }

    @IBAction func compPresetAction(_ sender: NSPopUpButton!) {
        
        assert(amiga.poweredOff)
        let defaults = AmigaProxy.defaults!
                
        // Revert to standard settings
        config.removeCompatibilityUserDefaults()
                
        // Override some options
        switch sender.selectedTag() {
            
        case 0:
            
            // Standard
            break
            
        case 1:
            
            // Accurate
            defaults.set(.CLX_SPR_SPR, value: true)
            defaults.set(.CLX_SPR_PLF, value: true)
            defaults.set(.CLX_PLF_PLF, value: true)
            
        case 2:
            
            // Accelerated
            defaults.set(.BLITTER_ACCURACY, value: 0)
            defaults.set(.DRIVE_SPEED, value: -1)
            defaults.set(.EMULATE_MECHANICS, value: false)
            defaults.set(.ACCURATE_KEYBOARD, value: false)
            defaults.set(.ECLOCK_SYNCING, value: false)
            
        default:
            fatalError()
        }
        
        // Update the configutation
        config.updateCompatibilityUserDefaults()
        refresh()
    }
    
    @IBAction func compDefaultsAction(_ sender: NSButton!) {
        
        config.saveCompatibilityUserDefaults()
    }
}
