// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension PreferencesController {
    
    func awakeVideoPrefsFromNib() {

        // Check for available enhancers
        let enhancers = parent.renderer.enhancerGallery
        for i in 0 ... enhancers.count - 1 {
            if let item = vidEnhancerPopUp.menu?.item(withTag: i) {
                item.isEnabled = (enhancers[i] != nil)
            }
        }

        // Check for available upscalers
        let upscalers = parent.renderer.upscalerGallery
        for i in 0 ... upscalers.count - 1 {
            if let item = vidUpscalerPopUp.menu?.item(withTag: i) {
                item.isEnabled = (upscalers[i] != nil)
            }
        }
        
        // Create dot mask textures
        parent.renderer.buildDotMasks()
    }
    
    func refreshVideoTab() {

        let renderer = parent.renderer!

        // Video
        vidEnhancerPopUp.selectItem(withTag: renderer.enhancer)
        vidUpscalerPopUp.selectItem(withTag: renderer.upscaler)
        vidPalettePopUp.selectItem(withTag: parent.palette)
        vidBrightnessSlider.doubleValue = parent.brightness
        vidContrastSlider.doubleValue = parent.contrast
        vidSaturationSlider.doubleValue = parent.saturation
        
        // Effects
        let shaderOptions = renderer.shaderOptions
        vidBlurPopUp.selectItem(withTag: Int(shaderOptions.blur))
        vidBlurRadiusSlider.floatValue = shaderOptions.blurRadius
        vidBlurRadiusSlider.isEnabled = shaderOptions.blur > 0
        
        vidBloomPopUp.selectItem(withTag: Int(shaderOptions.bloom))
        vidBloomRadiusSlider.floatValue = shaderOptions.bloomRadius
        vidBloomRadiusSlider.isEnabled = shaderOptions.bloom > 0
        vidBloomBrightnessSlider.floatValue = shaderOptions.bloomBrightness
        vidBloomBrightnessSlider.isEnabled = shaderOptions.bloom > 0
        vidBloomWeightSlider.floatValue = shaderOptions.bloomWeight
        vidBloomWeightSlider.isEnabled = shaderOptions.bloom > 0

        vidFlickerPopUp.selectItem(withTag: Int(shaderOptions.flicker))
        vidFlickerWeightSlider.floatValue = shaderOptions.flickerWeight
        vidFlickerWeightSlider.isEnabled = shaderOptions.flicker > 0

        vidDotMaskPopUp.selectItem(withTag: Int(shaderOptions.dotMask))
        for i in 0 ... 4 {
            vidDotMaskPopUp.item(at: i)?.image = renderer.dotmaskImages[i]
        }
        vidDotMaskBrightnessSlider.floatValue = shaderOptions.dotMaskBrightness
        vidDotMaskBrightnessSlider.isEnabled = shaderOptions.dotMask > 0
        
        vidScanlinesPopUp.selectItem(withTag: Int(shaderOptions.scanlines))
        vidScanlineBrightnessSlider.floatValue = shaderOptions.scanlineBrightness
        vidScanlineBrightnessSlider.isEnabled = shaderOptions.scanlines > 0
        vidScanlineWeightSlider.floatValue = shaderOptions.scanlineWeight
        vidScanlineWeightSlider.isEnabled = shaderOptions.scanlines == 2
        
        vidMisalignmentPopUp.selectItem(withTag: Int(shaderOptions.disalignment))
        vidMisalignmentXSlider.floatValue = shaderOptions.disalignmentH
        vidMisalignmentXSlider.isEnabled = shaderOptions.disalignment > 0
        vidMisalignmentYSlider.floatValue = shaderOptions.disalignmentV
        vidMisalignmentYSlider.isEnabled = shaderOptions.disalignment > 0

        // Geometry
        vidEyeXSlider.floatValue = parent.eyeX
        vidEyeYSlider.floatValue = parent.eyeY
        vidEyeZSlider.floatValue = parent.eyeZ

        // OK Button
        vidOKButton.title = buttonLabel
    }

    //
    // Action methods (Colors)
    //
    
    @IBAction func vidPaletteAction(_ sender: NSPopUpButton!) {
        
        parent.palette = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidBrightnessAction(_ sender: NSSlider!) {
        
        parent.brightness = sender.doubleValue
        refresh()
    }
    
    @IBAction func vidContrastAction(_ sender: NSSlider!) {
        
        parent.contrast = sender.doubleValue
        refresh()
    }
    
    @IBAction func vidSaturationAction(_ sender: NSSlider!) {
        
        parent.saturation = sender.doubleValue
        refresh()
    }
    
    //
    // Action methods (Effects)
    //
    
    @IBAction func vidEnhancerAction(_ sender: NSPopUpButton!) {
        
        parent.renderer.enhancer = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidUpscalerAction(_ sender: NSPopUpButton!) {

        parent.renderer.upscaler = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidBlurAction(_ sender: NSPopUpButton!) {
        
        parent.renderer.shaderOptions.blur = Int32(sender.selectedTag())
        refresh()
    }
    
    @IBAction func vidBlurRadiusAction(_ sender: NSSlider!) {
        
        parent.renderer.shaderOptions.blurRadius = sender.floatValue
        refresh()
    }
    
    @IBAction func vidBloomAction(_ sender: NSPopUpButton!) {
        
        parent.renderer.shaderOptions.bloom = Int32(sender.selectedTag())
        vidBloomRadiusAction(vidBloomRadiusSlider)
    }
    
    @IBAction func vidBloomRadiusAction(_ sender: NSSlider!) {
        
        parent.renderer.shaderOptions.bloomRadius = sender.floatValue
        refresh()
    }

    @IBAction func vidBloomBrightnessAction(_ sender: NSSlider!) {
        
        parent.renderer.shaderOptions.bloomBrightness = sender.floatValue
        refresh()
    }
    
    @IBAction func vidBloomWeightAction(_ sender: NSSlider!) {
        
        parent.renderer.shaderOptions.bloomWeight = sender.floatValue
        refresh()
    }

    @IBAction func vidFlickerAction(_ sender: NSPopUpButton!) {

        parent.renderer.shaderOptions.flicker = Int32(sender.selectedTag())
        refresh()
    }

    @IBAction func vidFlickerWeightAction(_ sender: NSSlider!) {

        parent.renderer.shaderOptions.flickerWeight = sender.floatValue
        refresh()
    }
    
    @IBAction func vidDotMaskAction(_ sender: NSPopUpButton!) {
        
        parent.renderer.shaderOptions.dotMask = Int32(sender.selectedTag())
        parent.renderer.buildDotMasks()
        refresh()
    }
    
    @IBAction func vidDotMaskBrightnessAction(_ sender: NSSlider!) {
        
        parent.renderer.shaderOptions.dotMaskBrightness = sender.floatValue
        parent.renderer.buildDotMasks()
        refresh()
    }
    
    @IBAction func vidScanlinesAction(_ sender: NSPopUpButton!) {
        
        parent.renderer.shaderOptions.scanlines = Int32(sender.selectedTag())
        refresh()
    }

    @IBAction func vidScanlineBrightnessAction(_ sender: NSSlider!) {
        
        parent.renderer.shaderOptions.scanlineBrightness = sender.floatValue
        refresh()
    }
    
    @IBAction func vidScanlineWeightAction(_ sender: NSSlider!) {
        
        parent.renderer.shaderOptions.scanlineWeight = sender.floatValue
        refresh()
    }
    
    @IBAction func vidDisalignmentAction(_ sender: NSPopUpButton!) {
        
        parent.renderer.shaderOptions.disalignment = Int32(sender.selectedTag())
        refresh()
    }

    @IBAction func vidDisalignmentHAction(_ sender: NSSlider!) {
        
        parent.renderer.shaderOptions.disalignmentH = sender.floatValue
        refresh()
    }
    
    @IBAction func vidDisalignmentVAction(_ sender: NSSlider!) {

        parent.renderer.shaderOptions.disalignmentV = sender.floatValue
        refresh()
    }
    
    //
    // Action methods (Geometry)
    //
        
    @IBAction func vidEyeXAction(_ sender: NSSlider!) {

        parent.eyeX = sender.floatValue
        refresh()
    }
    
    @IBAction func vidEyeYAction(_ sender: NSSlider!) {
        
        parent.eyeY = sender.floatValue
        refresh()
    }
    
    @IBAction func vidEyeZAction(_ sender: NSSlider!) {
        
        parent.eyeZ = sender.floatValue
        refresh()
    }
    
    //
    // Action methods (Misc)
    //
    
    func vidFactorySettingsAction() {
        
        parent.resetVideoUserDefaults()
        refresh()
    }
    
    @IBAction func vidFactorySettingsActionTFT(_ sender: Any!) {

        parent.resetVideoUserDefaults()
        parent.renderer.shaderOptions = ShaderDefaultsTFT
        refresh()
    }
    
    @IBAction func vidFactorySettingsActionCRT(_ sender: Any!) {

        parent.resetVideoUserDefaults()
        parent.renderer.shaderOptions = ShaderDefaultsCRT
        parent.renderer.buildDotMasks()
        refresh()
    }
}
