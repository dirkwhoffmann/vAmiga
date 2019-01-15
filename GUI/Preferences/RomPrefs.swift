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
    "This 32 KB Rom contains the Kickstart loader.",
    0x20765FEA67A8762D:
    "Amiga 1000 Boot Rom 252179-01",
    0x20765FEA67A8762E:
    "Amiga 1000 Boot Rom 252180-01"
]

let knownKickRoms : [UInt64 : String] = [
    0x0000000000000000:
    "This 256 KB or 512 KB Rom contains the Operating System.",
    0xFB166E49AF709AB8:
    "Kickstart 1.2",
    0xFB166E49AF709AB9:
    "Kickstart 1.3",
    0xFB166E49AF709ABA:
    "Kickstart 1.2 (512 KB)",
    0xFB166E49AF709ABB:
    "Kickstart 1.3 (512 KB)"
]


extension PreferencesController {

    func refreshRomTab() {
        
        guard let controller = myController else { return }
        guard let amiga = amigaProxy else { return }
        let config = amiga.config()
        
        track()
        
        let hasBoot      = amiga.hasBootRom()
        let hasKick      = amiga.hasKickRom()
        
        let bootURL      = controller.bootRomURL
        let kickURL      = controller.kickRomURL
        
        let bootHash     = amiga.bootRomFingerprint()
        let kickHash     = amiga.kickRomFingerprint()
        
        let romPresent   = NSImage.init(named: "rom")
        let romMissing   = NSImage.init(named: "rom_light")

        // Missing Roms
        let ready = amiga.readyToPowerUp()
        if (!ready) {
            romHeaderImage.image = NSImage.init(named: "NSCaution")
            if config.model == A1000 {
                romHeaderText.stringValue = "The selected Amiga model requires a Boot Rom to power up."
            } else {
                romHeaderText.stringValue = "The selected Amiga model requires a Kickstart Rom to run."
            }
        }
        romHeaderImage.isHidden   = !ready
        romHeaderText.isHidden    = !ready
        romHeaderSubText.isHidden = !ready
        
        // Boot Rom
        romBootHashText.isHidden  = !hasBoot
        romBootPathText.isHidden  = !hasBoot
        romBootButton.isHidden    = !hasBoot
        romBootCopyright.isHidden = !hasBoot
        romBootDropView.image = hasBoot ? romPresent : romMissing
        romBootHashText.stringValue = String(format: "Hash: %llX", bootHash)
        romBootPathText.stringValue = bootURL.absoluteString
        if let description = knownBootRoms[bootHash] {
            romBootDescription.stringValue = description
            romBootDescription.textColor = bootHash == 0 ? .secondaryLabelColor : .textColor
        } else {
            romBootDescription.stringValue = "An unknown, possibly patched Boot ROM."
            romBootDescription.textColor = .red
        }
        
        // Kickstart Rom
        romKickHashText.isHidden  = !hasKick
        romKickPathText.isHidden  = !hasKick
        romKickButton.isHidden    = !hasKick
        romKickCopyright.isHidden = !hasKick
        romKickDropView.image = hasKick ? romPresent : romMissing
        romKickHashText.stringValue = String(format: "Hash: %llX", kickHash)
        romKickPathText.stringValue = kickURL.absoluteString
        if let description = knownKickRoms[kickHash] {
            romKickDescription.stringValue = description
            romKickDescription.textColor = kickHash == 0 ? .secondaryLabelColor : .textColor
        } else {
            romKickDescription.stringValue = "An unknown, possibly patched Kickstart ROM."
            romKickDescription.textColor = .red
        }
    }

    
    //
    // Action methods
    //
    
    @IBAction func romDeleteBootAction(_ sender: Any!)
    {
        myController?.bootRomURL = URL(fileURLWithPath: "/")
        amigaProxy?.deleteBootRom()
        refresh()
    }
    
    @IBAction func romDeleteKickAction(_ sender: Any!)
    {
        myController?.kickRomURL = URL(fileURLWithPath: "/")
        amigaProxy?.deleteKickRom()
        refresh()
    }
}
