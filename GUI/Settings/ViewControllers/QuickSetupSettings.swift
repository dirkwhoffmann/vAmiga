// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class QuickSetupSettingsViewController: SettingsViewController {

    // Presets
    @IBOutlet weak var modelPopup: NSPopUpButton!
    @IBOutlet weak var monitorPopup: NSPopUpButton!
    @IBOutlet weak var profilePopup: NSPopUpButton!
    @IBOutlet weak var zoneButton: NSButton!

    // Description
    @IBOutlet weak var heading: NSTextField!
    @IBOutlet weak var text: NSTextField!
    @IBOutlet weak var apply: NSButton!

    // Selected video format
    var pal = true { didSet { refresh() } }

    // Popup entries
    let modelItems: [(tag: Int, title: String)] = [

        (ModelPreset.A500.rawValue, "Amiga 500"),
        (ModelPreset.A1000.rawValue, "Amiga 1000"),
        (ModelPreset.A2000.rawValue, "Amiga 2000"),
        (ModelPreset.A500P.rawValue, "Amiga 500+"),
        (ModelPreset.A600.rawValue, "Amiga 600"),
        (ModelPreset.A1200.rawValue, "Amiga 1200"),
        (ModelPreset.A3000.rawValue, "Amiga 3000")
    ]

    let monitorItems: [(tag: Int, title: String)] = [

        (MonitorPreset.TFT.rawValue, "TFT"),
        (MonitorPreset.CRT.rawValue, "CRT")
    ]

    let profileItems: [(tag: Int, title: String)] = [

        (ProfilePreset.DEFAULT.rawValue, "Default"),
        (ProfilePreset.GAMING.rawValue, "Gaming"),
        (ProfilePreset.PRODUCTIVITY.rawValue, "Productivity")
    ]

    override var showLock: Bool { true }

    override func viewDidLoad() {

        log(.lifetime)

        setup(modelPopup, items: modelItems)
        setup(monitorPopup, items: monitorItems)
        setup(profilePopup, items: profileItems)
        select(model: .A2000, monitor: .TFT, profile: .DEFAULT, pal: true)

        apply.target = self
        apply.action = #selector(applyAction(_:))
        zoneButton.target = self
        zoneButton.action = #selector(zoneAction(_:))
    }

    func setup(_ popup: NSPopUpButton, items: [(tag: Int, title: String)]) {

        popup.removeAllItems()
        for item in items {

            popup.addItem(withTitle: item.title)
            popup.lastItem!.tag = item.tag
        }

        popup.target = self
        popup.action = #selector(popupAction(_:))
    }

    //
    // Selected presets
    //

    var selectedModel: ModelPreset { ModelPreset(rawValue: modelPopup.selectedTag())! }
    var selectedMonitor: MonitorPreset { MonitorPreset(rawValue: monitorPopup.selectedTag())! }
    var selectedProfile: ProfilePreset { ProfilePreset(rawValue: profilePopup.selectedTag())! }

    func select(model: ModelPreset, monitor: MonitorPreset, profile: ProfilePreset, pal: Bool) {

        modelPopup.selectItem(withTag: model.rawValue)
        monitorPopup.selectItem(withTag: monitor.rawValue)
        profilePopup.selectItem(withTag: profile.rawValue)
        self.pal = pal
    }

    //
    // Methods from SettingsViewController
    //

    override func refresh() {

        super.refresh()

        guard let emu = emu else { return }

        // The configuration can only be changed in power-off state
        apply.isEnabled = emu.poweredOff
        presetPopup.isEnabled = emu.poweredOff

        // Video format
        zoneButton.image = NSImage(named: pal ? "flag-eu" : "flag-us")
        zoneButton.toolTip = pal ? "PAL" : "NTSC"

        // Describe the configuration
        text.attributedStringValue = bulletList(describe(emu))
    }

    //
    // Description
    //

    func describe(_ emu: EmulatorProxy) -> [String] {

        // Collect the option values of the presets that would be applied
        struct Key: Hashable { let opt: Int; let id: Int }
        var preset: [Key: Int] = [:]
        func add(_ options: [[NSNumber]]) {
            for o in options { preset[Key(opt: o[0].intValue, id: o[1].intValue)] = o[2].intValue }
        }
        add(EmulatorProxy.options(forModel: selectedModel))
        add(EmulatorProxy.options(forMonitor: selectedMonitor))
        add(EmulatorProxy.options(forProfile: selectedProfile))
        preset[Key(opt: Option.AMIGA_VIDEO_FORMAT.rawValue, id: -1)] =
        (pal ? VideoFormat.PAL : VideoFormat.NTSC).rawValue

        // Options not set by a preset keep their current values
        func value(_ opt: Option, _ id: Int = 0) -> Int {
            preset[Key(opt: opt.rawValue, id: id)] ??
            preset[Key(opt: opt.rawValue, id: -1)] ??
            emu.get(opt, id: id)
        }

        var features: [String] = []

        //
        // Model
        //

        let cpus = ["68000", "68010", "68EC020", "68020", "68EC030", "68030", "68EC040", "68LC040", "68040"]
        let cpu = cpus.indices.contains(value(.CPU_REVISION)) ? cpus[value(.CPU_REVISION)] : "680x0"
        let pal = value(.AMIGA_VIDEO_FORMAT) == VideoFormat.PAL.rawValue
        let mhz = (pal ? 7.09379 : 7.15909) * Double(max(1, value(.CPU_OVERCLOCKING)))
        features.append("Motorola \(cpu) CPU at \(Int(mhz.rounded())) MHz")

        let agnus = AgnusRevision(rawValue: value(.AGNUS_REVISION))
        let denise = DeniseRevision(rawValue: value(.DENISE_REVISION))
        switch (agnus, denise) {
        case (.AGA, _), (_, .AGA):
            features.append("AGA Chip Set")
        case (.ECS_1MB, .ECS), (.ECS_2MB, .ECS):
            features.append("ECS Agnus with ECS Denise")
        case (.ECS_1MB, _), (.ECS_2MB, _):
            features.append("ECS Agnus with OCS Denise")
        default:
            features.append("OCS Agnus with OCS Denise")
        }

        features.append("\(memory(value(.MEM_CHIP_RAM))) Chip RAM")
        if value(.MEM_SLOW_RAM) > 0 { features.append("\(memory(value(.MEM_SLOW_RAM))) Slow RAM") }
        if value(.MEM_FAST_RAM) > 0 { features.append("\(memory(value(.MEM_FAST_RAM))) Fast RAM") }

        var drives: [String: Int] = [:]
        var order: [String] = []
        for n in 0...3 where value(.DRIVE_CONNECT, n) != 0 {
            let type: String
            switch FloppyDriveType(rawValue: emu.get(.DRIVE_TYPE, drive: n)) {
            case .HD_35: type = "3.5″ HD"
            case .DD_525: type = "5.25″"
            default: type = "3.5″"
            }
            if drives[type] == nil { order.append(type) }
            drives[type, default: 0] += 1
        }
        for type in order {
            let count = drives[type]!
            features.append("\(spelledOut(count)) \(type) floppy drive" + (count == 1 ? "" : "s"))
        }
        let hds = (0...3).filter { value(.HDC_CONNECT, $0) != 0 }.count
        if hds > 0 { features.append("\(spelledOut(hds)) hard drive" + (hds == 1 ? "" : "s")) }

        features.append(pal ? "PAL video (50 Hz)" : "NTSC video (60 Hz)")
        if value(.MON_FLICKER_FIXER) != 0 { features.append("Flicker fixer") }

        //
        // Monitor
        //

        var effects: [String] = []
        if value(.MON_SCANLINES) != 0 { effects.append("scanlines") }
        if value(.MON_DOTMASK) != 0 { effects.append("dot mask") }
        if value(.MON_BLOOM) != 0 { effects.append("bloom") }
        if effects.isEmpty {
            features.append("Flat-panel picture")
        } else {
            features.append("CRT look with " + list(effects))
        }

        //
        // Profile
        //

        if value(.AMIGA_RUN_AHEAD) > 0 {
            features.append("\(spelledOut(value(.AMIGA_RUN_AHEAD)))-frame run-ahead")
        }
        if value(.AMIGA_VSYNC) != 0 {
            features.append("Host display sync (VSYNC)")
        }
        if value(.AMIGA_WARP_BOOT) > 0 {
            features.append("\(spelledOut(value(.AMIGA_WARP_BOOT)))-second warp boot")
        }
        switch value(.DC_SPEED) {
        case -1: features.append("Turbo floppy drives")
        case 1: break
        case let speed: features.append("\(speed)× floppy speed")
        }
        if value(.DRIVE_MECHANICS) != 0 {
            features.append("Authentic drive mechanics")
        }

        //
        // Compose the text
        //

        return features
    }

    // Formats a list of items as a bullet list with a hanging indent
    func bulletList(_ items: [String]) -> NSAttributedString {

        let font = text.font ?? NSFont.systemFont(ofSize: NSFont.systemFontSize)
        let indent = CGFloat(14)

        let style = NSMutableParagraphStyle()
        style.tabStops = [NSTextTab(textAlignment: .left, location: indent)]
        style.defaultTabInterval = indent
        style.headIndent = indent
        style.paragraphSpacing = 4

        return NSAttributedString(
            string: items.map { "•\t" + $0 }.joined(separator: "\n"),
            attributes: [.font: font,
                         .foregroundColor: text.textColor ?? NSColor.labelColor,
                         .paragraphStyle: style])
    }

    func spelledOut(_ number: Int) -> String {

        let formatter = NumberFormatter()
        formatter.locale = Locale(identifier: "en_US")
        formatter.numberStyle = .spellOut

        let words = formatter.string(from: NSNumber(value: number)) ?? "\(number)"
        return words.prefix(1).uppercased() + words.dropFirst()
    }

    func memory(_ kb: Int) -> String {

        return kb >= 1024 && kb % 1024 == 0 ? "\(kb / 1024) MB" : "\(kb) KB"
    }

    func list(_ items: [String]) -> String {

        switch items.count {
        case 0: return ""
        case 1: return items[0]
        default: return items.dropLast().joined(separator: ", ") + " and " + items.last!
        }
    }

    //
    // Action methods
    //

    @IBAction func popupAction(_ sender: NSPopUpButton!) {

        refresh()
    }

    @IBAction func zoneAction(_ sender: NSButton!) {

        pal.toggle()
    }

    @IBAction func applyAction(_ sender: NSButton!) {

        guard let emu = emu, emu.poweredOff else { return }

        emu.setAsyncModel(selectedModel)
        emu.setAsync(.AMIGA_VIDEO_FORMAT, value: (pal ? VideoFormat.PAL : VideoFormat.NTSC).rawValue)
        emu.setAsyncMonitor(selectedMonitor)
        emu.setAsyncProfile(selectedProfile)
    }

    override func preset(tag: Int) {

        switch tag {

        case 0:

            // Standard settings
            select(model: .A2000, monitor: .TFT, profile: .DEFAULT, pal: true)
            applyAction(apply)

        default:
            fatalError()
        }
    }

    override func save() {

        log(.lifetime)

        // Save all settings the presets can change
        config?.saveHardwareUserDefaults()
        config?.saveVideoUserDefaults()
        config?.savePerformanceUserDefaults()
        config?.saveCompatibilityUserDefaults()
        config?.savePeripheralsUserDefaults()
    }
}
