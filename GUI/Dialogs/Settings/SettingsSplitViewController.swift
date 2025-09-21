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

    var windowController: SettingsWindowController? {
        view.window?.windowController as? SettingsWindowController
    }
    var toolbar: SettingsToolbar? {
        windowController?.window?.toolbar as? SettingsToolbar
    }
    private var sidebarVC: SidebarViewController? {
        return splitViewItems.first?.viewController as? SidebarViewController
    }

    let main = NSStoryboard(name: "Settings", bundle: nil)

    lazy var generalVC: GeneralSettingsViewController = {
        return main.instantiateController(withIdentifier: "GeneralSettingsViewController") as! GeneralSettingsViewController
    }()
    lazy var controlsVC: ControlsSettingsViewController = {
        return main.instantiateController(withIdentifier: "ControlsSettingsViewController") as! ControlsSettingsViewController
    }()
    lazy var devicesVC: DevicesSettingsViewController = {
        return main.instantiateController(withIdentifier: "DevicesSettingsViewController") as! DevicesSettingsViewController
    }()
    lazy var hardwareVC: HardwareSettingsViewController = {
        return main.instantiateController(withIdentifier: "HardwareSettingsViewController") as! HardwareSettingsViewController
    }()

    var current: SettingsViewController?

    override func viewDidLoad() {

        super.viewDidLoad()
        sidebarVC?.selectionHandler = { [weak self] item in
            self?.showContent(for: item)
        }
    }

    private func showContent(for item: SidebarItem) {

        switch item.identifier.rawValue {

        case "general":     current = generalVC
        case "controls":    current = controlsVC
        case "devices":     current = devicesVC
        case "hardware":    current = hardwareVC
        default:            fatalError()
        }

        // Remove the old content pane
        removeSplitViewItem(splitViewItems[1])

        // Create a new split view item for the new content
        let newItem = NSSplitViewItem(viewController: current!)
        addSplitViewItem(newItem)
        current!.activate()

        toolbar!.update(presets: current?.presets ?? [])

        /*
        current!.view.window?.makeFirstResponder(currentVC)
        current!.refresh()
        */
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
