// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class HardwareSettingsViewController: SettingsViewController {

    // CPU
    @IBOutlet weak var cpuRevision: NSPopUpButton!
    @IBOutlet weak var cpuSpeed: NSPopUpButton!
    @IBOutlet weak var cpuInfo1: NSTextField!
    @IBOutlet weak var cpuInfo2: NSTextField!

    // Agnus
    @IBOutlet weak var agnusRevision: NSPopUpButton!
    @IBOutlet weak var machineType: NSPopUpButton!
    @IBOutlet weak var agnusInfo1: NSTextField!
    @IBOutlet weak var agnusInfo2: NSTextField!

    // Denise
    @IBOutlet weak var deniseRevision: NSPopUpButton!
    @IBOutlet weak var deniseInfo1: NSTextField!
    @IBOutlet weak var deniseInfo2: NSTextField!

    // CIAs
    @IBOutlet weak var ciaRevision: NSPopUpButton!
    @IBOutlet weak var ciaInfo1: NSTextField!
    @IBOutlet weak var ciaInfo2: NSTextField!

    // RTC
    @IBOutlet weak var rtcRevision: NSPopUpButton!
    @IBOutlet weak var rtcInfo1: NSTextField!
    @IBOutlet weak var rtcInfo2: NSTextField!
    @IBOutlet weak var rtcIcon: NSButton!

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
    @IBOutlet weak var ramInitPattern: NSPopUpButton!
    @IBOutlet weak var bankMap: NSPopUpButton!
    @IBOutlet weak var unmappingType: NSPopUpButton!

    // Warning
    @IBOutlet weak var warnImage: NSButton!
    
    override func viewDidLoad() {

        log(.lifetime)
    }

    //
    // Methods from SettingsViewController
    //

    /*
    var bootable: Bool {

        do { try emu?.isReady() } catch { return false }
        return emu?.poweredOff ?? false
    }
    */

    override func refresh() {

        super.refresh()

        refreshChipset()
        refreshMemory()
    }

    func refreshChipset() {

        guard let emu = emu, let config = config else { return }

        let poweredOff = emu.poweredOff
        let pal = config.machineType == VideoFormat.PAL.rawValue

        // CPU
        cpuRevision.selectItem(withTag: config.cpuRev)
        cpuSpeed.selectItem(withTag: config.cpuSpeed)
        switch CPURevision(rawValue: config.cpuRev) {

        case ._68000:
            cpuInfo1.stringValue = "Motorola MC68000"
            cpuInfo2.stringValue = "Original Amigas"

        case ._68010:
            cpuInfo1.stringValue = "Motorola MC68010"
            cpuInfo2.stringValue = ""

        case ._68EC020:
            cpuInfo1.stringValue = "Motorola 68EC020"
            cpuInfo2.stringValue = "A1200, A2500"

        default:
            cpuInfo1.stringValue = "Invalid"
            cpuInfo2.stringValue = ""
        }

        // Agnus
        machineType.selectItem(withTag: config.machineType)
        agnusRevision.selectItem(withTag: config.agnusRev)
        switch AgnusRevision(rawValue: config.agnusRev) {

        case .OCS_OLD:
            agnusInfo1.stringValue = pal ? "MOS 8367" : "MOS 8361"
            agnusInfo2.stringValue = "A1000, A2000A"

        case .OCS:
            agnusInfo1.stringValue = pal ? "MOS 8371" : "MOS 8370"
            agnusInfo2.stringValue = "Early A500, A2000"

        case .ECS_1MB:
            agnusInfo1.stringValue = "MOS 8372A"
            agnusInfo2.stringValue = "Later A500, A2000"

        case .ECS_2MB:
            agnusInfo1.stringValue = "MOS 8375"
            agnusInfo2.stringValue = "A500+, A600"

        default:
            agnusInfo1.stringValue = "Invalid"
            agnusInfo2.stringValue = ""
        }

        // Denise
        deniseRevision.selectItem(withTag: config.deniseRev)
        switch DeniseRevision(rawValue: config.deniseRev) {

        case .OCS:
            deniseInfo1.stringValue = "MOS 8362R8"
            deniseInfo2.stringValue = "A500, A1000, A2000"

        case .ECS:
            deniseInfo1.stringValue = "MOS 8373R4"
            deniseInfo2.stringValue = "A500+, A600"

        default:
            deniseInfo1.stringValue = "Invalid"
            deniseInfo2.stringValue = ""
        }

        // CIAs
        ciaRevision.selectItem(withTag: config.ciaRev)
        switch CIARevision(rawValue: config.ciaRev) {

        case .MOS_8520_DIP:
            ciaInfo1.stringValue = "MOS 8520"
            ciaInfo2.stringValue = "A500, A1000, A2000, A500+"

        case .MOS_8520_PLCC:
            ciaInfo1.stringValue = "MOS 8520PL"
            ciaInfo2.stringValue = "A600"

        default:
            ciaInfo1.stringValue = "Invalid"
            ciaInfo2.stringValue = ""
        }

        // RTC
        rtcRevision.selectItem(withTag: config.rtClock)
        switch RTCRevision(rawValue: config.rtClock) {

        case .NONE:
            rtcInfo1.stringValue = ""
            rtcInfo2.stringValue = ""
            rtcIcon.isHidden = true

        case .OKI:
            rtcInfo1.stringValue = "MSM6242B"
            rtcInfo2.stringValue = "A2000, A500+"
            rtcIcon.isHidden = false

        case .RICOH:
            rtcInfo1.stringValue = "RF5C01A"
            rtcInfo2.stringValue = "A3000, A4000"
            rtcIcon.isHidden = false

        default:
            rtcInfo1.stringValue = "Invalid"
            rtcInfo2.stringValue = ""
            rtcIcon.isHidden = true
        }

        // Disable some controls if emulator is powered on
        agnusRevision.isEnabled = poweredOff
        deniseRevision.isEnabled = poweredOff
        ciaRevision.isEnabled = poweredOff
        rtcRevision.isEnabled = poweredOff
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

        // Revert to standard settings
        EmulatorProxy.defaults.removeChipsetUserDefaults()

        // Modify some settings
        switch tag {

        case 0:

            // Amiga 500
            defaults.set(.AGNUS_REVISION, AgnusRevision.ECS_1MB.rawValue)
            defaults.set(.DENISE_REVISION, DeniseRevision.OCS.rawValue)
            defaults.set(.RTC_MODEL, RTCRevision.NONE.rawValue)

        case 1:

            // Amiga 1000
            defaults.set(.AGNUS_REVISION, AgnusRevision.OCS_OLD.rawValue)
            defaults.set(.DENISE_REVISION, DeniseRevision.OCS.rawValue)
            defaults.set(.RTC_MODEL, RTCRevision.NONE.rawValue)

        case 2:

            // Amiga 2000
            defaults.set(.AGNUS_REVISION, AgnusRevision.ECS_2MB.rawValue)
            defaults.set(.DENISE_REVISION, DeniseRevision.OCS.rawValue)
            defaults.set(.RTC_MODEL, RTCRevision.OKI.rawValue)

        case 3:

            // Amiga 500+
            defaults.set(.AGNUS_REVISION, AgnusRevision.ECS_2MB.rawValue)
            defaults.set(.DENISE_REVISION, DeniseRevision.ECS.rawValue)
            defaults.set(.RTC_MODEL, RTCRevision.OKI.rawValue)

        default:
            fatalError()
        }

        // Update the configuration
        config?.applyChipsetUserDefaults()
    }

    override func save() {

        log(.lifetime)
        config?.saveChipsetUserDefaults()
    }

    //
    // Action function (Chipset)
    //

    @IBAction func cpuRevAction(_ sender: NSPopUpButton!) {

        config?.cpuRev = sender.selectedTag()
    }

    @IBAction func cpuSpeedAction(_ sender: NSPopUpButton!) {

        config?.cpuSpeed = sender.selectedTag()
    }

    @IBAction func agnusRevAction(_ sender: NSPopUpButton!) {

        config?.agnusRev = sender.selectedTag()
    }

    @IBAction func machineTypeAction(_ sender: NSPopUpButton!) {

        config?.machineType = sender.selectedTag()
    }

    @IBAction func deniseRevAction(_ sender: NSPopUpButton!) {

        config?.deniseRev = sender.selectedTag()
    }

    @IBAction func ciaRevAction(_ sender: NSPopUpButton!) {

        config?.ciaRev = sender.selectedTag()
    }

    @IBAction func realTimeClockAction(_ sender: NSPopUpButton!) {

        config?.rtClock = sender.selectedTag()
    }

    //
    // Action function (Memory)
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
