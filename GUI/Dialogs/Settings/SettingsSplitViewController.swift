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

    /*
    private lazy var shaderVC: ShaderPreferencesViewController = {
        return main.instantiateController(withIdentifier: "ShaderPreferencesViewController") as! ShaderPreferencesViewController
    }()
    private lazy var recorderVC: RecorderPreferencesViewController = {
        return main.instantiateController(withIdentifier: "RecorderPreferencesViewController") as! RecorderPreferencesViewController
    }()
    */

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
        let newVC: NSViewController

        switch item.identifier.rawValue {
        case "general":
            newVC = generalVC
            /*
        case "shader":
            newVC = shaderVC
        case "recorder":
            newVC = recorderVC
             */
        default:
            newVC = NSViewController()
        }

        // Remove the old content pane
        removeSplitViewItem(splitViewItems[1])

        // Create a new split view item for the new content
        let newItem = NSSplitViewItem(viewController: newVC)
        addSplitViewItem(newItem)
    }
}
