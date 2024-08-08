// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* Emulated keyboard model. This variable controls the overall appearance of
 * the virtual keyboard. The keyboard can be drawn in a narrow style (A1000
 * keyboard) or a wide style (A500, A2000 keyboard).
 */
enum KBStyle: Int, Codable {

    case narrow
    case wide
}

/* Language of the emulated keyboard. The keyboard layout defines the number of
 * keys on the keyboard, the visua appearance of their key caps and their
 * physical shape.
 */
enum KBLayout: Int, Codable {

    case generic // Used as a fallback if no matching layout is found
    case us
    case german
    case italian
}

class VirtualKeyboardWindow: DialogWindow {
 
}

class VirtualKeyboardController: DialogController {

    // Array holding a reference to the view of each key
    var keyView = Array(repeating: nil as NSButton?, count: 128)

    // Image cache
    var keyImage = Array(repeating: nil as NSImage?, count: 128)

    // Image cache for keys that are currently pressed
    var pressedKeyImage = Array(repeating: nil as NSImage?, count: 128)

    /* Indicates if the window should close when a key is pressed. If the
     * virtual keyboard is opened as a sheet, this variable is set to true. If
     * it is opened as a seperate window, it is set to false.
     */
    var autoClose = true

    static func make(parent: MyController) -> VirtualKeyboardController? {

        let style = kbStyle(parent)
        let layout = kbLayout(parent)

        debug(.lifetime, "Virtual keyboard (style: \(style) layout: \(layout))")

        var xibName = ""
        let ansi = (layout == .us)

        if style == .narrow {
            xibName = ansi ? "A1000ANSI" : "A1000ISO"
        } else {
            xibName = ansi ? "A500ANSI" : "A500ISO"
        }

        let keyboard = VirtualKeyboardController(windowNibName: xibName)
        keyboard.parent = parent

        return keyboard
    }

    func showSheet() {

        autoClose = true
        super.showAsSheet()
    }

    func showWindow() {
        
        autoClose = false
        super.showWindow(self)
    }
    
    override func windowDidLoad() {
                
        // Setup key references
        for tag in 0 ... 127 {
            keyView[tag] = window!.contentView!.viewWithTag(tag) as? NSButton
        }

        // Compute key cap images
        updateImageCache()
        refresh()
    }
    
    override func dialogDidShow() {

        super.dialogDidShow()
        refresh()
    }
        
    func windowDidBecomeMain(_ notification: Notification) {
        
        refresh()
    }
    
    func refreshIfVisible() {
        
        if window?.isVisible == true {
            refresh()
        }
    }
    
    fileprivate func refresh() {
                       
        guard let keyboard = emu.keyboard else { return }
        
        for keycode in 0 ... 127 {
            
            if keyboard.isPressed(keycode) {
                keyView[keycode]?.image = pressedKeyImage[keycode]
            } else {
                keyView[keycode]?.image = keyImage[keycode]
            }
        }
    }
    
    func updateImageCache() {

        let style = VirtualKeyboardController.kbStyle(parent)
        let layout = VirtualKeyboardController.kbLayout(parent)

        for keycode in 0 ... 127 {
            let key = AmigaKey(keyCode: keycode)
            if let image = key.image(style: style, layout: layout) {
                keyImage[keycode] = image
                pressedKeyImage[keycode] = image.copy() as? NSImage
                pressedKeyImage[keycode]?.pressed()
            }
        }
    }
    
    func pressKey(keyCode: Int) {

        emu.keyboard.press(keyCode)
        refresh()
        
        DispatchQueue.main.async {
            
            usleep(useconds_t(100000))
            self.emu.keyboard.releaseAll()
            self.refresh()
        }
        
        if autoClose {
            cancelAction(self)
        }
    }
    
    func holdKey(keyCode: Int) {
        
        guard let keyboard = emu.keyboard else { return }
        
        keyboard.toggle(keyCode)
        refresh()
    }
        
    @IBAction func pressVirtualKey(_ sender: NSButton!) {
        
        // Not used at the moment
    }
    
    override func mouseDown(with event: NSEvent) {
                
        // If opened as a sheet, close if the user clicked inside unsued area
        if autoClose { cancelAction(self) }
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

    static func kbStyle(_ parent: MyController) -> KBStyle {
        
        // Determine if an A1000 is emulated
        let a1000 = parent.emu.mem.info.hasBootRom

        // Use a narrow keyboard for the A1000 and a wide keyboard otherwise
        return a1000 ? .narrow : .wide
    }

    static func kbLayout(_ parent: MyController) -> KBLayout {

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
