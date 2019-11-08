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
        // let config = amiga.config()

        let poweredOff   = amiga.isPoweredOff()
        let romHash      = amiga.mem.romFingerprint()
        let extHash      = amiga.mem.extFingerprint()
        let romRev       = amiga.mem.romRevision()
        let extRev       = amiga.mem.extRevision()

        track("romHash = \(hash)")
        track("extHash = \(extHash)")

        let hasRom        = romRev != ROM_MISSING
        let hasArosRom    = amiga.mem.isArosRom(romRev)
        let hasDiagRom    = amiga.mem.isDiagRom(romRev)
        let hasOrigRom    = amiga.mem.isOrigRom(romRev)
        let hasUnknownRom = romRev == ROM_UNKNOWN
        let hasKnownRom   = hasRom && !hasUnknownRom

        let hasExt        = extRev != ROM_MISSING
        let hasArosExt    = amiga.mem.isArosRom(extRev)
        let hasDiagExt    = amiga.mem.isDiagRom(extRev)
        let hasOrigExt    = amiga.mem.isOrigRom(extRev)
        let hasUnknownExt = extRev == ROM_UNKNOWN
        let hasKnownExt   = hasExt && !hasUnknownExt

        // let romURL        = controller.romURL

        let romMissing    = NSImage.init(named: "rom_light")
        let romOrig       = NSImage.init(named: "rom_original")
        let romAros       = NSImage.init(named: "rom_aros")
        let romDiag       = NSImage.init(named: "rom_diag")
        let romUnknown    = NSImage.init(named: "rom_unknown")
        
        // Lock controls if emulator is powered on
        romDropView.isEnabled = poweredOff
        romDeleteButton.isEnabled = poweredOff
        extDropView.isEnabled = poweredOff
        extDeleteButton.isEnabled = poweredOff
        romFactoryButton.isEnabled = poweredOff
        
        // Rom icon
        romDropView.image =
            hasArosRom ? romAros :
            hasDiagRom ? romDiag :
            hasOrigRom ? romOrig :
            hasRom     ? romUnknown : romMissing

        // Rom extension icon
        extDropView.image =
            hasArosExt ? romAros :
            hasDiagExt ? romDiag :
            hasOrigExt ? romOrig :
            hasExt     ? romUnknown : romMissing

        // Titles and subtitles
        var title: String    = amiga.mem.romTitle()
        var version: String  = amiga.mem.romVersion()
        var released: String = amiga.mem.romReleased()

        if hasKnownRom {
            romTitle.stringValue = title + " (" + version + ")"
            romSubtitle.stringValue = released
        } else if hasRom {
            romTitle.stringValue = "An uknown or patched Rom"
            romSubtitle.stringValue = "CRC " + String(Int(romHash), radix: 16)
        } else {
            romTitle.stringValue = "Kickstart Rom or Boot Rom"
            romSubtitle.stringValue = "(required)"
        }

        title = amiga.mem.extTitle()
        version = amiga.mem.extVersion()
        released = amiga.mem.extReleased()

        if hasKnownExt {
              extTitle.stringValue = title + " (" + version + ")"
              extSubtitle.stringValue = released
          } else if hasExt {
              extTitle.stringValue = "An uknown or patched Rom"
              extSubtitle.stringValue = "CRC " + String(Int(extHash), radix: 16)
          } else {
              extTitle.stringValue = "Kickstart Rom extension"
              extSubtitle.stringValue = "(optional)"
          }


        /*
        var text = ""
        var subText = ""
        
        text = knownRoms[hash] ?? "An unknown or unsupported Rom."
        subText = hasArosRom ? "Use original Amiga Roms for higher compatibility." : romURL.relativePath
        
        romText.stringValue = text
        romText.textColor = hasRom ? .textColor : .systemRed
        romSubText.stringValue = subText
            
        romHash.isHidden = !hasUnknownRom
        romHash.stringValue = String(format: "Hash: %llx", hash)
        */

        // Hide some controls
        romDeleteButton.isHidden = !hasRom
        extDeleteButton.isHidden = !hasExt

        // Warning message
        /*
        if hasOrigRom || hasOrigExt {
            
            romWarning.isHidden = false
            romWarning.stringValue = "Please obey legal regulations. Original Amiga Roms are copyrighted."
            romWarning.textColor = .textColor

        } else if hasUnknownRom {

            romWarning.isHidden = false
            romWarning.stringValue = "Use original Amiga Roms for higher compatibility."
            romWarning.textColor = .textColor

        } else {

            romWarning.isHidden = true
        }
        */

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

    @IBAction func romDeleteAction(_ sender: NSButton!) {

        myController?.romURL = URL(fileURLWithPath: "/")
        amigaProxy?.mem.deleteRom()
        
        refresh()
    }

    @IBAction func extDeleteAction(_ sender: NSButton!) {

        track()

        myController?.extURL = URL(fileURLWithPath: "/")
        amigaProxy?.mem.deleteExt()

        refresh()
    }

    @IBAction func extMapAddrAction(_ sender: NSPopUpButton!) {

        track()
        refresh()
    }

    @IBAction func romFactorySettingsAction(_ sender: NSButton!) {

        // Revert to the AROS Kickstart replacement
        amigaProxy?.mem.loadRom(fromBuffer: NSDataAsset(name: "aros-amiga-m68k-rom")?.data)
        myController?.romURL = URL(fileURLWithPath: "")
        amigaProxy?.mem.loadExt(fromBuffer: NSDataAsset(name: "aros-amiga-m68k-ext")?.data)
        myController?.extURL = URL(fileURLWithPath: "")
        refresh()
    }
}
