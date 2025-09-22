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

        log(.lifetime)
    }

    //
    // Methods from SettingsViewController
    //

    override func refresh() {

        super.refresh()

        guard let config = config else { return }

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

        // Collision detection
        compClxSprSpr.state = config.clxSprSpr ? .on : .off
        compClxSprPlf.state = config.clxSprPlf ? .on : .off
        compClxPlfPlf.state = config.clxPlfPlf ? .on : .off
    }

    override func preset(tag: Int) {

        let defaults = EmulatorProxy.defaults!

        // Revert to standard settings
        EmulatorProxy.defaults.removeCompatibilityUserDefaults()

        // Override some options
        switch tag {

        case 0:

            // Standard
            break

        case 1:

            // Accurate
            defaults.set(.DENISE_CLX_SPR_PLF, true)
            defaults.set(.DENISE_CLX_SPR_SPR, true)
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
        config?.applyCompatibilityUserDefaults()
    }

    override func save() {

        config?.saveCompatibilityUserDefaults()
    }

    //
    // Action methods
    //

    @IBAction func compBltAccuracyAction(_ sender: NSSlider!) {

        config?.blitterAccuracy = sender.integerValue
    }

    @IBAction func compSlowRamDelayAction(_ sender: NSButton!) {

        config?.slowRamDelay = sender.state == .on
    }

    @IBAction func compSlowRamMirrorAction(_ sender: NSButton!) {

        config?.slowRamMirror = sender.state == .on
    }

    @IBAction func compTodBugAction(_ sender: NSButton!) {

        config?.todBug = sender.state == .on
    }

    @IBAction func compPtrDropAction(_ sender: NSButton!) {

        config?.ptrDrops = sender.state == .on
    }

    @IBAction func compDriveSpeedAction(_ sender: NSPopUpButton!) {

        config?.driveSpeed = sender.selectedTag()
    }

    @IBAction func compMechanicsAction(_ sender: NSButton!) {

        config?.driveMechanics = sender.state == .on ? 1 : 0
    }

    @IBAction func compLockDskSyncAction(_ sender: NSButton!) {

        config?.lockDskSync = sender.state == .on
    }

    @IBAction func compAutoDskSyncAction(_ sender: NSButton!) {

        config?.autoDskSync = sender.state == .on
    }

    @IBAction func compEClockSyncingAction(_ sender: NSButton!) {

        config?.eClockSyncing = sender.state == .on
    }

    @IBAction func compAccurateKeyboardAction(_ sender: NSButton!) {

        config?.accurateKeyboard = sender.state == .on
    }

    //
    // Action methods (collision detection)
    //

    @IBAction func compClxSprSprAction(_ sender: NSButton!) {

        config?.clxSprSpr = sender.state == .on
    }

    @IBAction func compClxSprPlfAction(_ sender: NSButton!) {

        config?.clxSprPlf = sender.state == .on
    }

    @IBAction func compClxPlfPlfAction(_ sender: NSButton!) {

        config?.clxPlfPlf = sender.state == .on
    }
}
