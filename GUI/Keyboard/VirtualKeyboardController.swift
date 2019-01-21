//
// This file is part of VirtualC64 - A cycle accurate Commodore 64 emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
//

import Foundation

class VirtualKeyboardWindow : NSWindow {
    
    func respondToEvents() {
        DispatchQueue.main.async {
            self.makeFirstResponder(self)
        }
    }
    
    override func awakeFromNib() {
        
        respondToEvents()
    }
    
    override func flagsChanged(with event: NSEvent) {
        
        track()
        
        // Let the emulator handle the event first
        myController?.metalScreen.flagsChanged(with: event)
        
        // Update images
        let controller = delegate as! VirtualKeyboardController
        controller.refresh()
    }
}

class VirtualKeyboardController : UserDialogController, NSWindowDelegate
{
    // Array holding a reference to the view of each key
    var keyView = Array(repeating: nil as NSButton?, count: 128)

    // Image cache
    var keyImage = Array(repeating: nil as NSImage?, count: 128)

    // Image cache for keys that are currently pressed
    var pressedKeyImage = Array(repeating: nil as NSImage?, count: 128)

    /* Indicates if the window should close when a key is pressed.
     * If the virtual keyboard is opened as a sheet, this variable is set to
     * true. If it is opened as a seperate window, it is set to false.
     */
    var autoClose = true
    
    static func make() -> VirtualKeyboardController? {

        guard let config = myController?.amiga.config() else { return nil }

        var xibName = ""
        let language = Language.german
        let ansi = language == .us
        
        if config.model == A1000 {
            xibName = ansi ? "A1000ANSI" : "A1000ISO"
        } else {
            xibName = ansi ? "A500ANSI" : "A500ISO"
        }
      
        xibName = "A500ISO"
        return VirtualKeyboardController.init(windowNibName: xibName)
    }
    
    func showWindow(withParent controller: MyController) {
        
        track()
        autoClose = false
        showWindow(self)
    }
    
    override func windowDidLoad() {
        
        track()
        
        // Setup key references
        for tag in 0 ... 127 {
            keyView[tag] = window!.contentView!.viewWithTag(tag) as? NSButton
        }

        updateImageCache()
    }
    
    func windowWillClose(_ notification: Notification) {
    
        track()
        // releaseSpecialKeys()
    }
    
    func windowDidBecomeMain(_ notification: Notification) {
        
        track()
        refresh()
    }
    
    override func refresh() {
        
        track()
        
        guard let keyboard = amigaProxy?.keyboard else { return }
        
        for keycode in 0 ... 127 {
            
            if keyboard.keyIsPressed(keycode) {
                keyView[keycode]?.image = pressedKeyImage[keycode]
            } else {
                keyView[keycode]?.image = keyImage[keycode]
            }
        }
    }
    
    func updateImageCache() {
        
        for keycode in 0 ... 127 {
            let key = AmigaKey.init(keyCode: keycode)
            if let image = key.image(model: A500, country: .italian) {
                keyImage[keycode] = image
                // track("\(key)")
                pressedKeyImage[keycode] = image.copy() as? NSImage
                // track("darken")
                pressedKeyImage[keycode]?.darken()
            }
        }
    }
    
    @IBAction func pressVirtualKey(_ sender: NSButton!) {
        
        guard let keyboard = amigaProxy?.keyboard else { return }
        
        let amigaKeyCode = sender.tag
        track("Pressing Amiga Key \(amigaKeyCode)")
     
        keyboard.pressKey(amigaKeyCode)
        
        // Schedule automatic key release
        DispatchQueue.global().async {
            
            usleep(useconds_t(20000))
            amigaProxy?.keyboard.releaseAllKeys()
        }
        
        if (autoClose) {
            cancelAction(self)
        }
        
        refresh()
    }
    
    override func mouseDown(with event: NSEvent) {
        
        // Close window if the user clicked inside the unused area
        track()
        if (autoClose) {
            cancelAction(self)
        }
    }
}
