// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension MyController {
    
    public func refreshStatusBar() {
        
        guard let amiga = amigaProxy else { return }
        let running = amiga.isRunning()

        let dc = amiga.paula.getDiskControllerConfig()
        let df0Connected = dc.connected.0
        let df1Connected = dc.connected.1
        let df2Connected = dc.connected.2
        let df3Connected = dc.connected.3

        amiga.df0.inspect()
        amiga.df1.inspect()
        amiga.df2.inspect()
        amiga.df3.inspect()
        let info0 = amiga.df0.getInfo()
        let info1 = amiga.df1.getInfo()
        let info2 = amiga.df2.getInfo()
        let info3 = amiga.df3.getInfo()

        /*
        assert(info0.motor == amiga.diskController.spinning(0))
        assert(info1.motor == amiga.diskController.spinning(1))
        assert(info2.motor == amiga.diskController.spinning(2))
        assert(info3.motor == amiga.diskController.spinning(3))


        let df0spinning = amiga.diskController.spinning(0)
        let df1spinning = amiga.diskController.spinning(1)
        let df2spinning = amiga.diskController.spinning(2)
        let df3spinning = amiga.diskController.spinning(3)
         */

        // Icons
        df0Disk.image = amiga.df0.icon
        df1Disk.image = amiga.df1.icon
        df2Disk.image = amiga.df2.icon
        df3Disk.image = amiga.df3.icon
        warpIcon.image = hourglassIcon

        // Cylinders
        df0Cylinder.integerValue = Int(info0.head.cylinder)
        df1Cylinder.integerValue = Int(info1.head.cylinder)
        df2Cylinder.integerValue = Int(info2.head.cylinder)
        df3Cylinder.integerValue = Int(info3.head.cylinder)

        // Animation
        info0.motor && running ? df0DMA.startAnimation(self) : df0DMA.stopAnimation(self)
        info1.motor && running ? df1DMA.startAnimation(self) : df1DMA.stopAnimation(self)
        info2.motor && running ? df2DMA.startAnimation(self) : df2DMA.stopAnimation(self)
        info3.motor && running ? df3DMA.startAnimation(self) : df3DMA.stopAnimation(self)

        // Visibility
        let items: [NSView: Bool] = [
            
            powerLED: true,
            
            df0LED: df0Connected,
            df1LED: df1Connected,
            df2LED: df2Connected,
            df3LED: df3Connected,
            df0Disk: df0Connected && amiga.df0.hasDisk(),
            df1Disk: df1Connected && amiga.df1.hasDisk(),
            df2Disk: df2Connected && amiga.df2.hasDisk(),
            df3Disk: df3Connected && amiga.df3.hasDisk(),
            df0Cylinder: df0Connected,
            df1Cylinder: df1Connected,
            df2Cylinder: df2Connected,
            df3Cylinder: df3Connected,
            df0DMA: info0.motor,
            df1DMA: info1.motor,
            df2DMA: info2.motor,
            df3DMA: info3.motor,

            cmdLock: mapCommandKeys,
            
            clockSpeed: running,
            clockSpeedBar: running,
            warpIcon: running
        ]
        
        for (item, visible) in items {
            item.isHidden = !visible || !statusBar
        }
        
    }
    
    public func showStatusBar(_ value: Bool) {
        
        if statusBar != value {
            
            if value {
                
                metal.shrink()
                window?.setContentBorderThickness(24, for: .minY)
                adjustWindowSize()
                
            } else {
                
                metal.expand()
                window?.setContentBorderThickness(0, for: .minY)
                adjustWindowSize()
            }
            
            statusBar = value
            refreshStatusBar()
        }
    }
}
