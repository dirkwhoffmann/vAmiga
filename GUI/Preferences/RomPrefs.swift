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
    "A Boot Rom is required for emulating an Amiga 1000.",
    0xA160593CFFCBB233:
    "Amiga 1000 Boot Rom 252179-01",
    0xA98647146962EB76:
    "Amiga 1000 Boot Rom 252180-01"
]

let originalBootRoms : [UInt64] = [
    0xA160593CFFCBB233,
    0xA98647146962EB76
]

let knownKickRoms : [UInt64 : String] = [
    0x0000000000000000:
    "A Kickstart Rom is required for emulating an Amiga 500 or 2000.",
    0xE5CB7EE5200C4F0F:
    "Kickstart 1.2",
    0x047FB93FB8E383BC:
    "Kickstart 1.3",
    0xE3FF65D2C3A9B9E5:
    "Kickstart 1.2 (512 KB)",
    0x08A1122C7DEC695D:
    "Kickstart 1.3 (512 KB)",
    0xE74215EB368CD7F1:
    "AROS Kickstart replacement"
]

let originalKickRoms : [UInt64] = [
    0xE5CB7EE5200C4F0F,
    0x047FB93FB8E383BC,
    0xE3FF65D2C3A9B9E5,
    0x08A1122C7DEC695D,
]

extension PreferencesController {

    func refreshRomTab() {
        
        guard let controller = myController else { return }
        guard let amiga = amigaProxy else { return }
        let config = amiga.config()
        
        track()
        
        let bootHash     = amiga.bootRomFingerprint()
        let kickHash     = amiga.kickRomFingerprint()

        let hasBoot      = bootHash != 0
        let hasKick      = kickHash != 0
        let hasOrigBoot  = originalBootRoms.contains(bootHash)
        let hasOrigKick  = originalKickRoms.contains(kickHash)
        let hasAros      = kickHash == 0xE74215EB368CD7F1
        
        let bootURL      = controller.bootRomURL
        let kickURL      = controller.kickRomURL
    
        let romMissing   = NSImage.init(named: "rom_light")
        let romOriginal  = NSImage.init(named: "rom_original")
        let romAros      = NSImage.init(named: "rom_aros")
        let romUnknown   = NSImage.init(named: "rom_unknown")

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
        romHeaderImage.isHidden   = ready
        romHeaderText.isHidden    = ready
        romHeaderSubText.isHidden = ready
        
        // Boot Rom
        romBootHashText.isHidden  = !hasBoot
        romBootPathText.isHidden  = !hasBoot
        romBootButton.isHidden    = !hasBoot
        romBootCopyright.isHidden = !hasOrigBoot
        
        romBootDropView.image =
            hasOrigBoot ? romOriginal :
            hasBoot     ? romUnknown : romMissing
            
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
        romKickCopyright.isHidden = !hasOrigKick
        
        romKickDropView.image =
            hasOrigKick ? romOriginal :
            hasAros     ? romAros :
            hasKick     ? romUnknown : romMissing
        
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
