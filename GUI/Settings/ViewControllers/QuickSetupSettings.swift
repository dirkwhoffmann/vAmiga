// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class QuickSetupSettingsViewController: SettingsViewController {

    // Model
    @IBOutlet weak var modelPopup: NSPopUpButton!
    @IBOutlet weak var modelText: NSTextField!
    @IBOutlet weak var modelApply: NSButton!

    // Monitor
    @IBOutlet weak var monitorPopup: NSPopUpButton!
    @IBOutlet weak var monitorText: NSTextField!
    @IBOutlet weak var monitorApply: NSButton!

    // Profile
    @IBOutlet weak var profilePopup: NSPopUpButton!
    @IBOutlet weak var profileText: NSTextField!
    @IBOutlet weak var profileApply: NSButton!

    // Popup tag of the entry representing a custom configuration
    static let custom = -1

    // The presets that matched the emulator configuration in the last refresh
    private var shownModel: Int?
    private var shownMonitor: Int?
    private var shownProfile: Int?

    // Popup entries
    typealias Item = (tag: Int, title: String, text: String)

    let modelItems: [Item] = [

        (ModelPreset.A500.rawValue, "Amiga 500",
         "The classic home computer of the late 1980s with a 7 MHz 68000 CPU, " +
         "the original chipset (OCS), and 512 KB Chip RAM plus 512 KB Slow RAM. " +
         "Runs the vast majority of games and demos."),

        (ModelPreset.A1000.rawValue, "Amiga 1000",
         "The first Amiga, released in 1985. It is equipped with the earliest " +
         "Agnus revision and 512 KB Chip RAM plus 512 KB Slow RAM. " +
         "Choose it for very early software."),

        (ModelPreset.A2000.rawValue, "Amiga 2000",
         "The expandable desktop model with a 68000 CPU, a 1 MB ECS Agnus, " +
         "an OCS Denise, and 512 KB Chip RAM plus 512 KB Slow RAM."),

        (ModelPreset.A500P.rawValue, "Amiga 500+",
         "The revised Amiga 500 with the Enhanced Chip Set (ECS) and " +
         "512 KB Chip RAM plus 512 KB Slow RAM. " +
         "Some older games do not run on this model."),

        (ModelPreset.A600.rawValue, "Amiga 600",
         "The compact successor of the Amiga 500+. It shares the Enhanced " +
         "Chip Set (ECS) and comes with 1 MB Chip RAM."),

        (ModelPreset.A1200.rawValue, "Amiga 1200",
         "The 32-bit home computer with a 14 MHz 68EC020 CPU, the Advanced " +
         "Graphics Architecture (AGA), and 2 MB Chip RAM. " +
         "Required for AGA games and demos."),

        (ModelPreset.A3000.rawValue, "Amiga 3000",
         "The high-end workstation with a 28 MHz 68030 CPU, the Enhanced Chip " +
         "Set (ECS), 1 MB Chip RAM, and a built-in flicker fixer for " +
         "flicker-free interlace modes.")
    ]

    let monitorItems: [Item] = [

        (MonitorPreset.TFT.rawValue, "TFT",
         "A crisp and sharp image as seen on a modern flat-panel display."),

        (MonitorPreset.CRT.rawValue, "CRT",
         "Mimics a classic cathode ray tube monitor with visible scanlines, " +
         "a dot mask, and a soft glow.")
    ]

    let profileItems: [Item] = [

        (ProfilePreset.DEFAULT.rawValue, "Default",
         "Runs the Amiga at its original speed with authentic floppy drives. " +
         "Warp mode is never activated."),

        (ProfilePreset.GAMING.rawValue, "Gaming",
         "Optimized for smooth and responsive gameplay. Syncs to the refresh " +
         "rate of the host display, reduces input lag with run-ahead, and " +
         "warps automatically during disk access."),

        (ProfilePreset.PRODUCTIVITY.rawValue, "Productivity",
         "Optimized for the Workbench and applications. Boots in warp mode, " +
         "uses fast floppy drives without mechanical delays, and warps " +
         "automatically during disk access.")
    ]

    let customModelText =
    "The hardware does not match any of the predefined models. " +
    "It has been customized in the Hardware or Video settings."

    let customMonitorText =
    "The image settings do not match any of the predefined monitors. " +
    "They have been customized in the Video settings."

    let customProfileText =
    "The emulator settings do not match any of the predefined profiles. " +
    "They have been customized in the Performance, Compatibility, " +
    "or Peripherals settings."

    override var showLock: Bool { true }

    override func viewDidLoad() {

        log(.lifetime)

        setup(modelPopup, items: modelItems, action: #selector(popupAction(_:)))
        setup(monitorPopup, items: monitorItems, action: #selector(popupAction(_:)))
        setup(profilePopup, items: profileItems, action: #selector(popupAction(_:)))

        modelApply.target = self
        modelApply.action = #selector(modelApplyAction(_:))
        monitorApply.target = self
        monitorApply.action = #selector(monitorApplyAction(_:))
        profileApply.target = self
        profileApply.action = #selector(profileApplyAction(_:))
    }

    func setup(_ popup: NSPopUpButton, items: [Item], action: Selector) {

        popup.removeAllItems()
        for item in items {

            popup.addItem(withTitle: item.title)
            popup.lastItem!.tag = item.tag
        }
        popup.menu!.addItem(NSMenuItem.separator())
        popup.addItem(withTitle: "Custom")
        popup.lastItem!.tag = Self.custom

        popup.target = self
        popup.action = action
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

        // Update the descriptions and buttons
        refresh(modelPopup, text: modelText, apply: modelApply,
                items: modelItems, customText: customModelText)
        refresh(monitorPopup, text: monitorText, apply: monitorApply,
                items: monitorItems, customText: customMonitorText)
        refresh(profilePopup, text: profileText, apply: profileApply,
                items: profileItems, customText: customProfileText)

        // The hardware can only be changed in power-off state
        presetPopup.isEnabled = emu.poweredOff
        modelPopup.isEnabled = emu.poweredOff
        modelApply.isEnabled = modelApply.isEnabled && emu.poweredOff
    }

    func sync(_ popup: NSPopUpButton, current: Int, shown: inout Int?) {

        if current != shown {

            popup.selectItem(withTag: current)
            shown = current
        }

        // Only offer the custom entry if the configuration is custom
        popup.item(at: popup.indexOfItem(withTag: Self.custom))?.isHidden = current != Self.custom
    }

    func refresh(_ popup: NSPopUpButton, text: NSTextField, apply: NSButton,
                 items: [Item], customText: String) {

        let tag = popup.selectedTag()

        text.stringValue = items.first { $0.tag == tag }?.text ?? customText
        apply.isEnabled = tag != Self.custom
    }

    //
    // Action methods
    //

    @IBAction func popupAction(_ sender: NSPopUpButton!) {

        refresh()
    }

    @IBAction func modelApplyAction(_ sender: NSButton!) {

        if let model = ModelPreset(rawValue: modelPopup.selectedTag()) {
            emu?.setAsyncModel(model)
        }
    }

    @IBAction func monitorApplyAction(_ sender: NSButton!) {

        if let monitor = MonitorPreset(rawValue: monitorPopup.selectedTag()) {
            emu?.setAsyncMonitor(monitor)
        }
    }

    @IBAction func profileApplyAction(_ sender: NSButton!) {

        if let profile = ProfilePreset(rawValue: profilePopup.selectedTag()) {
            emu?.setAsyncProfile(profile)
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
