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
    var gamePadManager: GamePadManager? { controller?.gamePadManager }
    var emu: EmulatorProxy? { controller?.emu }

    func refresh() { }
    func preset(tag: Int) { }
    func save() { }
}
