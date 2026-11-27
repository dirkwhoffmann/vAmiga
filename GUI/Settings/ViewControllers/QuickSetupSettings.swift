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

    // Description
    @IBOutlet weak var text: NSTextField!
    @IBOutlet weak var apply: NSButton!

    // Popup tag of the entry representing a custom configuration
    static let custom = -1

    // The presets that matched the emulator configuration in the last refresh
    private var shownModel: Int?
    private var shownMonitor: Int?
    private var shownProfile: Int?

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

        apply.target = self
        apply.action = #selector(applyAction(_:))
    }

    func setup(_ popup: NSPopUpButton, items: [(tag: Int, title: String)]) {

        popup.removeAllItems()
        for item in items {

            popup.addItem(withTitle: item.title)
            popup.lastItem!.tag = item.tag
        }
        popup.menu!.addItem(NSMenuItem.separator())
        popup.addItem(withTitle: "Custom")
        popup.lastItem!.tag = Self.custom

        popup.target = self
        popup.action = #selector(popupAction(_:))
    }

    //
    // Selected presets
    //

    /* Note: The rawValue initializers of imported C++ enums never fail. Hence,
     * the custom entry must be filtered out explicitly.
     */
    func selectedTag(_ popup: NSPopUpButton) -> Int? {

        let tag = popup.selectedTag()
        return tag == Self.custom ? nil : tag
    }

    var selectedModel: ModelPreset? { selectedTag(modelPopup).map { ModelPreset(rawValue: $0)! } }
    var selectedMonitor: MonitorPreset? { selectedTag(monitorPopup).map { MonitorPreset(rawValue: $0)! } }
    var selectedProfile: ProfilePreset? { selectedTag(profilePopup).map { ProfilePreset(rawValue: $0)! } }

    // Indicates whether applying a popup's selection would change something
    func isPending(_ popup: NSPopUpButton, current: Int) -> Bool {

        return popup.selectedTag() != Self.custom && popup.selectedTag() != current
    }

    //
    // Methods from SettingsViewController
    //

    override func activate() {

        // Show the current configuration, discarding unapplied selections
        shownModel = nil
        shownMonitor = nil
        shownProfile = nil

        super.activate()
    }

    override func refresh() {

        super.refresh()

        guard let emu = emu else { return }

        // Select the matching presets if the configuration has changed
        sync(modelPopup, current: emu.modelPreset, shown: &shownModel)
        sync(monitorPopup, current: emu.monitorPreset, shown: &shownMonitor)
        sync(profilePopup, current: emu.profilePreset, shown: &shownProfile)

        // The hardware can only be changed in power-off state
        presetPopup.isEnabled = emu.poweredOff
        modelPopup.isEnabled = emu.poweredOff

        // Only offer to apply if a selection differs from the configuration
        let model = emu.poweredOff && isPending(modelPopup, current: emu.modelPreset)
        let monitor = isPending(monitorPopup, current: emu.monitorPreset)
        let profile = isPending(profilePopup, current: emu.profilePreset)
        apply.isEnabled = model || monitor || profile

        // Describe the configuration
        text.attributedStringValue = describe(emu)
    }

    func sync(_ popup: NSPopUpButton, current: Int, shown: inout Int?) {

        if current != shown {

            popup.selectItem(withTag: current)
            shown = current
        }

        // Only offer the custom entry if the configuration is custom
        popup.item(at: popup.indexOfItem(withTag: Self.custom))?.isHidden = current != Self.custom
    }

    //
    // Description
    //

    // Describes the configuration the selected presets result in
    func describe(_ emu: EmulatorProxy) -> NSAttributedString {

        // Collect the option values of the selected presets
        var preset: [Int: Int] = [:]
        func add(_ options: [NSNumber: NSNumber]) {
            for (opt, value) in options { preset[opt.intValue] = value.intValue }
        }
        if let model = selectedModel { add(EmulatorProxy.options(forModel: model)) }
        if let monitor = selectedMonitor { add(EmulatorProxy.options(forMonitor: monitor)) }
        if let profile = selectedProfile { add(EmulatorProxy.options(forProfile: profile)) }

        // Options not set by a preset keep their current values
        func value(_ opt: Option, _ id: Int = 0) -> Int {
            preset[opt.rawValue] ?? emu.get(opt, id: id)
        }

        var features: [String] = []

        //
        // Model
        //

        let cpus = ["68000", "68010", "68EC020", "68020", "68EC030",
                    "68030", "68EC040", "68LC040", "68040"]
        let cpu = cpus.indices.contains(value(.CPU_REVISION)) ? cpus[value(.CPU_REVISION)] : "680x0"
        let pal = value(.AMIGA_VIDEO_FORMAT) == VideoFormat.PAL.rawValue
        let mhz = (pal ? 7.09379 : 7.15909) * Double(max(1, value(.CPU_OVERCLOCKING)))
        features.append("Motorola \(cpu) CPU at \(Int(mhz.rounded())) MHz")

        let agnus = AgnusRevision(rawValue: value(.AGNUS_REVISION))
        let denise = DeniseRevision(rawValue: value(.DENISE_REVISION))
        switch (agnus, denise) {
        case (.AGA, _), (_, .AGA):
            features.append("Advanced Graphics Architecture (AGA)")
        case (.ECS_1MB, .ECS), (.ECS_2MB, .ECS):
            features.append("Enhanced Chip Set (ECS)")
        case (.ECS_1MB, _), (.ECS_2MB, _):
            features.append("ECS Agnus with OCS Denise")
        default:
            features.append("Original Chip Set (OCS)")
        }

        features.append("\(memory(value(.MEM_CHIP_RAM))) Chip RAM")
        if value(.MEM_SLOW_RAM) > 0 { features.append("\(memory(value(.MEM_SLOW_RAM))) Slow RAM") }
        if value(.MEM_FAST_RAM) > 0 { features.append("\(memory(value(.MEM_FAST_RAM))) Fast RAM") }

        var drives: [String: Int] = [:]
        var order: [String] = []
        for n in 0...3 where emu.get(.DRIVE_CONNECT, drive: n) != 0 {
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
            features.append("\(count) \(type) floppy drive" + (count == 1 ? "" : "s"))
        }
        let hds = (0...3).filter { emu.get(.HDC_CONNECT, drive: $0) != 0 }.count
        if hds > 0 { features.append("\(hds) hard drive" + (hds == 1 ? "" : "s")) }

        features.append(pal ? "PAL video (50 Hz)" : "NTSC video (60 Hz)")
        if value(.MON_FLICKER_FIXER) != 0 { features.append("Built-in flicker fixer") }

        //
        // Monitor
        //

        var effects: [String] = []
        if value(.MON_SCANLINES) != 0 { effects.append("scanlines") }
        if value(.MON_DOTMASK) != 0 { effects.append("dot mask") }
        if value(.MON_BLOOM) != 0 { effects.append("bloom") }
        if effects.isEmpty {
            features.append("Crisp flat-panel picture")
        } else {
            features.append("Classic CRT look with " + list(effects))
        }

        //
        // Profile
        //

        if value(.AMIGA_RUN_AHEAD) > 0 {
            features.append("\(value(.AMIGA_RUN_AHEAD))-frame run-ahead")
        }
        if value(.AMIGA_VSYNC) != 0 {
            features.append("Synced to the host display (VSYNC)")
        }
        if value(.AMIGA_WARP_BOOT) > 0 {
            features.append("\(value(.AMIGA_WARP_BOOT))-second warp boot")
        }
        switch Warp(rawValue: value(.AMIGA_WARP_MODE)) {
        case .AUTO: features.append("Automatic warp during disk access")
        case .ALWAYS: features.append("Permanent warp mode")
        default: break
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

        let headline = modelPopup.selectedItem?.tag == Self.custom ?
        "Custom Amiga" : (modelPopup.titleOfSelectedItem ?? "Amiga")

        let result = NSMutableAttributedString()
        result.append(NSAttributedString(
            string: headline + "\n",
            attributes: [.font: NSFont.boldSystemFont(ofSize: 17)]))

        let style = NSMutableParagraphStyle()
        style.headIndent = 14
        style.paragraphSpacingBefore = 3
        result.append(NSAttributedString(
            string: features.map { "•\t" + $0 }.joined(separator: "\n"),
            attributes: [.font: NSFont.systemFont(ofSize: NSFont.systemFontSize),
                         .paragraphStyle: style]))

        return result
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

    @IBAction func applyAction(_ sender: NSButton!) {

        guard let emu = emu else { return }

        if emu.poweredOff, isPending(modelPopup, current: emu.modelPreset), let model = selectedModel {
            emu.setAsyncModel(model)
        }
        if isPending(monitorPopup, current: emu.monitorPreset), let monitor = selectedMonitor {
            emu.setAsyncMonitor(monitor)
        }
        if isPending(profilePopup, current: emu.profilePreset), let profile = selectedProfile {
            emu.setAsyncProfile(profile)
        }
    }

    override func preset(tag: Int) {

        switch tag {

        case 0:

            // Standard settings
            emu?.setAsyncModel(.A2000)
            emu?.setAsyncMonitor(.TFT)
            emu?.setAsyncProfile(.DEFAULT)

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
