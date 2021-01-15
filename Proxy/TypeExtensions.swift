// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension EventSlot {

    var description: String {
        
        switch self {
        
        case .REG:   return "Registers"
        case .RAS:   return "Rasterline"
        case .CIAA:  return "CIA A"
        case .CIAB:  return "CIA B"
        case .BPL:   return "Bitplane DMA"
        case .DAS:   return "Other DMA"
        case .COP:   return "Copper"
        case .BLT:   return "Blitter"
        case .SEC:   return "Secondary"

        case .CH0:   return "Audio channel 0"
        case .CH1:   return "Audio channel 1"
        case .CH2:   return "Audio channel 2"
        case .CH3:   return "Audio channel 3"
        case .DSK:   return "Disk Controller"
        case .DCH:   return "Disk Change"
        case .VBL:   return "Vertical blank"
        case .IRQ:   return "Interrupts"
        case .IPL:   return "IPL"
        case .KBD:   return "Keyboard"
        case .TXD:   return "UART out"
        case .RXD:   return "UART in"
        case .POT:   return "Potentiometer"
        case .INS:   return "Inspector"
        case .COUNT: return "???"
        default:     fatalError()
        }
    }
}

extension FSVolumeType {

    var description: String {
        
        switch self {
        case .NODOS:    return "Unknown File System"
        case .OFS:      return "Original File System (OFS)"
        case .OFS_INTL: return "Original File System (OFS-INTL)"
        case .OFS_DC:   return "Original File System (OFS-DC)"
        case .OFS_LNFS: return "Original File System (OFS-LNFS)"
        case .FFS:      return "Fast File System (FFS)"
        case .FFS_INTL: return "Fast File System (FFS-INTL)"
        case .FFS_DC:   return "Fast File System (FFS-DC)"
        case .FFS_LNFS: return "Fast File System (FFS-LNFS)"
        default:        fatalError()
        }
    }
}

extension FSBlockType {
    
    var description: String {
        
        switch self {
        case .UNKNOWN_BLOCK:    return "Unknown block type"
        case .EMPTY_BLOCK:      return "Empty Block"
        case .BOOT_BLOCK:       return "Boot Block"
        case .ROOT_BLOCK:       return "Root Block"
        case .BITMAP_BLOCK:     return "Bitmap Block"
        case .BITMAP_EXT_BLOCK: return "Bitmap Extension Block"
        case .USERDIR_BLOCK:    return "User Directory Block"
        case .FILEHEADER_BLOCK: return "File Header Block"
        case .FILELIST_BLOCK:   return "File List Block"
        case .DATA_BLOCK_OFS:   return "Data Block (OFS)"
        case .DATA_BLOCK_FFS:   return "Data Block (FFS)"
        default:                fatalError()
        }
    }
}

extension FSItemType {
    
    var description: String {
        
        switch self {
        case .FSI_UNKNOWN:     return "Unknown"
        case .FSI_UNUSED:      return "Unused"
        case .FSI_DOS_HEADER:  return "AmigaDOS header signature"
        case .FSI_DOS_VERSION: return "AmigaDOS version number"
        case .FSI_BOOTCODE:    return "Boot code instruction"
        case .FSI_TYPE_ID:     return "Type identifier"
        case .FSI_SUBTYPE_ID:  return "Subtype identifier"
        case .FSI_SELF_REF:    return "Block reference to itself"
        case .FSI_CHECKSUM:    return "Checksum"
        case .FSI_HASHTABLE_SIZE: return "Hashtable size"
        case .FSI_HASH_REF:       return "Hashtable entry"
        case .FSI_PROT_BITS:          return "Protection status bits"
        case .FSI_BCPL_STRING_LENGTH: return "BCPL string Length"
        case .FSI_BCPL_DISK_NAME:     return "Disk name (BCPL character)"
        case .FSI_BCPL_DIR_NAME:      return "Directory name (BCPL character)"
        case .FSI_BCPL_FILE_NAME:     return "File name (BCPL character)"
        case .FSI_BCPL_COMMENT:       return "Comment (BCPL character)"
        case .FSI_CREATED_DAY:        return "Creation date (days)"
        case .FSI_CREATED_MIN:        return "Creation date (minutes)"
        case .FSI_CREATED_TICKS:      return "Creation date (ticks)"
        case .FSI_MODIFIED_DAY:       return "Modification date (day)"
        case .FSI_MODIFIED_MIN:       return "Modification date (minutes)"
        case .FSI_MODIFIED_TICKS:     return "Modification date (ticks)"
        case .FSI_NEXT_HASH_REF:      return "Reference to the next hash block"
        case .FSI_PARENT_DIR_REF:     return "Parent directory block reference"
        case .FSI_FILEHEADER_REF:     return "File header block reference"
        case .FSI_EXT_BLOCK_REF:      return "Next extension block reference"
        case .FSI_BITMAP_BLOCK_REF:   return "Bitmap block reference"
        case .FSI_BITMAP_EXT_BLOCK_REF:   return "Extension bitmap block reference"
        case .FSI_BITMAP_VALIDITY:  return "Bitmap validity bits"
        case .FSI_DATA_BLOCK_REF_COUNT: return "Number of data block references"
        case .FSI_FILESIZE:            return "File size"
        case .FSI_DATA_BLOCK_NUMBER:  return "Position in the data block chain"
        case .FSI_DATA_BLOCK_REF:       return "Data block reference"
        case .FSI_FIRST_DATA_BLOCK_REF:  return "Reference to the first data block"
        case .FSI_NEXT_DATA_BLOCK_REF:  return "Reference to next data block"
        case .FSI_DATA_COUNT:           return "Number of stored data bytes"
        case .FSI_DATA:                 return "Data byte"
        case .FSI_BITMAP:              return "Block allocation table"
        default:
            fatalError()
        }
    }
}

extension FSError {
    
    func description(expected exp: Int = 0) -> String {
        
        switch self {
        case .OK:
            return ""
        case .EXPECTED_VALUE:
            return String.init(format: "Expected $%02X", exp)
        case .EXPECTED_SMALLER_VALUE:
            return String.init(format: "Expected a value less or equal $%02X", exp)
        case .EXPECTED_DOS_REVISION:
            return "Expected a value between 0 and 7"
        case .EXPECTED_NO_REF:
            return "Did not expect a block reference here"
        case .EXPECTED_REF:
            return "Expected a block reference"
        case .EXPECTED_SELFREF:
            return "Expected a self-reference"
        case .PTR_TO_UNKNOWN_BLOCK:
            return "This reference points to a block of unknown type"
        case .PTR_TO_EMPTY_BLOCK:
            return "This reference points to an empty block"
        case .PTR_TO_BOOT_BLOCK:
            return "This reference points to a boot block"
        case .PTR_TO_ROOT_BLOCK:
            return "This reference points to the root block"
        case .PTR_TO_BITMAP_BLOCK:
            return "This reference points to a bitmap block"
        case .PTR_TO_USERDIR_BLOCK:
            return "This reference points to a user directory block"
        case .PTR_TO_FILEHEADER_BLOCK:
            return "This reference points to a file header block"
        case .PTR_TO_FILELIST_BLOCK:
            return "This reference points to a file header block"
        case .PTR_TO_DATA_BLOCK:
            return "This reference points to a data block"
        case .EXPECTED_DATABLOCK_NR:
            return "Invalid data block position number"
        case .INVALID_HASHTABLE_SIZE:
            return "Expected $48 (72 hash table entries)"
        default:
            track("\(self)")
            fatalError()
        }
    }
}
