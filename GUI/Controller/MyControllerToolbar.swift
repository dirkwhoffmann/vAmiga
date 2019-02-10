//
// This file is part of VirtualC64 - A cycle accurate Commodore 64 emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
//

import Foundation

extension MyController {
 
    struct InputDevice {
        static let none = -1
        static let keyset1 = 0
        static let keyset2 = 1
        static let mouse = 2
        static let joystick1 = 3
        static let joystick2 = 4
    }
    
    // NSDrawerState is deprected an not available natively in Swift
    struct NSDrawerState {
        static let closed = 0
        static let opening = 1
        static let open = 2
        static let closing = 3
    }
    
    func validateToolbarItems() {
        
        let button = pauseTbItem.view as! NSButton
        if amiga.isRunning() {
            button.image = NSImage.init(named: "pauseTemplate")
            pauseTbItem.label = "Pause"
        } else {
            button.image = NSImage.init(named: "continueTemplate")
            pauseTbItem.label = "Run"
        }
        
        validateJoystickToolbarItems()
    }
    
    func validateJoystickToolbarItem(_ popup: NSPopUpButton, selectedSlot: Int, port: AmigaControlPortProxy!) {
        
        let menu =  popup.menu
        let item3 = menu?.item(withTag: InputDevice.joystick1)
        let item4 = menu?.item(withTag: InputDevice.joystick2)
        
        // USB joysticks
        item3?.title = gamePadManager.gamePads[3]?.name ?? "USB Device 1"
        item4?.title = gamePadManager.gamePads[4]?.name ?? "USB Device 2"
        item3?.isEnabled = !gamePadManager.slotIsEmpty(InputDevice.joystick1)
        item4?.isEnabled = !gamePadManager.slotIsEmpty(InputDevice.joystick2)
        
        // Mark game pad connected to port
        popup.selectItem(withTag: selectedSlot)
    }
    
    func validateJoystickToolbarItems() {
    
        validateJoystickToolbarItem(controlPort1, selectedSlot: inputDevice1, port: amiga.controlPort1)
        validateJoystickToolbarItem(controlPort2, selectedSlot: inputDevice2, port: amiga.controlPort2)
    }
    
    @IBAction func port1Action(_ sender: NSPopUpButton) {
        
        setPort1(sender.selectedTag())
    }
 
    func setPort1(_ value: Int) {
        
        // Remember selection
        inputDevice1 = value
        
        // Avoid double mappings
        inputDevice2 = (inputDevice1 == inputDevice2) ? InputDevice.none : inputDevice2
        
        // Connect or disconnect mouse
        amiga.controlPort1.connectMouse(inputDevice1 == InputDevice.mouse);
        amiga.controlPort2.connectMouse(inputDevice2 == InputDevice.mouse);
        
        UserDefaults.standard.set(inputDevice1, forKey: Keys.inputDevice1)
        UserDefaults.standard.set(inputDevice2, forKey: Keys.inputDevice2)
        validateJoystickToolbarItems();
    }
    
    @IBAction func port2Action(_ sender: NSPopUpButton) {
        
        setPort2(sender.selectedTag())
    }
    
    func setPort2(_ value: Int) {
        
        // Remember selection
        inputDevice2 = value
        
        // Avoid double mappings
        inputDevice1 = (inputDevice1 == inputDevice2) ? InputDevice.none : inputDevice1
        
        // Connect or disconnect mouse
        amiga.controlPort1.connectMouse(inputDevice1 == InputDevice.mouse);
        amiga.controlPort2.connectMouse(inputDevice2 == InputDevice.mouse);
        
        UserDefaults.standard.set(inputDevice1, forKey: Keys.inputDevice1)
        UserDefaults.standard.set(inputDevice2, forKey: Keys.inputDevice2)
        validateJoystickToolbarItems()
    }
        
    public func debugOpenAction(_ sender: Any!) {
        
        let state = debugger.state
        if state == NSDrawerState.closed || state == NSDrawerState.closing {
            debugger.open()
        }
    }
    
    public func debugCloseAction(_ sender: Any!) {
        
        let state = debugger.state
        if state == NSDrawerState.open || state == NSDrawerState.opening {
            debugger.close()
        }
    }
    
    @IBAction func preferencesAction(_ sender: Any!) {
        
        openPreferences()
    }
    
    @IBAction func snapshotAction(_ sender: NSSegmentedControl) {
        
        switch(sender.selectedSegment) {
        
        case 0: // Rewind

            track("Rewind")
            if (amiga.restoreLatestAutoSnapshot()) {
                metal.snapToFront()
            }
        
        case 1: // Take

            track("Snap")
            amiga.takeUserSnapshot()
            
        case 2: // Restore
            
            track("Restore")
            if (amiga.restoreLatestUserSnapshot()) {
                metal.snapToFront()
            } else {
                NSSound.beep()
            }

        case 3: // Browse
            
            track("Browse")
            let nibName = NSNib.Name("SnapshotDialog")
            let controller = SnapshotDialog.init(windowNibName: nibName)
            controller.showSheet()

        default:
            assert(false)
        }
    }
    
    @IBAction func keyboardAction(_ sender: Any!) {
        
        // Open the virtual keyboard as a sheet
        virtualKeyboardSheet = VirtualKeyboardController.make()
        virtualKeyboardSheet?.showSheet()
    }

    @IBAction func snapshotsAction(_ sender: Any!) {
        
        let nibName = NSNib.Name("SnapshotDialog")
        let controller = SnapshotDialog.init(windowNibName: nibName)
        controller.showSheet()
    }

    @IBAction func restoreLatestAutoSnapshotAction(_ sender: Any!) {
        
        if (amiga.restoreLatestAutoSnapshot()) {
            amiga.deleteAutoSnapshot(0)
            metal.snapToFront()
        }
    }

    @IBAction func restoreLatestUserSnapshotAction(_ sender: Any!) {
        
        if (amiga.restoreLatestUserSnapshot()) {
            metal.snapToFront()
        }
    }
    
    @IBAction func printDocument(_ sender: Any!) {
    
        let window = mydocument.windowForSheet!

        // Printing properties
        let printInfo = mydocument.printInfo
        printInfo.horizontalPagination = .fit
        printInfo.isHorizontallyCentered = true
        printInfo.verticalPagination = .fit
        printInfo.isVerticallyCentered = true
        printInfo.orientation = .landscape
        printInfo.leftMargin = 32.0
        printInfo.rightMargin = 32.0
        printInfo.topMargin = 32.0
        printInfo.bottomMargin = 32.0

        // Image view
        let paperSize = printInfo.paperSize
        let image = metal.screenshot()
        let printRect = NSMakeRect(0.0, 0.0, paperSize.width, paperSize.height)
        let imageView = NSImageView.init(frame: printRect)
        imageView.image = image
        imageView.imageScaling = .scaleAxesIndependently
    
        // Print image
        let printOperation = NSPrintOperation.init(view: imageView, printInfo: printInfo)
        printOperation.runModal(for: window,
                                delegate: nil,
                                didRun: nil,
                                contextInfo: nil)
    }
    
}
