// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension PreferencesController {
    
    func refreshCaptureTab() {
        
        // Screenshots
        capSnapshotInterval.isEnabled = pref.autoSnapshots
        capAutoScreenshots.state = pref.autoScreenshots ? .on : .off
        capScreenshotInterval.integerValue = pref.screenshotInterval
        capScreenshotInterval.isEnabled = pref.autoScreenshots
        capScreenshotSourcePopup.selectItem(withTag: pref.screenshotSource)
        capScreenshotTargetPopup.selectItem(withTag: pref.screenshotTargetIntValue)
        
        // Snapshots
        capAutoSnapshots.state = pref.autoSnapshots ? .on : .off
        capSnapshotInterval.integerValue = pref.snapshotInterval
        
        // Screen captures
        let hasFFmpeg = amiga.screenRecorder.ffmpegInstalled
        capFFmpegText.isHidden = !hasFFmpeg
        capFFmpegIcon1.isHidden = !hasFFmpeg
        capFFmpegIcon2.isHidden = !hasFFmpeg
        
        capSource.selectItem(withTag: pref.captureSource)
        capBitRate.selectItem(withTag: pref.bitRate)
        capAspectX.integerValue = pref.aspectX
        capAspectY.integerValue = pref.aspectY
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

    @IBAction func capBitrateAction(_ sender: NSPopUpButton!) {
        
        track("tag = \(sender.selectedTag())")
        pref.bitRate = sender.selectedTag()
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
    // Action methods (Misc)
    //
    
    @IBAction func capPresetAction(_ sender: NSPopUpButton!) {
        
        track()
        assert(sender.selectedTag() == 0)
        
        UserDefaults.resetCaptureUserDefaults()
        pref.loadCaptureDefaults(CaptureDefaults.std)
        refresh()
    }
}
