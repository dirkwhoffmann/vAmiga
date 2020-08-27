// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ConfigurationController {

    func refreshMemoryTab() {

        let poweredOff = amiga.isPoweredOff

        // Chipset features
        memSlowRamMirror.state = config.slowRamMirror ? .on : .off

        // Lock symbol and explanation
        memLockImage.isHidden = poweredOff
        memLockText.isHidden = poweredOff
        memLockSubText.isHidden = poweredOff
    }

    @IBAction func slowRamMirrorAction(_ sender: NSButton!) {

        config.slowRamMirror = sender.state == .on
        refresh()
    }

    @IBAction func memPresetAction(_ sender: NSPopUpButton!) {
         
         switch sender.selectedTag() {
         case 0: config.loadMemoryDefaults(MemoryDefaults.std)
         default: fatalError()
         }
         refresh()
     }

     @IBAction func memDefaultsAction(_ sender: NSButton!) {
         
         config.saveMemoryUserDefaults()
     }
}
