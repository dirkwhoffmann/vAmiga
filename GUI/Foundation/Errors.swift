// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class VC64Error: Error {
    
    var errorCode: ErrorCode
    
    init(_ errorCode: ErrorCode) { self.errorCode = errorCode }
    
    var description: String {
        
        switch errorCode {
        
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
    
    static func alert(_ msg1: String, _ msg2: String, style: NSAlert.Style,
                      async: Bool = false, icon: String?) {
        
        if async == true {
            DispatchQueue.main.async {
                alert(msg1, msg2, style: style, async: false, icon: icon)
            }
        } else {
            let alert = NSAlert()
            alert.alertStyle = style
            if icon != nil { alert.icon = NSImage.init(named: icon!) }
            alert.messageText = msg1
            alert.informativeText = msg2
            alert.addButton(withTitle: "OK")
            alert.runModal()
        }
    }

    static func informational(_ msg1: String, _ msg2: String,
                              async: Bool = false, icon: String? = nil) {
        alert(msg1, msg2, style: .informational, async: async, icon: icon)
    }

    static func warning(_ msg1: String, _ msg2: String,
                        async: Bool = false, icon: String? = nil) {
        alert(msg1, msg2, style: .warning, async: async, icon: icon)
    }
    
    static func critical(_ msg1: String, _ msg2: String,
                         async: Bool = false, icon: String? = nil) {
        alert(msg1, msg2, style: .critical, async: async, icon: icon)
    }

    func warning(_ msg: String, async: Bool = false, icon: String? = nil) {
        VC64Error.warning(msg, description, async: async, icon: icon)
    }

    func critical(_ msg: String, async: Bool = false, icon: String? = nil) {
        VC64Error.warning(msg, description, async: async, icon: icon)
    }
    
    //
    // Customized alerts
    //
    
    func cantOpen(url: URL, async: Bool = false) {
        warning("\"\(url.lastPathComponent)\" can't be opened.", async: async)
    }
}
