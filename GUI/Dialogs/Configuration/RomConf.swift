// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ConfigurationController {
    
    func refreshRomTab() {

        let config = emu.mem.config
        let romTraits = emu.mem.romTraits
        let extTraits = emu.mem.extTraits

        let poweredOff      = emu.poweredOff

        // let romCrc          = amiga.mem.romFingerprint
        let hasRom          = romTraits.crc != 0
        let hasArosRom      = romTraits.vendor == .AROS
        let hasDiagRom      = romTraits.vendor == .DIAG
        let hasCommodoreRom = romTraits.vendor == .COMMODORE
        let hasHyperionRom  = romTraits.vendor == .HYPERION
        let hasEmutosRom    = romTraits.vendor == .EMUTOS
        let hasPatchedRom   = romTraits.patched
        let isRelocatedRom  = romTraits.relocated

        // let extCrc          = amiga.mem.extFingerprint
        let hasExt          = extTraits.crc != 0
        let hasArosExt      = extTraits.vendor == .AROS
        let hasDiagExt      = extTraits.vendor == .DIAG
        let hasCommodoreExt = extTraits.vendor == .COMMODORE
        let hasHyperionExt  = extTraits.vendor == .HYPERION
        let hasPatchedExt   = extTraits.patched

        let romMissing      = NSImage(named: "rom_missing")
        let romOrig         = NSImage(named: "rom_original")
        let romHyperion     = NSImage(named: "rom_hyp")
        let romEmutos       = NSImage(named: "rom_emutos")
        let romAros         = NSImage(named: "rom_aros")
        let romDiag         = NSImage(named: "rom_diag")
        let romPatched      = NSImage(named: "rom_patched")
        let romUnknown      = NSImage(named: "rom_unknown")
        let romRelocated    = NSImage(named: "rom_broken")

        // Lock controls if emulator is powered on
        romDropView.isEnabled = poweredOff
        romDeleteButton.isEnabled = poweredOff
        extDropView.isEnabled = poweredOff
        extDeleteButton.isEnabled = poweredOff
        romArosPopup.isEnabled = poweredOff
        
        // Icons
        romDropView.image =
        isRelocatedRom  ? romRelocated :
        hasHyperionRom  ? romHyperion :
        hasEmutosRom    ? romEmutos :
        hasArosRom      ? romAros :
        hasDiagRom      ? romDiag :
        hasCommodoreRom ? romOrig :
        hasPatchedRom   ? romPatched :
        hasRom          ? romUnknown : romMissing

        extDropView.image =
        hasHyperionExt  ? romHyperion :
        hasArosExt      ? romAros :
        hasDiagExt      ? romDiag :
        hasCommodoreExt ? romOrig :
        hasPatchedExt   ? romPatched :
        hasExt          ? romUnknown : romMissing

        // Titles and subtitles
        romTitle.stringValue = String(cString: romTraits.title)
        romSubtitle.stringValue = String(cString: romTraits.revision)
        romSubsubtitle.stringValue = String(cString: romTraits.released)
        romModel.stringValue = String(cString: romTraits.model)

        extTitle.stringValue = String(cString: extTraits.title)
        extSubtitle.stringValue = String(cString: extTraits.revision)
        extSubsubtitle.stringValue = String(cString: extTraits.released)
        extModel.stringValue = String(cString: extTraits.model)
        extMapAddr.selectItem(withTag: Int(config.extStart))

        // Hide some controls
        romDeleteButton.isHidden = !hasRom
        extDeleteButton.isHidden = !hasExt
        extMapText.isHidden = !hasExt
        extMapAddr.isHidden = !hasExt

        // Lock
        romLockImage.isHidden = poweredOff
        romLockInfo1.isHidden = poweredOff
        romLockInfo2.isHidden = poweredOff

        // Buttons
        romPowerButton.isHidden = !bootable

        // Explanation
        if isRelocatedRom {
            romExpImage.image = NSImage(named: "NSCaution")
            romExpImage.isHidden = false
            romExpInfo1.stringValue = "The selected Kickstart Rom is a relocation image."
            romExpInfo1.isHidden = false
            romExpInfo2.stringValue = "It won't work in the Rom slot."
            romExpInfo2.isHidden = false
        } else {
            romExpImage.image = NSImage(named: "NSInfo")
            romExpImage.isHidden = !poweredOff
            romExpInfo1.stringValue = "To add a Rom, drag a Rom image file onto one of the chip icons."
            romExpInfo1.isHidden = !poweredOff
            romExpInfo2.stringValue = "Original Roms are protected by copyright. Please obey legal regulations."
            romExpInfo2.isHidden = !poweredOff
        }
    }

    func refreshRomSelector() {

        romArosPopup.autoenablesItems = false

        let fm = FileManager.default

        for item in romArosPopup.itemArray where item.tag != 0 {

            switch UInt32(item.tag) {

            case CRC32_AROS_54705, CRC32_AROS_55696, CRC32_DIAG121, CRC32_DIAG13, CRC32_EMUTOS13:
                item.isEnabled = true

            default:
                if let url = UserDefaults.romUrl(fingerprint: item.tag) {
                    item.isEnabled = fm.fileExists(atPath: url.path)
                } else {
                    item.isEnabled = false
                }
            }

            if item.isEnabled {
                item.image = NSImage(named: "chipTemplate")
            } else {
                item.image = NSImage(named: "lockTemplate")
            }
        }
    }

    //
    // Action methods
    //

    @IBAction func romDeleteAction(_ sender: NSButton!) {

        emu.mem.deleteRom()
        refresh()
    }

    @IBAction func extDeleteAction(_ sender: NSButton!) {

        emu.mem.deleteExt()
        refresh()
    }

    @IBAction func extMapAddrAction(_ sender: NSPopUpButton!) {

        config.extStart = sender.selectedTag()
        refresh()
    }

    @IBAction func installRomAction(_ sender: NSButton!) {

        let crc32 = sender.selectedTag()

        switch UInt32(crc32) {
        case CRC32_AROS_54705: // Taken from UAE
            installAros(rom: "aros-svn54705-rom", ext: "aros-svn54705-ext")

        case CRC32_AROS_55696: // Taken from SAE
            installAros(rom: "aros-svn55696-rom", ext: "aros-svn55696-ext")

        case CRC32_EMUTOS13:
            install(rom: "emutos-13")

        case CRC32_DIAG121:
            install(rom: "diagrom-121")

        case CRC32_DIAG13:
            install(rom: "diagrom-13")

        default:
            if let url = UserDefaults.romUrl(fingerprint: crc32) {
                try? emu.mem.loadRom(url)
            }
        }

        refresh()
    }
    
    @IBAction func romDefaultsAction(_ sender: NSButton!) {
        
        do {
        
            try config.saveRomUserDefaults()
            
        } catch {
            
            parent.showAlert(.cantSaveRoms, error: error, window: window)
        }
    }

    func installAros() {

        installAros(crc32: CRC32_AROS_55696)
    }

    func installAros(crc32: UInt32) {

        switch crc32 {

        case CRC32_AROS_54705: // Taken from UAE
            installAros(rom: "aros-svn54705-rom", ext: "aros-svn54705-ext")

        case CRC32_AROS_55696: // Taken from SAE
            installAros(rom: "aros-svn55696-rom", ext: "aros-svn55696-ext")

        default:
            fatalError()
        }
    }

    func installAros(rom: String, ext: String) {

        // Install both Roms
        install(rom: rom)
        install(ext: ext)

        // Configure the location of the exansion Rom
        config.extStart = 0xE0

        // Make sure the machine has enough Ram to run Aros
        let chip = emu.get(.MEM_CHIP_RAM)
        let slow = emu.get(.MEM_SLOW_RAM)
        let fast = emu.get(.MEM_FAST_RAM)
        if chip + slow + fast < 1024*1024 { config.slowRam = 512 }
    }

    func install(rom: String) {

        let data = NSDataAsset(name: rom)!.data
        try? emu.mem.loadRom(buffer: data)
    }

    func install(ext: String) {

        let data = NSDataAsset(name: ext)!.data
        try? emu.mem.loadExt(buffer: data)
    }
}
