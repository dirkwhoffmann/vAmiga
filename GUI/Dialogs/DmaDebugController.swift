// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class DmaDebugController: DialogController {

    @IBOutlet weak var dmaDebugCpu: NSButton!
    @IBOutlet weak var dmaDebugRefresh: NSButton!
    @IBOutlet weak var dmaDebugDisk: NSButton!
    @IBOutlet weak var dmaDebugAudio: NSButton!
    @IBOutlet weak var dmaDebugBitplane: NSButton!
    @IBOutlet weak var dmaDebugSprites: NSButton!
    @IBOutlet weak var dmaDebugCopper: NSButton!
    @IBOutlet weak var dmaDebugBlitter: NSButton!

    @IBOutlet weak var dmaDebugCpuCol: NSColorWell!
    @IBOutlet weak var dmaDebugRefreshCol: NSColorWell!
    @IBOutlet weak var dmaDebugDiskCol: NSColorWell!
    @IBOutlet weak var dmaDebugAudioCol: NSColorWell!
    @IBOutlet weak var dmaDebugBitplaneCol: NSColorWell!
    @IBOutlet weak var dmaDebugSpritesCol: NSColorWell!
    @IBOutlet weak var dmaDebugCopperCol: NSColorWell!
    @IBOutlet weak var dmaDebugBlitterCol: NSColorWell!

    @IBOutlet weak var dmaDebugOpacity: NSSlider!
    @IBOutlet weak var dmaDebugDisplayMode: NSPopUpButton!

    /*
    override func showSheet(completionHandler handler:(() -> Void)? = nil) {

        super.showSheet(completionHandler: handler)
    }
    */

    override public func awakeFromNib() {

        track()
    }

    override func windowDidLoad() {

        refresh()
    }

    override func refresh() {

        track()

        let info = amiga.agnus.getDebuggerInfo()
        let rgb = info.colorRGB

        dmaDebugCpu.state = info.visualize.1 ? .on : .off
        dmaDebugRefresh.state = info.visualize.2 ? .on : .off
        dmaDebugDisk.state = info.visualize.3 ? .on : .off
        dmaDebugAudio.state = info.visualize.4 ? .on : .off
        dmaDebugBitplane.state = info.visualize.5 ? .on : .off
        dmaDebugSprites.state = info.visualize.6 ? .on : .off
        dmaDebugCopper.state = info.visualize.7 ? .on : .off
        dmaDebugBlitter.state = info.visualize.8 ? .on : .off

        dmaDebugCpuCol.color = NSColor.init(r: rgb.1.0, g: rgb.1.1, b: rgb.1.2)
        dmaDebugRefreshCol.color = NSColor.init(r: rgb.2.0, g: rgb.2.1, b: rgb.2.2)
        dmaDebugDiskCol.color = NSColor.init(r: rgb.3.0, g: rgb.3.1, b: rgb.3.2)
        dmaDebugAudioCol.color = NSColor.init(r: rgb.4.0, g: rgb.4.1, b: rgb.4.2)
        dmaDebugBitplaneCol.color = NSColor.init(r: rgb.5.0, g: rgb.5.1, b: rgb.5.2)
        dmaDebugSpritesCol.color = NSColor.init(r: rgb.6.0, g: rgb.6.1, b: rgb.6.2)
        dmaDebugCopperCol.color = NSColor.init(r: rgb.7.0, g: rgb.7.1, b: rgb.7.2)
        dmaDebugBlitterCol.color = NSColor.init(r: rgb.8.0, g: rgb.8.1, b: rgb.8.2)

        dmaDebugOpacity.doubleValue = info.opacity * 100.0
        dmaDebugDisplayMode.selectItem(withTag: info.displayMode.rawValue)
    }

    //
    // Action methods
    //

    @IBAction func dmaDebugVisualizeAction(_ sender: NSButton!) {

        let owner = BusOwner(Int8(sender.tag))
        amiga.agnus.dmaDebugSetVisualize(owner, value: sender.state == .on)
        refresh()
    }

    @IBAction func dmaDebugColorAction(_ sender: NSColorWell!) {

        let color = sender.color
        let owner = BusOwner(Int8(sender.tag))
        let r = Double(color.redComponent)
        let g = Double(color.greenComponent)
        let b = Double(color.blueComponent)
        amiga.agnus.dmaDebugSetColor(owner, r: r, g: g, b: b)
        refresh()
    }

    @IBAction func dmaDebugDisplayModeAction(_ sender: NSPopUpButton!) {

        amiga.agnus.dmaDebugSetDisplayMode(sender.selectedTag())
        refresh()
    }

    @IBAction func dmaDebugOpacityAction(_ sender: NSSlider!) {

        amiga.agnus.dmaDebugSetOpacity(sender.doubleValue / 100.0)
        refresh()
    }
}
