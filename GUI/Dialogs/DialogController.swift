// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class DialogWindow: NSWindow {
    
    /* Fetch first responder status
     * This function is called in awakeFromNib() to ensure that we receive key
     * key events. In theory, this call should not be needed, but I had issues
     * in the past with not responding windows.
     */
    func respondToEvents() {
        
        track()
        DispatchQueue.main.async {
            self.makeFirstResponder(self)
        }
    }
    
    override func awakeFromNib() {
        
        track()
        // respondToEvents()
    }
    
    // Called when the user presses ESC or Cmd+.
    override func cancelOperation(_ sender: Any?) {
        
        let controller = delegate as? DialogController
        controller?.cancelAction(sender)
    }
}

class DialogController: NSWindowController {

    func sheetDidShow() { }

    func showSheet(completionHandler handler:(() -> Void)? = nil) {

        myWindow?.beginSheet(window!, completionHandler: { result in
            if result == NSApplication.ModalResponse.OK {
                
                self.cleanup()
                handler?()
            }
        })

        sheetDidShow()
        refresh()
    }
        
    func refresh() {
        
    }
    
    func cleanup() {
        // Don't delete this function. Calling cleanup in the sheet's completion
        // handler makes sure that ARC doesn't delete the reference too early.
    }
    
    func hideSheet() {
    
        if let win = window {
            win.orderOut(self)
            myWindow?.endSheet(win, returnCode: .cancel)
        }
    }
    
    // Default action method for OK
    @IBAction func okAction(_ sender: Any!) {
        
        hideSheet()
    }
    
    // Default action method for Cancel
    @IBAction func cancelAction(_ sender: Any!) {
        
        hideSheet()
    }
}
