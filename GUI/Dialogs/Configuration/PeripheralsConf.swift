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

        func update(_ component: NSTextField, enable: Bool) {
            component.textColor = enable ? .controlTextColor : .disabledControlTextColor
            component.isEnabled = enable
        }
        func update(_ component: NSControl, enable: Bool) {
            component.isEnabled = enable
        }
        
        let poweredOff = emu.poweredOff
        
        // Floppy drives
        perDf1Connect.state = config.df1Connected ? .on : .off
        perDf2Connect.state = config.df2Connected ? .on : .off
        perDf3Connect.state = config.df3Connected ? .on : .off
        perDf0Type.selectItem(withTag: config.df0Type)
        perDf1Type.selectItem(withTag: config.df1Type)
        perDf2Type.selectItem(withTag: config.df2Type)
        perDf3Type.selectItem(withTag: config.df3Type)

        // Hard drives
        perHd0Connect.state = config.hd0Connected ? .on : .off
        perHd1Connect.state = config.hd1Connected ? .on : .off
        perHd2Connect.state = config.hd2Connected ? .on : .off
        perHd3Connect.state = config.hd3Connected ? .on : .off
        perHd0Type.selectItem(withTag: config.hd0Type)
        perHd1Type.selectItem(withTag: config.hd1Type)
        perHd2Type.selectItem(withTag: config.hd2Type)
        perHd3Type.selectItem(withTag: config.hd3Type)

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
        
        // Joysticks
        let enable = config.autofire
        perAutofire.state = enable ? .on : .off
        perAutofireCease.state = config.autofireBursts ? .on : .off
        perAutofireBullets.integerValue = config.autofireBullets
        perAutofireFrequency.integerValue = config.autofireFrequency
        update(perAutofireFrequency, enable: enable)
        update(perAutofireFrequencyText1, enable: enable)
        update(perAutofireFrequencyText2, enable: enable)
        update(perAutofireCease, enable: enable)
        update(perAutofireCeaseText, enable: enable)
        update(perAutofireBullets, enable: enable && perAutofireCease.state == .on)
        update(perAutofireBulletsText, enable: enable && perAutofireCease.state == .on)

        // Lock controls if emulator is powered on
        perDf1Connect.isEnabled = poweredOff
        perDf2Connect.isEnabled = poweredOff && perDf1Connect.state == .on
        perDf3Connect.isEnabled = poweredOff && perDf2Connect.state == .on
        perDf0Type.isEnabled = poweredOff
        perDf1Type.isEnabled = poweredOff && config.df1Connected
        perDf2Type.isEnabled = poweredOff && config.df2Connected
        perDf3Type.isEnabled = poweredOff && config.df3Connected
        perHd0Connect.isEnabled = poweredOff
        perHd1Connect.isEnabled = poweredOff // && perHd0Connect.state == .on
        perHd2Connect.isEnabled = poweredOff // && perHd1Connect.state == .on
        perHd3Connect.isEnabled = poweredOff // && perHd2Connect.state == .on
        perHd0Type.isEnabled = poweredOff
        perHd1Type.isEnabled = poweredOff // && config.hd1Connected
        perHd2Type.isEnabled = poweredOff // && config.hd2Connected
        perHd3Type.isEnabled = poweredOff // && config.hd3Connected
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
    }
    
    @IBAction func perDriveTypeAction(_ sender: NSPopUpButton!) {
        
        switch sender.tag {
        case 0: config.df0Type = sender.selectedTag()
        case 1: config.df1Type = sender.selectedTag()
        case 2: config.df2Type = sender.selectedTag()
        case 3: config.df3Type = sender.selectedTag()
        default: fatalError()
        }
    }

    @IBAction func perHdrConnectAction(_ sender: NSButton!) {
        
        switch sender.tag {
        case 0: config.hd0Connected = sender.state == .on
        case 1: config.hd1Connected = sender.state == .on
        case 2: config.hd2Connected = sender.state == .on
        case 3: config.hd3Connected = sender.state == .on
        default: fatalError()
        }
    }
    
    @IBAction func perHdrTypeAction(_ sender: NSPopUpButton!) {
        
    }
    
    @IBAction func perGameDeviceAction(_ sender: NSPopUpButton!) {
        
        switch sender.tag {
        case 1: config.gameDevice1 = sender.selectedTag()
        case 2: config.gameDevice2 = sender.selectedTag()
        default: fatalError()
        }
    }

    @IBAction func perAutofireAction(_ sender: NSButton!) {

        config.autofire = (sender.state == .on)
    }

    @IBAction func perAutofireCeaseAction(_ sender: NSButton!) {

        config.autofireBursts = (sender.state == .on)
    }

    @IBAction func perAutofireBulletsAction(_ sender: NSTextField!) {

        config.autofireBullets = sender.integerValue
    }

    @IBAction func perAutofireFrequencyAction(_ sender: NSSlider!) {

        config.autofireFrequency = sender.integerValue
    }

    @IBAction func perSerialDeviceAction(_ sender: NSPopUpButton!) {

        config.serialDevice = sender.selectedTag()
    }

    @IBAction func perSerialDevicePortAction(_ sender: NSTextField!) {
        
        if sender.integerValue > 0 && sender.integerValue < 65536 {
            config.serialDevicePort = sender.integerValue
        }
    }

    @IBAction func perPresetAction(_ sender: NSPopUpButton!) {
         
        emu.suspend()
        
        // Revert to standard settings
        EmulatorProxy.defaults.removePeripheralsUserDefaults()
        
        // Update the configuration
        config.applyPeripheralsUserDefaults()

        emu.resume()
     }

     @IBAction func perDefaultsAction(_ sender: NSButton!) {
         
         config.savePeripheralsUserDefaults()
     }
}
