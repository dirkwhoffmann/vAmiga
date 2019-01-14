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
        
        // Machine
        hwAmigaModelPopup.selectItem(withTag: config.model.rawValue)
        hwRealTimeClock.state = config.realTimeClock ? .on : .off
        
        // Memory
        hwChipRamPopup.selectItem(withTag: config.chipRamSize)
        hwSlowRamPopup.selectItem(withTag: config.slowRamSize)
        hwFastRamPopup.selectItem(withTag: config.fastRamSize)

        // Drive
        hwDf0Connect.state = config.df0.connected ? .on : .off
        hwDf0Type.selectItem(withTag: config.df0.type.rawValue)
        hwDf1Connect.state = config.df1.connected ? .on : .off
        hwDf1Type.selectItem(withTag: config.df1.type.rawValue)
        hwDf2Connect.state = config.df2.connected ? .on : .off
        hwDf2Type.selectItem(withTag: config.df2.type.rawValue)
        hwDf3Connect.state = config.df3.connected  ? .on : .off
        hwDf3Type.selectItem(withTag: config.df3.type.rawValue)
        
    }
    
    @IBAction func hwVicModelAction(_ sender: NSPopUpButton!) {
        
        proxy?.vic.setModel(sender.selectedTag())
        refresh()
    }
    
    @IBAction func hwVicGrayDotBugAction(_ sender: NSButton!) {
        
        proxy?.vic.setEmulateGrayDotBug(sender.state == .on)
        refresh()
    }
    
    @IBAction func hwCiaModelAction(_ sender: NSPopUpButton!) {
        
        proxy?.cia1.setModel(sender.selectedTag())
        proxy?.cia2.setModel(sender.selectedTag())
        refresh()
    }
    
    @IBAction func hwCiaTimerBBugAction(_ sender: NSButton!) {
        
        proxy?.cia1.setEmulateTimerBBug(sender.state == .on)
        proxy?.cia2.setEmulateTimerBBug(sender.state == .on)
        refresh()
    }
    
    @IBAction func hwSidFilterAction(_ sender: NSButton!) {
        
        proxy?.sid.setAudioFilter(sender.state == .on)
        refresh()
    }
    
    @IBAction func hwSidEngineAction(_ sender: NSPopUpButton!) {
        
        proxy?.sid.setReSID(sender.selectedTag() == 1)
        refresh()
    }
    
    @IBAction func hwSidSamplingAction(_ sender: NSPopUpButton!) {
        
        proxy?.sid.setSamplingMethod(sender.selectedTag())
        refresh()
    }
    
    @IBAction func hwSidModelAction(_ sender: NSPopUpButton!) {
        
        proxy?.sid.setModel(sender.selectedTag())
        refresh()
    }
    
    @IBAction func hwGlueLogicAction(_ sender: NSPopUpButton!) {
        
        proxy?.vic.setGlueLogic(sender.selectedTag())
        refresh()
    }
    
    @IBAction func hwRamInitPatternAction(_ sender: NSPopUpButton!) {
        
        proxy?.mem.setRamInitPattern(sender.selectedTag())
        refresh()
    }

    @IBAction func hwFactorySettingsAction(_ sender: NSPopUpButton!) {
        
        if sender.selectedTag() != C64_CUSTOM.rawValue {
            proxy?.setModel(sender.selectedTag())
        }
        refresh()
    }
}
