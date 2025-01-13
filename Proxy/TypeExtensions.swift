// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

typealias Accessor = vamiga.Accessor
typealias AmigaInfo = vamiga.AmigaInfo
typealias AgnusInfo = vamiga.AgnusInfo
typealias AgnusRevision = vamiga.AgnusRevision
typealias AgnusStats = vamiga.AgnusStats
typealias BankMap = vamiga.BankMap
typealias BlitterInfo = vamiga.BlitterInfo
typealias BootBlockId = vamiga.BootBlockId
typealias CIAInfo = vamiga.CIAInfo
typealias CIARevision = vamiga.CIARevision
typealias CIAStats = vamiga.CIAStats
typealias ControlPortDevice = vamiga.ControlPortDevice
typealias ControlPortInfo = vamiga.ControlPortInfo
typealias CopperInfo = vamiga.CopperInfo
typealias CPUInfo = vamiga.CPUInfo
typealias CPURevision = vamiga.CPURevision
typealias DasmNumbers = vamiga.DasmNumbers
typealias DasmSyntax = vamiga.DasmSyntax
typealias DeniseInfo = vamiga.DeniseInfo
typealias DeniseRevision = vamiga.DeniseRevision
typealias DiskControllerInfo = vamiga.DiskControllerInfo
typealias ErrorCode = vamiga.ErrorCode
typealias EventSlot = vamiga.EventSlot
typealias EventSlotInfo = vamiga.EventSlotInfo
typealias FileType = vamiga.FileType
typealias FloppyDriveInfo = vamiga.FloppyDriveInfo
typealias FloppyDriveType = vamiga.FloppyDriveType
typealias FSBlockType = vamiga.FSBlockType
typealias FSErrorReport = vamiga.FSErrorReport
typealias FSItemType = vamiga.FSItemType
typealias FSVolumeType = vamiga.FSVolumeType
typealias GamePadAction = vamiga.GamePadAction
typealias HardDriveInfo = vamiga.HardDriveInfo
typealias HIDEvent = vamiga.HIDEvent
typealias MemorySource = vamiga.MemorySource
typealias MemInfo = vamiga.MemInfo
typealias MemStats = vamiga.MemStats
typealias Message = vamiga.Message
typealias Option = vamiga.Option
typealias PaulaInfo = vamiga.PaulaInfo
typealias Probe = vamiga.Probe
typealias RomTraits = vamiga.RomTraits
typealias RTCRevision = vamiga.RTCRevision
typealias SerialPortDevice = vamiga.SerialPortDevice
typealias SerialPortInfo = vamiga.SerialPortInfo
typealias ServerType = vamiga.ServerType
typealias SpriteInfo = vamiga.SpriteInfo
typealias StateMachineInfo = vamiga.StateMachineInfo
typealias UARTInfo = vamiga.UARTInfo
typealias VideoFormat = vamiga.VideoFormat
typealias WarpMode = vamiga.WarpMode

extension EventSlot: CustomStringConvertible {

    public var description: String {
        
        switch self {
        
        case .REG:      return "Registers"
        case .CIAA:     return "CIA A"
        case .CIAB:     return "CIA B"
        case .BPL:      return "Bitplane DMA"
        case .DAS:      return "Other DMA"
        case .COP:      return "Copper"
        case .BLT:      return "Blitter"
        case .SEC:      return "Next Secondary Event"

        case .CH0:      return "Audio Channel 0"
        case .CH1:      return "Audio Channel 1"
        case .CH2:      return "Audio Channel 2"
        case .CH3:      return "Audio Channel 3"
        case .DSK:      return "Disk Controller"
        case .VBL:      return "Vertical Blank"
        case .IRQ:      return "Interrupts"
        case .IPL:      return "IPL"
        case .KBD:      return "Keyboard"
        case .TXD:      return "UART Out"
        case .RXD:      return "UART In"
        case .POT:      return "Potentiometer"
        case .TER:      return "Next Tertiary Event"

        case .DC0:      return "Disk Change Df0"
        case .DC1:      return "Disk Change Df1"
        case .DC2:      return "Disk Change Df2"
        case .DC3:      return "Disk Change Df3"
        case .HD0:      return "Hard Drive Hd0"
        case .HD1:      return "Hard Drive Hd1"
        case .HD2:      return "Hard Drive Hd2"
        case .HD3:      return "Hard Drive Hd3"
        case .MSE1:     return "Port 1 Mouse"
        case .MSE2:     return "Port 2 Mouse"
        case .SNP:      return "Snapshots"
        case .RSH:      return "Retro Shell"
        case .KEY:      return "Auto Typing"
        case .SRV:      return "Server Daemon"
        case .SER:      return "Null Modem Cable"
        case .BTR:      return "Beam Traps"
        case .ALA:      return "Alarms"
        case .INS:      return "Inspector"

        case .COUNT:    return "???"
            
        default:        fatalError()
        }
    }
}

extension FileType {

    init?(url: URL?) {
        self = url == nil ? .UNKNOWN : AmigaFileProxy.type(of: url)
    }

    static var all: [FileType] {
        return [ .SNAPSHOT, .SCRIPT, .ADF, .EADF, .HDF, .IMG, .DMS, .EXE, .DIR ]
    }

    static var draggable: [FileType] {
        return [ .SNAPSHOT, .SCRIPT, .ADF, .EADF, .HDF, .IMG, .DMS, .EXE, .DIR ]
    }
}

extension FSVolumeType: CustomStringConvertible {

    public var description: String {
        
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

extension FSBlockType: CustomStringConvertible {
    
    public var description: String {
        
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

extension FSItemType: CustomStringConvertible {
    
    public var description: String {
        
        switch self {
        
        case .UNKNOWN:               return "Unknown"
        case .UNUSED:                return "Unused"
        case .DOS_HEADER:            return "AmigaDOS header signature"
        case .DOS_VERSION:           return "AmigaDOS version number"
        case .BOOTCODE:              return "Boot code instruction"
        case .TYPE_ID:               return "Type identifier"
        case .SUBTYPE_ID:            return "Subtype identifier"
        case .SELF_REF:              return "Block reference to itself"
        case .CHECKSUM:              return "Checksum"
        case .HASHTABLE_SIZE:        return "Hashtable size"
        case .HASH_REF:              return "Hashtable entry"
        case .PROT_BITS:             return "Protection status bits"
        case .BCPL_STRING_LENGTH:    return "BCPL string Length"
        case .BCPL_DISK_NAME:        return "Disk name (BCPL character)"
        case .BCPL_DIR_NAME:         return "Directory name (BCPL character)"
        case .BCPL_FILE_NAME:        return "File name (BCPL character)"
        case .BCPL_COMMENT:          return "Comment (BCPL character)"
        case .CREATED_DAY:           return "Creation date (days)"
        case .CREATED_MIN:           return "Creation date (minutes)"
        case .CREATED_TICKS:         return "Creation date (ticks)"
        case .MODIFIED_DAY:          return "Modification date (day)"
        case .MODIFIED_MIN:          return "Modification date (minutes)"
        case .MODIFIED_TICKS:        return "Modification date (ticks)"
        case .NEXT_HASH_REF:         return "Reference to the next hash block"
        case .PARENT_DIR_REF:        return "Parent directory block reference"
        case .FILEHEADER_REF:        return "File header block reference"
        case .EXT_BLOCK_REF:         return "Next extension block reference"
        case .BITMAP_BLOCK_REF:      return "Bitmap block reference"
        case .BITMAP_EXT_BLOCK_REF:  return "Extension bitmap block reference"
        case .BITMAP_VALIDITY:       return "Bitmap validity bits"
        case .DATA_BLOCK_REF_COUNT:  return "Number of data block references"
        case .FILESIZE:              return "File size"
        case .DATA_BLOCK_NUMBER:     return "Position in the data block chain"
        case .DATA_BLOCK_REF:        return "Data block reference"
        case .FIRST_DATA_BLOCK_REF:  return "Reference to the first data block"
        case .NEXT_DATA_BLOCK_REF:   return "Reference to next data block"
        case .DATA_COUNT:            return "Number of stored data bytes"
        case .DATA:                  return "Data byte"
        case .BITMAP:                return "Block allocation table"
            
        default:
            fatalError()
        }
    }
}

extension ErrorCode {
    
    func description(expected exp: Int = 0) -> String {
        
        switch self {
        case .OK:
            return ""
        case .FS_EXPECTED_VALUE:
            return String(format: "Expected $%02X", exp)
        case .FS_EXPECTED_SMALLER_VALUE:
            return String(format: "Expected a value less or equal $%02X", exp)
        case .FS_EXPECTED_DOS_REVISION:
            return "Expected a value between 0 and 7"
        case .FS_EXPECTED_NO_REF:
            return "Did not expect a block reference here"
        case .FS_EXPECTED_REF:
            return "Expected a block reference"
        case .FS_EXPECTED_SELFREF:
            return "Expected a self-reference"
        case .FS_PTR_TO_UNKNOWN_BLOCK:
            return "This reference points to a block of unknown type"
        case .FS_PTR_TO_EMPTY_BLOCK:
            return "This reference points to an empty block"
        case .FS_PTR_TO_BOOT_BLOCK:
            return "This reference points to a boot block"
        case .FS_PTR_TO_ROOT_BLOCK:
            return "This reference points to the root block"
        case .FS_PTR_TO_BITMAP_BLOCK:
            return "This reference points to a bitmap block"
        case .FS_PTR_TO_USERDIR_BLOCK:
            return "This reference points to a user directory block"
        case .FS_PTR_TO_FILEHEADER_BLOCK:
            return "This reference points to a file header block"
        case .FS_PTR_TO_FILELIST_BLOCK:
            return "This reference points to a file header block"
        case .FS_PTR_TO_DATA_BLOCK:
            return "This reference points to a data block"
        case .FS_EXPECTED_DATABLOCK_NR:
            return "Invalid data block position number"
        case .FS_INVALID_HASHTABLE_SIZE:
            return "Expected $48 (72 hash table entries)"
        default:
            warn("\(self)")
            fatalError()
        }
    }
}
