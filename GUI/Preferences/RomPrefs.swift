// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension PreferencesController {

    func refreshRomTab() {
        
        // guard let controller = myController else { return }
        guard let amiga = amigaProxy else { return }

        let poweredOff   = amiga.isPoweredOff()
        let romRev       = amiga.mem.romRevision()
        let extRev       = amiga.mem.extRevision()

        let hasRom        = romRev != ROM_MISSING
        let hasArosRom    = amiga.mem.isArosRom(romRev)
        let hasDiagRom    = amiga.mem.isDiagRom(romRev)
        let hasOrigRom    = amiga.mem.isOrigRom(romRev)

        let hasExt        = extRev != ROM_MISSING
        let hasArosExt    = amiga.mem.isArosRom(extRev)
        let hasDiagExt    = amiga.mem.isDiagRom(extRev)
        let hasOrigExt    = amiga.mem.isOrigRom(extRev)

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
        
        // Icons
        romDropView.image =
            hasArosRom ? romAros :
            hasDiagRom ? romDiag :
            hasOrigRom ? romOrig :
            hasRom     ? romUnknown : romMissing

        extDropView.image =
            hasArosExt ? romAros :
            hasDiagExt ? romDiag :
            hasOrigExt ? romOrig :
            hasExt     ? romUnknown : romMissing

        // Titles and subtitles
        romTitle.stringValue = amiga.mem.romTitle()
        romSubtitle.stringValue = amiga.mem.romVersion()
        romSubsubtitle.stringValue = amiga.mem.romReleased()

        extTitle.stringValue = amiga.mem.extTitle()
        extSubtitle.stringValue = amiga.mem.extVersion()
        extSubsubtitle.stringValue = amiga.mem.extReleased()

        // Rom mapping addresses
        extMapAddr.selectItem(withTag: amiga.mem.extStart())

        // Hide some controls
        romDeleteButton.isHidden = !hasRom
        romMapText.isHidden = !hasRom
        romMapAddr.isHidden = !hasRom

        extDeleteButton.isHidden = !hasExt
        extMapText.isHidden = !hasExt
        extMapAddr.isHidden = !hasExt

        // Lock symbol and explanation
        romLockImage.isHidden = poweredOff
        romLockText.isHidden = poweredOff
        romLockSubText.isHidden = poweredOff
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

    @IBAction func romMapAddrAction(_ sender: NSPopUpButton!) {

        // Nothing to do here at the moment.
        // All supported Roms are mapped at $FE0000
        refresh()
    }

    @IBAction func extMapAddrAction(_ sender: NSPopUpButton!) {

        track()
        amigaProxy?.configure(VA_EXT_START, value: sender.selectedTag())
        refresh()
    }

    @IBAction func romFactorySettingsAction(_ sender: NSButton!) {

        // Revert to the AROS Kickstart replacement
        amigaProxy?.mem.loadRom(fromBuffer: NSDataAsset(name: "aros-amiga-m68k-rom")?.data)
        myController?.romURL = URL(fileURLWithPath: "")
        amigaProxy?.mem.loadExt(fromBuffer: NSDataAsset(name: "aros-amiga-m68k-ext")?.data)
        myController?.extURL = URL(fileURLWithPath: "")
        amigaProxy?.configure(VA_EXT_START, value: 0xE0)
        refresh()
    }
}
