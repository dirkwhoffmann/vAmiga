// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

let knownBootRoms : [UInt64 : String] = [
    0x0000000000000000:
    "This 64 KB Rom contains the Amiga Boot Rom. Pay attention to copyright restrictions when adding an original Rom.",
    0x20765FEA67A8762D:
    "Amiga 1000 Boot Rom V???"
]

let knownKickstartRoms : [UInt64 : String] = [
    0x0000000000000000:
    "This 256 KB Rom contains the Amiga Operating System. Pay attention to copyright restrictions when adding an original Rom.",
    0xFB166E49AF709AB8:
    "Kickstart 1.2 V???",
    0x4232D81CCD24FAAE:
    "Kickstart 1.3 V???"
]


extension PreferencesController {

    func refreshRomTab() {
        
        guard let controller = myController else { return }
        guard let amiga = amigaProxy else { return }
        let config = amiga.config()
        
        track()
        
        let romImage = NSImage.init(named: "rom")
        let romImageLight = NSImage.init(named: "rom_light")
        
        // Gather information about Roms
        let hasBootRom = false // amiga.bootRom != nil
        let hasKickstartRom = false // amiga.kickstartRom != nil
       
        let bootURL = "Lorem ipsum" // amiga.bootRomURL
        let kickstartURL = "Lorem ipsum"
        
        let bootHash = UInt64(0) //
        let kickstartHash = UInt64(0) // c64.kernalRomFingerprint()
        
        // Header image and description
        if amiga.readyToPowerUp() {
            /*
            romHeaderImage.image = NSImage.init(named: "AppIcon")
            romHeaderText.stringValue = "vAmiga is ready to run."
            romHeaderSubText.stringValue = ""
            romOkButton.title = "OK"
             */
        } else {
            /*
            romHeaderImage.image = NSImage.init(named: "NSCaution")
            romHeaderText.stringValue = "vAmiga cannot run."
            romHeaderSubText.stringValue = "Use drag and drop to add ROM images."
            romOkButton.title = "Quit"
             */
        }
        
        // Boot Rom
        romBootDropView.image = hasBootRom ? romImage : romImageLight
        romBootHashText.isHidden = !hasBootRom
        romBootHashText.stringValue = String(format: "Hash: %llX", bootHash)
        romBootPathText.isHidden = !hasBootRom
        romBootPathText.stringValue = bootURL
        romBootButton.isHidden = !hasBootRom
        if let description = knownBootRoms[bootHash] {
            romBootDescription.stringValue = description
            romBootDescription.textColor = bootHash == 0 ? .secondaryLabelColor : .textColor
        } else {
            romBootDescription.stringValue = "An unknown, possibly patched Boot ROM."
            romBootDescription.textColor = .red
        }
        
        // Kickstart Rom
        romKickstartDropView.image = hasKickstartRom ? romImage : romImageLight
        romKickstartHashText.isHidden = !hasKickstartRom
        romKickstartHashText.stringValue = String(format: "Hash: %llX", kickstartHash)
        romKickstartPathText.isHidden = !hasKickstartRom
        romKickstartPathText.stringValue = kickstartURL
        romKickstartButton.isHidden = !hasKickstartRom
        if let description = knownKickstartRoms[kickstartHash] {
            romKickstartDescription.stringValue = description
            romKickstartDescription.textColor = kickstartHash == 0 ? .secondaryLabelColor : .textColor
        } else {
            romKickstartDescription.stringValue = "An unknown, possibly patched Kickstart ROM."
            romKickstartDescription.textColor = .red
        }
    }

    
    //
    // Action methods
    //
    
    @IBAction func romDeleteBootAction(_ sender: Any!)
    {
        myController?.bootRomURL = URL(fileURLWithPath: "/")
        // proxy?.halt()
        // proxy?.mem.deleteBasicRom()
        refresh()
    }
    
    @IBAction func romDeleteKickstartAction(_ sender: Any!)
    {
        myController?.kickstartRomURL = URL(fileURLWithPath: "/")
        // proxy?.halt()
        // proxy?.mem.deleteCharacterRom()
        refresh()
    }
}
