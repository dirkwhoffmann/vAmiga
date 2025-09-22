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
    @IBOutlet weak var csCpuRevision: NSPopUpButton!
    @IBOutlet weak var csCpuSpeed: NSPopUpButton!
    @IBOutlet weak var csCpuInfo1: NSTextField!
    @IBOutlet weak var csCpuInfo2: NSTextField!

    // Agnus
    @IBOutlet weak var csAgnusRevision: NSPopUpButton!
    @IBOutlet weak var csMachineType: NSPopUpButton!
    @IBOutlet weak var csAgnusInfo1: NSTextField!
    @IBOutlet weak var csAgnusInfo2: NSTextField!

    // Denise
    @IBOutlet weak var csDeniseRevision: NSPopUpButton!
    @IBOutlet weak var csDeniseInfo1: NSTextField!
    @IBOutlet weak var csDeniseInfo2: NSTextField!

    // CIAs
    @IBOutlet weak var csCiaRevision: NSPopUpButton!
    @IBOutlet weak var csCiaInfo1: NSTextField!
    @IBOutlet weak var csCiaInfo2: NSTextField!

    // RTC
    @IBOutlet weak var csRtcRevision: NSPopUpButton!
    @IBOutlet weak var csRtcInfo1: NSTextField!
    @IBOutlet weak var csRtcInfo2: NSTextField!
    @IBOutlet weak var csRtcIcon: NSButton!

    // RAM
    @IBOutlet weak var memChipRamPopup: NSPopUpButton!
    @IBOutlet weak var memSlowRamPopup: NSPopUpButton!
    @IBOutlet weak var memFastRamPopup: NSPopUpButton!

    // Memory handling
    @IBOutlet weak var memRamInitPattern: NSPopUpButton!
    @IBOutlet weak var memBankMap: NSPopUpButton!
    @IBOutlet weak var memUnmappingType: NSPopUpButton!

    // Warning
    @IBOutlet weak var memWarnImage: NSButton!
    
    override func viewDidLoad() {

        print("HardwareSettingsViewController::viewDidLoad")
    }

    //
    // Refresh
    //

    var bootable: Bool {

        do { try emu?.isReady() } catch { return false }
        return emu?.poweredOff ?? false
    }

    override func refresh() {

        super.refresh()

        guard let emu = emu, let config = config else { return }

        let poweredOff = emu.poweredOff
        let pal = config.machineType == VideoFormat.PAL.rawValue

        // CPU
        csCpuRevision.selectItem(withTag: config.cpuRev)
        csCpuSpeed.selectItem(withTag: config.cpuSpeed)
        switch CPURevision(rawValue: config.cpuRev) {

        case ._68000:
            csCpuInfo1.stringValue = "Motorola MC68000"
            csCpuInfo2.stringValue = "Original Amigas"

        case ._68010:
            csCpuInfo1.stringValue = "Motorola MC68010"
            csCpuInfo2.stringValue = ""

        case ._68EC020:
            csCpuInfo1.stringValue = "Motorola 68EC020"
            csCpuInfo2.stringValue = "A1200, A2500"

        default:
            csCpuInfo1.stringValue = "Invalid"
            csCpuInfo2.stringValue = ""
        }

        // Agnus
        csMachineType.selectItem(withTag: config.machineType)
        csAgnusRevision.selectItem(withTag: config.agnusRev)
        switch AgnusRevision(rawValue: config.agnusRev) {

        case .OCS_OLD:
            csAgnusInfo1.stringValue = pal ? "MOS 8367" : "MOS 8361"
            csAgnusInfo2.stringValue = "A1000, A2000A"

        case .OCS:
            csAgnusInfo1.stringValue = pal ? "MOS 8371" : "MOS 8370"
            csAgnusInfo2.stringValue = "Early A500, A2000"

        case .ECS_1MB:
            csAgnusInfo1.stringValue = "MOS 8372A"
            csAgnusInfo2.stringValue = "Later A500, A2000"

        case .ECS_2MB:
            csAgnusInfo1.stringValue = "MOS 8375"
            csAgnusInfo2.stringValue = "A500+, A600"

        default:
            csAgnusInfo1.stringValue = "Invalid"
            csAgnusInfo2.stringValue = ""
        }

        // Denise
        csDeniseRevision.selectItem(withTag: config.deniseRev)
        switch DeniseRevision(rawValue: config.deniseRev) {

        case .OCS:
            csDeniseInfo1.stringValue = "MOS 8362R8"
            csDeniseInfo2.stringValue = "A500, A1000, A2000"

        case .ECS:
            csDeniseInfo1.stringValue = "MOS 8373R4"
            csDeniseInfo2.stringValue = "A500+, A600"

        default:
            csDeniseInfo1.stringValue = "Invalid"
            csDeniseInfo2.stringValue = ""
        }

        // CIAs
        csCiaRevision.selectItem(withTag: config.ciaRev)
        switch CIARevision(rawValue: config.ciaRev) {

        case .MOS_8520_DIP:
            csCiaInfo1.stringValue = "MOS 8520"
            csCiaInfo2.stringValue = "A500, A1000, A2000, A500+"

        case .MOS_8520_PLCC:
            csCiaInfo1.stringValue = "MOS 8520PL"
            csCiaInfo2.stringValue = "A600"

        default:
            csCiaInfo1.stringValue = "Invalid"
            csCiaInfo2.stringValue = ""
        }

        // RTC
        csRtcRevision.selectItem(withTag: config.rtClock)
        switch RTCRevision(rawValue: config.rtClock) {

        case .NONE:
            csRtcInfo1.stringValue = ""
            csRtcInfo2.stringValue = ""
            csRtcIcon.isHidden = true

        case .OKI:
            csRtcInfo1.stringValue = "MSM6242B"
            csRtcInfo2.stringValue = "A2000, A500+"
            csRtcIcon.isHidden = false

        case .RICOH:
            csRtcInfo1.stringValue = "RF5C01A"
            csRtcInfo2.stringValue = "A3000, A4000"
            csRtcIcon.isHidden = false

        default:
            csRtcInfo1.stringValue = "Invalid"
            csRtcInfo2.stringValue = ""
            csRtcIcon.isHidden = true
        }

        // Disable some controls if emulator is powered on
        csAgnusRevision.isEnabled = poweredOff
        csDeniseRevision.isEnabled = poweredOff
        csCiaRevision.isEnabled = poweredOff
        csRtcRevision.isEnabled = poweredOff

        // Lock
        /*
        hwLockImage.isHidden = emu.poweredOff
        hwLockInfo1.isHidden = emu.poweredOff
        hwLockInfo2.isHidden = emu.poweredOff
        */

        // Buttons
        // hwPowerButton.isHidden = !bootable
    }

    @IBAction func hwPresetAction(_ sender: NSPopUpButton!) {

        csPresetAction(sender)
    }

    /*
    @IBAction func hwDefaultsAction(_ sender: NSButton!) {

        csDefaultsAction(sender)
    }
    */

    @IBAction func csCpuRevAction(_ sender: NSPopUpButton!) {

        config?.cpuRev = sender.selectedTag()
    }

    @IBAction func csCpuSpeedAction(_ sender: NSPopUpButton!) {

        config?.cpuSpeed = sender.selectedTag()
    }

    @IBAction func csAgnusRevAction(_ sender: NSPopUpButton!) {

        config?.agnusRev = sender.selectedTag()
    }

    @IBAction func csMachineTypeAction(_ sender: NSPopUpButton!) {

        config?.machineType = sender.selectedTag()
    }

    @IBAction func csDeniseRevAction(_ sender: NSPopUpButton!) {

        config?.deniseRev = sender.selectedTag()
    }

    @IBAction func csCiaRevAction(_ sender: NSPopUpButton!) {

        config?.ciaRev = sender.selectedTag()
    }

    @IBAction func csRealTimeClockAction(_ sender: NSPopUpButton!) {

        config?.rtClock = sender.selectedTag()
    }

    @IBAction func csPresetAction(_ sender: NSPopUpButton!) {

        let defaults = EmulatorProxy.defaults!

        // Revert to standard settings
        EmulatorProxy.defaults.removeChipsetUserDefaults()

        // Modify some settings
        switch sender.selectedTag() {

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

    //
    // Action methods
    //

    override func preset(tag: Int) {

        print("HardwareSettingsViewController::preset")
    }

    override func save() {

        print("HardwareSettingsViewController::preset")
        config?.saveChipsetUserDefaults()
    }
}
