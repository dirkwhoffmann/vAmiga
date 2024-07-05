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
    override func cancelOperation(_ sender: Any?) {
                      
        if let controller = delegate as? DialogController {
            controller.cancelAction(sender)
        }
    }
}

protocol DialogControllerDelegate: AnyObject {
    
    // Called before beginSheet() is called
    func dialogWillShow()

    // Called after beginSheet() has beed called
    func dialogDidShow()

    // Called after the completion handler has been executed
    func cleanup()
}

class DialogController: NSWindowController, DialogControllerDelegate {

    var parent: MyController!
    var emu: EmulatorProxy!

    // References to all open dialogs (to make ARC happy)
    static var active: [DialogController] = []
    
    // Remembers whether awakeFromNib has been called
    var awake = false

    // Indicates if this dialog is displayed as a sheet
    var sheet = false

    convenience init?(with controller: MyController, nibName: NSNib.Name) {
    
        self.init(windowNibName: nibName)
        
        parent = controller
        emu = parent.emu
    }

    func register() {
        
        DialogController.active.append(self)
        debug(.lifetime, "Register: \(DialogController.active)")
    }
    
    func unregister() {
        
        DialogController.active = DialogController.active.filter {$0 != self}
        debug(.lifetime, "Unregister: \(DialogController.active)")
    }

    override func awakeFromNib() {
    
        awake = true
        window?.delegate = self
        dialogWillShow()
    }
    
    func dialogWillShow() {

        debug(.lifetime)
    }
    
    func dialogDidShow() {

        debug(.lifetime)
    }
    
    func cleanup() {

        debug(.lifetime)
    }
    
    func showAsWindow() {

        sheet = false
        register()

        if awake { dialogWillShow() }
        showWindow(self)
        dialogDidShow()
    }

    func showAsSheet(completionHandler handler:(() -> Void)? = nil) {

        sheet = true
        register()

        if awake { dialogWillShow() }
        parent.window?.beginSheet(window!, completionHandler: { result in handler?() })
        dialogDidShow()
    }

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

    @IBAction func okAction(_ sender: Any!) {
        
        hide()
    }
    
    @IBAction func cancelAction(_ sender: Any!) {
        
        hide()
    }
}

extension DialogController: NSWindowDelegate {

    func windowWillClose(_ notification: Notification) {

        unregister()
    }
}
