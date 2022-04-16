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
        
        // CPU
        csCpuInfo1.stringValue = "Motorola MC68000"
        csCpuInfo2.stringValue = "7.09 MHz"
        csCpuInfo3.stringValue = "All models"

        // Agnus
        csAgnusRevision.selectItem(withTag: config.agnusRev)
        switch AgnusRevision(rawValue: config.agnusRev) {

        case .OCS_OLD:
            csAgnusInfo1.stringValue = "MOS 8367"
            csAgnusInfo2.stringValue = "PAL"
            csAgnusInfo3.stringValue = "A1000, A2000A"

        case .OCS:
            csAgnusInfo1.stringValue = "MOS 8371"
            csAgnusInfo2.stringValue = "PAL"
            csAgnusInfo3.stringValue = "Early A500/A2000"

        case .ECS_1MB:
            csAgnusInfo1.stringValue = "MOS 8372A"
            csAgnusInfo2.stringValue = "PAL"
            csAgnusInfo3.stringValue = "Later A500/A2000"

        case .ECS_2MB:
            csAgnusInfo1.stringValue = "MOS 8375"
            csAgnusInfo2.stringValue = "PAL"
            csAgnusInfo3.stringValue = "A500+, A600"
            
        default:
            csAgnusInfo1.stringValue = "Invalid"
            csAgnusInfo2.stringValue = ""
            csAgnusInfo3.stringValue = ""
        }
        
        // Denise
        csDeniseRevision.selectItem(withTag: config.deniseRev)
        switch DeniseRevision(rawValue: config.deniseRev) {
        
        case .OCS:
            csDeniseInfo1.stringValue = "MOS 8362R8"
            csDeniseInfo2.stringValue = "PAL"
            csDeniseInfo3.stringValue = "A500, A1000, A2000"

        case .OCS_BRDRBLNK:
            csDeniseInfo1.stringValue = "MOS 8362R8"
            csDeniseInfo2.stringValue = "PAL"
            csDeniseInfo3.stringValue = "Emulator mod"
            
        case .ECS:
            csDeniseInfo1.stringValue = "MOS 8373R4"
            csDeniseInfo2.stringValue = "PAL"
            csDeniseInfo3.stringValue = "A500+, A600"
            
        default:
            csDeniseInfo1.stringValue = "Invalid"
            csDeniseInfo2.stringValue = ""
            csDeniseInfo3.stringValue = ""
        }

        // CIAs
        csCiaRevision.selectItem(withTag: config.ciaRev)
        switch CIARevision(rawValue: config.ciaRev) {

        case .MOS_8520_DIP:
            csCiaInfo1.stringValue = "MOS 8520"
            csCiaInfo2.stringValue = "40 pin package"
            csCiaInfo3.stringValue = "A500, A1000, A2000"

        case .MOS_8520_PLCC:
            csCiaInfo1.stringValue = "MOS 8520PL"
            csCiaInfo2.stringValue = "44 pin package"
            csCiaInfo3.stringValue = "A600"

        default:
            csCiaInfo1.stringValue = "Invalid"
            csCiaInfo2.stringValue = ""
            csCiaInfo3.stringValue = ""
        }
        
        // RTC
        csRtcRevision.selectItem(withTag: config.rtClock)
        switch RTCRevision(rawValue: config.rtClock) {
        
        case .NONE:
            csRtcInfo1.stringValue = ""
            csRtcInfo2.stringValue = ""
            csRtcInfo3.stringValue = ""

        case .OKI:
            csRtcInfo1.stringValue = "MSM6242B"
            csRtcInfo2.stringValue = "Real-time clock"
            csRtcInfo3.stringValue = "A2000"

        case .RICOH:
            csRtcInfo1.stringValue = "RF5C01A"
            csRtcInfo2.stringValue = "Real-time clock"
            csRtcInfo3.stringValue = "A3000, A4000"

        default:
            csRtcInfo1.stringValue = "Invalid"
            csRtcInfo2.stringValue = ""
            csRtcInfo3.stringValue = ""
        }
        
        // Disable some controls if emulator is powered on
        csCpuRevision.isEnabled = poweredOff
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

    @IBAction func csCpusRevAction(_ sender: NSPopUpButton!) {

        refresh()
    }

    @IBAction func csAgnusRevAction(_ sender: NSPopUpButton!) {

        config.agnusRev = sender.selectedTag()
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
            defaults.set(.AGNUS_REVISION, value: AgnusRevision.ECS_1MB.rawValue)
            defaults.set(.RTC_MODEL, value: RTCRevision.NONE.rawValue)

        case 1:
            
            // Amiga 1000
            defaults.set(.AGNUS_REVISION, value: AgnusRevision.OCS_OLD.rawValue)
            defaults.set(.RTC_MODEL, value: RTCRevision.NONE.rawValue)

        case 2:
            
            // Amiga 2000
            defaults.set(.AGNUS_REVISION, value: AgnusRevision.ECS_2MB.rawValue)
            defaults.set(.RTC_MODEL, value: RTCRevision.OKI.rawValue)

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
