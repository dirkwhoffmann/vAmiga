// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class GeneralSettingsViewController: NSViewController {

    var pref: Preferences { return myAppDelegate.pref }

    // Workspaces
    @IBOutlet weak var compressWorkspaces: NSButton!

    // Fullscreen
    @IBOutlet weak var aspectRatioButton: NSButton!
    @IBOutlet weak var exitOnEscButton: NSButton!

    // Snapshots
    @IBOutlet weak var snapshotCompressor: NSPopUpButton!
    @IBOutlet weak var autoSnapshots: NSButton!
    @IBOutlet weak var snapshotInterval: NSTextField!
    @IBOutlet weak var snapshotStorage: NSTextField!

    // Screenshots
    @IBOutlet weak var screenshotFormatPopup: NSPopUpButton!
    @IBOutlet weak var screenshotSourcePopup: NSPopUpButton!
    @IBOutlet weak var screenshotCutoutPopup: NSPopUpButton!
    @IBOutlet weak var screenshotCutoutText: NSTextField!
    @IBOutlet weak var screenshotWidth: NSTextField!
    @IBOutlet weak var screenshotWidthText: NSTextField!
    @IBOutlet weak var screenshotHeight: NSTextField!
    @IBOutlet weak var screenshotHeightText: NSTextField!

    // Misc
    @IBOutlet weak var ejectWithoutAskingButton: NSButton!
    @IBOutlet weak var detachWithoutAskingButton: NSButton!
    @IBOutlet weak var closeWithoutAskingButton: NSButton!
    @IBOutlet weak var pauseInBackground: NSButton!

    override func viewDidLoad() {

        print("GeneralSettingsViewController::viewDidLoad")
    }

    //
    // Refresh
    //

    func refresh() {

        print("GeneralSettingsViewController::refresh")
    }

    //
    // Action methods
    //

    @IBAction func snapshotStorageAction(_ sender: NSTextField!) {

        if sender.integerValue > 0 {
            pref.snapshotStorage = sender.integerValue
        }
        refresh()
    }

    @IBAction func autoSnapshotAction(_ sender: NSButton!) {

        pref.autoSnapshots = sender.state == .on
        refresh()
    }

    @IBAction func snapshotIntervalAction(_ sender: NSTextField!) {

        if sender.integerValue > 0 {
            pref.snapshotInterval = sender.integerValue
        }
        refresh()
    }

    @IBAction func aspectRatioAction(_ sender: NSButton!) {

        pref.keepAspectRatio = (sender.state == .on)
        refresh()
    }

    @IBAction func exitOnEscAction(_ sender: NSButton!) {

        pref.exitOnEsc = (sender.state == .on)
        refresh()
    }

    @IBAction func ejectWithoutAskingAction(_ sender: NSButton!) {

        pref.ejectWithoutAsking = (sender.state == .on)
        refresh()
    }

    @IBAction func detachWithoutAskingAction(_ sender: NSButton!) {

        pref.detachWithoutAsking = (sender.state == .on)
        refresh()
    }

    @IBAction func closeWithoutAskingAction(_ sender: NSButton!) {

        pref.closeWithoutAsking = (sender.state == .on)
        refresh()
    }

    @IBAction func pauseInBackgroundAction(_ sender: NSButton!) {

        pref.pauseInBackground = (sender.state == .on)
        refresh()
    }

    @IBAction func screenshotSourceAction(_ sender: NSPopUpButton!) {

        pref.screenshotSourceIntValue = sender.selectedTag()
        refresh()
    }

    @IBAction func screenshotFormatAction(_ sender: NSPopUpButton!) {

        pref.screenshotFormatIntValue = sender.selectedTag()
        refresh()
    }

    @IBAction func screenshotCutoutAction(_ sender: NSPopUpButton!) {

        pref.screenshotCutoutIntValue = sender.selectedTag()
        refresh()
    }

    @IBAction func screenshotWidthAction(_ sender: NSTextField!) {

        pref.aspectX = sender.integerValue
        refresh()
    }

    @IBAction func screenshotHeightAction(_ sender: NSTextField!) {

        pref.aspectY = sender.integerValue
        refresh()
    }

    @IBAction func generalPresetAction(_ sender: NSPopUpButton!) {

        assert(sender.selectedTag() == 0)

        // Revert to standard settings
        EmulatorProxy.defaults.removeGeneralUserDefaults()

        // Apply the new settings
        pref.applyGeneralUserDefaults()

        refresh()
    }
}
