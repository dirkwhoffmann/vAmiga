// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Dashboard: DialogController {

    @IBOutlet weak var chipRamPanel: ChipRamPanel!
    @IBOutlet weak var slowRamPanel: SlowRamPanel!
    @IBOutlet weak var fastRamPanel: FastRamPanel!
    @IBOutlet weak var romPanel: RomPanel!
    
    @IBOutlet weak var copperDmaPanel: CopperDmaPanel!
    @IBOutlet weak var blitterDmaPanel: BlitterDmaPanel!
    @IBOutlet weak var diskDmaPanel: TimeLinePanel!
    @IBOutlet weak var audioDmaPanel: TimeLinePanel!
    @IBOutlet weak var spriteDmaPanel: TimeLinePanel!
    @IBOutlet weak var bitplaneDmaPanel: TimeLinePanel!
    
    @IBOutlet weak var hostLoad: GaugePanel!
    @IBOutlet weak var hostFps: GaugePanel!

    @IBOutlet weak var amigaMhz: GaugePanel!
    @IBOutlet weak var amigaFps: GaugePanel!

    @IBOutlet weak var ciaAPanel: GaugePanel!
    @IBOutlet weak var ciaBPanel: GaugePanel!

    @IBOutlet weak var audioBufferPanel: GaugePanel!
    @IBOutlet weak var waveformPanelL: WaveformPanel!
    @IBOutlet weak var waveformPanelR: WaveformPanel!

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
    
        // Host
        let stats = emu.stats
        hostLoad.model.add(stats.cpuLoad)
        hostFps.model.add(parent.speedometer.gpuFps)

        // Amiga
        amigaFps.model.add(parent.speedometer.emuFps)
        amigaMhz.model.add(parent.speedometer.mhz)
        
        // CIAs
        ciaAPanel.model.add(emu.ciaA.stats.idlePercentage)
        ciaBPanel.model.add(emu.ciaB.stats.idlePercentage)

        // Audio
        audioBufferPanel.model.add(emu.audioPort.stats.fillLevel)
        
        waveformPanelL.update()
        waveformPanelR.update()
        
        
    }
    
    func refresh() {
               
        let info = emu.dmaDebugger.info
        
        // Colors
        /*
        let memColor = NSColor.white
        chipRamPanel.themeColor = memColor
        chipRamPanel.graph1Color = NSColor.init(r: 0x33, g: 0x99, b: 0xFF)
        chipRamPanel.graph2Color = NSColor.init(r: 0xFF, g: 0x33, b: 0x99)
        
        slowRamPanel.model.themeColor = memColor
        slowRamPanel.model.graph1Color = NSColor.init(r: 0x33, g: 0x99, b: 0xFF)
        slowRamPanel.model.graph2Color = NSColor.init(r: 0xFF, g: 0x33, b: 0x99)
        
        fastRamPanel.model.themeColor = memColor
        fastRamPanel.model.graph1Color = NSColor.init(r: 0x33, g: 0x99, b: 0xFF)
        fastRamPanel.model.graph2Color = NSColor.init(r: 0xFF, g: 0x33, b: 0x99)
        
        romPanel.model.themeColor = memColor
        romPanel.model.graph1Color = NSColor.init(r: 0x33, g: 0x99, b: 0xFF)
        romPanel.model.graph2Color = NSColor.init(r: 0xFF, g: 0x33, b: 0x99)
        */
        copperDmaPanel.themeColor = NSColor(info.copperColor)
        blitterDmaPanel.themeColor = NSColor(info.blitterColor)
        diskDmaPanel.themeColor = NSColor(info.diskColor)
        audioDmaPanel.themeColor = NSColor(info.audioColor)
        spriteDmaPanel.themeColor = NSColor(info.spriteColor)
        bitplaneDmaPanel.themeColor = NSColor(info.bitplaneColor)
    }
}
