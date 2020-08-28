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

        let extIdentifier = amiga.mem.extIdentifier
        let hasExt = extIdentifier != .ROM_MISSING

        // Timing
        memEClockSyncing.state = config.eClockSyncing ? .on : .off
        memSlowRamDelay.state = config.slowRamDelay ? .on : .off

        // Chipset features
        memSlowRamMirror.state = config.slowRamMirror ? .on : .off

        // Bank map
        memBankE0E7.item(at: 0)?.title = hasExt ? "Extension Rom" : "Kickstart Rom"
        memBankF0F7.item(at: 0)?.title = hasExt ? "Extension Rom" : "Kickstart Rom"

        switch MemorySource(rawValue: config.bankD8DB) {
        case .MEM_RTC: memBankD8DB.selectItem(withTag: 0)
        case .MEM_CUSTOM: memBankD8DB.selectItem(withTag: 1)
        case .MEM_NONE: memBankD8DB.selectItem(withTag: 2)
        default: fatalError()
        }
        switch MemorySource(rawValue: config.bankDC) {
        case .MEM_RTC: memBankDC.selectItem(withTag: 0)
        case .MEM_CUSTOM: memBankDC.selectItem(withTag: 1)
        case .MEM_NONE: memBankDC.selectItem(withTag: 2)
        default: fatalError()
        }
        switch MemorySource(rawValue: config.bankE0E7) {
        case .MEM_EXT: memBankE0E7.selectItem(withTag: 0)
        case .MEM_NONE: memBankE0E7.selectItem(withTag: 1)
        default: fatalError()
        }
        switch MemorySource(rawValue: config.bankF0F7) {
        case .MEM_EXT: memBankF0F7.selectItem(withTag: 0)
        case .MEM_NONE: memBankF0F7.selectItem(withTag: 1)
        default: fatalError()
        }
        unmappingType.selectItem(withTag: config.unmappingType)
        ramInitPattern.selectItem(withTag: config.ramInitPattern)

        // Lock symbol and explanation
        memLockImage.isHidden = true // poweredOff
        memLockText.isHidden = true // poweredOff
        memLockSubText.isHidden = true // poweredOff
        
        // Buttons
        memPowerButton.isHidden = !bootable
    }

    @IBAction func memEClockSyncingAction(_ sender: NSButton!) {
        
        config.eClockSyncing = sender.state == .on
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

    @IBAction func memBankD8DBAction(_ sender: NSPopUpButton!) {
                
        switch sender.selectedTag() {
        case 0: config.bankD8DB = MemorySource.MEM_RTC.rawValue
        case 1: config.bankD8DB = MemorySource.MEM_CUSTOM.rawValue
        case 2: config.bankD8DB = MemorySource.MEM_NONE.rawValue
        default: fatalError()
        }
        refresh()
    }

    @IBAction func memBankDCAction(_ sender: NSPopUpButton!) {
        
        switch sender.selectedTag() {
        case 0: config.bankDC = MemorySource.MEM_RTC.rawValue
        case 1: config.bankDC = MemorySource.MEM_CUSTOM.rawValue
        case 2: config.bankDC = MemorySource.MEM_NONE.rawValue
        default: fatalError()
        }
        refresh()
    }

    @IBAction func memBankE0E7Action(_ sender: NSPopUpButton!) {
        
        switch sender.selectedTag() {
        case 0: config.bankE0E7 = MemorySource.MEM_EXT.rawValue
        case 1: config.bankE0E7 = MemorySource.MEM_NONE.rawValue
        default: fatalError()
        }
        refresh()
    }

    @IBAction func memBankF0F7Action(_ sender: NSPopUpButton!) {

        switch sender.selectedTag() {
        case 0: config.bankF0F7 = MemorySource.MEM_EXT.rawValue
        case 1: config.bankF0F7 = MemorySource.MEM_NONE.rawValue
        default: fatalError()
        }
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
         
         switch sender.selectedTag() {
         case 0: config.loadMemoryDefaults(MemoryDefaults.std)
         default: fatalError()
         }
         refresh()
     }

     @IBAction func memDefaultsAction(_ sender: NSButton!) {
         
         config.saveMemoryUserDefaults()
     }
}
