//
// This file is part of VirtualC64 - A cycle accurate Commodore 64 emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
//

import Foundation

extension MyController {

    func setupDebugger() {
        
        // Create and assign binary number formatter
        let bF = MyFormatter.init(radix: 2, min: 0, max: 255)
        ciaPAbinary.formatter = bF
        ciaPRA.formatter = bF
        ciaDDRA.formatter = bF
        ciaPBbinary.formatter = bF
        ciaPRB.formatter = bF
        ciaDDRB.formatter = bF
        ciaIcrBinary.formatter = bF
        ciaImrBinary.formatter = bF
    }
    
    // Updates all visible values in the debug panel
    /*
    func refresh() {
        
        if let id = debugPanel.selectedTabViewItem?.identifier as? String {
            switch id {
                
            case "CIA":
                break
                
            case "VIC":
                break
                
            case "SID":
                refreshSID()
                break
                
            default:
                break
            }
        }
    }
    */
    
    func refreshFormatters(hex: Bool) {

        func assignFormatter(_ formatter: Formatter, _ controls: [NSControl]) {
            for control in controls {
                control.abortEditing()
                control.formatter = formatter
                control.needsDisplay = true
            }
        }
        
        // Create formatters
        let fmt3 = MyFormatter.init(radix: (hex ? 16 : 10), min: 0, max: 0x7)
        let fmt4 = MyFormatter.init(radix: (hex ? 16 : 10), min: 0, max: 0xF)
        let fmt8 = MyFormatter.init(radix: (hex ? 16 : 10), min: 0, max: 0xFF)
        let fmt9 = MyFormatter.init(radix: (hex ? 16 : 10), min: 0, max: 0x1FF)
        let fmt11 = MyFormatter.init(radix: (hex ? 16 : 10), min: 0, max: 0x7FF)
        let fmt12 = MyFormatter.init(radix: (hex ? 16 : 10), min: 0, max: 0xFFF)
        let fmt16 = MyFormatter.init(radix: (hex ? 16 : 10), min: 0, max: 0xFFFF)

        // Assign formatters
        assignFormatter(fmt3,
                        [])
        
        assignFormatter(fmt4,
                        [volume])
        
        assignFormatter(fmt8,
                        [ciaPA, ciaPB,
                         todHours, todMinutes, todSeconds, todTenth,
                         alarmHours, alarmMinutes, alarmSeconds, alarmTenth,
                         ciaImr, ciaIcr])
        assignFormatter(fmt8,
                        [])
    
        assignFormatter(fmt9,
                        [])

        assignFormatter(fmt11,
                        [])

        assignFormatter(fmt12,
                        [])

        assignFormatter(fmt16,
                        [ciaTimerA, ciaLatchA, ciaTimerB, ciaLatchB
                        ])
        

        // refresh()
    }
    
    func setUserEditing(_ enabled: Bool) {
        
        let controls:[NSControl] = [
            
            // CIA panel
            ciaPRA, ciaPRB, ciaDDRA, ciaDDRB, ciaLatchA, ciaLatchB,
            
            // Audio panel
            volume,
            
            // Video panel
        ]
  
        for control in controls {
            control.isEnabled = enabled
        }
    
        // Enable / disable table columns
        /*
        let columns = ["hex0", "hex1", "hex2", "hex3"]
        
        for column in columns {
            let columnId = NSUserInterfaceItemIdentifier(rawValue: column)
            if let tableColumn = memTableView.tableColumn(withIdentifier: columnId) {
                tableColumn.isEditable = enabled
            }
        }
        */
        
    // Change image and state of debugger control buttons
        if !amiga.readyToPowerUp() {
            stopAndGoButton.image = NSImage.init(named: "continueTemplate")
            stopAndGoButton.toolTip = "Run"
            stopAndGoButton.isEnabled = false
            stepIntoButton.isEnabled = false
            stepOverButton.isEnabled = false
        } else if amiga.isPaused() {
            stopAndGoButton.image = NSImage.init(named: "continueTemplate")
            stopAndGoButton.toolTip = "Run"
            stopAndGoButton.isEnabled = true
            stepIntoButton.isEnabled = true
            stepOverButton.isEnabled = true
        } else {
            stopAndGoButton.image = NSImage.init(named: "pauseTemplate")
            stopAndGoButton.toolTip = "Pause"
            stopAndGoButton.isEnabled = true
            stepIntoButton.isEnabled = false
            stepOverButton.isEnabled = false
        }
    }
    
    func enableUserEditing() {
        setUserEditing(true)
    }

    func disableUserEditing() {
        setUserEditing(false)
    }
}

extension MyController : NSTabViewDelegate {

    func tabView(_ tabView: NSTabView, didSelect tabViewItem: NSTabViewItem?) {
        // refresh()
    }
}
