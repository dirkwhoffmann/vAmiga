// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Dashboard: DialogController {

    @IBOutlet weak var chipRamPanel: DashboardPanel!
    @IBOutlet weak var slowRamPanel: DashboardPanel!
    @IBOutlet weak var fastRamPanel: DashboardPanel!
    @IBOutlet weak var romPanel: DashboardPanel!
    
    @IBOutlet weak var copperDmaPanel: DashboardPanel!
    @IBOutlet weak var blitterDmaPanel: DashboardPanel!
    @IBOutlet weak var diskDmaPanel: DashboardPanel!
    @IBOutlet weak var audioDmaPanel: DashboardPanel!
    @IBOutlet weak var spriteDmaPanel: DashboardPanel!
    @IBOutlet weak var bitplaneDmaPanel: DashboardPanel!
    
    @IBOutlet weak var hostLoad: DashboardPanel!
    @IBOutlet weak var hostFps: DashboardPanel!

    @IBOutlet weak var amigaMhz: DashboardPanel!
    @IBOutlet weak var amigaFps: DashboardPanel!

    @IBOutlet weak var ciaAPanel: DashboardPanel!
    @IBOutlet weak var ciaBPanel: DashboardPanel!

    @IBOutlet weak var audioBufferPanel: DashboardPanel!
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
        ciaAPanel.model.add(1.0 - emu.ciaA.stats.idlePercentage)
        ciaBPanel.model.add(1.0 - emu.ciaB.stats.idlePercentage)

        // Audio
        audioBufferPanel.model.add(emu.audioPort.stats.fillLevel)
        
        waveformPanelL.update()
        waveformPanelR.update()
    }
    
    func refresh() {
               
        let info = emu.dmaDebugger.info
        
        // Colors
        copperDmaPanel.themeColor = NSColor(info.copperColor)
        blitterDmaPanel.themeColor = NSColor(info.blitterColor)
        diskDmaPanel.themeColor = NSColor(info.diskColor)
        audioDmaPanel.themeColor = NSColor(info.audioColor)
        spriteDmaPanel.themeColor = NSColor(info.spriteColor)
        bitplaneDmaPanel.themeColor = NSColor(info.bitplaneColor)
    }
}
