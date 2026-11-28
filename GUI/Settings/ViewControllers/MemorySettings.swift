// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class MemorySettingsViewController: SettingsViewController {

    // Chip RAM
    @IBOutlet weak var chipRamPopup: NSPopUpButton!
    @IBOutlet weak var chipRamInfo1: NSTextField!
    @IBOutlet weak var chipRamInfo2: NSTextField!

    // Slow RAM
    @IBOutlet weak var slowRamPopup: NSPopUpButton!
    @IBOutlet weak var slowRamInfo1: NSTextField!
    @IBOutlet weak var slowRamInfo2: NSTextField!
    @IBOutlet weak var slowRamIcon: NSButton!

    // Fast RAM
    @IBOutlet weak var fastRamPopup: NSPopUpButton!
    @IBOutlet weak var fastRamInfo1: NSTextField!
    @IBOutlet weak var fastRamInfo2: NSTextField!
    @IBOutlet weak var fastRamIcon: NSButton!

    // Memory Properties
    @IBOutlet weak var busWidth: NSPopUpButton!
    @IBOutlet weak var ramInitPattern: NSPopUpButton!
    @IBOutlet weak var bankMap: NSPopUpButton!
    @IBOutlet weak var unmappingType: NSPopUpButton!

    // Warning
    @IBOutlet weak var warnImage: NSButton!

    override var showLock: Bool { true }

    override func viewDidLoad() {

        log(.lifetime)
    }

    //
    // Methods from SettingsViewController
    //

    override func refresh() {

        super.refresh()

        refreshToolbar()
        refreshMemory()
    }

    func refreshToolbar() {

        guard let emu = emu else { return }

        // Disable some controls
        presetPopup.isEnabled = emu.poweredOff
    }

    func refreshMemory() {

        guard let emu = emu, let config = config else { return }

        let poweredOff = emu.poweredOff
        let traits = emu.agnus.traits
        let badAgnus = traits.chipRamLimit < config.chipRam

        // Chip RAM
        let chipStart = 0
        let chipEnd = chipStart + config.chipRam * 1024
        chipRamPopup.selectItem(withTag: config.chipRam)
        chipRamInfo1.stringValue = "DRAM"
        chipRamInfo2.stringValue = String(format: "%06X - %06X", chipStart, chipEnd - 1)

        // Slow RAM
        let slowStart = 0xC00000
        let slowEnd = slowStart + config.slowRam * 1024
        slowRamPopup.selectItem(withTag: config.slowRam)
        slowRamInfo1.stringValue = "DRAM"
        slowRamInfo2.stringValue = String(format: "%06X - %06X", slowStart, slowEnd - 1)
        slowRamIcon.isHidden = config.slowRam == 0
        slowRamInfo1.isHidden = config.slowRam == 0
        slowRamInfo2.isHidden = config.slowRam == 0

        // Fast RAM
        let fastStart = 0x200000
        let fastEnd = fastStart + config.fastRam * 1024
        fastRamPopup.selectItem(withTag: config.fastRam)
        fastRamInfo1.stringValue = "DRAM"
        fastRamInfo2.stringValue = String(format: "%06X - %06X", fastStart, fastEnd - 1)
        fastRamIcon.isHidden = config.fastRam == 0
        fastRamInfo1.isHidden = config.fastRam == 0
        fastRamInfo2.isHidden = config.fastRam == 0

        // Memory properties
        busWidth.selectItem(withTag: config.busWidth)
        ramInitPattern.selectItem(withTag: config.ramInitPattern)
        bankMap.selectItem(withTag: config.bankMap)
        unmappingType.selectItem(withTag: config.unmappingType)

        // Disable some controls if emulator is powered on
        chipRamPopup.isEnabled = poweredOff
        slowRamPopup.isEnabled = poweredOff
        fastRamPopup.isEnabled = poweredOff
        ramInitPattern.isEnabled = poweredOff
        bankMap.isEnabled = poweredOff
        unmappingType.isEnabled = poweredOff

        // Memory warning
        warnImage.isHidden = !badAgnus
        warnImage.toolTip =
        "Chip Ram is not fully usable. " +
        "The selected Agnus revision is limited to address \(traits.chipRamLimit) KB."
    }

    override func preset(tag: Int) {

        let defaults = EmulatorProxy.defaults!

        // Modify some settings
        switch tag {

        case 0:

            // Amiga 500
            defaults.set(.MEM_BUS_WIDTH, 16);
            defaults.set(.MEM_CHIP_RAM, 512);
            defaults.set(.MEM_BANKMAP, BankMap.A500.rawValue)

        case 1:

            // Amiga 1000
            defaults.set(.MEM_BUS_WIDTH, 16);
            defaults.set(.MEM_CHIP_RAM, 512);
            defaults.set(.MEM_BANKMAP, BankMap.A1000.rawValue)

        case 2:

            // Amiga 2000
            defaults.set(.MEM_BUS_WIDTH, 16);
            defaults.set(.MEM_CHIP_RAM, 512);
            defaults.set(.MEM_BANKMAP, BankMap.A2000B.rawValue)

        case 3:

            // Amiga 500+
            defaults.set(.MEM_BUS_WIDTH, 16);
            defaults.set(.MEM_CHIP_RAM, 1024);
            defaults.set(.MEM_BANKMAP, BankMap.A500.rawValue)

        case 4:

            // Amiga 1200
            defaults.set(.MEM_BUS_WIDTH, 32);
            defaults.set(.MEM_CHIP_RAM, 2048);
            defaults.set(.MEM_BANKMAP, BankMap.A1200.rawValue)

        default:
            fatalError()
        }

        // Update the configuration
        config?.applyMemoryUserDefaults()
    }

    override func save() {

        log(.lifetime)
        config?.saveMemoryUserDefaults()
    }

    //
    // Action functions
    //

    @IBAction func chipRamAction(_ sender: NSPopUpButton!) {

        config?.chipRam = sender.selectedTag()
    }

    @IBAction func slowRamAction(_ sender: NSPopUpButton!) {

        config?.slowRam = sender.selectedTag()
    }

    @IBAction func fastRamAction(_ sender: NSPopUpButton!) {

        config?.fastRam = sender.selectedTag()
    }

    @IBAction func busWidthAction(_ sender: NSPopUpButton!) {

        config?.busWidth = sender.selectedTag()
    }

    @IBAction func bankMapAction(_ sender: NSPopUpButton!) {

        config?.bankMap = sender.selectedTag()
    }

    @IBAction func unmappingTypeAction(_ sender: NSPopUpButton!) {

        config?.unmappingType = sender.selectedTag()
    }

    @IBAction func ramInitPatternAction(_ sender: NSPopUpButton!) {

        config?.ramInitPattern = sender.selectedTag()
    }
}
