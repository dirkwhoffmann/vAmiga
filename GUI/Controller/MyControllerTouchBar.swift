//
// This file is part of VirtualC64 - A cycle accurate Commodore 64 emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
//

import Foundation

@available(OSX 10.12.2, *)
extension NSTouchBarItem.Identifier {
    
    static let rewind     = NSTouchBarItem.Identifier("com.virtualc64.TouchBarItem.rewind")
    static let snap       = NSTouchBarItem.Identifier("com.virtualc64.TouchBarItem.snap")
    static let revert     = NSTouchBarItem.Identifier("com.virtualc64.TouchBarItem.revert")
}

@available(OSX 10.12.2, *)
extension MyController : NSTouchBarDelegate {
    
    override open func makeTouchBar() -> NSTouchBar? {
 
        track()
        
        if (c64 == nil) {
            track("Cannot create touch bar (no C64 proxy).")
            return nil
        }

        let touchBar = NSTouchBar()
        touchBar.delegate = self

        // Configure items
        touchBar.defaultItemIdentifiers = [
      
            .rewind,
            .snap,
            .revert
        ]
        
        // Make touchbar customizable
        touchBar.customizationIdentifier = NSTouchBar.CustomizationIdentifier("com.vAmiga.touchbar")
        touchBar.customizationAllowedItemIdentifiers = [
    
            .rewind,
            .snap,
            .revert
        ]
        
        return touchBar
    }
    
    public func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        
        switch identifier {
            
        case NSTouchBarItem.Identifier.rewind:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let icon = NSImage(named: NSImage.Name("ttRewindTemplate"))!
            let resizedIcon = icon.resize(width: 24, height: 24)
            item.customizationLabel = "Rewind"
            item.view = NSButton(image: resizedIcon,
                                 target: self,
                                 action: #selector(restoreLatestAutoSnapshotAction(_:)))
            return item
            
        case NSTouchBarItem.Identifier.snap:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let icon = NSImage(named: NSImage.Name("ttStoreTemplate"))!
            let resizedIcon = icon.resize(width: 24, height: 24)
            item.customizationLabel = "Snap"
            item.view = NSButton(image: resizedIcon,
                                 target: self,
                                 action: #selector(takeSnapshot(_:)))
            return item
        
        case NSTouchBarItem.Identifier.revert:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let icon = NSImage(named: NSImage.Name("ttRestoreTemplate"))!
            let resizedIcon = icon.resize(width: 24, height: 24)
            item.customizationLabel = "Revert"
            item.view = NSButton(image: resizedIcon,
                                 target: self,
                                 action: #selector(restoreLatestUserSnapshotAction(_:)))
            return item
            
        default:
            return nil
        }
    }
}

