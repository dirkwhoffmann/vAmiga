// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

struct InputDevice {
    static let none = -1
    static let mouse = 0
    static let keyset1 = 1
    static let keyset2 = 2
    static let joystick1 = 3
    static let joystick2 = 4
}

extension MyController {

    @IBAction func port1Action(_ sender: NSPopUpButton) {
        
        config.gameDevice1 = sender.selectedTag()
    }
 
    @IBAction func port2Action(_ sender: NSPopUpButton) {
        
        config.gameDevice2 = sender.selectedTag()
    }
    
    @IBAction func inspectAction(_ sender: NSSegmentedControl) {

        switch sender.selectedSegment {

        case 0: inspectorAction(sender)
        case 1: monitorAction(sender)

        default: assert(false)
        }
    }
    
    @IBAction func snapshotAction(_ sender: NSSegmentedControl) {
        
        switch sender.selectedSegment {
            
        case 0: takeSnapshotAction(self)
        case 1: restoreSnapshotAction(self)
        case 2: browseSnapshotsAction(self)
            
        default:
            assert(false)
        }
    }
    
    @IBAction func screenshotAction(_ sender: NSSegmentedControl) {
        
        track()
        
        switch sender.selectedSegment {
            
        case 0: takeScreenshotAction(self)
        case 1: browseScreenshotsAction(self)
            
        default:
            assert(false)
        }
    }
}
