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

        let renderer = parent.renderer!

        // Drive
        emuWarpLoad.state = parent.warpLoad ? .on : .off
        emuDriveSounds.state = parent.driveSounds ? .on : .off
        emuDriveSoundPan.selectItem(withTag: Int(parent.driveSoundPan))
        emuDriveInsertSound.state = parent.driveInsertSound ? .on : .off
        emuDriveEjectSound.state = parent.driveEjectSound ? .on : .off
        emuDriveHeadSound.state = parent.driveHeadSound ? .on : .off
        emuDrivePollSound.state = parent.drivePollSound ? .on : .off
        emuDriveSoundPan.isEnabled = parent.driveSounds
        emuDriveInsertSound.isEnabled = parent.driveSounds
        emuDriveEjectSound.isEnabled = parent.driveSounds
        emuDriveHeadSound.isEnabled = parent.driveSounds
        emuDrivePollSound.isEnabled = parent.driveSounds
        emuDriveBlankDiskFormat.selectItem(withTag: parent.driveBlankDiskFormatIntValue)

        // Fullscreen
        emuAspectRatioButton.state = renderer.keepAspectRatio ? .on : .off
        emuExitOnEscButton.state = parent.kbController.exitOnEsc ? .on : .off
        
        // Snapshots and Screenshots
        emuAutoSnapshots.state = parent.autoSnapshots ? .on : .off
        emuSnapshotInterval.integerValue = parent.snapshotInterval
        emuSnapshotInterval.isEnabled = parent.autoSnapshots
        emuAutoScreenshots.state = parent.autoScreenshots ? .on : .off
        emuScreenshotInterval.integerValue = parent.screenshotInterval
        emuScreenshotInterval.isEnabled = parent.autoScreenshots
        emuScreenshotSourcePopup.selectItem(withTag: parent.screenshotSource)
        emuScreenshotTargetPopup.selectItem(withTag: parent.screenshotTargetIntValue)
        
        // Miscellaneous
        emuPauseInBackground.state = parent.pauseInBackground ? .on : .off
        emuCloseWithoutAskingButton.state = parent.closeWithoutAsking ? .on : .off
        emuEjectWithoutAskingButton.state = parent.ejectWithoutAsking ? .on : .off

        // OK Button
        emuOKButton.title = buttonLabel
    }

    //
    // Action methods (Drive)
    //
    
    @IBAction func emuWarpLoadAction(_ sender: NSButton!) {
        
        parent.warpLoad = sender.state == .on
        refresh()
    }
    
    @IBAction func emuDriveSoundsAction(_ sender: NSButton!) {
        
        parent.driveSounds = sender.state == .on
        refresh()
    }

    @IBAction func emuDriveSoundPanAction(_ sender: NSPopUpButton!) {
        
        track("tag = \(sender.selectedTag())")
        parent.driveSoundPan = Double(sender.selectedTag())
        refresh()
    }

    @IBAction func emuDriveInsertSoundAction(_ sender: NSButton!) {
        
        parent.driveInsertSound = sender.state == .on
        track("value = \(parent.driveInsertSound)")
        refresh()
    }

    @IBAction func emuDriveEjectSoundAction(_ sender: NSButton!) {
        
        parent.driveEjectSound = sender.state == .on
        track("value = \(parent.driveEjectSound)")
        refresh()
    }

    @IBAction func emuDriveHeadSoundAction(_ sender: NSButton!) {
        
        parent.driveHeadSound = sender.state == .on
        refresh()
    }

    @IBAction func emuDrivePollSoundAction(_ sender: NSButton!) {
        
        parent.drivePollSound = sender.state == .on
        refresh()
    }

    @IBAction func emuBlankDiskFormatAction(_ sender: NSPopUpButton!) {
        
        let tag = sender.selectedTag()
        parent.driveBlankDiskFormat = FileSystemType(rawValue: tag)
        refresh()
    }

    //
    // Action methods (Fullscreen)
    //
    
    @IBAction func emuAspectRatioAction(_ sender: NSButton!) {
        
        parent.renderer.keepAspectRatio = (sender.state == .on)
        refresh()
    }

    @IBAction func emuExitOnEscAction(_ sender: NSButton!) {
        
        parent.kbController.exitOnEsc = (sender.state == .on)
        refresh()
    }

    //
    // Action methods (Snapshots and screenshots)
    //
    
    @IBAction func emuAutoSnapshotAction(_ sender: NSButton!) {
        
        parent.autoSnapshots = sender.state == .on
        refresh()
    }
    
    @IBAction func emuSnapshotIntervalAction(_ sender: NSTextField!) {
        
        if sender.integerValue > 0 {
            parent.snapshotInterval = sender.integerValue
        }
        refresh()
    }
    
    @IBAction func emuAutoScreenshotAction(_ sender: NSButton!) {
        
        parent.autoScreenshots = sender.state == .on
        refresh()
    }
    
    @IBAction func emuScreenshotIntervalAction(_ sender: NSTextField!) {
        
        if sender.integerValue > 0 {
            parent.screenshotInterval = sender.integerValue
        }
        refresh()
    }
    
    @IBAction func emuScreenshotSourceAction(_ sender: NSPopUpButton!) {
        
        parent.screenshotSource = sender.selectedTag()
        refresh()
    }
    
    @IBAction func emuScreenshotTargetAction(_ sender: NSPopUpButton!) {
        
        parent.screenshotTargetIntValue = sender.selectedTag()
        refresh()
    }

    //
    // Action methods (Miscellaneous)
    //
    
    @IBAction func emuPauseInBackgroundAction(_ sender: NSButton!) {
        
        parent.pauseInBackground = (sender.state == .on)
        refresh()
    }
    
    @IBAction func emuCloseWithoutAskingAction(_ sender: NSButton!) {
        
        parent.closeWithoutAsking = (sender.state == .on)
        parent.needsSaving = amiga.isRunning()
        refresh()
    }
    
    @IBAction func emuEjectWithoutAskingAction(_ sender: NSButton!) {
        
        parent.ejectWithoutAsking = (sender.state == .on)
        refresh()
    }
    
    //
    // Action methods (Misc)
    //
    
    @IBAction func emuFactorySettingsAction(_ sender: Any!) {
        
        parent.resetEmulatorUserDefaults()
        refresh()
    }
}
