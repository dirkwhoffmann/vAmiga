// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class ServerSettingsViewController: SettingsViewController {
    
    // Remote Shell server
    @IBOutlet weak var rshEnable: NSButton!
    @IBOutlet weak var rshStatusIcon: NSImageView!
    @IBOutlet weak var rshStatusText: NSTextField!
    @IBOutlet weak var rshPortText: NSTextField!
    @IBOutlet weak var rshPort: RangeField!
    @IBOutlet weak var rshInfo: NSTextField!

    // RPC server
    @IBOutlet weak var rpcEnable: NSButton!
    @IBOutlet weak var rpcStatusIcon: NSImageView!
    @IBOutlet weak var rpcStatusText: NSTextField!
    @IBOutlet weak var rpcPortText: NSTextField!
    @IBOutlet weak var rpcPort: RangeField!
    @IBOutlet weak var rpcInfo: NSTextField!

    // DAP server (not implemented yet)
    @IBOutlet weak var dapEnable: NSButton!
    @IBOutlet weak var dapStatusIcon: NSImageView!
    @IBOutlet weak var dapStatusText: NSTextField!
    @IBOutlet weak var dapPortText: NSTextField!
    @IBOutlet weak var dapPort: RangeField!
    @IBOutlet weak var dapInfo: NSTextField!

    // Prometheus server
    @IBOutlet weak var promEnable: NSButton!
    @IBOutlet weak var promStatusIcon: NSImageView!
    @IBOutlet weak var promStatusText: NSTextField!
    @IBOutlet weak var promPortText: NSTextField!
    @IBOutlet weak var promPort: RangeField!
    @IBOutlet weak var promInfo: NSTextField!

    // Serial server
    @IBOutlet weak var serEnable: NSButton!
    @IBOutlet weak var serStatusIcon: NSImageView!
    @IBOutlet weak var serStatusText: NSTextField!
    @IBOutlet weak var serPortText: NSTextField!
    @IBOutlet weak var serPort: RangeField!
    @IBOutlet weak var serInfo: NSTextField!

    override func viewDidLoad() {

        log(.lifetime)
        [rshPort, rpcPort, promPort, serPort, dapPort].forEach { $0.setRange(0...65535) }

        // Hide the DAP adapter until it is supported
        dapEnable.isHidden = true
        dapStatusIcon.isHidden = true
        dapStatusText.isHidden = true
        dapPortText.isHidden = true
        dapPort.isHidden = true
        dapInfo.isHidden = true
    }

    override func refresh() {

        func prettyPrint(state: SrvState) -> (String, NSColor) {

            switch state {
            case .OFF:          return ("Disconnected", .systemRed)
            case .WAITING:      return ("Waiting",      .systemYellow)
            case .STARTING:     return ("Starting",     .systemYellow)
            case .LISTENING:    return ("Listening",    .systemYellow)
            case .CONNECTED:    return ("Connected",    .systemGreen)
            case .STOPPING:     return ("Stopping",     .systemYellow)
            case .INVALID:      return ("Invalid",      .systemRed)
            default:            return ("???",          .textColor)
            }
        }

        func update(_ icon: NSImageView, _ text: NSTextField, state: SrvState) {
            let format = prettyPrint(state: state)
            icon.contentTintColor = format.1
            text.stringValue = format.0
            // text.textColor = format.1
            text.font = NSFont.boldSystemFont(ofSize: text.font?.pointSize ?? NSFont.systemFontSize)

        }
        /*
        func update(_ components: [NSControl], enable: Bool) {
            components.forEach { $0.isEnabled = enable }
        }
        */

        super.refresh()

        guard let emu = emu, let config = config else { return }

        let serverInfo = emu.remoteManager.info

        // Remote Shell server
        var enable = config.rshServerEnable
        rshEnable.state = enable ? .on : .off
        rshPort.integerValue = Int(config.rshServerPort)
        update(rshStatusIcon, rshStatusText, state: serverInfo.rshInfo.state)
        rshPort.isEnabled = enable
        rshPortText.textColor = enable ? .textColor : .secondaryLabelColor
        rshInfo.textColor = enable ? .textColor : .secondaryLabelColor

        // RPC server
        enable = config.rpcServerEnable
        rpcEnable.state = enable ? .on : .off
        rpcPort.integerValue = Int(config.rpcServerPort)
        update(rpcStatusIcon, rpcStatusText, state: serverInfo.rpcInfo.state)
        rpcPort.isEnabled = enable
        rpcPortText.textColor = enable ? .textColor : .secondaryLabelColor
        rpcInfo.textColor = enable ? .textColor : .secondaryLabelColor

        // GDB server
        enable = config.gdbServerEnable
        dapEnable.state = enable ? .on : .off
        dapPort.integerValue = Int(config.gdbServerPort)
        update(dapStatusIcon, dapStatusText, state: serverInfo.gdbInfo.state)
        dapPort.isEnabled = enable
        dapPortText.textColor = enable ? .textColor : .secondaryLabelColor
        dapInfo.textColor = enable ? .textColor : .secondaryLabelColor

        // Prometheus server
        enable = config.promServerEnable
        promEnable.state = enable ? .on : .off
        promPort.integerValue = Int(config.promServerPort)
        update(promStatusIcon, promStatusText, state: serverInfo.promInfo.state)
        promPort.isEnabled = enable
        promPortText.textColor = enable ? .textColor : .secondaryLabelColor
        promInfo.textColor = enable ? .textColor : .secondaryLabelColor

        // Serial port server
        enable = config.serServerEnable
        serEnable.state = enable ? .on : .off
        serPort.integerValue = Int(config.serServerPort)
        update(serStatusIcon, serStatusText, state: serverInfo.serInfo.state)
        serPort.isEnabled = enable
        serPortText.textColor = enable ? .textColor : .secondaryLabelColor
        serInfo.textColor = enable ? .textColor : .secondaryLabelColor
    }

    @IBAction func enableAction(_ sender: NSButton!) {

        print("enableAction \(sender.integerValue)")

        switch sender.tag {
        case 0: config?.rshServerEnable = sender.state == .on
        case 1: config?.rpcServerEnable = sender.state == .on
        case 2: config?.gdbServerEnable = sender.state == .on
        case 3: config?.promServerEnable = sender.state == .on
        case 4: config?.serServerEnable = sender.state == .on
        default: fatalError()
        }
    }

    @IBAction func portAction(_ sender: NSTextField!) {

        print("portAction \(sender.integerValue)")

        switch sender.tag {
        case 0: config?.rshServerPort = sender.integerValue
        case 1: config?.rpcServerPort = sender.integerValue
        case 2: config?.gdbServerPort = sender.integerValue
        case 3: config?.promServerPort = sender.integerValue
        case 4: config?.serServerPort = sender.integerValue
        default: fatalError()
        }
    }


    //
    // Presets and Saving
    //

    override func preset(tag: Int) {

        guard let emu = emu else { return }

        emu.suspend()

        // Revert to standard settings
        EmulatorProxy.defaults.removeServerUserDefaults()

        // Update the configuration
        config?.applyServerUserDefaults()

        emu.resume()
    }

    override func save() {

        config?.saveServerUserDefaults()
    }
}
