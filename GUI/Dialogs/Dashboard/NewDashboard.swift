// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class DashboardWindowController: NSWindowController {
    
    var toolbar: DashboardToolbar? { return window?.toolbar as? DashboardToolbar }

    override func windowDidLoad() {
        
        super.windowDidLoad()
        
        // Connect toolbar to the view controller
        toolbar!.dashboard = contentViewController! as? MyViewController
    }
    
    @IBAction func buttonAction(_ sender: NSButton!) {
        
        print("Button pressed")
    }
}

class MyContainerView : NSView {
    
    @IBOutlet weak var currentViewController: NSViewController!
    
}

class MyViewController: NSViewController {
    
    @IBOutlet weak var myButton: NSButton!
    @IBOutlet weak var containerView: NSView!
            
    override func viewDidLoad() {
        
        super.viewDidLoad()

        print("MyViewController: viewDidLoad")
    }
    
    @IBAction func buttonAction(_ sender: NSButton!) {
        
        print("MyViewController: Button pressed")
    }
    
    @IBAction func switchToOverview(_ sender: Any) {
        switchToViewController(identifier: "ViewController1")
    }
    
    @IBAction func switchToSingleton(_ sender: Any) {
        switchToViewController(identifier: "ViewController2")
    }
    
    private func switchToViewController(identifier: String) {
        
        // Load the new view controller from the storyboard
        let storyboard = NSStoryboard(name: "StoryDashboard", bundle: nil)
        guard let newViewController = storyboard.instantiateController(withIdentifier: identifier) as? NSViewController else {
            print("Failed to instantiate view controller with identifier \(identifier)")
            return
        }
        
        if let currentViewController = children.first {
            
            // Remove the current view controller's view
            currentViewController.view.removeFromSuperview()
            currentViewController.removeFromParent()
            
            // Add the new view controller's view
            addChild(newViewController)
            containerView.addSubview(newViewController.view)
            
            // Match the new view's frame to the container
            /*
            newViewController.view.frame = containerView.bounds
            newViewController.view.autoresizingMask = [.width, .height]
             */
            
             newViewController.view.layoutSubtreeIfNeeded()
             containerView.frame.size = newViewController.view.frame.size
            
            if let window = view.window {
                window.setContentSize(containerView.frame.size)
            }
        }
    }
}

class MyViewController1: NSViewController {
    
    @IBAction func buttonAction(_ sender: NSButton!) {
        
        print("MyViewController1: Button pressed")
    }
}

class MyViewController2: NSViewController {
    
    @IBAction func buttonAction(_ sender: NSButton!) {
        
        print("MyViewController2: Button pressed")
    }
}
