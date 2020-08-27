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

        // Timing
        memEClockSyncing.state = config.eClockSyncing ? .on : .off
        memSlowRamDelay.state = config.slowRamDelay ? .on : .off

        // Chipset features
        memSlowRamMirror.state = config.slowRamMirror ? .on : .off

        // Lock symbol and explanation
        memLockImage.isHidden = poweredOff
        memLockText.isHidden = poweredOff
        memLockSubText.isHidden = poweredOff
    }

    @IBAction func memEClockSyncingAction(_ sender: NSButton!) {

        track()
        
        config.eClockSyncing = sender.state == .on
        refresh()
    }

    @IBAction func memSlowRamDelayAction(_ sender: NSButton!) {

        track()

        config.slowRamDelay = sender.state == .on
        refresh()
    }

    @IBAction func memSlowRamMirrorAction(_ sender: NSButton!) {

        track()

        config.slowRamMirror = sender.state == .on
        refresh()
    }

    @IBAction func memBankD8DBAction(_ sender: NSPopUpButton!) {

        track()

        // config. = sender.tag
        refresh()
    }

    @IBAction func memBankDCAction(_ sender: NSPopUpButton!) {

        track()

        // config. = sender.tag
        refresh()
    }

    @IBAction func memBankE0E7Action(_ sender: NSPopUpButton!) {

        track()

        // config. = sender.tag
        refresh()
    }

    @IBAction func memBankF0F7Action(_ sender: NSPopUpButton!) {

        track()

        // config. = sender.tag
        refresh()
    }

    @IBAction func memUnmappedAction(_ sender: NSPopUpButton!) {

        track()

        // config. = sender.tag
        refresh()
    }

    @IBAction func memPatternAction(_ sender: NSPopUpButton!) {

        track()

        // config. = sender.tag
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
