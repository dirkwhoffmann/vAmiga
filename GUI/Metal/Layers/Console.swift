// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation
import Carbon.HIToolbox

class Console: Layer {
 
    let controller: MyController
    
    // var amiga: AmigaProxy { return controller.amiga }
    var window: NSWindow { return controller.window! }
    var contentView: NSView { return window.contentView! }
    
    let scrollView = NSTextView.scrollableTextView()
    var textView: NSTextView
    
    // var isOpen = false
    var isDirty = false
        
    //
    // Initializing
    //
    
    override init(view: MTKView, device: MTLDevice, renderer: Renderer) {
        
        track()

        controller = renderer.parent
        
        textView = (scrollView.documentView as? NSTextView)!
        textView.isEditable = false
        textView.backgroundColor = NSColor.init(r: 0x80, g: 0x80, b: 0x80, a: 0x80)
        
        super.init(view: view, device: device, renderer: renderer)
        
        resize()
        isDirty = true
    }

    override func open() {
    
        super.open()
        resize()
    }

    override func update(frames: Int64) {
        
        super.update(frames: frames)

        if isDirty {
            
            let cpos = amiga.retroShell.cposRel
            
            let attr = [
                NSAttributedString.Key.foregroundColor: NSColor.white,
                NSAttributedString.Key.font: NSFont.monospacedSystemFont(ofSize: 14, weight: .medium)
            ]
            
            if let text = amiga.retroShell.getText() {
                
                let string = NSMutableAttributedString(string: text, attributes: attr)
                string.addAttribute(.backgroundColor,
                                    value: NSColor.blue,
                                    range: NSRange(location: string.length - 1 - cpos, length: 1))
                textView.textStorage?.setAttributedString(string)

            } else {
                track("ERROR: text is NULL\n")
            }
            
            isDirty = false
        }
    }
        
    override func alphaDidChange() {
                
        let a1 = Int(alpha.current * 0xFF)
        let a2 = Int(alpha.current * 0.8 * 0xFF)
        textView.textColor = NSColor.init(r: 0xFF, g: 0xFF, b: 0xFF, a: a1)
        textView.backgroundColor = NSColor.init(r: 0x80, g: 0x80, b: 0x80, a: a2)
        
        if alpha.current > 0 && scrollView.superview == nil {
            
            track("Adding console sub view")
            contentView.addSubview(scrollView)
        }
        
        if alpha.current == 0 && scrollView.superview != nil {
        
            track("Removing console sub view")
            scrollView.removeFromSuperview()
        }
    }
    
    func resize() {
        
        track()
        
        let size = controller.metal.frame.size
        let origin = controller.metal.frame.origin
        let newSize = NSSize.init(width: size.width, height: size.height)
        
        scrollView.setFrameSize(newSize)
        scrollView.frame.origin = CGPoint.init(x: origin.x, y: origin.y)
    }
    
    func keyDown(with event: NSEvent) {
        
        let macKey = MacKey.init(event: event)
        
        track()

        switch macKey.keyCode {
        
        case kVK_UpArrow: amiga.retroShell.pressUp()
        case kVK_DownArrow: amiga.retroShell.pressDown()
        case kVK_LeftArrow: amiga.retroShell.pressLeft()
        case kVK_RightArrow: amiga.retroShell.pressRight()
        case kVK_Home: amiga.retroShell.pressHome()
        case kVK_End: amiga.retroShell.pressEnd()
        case kVK_Return: amiga.retroShell.pressReturn()
        case kVK_Tab: amiga.retroShell.pressTab()

        default:
            
            if let c = event.characters?.utf8CString.first {
                amiga.retroShell.pressKey(c)
            }
        }
        
        isDirty = true
    }
    
    func keyUp(with event: NSEvent) {
        
    }

}
