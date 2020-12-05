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
        
        let contaminated = pref.bootBlock >= 10
                
        // Drive
        genDriveBlankDiskFormat.selectItem(withTag: pref.blankDiskFormatIntValue)
        genBootCode.selectItem(withTag: pref.bootBlock)
        genBootCode.isEnabled = pref.blankDiskFormat != .NODOS
        genBootCodeVirus.isHidden = !contaminated
        genEjectWithoutAskingButton.state = pref.ejectWithoutAsking ? .on : .off
        genDriveSounds.state = pref.driveSounds ? .on : .off
        genDriveSoundPan.selectItem(withTag: Int(pref.driveSoundPan))
        genDriveInsertSound.state = pref.driveInsertSound ? .on : .off
        genDriveEjectSound.state = pref.driveEjectSound ? .on : .off
        genDriveHeadSound.state = pref.driveHeadSound ? .on : .off
        genDrivePollSound.state = pref.drivePollSound ? .on : .off
        genDriveSoundPan.isEnabled = pref.driveSounds
        genDriveInsertSound.isEnabled = pref.driveSounds
        genDriveEjectSound.isEnabled = pref.driveSounds
        genDriveHeadSound.isEnabled = pref.driveSounds
        genDrivePollSound.isEnabled = pref.driveSounds

        // Fullscreen
        genAspectRatioButton.state = pref.keepAspectRatio ? .on : .off
        genExitOnEscButton.state = pref.exitOnEsc ? .on : .off
                
        // Drive
        genWarpMode.selectItem(withTag: pref.warpModeIntValue)

        // Miscellaneous
        genPauseInBackground.state = pref.pauseInBackground ? .on : .off
        genCloseWithoutAskingButton.state = pref.closeWithoutAsking ? .on : .off
    }

    //
    // Action methods (Drive)
    //
        
    @IBAction func genDriveSoundsAction(_ sender: NSButton!) {
        
        pref.driveSounds = sender.state == .on
        refresh()
    }

    @IBAction func genDriveSoundPanAction(_ sender: NSPopUpButton!) {
        
        pref.driveSoundPan = Double(sender.selectedTag())
        refresh()
    }

    @IBAction func genDriveInsertSoundAction(_ sender: NSButton!) {
        
        pref.driveInsertSound = sender.state == .on
        refresh()
    }

    @IBAction func genDriveEjectSoundAction(_ sender: NSButton!) {
        
        pref.driveEjectSound = sender.state == .on
        refresh()
    }

    @IBAction func genDriveHeadSoundAction(_ sender: NSButton!) {
        
        pref.driveHeadSound = sender.state == .on
        refresh()
    }

    @IBAction func genDrivePollSoundAction(_ sender: NSButton!) {
        
        track()
        pref.drivePollSound = sender.state == .on
        refresh()
    }

    @IBAction func genBlankDiskFormatAction(_ sender: NSPopUpButton!) {
        
        let tag = sender.selectedTag()
        pref.blankDiskFormatIntValue = tag
        refresh()
    }

    @IBAction func genBootCodeAction(_ sender: NSPopUpButton!) {
        
        track("Tag = \(sender.selectedTag())")
        
        let tag = sender.selectedTag()
        pref.bootBlock = tag
        refresh()
    }

    //
    // Action methods (Fullscreen)
    //
    
    @IBAction func genAspectRatioAction(_ sender: NSButton!) {
        
        pref.keepAspectRatio = (sender.state == .on)
        refresh()
    }

    @IBAction func genExitOnEscAction(_ sender: NSButton!) {
        
        pref.exitOnEsc = (sender.state == .on)
        refresh()
    }

    //
    // Action methods (Warp mode)
    //

    @IBAction func genWarpModeAction(_ sender: NSPopUpButton!) {
        
        pref.warpMode = WarpMode(rawValue: sender.selectedTag())!
        refresh()
    }
    
    //
    // Action methods (Miscellaneous)
    //
    
    @IBAction func genPauseInBackgroundAction(_ sender: NSButton!) {
        
        pref.pauseInBackground = (sender.state == .on)
        refresh()
    }
    
    @IBAction func genCloseWithoutAskingAction(_ sender: NSButton!) {
        
        pref.closeWithoutAsking = (sender.state == .on)
        for c in myAppDelegate.controllers {
            c.needsSaving = c.amiga.isRunning
        }
        refresh()
    }
    
    @IBAction func genEjectWithoutAskingAction(_ sender: NSButton!) {
        
        pref.ejectWithoutAsking = (sender.state == .on)
        refresh()
    }
    
    //
    // Action methods (Misc)
    //
    
    @IBAction func generalPresetAction(_ sender: NSPopUpButton!) {
        
        assert(sender.selectedTag() == 0)

        UserDefaults.resetGeneralUserDefaults()
        pref.loadGeneralUserDefaults()
        refresh()
    }
}
