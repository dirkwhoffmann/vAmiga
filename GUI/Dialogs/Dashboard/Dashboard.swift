// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import SwiftUI

enum PanelType: Int {
    
    case Combined       = 0
    case ChipRam        = 1
    case SlowRam        = 2
    case FastRam        = 3
    case Rom            = 4
    case CopperDma      = 5
    case BlitterDma     = 6
    case DiskDma        = 7
    case AudioDma       = 8
    case SpriteDma      = 9
    case BitplaneDma    = 10
    case CIAA           = 11
    case CIAB           = 12
    case AmigaMhz       = 13
    case AmigaFps       = 14
    case HostLoad       = 15
    case HostFps        = 16
    case AudioFillLevel = 17
    case WaveformL      = 18
    case WaveformR      = 19
}

class OverviewController : NSViewController {
    
    @IBOutlet weak var chipRamBox: NSBox!
    @IBOutlet weak var slowRamBox: NSBox!
    @IBOutlet weak var fastRamBox: NSBox!
    @IBOutlet weak var romBox: NSBox!
    @IBOutlet weak var copperDmaBox: NSBox!
    @IBOutlet weak var blitterDmaBox: NSBox!
    @IBOutlet weak var diskDmaBox: NSBox!
    @IBOutlet weak var audioDmaBox: NSBox!
    @IBOutlet weak var spriteDmaBox: NSBox!
    @IBOutlet weak var bitplaneDmaBox: NSBox!
    @IBOutlet weak var ciaABox: NSBox!
    @IBOutlet weak var ciaBBox: NSBox!
    @IBOutlet weak var hostLoadBox: NSBox!
    @IBOutlet weak var hostFpsBox: NSBox!
    @IBOutlet weak var amigaFpsBox: NSBox!
    @IBOutlet weak var amigaMhzBox: NSBox!
    @IBOutlet weak var fillLevelBox: NSBox!
    @IBOutlet weak var waveformLBox: NSBox!
    @IBOutlet weak var waveformRBox: NSBox!
}

class Dashboard: DialogController {
    
    var toolbar: DashboardToolbar? { return window?.toolbar as? DashboardToolbar }
    
    override func windowDidLoad() {
        
        super.windowDidLoad()
        
        // Register as delegate
        window?.delegate = self
        
        if let viewController = contentViewController as? DashboardViewController {
            
            // Connect the toolbar
            toolbar!.dashboard = viewController
            window!.toolbarStyle = .unified
            window!.titlebarAppearsTransparent = true
            
            // Switch to the default panel
            viewController.type = .Combined
        }
    }
    
    override func windowWillClose(_ notification: Notification) {

        super.windowWillClose(notification)

        // Unregister the dashboard
        if let index = parent.dashboards.firstIndex(where: { $0 === self }) {
            
            print("Removing dashboard at index \(index)")
            parent.dashboards.remove(at: index)
        }

        if parent.inspectors.isEmpty {
            
            print("All dashboards closed")
        }
    }
    
    func setController(_ controller: MyController) {
        
        parent = controller
        
        if let viewController = contentViewController as? DashboardViewController {
            viewController.myController = controller
            viewController.emu = emu
            viewController.waveformLPanel.audioPort = emu.audioPort
            viewController.waveformRPanel.audioPort = emu.audioPort
        }
    }
    
    func windowDidResize(_ notification: Notification) {
        
        // guard let window = notification.object as? NSWindow else { return }
        // print("New size: \(window.frame.size)")

        if let viewController = contentViewController as? DashboardViewController {
            viewController.windowDidResize(notification)
        }
    }
    
    func continuousRefresh() {
      
        (contentViewController as! DashboardViewController).continuousRefresh()
    }
    
    func processMessage(_ msg: Message) {
     
    }
}

class DashboardViewController: NSViewController {
    
    @IBOutlet weak var containerView: NSView!
    
    var emu: EmulatorProxy?
    var myController: MyController?
    
    var type: PanelType? { didSet { switchToPanel(type: type) } }
    
    var multiPanelController: NSViewController!
    var singlePanelController: NSViewController!
    
    let chipRamPanel = ChipRamPanel(frame: NSRect.zero)
    let slowRamPanel = SlowRamPanel(frame: NSRect.zero)
    let fastRamPanel = FastRamPanel(frame: NSRect.zero)
    let romPanel = RomPanel(frame: NSRect.zero)
    let copperDmaPanel = CopperDmaPanel(frame: NSRect.zero)
    let blitterDmaPanel = BlitterDmaPanel(frame: NSRect.zero)
    let diskDmaPanel = DiskDmaPanel(frame: NSRect.zero)
    let audioDmaPanel = AudioDmaPanel(frame: NSRect.zero)
    let spriteDmaPanel = SpriteDmaPanel(frame: NSRect.zero)
    let bitplaneDmaPanel = BitplaneDmaPanel(frame: NSRect.zero)
    let ciaAPanel = CIAAPanel(frame: NSRect.zero)
    let ciaBPanel = CIABPanel(frame: NSRect.zero)
    let amigaFpsPanel = AmigaFpsPanel(frame: NSRect.zero)
    let amigaMhzPanel = AmigaMhzPanel(frame: NSRect.zero)
    let hostLoadPanel = HostLoadPanel(frame: NSRect.zero)
    let hostFpsPanel = HostFpsPanel(frame: NSRect.zero)
    let fillLevelPanel = AudioFillLevelPanel(frame: NSRect.zero)
    let waveformLPanel = WaveformPanel(frame: NSRect.zero, channel: 0)
    let waveformRPanel = WaveformPanel(frame: NSRect.zero, channel: 1)
    
    override func viewDidLoad() {

        super.viewDidLoad()
   
        // Get the storyboard from the resources bundle
        let storyboard = NSStoryboard(name: "Dashboard", bundle: nil)

        // Load the view controllers from the storyboard
        multiPanelController = storyboard.instantiateController(withIdentifier: "ViewController1") as? NSViewController
        singlePanelController = storyboard.instantiateController(withIdentifier: "ViewController2") as? NSViewController
    }
    
    func windowDidResize(_ notification: Notification) {
    
    }
    
    func switchToPanel(type: PanelType?) {
        
        guard let type = type else { return }
        
        func add(_ subView: NSView, to parentView: NSView) {
            
            // Add padding to the SwiftUI view
            if let panel = subView as? DashboardPanel {
                if parentView is NSBox {
                    panel.padding = SwiftUICore.EdgeInsets(top: 8.0, leading: 8.0, bottom: 0.0, trailing: 8.0)
                } else {
                    panel.padding = SwiftUICore.EdgeInsets(top: 20.0, leading: 20.0, bottom: 20.0, trailing: 20.0)
                }
            }

            // Add the view
            parentView.addSubview(subView)
            (parentView as? NSBox)?.isTransparent = true
                        
            // Let the added view span the entire area of the parent view
            subView.translatesAutoresizingMaskIntoConstraints = false
            NSLayoutConstraint.activate([
                subView.leadingAnchor.constraint(equalTo: parentView.leadingAnchor),
                subView.trailingAnchor.constraint(equalTo: parentView.trailingAnchor),
                subView.topAnchor.constraint(equalTo: parentView.topAnchor),
                subView.bottomAnchor.constraint(equalTo: parentView.bottomAnchor)
            ])
        }
        
        switch type {
            
        case .Combined:
            
            switchToViewController(controller: multiPanelController)
            view.window?.minSize = NSSize(width: 400, height: 600)
            
            if let controller = children.first as? OverviewController {
                
                add(chipRamPanel, to: controller.chipRamBox)
                add(slowRamPanel, to: controller.slowRamBox)
                add(fastRamPanel, to: controller.fastRamBox)
                add(romPanel, to: controller.romBox)
                add(copperDmaPanel, to: controller.copperDmaBox)
                add(blitterDmaPanel, to: controller.blitterDmaBox)
                add(diskDmaPanel, to: controller.diskDmaBox)
                add(audioDmaPanel, to: controller.audioDmaBox)
                add(spriteDmaPanel, to: controller.spriteDmaBox)
                add(bitplaneDmaPanel, to: controller.bitplaneDmaBox)
                add(ciaAPanel, to: controller.ciaABox)
                add(ciaBPanel, to: controller.ciaBBox)
                add(amigaMhzPanel, to: controller.amigaMhzBox)
                add(amigaFpsPanel, to: controller.amigaFpsBox)
                add(hostLoadPanel, to: controller.hostLoadBox)
                add(hostFpsPanel, to: controller.hostFpsBox)
                add(fillLevelPanel, to: controller.fillLevelBox)
                add(waveformLPanel, to: controller.waveformLBox)
                add(waveformRPanel, to: controller.waveformRBox)
            }
        default:
            
            switchToViewController(controller: singlePanelController)
            view.window?.minSize = NSSize(width: 200, height: 160)

            if let view = children.first?.view {

                switch type {
                    
                case .ChipRam: add(chipRamPanel, to: view)
                case .SlowRam: add(slowRamPanel, to: view)
                case .FastRam: add(fastRamPanel, to: view)
                case .Rom: add(romPanel, to: view)
                case .CopperDma: add(copperDmaPanel, to: view)
                case .BlitterDma: add(blitterDmaPanel, to: view)
                case .DiskDma: add(diskDmaPanel, to: view)
                case .AudioDma: add(audioDmaPanel, to: view)
                case .SpriteDma: add(spriteDmaPanel, to: view)
                case .BitplaneDma: add(bitplaneDmaPanel, to: view)
                case .CIAA: add(ciaAPanel, to: view)
                case .CIAB: add(ciaBPanel, to: view)
                case .AmigaMhz: add(amigaMhzPanel, to: view)
                case .AmigaFps: add(amigaFpsPanel, to: view)
                case .HostLoad: add(hostLoadPanel, to: view)
                case .HostFps: add(hostFpsPanel, to: view)
                case .AudioFillLevel: add(fillLevelPanel, to: view)
                case .WaveformL: add(waveformLPanel, to: view)
                case .WaveformR: add(waveformRPanel, to: view)
                
                default:
                    fatalError()
                }
            }
        }
    }
    
    private func switchToViewController(controller newController: NSViewController) {
        
        if let currentController = children.first  {
            
            // Remove the current view controller and its view
            currentController.view.removeFromSuperview()
            currentController.removeFromParent()
                        
            // Add the new view controller and its view
            let newView = newController.view
            addChild(newController)
            containerView.addSubview(newView)
            
            // Resize the window
            view.window?.setContentSize(newView.frame.size)
            newController.view.frame = containerView.bounds
            newController.view.autoresizingMask = [.width, .height]
            
            // Make the new view span the entire area
            newController.view.translatesAutoresizingMaskIntoConstraints = false
            NSLayoutConstraint.activate([
                newController.view.topAnchor.constraint(equalTo: containerView.topAnchor),
                newController.view.bottomAnchor.constraint(equalTo: containerView.bottomAnchor),
                newController.view.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
                newController.view.trailingAnchor.constraint(equalTo: containerView.trailingAnchor)
            ])
        }
    }
    
    func continuousRefresh() {

        guard let emu = emu else { return }
        guard let myController = myController else { return }
        
        // Memory
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
        
        // DMA
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
        
        // Host
        let stats = emu.stats
        hostLoadPanel.model.add(stats.cpuLoad)
        hostFpsPanel.model.add(myController.speedometer.gpuFps)

        // Amiga
        amigaFpsPanel.model.add(myController.speedometer.emuFps)
        amigaMhzPanel.model.add(myController.speedometer.mhz)
        
        // CIAs
        ciaAPanel.model.add(1.0 - emu.ciaA.stats.idlePercentage)
        ciaBPanel.model.add(1.0 - emu.ciaB.stats.idlePercentage)

        // Audio
        fillLevelPanel.model.add(emu.audioPort.stats.fillLevel)
        waveformLPanel.update()
        waveformRPanel.update()
    }
}

