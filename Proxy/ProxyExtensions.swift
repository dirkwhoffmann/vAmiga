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

extension DiskFileProxy {
    
    func icon(protected: Bool) -> NSImage {
        
        let density = diskDensity
        
        var name: String
        switch type {
        case .FILETYPE_ADF, .FILETYPE_DMS, .FILETYPE_EXE, .FILETYPE_DIR:
            name = density == .DISK_HD ? "hd_adf" : "dd_adf"
        case .FILETYPE_IMG:
            name = "dd_dos"
        default:
            name = ""
        }
        
        if protected { name += "_protected" }
        return NSImage.init(named: name)!
    }
    
    var layoutInfo: String {
        
        var result = numSides == 1 ? "Single sided" : "Double sided"

        if diskDensity == .DISK_SD { result += ", single density" }
        if diskDensity == .DISK_DD { result += ", double density" }
        if diskDensity == .DISK_HD { result += ", high density" }

        result += " disk, \(numTracks) tracks with \(numSectors) sectors each"
        return result
    }
    
    var bootInfo: String {
        
        let name = bootBlockName!
        
        if bootBlockType == .BB_VIRUS {
            return "Contagious boot block (\(name))"
        } else {
            return name
        }
    }
}

extension HDFFileProxy {
    
    func icon() -> NSImage {
        
        return NSImage.init(named: "hdf")!
    }
    
    var layoutInfo: String {
        
        let capacity = numBlocks / 2000
        return "\(capacity) MB (\(numBlocks) sectors)"
    }
    
    var bootInfo: String {

        return ""
    }
}

extension FSVolumeType {

    var description: String {
        
        switch self {
        case .OFS:      return "Original File System (OFS)"
        case .OFS_INTL: return "Original File System (OFS-INTL)"
        case .OFS_DC:   return "Original File System (OFS-DC)"
        case .OFS_LNFS: return "Original File System (OFS-LNFS)"
        case .FFS:      return "Fast File System (FFS)"
        case .FFS_INTL: return "Fast File System (FFS-INTL)"
        case .FFS_DC:   return "Fast File System (FFS-DC)"
        case .FFS_LNFS: return "Fast File System (FFS-LNFS)"
        default:        return "Unknown file system"
        }
    }
}

extension NSAlert {

    static func warning(_ msg1: String, _ msg2: String, icon: String? = nil) {

        alert(msg1, msg2, style: .warning, icon: icon)
    }

    static func critical(_ msg1: String, _ msg2: String, icon: String? = nil) {
        
        alert(msg1, msg2, style: .critical, icon: icon)
    }
    
    static func alert(_ msg1: String, _ msg2: String, style: NSAlert.Style, icon: String?) {
        
        let alert = NSAlert()
        alert.alertStyle = style
        if icon != nil { alert.icon = NSImage.init(named: icon!) }
        alert.messageText = msg1
        alert.informativeText = msg2
        alert.addButton(withTitle: "OK")
        alert.runModal()
    }
}

extension FileError {
    
    func showAlert(_ str: String? = nil) {
        
        switch self {
        
        case .ERR_FILE_OK:
            return
            
        case .ERR_FILE_NOT_FOUND:
            NSAlert.warning("File " + str! + " could not be opened.",
                            "The file does not exists")
            
        case .ERR_INVALID_TYPE:
            NSAlert.warning("File " + str! + " could not be opened.",
                            "The file format does not match")
            
        case .ERR_CANT_READ:
            NSAlert.warning("Can't read from file " + str! + ".",
                            "The file cannot be opened.")
            
        case .ERR_CANT_WRITE:
            NSAlert.warning("Can't write to file " + str! + ".",
                            "The file cannot be opened.")
            
        case .ERR_OUT_OF_MEMORY:
            NSAlert.warning("The file operation cannot be performed.",
                            "Not enough memory.")
            
        case .ERR_UNSUPPORTED_SNAPSHOT:
            NSAlert.warning("Snapshot " + str! + " could not be opened.",
                            "The file was created with a different version of vAmiga.")

        case .ERR_MISSING_ROM_KEY:
            NSAlert.warning("Failed to decrypt the selected Rom image.",
                            "A rom.key file is required to process this file.")
            
        case .ERR_INVALID_ROM_KEY:
            NSAlert.warning("Failed to decrypt the selected Rom image.",
                            "Decrypting the Rom with the provided rom.key " +
                                "file did not produce a valid Rom image.")
            
        default:
            NSAlert.warning("The operation cannot be performed.",
                            "An uncategorized error exception has been thrown.")
        }
    }
}
