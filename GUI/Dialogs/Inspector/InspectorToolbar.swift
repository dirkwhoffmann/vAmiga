// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class InspectorToolbar: NSToolbar {
    
    @IBOutlet weak var inspector: Inspector!

    override func validateVisibleItems() {

    }
    
    func updateToolbar() {
        
    }
    
    //
    // Action methods
    //
    
    @IBAction func panelAction(_ sender: NSPopUpButton) {

        print("Panel action \(sender.selectedTag())")
        inspector.selectPanel(sender.selectedTag())
    }
}
