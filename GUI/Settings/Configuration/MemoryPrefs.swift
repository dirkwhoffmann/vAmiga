// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ConfigurationController {

    func refreshPeripheralsTab() {

        let poweredOff      = amiga.isPoweredOff

        // Chipset features
        memSlowRamMirror.state = config.slowRamMirror ? .on : .off

        // Lock symbol and explanation
        perLockImage.isHidden = poweredOff
        perLockText.isHidden = poweredOff
        perLockSubText.isHidden = poweredOff
        
        // Buttons
        perPowerButton.isHidden = !bootable
    }

    @IBAction func memSlowRamMirrorAction(_ sender: NSButton!) {

        config.slowRamMirror = sender.state == .on
        refresh()
    }

    @IBAction func memPresetAction(_ sender: NSPopUpButton!) {
         
         switch sender.selectedTag() {
         case 0: config.loadPeripheralsDefaults(PeripheralsDefaults.std)
         default: fatalError()
         }
         refresh()
     }

     @IBAction func memDefaultsAction(_ sender: NSButton!) {
         
         config.savePeripheralsUserDefaults()
     }
}
