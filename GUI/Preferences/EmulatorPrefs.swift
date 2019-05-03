// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

extension PreferencesController {
    
    func refreshEmulatorTab() {
        
        track()
        
        guard
            let amiga      = amigaProxy,
            let controller = myController,
            let metal      = controller.metal
            else { return }
        
        // VC1541
        emuWarpLoad.state = controller.warpLoad ? .on : .off
        emuDriveSounds.state = controller.driveNoise ? .on : .off
        emuDriveSoundsNoPoll.state = controller.driveNoiseNoPoll ? .on : .off
        emuDriveSoundsNoPoll.isEnabled = controller.driveNoise
        emuDriveBlankDiskFormat.selectItem(withTag: controller.driveBlankDiskFormatIntValue)

        // Fullscreen
        emuAspectRatioButton.state = metal.keepAspectRatio ? .on : .off
        emuExitOnEscButton.state = controller.keyboardcontroller.exitOnEsc ? .on : .off
        
        // Screenshots
        emuScreenshotSourcePopup.selectItem(withTag: controller.screenshotSource)
        emuScreenshotTargetPopup.selectItem(withTag: controller.screenshotTargetIntValue)
        
        // Documents
        emuCloseWithoutAskingButton.state = controller.closeWithoutAsking ? .on : .off
        emuEjectWithoutAskingButton.state = controller.ejectWithoutAsking ? .on : .off
        
        // Miscellaneous
        emuPauseInBackground.state = controller.pauseInBackground ? .on : .off
        emuAutoSnapshots.state = amiga.takeAutoSnapshots() ? .on : .off
        emuSnapshotInterval.integerValue = amiga.snapshotInterval()
        emuSnapshotInterval.isEnabled = amiga.takeAutoSnapshots()
    }

    //
    // Action methods (VC1541)
    //
    
    @IBAction func emuWarpLoadAction(_ sender: NSButton!) {
        
        myController?.warpLoad = sender.state == .on
        refresh()
    }
    
    @IBAction func emuDriveSoundsAction(_ sender: NSButton!) {
        
        myController?.driveNoise = sender.state == .on
        refresh()
    }

    @IBAction func emuDriveSoundsNoPollAction(_ sender: NSButton!) {
        
        myController?.driveNoiseNoPoll = sender.state == .on
        refresh()
    }

    @IBAction func emuBlankDiskFormatAction(_ sender: NSPopUpButton!) {
        
        track("\(sender.selectedTag())")
        let tag = sender.selectedTag()
        myController?.driveBlankDiskFormat = FileSystemType(rawValue: tag)
        refresh()
    }
    
    
    //
    // Action methods (Fullscreen)
    //
    
    @IBAction func emuAspectRatioAction(_ sender: NSButton!) {
        
        if let metal = myController?.metal {
            metal.keepAspectRatio = (sender.state == .on)
            refresh()
        }
    }

    @IBAction func emuExitOnEscAction(_ sender: NSButton!) {
        
        if let keyboard = myController?.keyboardcontroller {
            keyboard.exitOnEsc = (sender.state == .on)
            refresh()
        }
    }

    
    //
    // Action methods (Screenshots)
    //
    
    @IBAction func emuScreenshotSourceAction(_ sender: NSPopUpButton!) {
        
        myController?.screenshotSource = sender.selectedTag()
        refresh()
    }
    
    @IBAction func emuScreenshotTargetAction(_ sender: NSPopUpButton!) {
        
        myController?.screenshotTargetIntValue = sender.selectedTag()
        refresh()
    }
    
    
    //
    // Action methods (User Dialogs)
    //
    
    @IBAction func emuCloseWithoutAskingAction(_ sender: NSButton!) {
        
        myController?.closeWithoutAsking = (sender.state == .on)
        myController?.needsSaving = amigaProxy?.isRunning() ?? false
        refresh()
    }
    
    @IBAction func emuEjectWithoutAskingAction(_ sender: NSButton!) {
        
        myController?.ejectWithoutAsking = (sender.state == .on)
        refresh()
    }
    
    
    //
    // Action methods (Miscellaneous)
    //
    
    @IBAction func emuPauseInBackgroundAction(_ sender: NSButton!) {
        
        myController?.pauseInBackground = (sender.state == .on)
        refresh()
    }
    
    @IBAction func emuAutoSnapshotAction(_ sender: NSButton!) {
        
        amigaProxy?.setTakeAutoSnapshots(sender.state == .on)
        refresh()
    }
    
    @IBAction func emuSnapshotIntervalAction(_ sender: NSTextField!) {
        
        track("\(sender.integerValue)")
        if sender.integerValue > 0 {
            amigaProxy?.setSnapshotInterval(sender.integerValue)
        } else {
            track("IGNORING")
        }
        refresh()
    }
    
    
    //
    // Action methods (Misc)
    //
    
    @IBAction func emuFactorySettingsAction(_ sender: Any!) {
        
        myController?.resetEmulatorUserDefaults()
        refresh()
    }
}

