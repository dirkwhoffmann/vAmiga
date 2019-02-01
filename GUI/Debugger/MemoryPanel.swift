// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension Inspector {
    
    var bank : Int {
        get {
            return bankTableView.bank
        }
        set {
            if newValue >= 0 && newValue <= 255 {
                
                track("Switching to bank \(newValue)")
                bankTableView.bank = newValue
                memBankField.integerValue = newValue
                memBankStepper.integerValue = newValue
                memTableView.scrollRowToVisible(0)
                bankTableView.scrollRowToVisible(newValue)
                // bankTableView.selectRowIndexes([newValue], byExtendingSelection: false)
                refreshMemory()
            }
        }
    }
    
    func refreshMemory() {
        
        track("\(bank)")
        
        memBankField.integerValue = bank
        memBankStepper.integerValue = bank
        
        switch bank {
        case 0x00 ... 0x1F:
            memAreaPopup.selectItem(withTag: Int(MEM_CHIP.rawValue))
            
        case 0x20 ... 0x9F:
            memAreaPopup.selectItem(withTag: Int(MEM_FAST.rawValue))
            
        case 0xA0 ... 0xBF:
            memAreaPopup.selectItem(withTag: Int(MEM_CIA.rawValue))
            
        case 0xC0 ... 0xC7:
            memAreaPopup.selectItem(withTag: Int(MEM_SLOW.rawValue))
            
        case 0xDC ... 0xDE:
            memAreaPopup.selectItem(withTag: Int(MEM_RTC.rawValue))
            
        case 0xDF:
            memAreaPopup.selectItem(withTag: Int(MEM_CUSTOM.rawValue))
            
        case 0xF8 ... 0xFF:
            memAreaPopup.selectItem(withTag: Int(MEM_ROM.rawValue))

        default:
            memAreaPopup.selectItem(withTag: Int(MEM_UNMAPPED.rawValue))
        }
        
        memTableView.refresh()
    }
    
    @IBAction func memBankAction(_ sender: NSTextField!) {
     
        track("\(sender.integerValue)")
        bank = sender.integerValue
    }
    
    @IBAction func memStepperAction(_ sender: NSStepper!) {
        
        track("\(sender.integerValue)")
        bank = sender.integerValue
    }
    
    @IBAction func memAreaAction(_ sender: NSPopUpButton!) {
        
        switch (UInt32(sender.selectedTag())) {
            
        case MEM_CHIP.rawValue:   bank = 0x00
        case MEM_FAST.rawValue:   bank = 0x20
        case MEM_CIA.rawValue:    bank = 0xA0
        case MEM_SLOW.rawValue:   bank = 0xC0
        case MEM_RTC.rawValue:    bank = 0xDC
        case MEM_CUSTOM.rawValue: bank = 0xDF
        case MEM_ROM.rawValue:    bank = 0xFC
            
        default: fatalError()
        }
    }
    
    @IBAction func memSearchAction(_ sender: NSTextField!) {
        
        let input = sender.stringValue
        track("Going to address \(input)")
        
        guard let addr = Int(input, radix: 16) else {
            track("INVALID")
            return
        }
        
        bank = addr / 65536
        let row = (addr / 16) % 4096
        memTableView.scrollRowToVisible(row)
        memTableView.selectRowIndexes([row], byExtendingSelection: false)
        
        refreshMemory()
    }
}
