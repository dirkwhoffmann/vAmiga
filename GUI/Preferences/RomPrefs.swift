// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

struct Rom {
    
    static let missing             = 0x0000000000000000 as UInt64
    
    // Boot Roms
    static let boot_252179_01      = 0xa160593cffcbb233 as UInt64
    static let boot_252180_01      = 0xa98647146962eb76 as UInt64
    static let boot_a1000_1985     = 0xe923584a55d5c10c as UInt64
    static let boot_a1000          = 0x55bee0af674e7cfa as UInt64

    // Kickstart Roms
    static let aros                = 0xe74215eb368cd7f1 as UInt64
    
    static let kick12_33_180       = 0xe5cb7ee5200c4f0f as UInt64
    static let kick12_33_180_o     = 0xe3ff65d2c3a9b9e5 as UInt64

    static let kick13_35_5         = 0x047fb93fb8e383bc as UInt64
    static let kick13_35_5_b       = 0x42b199abf0febb13 as UInt64
    static let kick13_35_5_b2      = 0x482fa0f04538677b as UInt64
    static let kick13_35_5_b3      = 0xc9f1352946125739 as UInt64
    static let kick13_35_5_h       = 0x36890544db15eb40 as UInt64
    static let kick13_35_5_o       = 0x08a1122c7dec695d as UInt64

    static let kick204_37_175      = 0x845588ccf58fce86 as UInt64
}

    
let knownRoms : [UInt64 : String] = [

    // Rom.missing:             "",
    
    // Boot Roms
    Rom.boot_252179_01:      "Amiga 1000 Boot Rom (252179-01)",
    Rom.boot_252180_01:      "Amiga 1000 Boot Rom (252180-01)",
    Rom.boot_a1000_1985:     "Amiga 1000 Boot Rom (1985)",
    Rom.boot_a1000:          "Amiga 1000 Boot Rom",

    // Kickstart Roms
    Rom.aros:                "Free AROS Kickstart replacement",
    
    Rom.kick12_33_180:       "Kickstart 1.2 (revision 33.180)",
    Rom.kick12_33_180_o:     "Kickstart 1.2 (revision 33.180)",
    
    Rom.kick13_35_5:         "Kickstart 1.3 (revision 34.5)",
    Rom.kick13_35_5_b:       "Kickstart 1.3 (revision 34.5)",
    Rom.kick13_35_5_b2:      "Kickstart 1.3 (revision 34.5)",
    Rom.kick13_35_5_b3:      "Kickstart 1.3 (revision 34.5)",
    Rom.kick13_35_5_h:       "Kickstart 1.3 (revision 34.5)",
    Rom.kick13_35_5_o:       "Kickstart 1.3 (revision 34.5)",
    
    Rom.kick204_37_175:      "Kickstart 2.04 (revision 37.175)",
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
        
        let poweredOff   = amiga.isPoweredOff()
        let bootHash     = amiga.mem.bootRomFingerprint()
        let kickHash     = amiga.mem.kickRomFingerprint()
        let hash         = config.model == A1000 ? bootHash : kickHash

        track("bootHash = \(bootHash)")
        track("kickHash = \(kickHash)")
        track("hash = \(hash)")

        let hasRom        = hash != 0
        let hasArosRom    = hash == 0xE74215EB368CD7F1
        let hasKnownRom   = knownRoms[hash] != nil
        let hasUnknownRom = hasRom && !hasKnownRom
        let hasOrigRom    = hasKnownRom && !hasArosRom

        let bootRomURL    = controller.bootRomURL
        let kickRomURL    = controller.kickRomURL
        let url           = config.model == A1000 ? bootRomURL : kickRomURL
        
        let romMissing    = NSImage.init(named: "rom_light")
        let romOriginal   = NSImage.init(named: "rom_original")
        let romAros       = NSImage.init(named: "rom_aros")
        let romUnknown    = NSImage.init(named: "rom_unknown")
        
        // Lock controls if emulator is powered on
        romDropView.isEnabled = poweredOff
        romDeleteButton.isEnabled = poweredOff
        romFactoryButton.isEnabled = poweredOff
        
        // Rom icon
        romDropView.image =
            hasArosRom ? romAros :
            hasOrigRom ? romOriginal :
            hasRom     ? romUnknown : romMissing

        // Rom description
        var text = ""
        var subText = ""
        
        switch config.model {

        case A1000 where !hasRom:
            
            text = "The Amiga 1000 requires a Boot Rom to launch."
            subText = "Use drag and drop to add a Rom image."
            
        case A500 where !hasRom, A2000 where !hasRom:

            text = "The Amiga \(model) requires a Kickstart Rom to run."
            subText = "Use drag and drop to add a Rom image."

        default:
            
            text = knownRoms[hash] ?? "An unknown or unsupported Rom."
            subText = hasArosRom ? "Use original Amiga Roms for higher compatibility." : url.relativePath
        }
        
        romText.stringValue = text
        romText.textColor = hasRom ? .textColor : .systemRed
        romSubText.stringValue = subText
            
        romHash.isHidden = !hasUnknownRom
        romHash.stringValue = String(format: "Hash: %llx", hash)
        
        romDeleteButton.isHidden = !hasRom
        
        // Warning message
        if hasOrigRom {
            
            romWarning.isHidden = false
            romWarning.stringValue = "Please obey legal regulations. Original Amiga Roms are copyrighted."
            romWarning.textColor = .textColor

        } else {
            
            romWarning.isHidden = true
        }
        
        // Lock symbol and explanation
        romLockImage.isHidden = poweredOff
        romLockText.isHidden = poweredOff
        romLockSubText.isHidden = poweredOff
    }

    
    //
    // Action methods
    //

    @IBAction func romUnlockAction(_ sender: Any!)
    {
        amigaProxy?.powerOff()
        refresh()
    }
    
    @IBAction func romDeleteAction(_ sender: Any!)
    {
        if amigaProxy?.config().model == A1000 {
            
            myController?.bootRomURL = URL(fileURLWithPath: "/")
            amigaProxy?.mem.deleteBootRom()
            
        } else {
            
            myController?.kickRomURL = URL(fileURLWithPath: "/")
            amigaProxy?.mem.deleteKickRom()
        }
        
        refresh()
    }
    
    @IBAction func romFactorySettingsAction(_ sender: NSButton!)
    {
        // Remove Boot Rom
        amigaProxy?.mem.deleteBootRom()
        
        // Revert to the AROS Kickstart replacement
        amigaProxy?.mem.loadKickRom(fromBuffer: NSDataAsset(name: "aros.rom")?.data)
        myController?.kickRomURL = URL(fileURLWithPath: "")
        refresh()
    }
}
