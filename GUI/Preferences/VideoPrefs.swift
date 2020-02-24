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
        if let enhancers = myController?.renderer.enhancerGallery {
            for i in 0 ... enhancers.count - 1 {
                if let item = vidEnhancerPopUp.menu?.item(withTag: i) {
                    item.isEnabled = (enhancers[i] != nil)
                }
            }
        }

        // Check for available upscalers
        if let upscalers = myController?.renderer.upscalerGallery {
            for i in 0 ... upscalers.count - 1 {
                if let item = vidUpscalerPopUp.menu?.item(withTag: i) {
                    item.isEnabled = (upscalers[i] != nil)
                }
            }
        }
        
        // Create dot mask textures
        myController?.renderer.buildDotMasks()
    }
    
    func refreshVideoTab() {
        
        // guard let doc = myDocument else { return }
        guard
            let controller = myController,
            let renderer = controller.renderer
            else { return }

        // Video
        vidEnhancerPopUp.selectItem(withTag: renderer.enhancer)
        vidUpscalerPopUp.selectItem(withTag: renderer.upscaler)
        vidPalettePopUp.selectItem(withTag: controller.palette)
        vidBrightnessSlider.doubleValue = controller.brightness
        vidContrastSlider.doubleValue = controller.contrast
        vidSaturationSlider.doubleValue = controller.saturation
        
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
        vidEyeXSlider.floatValue = controller.eyeX
        vidEyeYSlider.floatValue = controller.eyeY
        vidEyeZSlider.floatValue = controller.eyeZ

        // OK Button
        vidOKButton.title = buttonLabel
    }

    //
    // Action methods (Colors)
    //
    
    @IBAction func vidPaletteAction(_ sender: NSPopUpButton!) {
        
        myController?.palette = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidBrightnessAction(_ sender: NSSlider!) {
        
        track("value = \(sender.doubleValue)")
        myController?.brightness = sender.doubleValue
        refresh()
    }
    
    @IBAction func vidContrastAction(_ sender: NSSlider!) {
        
        track("value = \(sender.doubleValue)")
        myController?.contrast = sender.doubleValue
        refresh()
    }
    
    @IBAction func vidSaturationAction(_ sender: NSSlider!) {
        
        track("value = \(sender.doubleValue)")
        myController?.saturation = sender.doubleValue
        refresh()
    }
    
    //
    // Action methods (Effects)
    //
    
    @IBAction func vidEnhancerAction(_ sender: NSPopUpButton!) {
        
        myController?.renderer.enhancer = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidUpscalerAction(_ sender: NSPopUpButton!) {

        myController?.renderer.upscaler = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidBlurAction(_ sender: NSPopUpButton!) {
        
        if let renderer = myController?.renderer {
            track("\(sender.selectedTag())")
            renderer.shaderOptions.blur = Int32(sender.selectedTag())
            refresh()
        }
    }
    
    @IBAction func vidBlurRadiusAction(_ sender: NSSlider!) {
        
        if let renderer = myController?.renderer {
            renderer.shaderOptions.blurRadius = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidBloomAction(_ sender: NSPopUpButton!) {
        
        if let renderer = myController?.renderer {

            renderer.shaderOptions.bloom = Int32(sender.selectedTag())
            vidBloomRadiusAction(vidBloomRadiusSlider)
        }
    }
    
    @IBAction func vidBloomRadiusAction(_ sender: NSSlider!) {
        
        if let renderer = myController?.renderer {
            
            renderer.shaderOptions.bloomRadius = sender.floatValue
            refresh()
        }
    }

    @IBAction func vidBloomBrightnessAction(_ sender: NSSlider!) {
        
        if let renderer = myController?.renderer {
            track("\(sender.floatValue)")
            renderer.shaderOptions.bloomBrightness = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidBloomWeightAction(_ sender: NSSlider!) {
        
        if let renderer = myController?.renderer {
            track("\(sender.floatValue)")
            renderer.shaderOptions.bloomWeight = sender.floatValue
            refresh()
        }
    }

    @IBAction func vidFlickerAction(_ sender: NSPopUpButton!) {

        if let renderer = myController?.renderer {
            track("\(sender.selectedTag())")
            renderer.shaderOptions.flicker = Int32(sender.selectedTag())
            refresh()
        }
    }

    @IBAction func vidFlickerWeightAction(_ sender: NSSlider!) {

        if let renderer = myController?.renderer {
            renderer.shaderOptions.flickerWeight = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidDotMaskAction(_ sender: NSPopUpButton!) {
        
        if let renderer = myController?.renderer {
            renderer.shaderOptions.dotMask = Int32(sender.selectedTag())
            renderer.buildDotMasks()
            refresh()
        }
    }
    
    @IBAction func vidDotMaskBrightnessAction(_ sender: NSSlider!) {
        
        if let renderer = myController?.renderer {
            renderer.shaderOptions.dotMaskBrightness = sender.floatValue
            renderer.buildDotMasks()
            refresh()
        }
    }
    
    @IBAction func vidScanlinesAction(_ sender: NSPopUpButton!) {
        
        if let renderer = myController?.renderer {
            renderer.shaderOptions.scanlines = Int32(sender.selectedTag())
            refresh()
        }
    }
    @IBAction func vidScanlineBrightnessAction(_ sender: NSSlider!) {
        
        if let renderer = myController?.renderer {
            renderer.shaderOptions.scanlineBrightness = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidScanlineWeightAction(_ sender: NSSlider!) {
        
        if let renderer = myController?.renderer {
            renderer.shaderOptions.scanlineWeight = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidDisalignmentAction(_ sender: NSPopUpButton!) {
        
        if let renderer = myController?.renderer {
            renderer.shaderOptions.disalignment = Int32(sender.selectedTag())
            refresh()
        }
    }
    @IBAction func vidDisalignmentHAction(_ sender: NSSlider!) {
        
        if let renderer = myController?.renderer {
            renderer.shaderOptions.disalignmentH = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidDisalignmentVAction(_ sender: NSSlider!) {

        if let renderer = myController?.renderer {
            renderer.shaderOptions.disalignmentV = sender.floatValue
            refresh()
        }
    }
    
    //
    // Action methods (Geometry)
    //
        
    @IBAction func vidEyeXAction(_ sender: NSSlider!) {

        myController?.eyeX = sender.floatValue
        refresh()
    }
    
    @IBAction func vidEyeYAction(_ sender: NSSlider!) {
        
        myController?.eyeY = sender.floatValue
        refresh()
    }
    
    @IBAction func vidEyeZAction(_ sender: NSSlider!) {
        
        myController?.eyeZ = sender.floatValue
        refresh()
    }
    
    //
    // Action methods (Misc)
    //
    
    func vidFactorySettingsAction() {
        
        myController?.resetVideoUserDefaults()
        refresh()
    }
    
    @IBAction func vidFactorySettingsActionTFT(_ sender: Any!) {

        track()
        
        myController?.resetVideoUserDefaults()
        myController?.renderer.shaderOptions = ShaderDefaultsTFT
        refresh()
    }
    
    @IBAction func vidFactorySettingsActionCRT(_ sender: Any!) {

        track()
        
        myController?.resetVideoUserDefaults()
        myController?.renderer.shaderOptions = ShaderDefaultsCRT
        myController?.renderer.buildDotMasks()
        refresh()
    }
}
