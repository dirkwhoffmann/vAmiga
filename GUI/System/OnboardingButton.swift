// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
class OnboardingButton: NSControl {

    @IBOutlet weak var icon: NSImageView?
    @IBOutlet weak var label: NSTextField?

    private let box = NSBox()
    private let clickButton = NSButton()

    var state: NSControl.StateValue = .off { didSet { update() } }

    @IBInspectable override var tag: Int {

        get { super.tag }
        set { super.tag = newValue }
    }

    override init(frame frameRect: NSRect) {

        super.init(frame: frameRect)
        setup()
    }

    required init?(coder: NSCoder) {

        super.init(coder: coder)
        setup()
    }

    private func setup() {

        wantsLayer = true

        // Box
        box.boxType = .primary
        box.titlePosition = .noTitle
        addSubview(box)

        // Clickable button (fills whole box, invisible)
        clickButton.bezelStyle = .regularSquare
        clickButton.isBordered = false
        clickButton.title = ""
        clickButton.target = self
        clickButton.action = #selector(buttonClicked)
        clickButton.imagePosition = .imageAbove
        addSubview(clickButton)
    }

    override func layout() {
        
        super.layout()
        box.frame = bounds
        clickButton.frame = bounds
    }

    @objc private func buttonClicked() {

        print("Button clicked")
        sendAction(action, to: target)
    }

    private func update() {

        print("updating state: \(state)")
        switch state {

        case .on:
            box.isTransparent = false
            label?.isEnabled = false
            icon?.isEnabled = true

        default:
            box.isTransparent = true
            label?.isEnabled = false
            icon?.isEnabled = false
        }
    }
}
