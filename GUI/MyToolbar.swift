// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension NSImage {

    static func sf(_ name: String, description: String? = nil) -> NSImage {

        let config = NSImage.SymbolConfiguration(pointSize: 28, weight: .light, scale: .small)
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

    static let myItem = NSToolbarItem.Identifier("MyItem")
    static let myOtherItem = NSToolbarItem.Identifier("MyOtherItem")
}

func image(_ name: String, description: String? = nil) -> NSImage {

    let config = NSImage.SymbolConfiguration(pointSize: 28, weight: .light, scale: .small)
    let img = NSImage(systemSymbolName: name, accessibilityDescription: description)!
    return img.withSymbolConfiguration(config)!
}

@MainActor
class MyToolbar: NSToolbar, NSToolbarDelegate {

    var inspectors: NSToolbarItem!
    var snapshots: NSToolbarItem!
    var port1: NSToolbarItem!
    var port2: NSToolbarItem!
    var keyboard: NSToolbarItem!
    var settings: NSToolbarItem!
    var controls: NSToolbarItem!

    var controller: MyController!

    // var amiga: EmulatorProxy { return controller.emu }
    // Set to true to gray out all toolbar items
    var globalDisable = false

    let images: [NSImage] = [
        image("magnifyingglass"),
        image("gauge.with.needle"),
        image("text.alignleft")
    ]

    let actions: [Selector] = [
        #selector(inspectorAction),
        #selector(gearAction),
        #selector(gearAction)
    ]

    init() {

        print("MyToolbar: init()")
        super.init(identifier: "MyToolbar")
        self.delegate = self
        self.allowsUserCustomization = true
        self.displayMode = .iconAndLabel
    }

    override init(identifier: NSToolbar.Identifier) {

        print("MyToolbar: init(identifier:)")
        super.init(identifier: identifier)
        self.delegate = self
        self.allowsUserCustomization = true
        self.displayMode = .iconAndLabel
    }

    /*
    required init?(coder: NSCoder) {

        super.init(coder: coder)
        self.delegate = self
    }
    */

    // MARK: - NSToolbarDelegate

    func toolbarAllowedItemIdentifiers(_ toolbar: NSToolbar) -> [NSToolbarItem.Identifier] {

        return [ .inspectors,
                 .snapshots,
                 .port1,
                 .port2,
                 .keyboard,
                 .settings,
                 .controls ,
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

            port1 = MyToolbarItemGroup(identifier: .port1,
                                     images: [image("keyboard")],
                                     actions: [#selector(port1Action)],
                                     target: self,
                                     label: "Port 1")

            return port1

        case .port2:

            port2 = ToolbarPopupItem(identifier: .port2,
                                     menuItems: ["Hey", "Du", "Ja"],
                                     image: image("gear"),
                                     action: #selector(gearAction),
                                     target: self,
                                     label: "Port 2")
            return port2

        case .keyboard:

            keyboard = MyToolbarItemGroup(identifier: .keyboard,
                                        images: [image("keyboard")],
                                        actions: [#selector(gearAction)],
                                        target: self,
                                        label: "Keyboard")

            return keyboard

        case .settings:

            let item = NSToolbarItem(itemIdentifier: itemIdentifier)
            item.label = "Settings"
            item.paletteLabel = "Settings"
            // item.toolTip = ""
            item.image = image("gear")
            item.target = self
            // item.action = #selector()
            return item

        case .port2, .controls:

            let item = NSToolbarItem(itemIdentifier: itemIdentifier)
            item.label = "ToDo"
            item.paletteLabel = "ToDo"
            // item.toolTip = ""
            item.image = NSImage(systemSymbolName: "star", accessibilityDescription: nil)
            item.target = self
            // item.action = #selector()
            return item

        case .myItem:

            let item = NSToolbarItem(itemIdentifier: .myItem)
            item.label = "My Action"
            item.paletteLabel = "My Action"
            item.toolTip = "Does something cool"
            item.image = NSImage(systemSymbolName: "star", accessibilityDescription: nil)
            item.target = self
            item.action = #selector(myAction)
            return item

        case .myOtherItem:

            let item = NSToolbarItem(itemIdentifier: .myOtherItem)
            item.label = "Other"
            item.paletteLabel = "Other"
            item.image = NSImage(systemSymbolName: "gearshape", accessibilityDescription: nil)
            item.target = self
            item.action = #selector(myOtherAction)
            return item

        default:
            return nil
        }
    }

    //
    // Actions
    //

    @objc private func inspectorsAction(_ sender: NSSegmentedControl) {

        let index = sender.selectedSegment
        switch index {
        case 0:
            print("First button pressed")
            // call your first action here
        case 1:
            print("Second button pressed")
            // call your second action here
        case 2:
            print("Third button pressed")
            // call your third action here
        default:
            break
        }
    }

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

    @objc private func port1Action() {
        print("port1Action")

        let menu = NSMenu()
        menu.addItem(withTitle: "Item 1", action: #selector(menuAction), keyEquivalent: "")
        menu.addItem(withTitle: "Item 2", action: #selector(menuAction), keyEquivalent: "")


    }

    @objc private func menuAction() {
        print("My menuAction triggered")
    }

    @objc private func gearAction() {
        print("My gearAction triggered")
    }

    @objc private func myAction() {
        print("My Action triggered")
    }

    @objc private func myOtherAction() {
        print("Other Action triggered")
    }

    /*

    @IBOutlet weak var controller: MyController!

    // References to toolbar items
    @IBOutlet weak var controlPort1Item: NSToolbarItem!
    @IBOutlet weak var controlPort2Item: NSToolbarItem!
    @IBOutlet weak var keyboardItem: NSToolbarItem!
    @IBOutlet weak var settingsItem: NSToolbarItem!
    @IBOutlet weak var controlsItem: NSToolbarItem!

    // Reference to toolbar item objects
    @IBOutlet weak var controlPort1: NSPopUpButton!
    @IBOutlet weak var controlPort2: NSPopUpButton!
    @IBOutlet weak var snapshotSegCtrl: NSSegmentedControl!
    @IBOutlet weak var screenshotSegCtrl: NSSegmentedControl!
    @IBOutlet weak var controlsSegCtrl: NSSegmentedControl!

    override func validateVisibleItems() {

        // Take care of the global disable flag
        for item in items { item.isEnabled = !globalDisable }

        // Disable the keyboard button if the virtual keyboard is open
        if  controller.virtualKeyboard?.window?.isVisible == true {
            (keyboardItem.view as? NSButton)?.isEnabled = false
        }

        // Disable the snapshot revert button if no snapshots have been taken
        snapshotSegCtrl.setEnabled(controller.snapshotCount > 0, forSegment: 1)

        // Update input devices
        controller.gamePadManager.refresh(popup: controlPort1)
        controller.gamePadManager.refresh(popup: controlPort2)
        controlPort1.selectItem(withTag: controller.config.gameDevice1)
        controlPort2.selectItem(withTag: controller.config.gameDevice2)

        controlPort1Item.menuFormRepresentation = nil
        controlPort2Item.menuFormRepresentation = nil
        keyboardItem.menuFormRepresentation = nil
        settingsItem.menuFormRepresentation = nil
        controlsItem.menuFormRepresentation = nil
    }
    */

    func updateToolbar() {

        /*
        if amiga.poweredOn {
            controlsSegCtrl.setEnabled(true, forSegment: 0) // Pause
            controlsSegCtrl.setEnabled(true, forSegment: 1) // Reset
            controlsSegCtrl.setToolTip("Power off", forSegment: 2) // Power
        } else {
            controlsSegCtrl.setEnabled(false, forSegment: 0) // Pause
            controlsSegCtrl.setEnabled(false, forSegment: 1) // Reset
            controlsSegCtrl.setToolTip("Power on", forSegment: 2) // Power
        }
        if amiga.running {
            controlsSegCtrl.setToolTip("Pause", forSegment: 0)
            controlsSegCtrl.setImage(NSImage(named: "pauseTemplate"), forSegment: 0)
        } else {
            controlsSegCtrl.setToolTip("Run", forSegment: 0)
            controlsSegCtrl.setImage(NSImage(named: "runTemplate"), forSegment: 0)
        }
        */
    }
    
    //
    // Action methods
    //

    /*
    @IBAction
    func inspectAction(_ sender: NSSegmentedControl) {

        switch sender.selectedSegment {

        case 0: controller.inspectorAction(sender)
        case 1: controller.dashboardAction(sender)
        case 2: controller.consoleAction(sender)

        default:
            fatalError()
        }
    }
    
    @IBAction
    func snapshotAction(_ sender: NSSegmentedControl) {
        
        switch sender.selectedSegment {
            
        case 0: controller.takeSnapshotAction(self)
        case 1: controller.restoreSnapshotAction(self)
        case 2: controller.browseSnapshotsAction(self)
            
        default:
            fatalError()
        }
    }
    
    @IBAction
    func screenshotAction(_ sender: NSSegmentedControl) {
                
        switch sender.selectedSegment {
            
        case 0: controller.takeScreenshotAction(self)
        case 1: controller.browseScreenshotsAction(self)
            
        default:
            fatalError()
        }
    }

    @IBAction
    func port1Action(_ sender: Any) {
        
        if let popup = sender as? NSPopUpButton {
            controller.config.gameDevice1 = popup.selectedTag()
        }
    }
 
    @IBAction
    func port2Action(_ sender: Any) {
        
        if let popup = sender as? NSPopUpButton {
            controller.config.gameDevice2 = popup.selectedTag()
        }
    }
            
    @IBAction
    func keyboardAction(_ sender: Any!) {
        
        if controller.virtualKeyboard == nil {
            controller.virtualKeyboard = VirtualKeyboardController.make(parent: controller)
        }
        if controller.virtualKeyboard?.window?.isVisible == false {
            controller.virtualKeyboard?.showAsSheet()
        }
    }
    
    @IBAction
    func preferencesAction(_ sender: Any!) {

        controller.settingsAction(sender)
    }

    @IBAction
    func controlsAction(_ sender: NSSegmentedControl) {

        switch sender.selectedSegment {

        case 0: controller.stopAndGoAction(self)
        case 1: controller.resetAction(self)
        case 2: controller.powerAction(self)

        default:
            fatalError()
        }
    }
    */
}
