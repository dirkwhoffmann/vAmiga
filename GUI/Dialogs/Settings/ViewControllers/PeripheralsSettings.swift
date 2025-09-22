// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class PeripheralsSettingsViewController: SettingsViewController {

    // Flopp drives
    @IBOutlet weak var perDf0Type: NSPopUpButton!
    @IBOutlet weak var perDf1Connect: NSButton!
    @IBOutlet weak var perDf1Type: NSPopUpButton!
    @IBOutlet weak var perDf2Connect: NSButton!
    @IBOutlet weak var perDf2Type: NSPopUpButton!
    @IBOutlet weak var perDf3Connect: NSButton!
    @IBOutlet weak var perDf3Type: NSPopUpButton!

    // Hard drives
    @IBOutlet weak var perHd0Connect: NSButton!
    @IBOutlet weak var perHd0Type: NSPopUpButton!
    @IBOutlet weak var perHd1Connect: NSButton!
    @IBOutlet weak var perHd1Type: NSPopUpButton!
    @IBOutlet weak var perHd2Connect: NSButton!
    @IBOutlet weak var perHd2Type: NSPopUpButton!
    @IBOutlet weak var perHd3Connect: NSButton!
    @IBOutlet weak var perHd3Type: NSPopUpButton!

    // Ports
    @IBOutlet weak var perGameDevice1: NSPopUpButton!
    @IBOutlet weak var perGameDevice2: NSPopUpButton!
    @IBOutlet weak var perSerialDevice: NSPopUpButton!
    @IBOutlet weak var perSerialPort: NSTextField!
    @IBOutlet weak var perSerialPortText: NSTextField!

    // Joystick
    @IBOutlet weak var perAutofire: NSButton!
    @IBOutlet weak var perAutofireText: NSTextField!
    @IBOutlet weak var perAutofireFrequency: NSSlider!
    @IBOutlet weak var perAutofireFrequencyText1: NSTextField!
    @IBOutlet weak var perAutofireFrequencyText2: NSTextField!
    @IBOutlet weak var perAutofireCease: NSButton!
    @IBOutlet weak var perAutofireCeaseText: NSTextField!
    @IBOutlet weak var perAutofireBullets: NSTextField!
    @IBOutlet weak var perAutofireBulletsText: NSTextField!

    // Lock
    @IBOutlet weak var perLockImage: NSButton!
    @IBOutlet weak var perLockInfo1: NSTextField!
    @IBOutlet weak var perLockInfo2: NSTextField!

    // Buttons
    @IBOutlet weak var perFactorySettingsPopup: NSPopUpButton!
    @IBOutlet weak var perOKButton: NSButton!
    @IBOutlet weak var perPowerButton: NSButton!
    
    override func viewDidLoad() {

        log(.lifetime)
    }

    //
    // Methods from SettingsViewController
    //
    
    override func refresh() {

        super.refresh()
    }

    override func preset(tag: Int) {

    }

    override func save() {

    }
}
