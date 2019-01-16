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
        
        // Check for available upscalers
        if let kernels = myController?.metalScreen.upscalerGallery {
            
            for i in 0 ... kernels.count - 1 {
                vidUpscalerPopup.menu!.item(withTag: i)?.isEnabled = (kernels[i] != nil)
            }
            myController?.metalScreen.buildDotMasks()
        }
    }
    
    func refreshVideoTab() {
        
        // guard let doc = myDocument else { return }
        guard let con = myController else { return }
        guard let metal = con.metalScreen else { return }
        guard let c64 = proxy else { return }

        track()
        
        // Video
        vidUpscalerPopup.selectItem(withTag: metal.upscaler)
        vidPalettePopup.selectItem(withTag: c64.vic.videoPalette())
        vidBrightnessSlider.doubleValue = c64.vic.brightness()
        vidContrastSlider.doubleValue = c64.vic.contrast()
        vidSaturationSlider.doubleValue = c64.vic.saturation()
        
        // Effects
        let shaderOptions = metal.shaderOptions
        vidBlurPopUp.selectItem(withTag: Int(shaderOptions.blur))
        vidBlurRadiusSlider.floatValue = shaderOptions.blurRadius
        vidBlurRadiusSlider.isEnabled = shaderOptions.blur > 0
        
        vidBloomPopup.selectItem(withTag: Int(shaderOptions.bloom))
        vidBloomRadiusRSlider.floatValue = shaderOptions.bloomRadiusR
        vidBloomRadiusRSlider.isEnabled = shaderOptions.bloom > 0
        vidBloomRadiusGSlider.floatValue = shaderOptions.bloomRadiusG
        vidBloomRadiusGSlider.isEnabled = shaderOptions.bloom > 1
        vidBloomRadiusBSlider.floatValue = shaderOptions.bloomRadiusB
        vidBloomRadiusBSlider.isEnabled = shaderOptions.bloom > 1
        vidBloomBrightnessSlider.floatValue = shaderOptions.bloomBrightness
        vidBloomBrightnessSlider.isEnabled = shaderOptions.bloom > 0
        vidBloomWeightSlider.floatValue = shaderOptions.bloomWeight
        vidBloomWeightSlider.isEnabled = shaderOptions.bloom > 0
        
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
        vidAspectRatioButton.state = metal.keepAspectRatio ? .on : .off
        vidEyeXSlider.floatValue = metal.eyeX()
        vidEyeYSlider.floatValue = metal.eyeY()
        vidEyeZSlider.floatValue = metal.eyeZ()
        
        vidOkButton.title = c64.isRunnable() ? "OK" : "Quit"
    }
    

    //
    // Action methods (Colors)
    //
    
    @IBAction func vidPaletteAction(_ sender: NSPopUpButton!) {
        
        proxy?.vic.setVideoPalette(sender.selectedTag())
        refresh()
    }
    
    @IBAction func vidBrightnessAction(_ sender: NSSlider!) {
        
        proxy?.vic.setBrightness(sender.doubleValue)
        refresh()
    }
    
    @IBAction func vidContrastAction(_ sender: NSSlider!) {
        
        proxy?.vic.setContrast(sender.doubleValue)
        refresh()
    }
    
    @IBAction func vidSaturationAction(_ sender: NSSlider!) {
        
        proxy?.vic.setSaturation(sender.doubleValue)
        refresh()
    }
    
    
    //
    // Action methods (Effects)
    //
    
    @IBAction func vidUpscalerAction(_ sender: NSPopUpButton!) {

        if let metal = myController?.metalScreen {
            track("\(sender.selectedTag())")
            metal.upscaler = sender.selectedTag()
            refresh()
        }
    }
    
    @IBAction func vidBlurAction(_ sender: NSPopUpButton!) {
        
        if let metal = myController?.metalScreen {
            track("\(sender.selectedTag())")
            metal.shaderOptions.blur = Int32(sender.selectedTag())
            refresh()
        }
    }
    
    @IBAction func vidBlurRadiusAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metalScreen {
            track("\(sender.floatValue)")
            metal.shaderOptions.blurRadius = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidBloomAction(_ sender: NSPopUpButton!) {
        
        if let metal = myController?.metalScreen {

            metal.shaderOptions.bloom = Int32(sender.selectedTag())
            vidBloomRadiusRAction(vidBloomRadiusRSlider)
        }
    }
    
    @IBAction func vidBloomRadiusRAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metalScreen {
            
            metal.shaderOptions.bloomRadiusR = sender.floatValue
            
            if vidBloomPopup.selectedTag() == 1 {
                
                // Use this value for the other channels, too
                metal.shaderOptions.bloomRadiusG = sender.floatValue
                metal.shaderOptions.bloomRadiusB = sender.floatValue
            }
            refresh()
        }
    }

    @IBAction func vidBloomRadiusGAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metalScreen {
            
            metal.shaderOptions.bloomRadiusG = sender.floatValue
            
            if vidBloomPopup.selectedTag() == 1 {
                
                // Use this value for the other channels, too
                metal.shaderOptions.bloomRadiusR = sender.floatValue
                metal.shaderOptions.bloomRadiusB = sender.floatValue
            }
            refresh()
        }
    }
    
    @IBAction func vidBloomRadiusBAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metalScreen {
            
            metal.shaderOptions.bloomRadiusB = sender.floatValue
            
            if vidBloomPopup.selectedTag() == 1 {
                
                // Use this value for the other channels, too
                metal.shaderOptions.bloomRadiusR = sender.floatValue
                metal.shaderOptions.bloomRadiusG = sender.floatValue
            }
            refresh()
        }
    }
    
    @IBAction func vidBloomBrightnessAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metalScreen {
            track("\(sender.floatValue)")
            metal.shaderOptions.bloomBrightness = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidBloomWeightAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metalScreen {
            track("\(sender.floatValue)")
            metal.shaderOptions.bloomWeight = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidDotMaskAction(_ sender: NSPopUpButton!) {
        
        if let metal = myController?.metalScreen {
            track("\(sender.selectedTag())")
            metal.shaderOptions.dotMask = Int32(sender.selectedTag())
            metal.buildDotMasks()
            refresh()
        }
    }
    
    @IBAction func vidDotMaskBrightnessAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metalScreen {
            track("\(sender.floatValue)")
            metal.shaderOptions.dotMaskBrightness = sender.floatValue
            metal.buildDotMasks()
            refresh()
        }
    }
    
    @IBAction func vidScanlinesAction(_ sender: NSPopUpButton!) {
        
        if let metal = myController?.metalScreen {
            track("\(sender.selectedTag())")
            metal.shaderOptions.scanlines = Int32(sender.selectedTag())
            refresh()
        }
    }
    @IBAction func vidScanlineBrightnessAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metalScreen {
            track("\(sender.floatValue)")
            metal.shaderOptions.scanlineBrightness = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidScanlineWeightAction(_ sender: NSSlider!)
    {
        if let metal = myController?.metalScreen {
            track("\(sender.floatValue)")
            metal.shaderOptions.scanlineWeight = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidDisalignmentAction(_ sender: NSPopUpButton!) {
        
        if let metal = myController?.metalScreen {
            track("\(sender.selectedTag())")
            metal.shaderOptions.disalignment = Int32(sender.selectedTag())
            refresh()
        }
    }
    @IBAction func vidDisalignmentHAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metalScreen {
            track("\(sender.floatValue)")
            metal.shaderOptions.disalignmentH = sender.floatValue
            refresh()
        }
    }
    
    @IBAction func vidDisalignmentVAction(_ sender: NSSlider!)
    {
        if let metal = myController?.metalScreen {
            track("\(sender.floatValue)")
            metal.shaderOptions.disalignmentV = sender.floatValue
            refresh()
        }
    }
    
    //
    // Action methods (Geometry)
    //
    
    @IBAction func vidAspectRatioAction(_ sender: NSButton!) {
        
        if let metal = myController?.metalScreen {
            metal.keepAspectRatio = (sender.state == .on)
            refresh()
        }
    }
    
    @IBAction func vidEyeXAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metalScreen {
            metal.setEyeX(sender.floatValue)
            refresh()
            
        }
    }
    
    @IBAction func vidEyeYAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metalScreen {
            metal.setEyeY(sender.floatValue)
            refresh()
        }
    }
    
    @IBAction func vidEyeZAction(_ sender: NSSlider!) {
        
        if let metal = myController?.metalScreen {
            metal.setEyeZ(sender.floatValue)
            refresh()
        }
    }
    
    //
    // Action methods (Misc)
    //
    
    func vidFactorySettingsAction() {
        
        myController?.resetVideoUserDefaults()
        refresh()
    }
    
    @IBAction func vidFactorySettingsActionTFT(_ sender: Any!)
    {
        track()
        
        myController?.resetVideoUserDefaults()
        myController?.metalScreen.shaderOptions = ShaderDefaultsTFT
        refresh()
    }
    
    @IBAction func vidFactorySettingsActionCRT(_ sender: Any!)
    {
        track()
        
        myController?.resetVideoUserDefaults()
        myController?.metalScreen.shaderOptions = ShaderDefaultsCRT
        myController?.metalScreen.buildDotMasks()
        refresh()
    }
}
