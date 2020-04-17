// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension PreferencesController {
    
    func refreshHardwareTab() {

        track()
        let config = amiga.config()
        let poweredOff = amiga.isPoweredOff()

        // Chipset
        hwAgnusRevisionPopup.selectItem(withTag: config.agnus.revision.rawValue)
        hwDeniseRevisionPopup.selectItem(withTag: config.denise.revision.rawValue)
        hwRealTimeClock.selectItem(withTag: config.rtc.model.rawValue)

        // Ports
        hwSerialDevice.selectItem(withTag: Int(config.serialPort.device.rawValue))

        // Memory
        hwChipRamPopup.selectItem(withTag: config.mem.chipSize / 1024)
        hwSlowRamPopup.selectItem(withTag: config.mem.slowSize / 1024)
        hwFastRamPopup.selectItem(withTag: config.mem.fastSize / 1024)

        // Drive
        hwDf1Connect.state = config.diskController.connected.1 ? .on : .off
        hwDf2Connect.state = config.diskController.connected.2 ? .on : .off
        hwDf3Connect.state = config.diskController.connected.3 ? .on : .off
        hwDf0Type.selectItem(withTag: config.df0.type.rawValue)
        hwDf1Type.selectItem(withTag: config.df1.type.rawValue)
        hwDf2Type.selectItem(withTag: config.df2.type.rawValue)
        hwDf3Type.selectItem(withTag: config.df3.type.rawValue)

        // Lock controls if emulator is powered on
        hwAgnusRevisionPopup.isEnabled = poweredOff
        hwDeniseRevisionPopup.isEnabled = poweredOff
        hwRealTimeClock.isEnabled = poweredOff
        hwChipRamPopup.isEnabled = poweredOff
        hwSlowRamPopup.isEnabled = poweredOff
        hwFastRamPopup.isEnabled = poweredOff
        hwDf1Connect.isEnabled = poweredOff
        hwDf2Connect.isEnabled = poweredOff
        hwDf3Connect.isEnabled = poweredOff
        hwDf0Type.isEnabled = poweredOff
        hwDf1Type.isEnabled = poweredOff && config.diskController.connected.1
        hwDf2Type.isEnabled = poweredOff && config.diskController.connected.2
        hwDf3Type.isEnabled = poweredOff && config.diskController.connected.3
        hwFactorySettingsPopup.isEnabled = poweredOff

        // Lock symbol and explanation
        hwLockImage.isHidden = poweredOff
        hwLockText.isHidden = poweredOff
        hwLockSubText.isHidden = poweredOff

        // OK Button
        hwOKButton.title = buttonLabel
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
            parent.mydocument?.showConfigurationAltert(ERR_CHIP_RAM_LIMIT)
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
 
    @IBAction func hwSerialDeviceAction(_ sender: NSPopUpButton!) {

        config.serialDevice = sender.selectedTag()
        refresh()
    }

    @IBAction func hwFactorySettingsAction(_ sender: NSPopUpButton!) {
        
        switch sender.selectedTag() {
        case 0: hwFactorySettingsAction(Defaults.A500)
        case 1: hwFactorySettingsAction(Defaults.A1000)
        case 2: hwFactorySettingsAction(Defaults.A2000)
        default: fatalError()
        }
    }

    func hwFactorySettingsAction(_ defaults: Defaults.ModelDefaults) {

        config.agnusRev = defaults.agnusRevision.rawValue
        config.deniseRev = defaults.deniseRevision.rawValue
        config.rtClock = defaults.realTimeClock.rawValue

        config.chipRam = defaults.chipRam
        config.slowRam = defaults.slowRam
        config.fastRam = defaults.fastRam

        config.serialDevice = defaults.serialDevice.rawValue

        config.df0Connected = defaults.driveConnect[0]
        config.df1Connected = defaults.driveConnect[1]
        config.df2Connected = defaults.driveConnect[2]
        config.df3Connected = defaults.driveConnect[3]
        config.df0Type = defaults.driveType[0].rawValue
        config.df1Type = defaults.driveType[1].rawValue
        config.df2Type = defaults.driveType[2].rawValue
        config.df3Type = defaults.driveType[3].rawValue

        refresh()
    }
}
