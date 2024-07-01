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

        // Blitter
        let level = config.blitterAccuracy
        compBltAccuracy.integerValue = level
        compBltLevel1.textColor = (level >= 1) ? .labelColor : .tertiaryLabelColor
        compBltLevel2.textColor = (level >= 2) ? .labelColor : .tertiaryLabelColor
        
        // Chipset features
        compTodBug.state = config.todBug ? .on : .off
        compPtrDrops.state = config.ptrDrops ? .on : .off

        // Floppy drives
        let speed = config.driveSpeed
        compDriveSpeed.selectItem(withTag: speed)
        compMechanics.state = config.driveMechanics != 0 ? .on : .off
        compLockDskSync.state = config.lockDskSync ? .on : .off
        compAutoDskSync.state = config.autoDskSync ? .on : .off

        // Timing
        compEClockSyncing.state = config.eClockSyncing ? .on : .off

        // Keyboard
        compAccurateKeyboard.state = config.accurateKeyboard ? .on : .off

        // Buttons
        compPowerButton.isHidden = !bootable
    }

    @IBAction func compBltAccuracyAction(_ sender: NSSlider!) {

        config.blitterAccuracy = sender.integerValue
        refresh()
    }
        
    @IBAction func compSlowRamDelayAction(_ sender: NSButton!) {

        config.slowRamDelay = sender.state == .on
        refresh()
    }

    @IBAction func compSlowRamMirrorAction(_ sender: NSButton!) {

        config.slowRamMirror = sender.state == .on
        refresh()
    }

    @IBAction func compTodBugAction(_ sender: NSButton!) {

        config.todBug = sender.state == .on
        refresh()
    }

    @IBAction func compPtrDropAction(_ sender: NSButton!) {

        config.ptrDrops = sender.state == .on
        refresh()
    }

    @IBAction func compDriveSpeedAction(_ sender: NSPopUpButton!) {

        config.driveSpeed = sender.selectedTag()
        refresh()
    }

    @IBAction func compMechanicsAction(_ sender: NSButton!) {

        config.driveMechanics = sender.state == .on ? 1 : 0
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
        
        let defaults = EmulatorProxy.defaults!
                
        // Revert to standard settings
        EmulatorProxy.defaults.removeCompatibilityUserDefaults()
                
        // Override some options
        switch sender.selectedTag() {
            
        case 0:
            
            // Standard
            break
            
        case 1:
            
            // Accurate
            defaults.set(.DENISE_CLX_SPR_SPR, true)
            defaults.set(.DENISE_CLX_SPR_PLF, true)
            defaults.set(.DENISE_CLX_PLF_PLF, true)

        case 2:
            
            // Accelerated
            defaults.set(.BLITTER_ACCURACY, 0)
            defaults.set(.DC_SPEED, -1)
            defaults.set(.DRIVE_MECHANICS, [0, 1, 2, 3], 1)
            defaults.set(.KBD_ACCURACY, false)
            defaults.set(.CIA_ECLOCK_SYNCING, [0, 1], false)

        default:
            fatalError()
        }
        
        // Update the configutation
        config.applyCompatibilityUserDefaults()
        refresh()
    }
    
    @IBAction func compDefaultsAction(_ sender: NSButton!) {
        
        config.saveCompatibilityUserDefaults()
    }
}
