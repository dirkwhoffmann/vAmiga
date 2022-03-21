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

        let romIdentifier   = amiga.mem.romIdentifier
        let hasRom          = romIdentifier != .MISSING
        let hasArosRom      = amiga.mem.isArosRom(romIdentifier)
        let hasDiagRom      = amiga.mem.isDiagRom(romIdentifier)
        let hasCommodoreRom = amiga.mem.isCommodoreRom(romIdentifier)
        let hasHyperionRom  = amiga.mem.isHyperionRom(romIdentifier)
        let hasPatchedRom   = amiga.mem.isPatchedRom(romIdentifier)

        let extIdentifier   = amiga.mem.extIdentifier
        let hasExt          = extIdentifier != .MISSING
        let hasArosExt      = amiga.mem.isArosRom(extIdentifier)
        let hasDiagExt      = amiga.mem.isDiagRom(extIdentifier)
        let hasCommodoreExt = amiga.mem.isCommodoreRom(extIdentifier)
        let hasHyperionExt  = amiga.mem.isHyperionRom(extIdentifier)
        let hasPatchedExt   = amiga.mem.isPatchedRom(extIdentifier)

        let romMissing      = NSImage(named: "rom_missing")
        let romOrig         = NSImage(named: "rom_original")
        let romHyperion     = NSImage(named: "rom_hyp")
        let romAros         = NSImage(named: "rom_aros")
        let romDiag         = NSImage(named: "rom_diag")
        let romPatched      = NSImage(named: "rom_patched")
        let romUnknown      = NSImage(named: "rom_unknown")
        
        // Lock controls if emulator is powered on
        romDropView.isEnabled = poweredOff
        romDeleteButton.isEnabled = poweredOff
        extDropView.isEnabled = poweredOff
        extDeleteButton.isEnabled = poweredOff
        romArosButton.isEnabled = poweredOff
        
        // Icons
        romDropView.image =
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

        // Explanation
        romExpImage.isHidden = !poweredOff
        romExpInfo1.isHidden = !poweredOff
        romExpInfo2.isHidden = !poweredOff

        // Lock
        romLockImage.isHidden = poweredOff
        romLockInfo1.isHidden = poweredOff
        romLockInfo2.isHidden = poweredOff

        // Buttons
        romPowerButton.isHidden = !bootable
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

    @IBAction func installArosAction(_ sender: NSButton!) {

        installAros()
    }
    
    @IBAction func romDefaultsAction(_ sender: NSButton!) {
        
        config.saveRomUserDefaults()
    }

    func installAros() {
        
        let arosRom = NSDataAsset(name: "aros-amiga-m68k-rom")!.data
        let arosExt = NSDataAsset(name: "aros-amiga-m68k-ext")!.data

        // Install the Aros Roms
        try? amiga.mem.loadRom(buffer: arosRom)
        try? amiga.mem.loadExt(buffer: arosExt)
        config.extStart = 0xE0

        // Make sure the machine has enough Ram to run Aros
        let chip = amiga.getConfig(.CHIP_RAM)
        let slow = amiga.getConfig(.SLOW_RAM)
        let fast = amiga.getConfig(.FAST_RAM)
        if chip + slow + fast < 1024*1024 { config.slowRam = 512 }
        
        refresh()
    }
}
