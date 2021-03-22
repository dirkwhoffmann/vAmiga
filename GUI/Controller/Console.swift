// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

class ConsoleTextView: NSTextView {
    
    override func keyDown(with event: NSEvent) {
        
        track()
    }
}

class Console {
 
    let controller: MyController
    
    var amiga: AmigaProxy { return controller.amiga }
    var window: NSWindow { return controller.window! }
    var contentView: NSView { return window.contentView! }
    
    let scrollView = NSTextView.scrollableTextView()
    var textView: NSTextView
    
    var isOpen = false
    
    //
    // Initializing
    //
    
    init(controller: MyController) {
        
        /*
        let FLT_MAX = CGFloat.greatestFiniteMagnitude
        
        self.controller = controller
        
        scrollView = NSScrollView.init() // frame: contentView.frame)
        let contentSize = scrollView.contentSize

        scrollView.borderType = .noBorder
        scrollView.hasVerticalScroller = true
        scrollView.hasHorizontalScroller = false
        scrollView.autoresizingMask = [ .width, .height ]

        let rect = NSRect.init(x: 0, y: 0, width: contentSize.width, height: contentSize.height)
        textView = NSTextView.init(frame: rect)
        
        textView.minSize = NSSize.init(width: 0.0, height: contentSize.height)
        textView.maxSize = NSSize.init(width: FLT_MAX, height: FLT_MAX)
        textView.isVerticallyResizable = true
        textView.isHorizontallyResizable = false
        textView.autoresizingMask = .width

        let containerSize = NSSize.init(width: contentSize.width, height: FLT_MAX)
        textView.textContainer?.containerSize = containerSize
        textView.textContainer?.widthTracksTextView = true
        scrollView.documentView = textView

        textView.backgroundColor = NSColor.init(r: 0x80, g: 0x80, b: 0x80, a: 0x80)
        */
        
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
    
    func keyDown(with event: NSEvent) {
        
        track()

        if let characters = event.characters {
            
            let attr = [
                NSAttributedString.Key.foregroundColor: NSColor.white,
                NSAttributedString.Key.font: NSFont.systemFont(ofSize: 20, weight: .medium)
            ]
            let string = NSAttributedString(string: characters, attributes: attr)
            
            textView.textStorage?.append(string)
        }
    }

    func keyUp(with event: NSEvent) {
        
        track()
    }

}
