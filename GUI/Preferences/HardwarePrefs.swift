// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

extension PreferencesController {
    
    func refreshHardwareTab() {
        
        guard let amiga = amigaProxy else { return }

        track()
        let config = amiga.config()
        let memConfig = amiga.memConfig()
        
        // Machine
        hwAmigaModelPopup.selectItem(withTag: config.model.rawValue)
        hwLayoutPopup.selectItem(withTag: config.layout)
        hwRealTimeClock.state = config.realTimeClock ? .on : .off
        
        // Memory
        hwChipRamPopup.selectItem(withTag: memConfig.chipRamSize)
        hwSlowRamPopup.selectItem(withTag: memConfig.slowRamSize)
        hwFastRamPopup.selectItem(withTag: memConfig.fastRamSize)

        // Drive
        hwDf0Connect.state = config.df0.connected ? .on : .off
        hwDf0Type.selectItem(withTag: config.df0.type.rawValue)
        hwDf1Connect.state = config.df1.connected ? .on : .off
        hwDf1Type.selectItem(withTag: config.df1.type.rawValue)
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

    @IBAction func hwDf0ConnectAction(_ sender: NSButton!) {
        
        amigaProxy?.configureDrive(0, connected: sender.state == .on)
        refresh()
    }

    @IBAction func hwDf0TypeAction(_ sender: NSPopUpButton!) {
        
        amigaProxy?.configureDrive(0, type: sender.selectedTag())
        refresh()
    }

    @IBAction func hwDf1ConnectAction(_ sender: NSButton!) {
        
        amigaProxy?.configureDrive(1, connected: sender.state == .on)
        myAppDelegate.df1Menu.isHidden = sender.state == .off
        refresh()
    }
    
    @IBAction func hwDf1TypeAction(_ sender: NSPopUpButton!) {
        
        amigaProxy?.configureDrive(1, type: sender.selectedTag())
        refresh()
    }

    @IBAction func hwUnlockAction(_ sender: Any!) {
        
        amigaProxy?.powerOff()
        refresh()
    }
    
    @IBAction func hwFactorySettingsAction(_ sender: NSPopUpButton!) {
        
        track("\(sender.selectedTag())")
        
        switch sender.selectedTag() {
            
        case A500.rawValue:
            
            amigaProxy?.configureModel(Defaults.a500.amigaModel.rawValue)
            amigaProxy?.configureLayout(Layout.us.rawValue)
            amigaProxy?.configureRealTimeClock(Defaults.a500.realTimeClock)
            
            amigaProxy?.configureChipMemory(Defaults.a500.chipRam)
            amigaProxy?.configureSlowMemory(Defaults.a500.slowRam)
            amigaProxy?.configureFastMemory(Defaults.a500.fastRam)
            
            amigaProxy?.configureDrive(0, connected: Defaults.a500.df0Connect)
            amigaProxy?.configureDrive(0, type:      Defaults.a500.df0Type.rawValue)
            amigaProxy?.configureDrive(1, connected: Defaults.a500.df1Connect)
            amigaProxy?.configureDrive(1, type:      Defaults.a500.df1Type.rawValue)
            
        case A1000.rawValue:
            
            amigaProxy?.configureModel(Defaults.a1000.amigaModel.rawValue)
            amigaProxy?.configureLayout(Layout.us.rawValue)
            amigaProxy?.configureRealTimeClock(Defaults.a1000.realTimeClock)
            
            amigaProxy?.configureChipMemory(Defaults.a1000.chipRam)
            amigaProxy?.configureSlowMemory(Defaults.a1000.slowRam)
            amigaProxy?.configureFastMemory(Defaults.a1000.fastRam)
            
            amigaProxy?.configureDrive(0, connected: Defaults.a1000.df0Connect)
            amigaProxy?.configureDrive(0, type:      Defaults.a1000.df0Type.rawValue)
            amigaProxy?.configureDrive(1, connected: Defaults.a1000.df1Connect)
            amigaProxy?.configureDrive(1, type:      Defaults.a1000.df1Type.rawValue)
            
        case A2000.rawValue:
            
            amigaProxy?.configureModel(Defaults.a2000.amigaModel.rawValue)
            amigaProxy?.configureLayout(Layout.us.rawValue)
            amigaProxy?.configureRealTimeClock(Defaults.a2000.realTimeClock)
            
            amigaProxy?.configureChipMemory(Defaults.a2000.chipRam)
            amigaProxy?.configureSlowMemory(Defaults.a2000.slowRam)
            amigaProxy?.configureFastMemory(Defaults.a2000.fastRam)
            
            amigaProxy?.configureDrive(0, connected: Defaults.a2000.df0Connect)
            amigaProxy?.configureDrive(0, type:      Defaults.a2000.df0Type.rawValue)
            amigaProxy?.configureDrive(1, connected: Defaults.a2000.df1Connect)
            amigaProxy?.configureDrive(1, type:      Defaults.a2000.df1Type.rawValue)
            
        default:
            track("Cannot restore factory defaults (unknown Amiga model).")
        }
        
        refresh()
    }
}
