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
    @IBOutlet weak var bltAccuracy: NSSlider!
    @IBOutlet weak var bltLevel2: NSTextField!
    @IBOutlet weak var bltLevel1: NSTextField!

    // Chipset features
    @IBOutlet weak var slowRamMirror: NSButton!
    @IBOutlet weak var slowRamDelay: NSButton!
    @IBOutlet weak var todBug: NSButton!
    @IBOutlet weak var ptrDrops: NSButton!

    // Timing
    @IBOutlet weak var eClockSyncing: NSButton!

    // Disk controller
    @IBOutlet weak var driveSpeed: NSPopUpButton!
    @IBOutlet weak var mechanics: NSButton!
    @IBOutlet weak var lockDskSync: NSButton!
    @IBOutlet weak var autoDskSync: NSButton!

    // Keyboard
    @IBOutlet weak var accurateKeyboard: NSButton!

    // Collision detection
    @IBOutlet weak var clxSprSpr: NSButton!
    @IBOutlet weak var clxSprPlf: NSButton!
    @IBOutlet weak var clxPlfPlf: NSButton!

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
        bltAccuracy.integerValue = level
        bltLevel1.textColor = (level >= 1) ? .labelColor : .tertiaryLabelColor
        bltLevel2.textColor = (level >= 2) ? .labelColor : .tertiaryLabelColor

        // Chipset features
        todBug.state = config.todBug ? .on : .off
        ptrDrops.state = config.ptrDrops ? .on : .off

        // Floppy drives
        let speed = config.driveSpeed
        driveSpeed.selectItem(withTag: speed)
        mechanics.state = config.driveMechanics != 0 ? .on : .off
        lockDskSync.state = config.lockDskSync ? .on : .off
        autoDskSync.state = config.autoDskSync ? .on : .off

        // Timing
        eClockSyncing.state = config.eClockSyncing ? .on : .off

        // Keyboard
        accurateKeyboard.state = config.accurateKeyboard ? .on : .off

        // Collision detection
        clxSprSpr.state = config.clxSprSpr ? .on : .off
        clxSprPlf.state = config.clxSprPlf ? .on : .off
        clxPlfPlf.state = config.clxPlfPlf ? .on : .off
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

    @IBAction func bltAccuracyAction(_ sender: NSSlider!) {

        config?.blitterAccuracy = sender.integerValue
    }

    @IBAction func slowRamDelayAction(_ sender: NSButton!) {

        config?.slowRamDelay = sender.state == .on
    }

    @IBAction func slowRamMirrorAction(_ sender: NSButton!) {

        config?.slowRamMirror = sender.state == .on
    }

    @IBAction func todBugAction(_ sender: NSButton!) {

        config?.todBug = sender.state == .on
    }

    @IBAction func ptrDropAction(_ sender: NSButton!) {

        config?.ptrDrops = sender.state == .on
    }

    @IBAction func driveSpeedAction(_ sender: NSPopUpButton!) {

        config?.driveSpeed = sender.selectedTag()
    }

    @IBAction func mechanicsAction(_ sender: NSButton!) {

        config?.driveMechanics = sender.state == .on ? 1 : 0
    }

    @IBAction func lockDskSyncAction(_ sender: NSButton!) {

        config?.lockDskSync = sender.state == .on
    }

    @IBAction func autoDskSyncAction(_ sender: NSButton!) {

        config?.autoDskSync = sender.state == .on
    }

    @IBAction func eClockSyncingAction(_ sender: NSButton!) {

        config?.eClockSyncing = sender.state == .on
    }

    @IBAction func accurateKeyboardAction(_ sender: NSButton!) {

        config?.accurateKeyboard = sender.state == .on
    }

    //
    // Action methods (collision detection)
    //

    @IBAction func clxSprSprAction(_ sender: NSButton!) {

        config?.clxSprSpr = sender.state == .on
    }

    @IBAction func clxSprPlfAction(_ sender: NSButton!) {

        config?.clxSprPlf = sender.state == .on
    }

    @IBAction func clxPlfPlfAction(_ sender: NSButton!) {

        config?.clxPlfPlf = sender.state == .on
    }
}
