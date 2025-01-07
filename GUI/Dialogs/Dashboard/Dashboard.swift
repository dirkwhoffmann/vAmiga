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
    
    @IBOutlet weak var activityPanel: ActivityBars!
    
    override func awakeFromNib() {

        super.awakeFromNib()
        refresh()
    }
    
    override func windowDidLoad() {
        
        // Remove later...
        parent.renderer.monitors.updateColors()
        parent.renderer.monitors.open(delay: 1.0)
        
        let max = Double((Constants.hpos_cnt_pal * Constants.vpos_cnt) / 4)
        chipRamPanel.configure(range: 0...max)
        slowRamPanel.configure(range: 0...max)
        fastRamPanel.configure(range: 0...max)
        romPanel.configure(range: 0...max)

        copperDmaPanel.configure(range: 0...(313 * 120), logScale: true)
        blitterDmaPanel.configure(range: 0...(313 * 120), logScale: true)
        diskDmaPanel.configure(range: 0...(313 * 3))
        audioDmaPanel.configure(range: 0...(313 * 4))
        spriteDmaPanel.configure(range: 0...(313 * 16))
        bitplaneDmaPanel.configure(range: 0...39330)
    }
    
    func continuousRefresh() {
                
        // DMA monitors
        let dma = emu.agnus.stats
        let copperDma = Double(dma.copperActivity)
        let blitterDma = Double(dma.blitterActivity)
        let diskDma = Double(dma.diskActivity)
        let audioDma = Double(dma.audioActivity)
        let spriteDma = Double(dma.spriteActivity)
        let bitplaneDma = Double(dma.bitplaneActivity)
        
        copperDmaPanel.model.add(copperDma)
        blitterDmaPanel.model.add(blitterDma)
        diskDmaPanel.model.add(diskDma)
        audioDmaPanel.model.add(audioDma)
        spriteDmaPanel.model.add(spriteDma)
        bitplaneDmaPanel.model.add(bitplaneDma)
        
        // Memory monitors
        let mem = emu.mem.stats
        let chipR = Double(mem.chipReads.accumulated)
        let chipW = Double(mem.chipWrites.accumulated)
        let slowR = Double(mem.slowReads.accumulated)
        let slowW = Double(mem.slowWrites.accumulated)
        let fastR = Double(mem.fastReads.accumulated)
        let fastW = Double(mem.fastWrites.accumulated)
        let kickR = Double(mem.kickReads.accumulated)
        let kickW = Double(mem.kickWrites.accumulated)
        chipRamPanel.model.add(chipR, chipW)
        slowRamPanel.model.add(slowR, slowW)
        fastRamPanel.model.add(fastR, fastW)
        romPanel.model.add(kickR, kickW)
    
        // Experimental
        let stats = emu.stats
        activityPanel.model.add(stats.cpuLoad + Double.random(in: -0.01...0.01))
        
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
