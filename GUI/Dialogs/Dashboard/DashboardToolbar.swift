// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class DashboardToolbar: NSToolbar {
    
    @IBOutlet weak var dashboard: DashboardViewController!
    @IBOutlet weak var selector: NSPopUpButton!
    @IBOutlet weak var selectorItem: NSToolbarItem!
    
    override func validateVisibleItems() {

        // When the popup button is in shrinked state, macOS crashes when
        // it is selected. To prevent this from happening, we disable the
        // popup button when it is no longer visible.
        selector.isEnabled = selectorItem.isVisible
    }
    
    func updateToolbar(full: Bool) {
        
        if full {
            
        }
    }
    
    //
    // Action methods
    //
    
    @IBAction func plusAction(_ sender: NSButton) {

        dashboard.myController?.addDashboard()
    }

    @IBAction func panelAction(_ sender: NSPopUpButton) {
        
        dashboard.type = PanelType(rawValue: sender.selectedTag())!
    }
}
