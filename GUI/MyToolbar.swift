// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension NSImage {

    static func sf(_ name: String, size: Int = 28, description: String? = nil) -> NSImage {

        let config = NSImage.SymbolConfiguration(pointSize: CGFloat(size), weight: .light, scale: .small)
        let img = NSImage(systemSymbolName: name, accessibilityDescription: description)!
        return img.withSymbolConfiguration(config)!
    }
}

extension NSToolbarItem.Identifier {

    static let inspectors = NSToolbarItem.Identifier("Inspectors")
    static let snapshots = NSToolbarItem.Identifier("Snapshots")
    static let port1 = NSToolbarItem.Identifier("Port1")
    static let port2 = NSToolbarItem.Identifier("Port2")
    static let keyboard = NSToolbarItem.Identifier("Keyboard")
    static let settings = NSToolbarItem.Identifier("Settings")
    static let controls = NSToolbarItem.Identifier("Controls")
}

func image(_ name: String, description: String? = nil) -> NSImage {

    let config = NSImage.SymbolConfiguration(pointSize: 28, weight: .light, scale: .small)
    let img = NSImage(systemSymbolName: name, accessibilityDescription: description)!
    return img.withSymbolConfiguration(config)!
}

@MainActor
class MyToolbar: NSToolbar, NSToolbarDelegate {

    var controller: MyController!
    var amiga: EmulatorProxy { return controller.emu }

    var inspectors: MyToolbarItemGroup!
    var snapshots: MyToolbarItemGroup!
    var port1: MyToolbarPopupItem!
    var port2: MyToolbarPopupItem!
    var keyboard: MyToolbarItemGroup!
    var settings: MyToolbarItemGroup!
    var controls: MyToolbarItemGroup!

    // Set to true to gray out all toolbar items
    var globalDisable = false

    init() {

        super.init(identifier: "MyToolbar")
        self.delegate = self
        self.allowsUserCustomization = true
        self.displayMode = .iconAndLabel
    }

    override init(identifier: NSToolbar.Identifier) {

        super.init(identifier: identifier)
        self.delegate = self
        self.allowsUserCustomization = true
        self.displayMode = .iconAndLabel
    }

    func toolbarAllowedItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {

        return [ .inspectors,
                 .snapshots,
                 .port1,
                 .port2,
                 .keyboard,
                 .settings,
                 .controls,
                 .flexibleSpace ]
    }

    func toolbarDefaultItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {

        return [ .inspectors, .flexibleSpace,
                 .snapshots, .flexibleSpace,
                 .port1, .port2, .flexibleSpace,
                 .keyboard, .flexibleSpace,
                 .settings, .flexibleSpace,
                 .controls ]
    }

    func toolbar(_ toolbar: NSToolbar,
                 itemForItemIdentifier itemIdentifier: NSToolbarItem.Identifier,
                 willBeInsertedIntoToolbar flag: Bool) -> NSToolbarItem? {

        let portItems = [ (NSImage.sf("nosign", size: 22), "None", -1),
                          (NSImage.sf("computermouse.fill", size: 22), "Mouse", 0),
                          (NSImage.sf("arrowkeys", size: 22), "Keyset 1", 1),
                          (NSImage.sf("arrowkeys", size: 22), "Keyset 2", 2),
                          (NSImage.sf("gamecontroller.fill", size: 22), "Gamepad 1", 3),
                          (NSImage.sf("gamecontroller.fill", size: 22), "Gamepad 2", 4),
                          (NSImage.sf("gamecontroller.fill", size: 22), "Gamepad 3", 5),
                          (NSImage.sf("gamecontroller.fill", size: 22), "Gamepad 4", 6) ]

        switch itemIdentifier {

        case .inspectors:

            let images: [NSImage] = [

                image("magnifyingglass"),
                image("gauge.with.needle"),
                image("text.alignleft")
            ]

            let actions: [Selector] = [

                #selector(inspectorAction),
                #selector(dashboardAction),
                #selector(consoleAction)
            ]

            inspectors = MyToolbarItemGroup(identifier: .inspectors,
                                          images: images,
                                          actions: actions,
                                          target: self,
                                          label: "Inspectors")
            return inspectors

        case .snapshots:

            let images: [NSImage] = [

                image("tray.and.arrow.down.fill"),
                image("tray.and.arrow.up.fill"),
                image("clock.arrow.trianglehead.counterclockwise.rotate.90")
            ]

            let actions: [Selector] = [

                #selector(takeSnapshotAction),
                #selector(restoreSnapshotAction),
                #selector(browseSnapshotAction)
            ]

            snapshots = MyToolbarItemGroup(identifier: .snapshots,
                                           images: images,
                                           actions: actions,
                                           target: self,
                                           label: "Snapshots")
            return snapshots

        case .port1:

            port1 = MyToolbarPopupItem(identifier: .port1,
                                     menuItems: portItems,
                                     image: image("gear"),
                                     action: #selector(port1Action(_:)),
                                     target: self,
                                     label: "Port 1")
            return port1

        case .port2:

            port2 = MyToolbarPopupItem(identifier: .port2,
                                     menuItems: portItems,
                                     image: image("gear"),
                                     action: #selector(port2Action(_:)),
                                     target: self,
                                     label: "Port 2")
            return port2

        case .keyboard:

            keyboard = MyToolbarItemGroup(identifier: .keyboard,
                                          images: [image("keyboard")],
                                          actions: [#selector(keyboardAction)],
                                          target: self,
                                          label: "Keyboard")
            return keyboard

        case .settings:

            settings = MyToolbarItemGroup(identifier: .settings,
                                          images: [image("gear")],
                                          actions: [#selector(settingsAction)],
                                          target: self,
                                          label: "Settings")
            return settings

        case .controls:

            let images: [NSImage] = [

                image("pause.circle"), // play.circle
                image("arrow.counterclockwise.circle"),
                image("power")
            ]

            let actions: [Selector] = [

                #selector(runAction),
                #selector(resetAction),
                #selector(powerAction)
            ]

            controls = MyToolbarItemGroup(identifier: .controls,
                                           images: images,
                                           actions: actions,
                                           target: self,
                                           label: "Controls")
            return controls

        default:
            return nil
        }
    }

    override func validateVisibleItems() {

        // REMOVE ASAP
        globalDisable = false

        // Take care of the global disable flag
        for item in items { item.isEnabled = !globalDisable }

        // Disable the keyboard button if the virtual keyboard is open
        if  controller.virtualKeyboard?.window?.isVisible == true {
            (keyboard.view as? NSButton)?.isEnabled = false
        }

        // Disable the snapshot revert button if no snapshots have been taken
        snapshots.setEnabled(controller.snapshotCount > 0, forSegment: 1)

        // Update input devices
        controller.gamePadManager.refresh(menu: port1.menu)
        controller.gamePadManager.refresh(menu: port2.menu)
        port1.selectItem(withTag: controller.config.gameDevice1)
        port2.selectItem(withTag: controller.config.gameDevice2)

        port1.menuFormRepresentation = nil
        port2.menuFormRepresentation = nil
        keyboard.menuFormRepresentation = nil
        settings.menuFormRepresentation = nil
        controls.menuFormRepresentation = nil
    }

    func updateToolbar() {

        if amiga.poweredOn {
            controls.setEnabled(true, forSegment: 0) // Pause
            controls.setEnabled(true, forSegment: 1) // Reset
            controls.setToolTip("Power off", forSegment: 2) // Power
        } else {
            controls.setEnabled(false, forSegment: 0) // Pause
            controls.setEnabled(false, forSegment: 1) // Reset
            controls.setToolTip("Power on", forSegment: 2) // Power
        }
        if amiga.running {
            controls.setToolTip("Pause", forSegment: 0)
            controls.setImage(NSImage.sf("pause.circle"), forSegment: 0)
        } else {
            controls.setToolTip("Run", forSegment: 0)
            controls.setImage(NSImage.sf("play.circle"), forSegment: 0)
        }
    }

    //
    // Action methods
    //

    @objc private func inspectorAction() {

        print("inspectorAction")
        controller.inspectorAction(self)
    }

    @objc private func dashboardAction() {

        print("inspectorAction")
        controller.dashboardAction(self)
    }

    @objc private func consoleAction() {

        print("consoleAction")
        controller.consoleAction(self)
    }

    @objc private func takeSnapshotAction() {

        print("takeSnapshotAction")
        controller.takeSnapshotAction(self)
    }

    @objc private func restoreSnapshotAction() {

        print("restoreSnapshotAction")
        controller.restoreSnapshotAction(self)
    }

    @objc private func browseSnapshotAction() {

        print("browseSnapshotAction")
        controller.browseSnapshotsAction(self)
    }

    @objc private func port1Action(_ sender: NSMenuItem) {

        print("port1Action \(sender.tag)")
        controller.config.gameDevice1 = sender.tag
    }

    @objc private func port2Action(_ sender: NSMenuItem) {

        print("port2Action \(sender.tag)")
        controller.config.gameDevice2 = sender.tag
    }

    @objc private func keyboardAction() {

        print("keyboardAction")
        if controller.virtualKeyboard == nil {
            controller.virtualKeyboard = VirtualKeyboardController.make(parent: controller)
        }
        if controller.virtualKeyboard?.window?.isVisible == false {
            controller.virtualKeyboard?.showAsSheet()
        }
    }

    @objc private func settingsAction() {

        print("settingsAction")
        controller.settingsAction(self)
    }

    @objc private func runAction() {

        print("My runAction triggered")
        controller.stopAndGoAction(self)
    }

    @objc private func resetAction() {

        print("My resetAction triggered")
        controller.resetAction(self)
    }

    @objc private func powerAction() {

        print("My powerAction triggered")
        controller.powerAction(self)
    }
}
