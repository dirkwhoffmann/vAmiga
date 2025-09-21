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
