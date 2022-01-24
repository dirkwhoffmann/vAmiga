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
        
        // Chipset
        csAgnusRevision.selectItem(withTag: config.agnusRev)
        csDeniseRevision.selectItem(withTag: config.deniseRev)
        csCiaRevision.selectItem(withTag: config.ciaRev)
        csRtcRevision.selectItem(withTag: config.rtClock)

        // Lock some controls if emulator is powered on
        csCpuRevision.isEnabled = poweredOff
        csAgnusRevision.isEnabled = poweredOff
        csDeniseRevision.isEnabled = poweredOff
        csCiaRevision.isEnabled = poweredOff
        csRtcRevision.isEnabled = poweredOff
        csFactorySettingsPopup.isEnabled = poweredOff

        // Lock symbol and explanation
        csLockImage.isHidden = poweredOff
        csLockText.isHidden = poweredOff
        csLockSubText.isHidden = poweredOff

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
        
        switch sender.selectedTag() {
        case 0: config.loadChipsetDefaults(HardwareDefaults.A500)
        case 1: config.loadChipsetDefaults(HardwareDefaults.A1000)
        case 2: config.loadChipsetDefaults(HardwareDefaults.A2000)
        default: fatalError()
        }
        refresh()
    }

    @IBAction func csDefaultsAction(_ sender: NSButton!) {
        
        config.saveHardwareUserDefaults()
    }
}
