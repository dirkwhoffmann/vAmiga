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
        
        // Snapshots
        genAutoSnapshots.state = pref.autoSnapshots ? .on : .off
        genSnapshotInterval.integerValue = pref.snapshotInterval

        // Screenshots
        genSnapshotInterval.isEnabled = pref.autoSnapshots
        genAutoScreenshots.state = pref.autoScreenshots ? .on : .off
        genScreenshotInterval.integerValue = pref.screenshotInterval
        genScreenshotInterval.isEnabled = pref.autoScreenshots
        genScreenshotSourcePopup.selectItem(withTag: pref.screenshotSource)
        genScreenshotTargetPopup.selectItem(withTag: pref.screenshotTargetIntValue)
                
        // Screen captures
        let hasFFmpeg = amiga.screenRecorder.hasFFmpeg
        genSource.selectItem(withTag: pref.captureSource)
        genBitRate.integerValue = pref.bitRate
        genAspectX.integerValue = pref.aspectX
        genAspectY.integerValue = pref.aspectY
        genSource.isEnabled = hasFFmpeg
        genBitRate.isEnabled = hasFFmpeg
        genAspectX.isEnabled = hasFFmpeg
        genAspectY.isEnabled = hasFFmpeg
        if hasFFmpeg {
            genFFmpegLabel.textColor = .textColor
            genFFmpegLabel.stringValue = "ffmpeg found in /usr/local/bin"
        } else {
            genFFmpegLabel.textColor = .warningColor
            genFFmpegLabel.stringValue = "/usr/local/bin/ffmpeg not found"
        }
        
        // Fullscreen
        genAspectRatioButton.state = pref.keepAspectRatio ? .on : .off
        genExitOnEscButton.state = pref.exitOnEsc ? .on : .off
                
        // Drive
        genWarpMode.selectItem(withTag: pref.warpModeIntValue)

        // Miscellaneous
        genEjectWithoutAskingButton.state = pref.ejectWithoutAsking ? .on : .off
        genPauseInBackground.state = pref.pauseInBackground ? .on : .off
        genCloseWithoutAskingButton.state = pref.closeWithoutAsking ? .on : .off
    }

    //
    // Action methods (Snapshots)
    //
    
    @IBAction func capAutoSnapshotAction(_ sender: NSButton!) {
        
        pref.autoSnapshots = sender.state == .on
        refresh()
    }
    
    @IBAction func capSnapshotIntervalAction(_ sender: NSTextField!) {
        
        if sender.integerValue > 0 {
            pref.snapshotInterval = sender.integerValue
        }
        refresh()
    }
    
    //
    // Action methods (Screenshots)
    //

    @IBAction func capAutoScreenshotAction(_ sender: NSButton!) {
        
        pref.autoScreenshots = sender.state == .on
        refresh()
    }
    
    @IBAction func capScreenshotIntervalAction(_ sender: NSTextField!) {
        
        if sender.integerValue > 0 {
            pref.screenshotInterval = sender.integerValue
        }
        refresh()
    }
    
    @IBAction func capScreenshotSourceAction(_ sender: NSPopUpButton!) {
        
        pref.screenshotSource = sender.selectedTag()
        refresh()
    }
    
    @IBAction func capScreenshotTargetAction(_ sender: NSPopUpButton!) {
        
        pref.screenshotTargetIntValue = sender.selectedTag()
        refresh()
    }

    //
    // Action methods (Screen captures)
    //
    
    @IBAction func capSourceAction(_ sender: NSPopUpButton!) {
        
        track("tag = \(sender.selectedTag())")
        pref.captureSource = sender.selectedTag()
        refresh()
    }

    @IBAction func capBitrateAction(_ sender: NSComboBox!) {
        
        track("value = \(sender.integerValue)")
        pref.bitRate = sender.integerValue
        refresh()
    }

    @IBAction func capAspectXAction(_ sender: NSTextField!) {
        
        track("value = \(sender.integerValue)")
        pref.aspectX = sender.integerValue
        refresh()
    }

    @IBAction func capAspectYAction(_ sender: NSTextField!) {
        
        track("value = \(sender.integerValue)")
        pref.aspectY = sender.integerValue
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
    
    @IBAction func genEjectWithoutAskingAction(_ sender: NSButton!) {
        
        pref.ejectWithoutAsking = (sender.state == .on)
        refresh()
    }
    
    @IBAction func genPauseInBackgroundAction(_ sender: NSButton!) {
        
        pref.pauseInBackground = (sender.state == .on)
        refresh()
    }
    
    @IBAction func genCloseWithoutAskingAction(_ sender: NSButton!) {
        
        pref.closeWithoutAsking = (sender.state == .on)
        for c in myAppDelegate.controllers {
            c.needsSaving = c.amiga.running
        }
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
