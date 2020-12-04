// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

public extension AmigaProxy {
    
    func df(_ nr: Int) -> DriveProxy? {
        
        switch nr {
            
        case 0: return df0
        case 1: return df1
        case 2: return df2
        case 3: return df3
        default:return nil
        }
    }
    
    func df(_ item: NSButton!) -> DriveProxy? {
        
        return df(item.tag)
    }
    
    func df(_ item: NSMenuItem!) -> DriveProxy? {
        
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
        // image.makeGlossy()
        
        return image
    }
}

public extension DriveProxy {
    
    var icon: NSImage {

        var name: String

        if hasWriteProtectedDisk() {
            name = isModifiedDisk ? "diskUPTemplate" : "diskPTemplate"
        } else {
            name = isModifiedDisk ? "diskUTemplate" : "diskTemplate"
        }

        return NSImage.init(named: name)!
    }
}

extension FSVolumeType {

    var description: String {
        
        /*
        switch dos {
        case .OFS:      text = "Original File System (OFS)"
        case .OFS_INTL: text = "Original File System (OFS-INTL)"
        case .OFS_DC:   text = "Original File System (OFS-DC)"
        case .OFS_LNFS: text = "Original File System (OFS-LNFS)"
        case .FFS:      text = "Fast File System (FFS)"
        case .FFS_INTL: text = "Fast File System (FFS-INTL)"
        case .FFS_DC:   text = "Fast File System (FFS-DC)"
        case .FFS_LNFS: text = "Fast File System (FFS-LNFS)"
        default:        text = "Unknown file system"
        }
        */
        return "???"
    }
}
