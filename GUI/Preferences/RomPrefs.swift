// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

let knownRoms : [UInt64 : String] = [
    0x0000000000000000:
    "",
    0xE74215EB368CD7F1:
    "AROS Kickstart replacement",
    0xA160593CFFCBB233:
    "Amiga 1000 Boot Rom 252179-01",
    0xA98647146962EB76:
    "Amiga 1000 Boot Rom 252180-01",
    0xE5CB7EE5200C4F0F:
    "Kickstart 1.2",
    0x047FB93FB8E383BC:
    "Kickstart 1.3",
    0xE3FF65D2C3A9B9E5:
    "Kickstart 1.2 (512 KB)",
    0x08A1122C7DEC695D:
    "Kickstart 1.3 (512 KB)",
    0x845588CCF58FCE86:
    "Kickstart 2.0 (512 KB)",
    0x72126DEF5AF27DCB:
    "Kickstart 3.0 (512 KB)",
]

let originalRoms : [UInt64] = [
    0xA160593CFFCBB233,
    0xA98647146962EB76,
    0xE5CB7EE5200C4F0F,
    0x047FB93FB8E383BC,
    0xE3FF65D2C3A9B9E5,
    0x08A1122C7DEC695D,
    0x845588CCF58FCE86,
    0x72126DEF5AF27DCB,
]

let incompatibleRoms : [UInt64] = [
    0x72126DEF5AF27DCB,
]

extension PreferencesController {

    func refreshRomTab() {
        
        guard let controller = myController else { return }
        guard let amiga = amigaProxy else { return }
        
        track()
        let config = amiga.config()
        
        let model =
            config.model == A1000 ? "1000" :
                config.model == A500 ? "500" : "2000"
        
        let running      = amiga.isRunning()
        let bootHash     = amiga.bootRomFingerprint()
        let kickHash     = amiga.kickRomFingerprint()
        let hash         = config.model == A1000 ? bootHash : kickHash
        
        let hasRom       = hash != 0
        let hasOrigRom   = originalRoms.contains(hash)
        let hasArosRom   = hash == 0xE74215EB368CD7F1
        let hasKnownRom  = knownRoms[hash] != nil

        let bootRomURL   = controller.bootRomURL
        let kickRomURL   = controller.kickRomURL
        let url          = config.model == A1000 ? bootRomURL : kickRomURL
        
        let romMissing   = NSImage.init(named: "rom_light")
        let romOriginal  = NSImage.init(named: "rom_original")
        let romAros      = NSImage.init(named: "rom_aros")
        let romUnknown   = NSImage.init(named: "rom_unknown")
        
        // Lock controls if emulator is running
        romDropView.isEnabled = !running
        romDeleteButton.isEnabled = !running
        romFactoryButton.isEnabled = !running
        
        // Rom icon
        romDropView.image =
            hasArosRom ? romAros :
            hasOrigRom ? romOriginal :
            hasRom     ? romUnknown : romMissing

        // Rom description
        var text = ""
        var subText = ""
        var warnText = ""
        
        switch config.model {

        case A1000 where !hasRom:
            
            text = "The Amiga 1000 requires a Boot Rom to launch."
            subText = "Use drag and drop to add a Rom image."
            
        case A500 where !hasRom, A2000 where !hasRom:

            text = "The Amiga \(model) requires a Kickstart Rom to run."
            subText = "Use drag and drop to add a Rom image."

        default:
            
            text = knownRoms[hash] ?? "An unknown, possibly patched Rom."
            subText = hasArosRom ? "Use original Amiga Roms for higher compatibility." : url.relativePath
        }
        
        romText.stringValue = text
        romText.textColor = hasRom ? .textColor : .systemRed
        romSubText.stringValue = subText
            
        romHash.isHidden = hasKnownRom
        romHash.stringValue = String(format: "Hash: %llx", hash)
        
        romDeleteButton.isHidden = !hasRom
        
        // Warning message
        if incompatibleRoms.contains(hash) {
            
            romWarning.isHidden = false
            romWarning.stringValue = "The selected kickstart is not compatible with an Amiga \(model)."
            romWarning.textColor = .systemRed
            
        } else if originalRoms.contains(hash) {
            
            romWarning.isHidden = false
            romWarning.stringValue = "Please obey legal regulations. Original Amiga Roms are copyrighted."
            romWarning.textColor = .textColor

        } else {
            
            romWarning.isHidden = true
        }
        
        // Lock symbol and explanation
        romLockImage.isHidden = !running
        romLockText.isHidden = !running
        romLockSubText.isHidden = !running
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
    
    @IBAction func romDeleteAction(_ sender: Any!)
    {
        if amigaProxy?.config().model == A1000 {
            
            myController?.bootRomURL = URL(fileURLWithPath: "/")
            amigaProxy?.deleteBootRom()
            
        } else {
            
            myController?.kickRomURL = URL(fileURLWithPath: "/")
            amigaProxy?.deleteKickRom()
        }
        
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
