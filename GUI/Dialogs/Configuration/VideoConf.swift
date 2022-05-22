// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ConfigurationController {
    
    func awakeVideoPrefsFromNib() {

        // Check for available enhancers
        let enhancers = parent.renderer.ressourceManager.enhancerGallery
        for i in 0 ..< enhancers.count {
            if let item = vidEnhancerPopUp.menu?.item(withTag: i) {
                item.isEnabled = (enhancers[i] != nil)
            }
        }

        // Check for available upscalers
        let upscalers = parent.renderer.ressourceManager.upscalerGallery
        for i in 0 ..< upscalers.count {
            if let item = vidUpscalerPopUp.menu?.item(withTag: i) {
                item.isEnabled = (upscalers[i] != nil)
            }
        }
        
        // Update dot mask textures
        parent.renderer.ressourceManager.buildDotMasks()
    }
    
    func refreshVideoTab() {

        let renderer = parent.renderer!

        // Geometry
        vidHAutoCenter.state = config.hAutoCenter ? .off : .on
        vidVAutoCenter.state = config.vAutoCenter ? .off : .on
        vidHCenter.isEnabled = !config.hAutoCenter
        vidVCenter.isEnabled = !config.vAutoCenter
        vidHCenter.floatValue = config.hCenter * 1000
        vidVCenter.floatValue = config.vCenter * 1000
        vidHZoom.floatValue = config.hZoom * 1000
        vidVZoom.floatValue = config.vZoom * 1000

        // Video
        vidEnhancerPopUp.selectItem(withTag: config.enhancer)
        vidUpscalerPopUp.selectItem(withTag: config.upscaler)
        vidPalettePopUp.selectItem(withTag: config.palette)
        vidBrightnessSlider.integerValue = config.brightness
        vidContrastSlider.integerValue = config.contrast
        vidSaturationSlider.integerValue = config.saturation
        
        // Effects
        vidBlurPopUp.selectItem(withTag: Int(config.blur))
        vidBlurRadiusSlider.floatValue = config.blurRadius
        vidBlurRadiusSlider.isEnabled = config.blur > 0
        
        vidBloomPopUp.selectItem(withTag: Int(config.bloom))
        vidBloomRadiusSlider.floatValue = config.bloomRadius
        vidBloomRadiusSlider.isEnabled = config.bloom > 0
        vidBloomBrightnessSlider.floatValue = config.bloomBrightness
        vidBloomBrightnessSlider.isEnabled = config.bloom > 0
        vidBloomWeightSlider.floatValue = config.bloomWeight
        vidBloomWeightSlider.isEnabled = config.bloom > 0

        vidFlickerPopUp.selectItem(withTag: Int(config.flicker))
        vidFlickerWeightSlider.floatValue = config.flickerWeight
        vidFlickerWeightSlider.isEnabled = config.flicker > 0

        vidDotMaskPopUp.selectItem(withTag: Int(config.dotMask))
        for i in 0 ... 4 {
            vidDotMaskPopUp.item(at: i)?.image = renderer.ressourceManager.dotmaskImages[i]
        }
        vidDotMaskBrightnessSlider.floatValue = config.dotMaskBrightness
        vidDotMaskBrightnessSlider.isEnabled = config.dotMask > 0
        
        vidScanlinesPopUp.selectItem(withTag: Int(config.scanlines))
        vidScanlineBrightnessSlider.floatValue = config.scanlineBrightness
        vidScanlineBrightnessSlider.isEnabled = config.scanlines > 0
        vidScanlineWeightSlider.floatValue = config.scanlineWeight
        vidScanlineWeightSlider.isEnabled = config.scanlines == 2
        
        vidMisalignmentPopUp.selectItem(withTag: Int(config.disalignment))
        vidMisalignmentXSlider.floatValue = config.disalignmentH
        vidMisalignmentXSlider.isEnabled = config.disalignment > 0
        vidMisalignmentYSlider.floatValue = config.disalignmentV
        vidMisalignmentYSlider.isEnabled = config.disalignment > 0
  
        // Buttons
        vidPowerButton.isHidden = !bootable
    }

    //
    // Action methods (Colors)
    //
    
    @IBAction func vidPaletteAction(_ sender: NSPopUpButton!) {
        
        config.palette = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidBrightnessAction(_ sender: NSSlider!) {
        
        config.brightness = sender.integerValue
        refresh()
    }
    
    @IBAction func vidContrastAction(_ sender: NSSlider!) {
        
        config.contrast = sender.integerValue
        refresh()
    }
    
    @IBAction func vidSaturationAction(_ sender: NSSlider!) {
        
        config.saturation = sender.integerValue
        refresh()
    }
    
    //
    // Action methods (Effects)
    //
    
    @IBAction func vidEnhancerAction(_ sender: NSPopUpButton!) {
        
        config.enhancer = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidUpscalerAction(_ sender: NSPopUpButton!) {

        config.upscaler = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidBlurAction(_ sender: NSPopUpButton!) {
        
        config.blur = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidBlurRadiusAction(_ sender: NSSlider!) {
        
        config.blurRadius = sender.floatValue
        refresh()
    }
    
    @IBAction func vidBloomAction(_ sender: NSPopUpButton!) {
        
        config.bloom = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidBloomRadiusAction(_ sender: NSSlider!) {
        
        config.bloomRadius = sender.floatValue
        refresh()
    }

    @IBAction func vidBloomBrightnessAction(_ sender: NSSlider!) {
        
        config.bloomBrightness = sender.floatValue
        refresh()
    }
    
    @IBAction func vidBloomWeightAction(_ sender: NSSlider!) {
        
        config.bloomWeight = sender.floatValue
        refresh()
    }

    @IBAction func vidFlickerAction(_ sender: NSPopUpButton!) {

        config.flicker = sender.selectedTag()
        refresh()
    }

    @IBAction func vidFlickerWeightAction(_ sender: NSSlider!) {

        config.flickerWeight = sender.floatValue
        refresh()
    }
    
    @IBAction func vidDotMaskAction(_ sender: NSPopUpButton!) {
        
        config.dotMask = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidDotMaskBrightnessAction(_ sender: NSSlider!) {
        
        config.dotMaskBrightness = sender.floatValue
        refresh()
    }
    
    @IBAction func vidScanlinesAction(_ sender: NSPopUpButton!) {
        
        config.scanlines = sender.selectedTag()
        refresh()
    }

    @IBAction func vidScanlineBrightnessAction(_ sender: NSSlider!) {
        
        config.scanlineBrightness = sender.floatValue
        refresh()
    }
    
    @IBAction func vidScanlineWeightAction(_ sender: NSSlider!) {
        
        config.scanlineWeight = sender.floatValue
        refresh()
    }
    
    @IBAction func vidDisalignmentAction(_ sender: NSPopUpButton!) {
        
        config.disalignment = sender.selectedTag()
        refresh()
    }

    @IBAction func vidDisalignmentHAction(_ sender: NSSlider!) {
        
        config.disalignmentH = sender.floatValue
        refresh()
    }
    
    @IBAction func vidDisalignmentVAction(_ sender: NSSlider!) {

        config.disalignmentV = sender.floatValue
        refresh()
    }
    
    //
    // Action methods (Geometry)
    //

    @IBAction func vidHAutoCenterAction(_ sender: NSButton) {

        let state = sender.state == .off
        debug(.config, "hAutoCenter = \(state)")

        config.hAutoCenter = state
        parent.renderer.canvas.updateTextureRect()
        refresh()
    }

    @IBAction func vidVAutoCenterAction(_ sender: NSButton) {

        let state = sender.state == .off
        debug(.config, "vAutoCenter = \(state)")

        config.vAutoCenter = state
        parent.renderer.canvas.updateTextureRect()
        refresh()
    }

    @IBAction func vidHCenterAction(_ sender: NSSlider!) {

        let value = sender.floatValue / 1000
        debug(.config, "hCenter = \(value)")

        config.hCenter = value
        refresh()
    }
    
    @IBAction func vidVCenterAction(_ sender: NSSlider!) {

        let value = sender.floatValue / 1000
        debug(.config, "vCenter = \(value)")

        config.vCenter = value
        refresh()
    }
    
    @IBAction func vidHZoomAction(_ sender: NSSlider!) {

        let value = sender.floatValue / 1000
        debug(.config, "hZoom = \(value)")

        config.hZoom = value
        refresh()
    }

    @IBAction func vidVZoomAction(_ sender: NSSlider!) {

        let value = sender.floatValue / 1000
        debug(.config, "vZoom = \(value)")

        config.vZoom = value
        refresh()
    }
    
    //
    // Action methods (Misc)
    //
    
    @IBAction func vidPresetAction(_ sender: NSMenuItem!) {
              
        let defaults = AmigaProxy.defaults!

        switch sender.tag {
            
        case 0: // Recommended settings
            
            AmigaProxy.defaults.removeVideoUserDefaults()
                        
        case 1: // Narrow Geometry
            
            AmigaProxy.defaults.removeGeometryUserDefaults()
            defaults.set(Keys.Vid.hAutoCenter, true)
            defaults.set(Keys.Vid.vAutoCenter, true)
            defaults.set(Keys.Vid.hCenter, 0.6)
            defaults.set(Keys.Vid.vCenter, 0.47)
            defaults.set(Keys.Vid.hZoom, 1.0)
            defaults.set(Keys.Vid.vZoom, 0.27)
            
        case 2: // Wide Geometry
            
            AmigaProxy.defaults.removeGeometryUserDefaults()
            defaults.set(Keys.Vid.hAutoCenter, true)
            defaults.set(Keys.Vid.vAutoCenter, true)
            defaults.set(Keys.Vid.hCenter, 0.409)
            defaults.set(Keys.Vid.vCenter, 0.143)
            defaults.set(Keys.Vid.hZoom, 0.747)
            defaults.set(Keys.Vid.vZoom, 0.032)

        case 3: // Extreme Geometry
            
            AmigaProxy.defaults.removeGeometryUserDefaults()
            defaults.set(Keys.Vid.hAutoCenter, false)
            defaults.set(Keys.Vid.vAutoCenter, false)
            defaults.set(Keys.Vid.hCenter, 0)
            defaults.set(Keys.Vid.vCenter, 0)
            defaults.set(Keys.Vid.hZoom, 0)
            defaults.set(Keys.Vid.vZoom, 0)
            
        case 6: // TFT Appearance
            
            AmigaProxy.defaults.removeColorUserDefaults()
            AmigaProxy.defaults.removeShaderUserDefaults()

        case 7: // CRT Appearance
            
            AmigaProxy.defaults.removeColorUserDefaults()
            AmigaProxy.defaults.removeShaderUserDefaults()
            defaults.set(Keys.Vid.blurRadius, 1.5)
            defaults.set(Keys.Vid.bloom, 1)
            defaults.set(Keys.Vid.dotMask, 1)
            defaults.set(Keys.Vid.scanlines, 2)

        case 10: // My personal monitor (ViewSonic VP191b)

            AmigaProxy.defaults.removeColorUserDefaults()
            AmigaProxy.defaults.removeShaderUserDefaults()
            AmigaProxy.defaults.removeGeometryUserDefaults()
            defaults.set(Keys.Vid.hAutoCenter, false)
            defaults.set(Keys.Vid.vAutoCenter, false)
            defaults.set(Keys.Vid.hCenter, 0.39813587)
            defaults.set(Keys.Vid.vCenter, 1.0)
            defaults.set(Keys.Vid.hZoom, 0.6763221)
            defaults.set(Keys.Vid.vZoom, 0.032)

        default:
            fatalError()
        }
        
        config.applyVideoUserDefaults()
        refresh()
    }
    
    @IBAction func vidDefaultsAction(_ sender: NSButton!) {
        
        config.saveVideoUserDefaults()
    }
}
