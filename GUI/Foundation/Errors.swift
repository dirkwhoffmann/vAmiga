// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ErrorCode {

    var description: String {
        
        switch self {
        
        case .OK:
            fatalError()
        case .UNKNOWN:
            return "Unknown error type."
        case .FILE_NOT_FOUND:
            return "File not found."
        case .FILE_TYPE_MISMATCH:
            return "The content of the file does not match its format."
        case .FILE_CANT_READ:
            return "The file could not be opened for reading."
        case .FILE_CANT_WRITE:
            return "The file could not be opened for writing."
        case .FILE_CANT_CREATE:
            return "Unable to create file."
        case .OUT_OF_MEMORY:
            return "Out of memory."
        case .CHIP_RAM_LIMIT:
            return ""
        case .AROS_RAM_LIMIT:
            return ""
        case .ROM_MISSING:
            return "Missing Rom."
        case .AROS_NO_EXTROM:
            return "Missing Extension Rom."
        case .DISK_CANT_DECODE:
            return "Unable to decode the MFM bit stream."
        case .DISK_INVALID_DIAMETER:
            return "Invalid disk diameter."
        case .DISK_INVALID_DENSITY:
            return "Invalid disk density."
        case .SNP_TOO_OLD:
            return "The snapshot was created with an older version of " +
                "vAmiga and is incompatible with this release."
        case .SNP_TOO_NEW:
            return "The snapshot was created with a newer version of " +
                "vAmiga and is incompatible with this release."
        case .UNSUPPORTED_SNAPSHOT:
            return "Unsupported Snapshot Revision."
        case .MISSING_ROM_KEY:
            return "No key.rom file found."
        case .INVALID_ROM_KEY:
            return "Invalid Rom key."
        case .FS_UNKNOWN:
            return ""
        case .FS_UNSUPPORTED:
            return "Unsupported file system."
        case .FS_WRONG_BSIZE:
            return "Invalid block size."
        case .FS_WRONG_CAPACITY:
            return "Wrong file system capacity."
        case .FS_HAS_CYCLES:
            return "Cyclic reference chain detected."
        case .FS_CORRUPTED:
            return "Corrupted file system."
        case .FS_DIRECTORY_NOT_EMPTY:
            return "Directory is not empty."
        case .FS_CANNOT_CREATE_DIR:
            return "Unable to create directory."
        case .FS_CANNOT_CREATE_FILE:
            return "Unable to create file."
        case.FS_EXPECTED_VALUE,
            .FS_EXPECTED_SMALLER_VALUE,
            .FS_EXPECTED_DOS_REVISION,
            .FS_EXPECTED_NO_REF,
            .FS_EXPECTED_REF,
            .FS_EXPECTED_SELFREF,
            .FS_PTR_TO_UNKNOWN_BLOCK,
            .FS_PTR_TO_EMPTY_BLOCK,
            .FS_PTR_TO_BOOT_BLOCK,
            .FS_PTR_TO_ROOT_BLOCK,
            .FS_PTR_TO_BITMAP_BLOCK,
            .FS_PTR_TO_BITMAP_EXT_BLOCK,
            .FS_PTR_TO_USERDIR_BLOCK,
            .FS_PTR_TO_FILEHEADER_BLOCK,
            .FS_PTR_TO_FILELIST_BLOCK,
            .FS_PTR_TO_DATA_BLOCK,
            .FS_EXPECTED_DATABLOCK_NR,
            .FS_INVALID_HASHTABLE_SIZE:
            fatalError()
            
        default:
            return ""
        }
    }
}

class VAError: Error {
    
    var errorCode: ErrorCode
    
    init(_ errorCode: ErrorCode) { self.errorCode = errorCode }
}

enum MetalError: Error {

    case METAL_NOT_SUPPORTED
    case METAL_CANT_GET_DEVICE
    case METAL_CANT_GET_LAYER
    case METAL_CANT_CREATE_COMMAND_QUEUE
    case METAL_CANT_CREATE_SHADER_LIBARY
    case METAL_CANT_CREATE_TEXTURE
    
    var description: String {
        
        switch self {
        
        case .METAL_NOT_SUPPORTED:
            return "No suitable Metal hardware found."
        case .METAL_CANT_GET_DEVICE:
            return "Failed to obtain Metal device."
        case .METAL_CANT_GET_LAYER:
            return "Failed to obtain Core Animation layer."
        case .METAL_CANT_CREATE_COMMAND_QUEUE:
            return "Failed to create the command queue."
        case .METAL_CANT_CREATE_SHADER_LIBARY:
            return "Failed to create the shader library."
        case .METAL_CANT_CREATE_TEXTURE:
            return "Failed to create GPU texture."
        }
    }
}

extension Error {

    func alert(_ msg1: String, _ msg2: String, style: NSAlert.Style,
               async: Bool = false, icon: String?) {
    
        if async == true {
            DispatchQueue.main.async {
                self.alert(msg1, msg2, style: style, async: false, icon: icon)
            }
        }
    
        var image: NSImage?
        if icon != nil {
            image = NSImage.init(named: icon!)
        } else {
            if self is MetalError { image = NSImage.init(named: "metal") }
        }
        
        let alert = NSAlert()
        alert.alertStyle = style
        alert.icon = image
        alert.messageText = msg1
        alert.informativeText = msg2
        alert.addButton(withTitle: "OK")
        alert.runModal()
    }
    
    func informational(_ msg1: String, _ msg2: String,
                       async: Bool = false, icon: String? = nil) {
        
        alert(msg1, msg2, style: .informational, async: async, icon: nil)
    }
    
    func warning(_ msg1: String, _ msg2: String,
                 async: Bool = false, icon: String? = nil) {
        
        alert(msg1, msg2, style: .warning, async: async, icon: icon)
    }
    
    func critical(_ msg1: String, _ msg2: String,
                  async: Bool = false, icon: String? = nil) {
        
        alert(msg1, msg2, style: .critical, async: async, icon: icon)
    }

    func warning(_ msg: String, async: Bool = false, icon: String? = nil) {

        if let error = self as? VAError {
            warning(msg, error.errorCode.description, async: async, icon: icon)
        }
        if let error = self as? MetalError {
            warning(msg, error.description, async: async, icon: icon)
        }
    }

    func critical(_ msg: String, async: Bool = false, icon: String? = nil) {

        if let error = self as? VAError {
            warning(msg, error.errorCode.description, async: async, icon: icon)
        }
        if let error = self as? MetalError {
            warning(msg, error.description, async: async, icon: icon)
        }
    }
    
    //
    // Customized alerts
    //
    
    func cantOpen(url: URL, async: Bool = false) {
        
        warning("\"\(url.lastPathComponent)\" can't be opened.", async: async)
    }
}
