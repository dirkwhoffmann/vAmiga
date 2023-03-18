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

        let poweredOff      = amiga.poweredOff

        let romCrc          = amiga.mem.romFingerprint
        let hasRom          = romCrc != CRC32_MISSING
        let hasArosRom      = amiga.mem.isArosRom(romCrc)
        let hasDiagRom      = amiga.mem.isDiagRom(romCrc)
        let hasCommodoreRom = amiga.mem.isCommodoreRom(romCrc)
        let hasHyperionRom  = amiga.mem.isHyperionRom(romCrc)
        let hasPatchedRom   = amiga.mem.isPatchedRom(romCrc)
        let isRelocatedRom  = amiga.mem.isRelocated

        let extCrc          = amiga.mem.extFingerprint
        let hasExt          = extCrc != CRC32_MISSING
        let hasArosExt      = amiga.mem.isArosRom(extCrc)
        let hasDiagExt      = amiga.mem.isDiagRom(extCrc)
        let hasCommodoreExt = amiga.mem.isCommodoreRom(extCrc)
        let hasHyperionExt  = amiga.mem.isHyperionRom(extCrc)
        let hasPatchedExt   = amiga.mem.isPatchedRom(extCrc)

        let romMissing      = NSImage(named: "rom_missing")
        let romOrig         = NSImage(named: "rom_original")
        let romHyperion     = NSImage(named: "rom_hyp")
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
        romTitle.stringValue = amiga.mem.romTitle
        romSubtitle.stringValue = amiga.mem.romVersion
        romSubsubtitle.stringValue = amiga.mem.romReleased
        romModel.stringValue = amiga.mem.romModel
        
        extTitle.stringValue = amiga.mem.extTitle
        extSubtitle.stringValue = amiga.mem.extVersion
        extSubsubtitle.stringValue = amiga.mem.extReleased
        extMapAddr.selectItem(withTag: amiga.mem.extStart)
        extModel.stringValue = amiga.mem.extModel

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
            // romExpInfo1.stringValue = "The selected Kickstart Rom is a relocation image."
            // romExpInfo1.stringValue = "The selected Kickstart Rom won't work in the Rom slot."
            romExpInfo1.stringValue = "The selected Kickstart Rom is a relocation image."
            romExpInfo1.isHidden = false
            // romExpInfo2.stringValue = "It won't work in the Rom slot and needs to be loaded from disk."
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

            case CRC32_AROS_54705, CRC32_AROS_55696, CRC32_DIAG121:
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

        amiga.mem.deleteRom()
        refresh()
    }

    @IBAction func extDeleteAction(_ sender: NSButton!) {

        amiga.mem.deleteExt()
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

        case CRC32_DIAG121:
            install(rom: "diagrom-121")

        default:
            if let url = UserDefaults.romUrl(fingerprint: crc32) {
                try? amiga.mem.loadRom(url)
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
        let chip = amiga.getConfig(.CHIP_RAM)
        let slow = amiga.getConfig(.SLOW_RAM)
        let fast = amiga.getConfig(.FAST_RAM)
        if chip + slow + fast < 1024*1024 { config.slowRam = 512 }
    }

    func install(rom: String) {

        let data = NSDataAsset(name: rom)!.data
        try? amiga.mem.loadRom(buffer: data)
    }

    func install(ext: String) {

        let data = NSDataAsset(name: ext)!.data
        try? amiga.mem.loadExt(buffer: data)
    }
}
