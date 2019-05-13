// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Foundation

public extension AmigaProxy {
    
    func df(_ nr: Int) -> DriveProxy {
        
        switch nr {
            
        case 0: return df0
        case 1: return df1
        case 2: return df2
        case 3: return df3

        default: fatalError()
        }
    }
    
    func df(_ item: NSButton!) -> DriveProxy {
        
        return df(item.tag)
    }
    
    func df(_ item: NSMenuItem!) -> DriveProxy {
        
        return df(item.tag)
    }
    
    func image(data: UnsafeMutablePointer<UInt8>?, size: NSSize) -> NSImage {
        
        var bitmap = data
        let width = Int(size.width)
        let height = Int(size.height)
        let imageRep = NSBitmapImageRep(bitmapDataPlanes: &bitmap,
                                        pixelsWide: width,
                                        pixelsHigh: height,
                                        bitsPerSample: 8,
                                        samplesPerPixel: 4,
                                        hasAlpha: true,
                                        isPlanar: false,
                                        colorSpaceName: NSColorSpaceName.calibratedRGB,
                                        bytesPerRow: 4*width,
                                        bitsPerPixel: 32)
        let image = NSImage(size: (imageRep?.size)!)
        image.addRepresentation(imageRep!)
        image.makeGlossy()
        
        return image
    }
    
    func autoSnapshotImage(_ item: Int) -> NSImage {
        
        let data = autoSnapshotImageData(item)
        return image(data: data, size: autoSnapshotImageSize(item))
    }
    
    func userSnapshotImage(_ item: Int) -> NSImage {
        
        let data = userSnapshotImageData(item)
        return image(data: data, size: userSnapshotImageSize(item))
    }
}

public extension DriveProxy {
    
    var icon: NSImage {
        get {
            var name: String
            
            if hasWriteProtectedDisk() {
                name = hasModifiedDisk() ? "diskUPTemplate" : "diskPTemplate"
            } else {
                name = hasModifiedDisk() ? "diskUTemplate" : "diskTemplate"
            }
            
            return NSImage.init(named: name)!
        }
    }
}
