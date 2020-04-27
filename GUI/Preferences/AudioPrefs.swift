// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ConfigController {
    
    func refreshAudioTab() {
        
        track()
        let config = amiga.config().audio
        
        // In
        audVol0.doubleValue = config.vol.0 * 100 / 0.0000025
        audVol1.doubleValue = config.vol.1 * 100 / 0.0000025
        audVol2.doubleValue = config.vol.2 * 100 / 0.0000025
        audVol3.doubleValue = config.vol.3 * 100 / 0.0000025
        audPan0.doubleValue = config.pan.0 * 100
        audPan1.doubleValue = config.pan.1 * 100
        audPan2.doubleValue = config.pan.2 * 100
        audPan3.doubleValue = config.pan.3 * 100

        // Out
        audVolL.doubleValue = config.volL * 100
        audVolR.doubleValue = config.volR * 100
        audSamplingMethod.selectItem(withTag: config.samplingMethod.rawValue)
        audFilterType.selectItem(withTag: config.filterType.rawValue)
        audFilterAlwaysOn.state = config.filterAlwaysOn ? .on : .off
        audFilterAlwaysOn.isEnabled = config.filterType != FILT_NONE
    }

    @IBAction func audVol0Action(_ sender: NSSlider!) {

        track()
        config.vol0 = (sender.doubleValue / 100.0) * 0.0000025
        refresh()
    }
    
    @IBAction func audVol1Action(_ sender: NSSlider!) {

        track()
        config.vol1 = (sender.doubleValue / 100.0) * 0.0000025
        refresh()
    }
    
    @IBAction func audVol2Action(_ sender: NSSlider!) {

        track()
        config.vol2 = (sender.doubleValue / 100.0) * 0.0000025
        refresh()
    }
    
    @IBAction func audVol3Action(_ sender: NSSlider!) {

        track()
        config.vol3 = (sender.doubleValue / 100.0) * 0.0000025
        refresh()
    }
    
    @IBAction func audPan0Action(_ sender: NSSlider!) {
        
        track()
        config.pan0 = sender.doubleValue / 100.0
        refresh()
    }
    
    @IBAction func audPan1Action(_ sender: NSSlider!) {
        
        track()
        config.pan1 = sender.doubleValue / 100.0
        refresh()
    }
    
    @IBAction func audPan2Action(_ sender: NSSlider!) {
        
        track()
        config.pan2 = sender.doubleValue / 100.0
        refresh()
    }
    
    @IBAction func audPan3Action(_ sender: NSSlider!) {
        
        track()
        config.pan3 = sender.doubleValue / 100.0
        refresh()
    }

    @IBAction func audVolLAction(_ sender: NSSlider!) {
        
        track()
        config.volL = sender.doubleValue / 100.0
        refresh()
    }

    @IBAction func audVolRAction(_ sender: NSSlider!) {
        
        track()
        config.volR = sender.doubleValue / 100.0
        refresh()
    }

    @IBAction func audSamplingMethodAction(_ sender: NSPopUpButton!) {
        
        config.samplingMethod = sender.selectedTag()
        refresh()
    }
    
    @IBAction func audFilterTypeAction(_ sender: NSPopUpButton!) {
        
        config.filterType = sender.selectedTag()
        refresh()
    }

    @IBAction func audFilterAlwaysOnAction(_ sender: NSButton!) {

        config.filterAlwaysOn = sender.state == .on
        refresh()
    }
    
    @IBAction func audPresetAction(_ sender: NSPopUpButton!) {
        
        track()
        
        switch sender.selectedTag() {
        case 0: config.loadAudioDefaults(AudioDefaults.std)
        default: fatalError()
        }
        refresh()
    }
    
    @IBAction func audDefaultsAction(_ sender: NSButton!) {
        
        track()
        config.saveAudioUserDefaults()
    }
}
