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
    @IBOutlet weak var audVol0: NSSlider!
    @IBOutlet weak var audVol1: NSSlider!
    @IBOutlet weak var audVol2: NSSlider!
    @IBOutlet weak var audVol3: NSSlider!
    @IBOutlet weak var audPan0: NSSlider!
    @IBOutlet weak var audPan1: NSSlider!
    @IBOutlet weak var audPan2: NSSlider!
    @IBOutlet weak var audPan3: NSSlider!

    // Out
    @IBOutlet weak var audVolL: NSSlider!
    @IBOutlet weak var audVolR: NSSlider!

    // Drive volumes
    @IBOutlet weak var audStepVolume: NSSlider!
    @IBOutlet weak var audPollVolume: NSSlider!
    @IBOutlet weak var audEjectVolume: NSSlider!
    @IBOutlet weak var audInsertVolume: NSSlider!
    @IBOutlet weak var audDf0Pan: NSSlider!
    @IBOutlet weak var audDf1Pan: NSSlider!
    @IBOutlet weak var audDf2Pan: NSSlider!
    @IBOutlet weak var audDf3Pan: NSSlider!
    @IBOutlet weak var audHd0Pan: NSSlider!
    @IBOutlet weak var audHd1Pan: NSSlider!
    @IBOutlet weak var audHd2Pan: NSSlider!
    @IBOutlet weak var audHd3Pan: NSSlider!

    // Filter
    @IBOutlet weak var audFilterType: NSPopUpButton!

    // Mixer
    @IBOutlet weak var audSamplingMethod: NSPopUpButton!
    @IBOutlet weak var audSamplingMethodText: NSTextField!
    @IBOutlet weak var audASR: NSPopUpButton!
    @IBOutlet weak var audASRText: NSTextField!
    @IBOutlet weak var audCapacity: NSSlider!
    @IBOutlet weak var audCapacityText: NSTextField!

    // Buttons
    @IBOutlet weak var audOKButton: NSButton!
    @IBOutlet weak var audPowerButton: NSButton!
    
    override func viewDidLoad() {

        log(.lifetime)
    }

    //
    // Methods from SettingsViewController
    //

    override func refresh() {

        super.refresh()
    }

    override func preset(tag: Int) {

    }

    override func save() {

    }
}
