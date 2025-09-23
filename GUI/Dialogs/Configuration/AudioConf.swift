// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/*
@MainActor
extension ConfigurationController {
    
    func refreshAudioTab() {
        
        //
        // Mixer
        //
        
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
        
        // Drives
        audStepVolume.integerValue = config.stepVolume
        audPollVolume.integerValue = config.pollVolume
        audInsertVolume.integerValue = config.insertVolume
        audEjectVolume.integerValue = config.ejectVolume
        audDf0Pan.integerValue = config.df0Pan
        audDf1Pan.integerValue = config.df1Pan
        audDf2Pan.integerValue = config.df2Pan
        audDf3Pan.integerValue = config.df3Pan
        audHd0Pan.integerValue = config.hd0Pan
        audHd1Pan.integerValue = config.hd1Pan
        audHd2Pan.integerValue = config.hd2Pan
        audHd3Pan.integerValue = config.hd3Pan
        
        // Audio filter
        audFilterType.selectItem(withTag: config.filterType)
        
        //
        // Sampler
        //
        
        audSamplingMethod.selectItem(withTag: config.samplingMethod)
        audASR.selectItem(withTag: config.asr)
        audCapacity.integerValue = config.audioBufferSize
        audCapacityText.stringValue = "\(config.audioBufferSize) samples"
        
        switch SamplingMethod(rawValue: Int32(config.samplingMethod)) {
        case .NONE:
            audSamplingMethodText.stringValue = "Instructs the sampler to select the most recent sample from the ring buffer. This minimizes latency but may introduce jitter when the sample rate fluctuates."
        case .NEAREST:
            audSamplingMethodText.stringValue = "Instructs the sampler to pick the sample closest to the target timestamp. It improves timing accuracy over the latest-sample method but may still have minor mismatches."
        case .LINEAR:
            audSamplingMethodText.stringValue = "Instructs the sampler to compute a value between two neighboring samples for smoother output. Increases computation slightly but reduces artifacts and improves fidelity."
        default:
            break
        }
        
        switch (config.asr) {
        case 0:
            audASRText.stringValue = "Audio samples are synthesized at a constant sampling rate, ignoring drift between emulated and real-time playback rates. This may cause buffer underflows and overflows over time, leading to audio stutter or glitches."
        default:
            audASRText.stringValue = "ASR (Adaptive Sample Rate) dynamically adjusts the sampling rate to maintain audio sync. This prevents buffer underflows and overflows by adapting to slight drift between emulated and real-time playback rates."
        }
        
        //
        // Commons
        //
        
        audPowerButton.isHidden = !bootable
    }

    //
    // Mixer
    //
    
    @IBAction func audVol0Action(_ sender: NSSlider!) {

        config.vol0 = sender.integerValue
    }
    
    @IBAction func audVol1Action(_ sender: NSSlider!) {

        config.vol1 = sender.integerValue
    }
    
    @IBAction func audVol2Action(_ sender: NSSlider!) {

        config.vol2 = sender.integerValue
    }
    
    @IBAction func audVol3Action(_ sender: NSSlider!) {

        config.vol3 = sender.integerValue
    }
    
    @IBAction func audPan0Action(_ sender: NSSlider!) {
        
        config.pan0 = sender.integerValue
    }
    
    @IBAction func audPan1Action(_ sender: NSSlider!) {
        
        config.pan1 = sender.integerValue
    }
    
    @IBAction func audPan2Action(_ sender: NSSlider!) {
        
        config.pan2 = sender.integerValue
    }
    
    @IBAction func audPan3Action(_ sender: NSSlider!) {
        
        config.pan3 = sender.integerValue
    }

    @IBAction func audVolLAction(_ sender: NSSlider!) {
        
        config.volL = sender.integerValue
    }

    @IBAction func audVolRAction(_ sender: NSSlider!) {
        
        config.volR = sender.integerValue
    }
    
    @IBAction func audDrivePanAction(_ sender: NSSlider!) {
                                
        switch sender.tag {
        case 0: config.df0Pan = sender.integerValue
        case 1: config.df1Pan = sender.integerValue
        case 2: config.df2Pan = sender.integerValue
        case 3: config.df3Pan = sender.integerValue
        default: fatalError()
        }
    }

    @IBAction func audHdPanAction(_ sender: NSSlider!) {
                                
        switch sender.tag {
        case 0: config.hd0Pan = sender.integerValue
        case 1: config.hd1Pan = sender.integerValue
        case 2: config.hd2Pan = sender.integerValue
        case 3: config.hd3Pan = sender.integerValue
        default: fatalError()
        }
    }

    @IBAction func audStepVolumeAction(_ sender: NSSlider!) {

        config.stepVolume = sender.integerValue
    }

    @IBAction func audPollVolumeAction(_ sender: NSSlider!) {

        config.pollVolume = sender.integerValue
    }

    @IBAction func audInsertVolumeAction(_ sender: NSSlider!) {

        config.insertVolume = sender.integerValue
    }

    @IBAction func audEjectVolumeAction(_ sender: NSSlider!) {

        config.ejectVolume = sender.integerValue
    }
     
    @IBAction func audFilterTypeAction(_ sender: NSPopUpButton!) {
        
        config.filterType = sender.selectedTag()
    }

    //
    // Sampler
    //
    
    @IBAction func audSamplingMethodAction(_ sender: NSPopUpButton!) {
        
        config.samplingMethod = sender.selectedTag()
    }
    
    @IBAction func audASRAction(_ sender: NSPopUpButton!) {
        
        config.asr = sender.selectedTag()
    }

    @IBAction func audCapacityAction(_ sender: NSSlider!) {

        print("Value: \(sender.integerValue)")
        
        config.audioBufferSize = sender.integerValue
    }
    
    @IBAction func audPresetAction(_ sender: NSPopUpButton!) {
        
        emu.suspend()

        // Revert to standard settings
        EmulatorProxy.defaults.removeAudioUserDefaults()

        // Update the configuration
        config.applyAudioUserDefaults()

        switch sender.selectedTag() {

        case 0: // Standard
            config.pan0 = 50
            config.pan1 = 350
            config.pan2 = 350
            config.pan3 = 50

        case 1: // Stereo
            config.pan0 = 100
            config.pan1 = 300
            config.pan2 = 300
            config.pan3 = 100

        case 2: // Mono
            config.pan0 = 0
            config.pan1 = 0
            config.pan2 = 0
            config.pan3 = 0

        default:
            fatalError()
        }
        
        emu.resume()
    }
    
    @IBAction func audDefaultsAction(_ sender: NSButton!) {
        
        config.saveAudioUserDefaults()
    }
}
*/
