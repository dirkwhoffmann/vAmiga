// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ConfigurationController {

    func refreshPeripheralsTab() {

        let poweredOff = amiga.poweredOff
        
        // Floppy drives
        perDf1Connect.state = config.df1Connected ? .on : .off
        perDf2Connect.state = config.df2Connected ? .on : .off
        perDf3Connect.state = config.df3Connected ? .on : .off
        perDf0Type.selectItem(withTag: config.df0Type)
        perDf1Type.selectItem(withTag: config.df1Type)
        perDf2Type.selectItem(withTag: config.df2Type)
        perDf3Type.selectItem(withTag: config.df3Type)

        // Hard drives
        perDh0Connect.state = config.dh0Connected ? .on : .off
        perDh1Connect.state = config.dh1Connected ? .on : .off
        perDh2Connect.state = config.dh2Connected ? .on : .off
        perDh3Connect.state = config.dh3Connected ? .on : .off
        perDh0Type.selectItem(withTag: config.dh0Type)
        perDh1Type.selectItem(withTag: config.dh1Type)
        perDh2Type.selectItem(withTag: config.dh2Type)
        perDh3Type.selectItem(withTag: config.dh3Type)

        // Ports
        let nullmodem = SerialPortDevice.NULLMODEM.rawValue
        parent.gamePadManager.refresh(popup: perGameDevice1, hide: true)
        parent.gamePadManager.refresh(popup: perGameDevice2, hide: true)
        perGameDevice1.selectItem(withTag: config.gameDevice1)
        perGameDevice2.selectItem(withTag: config.gameDevice2)
        perSerialDevice.selectItem(withTag: config.serialDevice)
        perSerialPort.integerValue = config.serialDevicePort
        perSerialPort.isHidden = config.serialDevice != nullmodem
        perSerialPortText.isHidden = config.serialDevice != nullmodem
        
        // Lock controls if emulator is powered on
        perDf1Connect.isEnabled = poweredOff
        perDf2Connect.isEnabled = poweredOff && perDf1Connect.state == .on
        perDf3Connect.isEnabled = poweredOff && perDf2Connect.state == .on
        perDf0Type.isEnabled = poweredOff
        perDf1Type.isEnabled = poweredOff && config.df1Connected
        perDf2Type.isEnabled = poweredOff && config.df2Connected
        perDf3Type.isEnabled = poweredOff && config.df3Connected
        perDh0Connect.isEnabled = poweredOff
        perDh1Connect.isEnabled = poweredOff && perDh0Connect.state == .on
        perDh2Connect.isEnabled = poweredOff && perDh1Connect.state == .on
        perDh3Connect.isEnabled = poweredOff && perDh2Connect.state == .on
        perDh0Type.isEnabled = poweredOff
        perDh1Type.isEnabled = poweredOff && config.dh1Connected
        perDh2Type.isEnabled = poweredOff && config.dh2Connected
        perDh3Type.isEnabled = poweredOff && config.dh3Connected
        perFactorySettingsPopup.isEnabled = poweredOff

        // Lock symbol and explanation
        perLockImage.isHidden = poweredOff
        perLockInfo1.isHidden = poweredOff
        perLockInfo2.isHidden = poweredOff
        
        // Buttons
        perPowerButton.isHidden = !bootable
    }

    @IBAction func perDriveConnectAction(_ sender: NSButton!) {
        
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
    
    @IBAction func perDriveTypeAction(_ sender: NSPopUpButton!) {
        
        switch sender.tag {
        case 0: config.df0Type = sender.selectedTag()
        case 1: config.df1Type = sender.selectedTag()
        case 2: config.df2Type = sender.selectedTag()
        case 3: config.df3Type = sender.selectedTag()
        default: fatalError()
        }
        refresh()
    }

    @IBAction func perHdrConnectAction(_ sender: NSButton!) {
        
        switch sender.tag {
        case 0: config.dh0Connected = sender.state == .on
        case 1: config.dh1Connected = sender.state == .on
        case 2: config.dh2Connected = sender.state == .on
        case 3: config.dh3Connected = sender.state == .on
        default: fatalError()
        }
        
        // Disconnect dh(n+1) if dfn is disconnected
        if !config.dh0Connected { config.dh1Connected = false }
        if !config.dh1Connected { config.dh2Connected = false }
        if !config.dh2Connected { config.dh3Connected = false }

        refresh()
    }
    
    @IBAction func perHdrTypeAction(_ sender: NSPopUpButton!) {
        
        refresh()
    }
    
    @IBAction func perGameDeviceAction(_ sender: NSPopUpButton!) {

        track("port: \(sender.tag) device: \(sender.selectedTag())")
        
        switch sender.tag {
        case 1: config.gameDevice1 = sender.selectedTag()
        case 2: config.gameDevice2 = sender.selectedTag()
        default: fatalError()
        }
        refresh()
    }

    @IBAction func perSerialDeviceAction(_ sender: NSPopUpButton!) {

        config.serialDevice = sender.selectedTag()
        refresh()
    }

    @IBAction func perSerialDevicePortAction(_ sender: NSTextField!) {
        
        if sender.integerValue > 0 && sender.integerValue < 65536 {
            config.serialDevicePort = sender.integerValue
        }
        refresh()
    }

    @IBAction func perPresetAction(_ sender: NSPopUpButton!) {
         
         switch sender.selectedTag() {
         case 0: config.loadPeripheralsDefaults(PeripheralsDefaults.std)
         default: fatalError()
         }
         refresh()
     }

     @IBAction func perDefaultsAction(_ sender: NSButton!) {
         
         config.savePeripheralsUserDefaults()
     }
}
