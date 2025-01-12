// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class Dashboard: DialogController {
    
    var toolbar: DashboardToolbar? { return window?.toolbar as? DashboardToolbar }
    var viewController: DashboardViewController? { return contentViewController as? DashboardViewController }

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
            
            // print("Removing dashboard at index \(index)")
            parent.dashboards.remove(at: index)
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

        guard let window = notification.object as? NSWindow else { return }
        print("New size: \(window.frame.size)")

        window.titleVisibility = window.frame.size.width < 300 ? .hidden : .visible

        viewController?.windowDidResize(notification)
    }
    
    func continuousRefresh() {
      
        viewController?.continuousRefresh()
    }
    
    func processMessage(_ msg: Message) {
     
    }
}

