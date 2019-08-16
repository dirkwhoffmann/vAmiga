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

        // Graphics
        compClxSprSpr.state = config.clxSprSpr ? .on : .off
        compClxSprPlf.state = config.clxSprPlf ? .on : .off
        compClxPlfPlf.state = config.clxPlfPlf ? .on : .off

        // Blitter
        let a = config.blitterAccuracy
        compBltAccuracy.intValue = a
        compBltLevel1.textColor = (a >= 1) ? .labelColor : .tertiaryLabelColor
        compBltLevel2.textColor = (a >= 2) ? .labelColor : .tertiaryLabelColor
        compBltLevel3.textColor = (a >= 3) ? .labelColor : .tertiaryLabelColor

        // Audio
        compFilterActivation.selectItem(withTag: config.filterActivation.rawValue)

        // Disk controller
        compFifoBuffering.state = config.fifoBuffering ? .on : .off

        // Lock controls if emulator is powered on
        // compBltAccuracy.isEnabled = poweredOff
        compFifoBuffering.isEnabled = poweredOff

        // Label the OK button
        compOKButton.title = okLabel
    }

    @IBAction func compClxSprSprAction(_ sender: NSButton!) {

        amigaProxy?.configure(VA_CLX_SPR_SPR, enable: sender.state == .on)
        refresh()
    }

    @IBAction func compClxSprPlfAction(_ sender: NSButton!) {

        amigaProxy?.configure(VA_CLX_SPR_PLF, enable: sender.state == .on)
        refresh()
    }

    @IBAction func compClxPlfPlfAction(_ sender: NSButton!) {

        amigaProxy?.configure(VA_CLX_PLF_PLF, enable: sender.state == .on)
        refresh()
    }

    @IBAction func compFilterActivationAction(_ sender: NSPopUpButton!) {

        amigaProxy?.configure(VA_FILTER_ACTIVATION, value: sender.selectedTag())
        refresh()
    }

    @IBAction func compBltAccuracyAction(_ sender: NSSlider!) {

        amigaProxy?.configure(VA_BLITTER_ACCURACY, value: sender.integerValue)
        refresh()
    }

    @IBAction func compFifoBufferingAction(_ sender: NSButton!) {

        amigaProxy?.configureFifoBuffering(sender.state == .on)
        refresh()
    }

    @IBAction func compFactorySettingsAction(_ sender: Any!) {

        myController?.resetCompatibilityUserDefaults()
        refresh()
    }
}
