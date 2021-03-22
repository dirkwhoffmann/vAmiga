// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

class Console {
 
    let controller: MyController
    
    var amiga: AmigaProxy { return controller.amiga }
    var window: NSWindow { return controller.window! }
    var contentView: NSView { return window.contentView! }
    
    let scrollView = NSTextView.scrollableTextView()
    let textView: NSTextView
    
    var isOpen = false
    
    //
    // Initializing
    //
    
    init(controller: MyController) {
        
        self.controller = controller
        textView = (scrollView.documentView as? NSTextView)!
        textView.backgroundColor = NSColor.init(r: 0x80, g: 0x80, b: 0x80, a: 0x80)
    }
    
    func open() {
            
        contentView.addSubview(scrollView)
        resize()
        
        textView.isEditable = false
        
        // textView.size
        for _ in 0...5 {
            let myString = "Swift Attributed String\n"
            let myAttribute = [
                NSAttributedString.Key.foregroundColor: NSColor.white,
                NSAttributedString.Key.font: NSFont.systemFont(ofSize: 20, weight: .medium)
            ]
            let myAttrString = NSAttributedString(string: myString, attributes: myAttribute)
            let myString2 = "Holla, die Waldfee\n"
            let myAttrString2 = NSAttributedString(string: myString2, attributes: myAttribute)

            textView.textStorage?.append(myAttrString)
            textView.textStorage?.append(myAttrString2)
        }
    }
    
    func close() {
        
        scrollView.removeFromSuperview()
    }
    
    func toggle() {
        
        scrollView.superview == nil ? open() : close()
    }
    
    func resize() {
        
        track()
        
        let size = controller.metal.frame.size
        let origin = controller.metal.frame.origin
        scrollView.setFrameSize(size as NSSize)
        scrollView.frame.origin = origin
    }
}
