// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension PreferencesController {
    
    func refreshGeneralTab() {
        
        track()

        // Drive
        emuDriveBlankDiskFormat.selectItem(withTag: pref.driveBlankDiskFormatIntValue)
        emuEjectWithoutAskingButton.state = pref.ejectWithoutAsking ? .on : .off
        emuDriveSounds.state = pref.driveSounds ? .on : .off
        emuDriveSoundPan.selectItem(withTag: Int(pref.driveSoundPan))
        emuDriveInsertSound.state = pref.driveInsertSound ? .on : .off
        emuDriveEjectSound.state = pref.driveEjectSound ? .on : .off
        emuDriveHeadSound.state = pref.driveHeadSound ? .on : .off
        emuDrivePollSound.state = pref.drivePollSound ? .on : .off
        emuDriveSoundPan.isEnabled = pref.driveSounds
        emuDriveInsertSound.isEnabled = pref.driveSounds
        emuDriveEjectSound.isEnabled = pref.driveSounds
        emuDriveHeadSound.isEnabled = pref.driveSounds
        emuDrivePollSound.isEnabled = pref.driveSounds

        // Fullscreen
        emuAspectRatioButton.state = pref.keepAspectRatio ? .on : .off
        emuExitOnEscButton.state = pref.exitOnEsc ? .on : .off
        
        // Snapshots and Screenshots
        emuAutoSnapshots.state = pref.autoSnapshots ? .on : .off
        emuSnapshotInterval.integerValue = pref.snapshotInterval
        emuSnapshotInterval.isEnabled = pref.autoSnapshots
        emuAutoScreenshots.state = pref.autoScreenshots ? .on : .off
        emuScreenshotInterval.integerValue = pref.screenshotInterval
        emuScreenshotInterval.isEnabled = pref.autoScreenshots
        emuScreenshotSourcePopup.selectItem(withTag: pref.screenshotSource)
        emuScreenshotTargetPopup.selectItem(withTag: pref.screenshotTargetIntValue)
        
        // Drive
        emuWarpMode.selectItem(withTag: pref.warpModeIntValue)

        // Miscellaneous
        emuPauseInBackground.state = pref.pauseInBackground ? .on : .off
        emuCloseWithoutAskingButton.state = pref.closeWithoutAsking ? .on : .off
    }

    //
    // Action methods (Drive)
    //
        
    @IBAction func emuDriveSoundsAction(_ sender: NSButton!) {
        
        pref.driveSounds = sender.state == .on
        refresh()
    }

    @IBAction func emuDriveSoundPanAction(_ sender: NSPopUpButton!) {
        
        pref.driveSoundPan = Double(sender.selectedTag())
        refresh()
    }

    @IBAction func emuDriveInsertSoundAction(_ sender: NSButton!) {
        
        pref.driveInsertSound = sender.state == .on
        refresh()
    }

    @IBAction func emuDriveEjectSoundAction(_ sender: NSButton!) {
        
        pref.driveEjectSound = sender.state == .on
        refresh()
    }

    @IBAction func emuDriveHeadSoundAction(_ sender: NSButton!) {
        
        pref.driveHeadSound = sender.state == .on
        refresh()
    }

    @IBAction func emuDrivePollSoundAction(_ sender: NSButton!) {
        
        track()
        pref.drivePollSound = sender.state == .on
        refresh()
    }

    @IBAction func emuBlankDiskFormatAction(_ sender: NSPopUpButton!) {
        
        let tag = sender.selectedTag()
        pref.driveBlankDiskFormat = FileSystemType(rawValue: tag)!
        refresh()
    }

    //
    // Action methods (Fullscreen)
    //
    
    @IBAction func emuAspectRatioAction(_ sender: NSButton!) {
        
        pref.keepAspectRatio = (sender.state == .on)
        refresh()
    }

    @IBAction func emuExitOnEscAction(_ sender: NSButton!) {
        
        pref.exitOnEsc = (sender.state == .on)
        refresh()
    }

    //
    // Action methods (Snapshots and screenshots)
    //
    
    @IBAction func emuAutoSnapshotAction(_ sender: NSButton!) {
        
        pref.autoSnapshots = sender.state == .on
        refresh()
    }
    
    @IBAction func emuSnapshotIntervalAction(_ sender: NSTextField!) {
        
        if sender.integerValue > 0 {
            pref.snapshotInterval = sender.integerValue
        }
        refresh()
    }
    
    @IBAction func emuAutoScreenshotAction(_ sender: NSButton!) {
        
        pref.autoScreenshots = sender.state == .on
        refresh()
    }
    
    @IBAction func emuScreenshotIntervalAction(_ sender: NSTextField!) {
        
        if sender.integerValue > 0 {
            pref.screenshotInterval = sender.integerValue
        }
        refresh()
    }
    
    @IBAction func emuScreenshotSourceAction(_ sender: NSPopUpButton!) {
        
        pref.screenshotSource = sender.selectedTag()
        refresh()
    }
    
    @IBAction func emuScreenshotTargetAction(_ sender: NSPopUpButton!) {
        
        pref.screenshotTargetIntValue = sender.selectedTag()
        refresh()
    }

    //
    // Action methods (Warp mode)
    //

    @IBAction func emuWarpModeAction(_ sender: NSPopUpButton!) {
        
        pref.warpMode = WarpMode(rawValue: sender.selectedTag())!
        refresh()
    }
    
    //
    // Action methods (Miscellaneous)
    //
    
    @IBAction func emuPauseInBackgroundAction(_ sender: NSButton!) {
        
        pref.pauseInBackground = (sender.state == .on)
        refresh()
    }
    
    @IBAction func emuCloseWithoutAskingAction(_ sender: NSButton!) {
        
        pref.closeWithoutAsking = (sender.state == .on)
        for c in myAppDelegate.controllers {
            c.needsSaving = c.amiga.isRunning
        }
        refresh()
    }
    
    @IBAction func emuEjectWithoutAskingAction(_ sender: NSButton!) {
        
        pref.ejectWithoutAsking = (sender.state == .on)
        refresh()
    }
    
    //
    // Action methods (Misc)
    //
    
    @IBAction func generalPresetAction(_ sender: NSPopUpButton!) {
        
        track()
        assert(sender.selectedTag() == 0)

        UserDefaults.resetGeneralUserDefaults()
        pref.loadEmulatorUserDefaults()
        refresh()
    }
}
