// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Cocoa

class SettingsSplitViewController: NSSplitViewController {

    func instantiate(_ identifier: String) -> Any {

        let main = NSStoryboard(name: "Settings", bundle: nil)
        return main.instantiateController(withIdentifier: identifier)
    }

    var windowController: SettingsWindowController? {
        view.window?.windowController as? SettingsWindowController
    }
    var toolbar: SettingsToolbar? {
        windowController?.window?.toolbar as? SettingsToolbar
    }
    private var sidebarVC: SidebarViewController? {
        return splitViewItems.first?.viewController as? SidebarViewController
    }

    // let main = NSStoryboard(name: "Settings", bundle: nil)

    lazy var generalVC: GeneralSettingsViewController = {
        instantiate("General") as! GeneralSettingsViewController
    }()
    lazy var capturesVC: CapturesSettingsViewController = {
        instantiate("Captures") as! CapturesSettingsViewController
    }()
    lazy var controlsVC: ControlsSettingsViewController = {
        instantiate("Controls") as! ControlsSettingsViewController
    }()
    lazy var devicesVC: DevicesSettingsViewController = {
        instantiate("Devices") as! DevicesSettingsViewController
    }()
    lazy var romsVC: RomSettingsViewController = {
        instantiate("Roms") as! RomSettingsViewController
    }()
    lazy var hardwareVC = instantiate("Hardware") as! HardwareSettingsViewController
    /*
    lazy var hardwareVC: HardwareSettingsViewController = {
        instantiate("Hardware") as! HardwareSettingsViewController
    }()
     */
    lazy var peripheralsVC: PeripheralsSettingsViewController = {
        instantiate("Peripherals") as! PeripheralsSettingsViewController
    }()
    lazy var compatibilityVC: CompatibilitySettingsViewController = {
        instantiate("Compatibility") as! CompatibilitySettingsViewController
    }()
    lazy var performanceVC = instantiate("Performance") as! PerformanceSettingsViewController
    lazy var audioVC = instantiate("Audio") as! AudioSettingsViewController
    lazy var videoVC = instantiate("Video") as! VideoSettingsViewController

    var current: SettingsViewController?

    override func viewDidLoad() {

        super.viewDidLoad()
        sidebarVC?.selectionHandler = { [weak self] item in
            self?.showContent(for: item)
        }
    }

    private func showContent(for item: SidebarItem) {

        switch item.title {

        case "General":         current = generalVC
        case "Captures":        current = capturesVC
        case "Controls":        current = controlsVC
        case "Devices":         current = devicesVC
        case "Roms":            current = romsVC
        case "Hardware":        current = hardwareVC
        case "Peripherals":     current = peripheralsVC
        case "Performance":     current = performanceVC
        case "Compatibility":   current = compatibilityVC
        case "Audio":           current = audioVC
        case "Video":           current = videoVC

        default:            fatalError()
        }

        // Remove the old content pane
        removeSplitViewItem(splitViewItems[1])

        // Create a new split view item for the new content
        let newItem = NSSplitViewItem(viewController: current!)
        addSplitViewItem(newItem)
        current!.activate()

        // toolbar!.update(presets: current?.presets ?? [])
    }

    override func keyDown(with event: NSEvent) {

        print("keyDown: \(event)")
        current?.keyDown(with: event)
    }

    override func flagsChanged(with event: NSEvent) {

        print("flagsChanged: \(event)")
        current?.flagsChanged(with: event)
    }

    @IBAction func presetAction(_ sender: NSPopUpButton) {

        print("presetAction")
        current?.preset(tag: sender.selectedTag())
        current?.refresh()
    }

    @IBAction func saveAction(_ sender: Any) {

        print("saveAction")
        current?.save()
        current?.refresh()
    }
}
