// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Monitor: DialogController {

    // Panels
    @IBOutlet weak var myView0: DoubleTimeSeries!
    @IBOutlet weak var myView1: DoubleTimeSeries!
    @IBOutlet weak var myView2: DoubleTimeSeries!
    @IBOutlet weak var myView3: DoubleTimeSeries!
    @IBOutlet weak var myView4: TimeSeries!
    @IBOutlet weak var myView5: TimeSeries!
    @IBOutlet weak var myView6: TimeSeries!
    @IBOutlet weak var myView7: TimeSeries!
    @IBOutlet weak var myView8: TimeSeries!
    @IBOutlet weak var myView9: TimeSeries!
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
        myView4.model.add(Double(dma.copperActivity) / (313 * 120))
        myView5.model.add(Double(dma.blitterActivity) / (313 * 120))
        myView6.model.add(Double(dma.diskActivity) / (313 * 3))
        myView7.model.add(Double(dma.audioActivity) / (313 * 4))
        myView8.model.add(Double(dma.spriteActivity) / (313 * 16))
        myView9.model.add(Double(dma.bitplaneActivity) / 39330)
        
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
        myView0.model.add(chipW, chipR)
        myView1.model.add(slowW, slowR)
        myView2.model.add(fastW, fastR)
        myView3.model.add(kickW, kickR)
    
        // Audio monitors
        myView10.update()
        myView11.update()
    }
    
    func refresh() {
               
        let info = emu.dmaDebugger.info
        
        // Axis scaling
        myView4.model.logScale = true
        myView5.model.logScale = true

        // Colors
        myView4.model.color = NSColor(info.copperColor)
        myView5.model.color = NSColor(info.blitterColor)
        myView6.model.color = NSColor(info.diskColor)
        myView7.model.color = NSColor(info.audioColor)
        myView8.model.color = NSColor(info.spriteColor)
        myView9.model.color = NSColor(info.bitplaneColor)
    }
}
