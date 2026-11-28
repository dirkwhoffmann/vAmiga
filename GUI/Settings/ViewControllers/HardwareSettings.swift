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
    @IBOutlet weak var cpRevision: NSPopUpButton!
    @IBOutlet weak var cpLabel: NSTextField!
    
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
        refreshChipset()
    }

    func refreshToolbar() {
       
        guard let emu = emu else { return }
                
        // Disable some controls
        presetPopup.isEnabled = emu.poweredOff
    }
    
    func refreshChipset() {

        guard let emu = emu, let config = config else { return }

        let poweredOff = emu.poweredOff
        let pal = config.machineType == VideoFormat.PAL.rawValue

        // CPU
        cpuRevision.selectItem(withTag: config.cpuRev)
        cpuSpeed.selectItem(withTag: config.cpuSpeed)
        
        // Coprocessor
        cpRevision.selectItem(withTag: config.copro)
        
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

    override func preset(tag: Int) {

        let defaults = EmulatorProxy.defaults!

        // Revert to standard settings
        EmulatorProxy.defaults.removeChipsetUserDefaults()

        // Modify some settings
        switch tag {

        case 0:

            // Amiga 500
            defaults.set(.CPU_REVISION, CPURevision._68000.rawValue)
            defaults.set(.CPU_OVERCLOCKING, 0)
            defaults.set(.AGNUS_REVISION, AgnusRevision.ECS_1MB.rawValue)
            defaults.set(.DENISE_REVISION, DeniseRevision.OCS.rawValue)
            defaults.set(.RTC_MODEL, RTCRevision.NONE.rawValue)

        case 1:

            // Amiga 1000
            defaults.set(.CPU_REVISION, CPURevision._68000.rawValue)
            defaults.set(.CPU_OVERCLOCKING, 0)
            defaults.set(.AGNUS_REVISION, AgnusRevision.OCS_OLD.rawValue)
            defaults.set(.DENISE_REVISION, DeniseRevision.OCS.rawValue)
            defaults.set(.RTC_MODEL, RTCRevision.NONE.rawValue)

        case 2:

            // Amiga 2000
            defaults.set(.CPU_REVISION, CPURevision._68000.rawValue)
            defaults.set(.CPU_OVERCLOCKING, 0)
            defaults.set(.AGNUS_REVISION, AgnusRevision.ECS_2MB.rawValue)
            defaults.set(.DENISE_REVISION, DeniseRevision.OCS.rawValue)
            defaults.set(.RTC_MODEL, RTCRevision.OKI.rawValue)

        case 3:

            // Amiga 500+
            defaults.set(.CPU_REVISION, CPURevision._68000.rawValue)
            defaults.set(.CPU_OVERCLOCKING, 0)
            defaults.set(.AGNUS_REVISION, AgnusRevision.ECS_2MB.rawValue)
            defaults.set(.DENISE_REVISION, DeniseRevision.ECS.rawValue)
            defaults.set(.RTC_MODEL, RTCRevision.OKI.rawValue)

        case 4:

            // Amiga 1200
            defaults.set(.CPU_REVISION, CPURevision._68EC020.rawValue)
            defaults.set(.CPU_OVERCLOCKING, 2)
            defaults.set(.AGNUS_REVISION, AgnusRevision.AGA.rawValue)
            defaults.set(.DENISE_REVISION, DeniseRevision.AGA.rawValue)
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
    // Action functions
    //

    @IBAction func cpuRevAction(_ sender: NSPopUpButton!) {

        config?.cpuRev = sender.selectedTag()
    }

    @IBAction func cpRevAction(_ sender: NSPopUpButton!) {

        config?.copro = sender.selectedTag()
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
}
