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
    @IBOutlet weak var palettePopUp: NSPopUpButton!
    @IBOutlet weak var brightnessSlider: NSSlider!
    @IBOutlet weak var contrastSlider: NSSlider!
    @IBOutlet weak var saturationSlider: NSSlider!
    @IBOutlet weak var brightnessLabel: NSTextField!
    @IBOutlet weak var contrastLabel: NSTextField!
    @IBOutlet weak var saturationLabel: NSTextField!

    // Geometry
    @IBOutlet weak var zoom: NSPopUpButton!
    @IBOutlet weak var hZoom: NSSlider!
    @IBOutlet weak var vZoom: NSSlider!
    @IBOutlet weak var hZoomLabel: NSTextField!
    @IBOutlet weak var vZoomLabel: NSTextField!
    @IBOutlet weak var center: NSPopUpButton!
    @IBOutlet weak var hCenter: NSSlider!
    @IBOutlet weak var vCenter: NSSlider!
    @IBOutlet weak var hCenterLabel: NSTextField!
    @IBOutlet weak var vCenterLabel: NSTextField!

    // Frame rate
    @IBOutlet weak var syncMode: NSPopUpButton!
    @IBOutlet weak var fpsSlider: NSSlider!
    @IBOutlet weak var fpsMin: NSTextField!
    @IBOutlet weak var fpsMax: NSTextField!

    // Effects
    @IBOutlet weak var enhancerPopUp: NSPopUpButton!
    @IBOutlet weak var upscalerPopUp: NSPopUpButton!
    @IBOutlet weak var blurPopUp: NSPopUpButton!
    @IBOutlet weak var blurRadiusSlider: NSSlider!

    @IBOutlet weak var bloomPopUp: NSPopUpButton!
    @IBOutlet weak var bloomRadiusSlider: NSSlider!
    @IBOutlet weak var bloomBrightnessSlider: NSSlider!
    @IBOutlet weak var bloomWeightSlider: NSSlider!

    @IBOutlet weak var flickerPopUp: NSPopUpButton!
    @IBOutlet weak var flickerWeightSlider: NSSlider!

    @IBOutlet weak var dotMaskPopUp: NSPopUpButton!
    @IBOutlet weak var dotMaskBrightnessSlider: NSSlider!

    @IBOutlet weak var scanlinesPopUp: NSPopUpButton!
    @IBOutlet weak var scanlineBrightnessSlider: NSSlider!
    @IBOutlet weak var scanlineWeightSlider: NSSlider!

    @IBOutlet weak var misalignmentPopUp: NSPopUpButton!
    @IBOutlet weak var misalignmentXSlider: NSSlider!
    @IBOutlet weak var misalignmentYSlider: NSSlider!
    
    override func viewDidLoad() {

        log(.lifetime)
        initVideoTab()
    }

    func initVideoTab() {

        guard let controller = controller else { fatalError() }

        // Check for available enhancers
        let enhancers = controller.renderer.ressourceManager.enhancerGallery
        for i in 0 ..< enhancers.count {
            if let item = enhancerPopUp.menu?.item(withTag: i) {
                item.isEnabled = (enhancers[i] != nil)
            }
        }

        // Check for available upscalers
        let upscalers = controller.renderer.ressourceManager.upscalerGallery
        for i in 0 ..< upscalers.count {
            if let item = upscalerPopUp.menu?.item(withTag: i) {
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
        palettePopUp.selectItem(withTag: palette)
        brightnessSlider.integerValue = config.brightness
        contrastSlider.integerValue = config.contrast
        saturationSlider.integerValue = config.saturation
        brightnessSlider.isEnabled = adjustable
        contrastSlider.isEnabled = adjustable
        saturationSlider.isEnabled = adjustable
        brightnessLabel.textColor = adjustable ? .labelColor : .disabledControlTextColor
        contrastLabel.textColor = adjustable ? .labelColor : .disabledControlTextColor
        saturationLabel.textColor = adjustable ? .labelColor : .disabledControlTextColor

        // Geometry
        zoom.selectItem(withTag: config.zoom)
        hZoom.integerValue = config.hZoom
        vZoom.integerValue = config.vZoom
        hZoom.isEnabled = config.zoom == 0
        vZoom.isEnabled = config.zoom == 0
        hZoomLabel.textColor = config.zoom == 0 ? .labelColor : .disabledControlTextColor
        vZoomLabel.textColor = config.zoom == 0 ? .labelColor : .disabledControlTextColor
        center.selectItem(withTag: config.center)
        hCenter.integerValue = config.hCenter
        vCenter.integerValue = config.vCenter
        hCenter.isEnabled = config.center == 0
        vCenter.isEnabled = config.center == 0
        hCenterLabel.textColor = config.center == 0 ? .labelColor : .disabledControlTextColor
        vCenterLabel.textColor = config.center == 0 ? .labelColor : .disabledControlTextColor

        // Upscalers
        enhancerPopUp.selectItem(withTag: config.enhancer)
        upscalerPopUp.selectItem(withTag: config.upscaler)

        // Effects
        blurPopUp.selectItem(withTag: Int(config.blur))
        blurRadiusSlider.integerValue = config.blurRadius
        blurRadiusSlider.isEnabled = config.blur > 0

        bloomPopUp.selectItem(withTag: Int(config.bloom))
        bloomRadiusSlider.integerValue = config.bloomRadius
        bloomRadiusSlider.isEnabled = config.bloom > 0
        bloomBrightnessSlider.integerValue = config.bloomBrightness
        bloomBrightnessSlider.isEnabled = config.bloom > 0
        bloomWeightSlider.integerValue = config.bloomWeight
        bloomWeightSlider.isEnabled = config.bloom > 0

        flickerPopUp.selectItem(withTag: Int(config.flicker))
        flickerWeightSlider.integerValue = config.flickerWeight
        flickerWeightSlider.isEnabled = config.flicker > 0

        dotMaskPopUp.selectItem(withTag: Int(config.dotMask))
        for i in 0 ... 4 {
            dotMaskPopUp.item(at: i)?.image = renderer.ressourceManager.dotmaskImages[i]
        }
        dotMaskBrightnessSlider.integerValue = config.dotMaskBrightness
        dotMaskBrightnessSlider.isEnabled = config.dotMask > 0

        scanlinesPopUp.selectItem(withTag: Int(config.scanlines))
        scanlineBrightnessSlider.integerValue = config.scanlineBrightness
        scanlineBrightnessSlider.isEnabled = config.scanlines > 0
        scanlineWeightSlider.integerValue = config.scanlineWeight
        scanlineWeightSlider.isEnabled = config.scanlines == 2

        misalignmentPopUp.selectItem(withTag: Int(config.disalignment))
        misalignmentXSlider.integerValue = config.disalignmentH
        misalignmentXSlider.isEnabled = config.disalignment > 0
        misalignmentYSlider.integerValue = config.disalignmentV
        misalignmentYSlider.isEnabled = config.disalignment > 0
    }

    override func preset(tag: Int) {

        let defaults = EmulatorProxy.defaults!

        switch tag {

        case 0: // Recommended settings (all)

            EmulatorProxy.defaults.removeVideoUserDefaults()

        case 10: // Recommended settings (geometry)

            EmulatorProxy.defaults.removeGeometryUserDefaults()

        case 11: // My personal monitor (ViewSonic VP191b)

            infomsg(1, "ViewSonic VP191b")
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

    @IBAction func paletteAction(_ sender: NSPopUpButton!) {

        config?.palette = sender.selectedTag()
    }

    @IBAction func brightnessAction(_ sender: NSSlider!) {

        config?.brightness = sender.integerValue
    }

    @IBAction func contrastAction(_ sender: NSSlider!) {

        config?.contrast = sender.integerValue
    }

    @IBAction func saturationAction(_ sender: NSSlider!) {

        config?.saturation = sender.integerValue
    }

    //
    // Action methods (Geometry)
    //

    @IBAction func zoomAction(_ sender: NSPopUpButton) {

        config?.zoom = sender.selectedTag()
        refresh()
    }

    @IBAction func hZoomAction(_ sender: NSSlider!) {

        config?.hZoom = sender.integerValue
        refresh()
    }

    @IBAction func vZoomAction(_ sender: NSSlider!) {

        config?.vZoom = sender.integerValue
        refresh()
    }

    @IBAction func centerAction(_ sender: NSPopUpButton) {

        config?.center = sender.selectedTag()
        refresh()
    }

    @IBAction func hCenterAction(_ sender: NSSlider!) {

        config?.hCenter = sender.integerValue
        refresh()
    }

    @IBAction func vCenterAction(_ sender: NSSlider!) {

        config?.vCenter = sender.integerValue
        refresh()
    }

    //
    // Action methods (Effects)
    //

    @IBAction func enhancerAction(_ sender: NSPopUpButton!) {

        config?.enhancer = sender.selectedTag()
        refresh()
    }

    @IBAction func upscalerAction(_ sender: NSPopUpButton!) {

        config?.upscaler = sender.selectedTag()
        refresh()
    }

    @IBAction func blurAction(_ sender: NSPopUpButton!) {

        config?.blur = sender.selectedTag()
        refresh()
    }

    @IBAction func blurRadiusAction(_ sender: NSSlider!) {

        config?.blurRadius = sender.integerValue
        refresh()
    }

    @IBAction func bloomAction(_ sender: NSPopUpButton!) {

        config?.bloom = sender.selectedTag()
        refresh()
    }

    @IBAction func bloomRadiusAction(_ sender: NSSlider!) {

        config?.bloomRadius = sender.integerValue
        refresh()
    }

    @IBAction func bloomBrightnessAction(_ sender: NSSlider!) {

        config?.bloomBrightness = sender.integerValue
        refresh()
    }

    @IBAction func bloomWeightAction(_ sender: NSSlider!) {

        config?.bloomWeight = sender.integerValue
        refresh()
    }

    @IBAction func flickerAction(_ sender: NSPopUpButton!) {

        config?.flicker = sender.selectedTag()
        refresh()
    }

    @IBAction func flickerWeightAction(_ sender: NSSlider!) {

        config?.flickerWeight = sender.integerValue
        refresh()
    }

    @IBAction func dotMaskAction(_ sender: NSPopUpButton!) {

        config?.dotMask = sender.selectedTag()
        refresh()
    }

    @IBAction func dotMaskBrightnessAction(_ sender: NSSlider!) {

        config?.dotMaskBrightness = sender.integerValue
        refresh()
    }

    @IBAction func scanlinesAction(_ sender: NSPopUpButton!) {

        config?.scanlines = sender.selectedTag()
        refresh()
    }

    @IBAction func scanlineBrightnessAction(_ sender: NSSlider!) {

        config?.scanlineBrightness = sender.integerValue
        refresh()
    }

    @IBAction func scanlineWeightAction(_ sender: NSSlider!) {

        config?.scanlineWeight = sender.integerValue
        refresh()
    }

    @IBAction func disalignmentAction(_ sender: NSPopUpButton!) {

        config?.disalignment = sender.selectedTag()
        refresh()
    }

    @IBAction func disalignmentHAction(_ sender: NSSlider!) {

        config?.disalignmentH = sender.integerValue
        refresh()
    }

    @IBAction func disalignmentVAction(_ sender: NSSlider!) {

        config?.disalignmentV = sender.integerValue
        refresh()
    }
}
