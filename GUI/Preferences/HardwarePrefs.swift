// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// swiftlint:disable colon

extension PreferencesController {
    
    func refreshHardwareTab() {
        
        guard let amiga = amigaProxy else { return }

        track()
        let config = amiga.config()
        let poweredOff = amiga.isPoweredOff()

        // Machine
        hwAmigaModelPopup.selectItem(withTag: config.model.rawValue)
        hwAgnusRevisionPopup.selectItem(withTag: config.agnusRevision.rawValue)
        hwDeniseRevisionPopup.selectItem(withTag: config.deniseRevision.rawValue)
        hwLayoutPopup.selectItem(withTag: config.layout)
        hwRealTimeClock.state = config.realTimeClock ? .on : .off
        
        // Memory
        hwChipRamPopup.selectItem(withTag: config.mem.chipRamSize / 1024)
        hwSlowRamPopup.selectItem(withTag: config.mem.slowRamSize / 1024)
        hwFastRamPopup.selectItem(withTag: config.mem.fastRamSize / 1024)

        // Drive
        hwDf1Connect.state = config.diskController.connected.1 ? .on : .off
        hwDf2Connect.state = config.diskController.connected.2 ? .on : .off
        hwDf3Connect.state = config.diskController.connected.3 ? .on : .off
        hwDf0Type.selectItem(withTag: config.df0.type.rawValue)
        hwDf1Type.selectItem(withTag: config.df1.type.rawValue)
        hwDf2Type.selectItem(withTag: config.df2.type.rawValue)
        hwDf3Type.selectItem(withTag: config.df3.type.rawValue)

        // Ports
        hwSerialDevice.selectItem(withTag: Int(config.serialDevice))

        // Lock controls if emulator is powered on
        hwAmigaModelPopup.isEnabled = poweredOff
        hwRealTimeClock.isEnabled = poweredOff && config.model != AMIGA_2000
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

        // Label the OK button
        hwOKButton.title = okLabel
    }
    
    @IBAction func hwAmigaModelAction(_ sender: NSPopUpButton!) {
        
        amigaProxy?.configureModel(sender.selectedTag())
        refresh()
    }

    @IBAction func hwAgnusRevAction(_ sender: NSPopUpButton!) {

        amigaProxy?.configure(VA_AGNUS_REVISION, value: sender.selectedTag())
        refresh()
    }

    @IBAction func hwDeniseRevAction(_ sender: NSPopUpButton!) {

        amigaProxy?.configure(VA_DENISE_REVISION, value: sender.selectedTag())
        refresh()
    }

    @IBAction func hwLayoutAction(_ sender: NSPopUpButton!) {

        amigaProxy?.configure(VA_KB_LAYOUT, value: sender.selectedTag())
        refresh()
    }
    
    @IBAction func hwRealTimeClocktAction(_ sender: NSButton!) {
        
        amigaProxy?.configure(VA_RT_CLOCK, enable: sender.state == .on)
        refresh()
    }
    
    @IBAction func hwChipRamAction(_ sender: NSPopUpButton!) {
        
        amigaProxy?.configure(VA_CHIP_RAM, value: sender.selectedTag())
        refresh()
    }

    @IBAction func hwSlowRamAction(_ sender: NSPopUpButton!) {
        
        amigaProxy?.configure(VA_SLOW_RAM, value: sender.selectedTag())
        refresh()
    }

    @IBAction func hwFastRamAction(_ sender: NSPopUpButton!) {
        
        amigaProxy?.configure(VA_FAST_RAM, value: sender.selectedTag())
        refresh()
    }

    @IBAction func hwDriveConnectAction(_ sender: NSButton!) {
        
        let driveNr = sender.tag
        amigaProxy?.configureDrive(driveNr, connected: sender.state == .on)
        refresh()
    }
    
    @IBAction func hwDriveTypeAction(_ sender: NSPopUpButton!) {
        
        track()
        
        let nr = sender.tag
        amigaProxy?.configureDrive(nr, type: sender.selectedTag())
        refresh()
    }
 
    @IBAction func hwSerialDeviceAction(_ sender: NSPopUpButton!) {

        amigaProxy?.configure(VA_SERIAL_DEVICE, value: sender.selectedTag())
        refresh()
    }

    @IBAction func hwFactorySettingsAction(_ sender: NSPopUpButton!) {
        
        track("\(sender.selectedTag())")

        switch sender.selectedTag() {

        case 0: hwFactorySettingsAction(Defaults.A500)
        case 1: hwFactorySettingsAction(Defaults.A1000)
        case 2: hwFactorySettingsAction(Defaults.A2000)
        default: track("Cannot restore factory defaults (unknown Amiga model).")
         }
    }

    func hwFactorySettingsAction(_ defaults: Defaults.ModelDefaults) {

        amigaProxy?.configureModel(defaults.amigaModel.rawValue)
        amigaProxy?.configure(VA_KB_LAYOUT, value: defaults.layout.rawValue)
        amigaProxy?.configure(VA_RT_CLOCK, enable: defaults.realTimeClock)

        amigaProxy?.configure(VA_CHIP_RAM, value: defaults.chipRam)
        amigaProxy?.configure(VA_SLOW_RAM, value: defaults.slowRam)
        amigaProxy?.configure(VA_FAST_RAM, value: defaults.fastRam)

        for i in 0...3 {
            amigaProxy?.configureDrive(i, connected: defaults.driveConnect[i])
            amigaProxy?.configureDrive(i, type:      defaults.driveType[i].rawValue)
        }

        refresh()
    }
}
