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

        track()
        let poweredOff = amiga.isPoweredOff

        // Chipset
        hwAgnusRevisionPopup.selectItem(withTag: config.agnusRev)
        hwDeniseRevisionPopup.selectItem(withTag: config.deniseRev)
        hwRealTimeClock.selectItem(withTag: config.rtClock)

        // Memory
        hwChipRamPopup.selectItem(withTag: config.chipRam)
        hwSlowRamPopup.selectItem(withTag: config.slowRam)
        hwFastRamPopup.selectItem(withTag: config.fastRam)

        // Drive
        hwDf1Connect.state = config.df1Connected ? .on : .off
        hwDf2Connect.state = config.df2Connected ? .on : .off
        hwDf3Connect.state = config.df3Connected ? .on : .off
        hwDf0Type.selectItem(withTag: config.df0Type)
        hwDf1Type.selectItem(withTag: config.df1Type)
        hwDf2Type.selectItem(withTag: config.df2Type)
        hwDf3Type.selectItem(withTag: config.df3Type)

        // Ports
        parent.gamePadManager.refresh(popup: hwGameDevice1, hide: true)
        parent.gamePadManager.refresh(popup: hwGameDevice2, hide: true)
        hwGameDevice1.selectItem(withTag: config.gameDevice1)
        hwGameDevice2.selectItem(withTag: config.gameDevice2)
        hwSerialDevice.selectItem(withTag: Int(config.serialDevice))

        // Lock controls if emulator is powered on
        hwAgnusRevisionPopup.isEnabled = poweredOff
        hwDeniseRevisionPopup.isEnabled = poweredOff
        hwRealTimeClock.isEnabled = poweredOff
        hwChipRamPopup.isEnabled = poweredOff
        hwSlowRamPopup.isEnabled = poweredOff
        hwFastRamPopup.isEnabled = poweredOff
        hwDf1Connect.isEnabled = poweredOff
        hwDf2Connect.isEnabled = poweredOff && hwDf1Connect.state == .on
        hwDf3Connect.isEnabled = poweredOff && hwDf2Connect.state == .on
        hwDf0Type.isEnabled = poweredOff
        hwDf1Type.isEnabled = poweredOff && config.df1Connected
        hwDf2Type.isEnabled = poweredOff && config.df2Connected
        hwDf3Type.isEnabled = poweredOff && config.df3Connected
        hwFactorySettingsPopup.isEnabled = poweredOff

        // Lock symbol and explanation
        hwLockImage.isHidden = poweredOff
        hwLockText.isHidden = poweredOff
        hwLockSubText.isHidden = poweredOff

        // Boot button
        hwPowerButton.isHidden = !bootable
    }
    
    @IBAction func hwAgnusRevAction(_ sender: NSPopUpButton!) {

        config.agnusRev = sender.selectedTag()
        refresh()
    }

    @IBAction func hwDeniseRevAction(_ sender: NSPopUpButton!) {

        config.deniseRev = sender.selectedTag()
        refresh()
    }

    @IBAction func hwRealTimeClockAction(_ sender: NSPopUpButton!) {
        
        config.rtClock = sender.selectedTag()
        refresh()
    }
    
    @IBAction func hwChipRamAction(_ sender: NSPopUpButton!) {

        let chipRamWanted = sender.selectedTag()
        let chipRamLimit = amiga.agnus.chipRamLimit()

        if chipRamWanted > chipRamLimit {
            parent.mydocument.showConfigurationAltert(.ERR_CHIP_RAM_LIMIT)
        } else {
            config.chipRam = sender.selectedTag()
        }
        
        refresh()
    }

    @IBAction func hwSlowRamAction(_ sender: NSPopUpButton!) {
        
        config.slowRam = sender.selectedTag()
        refresh()
    }

    @IBAction func hwFastRamAction(_ sender: NSPopUpButton!) {
        
        config.fastRam = sender.selectedTag()
        refresh()
    }

    @IBAction func hwDriveConnectAction(_ sender: NSButton!) {
        
        switch sender.tag {
        case 0: config.df0Connected = sender.state == .on
        case 1: config.df1Connected = sender.state == .on
        case 2: config.df2Connected = sender.state == .on
        case 3: config.df3Connected = sender.state == .on
        default: fatalError()
        }
        
        // Disconnect df(n+1) if dfn is disconnected
        if !config.df1Connected { config.df2Connected = false }
        if !config.df2Connected { config.df3Connected = false }

        refresh()
    }
    
    @IBAction func hwDriveTypeAction(_ sender: NSPopUpButton!) {
        
        switch sender.tag {
        case 0: config.df0Type = sender.tag
        case 1: config.df1Type = sender.tag
        case 2: config.df2Type = sender.tag
        case 3: config.df3Type = sender.tag
        default: fatalError()
        }
        refresh()
    }

    @IBAction func hwGameDeviceAction(_ sender: NSPopUpButton!) {

        track("port: \(sender.tag) device: \(sender.selectedTag())")
        
        switch sender.tag {
        case 1: config.gameDevice1 = sender.selectedTag()
        case 2: config.gameDevice2 = sender.selectedTag()
        default: fatalError()
        }
        refresh()
    }

    @IBAction func hwSerialDeviceAction(_ sender: NSPopUpButton!) {

        config.serialDevice = sender.selectedTag()
        refresh()
    }

    @IBAction func hwPresetAction(_ sender: NSPopUpButton!) {
        
        switch sender.selectedTag() {
        case 0: config.loadHardwareDefaults(HardwareDefaults.A500)
        case 1: config.loadHardwareDefaults(HardwareDefaults.A1000)
        case 2: config.loadHardwareDefaults(HardwareDefaults.A2000)
        default: fatalError()
        }
        refresh()
    }

    @IBAction func hwDefaultsAction(_ sender: NSButton!) {
        
        config.saveHardwareUserDefaults()
    }
}
