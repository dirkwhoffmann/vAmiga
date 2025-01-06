// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Dashboard: DialogController {

    // Panels
    @IBOutlet weak var chipRamPanel: ChipRamPanel!
    @IBOutlet weak var slowRamPanel: SlowRamPanel!
    @IBOutlet weak var fastRamPanel: FastRamPanel!
    @IBOutlet weak var romPanel: RomPanel!
    @IBOutlet weak var copperDmaPanel: CopperDmaPanel!
    @IBOutlet weak var blitterDmaPanel: BlitterDmaPanel!
    @IBOutlet weak var diskDmaPanel: TimeSeries!
    @IBOutlet weak var audioDmaPanel: TimeSeries!
    @IBOutlet weak var spriteDmaPanel: TimeSeries!
    @IBOutlet weak var bitplaneDmaPanel: TimeSeries!
    @IBOutlet weak var myView10: WaveformPanel!
    @IBOutlet weak var myView11: WaveformPanel!
    
    override func awakeFromNib() {

        super.awakeFromNib()
        refresh()
    }
    
    override func windowDidLoad() {
        
        // Remove later...
        parent.renderer.monitors.updateColors()
        parent.renderer.monitors.open(delay: 1.0)
    }
    
    func continuousRefresh() {
                
        // DMA monitors
        let dma = emu.agnus.stats
        copperDmaPanel.model.add(Double(dma.copperActivity) / (313 * 120))
        blitterDmaPanel.model.add(Double(dma.blitterActivity) / (313 * 120))
        diskDmaPanel.model.add(Double(dma.diskActivity) / (313 * 3))
        audioDmaPanel.model.add(Double(dma.audioActivity) / (313 * 4))
        spriteDmaPanel.model.add(Double(dma.spriteActivity) / (313 * 16))
        bitplaneDmaPanel.model.add(Double(dma.bitplaneActivity) / 39330)
        
        // Memory monitors
        let mem = emu.mem.stats
        let max = Double((Constants.hpos_cnt_pal * Constants.vpos_cnt) / 4)
        let chipR = Double(mem.chipReads.accumulated) / max
        let chipW = Double(mem.chipWrites.accumulated) / max
        let slowR = Double(mem.slowReads.accumulated) / max
        let slowW = Double(mem.slowWrites.accumulated) / max
        let fastR = Double(mem.fastReads.accumulated) / max
        let fastW = Double(mem.fastWrites.accumulated) / max
        let kickR = Double(mem.kickReads.accumulated) / max
        let kickW = Double(mem.kickWrites.accumulated) / max
        chipRamPanel.model.add(chipR, chipW)
        slowRamPanel.model.add(slowR, slowW)
        fastRamPanel.model.add(fastR, fastW)
        romPanel.model.add(kickR, kickW)
    
        // Audio monitors
        myView10.update()
        myView11.update()
    }
    
    func refresh() {
               
        let info = emu.dmaDebugger.info
        
        // Colors
        let memColor = NSColor.white
        chipRamPanel.model.themeColor = memColor
        chipRamPanel.model.graph1Color = NSColor.init(r: 0x33, g: 0x99, b: 0xFF)
        chipRamPanel.model.graph2Color = NSColor.init(r: 0xFF, g: 0x33, b: 0x99)
        
        slowRamPanel.model.themeColor = memColor
        slowRamPanel.model.graph1Color = NSColor.init(r: 0x33, g: 0x99, b: 0xFF)
        slowRamPanel.model.graph2Color = NSColor.init(r: 0xFF, g: 0x33, b: 0x99)
        
        fastRamPanel.model.themeColor = memColor
        fastRamPanel.model.graph1Color = NSColor.init(r: 0x33, g: 0x99, b: 0xFF)
        fastRamPanel.model.graph2Color = NSColor.init(r: 0xFF, g: 0x33, b: 0x99)
        
        romPanel.model.themeColor = memColor
        romPanel.model.graph1Color = NSColor.init(r: 0x33, g: 0x99, b: 0xFF)
        romPanel.model.graph2Color = NSColor.init(r: 0xFF, g: 0x33, b: 0x99)

        copperDmaPanel.model.themeColor = NSColor(info.copperColor)
        blitterDmaPanel.model.themeColor = NSColor(info.blitterColor)
        diskDmaPanel.model.themeColor = NSColor(info.diskColor)
        audioDmaPanel.model.themeColor = NSColor(info.audioColor)
        spriteDmaPanel.model.themeColor = NSColor(info.spriteColor)
        bitplaneDmaPanel.model.themeColor = NSColor(info.bitplaneColor)
    }
}
