// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class VideoSettingsViewController: SettingsViewController {

    // Palette
    @IBOutlet weak var vidPalettePopUp: NSPopUpButton!
    @IBOutlet weak var vidBrightnessSlider: NSSlider!
    @IBOutlet weak var vidContrastSlider: NSSlider!
    @IBOutlet weak var vidSaturationSlider: NSSlider!
    @IBOutlet weak var vidBrightnessLabel: NSTextField!
    @IBOutlet weak var vidContrastLabel: NSTextField!
    @IBOutlet weak var vidSaturationLabel: NSTextField!

    // Geometry
    @IBOutlet weak var vidZoom: NSPopUpButton!
    @IBOutlet weak var vidHZoom: NSSlider!
    @IBOutlet weak var vidVZoom: NSSlider!
    @IBOutlet weak var vidHZoomLabel: NSTextField!
    @IBOutlet weak var vidVZoomLabel: NSTextField!
    @IBOutlet weak var vidCenter: NSPopUpButton!
    @IBOutlet weak var vidHCenter: NSSlider!
    @IBOutlet weak var vidVCenter: NSSlider!
    @IBOutlet weak var vidHCenterLabel: NSTextField!
    @IBOutlet weak var vidVCenterLabel: NSTextField!

    // Frame rate
    @IBOutlet weak var vidSyncMode: NSPopUpButton!
    @IBOutlet weak var vidFpsSlider: NSSlider!
    @IBOutlet weak var vidFpsMin: NSTextField!
    @IBOutlet weak var vidFpsMax: NSTextField!

    // Effects
    @IBOutlet weak var vidEnhancerPopUp: NSPopUpButton!
    @IBOutlet weak var vidUpscalerPopUp: NSPopUpButton!
    @IBOutlet weak var vidBlurPopUp: NSPopUpButton!
    @IBOutlet weak var vidBlurRadiusSlider: NSSlider!

    @IBOutlet weak var vidBloomPopUp: NSPopUpButton!
    @IBOutlet weak var vidBloomRadiusSlider: NSSlider!
    @IBOutlet weak var vidBloomBrightnessSlider: NSSlider!
    @IBOutlet weak var vidBloomWeightSlider: NSSlider!

    @IBOutlet weak var vidFlickerPopUp: NSPopUpButton!
    @IBOutlet weak var vidFlickerWeightSlider: NSSlider!

    @IBOutlet weak var vidDotMaskPopUp: NSPopUpButton!
    @IBOutlet weak var vidDotMaskBrightnessSlider: NSSlider!

    @IBOutlet weak var vidScanlinesPopUp: NSPopUpButton!
    @IBOutlet weak var vidScanlineBrightnessSlider: NSSlider!
    @IBOutlet weak var vidScanlineWeightSlider: NSSlider!

    @IBOutlet weak var vidMisalignmentPopUp: NSPopUpButton!
    @IBOutlet weak var vidMisalignmentXSlider: NSSlider!
    @IBOutlet weak var vidMisalignmentYSlider: NSSlider!

    // Buttons
    @IBOutlet weak var vidOKButton: NSButton!
    @IBOutlet weak var vidPowerButton: NSButton!
    
    override func viewDidLoad() {

        print("VideoSettingsViewController::viewDidLoad")
    }

    //
    // Refresh
    //

    override func refresh() {

    }

    //
    // Action methods
    //

    override func preset(tag: Int) {

        print("VideoSettingsViewController::preset")
    }

    override func save() {

        print("VideoSettingsViewController::preset")
    }
}
