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
    
    // var emu: EmulatorProxy! { return inspector.parent.emu }
    
    override func validateVisibleItems() {

    }
    
    func updateToolbar(full: Bool) {
        
        if full {
            
        }
    }
    
    //
    // Action methods
    //
    
    @IBAction func plusAction(_ sender: NSButton) {

        print("plusAction")
        // inspector.parent.addInspector()
    }

    @IBAction func panelAction(_ sender: NSPopUpButton) {
        
        dashboard.type = PanelType(rawValue: sender.selectedTag())!
    }
}
