// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Cocoa

class MyToolbarButton: NSButton {

    init(image: NSImage, target: Any?, action: Selector?) {

        super.init(frame: .zero)

        self.image = image
        self.target = target as AnyObject
        self.action = action

        self.isBordered = true
        self.bezelStyle = .smallSquare
        self.imagePosition = .imageOnly
        self.title = ""
        self.translatesAutoresizingMaskIntoConstraints = false
        self.wantsLayer = true
        self.layer?.backgroundColor = NSColor.clear.cgColor
    }

    required init?(coder: NSCoder) {

        super.init(coder: coder)
    }

    override func draw(_ dirtyRect: NSRect) {

        // Instead of calling super.draw(_:), manually draw the image
        if let image = self.image {

            let imageRect = NSRect(
                x: (bounds.width - image.size.width) / 2,
                y: (bounds.height - image.size.height) / 2,
                width: image.size.width,
                height: image.size.height
            )
            image.draw(in: imageRect)
        }
    }
}

/* This class represents a single “grouped” toolbar item, similar to a
 * segmented control but with independent views.
 */
class MyToolbarItemGroup: NSToolbarItem {

    let debug = false

    /// Maximum of 3 views (mimicking a segmented control)
    var buttons: [NSButton] = []

    /// Container view for Auto Layout
    private let container: NSView = {

        let view = NSView()
        view.translatesAutoresizingMaskIntoConstraints = false
        return view
    }()

    init(identifier: NSToolbarItem.Identifier,
         images: [NSImage],
         actions: [Selector],
         target: AnyObject? = nil,
         label: String,
         paletteLabel: String? = nil) {

        super.init(itemIdentifier: identifier)

        self.view = container
        self.label = label
        self.paletteLabel = paletteLabel ?? label

        let count = min(images.count, 3)
        for i in 0..<count {

            let button = MyToolbarButton(image: images[i], target: target, action: actions[i])

            container.addSubview(button)
            buttons.append(button)
        }

        if debug {

            container.wantsLayer = true
            container.layer?.backgroundColor = NSColor.systemYellow.withAlphaComponent(0.3).cgColor

            for button in buttons {

                button.layer?.backgroundColor = NSColor.systemGreen.withAlphaComponent(0.3).cgColor
            }
        }

        setupConstraints()

    }

    convenience init(identifier: NSToolbarItem.Identifier,
                     image: NSImage, action: Selector, target: AnyObject? = nil,
                     label: String, paletteLabel: String? = nil) {

        self.init(identifier: identifier, images: [image], actions: [action],
             target: target, label: label, paletteLabel: paletteLabel)
    }

    private func setupConstraints() {

        guard !buttons.isEmpty else { return }

        let horizontalPadding: CGFloat = 6
        let spacing: CGFloat = 4

        container.heightAnchor.constraint(equalToConstant: 42).isActive = true

        for (index, button) in buttons.enumerated() {

            // Height
            button.centerYAnchor.constraint(equalTo: container.centerYAnchor).isActive = true

            // Width
            if index == 0 {
                button.leadingAnchor.constraint(equalTo: container.leadingAnchor, constant: horizontalPadding).isActive = true
            } else {
                button.leadingAnchor.constraint(equalTo: buttons[index - 1].trailingAnchor, constant: spacing).isActive = true
            }

            if index == buttons.count - 1 {
                button.trailingAnchor.constraint(equalTo: container.trailingAnchor, constant: -horizontalPadding).isActive = true
            }
        }
    }

    /// Access buttons if needed (e.g., enable/disable)
    func button(at index: Int) -> NSButton? {

        guard index >= 0 && index < buttons.count else { return nil }
        return buttons[index]
    }
}

class ToolbarPopupItem: MyToolbarItemGroup {

    var items: [(NSImage, String)] = []

    convenience init(identifier: NSToolbarItem.Identifier,
                     menuItems: [(NSImage, String)],
                     image: NSImage, action: Selector, target: AnyObject?,
                     label: String, paletteLabel: String? = nil) {

        self.init(identifier: identifier,
                  image: image, action: #selector(showMenu),
                  label: label, paletteLabel: paletteLabel)

        self.items = menuItems
        self.buttons[0].target = self
        // self.buttons[0].action = #selector(showMenu)
    }

    @objc func menuAction(_ sender: NSMenuItem) {
        print("Selected: \(sender.tag)")
    }

    @objc func showMenu() {

        print("showMenu:")

        let menu = NSMenu()
        for (index, (image, title)) in items.enumerated() {

            let item = menu.addItem(withTitle: title, action: #selector(menuAction), keyEquivalent: "")
            item.tag = index
            item.image = image
            item.target = self
        }
        
        let point = NSPoint(x: 0, y: self.buttons[0].bounds.height)
        menu.popUp(positioning: nil, at: point, in: self.buttons[0])
    }
}
