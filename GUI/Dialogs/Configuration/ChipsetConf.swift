// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ConfigurationController {
    
    func refreshChipsetTab() {

        let poweredOff = amiga.poweredOff
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

        case ._68020:
            csCpuInfo1.stringValue = "Motorola MC68020"
            csCpuInfo2.stringValue = ""

            /*
        case ._68EC030:
            csCpuInfo1.stringValue = "Motorola 68EC030"
            csCpuInfo2.stringValue = ""

        case ._68030:
            csCpuInfo1.stringValue = "Motorola MC68030"
            csCpuInfo2.stringValue = ""

        case ._68EC040:
            csCpuInfo1.stringValue = "Motorola 68EC040"
            csCpuInfo2.stringValue = ""

        case ._68LC040:
            csCpuInfo1.stringValue = "Motorola 68LC040"
            csCpuInfo2.stringValue = ""

        case ._68040:
            csCpuInfo1.stringValue = "Motorola 68040"
            csCpuInfo2.stringValue = ""
             */
            
        default:
            csCpuInfo1.stringValue = "Invalid"
            csCpuInfo2.stringValue = ""
        }

        /*
        let mfreq = Double(amiga.masterFrequency) / 1000000
        for i in 0 ... 6 {
            let tag = Double(csCpuSpeed.item(at: i)!.tag)
            let hz = (tag == 0 ? 1 : tag) * mfreq / 4
            csCpuSpeed.item(at: i)?.title = String(format: "%.0f", hz) + " MHz"
        }
        */

        // Oscillator
        csOscillator.selectItem(withTag: config.vsync)
        let hz = Double(pal ? CLK_FREQUENCY_PAL : CLK_FREQUENCY_NTSC) / 1000000
        csOscillator.item(at: 0)?.title = String(format: "%.2f", hz) + " MHz"

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
        // csCpuRevision.isEnabled = poweredOff
        csAgnusRevision.isEnabled = poweredOff
        csDeniseRevision.isEnabled = poweredOff
        csCiaRevision.isEnabled = poweredOff
        csRtcRevision.isEnabled = poweredOff
        csFactorySettingsPopup.isEnabled = poweredOff

        // Lock
        csLockImage.isHidden = poweredOff
        csLockInfo1.isHidden = poweredOff
        csLockInfo2.isHidden = poweredOff

        // Buttons
        csPowerButton.isHidden = !bootable
    }

    @IBAction func csCpuRevAction(_ sender: NSPopUpButton!) {

        config.cpuRev = sender.selectedTag()
        refresh()
    }

    @IBAction func csCpuSpeedAction(_ sender: NSPopUpButton!) {

        config.cpuSpeed = sender.selectedTag()
        refresh()
    }

    @IBAction func csOscillatorAction(_ sender: NSPopUpButton!) {

        config.vsync = sender.selectedTag()
        refresh()
    }

    @IBAction func csAgnusRevAction(_ sender: NSPopUpButton!) {

        config.agnusRev = sender.selectedTag()
        refresh()
    }

    @IBAction func csMachineTypeAction(_ sender: NSPopUpButton!) {

        config.machineType = sender.selectedTag()
        refresh()
    }

    @IBAction func csDeniseRevAction(_ sender: NSPopUpButton!) {

        config.deniseRev = sender.selectedTag()
        refresh()
    }
    
    @IBAction func csCiaRevAction(_ sender: NSPopUpButton!) {

        config.ciaRev = sender.selectedTag()
        refresh()
    }

    @IBAction func csRealTimeClockAction(_ sender: NSPopUpButton!) {
        
        config.rtClock = sender.selectedTag()
        refresh()
    }
    
    @IBAction func csPresetAction(_ sender: NSPopUpButton!) {
        
        let defaults = AmigaProxy.defaults!
        
        // Revert to standard settings
        AmigaProxy.defaults.removeChipsetUserDefaults() 
        
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
        config.applyChipsetUserDefaults()

        refresh()
    }

    @IBAction func csDefaultsAction(_ sender: NSButton!) {
        
        config.saveChipsetUserDefaults()
    }
}
