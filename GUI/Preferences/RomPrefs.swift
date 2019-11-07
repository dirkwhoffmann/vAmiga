// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// swiftlint:disable colon

struct Rom {
    
    static let missing             = 0x0000000000000000 as UInt64
    
    // Boot Roms
    static let boot_a1000_1985_8k  = 0xe923584a55d5c10c as UInt64
    static let boot_a1000_1985_64k = 0x83b3ec51d394e10c as UInt64

    // Kickstart Roms
    static let aros                = 0x23de891c17754732 as UInt64
    
    static let kick12_33_180       = 0xe5cb7ee5200c4f0f as UInt64
    static let kick12_33_180_o     = 0xe3ff65d2c3a9b9e5 as UInt64

    static let kick13_35_5         = 0x047fb93fb8e383bc as UInt64
    static let kick13_35_5_b       = 0x42b199abf0febb13 as UInt64
    static let kick13_35_5_b2      = 0x482fa0f04538677b as UInt64
    static let kick13_35_5_b3      = 0xc9f1352946125739 as UInt64
    static let kick13_35_5_h       = 0x36890544db15eb40 as UInt64
    static let kick13_35_5_o       = 0x08a1122c7dec695d as UInt64

    static let kick204_37_175      = 0x845588ccf58fce86 as UInt64

    static let kick31_40_63        = 0xd059a4095ef1d70d as UInt64
    static let kick31_40_63_a      = 0xd87e84eeacc77dcd as UInt64
    static let kick31_40_63_b      = 0xd87e84eeacc77aaa as UInt64

    static let kick314_46_143      = 0x2005bba6e9c6c6ac as UInt64

    static let diag11              = 0x3caee2ad138eb229 as UInt64
    static let logica20            = 0x110c854766f14cd8 as UInt64
}

let knownRoms: [UInt64: String] = [

    // Boot Roms
    Rom.boot_a1000_1985_8k:  "Amiga 1000 Boot Rom (1985)",
    Rom.boot_a1000_1985_64k: "Amiga 1000 Boot Rom (1985)",

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

    Rom.kick31_40_63:        "Kickstart 3.1 (revision 40.63)",
    Rom.kick31_40_63_a:      "Kickstart 3.1 (revision 40.63)",
    Rom.kick31_40_63_b:      "Kickstart 3.1 (revision 40.63)",

    Rom.kick314_46_143:      "Kickstart 3.1.4 (revision 46.143)",
    
    Rom.diag11:              "DiagROM 1.1",
    Rom.logica20:            "The Diagnostic V2.0 (Logica)"
]

extension PreferencesController {

    func refreshRomTab() {
        
        guard let controller = myController else { return }
        guard let amiga = amigaProxy else { return }
        
        track()
        let config = amiga.config()
        
        let model =
            config.model == AMIGA_1000 ? "1000" :
                config.model == AMIGA_500 ? "500" : "2000"
        
        let poweredOff   = amiga.isPoweredOff()
        let hash         = amiga.mem.romFingerprint()
        let extHash      = amiga.mem.extRomFingerprint()

        track("romHash = \(hash)")
        track("extHash = \(extHash)")

        let hasRom        = hash != 0
        let hasArosRom    = hash == Rom.aros
        let hasDiagRom    = hash == Rom.diag11 || hash == Rom.logica20
        let hasKnownRom   = knownRoms[hash] != nil
        let hasUnknownRom = hasRom && !hasKnownRom
        let hasOrigRom    = hasKnownRom && !hasArosRom && !hasDiagRom

        let romURL        = controller.romURL

        let romMissing    = NSImage.init(named: "rom_light")
        let romOriginal   = NSImage.init(named: "rom_original")
        let romAros       = NSImage.init(named: "rom_aros")
        let romDiag       = NSImage.init(named: "rom_diag")
        let romUnknown    = NSImage.init(named: "rom_unknown")
        
        // Lock controls if emulator is powered on
        romDropView.isEnabled = poweredOff
        romDeleteButton.isEnabled = poweredOff
        romFactoryButton.isEnabled = poweredOff
        
        // Rom icon
        romDropView.image =
            hasArosRom ? romAros :
            hasDiagRom ? romDiag :
            hasOrigRom ? romOriginal :
            hasRom     ? romUnknown : romMissing

        // Rom description
        var text = ""
        var subText = ""
        
        switch config.model {

        case AMIGA_1000 where !hasRom:
            
            text = "The Amiga 1000 requires a Boot Rom to launch."
            subText = "Use drag and drop to add a Rom image."
            
        case AMIGA_500 where !hasRom, AMIGA_2000 where !hasRom:

            text = "The Amiga \(model) requires a Kickstart Rom to run."
            subText = "Use drag and drop to add a Rom image."

        default:
            
            text = knownRoms[hash] ?? "An unknown or unsupported Rom."
            subText = hasArosRom ? "Use original Amiga Roms for higher compatibility." : romURL.relativePath
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

        // Label the OK button
        romOKButton.title = okLabel
    }

    //
    // Action methods
    //

    @IBAction func romDeleteAction(_ sender: Any!) {

        myController?.romURL = URL(fileURLWithPath: "/")
        amigaProxy?.mem.deleteRom()
        
        refresh()
    }
    
    @IBAction func romFactorySettingsAction(_ sender: NSButton!) {

        // Revert to the AROS Kickstart replacement
        amigaProxy?.mem.loadRom(fromBuffer: NSDataAsset(name: "aros-amiga-m68k-rom")?.data)
        myController?.romURL = URL(fileURLWithPath: "")
        amigaProxy?.mem.loadExtRom(fromBuffer: NSDataAsset(name: "aros-amiga-m68k-ext")?.data)
        myController?.extURL = URL(fileURLWithPath: "")
        refresh()
    }
}
