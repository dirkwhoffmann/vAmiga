// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class SettingsViewController: NSViewController {

    var pref: Preferences { myAppDelegate.pref }
    var controller: MyController? { MyAppDelegate.currentController }
    var config: Configuration? { return controller?.config }
    var gamePadManager: GamePadManager? { controller?.gamePadManager }
    var emu: EmulatorProxy? { controller?.emu }

    var presets: [(String,Int)] { [ ("Standard settings", 0) ] }

    func activate() {

        // view.window?.makeFirstResponder(self)
        refresh()
    }

    func refresh() { }
    override func keyDown(with event: NSEvent) { }
    override func flagsChanged(with event: NSEvent) { }
    func preset(tag: Int) { }
    func save() { }
    
}

class SettingsView: NSView {

    private var bgLayer: CALayer?

    override func viewDidMoveToSuperview() {

        super.viewDidMoveToSuperview()
        wantsLayer = true

        if let layer = self.layer, bgLayer == nil {

            let background = CALayer()
            background.contents = NSImage(named: "vAmigaBg")
            background.contentsGravity = .resizeAspectFill
            background.autoresizingMask = [.layerWidthSizable, .layerHeightSizable]
            layer.insertSublayer(background, at: 0)
            bgLayer = background
        }
    }

    override func layout() {

        super.layout()
        bgLayer?.frame = CGRect(x: 0, y: 0,
                                width: bounds.width,
                                height: bounds.height - 32)
    }
}
