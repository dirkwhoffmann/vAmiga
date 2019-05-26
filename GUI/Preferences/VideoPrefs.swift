// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

extension PreferencesController {
    
    func awakeVideoPrefsFromNib() {

        // Check for available enhancers
        if let enhancers = myController?.metal.enhancerGallery {
            for i in 0 ... enhancers.count - 1 {
                if let item = vidEnhancerPopUp.menu?.item(withTag: i) {
                    item.isEnabled = (enhancers[i] != nil)
                }
            }
        }

        // Check for available upscalers
        if let upscalers = myController?.metal.upscalerGallery {
            for i in 0 ... upscalers.count - 1 {
                if let item = vidUpscalerPopUp.menu?.item(withTag: i) {
                    item.isEnabled = (upscalers[i] != nil)
                }
            }
        }
        
        // Create dot mask textures
        myController?.metal.buildDotMasks()
    }
    
    func refreshVideoTab() {
        
        // guard let doc = myDocument else { return }
        guard
            let controller = myController,
            let metal = controller.metal
            else { return }

        // Video
        vidEnhancerPopUp.selectItem(withTag: metal.enhancer)
        vidUpscalerPopUp.selectItem(withTag: metal.upscaler)
        vidPalettePopUp.selectItem(withTag: controller.palette)
        vidBrightnessSlider.doubleValue = controller.brightness
        vidContrastSlider.doubleValue = controller.contrast
        vidSaturationSlider.doubleValue = controller.saturation
        
        // Effects
        let shaderOptions = metal.shaderOptions
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
            vidDotMaskPopUp.item(at: i)?.image = metal.dotmaskImages[i]
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
        
        myController?.metal.enhancer = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidUpscalerAction(_ sender: NSPopUpButton!) {

        myController?.metal.upscaler = sender.selectedTag()
        refresh()
    }
    
    @IBAction func vidBlurAction(_ sender: NSPopUpButton!) {
        
        if let metal = myController?.metal {
            track("\(sender.selectedTag())")
            metal.shaderOptions.blur = Int32(sender.selectedTag())
            refresh()
        }
    }
    
    @IBAction func vidBlurRadiusAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metal {
            metal.shaderOptions.blurRadius = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidBloomAction(_ sender: NSPopUpButton!) {
        
        if let metal = myController?.metal {

            metal.shaderOptions.bloom = Int32(sender.selectedTag())
            vidBloomRadiusAction(vidBloomRadiusSlider)
        }
    }
    
    @IBAction func vidBloomRadiusAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metal {
            
            metal.shaderOptions.bloomRadius = sender.floatValue
            refresh()
        }
    }

    @IBAction func vidBloomBrightnessAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metal {
            track("\(sender.floatValue)")
            metal.shaderOptions.bloomBrightness = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidBloomWeightAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metal {
            track("\(sender.floatValue)")
            metal.shaderOptions.bloomWeight = sender.floatValue
            refresh()
        }
    }

    @IBAction func vidFlickerAction(_ sender: NSPopUpButton!) {

        if let metal = myController?.metal {
            track("\(sender.selectedTag())")
            metal.shaderOptions.flicker = Int32(sender.selectedTag())
            refresh()
        }
    }

    @IBAction func vidFlickerWeightAction(_ sender: NSSlider!) {

        if let metal = myController?.metal {
            metal.shaderOptions.flickerWeight = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidDotMaskAction(_ sender: NSPopUpButton!) {
        
        if let metal = myController?.metal {
            metal.shaderOptions.dotMask = Int32(sender.selectedTag())
            metal.buildDotMasks()
            refresh()
        }
    }
    
    @IBAction func vidDotMaskBrightnessAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metal {
            track("\(sender.floatValue)")
            metal.shaderOptions.dotMaskBrightness = sender.floatValue
            metal.buildDotMasks()
            refresh()
        }
    }
    
    @IBAction func vidScanlinesAction(_ sender: NSPopUpButton!) {
        
        if let metal = myController?.metal {
            track("\(sender.selectedTag())")
            metal.shaderOptions.scanlines = Int32(sender.selectedTag())
            refresh()
        }
    }
    @IBAction func vidScanlineBrightnessAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metal {
            track("\(sender.floatValue)")
            metal.shaderOptions.scanlineBrightness = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidScanlineWeightAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metal {
            track("\(sender.floatValue)")
            metal.shaderOptions.scanlineWeight = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidDisalignmentAction(_ sender: NSPopUpButton!) {
        
        if let metal = myController?.metal {
            track("\(sender.selectedTag())")
            metal.shaderOptions.disalignment = Int32(sender.selectedTag())
            refresh()
        }
    }
    @IBAction func vidDisalignmentHAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metal {
            track("\(sender.floatValue)")
            metal.shaderOptions.disalignmentH = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidDisalignmentVAction(_ sender: NSSlider!) {

        if let metal = myController?.metal {
            track("\(sender.floatValue)")
            metal.shaderOptions.disalignmentV = sender.floatValue
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
        myController?.metal.shaderOptions = ShaderDefaultsTFT
        refresh()
    }
    
    @IBAction func vidFactorySettingsActionCRT(_ sender: Any!) {

        track()
        
        myController?.resetVideoUserDefaults()
        myController?.metal.shaderOptions = ShaderDefaultsCRT
        myController?.metal.buildDotMasks()
        refresh()
    }
}
