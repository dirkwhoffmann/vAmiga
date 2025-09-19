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

    let main = NSStoryboard(name: "Settings", bundle: nil)

    private lazy var generalVC: GeneralSettingsViewController = {
        return main.instantiateController(withIdentifier: "GeneralSettingsViewController") as! GeneralSettingsViewController
    }()
    private lazy var controlsVC: ControlsSettingsViewController = {
        return main.instantiateController(withIdentifier: "ControlsSettingsViewController") as! ControlsSettingsViewController
    }()
    private lazy var devicesVC: DevicesSettingsViewController = {
        return main.instantiateController(withIdentifier: "DevicesSettingsViewController") as! DevicesSettingsViewController
    }()

    var currentVC: SettingsViewController?

    private var sidebarVC: SidebarViewController? {
        return splitViewItems.first?.viewController as? SidebarViewController
    }

    override func viewDidLoad() {

        super.viewDidLoad()
        sidebarVC?.selectionHandler = { [weak self] item in
            self?.showContent(for: item)
        }
    }

    private func showContent(for item: SidebarItem) {

        // let newVC: SettingsViewController

        print("showContent")

        switch item.identifier.rawValue {

        case "general":     currentVC = generalVC
        case "controls":    currentVC = controlsVC
        case "devices":     currentVC = devicesVC
        default:            fatalError()
        }

        // Remove the old content pane
        removeSplitViewItem(splitViewItems[1])

        // Create a new split view item for the new content
        let newItem = NSSplitViewItem(viewController: currentVC!)
        addSplitViewItem(newItem)
        currentVC!.refresh()
    }

    @IBAction func presetAction(_ sender: NSPopUpButton) {

        print("presetAction")
        currentVC?.preset(tag: sender.selectedTag())
        currentVC?.refresh()
    }
}
