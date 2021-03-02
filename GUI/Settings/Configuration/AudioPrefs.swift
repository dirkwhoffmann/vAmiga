// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ConfigurationController {
    
    func refreshAudioTab() {
        
        // let config = amiga.config.audio
        
        // In
        audVol0.integerValue = config.vol0
        audVol1.integerValue = config.vol1
        audVol2.integerValue = config.vol2
        audVol3.integerValue = config.vol3
        audPan0.integerValue = config.pan0
        audPan1.integerValue = config.pan1
        audPan2.integerValue = config.pan2
        audPan3.integerValue = config.pan3

        // Out
        audVolL.integerValue = config.volL
        audVolR.integerValue = config.volR
        audSamplingMethod.selectItem(withTag: config.samplingMethod)

        // Drives
        audDf0Pan.integerValue = config.df0Pan
        audDf1Pan.integerValue = config.df1Pan
        audDf2Pan.integerValue = config.df2Pan
        audDf3Pan.integerValue = config.df3Pan
        audStepVolume.integerValue = config.stepVolume
        audPollVolume.integerValue = config.pollVolume
        audInsertVolume.integerValue = config.insertVolume
        audEjectVolume.integerValue = config.ejectVolume
        
        // Buttons
        audPowerButton.isHidden = !bootable
    }

    @IBAction func audVol0Action(_ sender: NSSlider!) {

        config.vol0 = sender.integerValue
        refresh()
    }
    
    @IBAction func audVol1Action(_ sender: NSSlider!) {

        config.vol1 = sender.integerValue
        refresh()
    }
    
    @IBAction func audVol2Action(_ sender: NSSlider!) {

        config.vol2 = sender.integerValue
        refresh()
    }
    
    @IBAction func audVol3Action(_ sender: NSSlider!) {

        config.vol3 = sender.integerValue
        refresh()
    }
    
    @IBAction func audPan0Action(_ sender: NSSlider!) {
        
        config.pan0 = sender.integerValue
        refresh()
    }
    
    @IBAction func audPan1Action(_ sender: NSSlider!) {
        
        config.pan1 = sender.integerValue
        refresh()
    }
    
    @IBAction func audPan2Action(_ sender: NSSlider!) {
        
        config.pan2 = sender.integerValue
        refresh()
    }
    
    @IBAction func audPan3Action(_ sender: NSSlider!) {
        
        config.pan3 = sender.integerValue
        refresh()
    }

    @IBAction func audVolLAction(_ sender: NSSlider!) {
        
        config.volL = sender.integerValue
        refresh()
    }

    @IBAction func audVolRAction(_ sender: NSSlider!) {
        
        config.volR = sender.integerValue
        refresh()
    }

    @IBAction func audSamplingMethodAction(_ sender: NSPopUpButton!) {
        
        config.samplingMethod = sender.selectedTag()
        refresh()
    }
    
    @IBAction func audDrivePanAction(_ sender: NSSlider!) {
                                
        switch sender.tag {
        case 0: config.df0Pan = sender.integerValue
        case 1: config.df1Pan = sender.integerValue
        case 2: config.df2Pan = sender.integerValue
        case 3: config.df3Pan = sender.integerValue
        default: fatalError()
        }
        refresh()
    }

    @IBAction func audStepVolumeAction(_ sender: NSSlider!) {

        config.stepVolume = sender.integerValue
        refresh()
    }

    @IBAction func audPollVolumeAction(_ sender: NSSlider!) {

        config.pollVolume = sender.integerValue
        refresh()
    }

    @IBAction func audInsertVolumeAction(_ sender: NSSlider!) {

        config.insertVolume = sender.integerValue
        refresh()
    }

    @IBAction func audEjectVolumeAction(_ sender: NSSlider!) {

        config.ejectVolume = sender.integerValue
        refresh()
    }
        
    @IBAction func audPresetAction(_ sender: NSPopUpButton!) {
        
        track()
        
        switch sender.selectedTag() {
        case 0: config.loadAudioDefaults(AudioDefaults.std)
        case 1: config.loadAudioDefaults(AudioDefaults.stereo)
        case 2: config.loadAudioDefaults(AudioDefaults.mono)
        default: fatalError()
        }
        refresh()
    }
    
    @IBAction func audDefaultsAction(_ sender: NSButton!) {
        
        track()
        config.saveAudioUserDefaults()
    }
}
