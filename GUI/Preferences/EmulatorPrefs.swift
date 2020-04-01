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
        emuDriveSounds.state = parent.driveNoise ? .on : .off
        emuDriveSoundsNoPoll.state = parent.driveNoiseNoPoll ? .on : .off
        emuDriveSoundsNoPoll.isEnabled = parent.driveNoise
        emuDriveBlankDiskFormat.selectItem(withTag: parent.driveBlankDiskFormatIntValue)

        // Fullscreen
        emuAspectRatioButton.state = renderer.keepAspectRatio ? .on : .off
        emuExitOnEscButton.state = parent.kbController.exitOnEsc ? .on : .off
        
        // Snapshots and Screenshots
        emuAutoSnapshots.state = amiga.takeAutoSnapshots() ? .on : .off
        emuSnapshotInterval.integerValue = amiga.snapshotInterval()
        emuSnapshotInterval.isEnabled = amiga.takeAutoSnapshots()
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
        
        parent.driveNoise = sender.state == .on
        refresh()
    }

    @IBAction func emuDriveSoundsNoPollAction(_ sender: NSButton!) {
        
        parent.driveNoiseNoPoll = sender.state == .on
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
        
        amiga.setTakeAutoSnapshots(sender.state == .on)
        refresh()
    }
    
    @IBAction func emuSnapshotIntervalAction(_ sender: NSTextField!) {
        
        if sender.integerValue > 0 {
            amiga.setSnapshotInterval(sender.integerValue)
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
