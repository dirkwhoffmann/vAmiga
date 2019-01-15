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
    "This Rom contains the Kickstart disk loader.",
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
    "This Rom contains the Amiga Operation System.",
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
        
        // let running      = amiga.isRunning()
        let running      = proxy?.isRunning() ?? false
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

        // Lock controls if emulator is running
        romBootDropView.isEnabled = !running
        romBootDeleteButton.isEnabled = !running
        romKickDropView.isEnabled = !running
        romKickDeleteButton.isEnabled = !running
        
        // Check for missing Roms
        let ready = amiga.readyToPowerUp()
        if (running) {
            romLockImage.image = NSImage.init(named: "Lock")
            romLockText.stringValue = "The Rom settings are locked because the emulator is running."
            romLockSubText.stringValue = "Click to power down and unlock."
        } else {
            romLockImage.image = NSImage.init(named: "NSCaution")
            if config.model == A1000 {
                romLockText.stringValue = "The selected Amiga model requires a Boot Rom to run."
            } else {
                romLockText.stringValue = "The selected Amiga model requires a Kickstart Rom to run."
            }
            romLockSubText.stringValue = "Use drag and drop to add a Rom image."
        }
        romLockImage.isHidden = !running && ready
        romLockText.isHidden = !running && ready
        romLockSubText.isHidden = !running && ready
        
        // Boot Rom
        romBootTitle.textColor = hasBoot ? .textColor : .secondaryLabelColor
        romBootText.textColor = hasBoot ? .textColor : .secondaryLabelColor
        romBootHash.isHidden = !hasBoot
        romBootPath.isHidden = !hasBoot
        romBootDeleteButton.isHidden = !hasBoot
        romBootSubText.isHidden = true
        
        romBootDropView.image =
            hasOrigBoot ? romOriginal :
            hasBoot     ? romUnknown : romMissing
            
        romBootHash.stringValue = String(format: "Hash: %llX", bootHash)
        romBootPath.stringValue = bootURL.relativePath
        if let description = knownBootRoms[bootHash] {
            romBootText.stringValue = description
        } else {
            romBootText.stringValue = "An unknown, possibly patched Boot ROM."
            romBootText.textColor = .red
        }
        
        // Kickstart Rom
        romKickTitle.textColor = hasKick ? .textColor : .secondaryLabelColor
        romKickText.textColor = hasKick ? .textColor : .secondaryLabelColor
        romKickHash.isHidden = !hasKick
        romKickPath.isHidden  = !hasKick
        romKickDeleteButton.isHidden = !hasKick
        romKickSubText.isHidden = true
        
        romKickDropView.image =
            hasOrigKick ? romOriginal :
            hasAros     ? romAros :
            hasKick     ? romUnknown : romMissing
        
        romKickHash.stringValue = String(format: "Hash: %llX", kickHash)
        romKickPath.stringValue = kickURL.relativePath
        if let description = knownKickRoms[kickHash] {
            romKickText.stringValue = description
        } else {
            romKickText.stringValue = "An unknown, possibly patched Kickstart ROM."
            romKickText.textColor = .red
        }
    }

    
    //
    // Action methods
    //

    @IBAction func romUnlockAction(_ sender: Any!)
    {
        amigaProxy?.powerOff()
        proxy?.halt()
        refresh()
    }
    
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
    
    @IBAction func romFactorySettingsAction(_ sender: NSButton!)
    {
        // Remove Boot Rom
        amigaProxy?.deleteBootRom()
        
        // Revert to the AROS Kickstart replacement
        amigaProxy?.loadKickRom(fromBuffer: NSDataAsset(name: "aros.rom")?.data)
        myController?.kickRomURL = URL(fileURLWithPath: "")
        refresh()
    }
}
