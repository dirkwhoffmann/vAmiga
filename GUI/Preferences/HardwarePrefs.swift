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
        let memConfig = amiga.memConfig()
        let poweredOff = amiga.isPoweredOff()

        // Machine
        hwAmigaModelPopup.selectItem(withTag: config.model.rawValue)
        hwLayoutPopup.selectItem(withTag: config.layout)
        hwRealTimeClock.state = config.realTimeClock ? .on : .off
        
        // Memory
        hwChipRamPopup.selectItem(withTag: memConfig.chipRamSize)
        hwSlowRamPopup.selectItem(withTag: memConfig.slowRamSize)
        hwFastRamPopup.selectItem(withTag: memConfig.fastRamSize)

        // Drive
        hwDf1Connect.state = config.df1.connected ? .on : .off
        hwDf2Connect.state = config.df2.connected ? .on : .off
        hwDf3Connect.state = config.df3.connected ? .on : .off
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
        hwDf1Type.isEnabled = poweredOff && config.df1.connected
        hwDf2Type.isEnabled = poweredOff && config.df2.connected
        hwDf3Type.isEnabled = poweredOff && config.df3.connected

        // Label the OK button
        hwOKButton.title = okLabel
    }
    
    @IBAction func hwAmigaModelAction(_ sender: NSPopUpButton!) {
        
        amigaProxy?.configureModel(sender.selectedTag())
        refresh()
    }
    
    @IBAction func hwLayoutAction(_ sender: NSPopUpButton!) {
        
        amigaProxy?.configureLayout(sender.selectedTag())
        refresh()
    }
    
    @IBAction func hwRealTimeClocktAction(_ sender: NSButton!) {
        
        amigaProxy?.configureRealTimeClock(sender.state == .on)
        refresh()
    }
    
    @IBAction func hwChipRamAction(_ sender: NSPopUpButton!) {
        
        amigaProxy?.configureChipMemory(sender.selectedTag())
        refresh()
    }

    @IBAction func hwSlowRamAction(_ sender: NSPopUpButton!) {
        
        amigaProxy?.configureSlowMemory(sender.selectedTag())
        refresh()
    }

    @IBAction func hwFastRamAction(_ sender: NSPopUpButton!) {
        
        amigaProxy?.configureFastMemory(sender.selectedTag())
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
 
    @IBAction func hwDriveSpeedAction(_ sender: NSPopUpButton!) {
        
        let nr = sender.tag
        amigaProxy?.configureDrive(nr, speed: sender.selectedTag())
        refresh()
    }

    @IBAction func hwSerialDeviceAction(_ sender: NSPopUpButton!) {

        amigaProxy?.configure(VA_SERIAL_DEVICE, value: sender.selectedTag())
        refresh()
    }
    
    @IBAction func hwFactorySettingsAction(_ sender: NSPopUpButton!) {
        
        track("\(sender.selectedTag())")
        
        switch sender.selectedTag() {
            
        case AMIGA_500.rawValue:
            
            amigaProxy?.configureModel(Defaults.A500.amigaModel.rawValue)
            amigaProxy?.configureLayout(Layout.us.rawValue)
            amigaProxy?.configureRealTimeClock(Defaults.A500.realTimeClock)
            
            amigaProxy?.configureChipMemory(Defaults.A500.chipRam)
            amigaProxy?.configureSlowMemory(Defaults.A500.slowRam)
            amigaProxy?.configureFastMemory(Defaults.A500.fastRam)
            
            amigaProxy?.configureDrive(0, connected: Defaults.A500.df0Connect)
            amigaProxy?.configureDrive(0, type:      Defaults.A500.df0Type.rawValue)
            amigaProxy?.configureDrive(1, connected: Defaults.A500.df1Connect)
            amigaProxy?.configureDrive(1, type:      Defaults.A500.df1Type.rawValue)
            amigaProxy?.configureDrive(2, connected: Defaults.A500.df2Connect)
            amigaProxy?.configureDrive(2, type:      Defaults.A500.df2Type.rawValue)
            amigaProxy?.configureDrive(3, connected: Defaults.A500.df3Connect)
            amigaProxy?.configureDrive(3, type:      Defaults.A500.df3Type.rawValue)

        case AMIGA_1000.rawValue:
            
            amigaProxy?.configureModel(Defaults.A1000.amigaModel.rawValue)
            amigaProxy?.configureLayout(Layout.us.rawValue)
            amigaProxy?.configureRealTimeClock(Defaults.A1000.realTimeClock)
            
            amigaProxy?.configureChipMemory(Defaults.A1000.chipRam)
            amigaProxy?.configureSlowMemory(Defaults.A1000.slowRam)
            amigaProxy?.configureFastMemory(Defaults.A1000.fastRam)

            amigaProxy?.configureDrive(0, connected: Defaults.A1000.df0Connect)
            amigaProxy?.configureDrive(0, type:      Defaults.A1000.df0Type.rawValue)
            amigaProxy?.configureDrive(1, connected: Defaults.A1000.df1Connect)
            amigaProxy?.configureDrive(1, type:      Defaults.A1000.df1Type.rawValue)
            amigaProxy?.configureDrive(2, connected: Defaults.A1000.df2Connect)
            amigaProxy?.configureDrive(2, type:      Defaults.A1000.df2Type.rawValue)
            amigaProxy?.configureDrive(3, connected: Defaults.A1000.df3Connect)
            amigaProxy?.configureDrive(3, type:      Defaults.A1000.df3Type.rawValue)

        case AMIGA_2000.rawValue:
            
            amigaProxy?.configureModel(Defaults.A2000.amigaModel.rawValue)
            amigaProxy?.configureRealTimeClock(Defaults.A2000.realTimeClock)
            
            amigaProxy?.configureChipMemory(Defaults.A2000.chipRam)
            amigaProxy?.configureSlowMemory(Defaults.A2000.slowRam)
            amigaProxy?.configureFastMemory(Defaults.A2000.fastRam)

            amigaProxy?.configureDrive(0, connected: Defaults.A2000.df0Connect)
            amigaProxy?.configureDrive(0, type:      Defaults.A2000.df0Type.rawValue)
            amigaProxy?.configureDrive(1, connected: Defaults.A2000.df1Connect)
            amigaProxy?.configureDrive(1, type:      Defaults.A2000.df1Type.rawValue)
            amigaProxy?.configureDrive(2, connected: Defaults.A2000.df2Connect)
            amigaProxy?.configureDrive(2, type:      Defaults.A2000.df2Type.rawValue)
            amigaProxy?.configureDrive(3, connected: Defaults.A2000.df3Connect)
            amigaProxy?.configureDrive(3, type:      Defaults.A2000.df3Type.rawValue)

        default:
            track("Cannot restore factory defaults (unknown Amiga model).")
        }
        
        refresh()
    }
}
