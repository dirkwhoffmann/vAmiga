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
    func sheetWillShow()

    // Called after beginSheet() has beed called
    func sheetDidShow()

    // Called after the completion handler has been executed
    func cleanup()
}

class DialogController: NSWindowController, DialogControllerDelegate {

    var parent: MyController!
    var amiga: AmigaProxy!

    // References to all open dialogs (to make ARC happy)
    static var active: [DialogController] = []
    
    // Remembers whether awakeFromNib has been called
    var awake = false

    // Indicates if this dialog is displayed as a sheet
    var sheet = false

    convenience init?(with controller: MyController, nibName: NSNib.Name) {
    
        self.init(windowNibName: nibName)
        
        parent = controller
        amiga = parent.amiga
    }

    func register() {
        
        DialogController.active.append(self)
        debug(.lifetime, "Register: \(DialogController.active)")
    }
    
    func unregister() {
        
        DialogController.active = DialogController.active.filter {$0 != self}
        debug(.lifetime, "Unregister: \(DialogController.active)")
    }
    
    override func windowWillLoad() {
    }
    
    override func windowDidLoad() {
    }
    
    override func awakeFromNib() {
    
        awake = true
        window?.delegate = self
        sheetWillShow()
    }
    
    func sheetWillShow() {

        debug(.lifetime)
    }
    
    func sheetDidShow() {

        debug(.lifetime)
    }
    
    func cleanup() {

        debug(.lifetime)
    }
    
    func showAsWindow(completionHandler handler:(() -> Void)? = nil) {

        sheet = false
        register()
        if awake { sheetWillShow() }
        
        showWindow(self)
        sheetDidShow()
    }

    func showAsSheet(completionHandler handler:(() -> Void)? = nil) {

        sheet = true
        register()

        if awake { sheetWillShow() }
        /*
        parent.window?.beginSheet(window!, completionHandler: { result in

            handler?()
            self.cleanup()
        })
        */
        parent.window?.beginSheet(window!)
        sheetDidShow()
    }

    func hideSheet() {

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
        
        hideSheet()
    }
    
    @IBAction func cancelAction(_ sender: Any!) {
        
        hideSheet()
    }
}

extension DialogController: NSWindowDelegate {

    func windowWillClose(_ notification: Notification) {

        unregister()
    }
}
