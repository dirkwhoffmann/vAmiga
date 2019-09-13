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
        // let poweredOff = amiga.isPoweredOff()

        // Graphics
        compClxSprSpr.state = config.clxSprSpr ? .on : .off
        compClxSprPlf.state = config.clxSprPlf ? .on : .off
        compClxPlfPlf.state = config.clxPlfPlf ? .on : .off

        // CPU
        compCpuEngine.selectItem(withTag: config.cpuEngine.rawValue)
        compCpuSpeed.selectItem(withTag: Int(config.cpuSpeed))

        // Blitter
        let a = config.blitter.accuracy
        compBltAccuracy.intValue = a
        compBltLevel1.textColor = (a >= 1) ? .labelColor : .tertiaryLabelColor
        compBltLevel2.textColor = (a >= 2) ? .labelColor : .tertiaryLabelColor

        // Audio
        compFilterActivation.selectItem(withTag: config.filterActivation.rawValue)

        // Floppy drives
        compDriveSpeed.selectItem(withTag: Int(config.diskController.speed))
        compFifoBuffering.isHidden = config.diskController.speed > 256
        compFifoBuffering.state = config.diskController.useFifo ? .on : .off

        // Lock controls if emulator is powered on
        // Nothing to do here at the moment

        // Label the OK button
        compOKButton.title = okLabel
    }

    @IBAction func compCpuEngineAction(_ sender: NSPopUpButton!) {

        amigaProxy?.configure(VA_CPU_ENGINE, value: sender.selectedTag())
        refresh()
    }

    @IBAction func compCpuSpeedAction(_ sender: NSPopUpButton!) {

        amigaProxy?.configure(VA_CPU_SPEED, value: sender.selectedTag())
        refresh()
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

    @IBAction func compDriveSpeedAction(_ sender: NSPopUpButton!) {

        amigaProxy?.configure(VA_DRIVE_SPEED, value: sender.selectedTag())
        refresh()
    }

    @IBAction func compFifoBufferingAction(_ sender: NSButton!) {

        amigaProxy?.configure(VA_FIFO_BUFFERING, enable: sender.state == .on)
        refresh()
    }

    @IBAction func compFactorySettingsAction(_ sender: Any!) {

        myController?.resetCompatibilityUserDefaults()
        refresh()
    }
}
