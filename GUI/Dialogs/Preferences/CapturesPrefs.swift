// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

@MainActor
extension PreferencesController {
    
    func refreshCapturesTab() {
        
        // Initialize combo boxes
        if capVideoFFmpegPath.tag == 0 {
            
            capVideoFFmpegPath.tag = 1
            
            for i in 0...5 {
                if let path = emu.recorder.findFFmpeg(i) {
                    capVideoFFmpegPath.addItem(withObjectValue: path)
                } else {
                    break
                }
            }
        }
        
        // Screenshots
        let framebuffer = pref.screenshotSource == .framebuffer
        let custom = pref.screenshotCutout == .custom
        capScreenshotFormatPopup.selectItem(withTag: pref.screenshotFormatIntValue)
        capScreenshotSourcePopup.selectItem(withTag: pref.screenshotSourceIntValue)
        capScreenshotCutoutPopup.selectItem(withTag: pref.screenshotCutoutIntValue)
        capScreenshotCutoutPopup.isHidden = framebuffer
        capScreenshotCutoutText.isHidden = framebuffer
        capScreenshotWidth.integerValue = pref.screenshotWidth
        capScreenshotWidth.isHidden = !custom || framebuffer
        capScreenshotWidthText.isHidden = !custom || framebuffer
        capScreenshotHeight.integerValue = pref.screenshotHeight
        capScreenshotHeight.isHidden = !custom || framebuffer
        capScreenshotHeightText.isHidden = !custom || framebuffer

        // Screen captures
        let hasFFmpeg = emu.recorder.hasFFmpeg
        capVideoFFmpegPath.stringValue = emu.recorder.path
        capVideoFFmpegPath.textColor = hasFFmpeg ? .textColor : .warning
        capVideoSource.selectItem(withTag: pref.captureSourceIntValue)
        capVideoBitRate.stringValue = "\(pref.bitRate)"
        capVideoAspectX.integerValue = pref.aspectX
        capVideoAspectY.integerValue = pref.aspectY
        capVideoSource.isEnabled = hasFFmpeg
        capVideoBitRate.isEnabled = hasFFmpeg
        capVideoBitRateText.textColor = hasFFmpeg ? .labelColor : .disabledControlTextColor
        capVideoAspectX.isEnabled = hasFFmpeg
        capVideoAspectY.isEnabled = hasFFmpeg
    }

    func selectCapturesTab() {

        refreshCapturesTab()
    }

    //
    // Action methods (Screenshots)
    //

    @IBAction func capScreenshotSourceAction(_ sender: NSPopUpButton!) {
        
        pref.screenshotSourceIntValue = sender.selectedTag()
        refresh()
    }
    
    @IBAction func capScreenshotFormatAction(_ sender: NSPopUpButton!) {
        
        pref.screenshotFormatIntValue = sender.selectedTag()
        refresh()
    }

    @IBAction func capScreenshotCutoutAction(_ sender: NSPopUpButton!) {
        
        pref.screenshotCutoutIntValue = sender.selectedTag()
        refresh()
    }

    @IBAction func capScreenshotWidthAction(_ sender: NSTextField!) {
        
        pref.aspectX = sender.integerValue
        refresh()
    }

    @IBAction func capScreenshotHeightAction(_ sender: NSTextField!) {
        
        pref.aspectY = sender.integerValue
        refresh()
    }

    //
    // Action methods (Videos)
    //
    
    @IBAction func capPathAction(_ sender: NSComboBox!) {

        let path = sender.stringValue
        pref.ffmpegPath = path
        refresh()
        
        // Display a warning if the recorder is inaccessible
        let fm = FileManager.default
        if fm.fileExists(atPath: path), !fm.isExecutableFile(atPath: path) {

            parent.showAlert(.recorderSandboxed(exec: path), window: window)
        }
    }
        
    @IBAction func capVideoSourceAction(_ sender: NSPopUpButton!) {
        
        pref.captureSourceIntValue = sender.selectedTag()
        refresh()
    }

    @IBAction func capVideoBitRateAction(_ sender: NSComboBox!) {
        
        var input = sender.objectValueOfSelectedItem as? Int
        if input == nil { input = sender.integerValue }
        
        if let bitrate = input {
            pref.bitRate = bitrate
        }
        refresh()
    }

    @IBAction func capVideoAspectXAction(_ sender: NSTextField!) {
        
        pref.aspectX = sender.integerValue
        refresh()
    }

    @IBAction func capVideoAspectYAction(_ sender: NSTextField!) {
        
        pref.aspectY = sender.integerValue
        refresh()
    }
    
    //
    // Action methods (Misc)
    //
    
    @IBAction func capturesPresetAction(_ sender: NSPopUpButton!) {
        
        assert(sender.selectedTag() == 0)
                        
        // Revert to standard settings
        EmulatorProxy.defaults.removeCapturesUserDefaults()
                        
        // Apply the new settings
        pref.applyCapturesUserDefaults()
        
        refresh()
    }
}
