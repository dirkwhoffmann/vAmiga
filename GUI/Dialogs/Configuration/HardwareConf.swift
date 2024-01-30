// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ConfigurationController {

    func refreshHardwareTab() {

        refreshChipsetTab()
        refreshMemoryTab()

        // Lock
        hwLockImage.isHidden = amiga.poweredOff
        hwLockInfo1.isHidden = amiga.poweredOff
        hwLockInfo2.isHidden = amiga.poweredOff

        // Buttons
        hwPowerButton.isHidden = !bootable
    }

    @IBAction func hwPresetAction(_ sender: NSPopUpButton!) {

        csPresetAction(sender)
        memPresetAction(sender)
    }

    @IBAction func hwDefaultsAction(_ sender: NSButton!) {

        csDefaultsAction(sender)
        memDefaultsAction(sender)
    }

    //
    // Chipset panel
    //

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
        hwFactorySettingsPopup.isEnabled = poweredOff
    }

    @IBAction func csCpuRevAction(_ sender: NSPopUpButton!) {

        config.cpuRev = sender.selectedTag()
        refresh()
    }

    @IBAction func csCpuSpeedAction(_ sender: NSPopUpButton!) {

        config.cpuSpeed = sender.selectedTag()
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

    //
    // Memory panel
    //

    func refreshMemoryTab() {

        let poweredOff = amiga.poweredOff

        // Memory
        memChipRamPopup.selectItem(withTag: config.chipRam)
        memSlowRamPopup.selectItem(withTag: config.slowRam)
        memFastRamPopup.selectItem(withTag: config.fastRam)
        memRamInitPattern.selectItem(withTag: config.ramInitPattern)
        memBankMap.selectItem(withTag: config.bankMap)
        memUnmappingType.selectItem(withTag: config.unmappingType)

        // Disable some controls if emulator is powered on
        memChipRamPopup.isEnabled = poweredOff
        memSlowRamPopup.isEnabled = poweredOff
        memFastRamPopup.isEnabled = poweredOff
        memRamInitPattern.isEnabled = poweredOff
        memBankMap.isEnabled = poweredOff
        memUnmappingType.isEnabled = poweredOff

        // Chipset features
        compSlowRamDelay.state = config.slowRamDelay ? .on : .off
        compSlowRamMirror.state = config.slowRamMirror ? .on : .off
        compSlowRamMirror.isEnabled = amiga.agnus.isECS

        // Warning
        let badAgnus = amiga.agnus.chipRamLimit < config.chipRam
        if badAgnus {
            memWarnInfo1.stringValue = "Chip Ram is not fully usable."
            memWarnInfo2.stringValue = "The selected Agnus revision is limited to address \(amiga.agnus.chipRamLimit) KB."
        }
        memWarnImage.isHidden = !badAgnus
        memWarnInfo1.isHidden = !badAgnus
        memWarnInfo2.isHidden = !badAgnus
    }

    @IBAction func memChipRamAction(_ sender: NSPopUpButton!) {

        config.chipRam = sender.selectedTag()
        refresh()
    }

    @IBAction func memSlowRamAction(_ sender: NSPopUpButton!) {

        config.slowRam = sender.selectedTag()
        refresh()
    }

    @IBAction func memFastRamAction(_ sender: NSPopUpButton!) {

        config.fastRam = sender.selectedTag()
        refresh()
    }

    @IBAction func memBankMapAction(_ sender: NSPopUpButton!) {

        config.bankMap = sender.selectedTag()
        refresh()
    }

    @IBAction func memUnmappingTypeAction(_ sender: NSPopUpButton!) {

        config.unmappingType = sender.selectedTag()
        refresh()
    }

    @IBAction func memRamInitPatternAction(_ sender: NSPopUpButton!) {

        config.ramInitPattern = sender.selectedTag()
        refresh()
    }

    @IBAction func memPresetAction(_ sender: NSPopUpButton!) {

        let defaults = AmigaProxy.defaults!

        amiga.suspend()

        // Revert to standard settings
        AmigaProxy.defaults.removeMemoryUserDefaults()

        // Adjust some settings
        switch sender.selectedTag() {

        case 0:

            // Amiga 500
            defaults.set(.CHIP_RAM, 512)
            defaults.set(.SLOW_RAM, 512)
            defaults.set(.BANKMAP, BankMap.A500.rawValue)

        case 1:

            // Amiga 1000
            defaults.set(.CHIP_RAM, 256)
            defaults.set(.SLOW_RAM, 0)
            defaults.set(.BANKMAP, BankMap.A1000.rawValue)

        case 2:

            // Amiga 2000
            defaults.set(.CHIP_RAM, 1024)
            defaults.set(.SLOW_RAM, 0)
            defaults.set(.BANKMAP, BankMap.A2000B.rawValue)

        case 3:

            // Amiga 500+
            defaults.set(.CHIP_RAM, 512)
            defaults.set(.SLOW_RAM, 512)
            defaults.set(.BANKMAP, BankMap.A500.rawValue)

        default:
            fatalError()
        }

        // Update the configutation
        config.applyMemoryUserDefaults()

        amiga.resume()
        refresh()
    }

    @IBAction func memDefaultsAction(_ sender: NSButton!) {

        config.saveMemoryUserDefaults()
    }
}
