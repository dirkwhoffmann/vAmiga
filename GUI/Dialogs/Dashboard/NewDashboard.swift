// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class DashboardWindowController: NSWindowController, NSWindowDelegate {
    
    var toolbar: DashboardToolbar? { return window?.toolbar as? DashboardToolbar }

    override func windowDidLoad() {
        
        super.windowDidLoad()

        // Register as delegate
        window?.delegate = self

        if let viewController = contentViewController as? DashboardViewController {
            
            // Connect the toolbar
            toolbar!.dashboard = viewController
            
            // Adjust the window size
            viewController.adjustSize()
        }
    }
    
    func windowDidResize(_ notification: Notification) {
            
        guard let window = notification.object as? NSWindow else { return }
            let newSize = window.frame.size
            print("Window resized: \(newSize.width) x \(newSize.height)")
        }
    
    @IBAction func buttonAction(_ sender: NSButton!) {
        
        print("Button pressed")
    }
}

class DashboardViewController: NSViewController {
    
    @IBOutlet weak var containerView: NSView!
            
    var type = PanelType.Combined { didSet { switchToPanel(type: type) } }
    
    let chipRamPanel = ChipRamPanel(frame: NSRect.init(x: 0, y: 0, width: 200, height: 128))
    let slowRamPanel = SlowRamPanel(frame: NSRect.init(x: 0, y: 0, width: 200, height: 128))
    let fastRamPanel = FastRamPanel(frame: NSRect.init(x: 0, y: 0, width: 200, height: 128))
    let romPanel = RomPanel(frame: NSRect.init(x: 0, y: 0, width: 200, height: 128))
    let copperDmaPanel = CopperDmaPanel(frame: NSRect.init(x: 0, y: 0, width: 200, height: 128))
    let blitterDmaPanel = BlitterDmaPanel(frame: NSRect.init(x: 0, y: 0, width: 200, height: 128))
    let diskDmaPanel = DiskDmaPanel(frame: NSRect.init(x: 0, y: 0, width: 200, height: 128))
    let audioDmaPanel = AudioDmaPanel(frame: NSRect.init(x: 0, y: 0, width: 200, height: 128))
    let spriteDmaPanel = SpriteDmaPanel(frame: NSRect.init(x: 0, y: 0, width: 200, height: 128))
    let bitplaneDmaPanel = BitplaneDmaPanel(frame: NSRect.init(x: 0, y: 0, width: 200, height: 128))
    
    let proposedSize: [NSRect] = [ NSRect(x: 0, y: 0, width: 600, height: 740),
                                   NSRect(x: 0, y: 0, width: 450, height: 240) ]
    
    func getPanel(type: PanelType) -> DashboardPanel? {
        
        switch type {
        case .ChipRam: return chipRamPanel
        case .SlowRam: return slowRamPanel
        case .FastRam: return fastRamPanel
        case .Rom: return romPanel
        case .CopperDma: return copperDmaPanel
        case .BlitterDma: return blitterDmaPanel
        case .DiskDma: return diskDmaPanel
        case .AudioDma: return audioDmaPanel
        case .SpriteDma: return spriteDmaPanel
        case .BitplaneDma: return bitplaneDmaPanel
        default: return nil
        }
    }
    
    func proposedSize(for type: PanelType) -> NSRect {
        
        return type == .Combined ? proposedSize[0] : proposedSize[1]
    }
    
    override func viewDidLoad() {
        
        print("MyViewController: viewDidLoad")

        super.viewDidLoad()
    }
    
    @IBAction func buttonAction(_ sender: NSButton!) {
        
        print("MyViewController: Button pressed")
    }
    
    func switchToPanel(type: PanelType) {
                
        switch type {
            
        case .Combined:
            
            switchToViewController(identifier: "ViewController1", frameRect: proposedSize[0])
            
        default:
            
            switchToViewController(identifier: "ViewController2", frameRect: proposedSize[1])
            
            if let currentController = children.first, let panel = getPanel(type: type) {

                currentController.view.addSubview(panel)

                // Make the added view span the entire area
                panel.translatesAutoresizingMaskIntoConstraints = false
                NSLayoutConstraint.activate([
                    panel.leadingAnchor.constraint(equalTo: currentController.view.leadingAnchor),
                    panel.trailingAnchor.constraint(equalTo: currentController.view.trailingAnchor),
                    panel.topAnchor.constraint(equalTo: currentController.view.topAnchor),
                    panel.bottomAnchor.constraint(equalTo: currentController.view.bottomAnchor)
                ])
            }
        }
    }
    
    private func switchToViewController(identifier: String, frameRect: NSRect) {
                            
        // Get the storyboard from the resources bundle
        let storyboard = NSStoryboard(name: "StoryDashboard", bundle: nil)

        // Load the new view controller from the storyboard
        if let newController = storyboard.instantiateController(withIdentifier: identifier) as? NSViewController {
            
            if let currentController = children.first  {
                
                // Remove the current view controller and its view
                currentController.view.removeFromSuperview()
                currentController.removeFromParent()
                
                // Add the new view controller and its view
                let newView = newController.view
                addChild(newController)
                containerView.addSubview(newView)
                
                // Adjust the window size to the new panel type
                adjustSize()
            }
        }
    }
    
    func adjustSize() {
        
        // Get the current view controller
        guard let controller = children.first else { return }
                
        // Resize the window
        view.frame = proposedSize(for: type)
        view.window?.setContentSize(view.frame.size)
        controller.view.frame = containerView.bounds
        controller.view.autoresizingMask = [.width, .height]


        // Make the new view span the entire area
        controller.view.translatesAutoresizingMaskIntoConstraints = false
        NSLayoutConstraint.activate([
            controller.view.topAnchor.constraint(equalTo: containerView.topAnchor),
            controller.view.bottomAnchor.constraint(equalTo: containerView.bottomAnchor),
            controller.view.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            controller.view.trailingAnchor.constraint(equalTo: containerView.trailingAnchor)
        ])
    }
}

