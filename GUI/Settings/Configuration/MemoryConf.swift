// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ConfigurationController {
    
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
        memFactorySettingsPopup.isEnabled = poweredOff

        // Chipset features
        memSlowRamDelay.state = config.slowRamDelay ? .on : .off
        memSlowRamMirror.state = config.slowRamMirror ? .on : .off
        memSlowRamMirror.isEnabled = amiga.agnus.isECS

        // Warning
        let badAgnus = amiga.agnus.chipRamLimit < config.chipRam
        if badAgnus {
            memWarnInfo1.stringValue = "Chip Ram is not fully usable."
            memWarnInfo2.stringValue = "The selected Agnus revision is limited to address \(amiga.agnus.chipRamLimit) MB."
        }
        memWarnImage.isHidden = !badAgnus
        memWarnInfo1.isHidden = !badAgnus
        memWarnInfo2.isHidden = !badAgnus

        // Lock
        memLockImage.isHidden = poweredOff
        memLockText.isHidden = poweredOff

        // Buttons
        memPowerButton.isHidden = !bootable
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

    @IBAction func memSlowRamDelayAction(_ sender: NSButton!) {

        config.slowRamDelay = sender.state == .on
        refresh()
    }

    @IBAction func memSlowRamMirrorAction(_ sender: NSButton!) {

        config.slowRamMirror = sender.state == .on
        refresh()
    }

    @IBAction func memPresetAction(_ sender: NSPopUpButton!) {
        
        switch sender.selectedTag() {
        case 0: config.loadMemoryDefaults(MemoryDefaults.A500)
        case 1: config.loadMemoryDefaults(MemoryDefaults.A1000)
        case 2: config.loadMemoryDefaults(MemoryDefaults.A2000)
        default: fatalError()
        }
        refresh()
    }

    @IBAction func memDefaultsAction(_ sender: NSButton!) {
        
        config.saveMemoryUserDefaults()
    }
}
