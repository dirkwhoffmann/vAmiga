// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class DialogWindow: NSWindow {

    // Delegation method for ESC and Cmd+
    @MainActor
    override func cancelOperation(_ sender: Any?) {
                      
        if let controller = delegate as? DialogController {
            controller.cancelAction(sender)
        }
    }
}

protocol DialogControllerDelegate: AnyObject {
    
    // Called before showWindow() or beginSheet() is called
    @MainActor func dialogWillShow()

    // Called after showWindow() or beginSheet() has beed called
    @MainActor func dialogDidShow()

    // Called after the completion handler has been executed
    @MainActor func cleanup()
}

/* Base class for all auxiliary windows. The class extends NSWindowController
 * by a reference to the controller of the connected emulator window (parent)
 * and a reference to the parents proxy object. It also provides some wrappers
 * around showing and hiding the window.
 */
class DialogController: NSWindowController, DialogControllerDelegate {

    var parent: MyController!
    var emu: EmulatorProxy! { return parent.emu }
    var amiga: AmigaProxy { return emu.amiga }

    // References to all open dialogs (to make ARC happy)
    static var active: [DialogController] = []
    
    // Lock that is kept during the lifetime of the dialog
    var lock = NSLock()

    // Indicates if this dialog is displayed as a sheet
    var sheet = false

    @MainActor
    convenience init?(with controller: MyController, nibName: NSNib.Name) {
    
        self.init(windowNibName: nibName)
        
        lock.lock()
        parent = controller
    }

    @MainActor
    func register() {
        
        DialogController.active.append(self)
        debug(.lifetime, "Register: \(DialogController.active)")
    }
    
    @MainActor
    func unregister() {
        
        DialogController.active = DialogController.active.filter {$0 != self}
        debug(.lifetime, "Unregister: \(DialogController.active)")
    }

    @MainActor
    override func windowDidLoad() {
        
        debug(.lifetime)

        super.windowDidLoad()
        self.window?.delegate = self
    }
    
    @MainActor
    func dialogWillShow() {

        debug(.lifetime)
    }
    
    @MainActor
    func dialogDidShow() {

        debug(.lifetime)
    }
    
    @MainActor
    func cleanup() {

        debug(.lifetime)
    }
    
    @MainActor
    func showAsWindow() {

        debug(.lifetime)
        
        sheet = false
        register()
        loadWindow()
        dialogWillShow()
        showWindow(self)
        dialogDidShow()
    }

    @MainActor
    func showAsSheet(completionHandler handler:(() -> Void)? = nil) {

        debug(.lifetime)
        
        sheet = true
        register()
        loadWindow()
        dialogWillShow()
        parent.window?.beginSheet(window!, completionHandler: { result in handler?() })
        dialogDidShow()
    }

    @MainActor
    func hide() {

        cleanup()

        if sheet {
            if let win = window {
                parent.window?.endSheet(win, returnCode: .cancel)
            }
        } else {
            close()
        }

        unregister()
    }

    @MainActor
    func join() {

        debug(.shutdown, "Wait until the window is closed...")

        lock.lock()
        lock.unlock()
    }
    
    @IBAction
    func okAction(_ sender: Any!) {
        
        hide()
    }
    @IBAction
    func cancelAction(_ sender: Any!) {
        
        hide()
    }
}

extension DialogController: NSWindowDelegate {

    @MainActor
    func windowDidBecomeKey(_ notification: Notification) {

        debug(.lifetime)
    }

    @MainActor
    func windowWillClose(_ notification: Notification) {

        unregister()
        lock.unlock()
    }
}
