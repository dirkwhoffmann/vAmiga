//
// This file is part of VirtualC64 - A cycle accurate Commodore 64 emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
//

class VirtualKeyboardWindow: DialogWindow {
 
}

class VirtualKeyboardController: DialogController, NSWindowDelegate {

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

        let model  = config.keyboard.model
        let lang   = config.keyboard.language
        let layout = Layout(rawValue: lang.rawValue) ?? .us
        let ansi   = (layout == .us)

        var xibName = ""
        
        if model == KB_A1000 {
            xibName = ansi ? "A1000ANSI" : "A1000ISO"
        } else {
            xibName = ansi ? "A500ANSI" : "A500ISO"
        }

        track("xibName = \(xibName) layout = \(layout)")

        return VirtualKeyboardController.init(windowNibName: xibName)
    }
    
    func showWindow() {
        
        autoClose = false
        showWindow(self)
    }
    
    override func windowDidLoad() {
        
        track()
        
        // Setup key references
        for tag in 0 ... 127 {
            keyView[tag] = window!.contentView!.viewWithTag(tag) as? NSButton
        }

        // Compute key cap images
        updateImageCache()
    }
    
    func windowWillClose(_ notification: Notification) {
    
    }
    
    func windowDidBecomeMain(_ notification: Notification) {
        
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

        guard let config = amigaProxy?.config() else { return }

        let lang = config.keyboard.language
        let layout = Layout(rawValue: lang.rawValue) ?? .us

        track("layout = \(layout)")
        
        for keycode in 0 ... 127 {
            let key = AmigaKey.init(keyCode: keycode)
            if let image = key.image(model: config.keyboard.model, layout: layout) {
                keyImage[keycode] = image
                pressedKeyImage[keycode] = image.copy() as? NSImage
                pressedKeyImage[keycode]?.pressed()
            }
        }
    }
    
    func pressKey(keyCode: Int) {
        
        guard let keyboard = amigaProxy?.keyboard else { return }

        keyboard.pressKey(keyCode)
        refresh()
        
        DispatchQueue.main.async {
            
            usleep(useconds_t(100000))
            amigaProxy?.keyboard.releaseAllKeys()
            self.refresh()
        }
        
        if autoClose {
            cancelAction(self)
        }
    }
    
    func holdKey(keyCode: Int) {
        
        guard let keyboard = amigaProxy?.keyboard else { return }
        
        keyboard.pressKey(keyCode)
        refresh()
    }
        
    @IBAction func pressVirtualKey(_ sender: NSButton!) {
        
        // not used at the moment
    }
    
    override func mouseDown(with event: NSEvent) {
                
        // Close window if the user clicked inside the unused area
        track()
        if autoClose {
            cancelAction(self)
        }
    }
}

/* Subclass of NSButton for the keys in the virtual keyboard.
 */
class Keycap: NSButton {
    
    override func mouseDown(with event: NSEvent) {
        
        if let controller = window?.delegate as? VirtualKeyboardController {
            
            controller.pressKey(keyCode: self.tag)
        }
    }
    
    override func rightMouseDown(with event: NSEvent) {
    
        if let controller = window?.delegate as? VirtualKeyboardController {
            
            controller.holdKey(keyCode: self.tag)
        }
    }
}
