// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* Emulated keyboard model
 * The keyboard style defines the overall appearance of the virtual keyboard.
 * The user can choose between a narrow style (as uses for the Amiga 1000) and
 * a wide style (as used for the Amiga 500 and Amiga 2000).
 */
enum KBStyle: Int, Codable {

    case narrow
    case wide
}

/* Language of the emulated keyboard
 * The keyboard layout defines the number of keys on the keyboard, the visual
 * appearance of their key caps and their physical shape.
 */
enum KBLayout: Int, Codable {

    case generic // Used as a fallback if no matching layout is found
    case us
    case german
    case italian
}

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

        track("Virtual keyboard (style: \(kbStyle) layout: \(kbLayout))")

        var xibName = ""
        let ansi = (kbLayout == .us)

        if kbStyle == .narrow {
            xibName = ansi ? "A1000ANSI" : "A1000ISO"
        } else {
            xibName = ansi ? "A500ANSI" : "A500ISO"
        }

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

        let style = VirtualKeyboardController.kbStyle
        let layout = VirtualKeyboardController.kbLayout

        for keycode in 0 ... 127 {
            let key = AmigaKey.init(keyCode: keycode)
            if let image = key.image(style: style, layout: layout) {
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

extension VirtualKeyboardController {

    static var kbStyle: KBStyle {

        // Determine if an A1000 is emulated
        let a1000 = amigaProxy?.mem.hasBootRom() ?? false

        // Use a narrow keyboard for the A1000 and a wide keyboard otherwise
        return a1000 ? .narrow : .wide
    }

    static var kbLayout: KBLayout {

         // Get the first two characters of the ISO-639-1 language code
         let lang = Locale.preferredLanguages[0].prefix(2)

         // Translate language into a keyboard layout identifier
         switch lang {

         case "de": return .german
         case "it": return .italian
         default: return .us
         }
     }
}
