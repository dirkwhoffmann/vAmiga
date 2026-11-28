// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class RomSettingsViewController: SettingsViewController {

    // Rom info
    @IBOutlet weak var romDropView: RomDropView!
    @IBOutlet weak var romSelector: NSPopUpButton!
    @IBOutlet weak var romTitle: NSTextField!
    @IBOutlet weak var romSubtitle: NSTextField!
    @IBOutlet weak var romSubsubtitle: NSTextField!
    @IBOutlet weak var romType: NSTextField!
    @IBOutlet weak var romDeleteButton: NSButton!

    @IBOutlet weak var extDropView: ExtRomDropView!
    @IBOutlet weak var extSelector: NSPopUpButton!
    @IBOutlet weak var extTitle: NSTextField!
    @IBOutlet weak var extSubtitle: NSTextField!
    @IBOutlet weak var extSubsubtitle: NSTextField!
    @IBOutlet weak var extType: NSTextField!
    @IBOutlet weak var extDeleteButton: NSButton!
    @IBOutlet weak var extMapText: NSTextField!
    @IBOutlet weak var extMapAddr: NSPopUpButton!

    // Explanation
    @IBOutlet weak var romExpImage: NSButton!
    @IBOutlet weak var romExpInfo1: NSTextField!
    @IBOutlet weak var romExpInfo2: NSTextField!

    override var showLock: Bool { true }

    override func viewDidLoad() {

        log(.lifetime)
        refreshRomSelector()

        romSelector.target = self
        romSelector.action = #selector(romSelectorAction(_:))
        extSelector.target = self
        extSelector.action = #selector(extSelectorAction(_:))
    }
    
    //
    // Methods from SettingsViewController
    //

    override func refresh() {

        super.refresh()

        guard let emu = emu else { return }

        let config = emu.mem.config
        let romTraits = emu.mem.romTraits
        let extTraits = emu.mem.extTraits

        let poweredOff      = emu.poweredOff

        let hasRom          = romTraits.crc != 0
        let hasArosRom      = romTraits.vendor == .AROS
        let hasDemoRom      = romTraits.type == .AMIGA_DEMO
        let hasDiagRom      = romTraits.type == .AMIGA_DIAG
        let hasCommodoreRom = romTraits.vendor == .COMMODORE
        let hasHyperionRom  = romTraits.vendor == .HYPERION
        let hasEmutosRom    = romTraits.vendor == .EMUTOS
        let hasPatchedRom   = (romTraits.flags & retro.vault.RomFlags.Patched) != 0
        let hasRelocatedRom = (romTraits.flags & retro.vault.RomFlags.Relocated) != 0

        let hasExt          = extTraits.crc != 0
        let hasArosExt      = extTraits.vendor == .AROS
        let hasDiagExt      = extTraits.type == .AMIGA_DIAG
        let hasCommodoreExt = extTraits.vendor == .COMMODORE
        let hasHyperionExt  = extTraits.vendor == .HYPERION
        let hasPatchedExt   = (extTraits.flags & retro.vault.RomFlags.Patched) != 0
        let hasRelocatedExt = (extTraits.flags & retro.vault.RomFlags.Relocated) != 0

        let romMissing      = NSImage(named: "rom_missing")
        let romOrig         = NSImage(named: "rom_original")
        let romHyperion     = NSImage(named: "rom_hyp")
        let romEmutos       = NSImage(named: "rom_emutos")
        let romAros         = NSImage(named: "rom_aros")
        let romDemo         = NSImage(named: "rom_demo")
        let romDiag         = NSImage(named: "rom_diag")
        let romUnknown      = NSImage(named: "rom_unknown")
        let patched         = NSImage(named: "patched")
        let relocated       = NSImage(named: "relocated")

        // Lock controls if emulator is powered on
        romDropView.isEnabled = poweredOff
        romSelector.isEnabled = poweredOff
        extSelector.isEnabled = poweredOff
        romDeleteButton.isEnabled = poweredOff
        extDropView.isEnabled = poweredOff
        extDeleteButton.isEnabled = poweredOff
        presetPopup.isEnabled = poweredOff

        // Icons
        romDropView.image =
        hasHyperionRom  ? romHyperion :
        hasEmutosRom    ? romEmutos :
        hasArosRom      ? romAros :
        hasDiagRom      ? romDiag :
        hasDemoRom      ? romDemo :
        hasCommodoreRom ? romOrig :
        hasRom          ? romUnknown : romMissing

        romDropView.auxIcon.image =
        hasPatchedRom ? patched :
        hasRelocatedRom ? relocated : nil

        extDropView.image =
        hasHyperionExt  ? romHyperion :
        hasArosExt      ? romAros :
        hasDiagExt      ? romDiag :
        hasCommodoreExt ? romOrig :
        hasExt          ? romUnknown : romMissing

        extDropView.auxIcon.image =
        hasPatchedExt ? patched :
        hasRelocatedExt ? relocated : nil

        // Rom library selectors
        refresh(selector: romSelector, extensions: false, installed: romTraits.crc)
        refresh(selector: extSelector, extensions: true, installed: extTraits.crc)

        // Titles and subtitles
        romTitle.stringValue = String(cString: romTraits.title)
        romSubtitle.stringValue = String(cString: romTraits.revision)
        romSubsubtitle.stringValue = RomSettingsViewController.romDateString(romTraits.released)
        romType.stringValue = hasRom ? String(cString: retro.vault.RomTypeEnum.help(romTraits.type)) : ""

        extTitle.stringValue = String(cString: extTraits.title)
        extSubtitle.stringValue = String(cString: extTraits.revision)
        extSubsubtitle.stringValue = RomSettingsViewController.romDateString(extTraits.released)
        extType.stringValue = hasExt ? String(cString: retro.vault.RomTypeEnum.help(extTraits.type)) : ""
        extMapAddr.selectItem(withTag: Int(config.extStart))

        if romDropView.image == romUnknown {
            romSubtitle.stringValue = String(format:"CRC32: 0x%08X", romTraits.crc)
        }
        if extDropView.image == romUnknown {
            extSubtitle.stringValue = String(format:"CRC32: 0x%08X", extTraits.crc)
        }
        // Hide some controls
        romDeleteButton.isHidden = !hasRom
        extDeleteButton.isHidden = !hasExt
        extMapText.isHidden = !hasExt
        extMapAddr.isHidden = !hasExt

        // Explanation
        if hasRelocatedRom {
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

    // Formats a RomDate for display, e.g. "April 2025", "2022", or "" if unknown
    static func romDateString(_ date: retro.vault.RomDate) -> String {

        let months = ["", "January", "February", "March", "April", "May", "June",
                      "July", "August", "September", "October", "November", "December"]

        if date.year == 0 { return "" }
        if date.month == 0 || date.month > 12 { return "\(date.year)" }

        return "\(months[Int(date.month)]) \(date.year)"
    }

    // Lists "None" and the Roms the Rom manager has located on disk
    func refresh(selector: NSPopUpButton, extensions: Bool, installed: UInt32) {

        let menu = selector.menu!

        // Keep the first item: it is the pull-down button's own image
        while menu.items.count > 1 { menu.removeItem(at: 1) }

        let none = NSMenuItem(title: "None", action: nil, keyEquivalent: "")
        none.tag = 0
        none.state = installed == 0 ? .on : .off
        menu.addItem(none)

        let roms = EmulatorProxy.availableRoms(extensions) ?? []
        if !roms.isEmpty { menu.addItem(NSMenuItem.separator()) }

        for crc in roms.map({ $0.uint32Value }) {

            let traits = EmulatorProxy.romTraits(forCRC32: crc)
            var title = String(cString: traits.title)
            let revision = String(cString: traits.revision)
            if !revision.isEmpty { title += " (\(revision))" }

            let item = NSMenuItem(title: title, action: nil, keyEquivalent: "")
            item.tag = Int(crc)
            item.state = crc == installed ? .on : .off
            menu.addItem(item)
        }
    }

    func refreshRomSelector() {

        presetPopup.autoenablesItems = false

        let fm = FileManager.default

        for item in presetPopup.itemArray where item.tag != 0 {

            switch UInt32(item.tag) {

            case retro.vault.CRC32_AROS_54705, retro.vault.CRC32_AROS_55696, retro.vault.CRC32_AROS_20250219, retro.vault.CRC32_AROS_20260820,
                retro.vault.CRC32_DIAG121, retro.vault.CRC32_DIAG13, retro.vault.CRC32_DIAG20,
                retro.vault.CRC32_EMUTOS13:

                item.isEnabled = true

            default:
                if let url = UserDefaults.romUrl(fingerprint: item.tag) {
                    item.isEnabled = fm.fileExists(atPath: url.path)
                } else {
                    item.isEnabled = false
                }
            }

            if item.isEnabled {
                item.image = NSImage(systemSymbolName: "memorychip", 
                                     accessibilityDescription: "Unlocked Kickstart")
            } else {
                item.image = NSImage(systemSymbolName: "lock",
                                     accessibilityDescription: "Locked Kickstart")
            }
        }
    }

    //
    // Action methods
    //

    @IBAction func romDeleteAction(_ sender: NSButton!) {

        emu?.mem.deleteRom()
        refresh()
    }

    @IBAction func extDeleteAction(_ sender: NSButton!) {

        emu?.mem.deleteExt()
        refresh()
    }

    @IBAction func romSelectorAction(_ sender: NSPopUpButton!) {

        install(crc: UInt32(sender.selectedTag()), extension: false)
    }

    @IBAction func extSelectorAction(_ sender: NSPopUpButton!) {

        install(crc: UInt32(sender.selectedTag()), extension: true)
    }

    /* Keeps a copy of the installed Rom in the Rom library, provided that the
     * Rom database knows it. The copy is named after the Rom's checksum, which
     * is where the preset menu and the Rom library selectors look for it.
     */
    func saveToLibrary(extension ext: Bool) {

        guard let emu = emu else { return }

        let crc = ext ? emu.mem.extTraits.crc : emu.mem.romTraits.crc
        guard EmulatorProxy.isKnownRom(crc), let url = UserDefaults.romUrl(crc32: crc) else { return }

        // Nothing to do if the library has it already
        if FileManager.default.fileExists(atPath: url.path) { return }

        loginfo(1, "Saving \(url)")
        if ext { try? emu.mem.saveExt(url) } else { try? emu.mem.saveRom(url) }

        UserDefaults.rescanRoms()
        refreshRomSelector()
    }

    // Installs a Rom from the Rom library, or removes the Rom if crc is 0
    func install(crc: UInt32, extension ext: Bool) {

        guard let emu = emu else { return }

        if crc == 0 {

            if ext { emu.mem.deleteExt() } else { emu.mem.deleteRom() }

        } else if let url = EmulatorProxy.romURL(forCRC32: crc) {

            do {
                if ext { try emu.mem.loadExt(url) } else { try emu.mem.loadRom(url) }
            } catch {
                controller?.showAlert(.cantOpen(url: url), error: error,
                                      async: true, window: controller?.window)
            }
        }
        refresh()
    }

    @IBAction func extMapAddrAction(_ sender: NSPopUpButton!) {

        config?.extStart = sender.selectedTag()
    }


    func installAros() {

        installAros(crc32: retro.vault.CRC32_AROS_20250219)
    }

    func installAros(crc32: UInt32) {

        switch crc32 {

        case retro.vault.CRC32_AROS_54705:       // Taken from UAE
            installAros(rom: "aros-svn54705-rom", ext: "aros-svn54705-ext")

        case retro.vault.CRC32_AROS_55696:       // Taken from SAE
            installAros(rom: "aros-svn55696-rom", ext: "aros-svn55696-ext")

        case retro.vault.CRC32_AROS_20250219:    // 2025 version
            installAros(rom: "aros-20250219-rom", ext: "aros-20250219-ext")

        case retro.vault.CRC32_AROS_20260820:    // 2026 version
            installAros(rom: "aros-20260820-rom", ext: "aros-20260820-ext")

        default:
            fatalError()
        }
    }

    func installAros(rom: String, ext: String) {

        guard let config = config, let emu = emu else { return }

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
        try? emu?.mem.loadRom(buffer: data)
    }

    func install(ext: String) {

        let data = NSDataAsset(name: ext)!.data
        try? emu?.mem.loadExt(buffer: data)
    }

    override func preset(tag: Int) {

        let crc32 = UInt32(tag)

        switch crc32 {
        case retro.vault.CRC32_AROS_54705, retro.vault.CRC32_AROS_55696, retro.vault.CRC32_AROS_20250219, retro.vault.CRC32_AROS_20260820:
            installAros(crc32: crc32)

        case retro.vault.CRC32_EMUTOS13:
            install(rom: "emutos-13")

        case retro.vault.CRC32_DIAG121:
            install(rom: "diagrom-121")

        case retro.vault.CRC32_DIAG13:
            install(rom: "diagrom-13")

        case retro.vault.CRC32_DIAG20:
            install(rom: "diagrom-20")

        default:
            if let url = UserDefaults.romUrl(crc32: crc32) {
                try? emu?.mem.loadRom(url)
            }
        }

        refresh()
    }

    override func save() {

        do {

            try config?.saveRomUserDefaults()

        } catch {

            controller?.showAlert(.cantSaveRoms,
                                  error: error,
                                  window: controller!.window)
        }
    }
}
