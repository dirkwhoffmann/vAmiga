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
        
        let controller = delegate as! VirtualKeyboardController
        controller.stickyKeys = event.modifierFlags.contains(.shift)
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

    /*  Indicates if we're in "sticky" mode
     *  In sicky mode, only keyDown events are triggered and the keyUp events
     *  are delayed until sticky mode is disabled. It is used key combinations
     *  such as Shift + Amiga + <some other key>
     */
    var stickyKeys = false {
        didSet {
            if !stickyKeys {
                amigaProxy?.keyboard.releaseAllKeys()
                if (autoClose) {
                    cancelAction(self)
                }
            }
            refresh()
        }
    }
    
    /* Indicates if the window should close when a key is pressed.
     * If the virtual keyboard is opened as a sheet, this variable is set to
     * true. If it is opened as a seperate window, it is set to false.
     */
    var autoClose = true
    
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
                pressedKeyImage[keycode] = image.copy() as? NSImage
                pressedKeyImage[keycode]?.darken()
            }
        }
    }
    
    @IBAction func pressVirtualKey(_ sender: NSButton!) {
        
        guard let keyboard = amigaProxy?.keyboard else { return }
        
        let amigaKeyCode = sender.tag
        track("Pressing Amiga Key \(amigaKeyCode)")
     
        keyboard.pressKey(amigaKeyCode)
        
        // Schedule automatic key release if we're not in sticky mode
        if !stickyKeys {
            DispatchQueue.global().async {
                
                usleep(useconds_t(20000))
                amigaProxy?.keyboard.releaseAllKeys()
            }
            
            if (autoClose) {
                cancelAction(self)
            }
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
