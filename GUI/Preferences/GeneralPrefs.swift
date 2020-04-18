// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension PreferencesController {
    
    func refreshEmulatorTab() {
        
        track()

        // Drive
        emuWarpLoad.state = prefs.warpLoad ? .on : .off
        emuDriveSounds.state = prefs.driveSounds ? .on : .off
        emuDriveSoundPan.selectItem(withTag: Int(prefs.driveSoundPan))
        emuDriveInsertSound.state = prefs.driveInsertSound ? .on : .off
        emuDriveEjectSound.state = prefs.driveEjectSound ? .on : .off
        emuDriveHeadSound.state = prefs.driveHeadSound ? .on : .off
        emuDrivePollSound.state = prefs.drivePollSound ? .on : .off
        emuDriveSoundPan.isEnabled = prefs.driveSounds
        emuDriveInsertSound.isEnabled = prefs.driveSounds
        emuDriveEjectSound.isEnabled = prefs.driveSounds
        emuDriveHeadSound.isEnabled = prefs.driveSounds
        emuDrivePollSound.isEnabled = prefs.driveSounds
        emuDriveBlankDiskFormat.selectItem(withTag: prefs.driveBlankDiskFormatIntValue)

        // Fullscreen
        emuAspectRatioButton.state = prefs.keepAspectRatio ? .on : .off
        emuExitOnEscButton.state = prefs.exitOnEsc ? .on : .off
        
        // Snapshots and Screenshots
        emuAutoSnapshots.state = prefs.autoSnapshots ? .on : .off
        emuSnapshotInterval.integerValue = prefs.snapshotInterval
        emuSnapshotInterval.isEnabled = prefs.autoSnapshots
        emuAutoScreenshots.state = prefs.autoScreenshots ? .on : .off
        emuScreenshotInterval.integerValue = prefs.screenshotInterval
        emuScreenshotInterval.isEnabled = prefs.autoScreenshots
        emuScreenshotSourcePopup.selectItem(withTag: prefs.screenshotSource)
        emuScreenshotTargetPopup.selectItem(withTag: prefs.screenshotTargetIntValue)
        
        // Miscellaneous
        emuPauseInBackground.state = prefs.pauseInBackground ? .on : .off
        emuCloseWithoutAskingButton.state = prefs.closeWithoutAsking ? .on : .off
        emuEjectWithoutAskingButton.state = prefs.ejectWithoutAsking ? .on : .off

        // OK Button
        emuOKButton.title = buttonLabel
    }

    //
    // Action methods (Drive)
    //
    
    @IBAction func emuWarpLoadAction(_ sender: NSButton!) {
        
        prefs.warpLoad = sender.state == .on
        refresh()
    }
    
    @IBAction func emuDriveSoundsAction(_ sender: NSButton!) {
        
        prefs.driveSounds = sender.state == .on
        refresh()
    }

    @IBAction func emuDriveSoundPanAction(_ sender: NSPopUpButton!) {
        
        prefs.driveSoundPan = Double(sender.selectedTag())
        refresh()
    }

    @IBAction func emuDriveInsertSoundAction(_ sender: NSButton!) {
        
        prefs.driveInsertSound = sender.state == .on
        refresh()
    }

    @IBAction func emuDriveEjectSoundAction(_ sender: NSButton!) {
        
        prefs.driveEjectSound = sender.state == .on
        refresh()
    }

    @IBAction func emuDriveHeadSoundAction(_ sender: NSButton!) {
        
        prefs.driveHeadSound = sender.state == .on
        refresh()
    }

    @IBAction func emuDrivePollSoundAction(_ sender: NSButton!) {
        
        track()
        prefs.drivePollSound = sender.state == .on
        refresh()
    }

    @IBAction func emuBlankDiskFormatAction(_ sender: NSPopUpButton!) {
        
        let tag = sender.selectedTag()
        prefs.driveBlankDiskFormat = FileSystemType(rawValue: tag)
        refresh()
    }

    //
    // Action methods (Fullscreen)
    //
    
    @IBAction func emuAspectRatioAction(_ sender: NSButton!) {
        
        prefs.keepAspectRatio = (sender.state == .on)
        refresh()
    }

    @IBAction func emuExitOnEscAction(_ sender: NSButton!) {
        
        prefs.exitOnEsc = (sender.state == .on)
        refresh()
    }

    //
    // Action methods (Snapshots and screenshots)
    //
    
    @IBAction func emuAutoSnapshotAction(_ sender: NSButton!) {
        
        prefs.autoSnapshots = sender.state == .on
        refresh()
    }
    
    @IBAction func emuSnapshotIntervalAction(_ sender: NSTextField!) {
        
        if sender.integerValue > 0 {
            prefs.snapshotInterval = sender.integerValue
        }
        refresh()
    }
    
    @IBAction func emuAutoScreenshotAction(_ sender: NSButton!) {
        
        prefs.autoScreenshots = sender.state == .on
        refresh()
    }
    
    @IBAction func emuScreenshotIntervalAction(_ sender: NSTextField!) {
        
        if sender.integerValue > 0 {
            prefs.screenshotInterval = sender.integerValue
        }
        refresh()
    }
    
    @IBAction func emuScreenshotSourceAction(_ sender: NSPopUpButton!) {
        
        prefs.screenshotSource = sender.selectedTag()
        refresh()
    }
    
    @IBAction func emuScreenshotTargetAction(_ sender: NSPopUpButton!) {
        
        prefs.screenshotTargetIntValue = sender.selectedTag()
        refresh()
    }

    //
    // Action methods (Miscellaneous)
    //
    
    @IBAction func emuPauseInBackgroundAction(_ sender: NSButton!) {
        
        prefs.pauseInBackground = (sender.state == .on)
        refresh()
    }
    
    @IBAction func emuCloseWithoutAskingAction(_ sender: NSButton!) {
        
        prefs.closeWithoutAsking = (sender.state == .on)
        parent.needsSaving = amiga.isRunning()
        refresh()
    }
    
    @IBAction func emuEjectWithoutAskingAction(_ sender: NSButton!) {
        
        prefs.ejectWithoutAsking = (sender.state == .on)
        refresh()
    }
    
    //
    // Action methods (Misc)
    //
    
    @IBAction func emuFactorySettingsAction(_ sender: Any!) {
        
        UserDefaults.resetGeneralUserDefaults()
        prefs.loadGeneralUserDefaults()
        refresh()
    }
}
