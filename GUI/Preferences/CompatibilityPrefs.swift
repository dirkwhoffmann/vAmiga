// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension PreferencesController {

    func refreshCompatibilityTab() {

        track()

        guard let amiga = amigaProxy else { return }

        let config = amiga.config()
        let poweredOff = amiga.isPoweredOff()

        // Blitter
        compExactBlitter.state = config.exactBlitter ? .on : .off

        // Disk controller
        compFifoBuffering.state = config.fifoBuffering ? .on : .off

        // Lock controls if emulator is powered on
        compExactBlitter.isEnabled = false // poweredOff
        compFifoBuffering.isEnabled = poweredOff
    }

    @IBAction func compExactBlitterAction(_ sender: NSButton!) {

        amigaProxy?.configureExactBlitter(sender.state == .on)
        refresh()
    }

    @IBAction func compFifoBufferingAction(_ sender: NSButton!) {

        amigaProxy?.configureFifoBuffering(sender.state == .on)
        refresh()
    }

    @IBAction func compUnlockAction(_ sender: Any!) {

        amigaProxy?.powerOff()
        refresh()
    }

    @IBAction func compFactorySettingsAction(_ sender: Any!) {

        myController?.resetCompatibilityUserDefaults()
        refresh()
    }
}
