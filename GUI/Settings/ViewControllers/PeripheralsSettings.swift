// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class PeripheralsSettingsViewController: SettingsViewController {

    // Flopp drives
    @IBOutlet weak var df0Type: NSPopUpButton!
    @IBOutlet weak var df1Connect: NSButton!
    @IBOutlet weak var df1Type: NSPopUpButton!
    @IBOutlet weak var df2Connect: NSButton!
    @IBOutlet weak var df2Type: NSPopUpButton!
    @IBOutlet weak var df3Connect: NSButton!
    @IBOutlet weak var df3Type: NSPopUpButton!

    // Hard drives
    @IBOutlet weak var hd0Connect: NSButton!
    @IBOutlet weak var hd0Type: NSPopUpButton!
    @IBOutlet weak var hd1Connect: NSButton!
    @IBOutlet weak var hd1Type: NSPopUpButton!
    @IBOutlet weak var hd2Connect: NSButton!
    @IBOutlet weak var hd2Type: NSPopUpButton!
    @IBOutlet weak var hd3Connect: NSButton!
    @IBOutlet weak var hd3Type: NSPopUpButton!

    // Ports
    @IBOutlet weak var gameDevice1: NSPopUpButton!
    @IBOutlet weak var gameDevice2: NSPopUpButton!
    @IBOutlet weak var serialDevice: NSPopUpButton!
    @IBOutlet weak var serialPort: NSTextField!
    @IBOutlet weak var serialPortText: NSTextField!

    // Joystick
    @IBOutlet weak var autofire: NSButton!
    @IBOutlet weak var autofireText: NSTextField!
    @IBOutlet weak var autofireFrequency: NSSlider!
    @IBOutlet weak var autofireFrequencyText1: NSTextField!
    @IBOutlet weak var autofireFrequencyText2: NSTextField!
    @IBOutlet weak var autofireCease: NSButton!
    @IBOutlet weak var autofireCeaseText: NSTextField!
    @IBOutlet weak var autofireBullets: NSTextField!
    @IBOutlet weak var autofireBulletsText: NSTextField!

    override var showLock: Bool { true }
    
    override func viewDidLoad() {

        log(.lifetime)
    }

    //
    // Methods from SettingsViewController
    //
    
    override func refresh() {

        func update(_ component: NSTextField, enable: Bool = true, hidden: Bool = false) {
            component.textColor = enable ? .controlTextColor : .disabledControlTextColor
            component.isEnabled = enable
            component.isHidden = hidden
        }
        func update(_ component: NSControl, enable: Bool = true, hidden: Bool = false) {
            component.isEnabled = enable
            component.isHidden = hidden
        }

        super.refresh()

        guard let emu = emu, let config = config else { return }
        let poweredOff = emu.poweredOff

        // Floppy drives
        df1Connect.state = config.df1Connected ? .on : .off
        df2Connect.state = config.df2Connected ? .on : .off
        df3Connect.state = config.df3Connected ? .on : .off
        df0Type.selectItem(withTag: config.df0Type)
        df1Type.selectItem(withTag: config.df1Type)
        df2Type.selectItem(withTag: config.df2Type)
        df3Type.selectItem(withTag: config.df3Type)

        // Hard drives
        hd0Connect.state = config.hd0Connected ? .on : .off
        hd1Connect.state = config.hd1Connected ? .on : .off
        hd2Connect.state = config.hd2Connected ? .on : .off
        hd3Connect.state = config.hd3Connected ? .on : .off
        hd0Type.selectItem(withTag: config.hd0Type)
        hd1Type.selectItem(withTag: config.hd1Type)
        hd2Type.selectItem(withTag: config.hd2Type)
        hd3Type.selectItem(withTag: config.hd3Type)

        // Ports
        let nullmodem = SerialPortDevice.NULLMODEM.rawValue
        gamePadManager?.refresh(popup: gameDevice1, hide: true)
        gamePadManager?.refresh(popup: gameDevice2, hide: true)
        gameDevice1.selectItem(withTag: config.gameDevice1)
        gameDevice2.selectItem(withTag: config.gameDevice2)
        serialDevice.selectItem(withTag: config.serialDevice)
        serialPort.integerValue = config.serialDevicePort
        serialPort.isHidden = config.serialDevice != nullmodem
        serialPortText.isHidden = config.serialDevice != nullmodem

        // Joysticks
        autofire.state = config.autofire ? .on : .off
        autofireCease.state = config.autofireBursts ? .on : .off
        autofireBullets.integerValue = config.autofireBullets
        autofireFrequency.integerValue = config.autofireDelay
        update(autofireFrequency, hidden: !config.autofire)
        update(autofireFrequencyText1, hidden: !config.autofire)
        update(autofireFrequencyText2, hidden: !config.autofire)
        update(autofireCease, hidden: !config.autofire)
        update(autofireCeaseText, hidden: !config.autofire)
        update(autofireBullets, hidden: !config.autofire || !config.autofireBursts)
        update(autofireBulletsText, hidden: !config.autofire || !config.autofireBursts)

        // Lock controls if emulator is powered on
        df1Connect.isEnabled = poweredOff
        df2Connect.isEnabled = poweredOff && df1Connect.state == .on
        df3Connect.isEnabled = poweredOff && df2Connect.state == .on
        df0Type.isEnabled = poweredOff
        df1Type.isEnabled = poweredOff && config.df1Connected
        df2Type.isEnabled = poweredOff && config.df2Connected
        df3Type.isEnabled = poweredOff && config.df3Connected
        hd0Connect.isEnabled = poweredOff
        hd1Connect.isEnabled = poweredOff // && perHd0Connect.state == .on
        hd2Connect.isEnabled = poweredOff // && perHd1Connect.state == .on
        hd3Connect.isEnabled = poweredOff // && perHd2Connect.state == .on
        hd0Type.isEnabled = poweredOff
        hd1Type.isEnabled = poweredOff // && config.hd1Connected
        hd2Type.isEnabled = poweredOff // && config.hd2Connected
        hd3Type.isEnabled = poweredOff // && config.hd3Connected
    }

    override func preset(tag: Int) {

        emu?.suspend()

        // Revert to standard settings
        EmulatorProxy.defaults.removePeripheralsUserDefaults()

        // Update the configuration
        config?.applyPeripheralsUserDefaults()

        emu?.resume()
    }

    override func save() {

        config?.savePeripheralsUserDefaults()
    }

    //
    // Action methods
    //

    @IBAction func driveConnectAction(_ sender: NSButton!) {

        guard let config = config else { return }

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

    @IBAction func driveTypeAction(_ sender: NSPopUpButton!) {

        switch sender.tag {
        case 0: config?.df0Type = sender.selectedTag()
        case 1: config?.df1Type = sender.selectedTag()
        case 2: config?.df2Type = sender.selectedTag()
        case 3: config?.df3Type = sender.selectedTag()
        default: fatalError()
        }
    }

    @IBAction func hdrConnectAction(_ sender: NSButton!) {

        switch sender.tag {
        case 0: config?.hd0Connected = sender.state == .on
        case 1: config?.hd1Connected = sender.state == .on
        case 2: config?.hd2Connected = sender.state == .on
        case 3: config?.hd3Connected = sender.state == .on
        default: fatalError()
        }
    }

    @IBAction func hdrTypeAction(_ sender: NSPopUpButton!) {

    }

    @IBAction func gameDeviceAction(_ sender: NSPopUpButton!) {

        switch sender.tag {
        case 1: config?.gameDevice1 = sender.selectedTag()
        case 2: config?.gameDevice2 = sender.selectedTag()
        default: fatalError()
        }
    }

    @IBAction func autofireAction(_ sender: NSButton!) {

        config?.autofire = (sender.state == .on)
    }

    @IBAction func autofireCeaseAction(_ sender: NSButton!) {

        config?.autofireBursts = (sender.state == .on)
    }

    @IBAction func autofireBulletsAction(_ sender: NSTextField!) {

        config?.autofireBullets = sender.integerValue
    }

    @IBAction func autofireFrequencyAction(_ sender: NSSlider!) {

        config?.autofireDelay = sender.integerValue
    }

    @IBAction func serialDeviceAction(_ sender: NSPopUpButton!) {

        config?.serialDevice = sender.selectedTag()
    }

    @IBAction func serialDevicePortAction(_ sender: NSTextField!) {

        if sender.integerValue > 0 && sender.integerValue < 65536 {
            config?.serialDevicePort = sender.integerValue
        }
    }
}
