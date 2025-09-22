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

        log(.lifetime)
        initVideoTab()
    }

    func initVideoTab() {

        guard let controller = controller else { fatalError() }

        // Check for available enhancers
        let enhancers = controller.renderer.ressourceManager.enhancerGallery
        for i in 0 ..< enhancers.count {
            if let item = vidEnhancerPopUp.menu?.item(withTag: i) {
                item.isEnabled = (enhancers[i] != nil)
            }
        }

        // Check for available upscalers
        let upscalers = controller.renderer.ressourceManager.upscalerGallery
        for i in 0 ..< upscalers.count {
            if let item = vidUpscalerPopUp.menu?.item(withTag: i) {
                item.isEnabled = (upscalers[i] != nil)
            }
        }

        // Update dot mask textures
        controller.renderer.ressourceManager.buildDotMasks()
    }

    //
    // Methods from SettingsViewController
    //

    override func refresh() {

        super.refresh()

        guard let config = config, let renderer = controller?.renderer else { return }

        let palette = config.palette
        let adjustable = palette != Palette.RGB.rawValue

        // Colors
        vidPalettePopUp.selectItem(withTag: palette)
        vidBrightnessSlider.integerValue = config.brightness
        vidContrastSlider.integerValue = config.contrast
        vidSaturationSlider.integerValue = config.saturation
        vidBrightnessSlider.isEnabled = adjustable
        vidContrastSlider.isEnabled = adjustable
        vidSaturationSlider.isEnabled = adjustable
        vidBrightnessLabel.textColor = adjustable ? .labelColor : .disabledControlTextColor
        vidContrastLabel.textColor = adjustable ? .labelColor : .disabledControlTextColor
        vidSaturationLabel.textColor = adjustable ? .labelColor : .disabledControlTextColor

        // Geometry
        vidZoom.selectItem(withTag: config.zoom)
        vidHZoom.integerValue = config.hZoom
        vidVZoom.integerValue = config.vZoom
        vidHZoom.isEnabled = config.zoom == 0
        vidVZoom.isEnabled = config.zoom == 0
        vidHZoomLabel.textColor = config.zoom == 0 ? .labelColor : .disabledControlTextColor
        vidVZoomLabel.textColor = config.zoom == 0 ? .labelColor : .disabledControlTextColor
        vidCenter.selectItem(withTag: config.center)
        vidHCenter.integerValue = config.hCenter
        vidVCenter.integerValue = config.vCenter
        vidHCenter.isEnabled = config.center == 0
        vidVCenter.isEnabled = config.center == 0
        vidHCenterLabel.textColor = config.center == 0 ? .labelColor : .disabledControlTextColor
        vidVCenterLabel.textColor = config.center == 0 ? .labelColor : .disabledControlTextColor

        // Upscalers
        vidEnhancerPopUp.selectItem(withTag: config.enhancer)
        vidUpscalerPopUp.selectItem(withTag: config.upscaler)

        // Effects
        vidBlurPopUp.selectItem(withTag: Int(config.blur))
        vidBlurRadiusSlider.integerValue = config.blurRadius
        vidBlurRadiusSlider.isEnabled = config.blur > 0

        vidBloomPopUp.selectItem(withTag: Int(config.bloom))
        vidBloomRadiusSlider.integerValue = config.bloomRadius
        vidBloomRadiusSlider.isEnabled = config.bloom > 0
        vidBloomBrightnessSlider.integerValue = config.bloomBrightness
        vidBloomBrightnessSlider.isEnabled = config.bloom > 0
        vidBloomWeightSlider.integerValue = config.bloomWeight
        vidBloomWeightSlider.isEnabled = config.bloom > 0

        vidFlickerPopUp.selectItem(withTag: Int(config.flicker))
        vidFlickerWeightSlider.integerValue = config.flickerWeight
        vidFlickerWeightSlider.isEnabled = config.flicker > 0

        vidDotMaskPopUp.selectItem(withTag: Int(config.dotMask))
        for i in 0 ... 4 {
            vidDotMaskPopUp.item(at: i)?.image = renderer.ressourceManager.dotmaskImages[i]
        }
        vidDotMaskBrightnessSlider.integerValue = config.dotMaskBrightness
        vidDotMaskBrightnessSlider.isEnabled = config.dotMask > 0

        vidScanlinesPopUp.selectItem(withTag: Int(config.scanlines))
        vidScanlineBrightnessSlider.integerValue = config.scanlineBrightness
        vidScanlineBrightnessSlider.isEnabled = config.scanlines > 0
        vidScanlineWeightSlider.integerValue = config.scanlineWeight
        vidScanlineWeightSlider.isEnabled = config.scanlines == 2

        vidMisalignmentPopUp.selectItem(withTag: Int(config.disalignment))
        vidMisalignmentXSlider.integerValue = config.disalignmentH
        vidMisalignmentXSlider.isEnabled = config.disalignment > 0
        vidMisalignmentYSlider.integerValue = config.disalignmentV
        vidMisalignmentYSlider.isEnabled = config.disalignment > 0
    }

    override func preset(tag: Int) {

        let defaults = EmulatorProxy.defaults!

        switch tag {

        case 0: // Recommended settings (all)

            EmulatorProxy.defaults.removeVideoUserDefaults()

        case 10: // Recommended settings (geometry)

            EmulatorProxy.defaults.removeGeometryUserDefaults()

        case 11: // My personal monitor (ViewSonic VP191b)

            debug(1, "ViewSonic VP191b")
            EmulatorProxy.defaults.removeGeometryUserDefaults()
            defaults.set(.MON_ZOOM, 0)          // (Keys.Vid.zoom, 0)
            defaults.set(.MON_HZOOM, 676)       // (Keys.Vid.hZoom, 0.6763221)
            defaults.set(.MON_VZOOM, 032)       // (Keys.Vid.vZoom, 0.032)
            defaults.set(.MON_CENTER, 0)        // (Keys.Vid.center, 0)
            defaults.set(.MON_HCENTER, 398)     // (Keys.Vid.hCenter, 0.39813587)
            defaults.set(.MON_VCENTER, 1000)    // (Keys.Vid.vCenter, 1.0)

        case 20: // Recommended settings (colors + shader)

            EmulatorProxy.defaults.removeColorUserDefaults()
            EmulatorProxy.defaults.removeShaderUserDefaults()

        case 21: // TFT monitor

            EmulatorProxy.defaults.removeColorUserDefaults()
            EmulatorProxy.defaults.removeShaderUserDefaults()

        case 22: // CRT monitor

            EmulatorProxy.defaults.removeColorUserDefaults()
            EmulatorProxy.defaults.removeShaderUserDefaults()
            defaults.set(.MON_BLUR_RADIUS, 330) // (Keys.Vid.blurRadius, 1.5)
            defaults.set(.MON_BLOOM, 1)         // (Keys.Vid.bloom, 1)
            defaults.set(.MON_DOTMASK, 1)       // (Keys.Vid.dotMask, 1)
            defaults.set(.MON_SCANLINES, 2)     // (Keys.Vid.scanlines, 2)

        default:
            fatalError()
        }

        config?.applyVideoUserDefaults()
    }

    override func save() {

        config?.saveVideoUserDefaults()
    }

    //
    // Action methods (Colors)
    //

    @IBAction func vidPaletteAction(_ sender: NSPopUpButton!) {

        config?.palette = sender.selectedTag()
    }

    @IBAction func vidBrightnessAction(_ sender: NSSlider!) {

        config?.brightness = sender.integerValue
    }

    @IBAction func vidContrastAction(_ sender: NSSlider!) {

        config?.contrast = sender.integerValue
    }

    @IBAction func vidSaturationAction(_ sender: NSSlider!) {

        config?.saturation = sender.integerValue
    }

    //
    // Action methods (Geometry)
    //

    @IBAction func vidZoomAction(_ sender: NSPopUpButton) {

        config?.zoom = sender.selectedTag()
        refresh()
    }

    @IBAction func vidHZoomAction(_ sender: NSSlider!) {

        config?.hZoom = sender.integerValue
        refresh()
    }

    @IBAction func vidVZoomAction(_ sender: NSSlider!) {

        config?.vZoom = sender.integerValue
        refresh()
    }

    @IBAction func vidCenterAction(_ sender: NSPopUpButton) {

        config?.center = sender.selectedTag()
        refresh()
    }

    @IBAction func vidHCenterAction(_ sender: NSSlider!) {

        config?.hCenter = sender.integerValue
        refresh()
    }

    @IBAction func vidVCenterAction(_ sender: NSSlider!) {

        config?.vCenter = sender.integerValue
        refresh()
    }

    //
    // Action methods (Effects)
    //

    @IBAction func vidEnhancerAction(_ sender: NSPopUpButton!) {

        config?.enhancer = sender.selectedTag()
        refresh()
    }

    @IBAction func vidUpscalerAction(_ sender: NSPopUpButton!) {

        config?.upscaler = sender.selectedTag()
        refresh()
    }

    @IBAction func vidBlurAction(_ sender: NSPopUpButton!) {

        config?.blur = sender.selectedTag()
        refresh()
    }

    @IBAction func vidBlurRadiusAction(_ sender: NSSlider!) {

        config?.blurRadius = sender.integerValue
        refresh()
    }

    @IBAction func vidBloomAction(_ sender: NSPopUpButton!) {

        config?.bloom = sender.selectedTag()
        refresh()
    }

    @IBAction func vidBloomRadiusAction(_ sender: NSSlider!) {

        config?.bloomRadius = sender.integerValue
        refresh()
    }

    @IBAction func vidBloomBrightnessAction(_ sender: NSSlider!) {

        config?.bloomBrightness = sender.integerValue
        refresh()
    }

    @IBAction func vidBloomWeightAction(_ sender: NSSlider!) {

        config?.bloomWeight = sender.integerValue
        refresh()
    }

    @IBAction func vidFlickerAction(_ sender: NSPopUpButton!) {

        config?.flicker = sender.selectedTag()
        refresh()
    }

    @IBAction func vidFlickerWeightAction(_ sender: NSSlider!) {

        config?.flickerWeight = sender.integerValue
        refresh()
    }

    @IBAction func vidDotMaskAction(_ sender: NSPopUpButton!) {

        config?.dotMask = sender.selectedTag()
        refresh()
    }

    @IBAction func vidDotMaskBrightnessAction(_ sender: NSSlider!) {

        config?.dotMaskBrightness = sender.integerValue
        refresh()
    }

    @IBAction func vidScanlinesAction(_ sender: NSPopUpButton!) {

        config?.scanlines = sender.selectedTag()
        refresh()
    }

    @IBAction func vidScanlineBrightnessAction(_ sender: NSSlider!) {

        config?.scanlineBrightness = sender.integerValue
        refresh()
    }

    @IBAction func vidScanlineWeightAction(_ sender: NSSlider!) {

        config?.scanlineWeight = sender.integerValue
        refresh()
    }

    @IBAction func vidDisalignmentAction(_ sender: NSPopUpButton!) {

        config?.disalignment = sender.selectedTag()
        refresh()
    }

    @IBAction func vidDisalignmentHAction(_ sender: NSSlider!) {

        config?.disalignmentH = sender.integerValue
        refresh()
    }

    @IBAction func vidDisalignmentVAction(_ sender: NSSlider!) {

        config?.disalignmentV = sender.integerValue
        refresh()
    }
}
