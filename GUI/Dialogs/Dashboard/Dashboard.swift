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
        let max = Double((Constants.hpos_cnt_pal * Constants.vpos_cnt) / 2)
        let chipR = Double(mem.chipReads.accumulated) / max
        let chipW = Double(mem.chipWrites.accumulated) / max
        let slowR = Double(mem.slowReads.accumulated) / max
        let slowW = Double(mem.slowWrites.accumulated) / max
        let fastR = Double(mem.fastReads.accumulated) / max
        let fastW = Double(mem.fastWrites.accumulated) / max
        let kickR = Double(mem.kickReads.accumulated) / max
        let kickW = Double(mem.kickWrites.accumulated) / max
        chipRamPanel.model.add(chipW, chipR)
        slowRamPanel.model.add(slowW, slowR)
        fastRamPanel.model.add(fastW, fastR)
        romPanel.model.add(kickW, kickR)
    
        // Audio monitors
        myView10.update()
        myView11.update()
    }
    
    func refresh() {
               
        let info = emu.dmaDebugger.info
        
        // Colors
        copperDmaPanel.model.color = NSColor(info.copperColor)
        blitterDmaPanel.model.color = NSColor(info.blitterColor)
        diskDmaPanel.model.color = NSColor(info.diskColor)
        audioDmaPanel.model.color = NSColor(info.audioColor)
        spriteDmaPanel.model.color = NSColor(info.spriteColor)
        bitplaneDmaPanel.model.color = NSColor(info.bitplaneColor)
    }
}
