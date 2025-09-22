// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class AudioSettingsViewController: SettingsViewController {

    // In
    @IBOutlet weak var vol0: NSSlider!
    @IBOutlet weak var vol1: NSSlider!
    @IBOutlet weak var vol2: NSSlider!
    @IBOutlet weak var vol3: NSSlider!
    @IBOutlet weak var pan0: NSSlider!
    @IBOutlet weak var pan1: NSSlider!
    @IBOutlet weak var pan2: NSSlider!
    @IBOutlet weak var pan3: NSSlider!

    // Out
    @IBOutlet weak var volL: NSSlider!
    @IBOutlet weak var volR: NSSlider!

    // Drive volumes
    @IBOutlet weak var stepVolume: NSSlider!
    @IBOutlet weak var pollVolume: NSSlider!
    @IBOutlet weak var ejectVolume: NSSlider!
    @IBOutlet weak var insertVolume: NSSlider!
    @IBOutlet weak var df0Pan: NSSlider!
    @IBOutlet weak var df1Pan: NSSlider!
    @IBOutlet weak var df2Pan: NSSlider!
    @IBOutlet weak var df3Pan: NSSlider!
    @IBOutlet weak var hd0Pan: NSSlider!
    @IBOutlet weak var hd1Pan: NSSlider!
    @IBOutlet weak var hd2Pan: NSSlider!
    @IBOutlet weak var hd3Pan: NSSlider!

    // Filter
    @IBOutlet weak var filterType: NSPopUpButton!

    // Mixer
    @IBOutlet weak var samplingMethod: NSPopUpButton!
    @IBOutlet weak var samplingMethodHelp: NSButton!
    @IBOutlet weak var ASR: NSPopUpButton!
    @IBOutlet weak var ASRHelp: NSButton!
    @IBOutlet weak var capacity: NSSlider!
    @IBOutlet weak var capacityText: NSTextField!

    override func viewDidLoad() {

        log(.lifetime)
    }

    //
    // Methods from SettingsViewController
    //

    override func refresh() {

        super.refresh()

        guard let config = config else { return }
        
        //
        // Mixer
        //

        // In
        vol0.integerValue = config.vol0
        vol1.integerValue = config.vol1
        vol2.integerValue = config.vol2
        vol3.integerValue = config.vol3
        pan0.integerValue = config.pan0
        pan1.integerValue = config.pan1
        pan2.integerValue = config.pan2
        pan3.integerValue = config.pan3

        // Out
        volL.integerValue = config.volL
        volR.integerValue = config.volR

        // Drives
        stepVolume.integerValue = config.stepVolume
        pollVolume.integerValue = config.pollVolume
        insertVolume.integerValue = config.insertVolume
        ejectVolume.integerValue = config.ejectVolume
        df0Pan.integerValue = config.df0Pan
        df1Pan.integerValue = config.df1Pan
        df2Pan.integerValue = config.df2Pan
        df3Pan.integerValue = config.df3Pan
        hd0Pan.integerValue = config.hd0Pan
        hd1Pan.integerValue = config.hd1Pan
        hd2Pan.integerValue = config.hd2Pan
        hd3Pan.integerValue = config.hd3Pan

        // Audio filter
        filterType.selectItem(withTag: config.filterType)

        //
        // Sampler
        //

        samplingMethod.selectItem(withTag: config.samplingMethod)
        ASR.selectItem(withTag: config.asr)
        capacity.integerValue = config.audioBufferSize
        capacityText.stringValue = "\(config.audioBufferSize) samples"

        switch SamplingMethod(rawValue: Int32(config.samplingMethod)) {
        case .NONE:
            samplingMethodHelp.toolTip = "Instructs the sampler to select the most recent sample from the ring buffer. This minimizes latency but may introduce jitter when the sample rate fluctuates."
        case .NEAREST:
            samplingMethodHelp.toolTip = "Instructs the sampler to pick the sample closest to the target timestamp. It improves timing accuracy over the latest-sample method but may still have minor mismatches."
        case .LINEAR:
            samplingMethodHelp.toolTip = "Instructs the sampler to compute a value between two neighboring samples for smoother output. Increases computation slightly but reduces artifacts and improves fidelity."
        default:
            break
        }

        switch (config.asr) {
        case 0:
            ASRHelp.toolTip = "Audio samples are synthesized at a constant sampling rate, ignoring drift between emulated and real-time playback rates. This may cause buffer underflows and overflows over time, leading to audio stutter or glitches."
        default:
            ASRHelp.toolTip = "ASR (Adaptive Sample Rate) dynamically adjusts the sampling rate to maintain audio sync. This prevents buffer underflows and overflows by adapting to slight drift between emulated and real-time playback rates."
        }
    }

    override func preset(tag: Int) {

        guard let emu = emu, let config = config else { return }

        emu.suspend()

        // Revert to standard settings
        EmulatorProxy.defaults.removeAudioUserDefaults()

        // Update the configuration
        config.applyAudioUserDefaults()

        switch tag {

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

    override func save() {

        config?.saveAudioUserDefaults()
    }

    //
    // Action functions (Mixer)
    //

    @IBAction func volAction(_ sender: NSSlider!) {

        switch sender.tag {
        case 0: config?.vol0 = sender.integerValue
        case 1: config?.vol1 = sender.integerValue
        case 2: config?.vol2 = sender.integerValue
        case 3: config?.vol3 = sender.integerValue
        default: fatalError()
        }
    }

    @IBAction func panAction(_ sender: NSSlider!) {

        switch sender.tag {
        case 0: config?.pan0 = sender.integerValue
        case 1: config?.pan1 = sender.integerValue
        case 2: config?.pan2 = sender.integerValue
        case 3: config?.pan3 = sender.integerValue
        default: fatalError()
        }
    }

    @IBAction func volLAction(_ sender: NSSlider!) {

        config?.volL = sender.integerValue
    }

    @IBAction func volRAction(_ sender: NSSlider!) {

        config?.volR = sender.integerValue
    }

    @IBAction func dfPanAction(_ sender: NSSlider!) {

        switch sender.tag {
        case 0: config?.df0Pan = sender.integerValue
        case 1: config?.df1Pan = sender.integerValue
        case 2: config?.df2Pan = sender.integerValue
        case 3: config?.df3Pan = sender.integerValue
        default: fatalError()
        }
    }

    @IBAction func hdPanAction(_ sender: NSSlider!) {

        switch sender.tag {
        case 0: config?.hd0Pan = sender.integerValue
        case 1: config?.hd1Pan = sender.integerValue
        case 2: config?.hd2Pan = sender.integerValue
        case 3: config?.hd3Pan = sender.integerValue
        default: fatalError()
        }
    }

    @IBAction func stepVolumeAction(_ sender: NSSlider!) {

        config?.stepVolume = sender.integerValue
    }

    @IBAction func pollVolumeAction(_ sender: NSSlider!) {

        config?.pollVolume = sender.integerValue
    }

    @IBAction func insertVolumeAction(_ sender: NSSlider!) {

        config?.insertVolume = sender.integerValue
    }

    @IBAction func ejectVolumeAction(_ sender: NSSlider!) {

        config?.ejectVolume = sender.integerValue
    }

    @IBAction func filterTypeAction(_ sender: NSPopUpButton!) {

        config?.filterType = sender.selectedTag()
    }

    //
    // Action functions (Sampler)
    //

    @IBAction func samplingMethodAction(_ sender: NSPopUpButton!) {

        config?.samplingMethod = sender.selectedTag()
    }

    @IBAction func ASRAction(_ sender: NSPopUpButton!) {

        config?.asr = sender.selectedTag()
    }

    @IBAction func capacityAction(_ sender: NSSlider!) {

        config?.audioBufferSize = sender.integerValue
    }
}
