// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#import "Constants.h"
#import "config.h"
#import "AgnusTypes.h"
#import "AmigaTypes.h"
#import "AmigaFileTypes.h"
#import "AudioPortTypes.h"
#import "BlitterTypes.h"
#import "BootBlockImageTypes.h"
#import "CmdQueueTypes.h"
#import "ControlPortTypes.h"
#import "CopperTypes.h"
#import "CPUTypes.h"
#import "CIATypes.h"
#import "DeniseTypes.h"
#import "DiskControllerTypes.h"
#import "DmaDebuggerTypes.h"
#import "ErrorTypes.h"
#import "FloppyDiskTypes.h"
#import "FloppyDriveTypes.h"
#import "FSTypes.h"
#import "HardDriveTypes.h"
#import "HdControllerTypes.h"
#import "JoystickTypes.h"
#import "KeyboardTypes.h"
#import "MemoryTypes.h"
#import "MsgQueueTypes.h"
#import "MouseTypes.h"
#import "PaulaTypes.h"
#import "PixelEngineTypes.h"
#import "RemoteManagerTypes.h"
#import "RetroShellTypes.h"
#import "RomFileTypes.h"
#import "RTCTypes.h"
#import "SerialPortTypes.h"
#import "StateMachineTypes.h"
#import "UARTTypes.h"
#import "ZorroBoardTypes.h"

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>


//
// Forward declarations
//

@class ADFFileProxy;
@class AgnusProxy;
@class AmigaProxy;
@class AmigaFileProxy;
@class BlitterProxy;
@class CIAProxy;
@class ControlPortProxy;
@class CopperProxy;
@class CPUProxy;
@class DebuggerProxy;
@class DeniseProxy;
@class DiskControllerProxy;
@class DiskFileProxy;
@class DMSFileProxy;
@class DmaDebuggerProxy;
@class FloppyDriveProxy;
@class EXEFileProxy;
@class ExtendedRomFileProxy;
@class FloppyFileProxy;
@class FolderProxy;
@class GuardsProxy;
@class HardDriveProxy;
@class HDFFileProxy;
@class IMGFileProxy;
@class JoystickProxy;
@class KeyboardProxy;
@class MemProxy;
@class MouseProxy;
@class PaulaProxy;
@class DefaultsProxy;
@class RemoteManagerProxy;
@class RetroShellProxy;
@class RomFileProxy;
@class RtcProxy;
@class RecorderProxy;
@class ScriptProxy;
@class SerialPortProxy;
@class SnapshotProxy;
@class STFileProxy;
@class VideoPortProxy;


//
// Exception wrapper
//

@interface ExceptionWrapper : NSObject {
    
    ErrorCode errorCode;
    NSString *what;
}

@property ErrorCode errorCode;
@property NSString *what;

@end


//
// Base proxies
//

@interface Proxy : NSObject {
    
    // Reference to the wrapped C++ object
    @public void *obj;    
}

@end

@interface CoreComponentProxy : Proxy { }

@property (readonly) NSInteger objid;

@end


//
// Constants
//

@interface Constants : NSObject {

}

@property (class, readonly) NSInteger vpos_cnt_pal;
@property (class, readonly) NSInteger vpos_max_pal;

@property (class, readonly) NSInteger vpos_cnt_ntsc;
@property (class, readonly) NSInteger vpos_max_ntsc;

@property (class, readonly) NSInteger vpos_cnt;
@property (class, readonly) NSInteger vpos_max;

@property (class, readonly) NSInteger hpos_cnt_pal;
@property (class, readonly) NSInteger hpos_max_pal;

@property (class, readonly) NSInteger hpos_cnt_ntsc;
@property (class, readonly) NSInteger hpos_max_ntsc;

@property (class, readonly) NSInteger hpos_max;
@property (class, readonly) NSInteger hpos_cnt;

@end


//
// Emulator
//

@interface EmulatorProxy : CoreComponentProxy {
        
    AgnusProxy *agnus;
    AmigaProxy *amiga;
    CIAProxy *ciaA;
    CIAProxy *ciaB;
    ControlPortProxy *controlPort1;
    ControlPortProxy *controlPort2;
    CopperProxy *copper;
    CPUProxy *cpu;
    DebuggerProxy *debugger;
    DeniseProxy *denise;
    DiskControllerProxy *diskController;
    DmaDebuggerProxy *dmaDebugger;
    FloppyDriveProxy *df0;
    FloppyDriveProxy *df1;
    FloppyDriveProxy *df2;
    FloppyDriveProxy *df3;
    GuardsProxy *breakpoints;
    GuardsProxy *watchpoints;
    HardDriveProxy *hd0;
    HardDriveProxy *hd1;
    HardDriveProxy *hd2;
    HardDriveProxy *hd3;
    KeyboardProxy *keyboard;
    MemProxy *mem;
    PaulaProxy *paula;
    DefaultsProxy *properties;
    RemoteManagerProxy *remoteManager;
    RetroShellProxy *retroShell;
    RtcProxy *rtc;
    RecorderProxy *recorder;
    SerialPortProxy *serialPort;
    VideoPortProxy *videoPort;
}

@property (readonly, strong) AgnusProxy *agnus;
@property (readonly, strong) AmigaProxy *amiga;
@property (readonly, strong) BlitterProxy *blitter;
@property (readonly, strong) CIAProxy *ciaA;
@property (readonly, strong) CIAProxy *ciaB;
@property (readonly, strong) ControlPortProxy *controlPort1;
@property (readonly, strong) ControlPortProxy *controlPort2;
@property (readonly, strong) CopperProxy *copper;
@property (readonly, strong) CPUProxy *cpu;
@property (readonly, strong) DebuggerProxy *debugger;
@property (readonly, strong) DeniseProxy *denise;
@property (readonly, strong) DiskControllerProxy *diskController;
@property (readonly, strong) DmaDebuggerProxy *dmaDebugger;
@property (readonly, strong) FloppyDriveProxy *df0;
@property (readonly, strong) FloppyDriveProxy *df1;
@property (readonly, strong) FloppyDriveProxy *df2;
@property (readonly, strong) FloppyDriveProxy *df3;
@property (readonly, strong) GuardsProxy *breakpoints;
@property (readonly, strong) GuardsProxy *watchpoints;
@property (readonly, strong) GuardsProxy *copperBreakpoints;
@property (readonly, strong) HardDriveProxy *hd0;
@property (readonly, strong) HardDriveProxy *hd1;
@property (readonly, strong) HardDriveProxy *hd2;
@property (readonly, strong) HardDriveProxy *hd3;
@property (readonly, strong) KeyboardProxy *keyboard;
@property (readonly, strong) MemProxy *mem;
@property (readonly, strong) PaulaProxy *paula;
@property (readonly, strong) RemoteManagerProxy *remoteManager;
@property (readonly, strong) RetroShellProxy *retroShell;
@property (readonly, strong) RtcProxy *rtc;
@property (readonly, strong) RecorderProxy *recorder;
@property (readonly, strong) SerialPortProxy *serialPort;
@property (readonly, strong) VideoPortProxy *videoPort;

@property (class, readonly, strong) DefaultsProxy *defaults;

- (void)dealloc;
- (void)kill;

@property (readonly) AmigaInfo info;
@property (readonly) BOOL isWarping;
@property BOOL trackMode;
@property (readonly) NSInteger cpuLoad;
@property (readonly) BOOL fpuSupport;
@property InspectionTarget inspectionTarget;
- (void) removeInspectionTarget;

- (void)launch:(const void *)listener function:(Callback *)func;

- (NSInteger)get:(Option)opt;
- (NSInteger)get:(Option)opt id:(NSInteger)id;
- (NSInteger)get:(Option)opt drive:(NSInteger)id;
- (BOOL)set:(Option)opt value:(NSInteger)val;
- (BOOL)set:(Option)opt enable:(BOOL)val;
- (BOOL)set:(Option)opt id:(NSInteger)id value:(NSInteger)val;
- (BOOL)set:(Option)opt id:(NSInteger)id enable:(BOOL)val;
- (BOOL)set:(Option)opt drive:(NSInteger)id value:(NSInteger)val;
- (BOOL)set:(Option)opt drive:(NSInteger)id enable:(BOOL)val;

- (void)hardReset;
- (void)softReset;

@property (readonly) BOOL poweredOn;
@property (readonly) BOOL poweredOff;
@property (readonly) BOOL running;
@property (readonly) BOOL paused;

- (void)isReady:(ExceptionWrapper *)ex;
- (void)powerOn;
- (void)powerOff;
- (void)run:(ExceptionWrapper *)ex;
- (void)pause;
- (void)halt;

- (void)wakeUp;

- (void)suspend;
- (void)resume;

// - (void)continueScript;

 - (void) loadSnapshot:(SnapshotProxy *)proxy exception:(ExceptionWrapper *)ex;

- (NSInteger)getConfig:(Option)opt;
- (NSInteger)getConfig:(Option)opt id:(NSInteger)id;
- (NSInteger)getConfig:(Option)opt drive:(NSInteger)id;
- (BOOL)configure:(Option)opt value:(NSInteger)val;
- (BOOL)configure:(Option)opt enable:(BOOL)val;
- (BOOL)configure:(Option)opt id:(NSInteger)id value:(NSInteger)val;
- (BOOL)configure:(Option)opt id:(NSInteger)id enable:(BOOL)val;
- (BOOL)configure:(Option)opt drive:(NSInteger)id value:(NSInteger)val;
- (BOOL)configure:(Option)opt drive:(NSInteger)id enable:(BOOL)val;

- (void)setListener:(const void *)sender function:(Callback *)func;

- (void)setAlarmAbs:(NSInteger)cycle payload:(NSInteger)value;
- (void)setAlarmRel:(NSInteger)cycle payload:(NSInteger)value;

- (void)exportConfig:(NSURL *)url exception:(ExceptionWrapper *)ex;

- (void)put:(CmdType)cmd;
- (void)put:(CmdType)type value:(NSInteger)value;
- (void)put:(CmdType)type value:(NSInteger)value value2:(NSInteger)value2;
- (void)put:(CmdType)type key:(KeyCmd)cmd;
- (void)put:(CmdType)type action:(GamePadCmd)cmd;
- (void)put:(CmdType)type coord:(CoordCmd)cmd;

@end


//
// Properties
//

@interface DefaultsProxy : Proxy { }
    
- (void)load:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)save:(NSURL *)url exception:(ExceptionWrapper *)ex;

- (void)register:(NSString *)key value:(NSString *)value;

- (NSString *)getString:(NSString *)key;
- (NSInteger)getInt:(NSString *)key;
- (NSInteger)getOpt:(Option)option;
- (NSInteger)getOpt:(Option)option nr:(NSInteger)nr;

- (void)setKey:(NSString *)key value:(NSString *)value;
- (void)setOpt:(Option)option value:(NSInteger)value;
- (void)setOpt:(Option)option nr:(NSInteger)nr value:(NSInteger)value;

- (void)removeAll;
- (void)removeKey:(NSString *)key;
- (void)remove:(Option)option;
- (void)remove:(Option) option nr:(NSInteger)nr;

@end


//
// Guards (Breakpoints, Watchpoints, Catchpoints)
//

@interface GuardsProxy : Proxy { }
    
@property (readonly) NSInteger count;
- (NSInteger)addr:(NSInteger)nr;

- (BOOL)isSet:(NSInteger)nr;
- (BOOL)isSetAt:(NSInteger)addr;
- (void)setAt:(NSInteger)addr;
- (void)remove:(NSInteger)nr;
- (void)removeAt:(NSInteger)addr;
- (void)removeAll;

- (void)replace:(NSInteger)nr addr:(NSInteger)addr;

- (BOOL)isEnabled:(NSInteger)nr;
- (BOOL)isEnabledAt:(NSInteger)addr;
- (BOOL)isDisabled:(NSInteger)nr;
- (BOOL)isDisabledAt:(NSInteger)addr;

- (void)enable:(NSInteger)nr;
- (void)enableAt:(NSInteger)addr;
- (void)disable:(NSInteger)nr;
- (void)disableAt:(NSInteger)addr;

@end


//
// CPU
//

@interface CPUProxy : CoreComponentProxy { }
    
@property (readonly) CPUInfo info;
@property (readonly) CPUInfo cachedInfo;
@property (readonly) i64 clock;
@property (readonly) BOOL halted;

@property (readonly) NSInteger loggedInstructions;
- (void)clearLog;

- (NSString *)disassembleRecordedInstr:(NSInteger)i length:(NSInteger *)len;
- (NSString *)disassembleRecordedBytes:(NSInteger)i length:(NSInteger)len;
- (NSString *)disassembleRecordedFlags:(NSInteger)i;
- (NSString *)disassembleRecordedPC:(NSInteger)i;

- (NSString *)disassembleWord:(NSInteger)value;
- (NSString *)disassembleAddr:(NSInteger)addr;
- (NSString *)disassembleInstr:(NSInteger)addr length:(NSInteger *)len;
- (NSString *)disassembleWords:(NSInteger)addr length:(NSInteger)len;

- (NSString *)vectorName:(NSInteger)nr;

@end


//
// Amiga
//

@interface AmigaProxy : CoreComponentProxy { }

@property (readonly) AmigaInfo info;
// @property (readonly) AmigaInfo cachedInfo;

- (SnapshotProxy *) takeSnapshot;

@end


//
// CIA
//

@interface CIAProxy : CoreComponentProxy { }
 
@property (readonly) CIAInfo info;
@property (readonly) CIAInfo cachedInfo;

@end


//
// Memory
//

@interface MemProxy : CoreComponentProxy { }

@property (readonly) MemConfig config;
@property (readonly) MemInfo info;
@property (readonly) MemInfo cachedInfo;
@property (readonly) MemStats stats;

@property (readonly) RomTraits romTraits;
@property (readonly) RomTraits womTraits;
@property (readonly) RomTraits extTraits;

- (void)deleteRom;
- (BOOL)isRom:(NSURL *)url;
- (void)loadRom:(RomFileProxy *)proxy exception:(ExceptionWrapper *)ex;
- (void)loadRomFromBuffer:(NSData *)buffer exception:(ExceptionWrapper *)ex;
- (void)loadRomFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex;

- (void)deleteExt;
- (BOOL)isExt:(NSURL *)url;
- (void)loadExt:(ExtendedRomFileProxy *)proxy exception:(ExceptionWrapper *)ex;
- (void)loadExtFromBuffer:(NSData *)buffer exception:(ExceptionWrapper *)ex;
- (void)loadExtFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex;

- (void)saveRom:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)saveWom:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)saveExt:(NSURL *)url exception:(ExceptionWrapper *)ex;

- (MemorySource)memSrc:(Accessor)accessor addr:(NSInteger)addr;
- (NSInteger)spypeek16:(Accessor)accessor addr:(NSInteger)addr;

@end


//
// Agnus
//

@interface AgnusProxy : CoreComponentProxy { }

@property (readonly) AgnusInfo info;
@property (readonly) AgnusInfo cachedInfo;
@property (readonly) AgnusStats stats;
@property (readonly) AgnusTraits traits;

- (EventSlotInfo)cachedSlotInfo:(NSInteger)slot;
@property (readonly) NSInteger frameCount;

@end


//
// Copper
//

@interface CopperProxy : CoreComponentProxy { }

@property (readonly) CopperInfo info;
@property (readonly) CopperInfo cachedInfo;

- (BOOL)isIllegalInstr:(NSInteger)addr;
- (NSString *)disassemble:(NSInteger)addr symbolic:(BOOL)sym;
- (NSString *)disassemble:(NSInteger)list instr:(NSInteger)offset symbolic:(BOOL)sym;

@end


//
// Blitter
//

@interface BlitterProxy : CoreComponentProxy { }

@property (readonly) BlitterInfo info;
@property (readonly) BlitterInfo cachedInfo;

@end


//
// DMA Debugger
//

@interface DmaDebuggerProxy : Proxy { }

@property (readonly) DmaDebuggerInfo info;

@end


//
// Denise
//

@interface DeniseProxy : CoreComponentProxy { }

@property (readonly) DeniseInfo info;
@property (readonly) DeniseInfo cachedInfo;
- (SpriteInfo)getSpriteInfo:(NSInteger)nr;

- (NSInteger)sprDataLines:(NSInteger)nr;
- (u64)sprData:(NSInteger)nr line:(NSInteger)line;
- (u16)sprColor:(NSInteger)nr reg:(NSInteger)reg;

@end


//
// Recorder
//

@interface RecorderProxy : Proxy { }

@property NSString *path;
- (NSString *)findFFmpeg:(NSInteger)nr;
@property (readonly) BOOL hasFFmpeg;
@property (readonly) BOOL recording;
@property (readonly) double duration;
@property (readonly) NSInteger frameRate;
@property (readonly) NSInteger bitRate;
@property (readonly) NSInteger sampleRate;

- (void)startRecording:(NSRect)rect
               bitRate:(NSInteger)rate
               aspectX:(NSInteger)aspectX
               aspectY:(NSInteger)aspectY
             exception:(ExceptionWrapper *)ex;
- (void)stopRecording;
- (BOOL)exportAs:(NSString *)path;

@end


//
// Paula
//

@interface PaulaProxy : CoreComponentProxy { }

@property (readonly) PaulaInfo info;
@property (readonly) PaulaInfo cachedInfo;
@property (readonly) StateMachineInfo audioInfo0;
@property (readonly) StateMachineInfo audioInfo1;
@property (readonly) StateMachineInfo audioInfo2;
@property (readonly) StateMachineInfo audioInfo3;
@property (readonly) UARTInfo uartInfo;
@property (readonly) UARTInfo cachedUartInfo;
@property (readonly) AudioPortStats audioPortStats;

- (NSInteger)copyMono:(float *)target size:(NSInteger)n;
- (NSInteger)copyStereo:(float *)target1 buffer2:(float *)target2 size:(NSInteger)n;

- (float)drawWaveformL:(u32 *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(u32)c;
- (float)drawWaveformL:(u32 *)buffer size:(NSSize)size scale:(float)s color:(u32)c;
- (float)drawWaveformR:(u32 *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(u32)c;
- (float)drawWaveformR:(u32 *)buffer size:(NSSize)size scale:(float)s color:(u32)c;

@end


//
// RTC
//

@interface RtcProxy : CoreComponentProxy { }

- (void)update;

@end


//
// ControlPort
//

@interface ControlPortProxy : CoreComponentProxy {
        
    MouseProxy *mouse;
    JoystickProxy *joystick;
}

@property (readonly, strong) MouseProxy *mouse;
@property (readonly, strong) JoystickProxy *joystick;

@property (readonly) ControlPortInfo info;
@property (readonly) ControlPortInfo cachedInfo;

@end


//
// Video port
//

@interface VideoPortProxy : CoreComponentProxy { }

- (void)texture:(u32 **)ptr nr:(NSInteger *)nr lof:(bool *)lof prevlof:(bool *)prevlof;

@end


//
// SerialPort
//

@interface SerialPortProxy : CoreComponentProxy { }

@property (readonly) SerialPortInfo info;
@property (readonly) SerialPortInfo cachedInfo;

- (NSInteger)readIncomingPrintableByte;
- (NSInteger)readOutgoingPrintableByte;

@end


//
// Mouse
//

@interface MouseProxy : CoreComponentProxy { }

- (BOOL)detectShakeAbs:(NSPoint)pos;
- (BOOL)detectShakeRel:(NSPoint)pos;
- (void)setXY:(NSPoint)pos;
- (void)setDxDy:(NSPoint)pos;
- (void)trigger:(GamePadAction)event;

@end


//
// Joystick
//

@interface JoystickProxy : CoreComponentProxy { }

- (void)trigger:(GamePadAction)event;

@end


//
// Keyboard
//

@interface KeyboardProxy : CoreComponentProxy { }

- (BOOL)isPressed:(NSInteger)keycode;
- (void)press:(NSInteger)keycode;
- (void)release:(NSInteger)keycode;
- (void)toggle:(NSInteger)keycode;
- (void)releaseAll;

@end


//
// DiskController
//

@interface DiskControllerProxy : CoreComponentProxy { }

- (DiskControllerConfig)getConfig;
@property (readonly) DiskControllerInfo info;
@property (readonly) NSInteger selectedDrive;
@property (readonly) DriveState state;
@property (readonly) BOOL spinning;

@end


//
// FloppyDriveProxy
//

@interface FloppyDriveProxy : CoreComponentProxy { }

@property (readonly) FloppyDriveInfo info;
@property (readonly) FloppyDriveInfo cachedInfo;

- (void)setModificationFlag:(BOOL)value;
- (void)setProtectionFlag:(BOOL)value;
- (void)markDiskAsModified;
- (void)markDiskAsUnmodified;
- (void)toggleWriteProtection;

- (BOOL)isInsertable:(Diameter)type density:(Density)density;
- (void)eject;
- (void)swap:(FloppyFileProxy *)fileProxy exception:(ExceptionWrapper *)ex;
- (void)insertNew:(FSVolumeType)fs bootBlock:(BootBlockId)bb name:(NSString *)name exception:(ExceptionWrapper *)ex;
@property (readonly) BOOL motor;
@property (readonly) BOOL selected;
@property (readonly) BOOL writing;

- (NSString *)readTrackBits:(NSInteger)track;

@end


//
// HardDrive
//

@interface HardDriveProxy : CoreComponentProxy { }

@property (readonly) NSInteger nr;
@property (readonly) BOOL isConnected;
@property (readonly) NSInteger currentCyl;
@property (readonly) NSInteger currentHead;
@property (readonly) NSInteger currentOffset;

@property (readonly) BOOL hasDisk;
@property (readonly) BOOL hasModifiedDisk;
@property (readonly) BOOL hasProtectedDisk;
@property (readonly) BOOL hasUnmodifiedDisk;
@property (readonly) BOOL hasUnprotectedDisk;

- (void)setModificationFlag:(BOOL)value;
- (void)setProtectionFlag:(BOOL)value;
- (void)markDiskAsModified;
- (void)markDiskAsUnmodified;
- (void)toggleWriteProtection;

@property (readonly) HardDriveInfo info;
@property (readonly) NSInteger capacity;
@property (readonly) NSInteger partitions;
@property (readonly) NSInteger cylinders;
@property (readonly) NSInteger heads;
@property (readonly) NSInteger sectors;
@property (readonly) NSInteger bsize;
@property (readonly) HdcState hdcState;
@property (readonly) BOOL isCompatible;
@property (readonly) BOOL writeThroughEnabled;
- (NSString *)nameOfPartition:(NSInteger)nr;
- (NSInteger)lowerCylOfPartition:(NSInteger)nr;
- (NSInteger)upperCylOfPartition:(NSInteger)nr;
@property (readonly) HardDriveState state;
- (void)attachFile:(NSURL *)path exception:(ExceptionWrapper *)ex;
- (void)attach:(HDFFileProxy *)hdf exception:(ExceptionWrapper *)ex;
- (void)attach:(NSInteger)c h:(NSInteger)h s:(NSInteger)s b:(NSInteger)b exception:(ExceptionWrapper *)ex;
- (void)format:(FSVolumeType)fs name:(NSString *)name exception:(ExceptionWrapper *)ex;
- (void)changeGeometry:(NSInteger)c h:(NSInteger)h s:(NSInteger)s b:(NSInteger)b exception:(ExceptionWrapper *)ex;
- (NSArray *) geometries;
- (void)writeToFile:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)enableWriteThrough:(ExceptionWrapper *)ex;
- (void)disableWriteThrough;

@end


//
// FileSystem
//

@interface FileSystemProxy : Proxy { }

+ (instancetype)makeWithADF:(ADFFileProxy *)adf exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithHDF:(HDFFileProxy *)hdf partition:(NSInteger)nr exception:(ExceptionWrapper *)ex;

@property (readonly) NSString *name;
@property (readonly) NSString *creationDate;
@property (readonly) NSString *modificationDate;
@property (readonly) NSString *bootBlockName;
@property (readonly) NSString *capacityString;
@property (readonly) NSString *fillLevelString;
@property (readonly) FSVolumeType dos;
@property (readonly) BOOL isOFS;
@property (readonly) BOOL isFFS;
@property (readonly) NSInteger blockSize;
@property (readonly) NSInteger numBlocks;
@property (readonly) NSInteger numBytes;
@property (readonly) NSInteger usedBlocks;
@property (readonly) double fillLevel;
@property (readonly) BOOL hasVirus;
- (void)killVirus;
- (FSBlockType)blockType:(NSInteger)blockNr;
- (FSItemType)itemType:(NSInteger)blockNr pos:(NSInteger)pos;
- (FSErrorReport)check:(BOOL)strict;
- (ErrorCode)check:(NSInteger)nr pos:(NSInteger)pos expected:(unsigned char *)exp strict:(BOOL)strict;
- (BOOL)isCorrupted:(NSInteger)blockNr;
- (NSInteger)getCorrupted:(NSInteger)blockNr;
- (NSInteger)nextCorrupted:(NSInteger)blockNr;
- (NSInteger)prevCorrupted:(NSInteger)blockNr;
- (void)printDirectory:(BOOL) recursive;
- (NSInteger)readByte:(NSInteger)block offset:(NSInteger)offset;
- (NSString *)ascii:(NSInteger)block offset:(NSInteger)offset length:(NSInteger)len;
- (void)export:(NSString *)path exception:(ExceptionWrapper *)ex;
- (FSBlockType)getDisplayType:(NSInteger)column;
- (NSInteger)diagnoseImageSlice:(NSInteger)column;
- (NSInteger)nextBlockOfType:(FSBlockType)type after:(NSInteger)after;
- (NSInteger)nextCorruptedBlock:(NSInteger)after;

@end


//
// Debugger
//

@interface DebuggerProxy : CoreComponentProxy { }

- (void)stepInto;
- (void)stepOver;

- (NSString *)ascDump:(Accessor)accessor addr:(NSInteger)addr bytes:(NSInteger)bytes;
- (NSString *)hexDump:(Accessor)accessor addr:(NSInteger)addr bytes:(NSInteger)bytes;

@end


//
// RetroShell
//

@interface RetroShellProxy : Proxy { }

@property (readonly) NSInteger cursorRel;

- (NSString *)getText;
- (void)pressUp;
- (void)pressDown;
- (void)pressLeft;
- (void)pressRight;
- (void)pressHome;
- (void)pressEnd;
- (void)pressBackspace;
- (void)pressDelete;
- (void)pressCut;
- (void)pressReturn;
- (void)pressShiftReturn;
- (void)pressTab;
- (void)pressKey:(char)c;

@end

//
// RemoteManager
//

@interface RemoteManagerProxy : Proxy { }

@property (readonly) NSInteger numLaunching;
@property (readonly) NSInteger numListening;
@property (readonly) NSInteger numConnected;
@property (readonly) NSInteger numErroneous;

@end

//
// F I L E   T Y P E   P R O X I E S
//

@protocol MakeWithFile <NSObject>
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
@end

@protocol MakeWithBuffer <NSObject>
+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
@end

@protocol MakeWithDrive <NSObject>
+ (instancetype)makeWithDrive:(FloppyDriveProxy *)proxy exception:(ExceptionWrapper *)ex;
@end

@protocol MakeWithHardDrive <NSObject>
+ (instancetype)makeWithHardDrive:(HardDriveProxy *)proxy exception:(ExceptionWrapper *)ex;
@end

@protocol MakeWithFileSystem <NSObject>
+ (instancetype)makeWithFileSystem:(FileSystemProxy *)proxy exception:(ExceptionWrapper *)ex;
@end

//
// AmigaFile
//

@interface AmigaFileProxy : Proxy { }

+ (FileType) typeOfUrl:(NSURL *)url;

@property (readonly) FileType type;
@property (readonly) NSInteger size;
@property (readonly) NSString *sizeAsString;
@property (readonly) u64 fnv;

- (void)setPath:(NSString *)path;
- (NSInteger)writeToFile:(NSString *)path exception:(ExceptionWrapper *)ex;

@end

//
// Snapshot
//

@interface SnapshotProxy : AmigaFileProxy <MakeWithFile, MakeWithBuffer> {
    
    NSImage *preview;
}

+ (instancetype)make:(void *)file;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithAmiga:(AmigaProxy *)proxy;

@property (readonly, strong) NSImage *previewImage;
@property (readonly) time_t timeStamp;

@end


//
// Script
//

@interface ScriptProxy : AmigaFileProxy <MakeWithFile, MakeWithBuffer> { }

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
 
- (void)execute:(EmulatorProxy *)proxy;

@end


//
// RomFile
//

@interface RomFileProxy : AmigaFileProxy <MakeWithFile, MakeWithBuffer> { }

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
 
@end


//
// ExtendedRomFile
//

@interface ExtendedRomFileProxy : AmigaFileProxy <MakeWithFile, MakeWithBuffer> { }

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
 
@end


//
// DiskFileProxy
//

@interface DiskFileProxy : AmigaFileProxy {
}

@property (readonly) NSInteger numCyls;
@property (readonly) NSInteger numHeads;
@property (readonly) NSInteger numTracks;
@property (readonly) NSInteger bsize;
@property (readonly) NSInteger numSectors;
@property (readonly) NSInteger numBlocks;
@property (readonly) NSString *describeGeometry;
@property (readonly) NSString *describeCapacity;

- (NSInteger)readByte:(NSInteger)b offset:(NSInteger)offset;
- (void)readSector:(NSInteger)b destination:(unsigned char *)buf;

- (NSString *)hexdump:(NSInteger)b offset:(NSInteger)offset len:(NSInteger)len;
- (NSString *)asciidump:(NSInteger)b offset:(NSInteger)offset len:(NSInteger)len;

@end


//
// FloppyFileProxy
//

@interface FloppyFileProxy : DiskFileProxy {
}

@property (readonly) FSVolumeType dos;
@property (readonly) Diameter diskType;
@property (readonly) Density diskDensity;
@property (readonly) BOOL isSD;
@property (readonly) BOOL isDD;
@property (readonly) BOOL isHD;

@property (readonly) BootBlockType bootBlockType;
@property (readonly) NSString *bootBlockName;
@property (readonly) BOOL hasVirus;

- (void)killVirus;

@end


//
// ADFFileProxy
//

@interface ADFFileProxy : FloppyFileProxy <MakeWithFile, MakeWithBuffer, MakeWithDrive> {
}

+ (instancetype)makeWithDiameter:(Diameter)type density:(Density)density exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithDrive:(FloppyDriveProxy *)drive exception:(ExceptionWrapper *)ex;

- (void)format:(FSVolumeType)fs bootBlock:(NSInteger)bb name:(NSString *)name;

@end


//
// HDFFile
//

@interface HDFFileProxy : DiskFileProxy <MakeWithFile, MakeWithBuffer, MakeWithHardDrive> {
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithHardDrive:(HardDriveProxy *)drive exception:(ExceptionWrapper *)ex;

@property (readonly) BOOL hasRDB;
@property (readonly) NSInteger numPartitions;
@property (readonly) NSInteger numDrivers;

- (NSInteger)writeToFile:(NSString *)path partition:(NSInteger)nr exception:(ExceptionWrapper *)ex;

@end


//
// EADFFileProxy
//

@interface EADFFileProxy : FloppyFileProxy <MakeWithFile, MakeWithBuffer, MakeWithDrive> {
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithDrive:(FloppyDriveProxy *)drive exception:(ExceptionWrapper *)ex;

@end


//
// IMGFileProxy
//

@interface IMGFileProxy : FloppyFileProxy <MakeWithFile, MakeWithBuffer, MakeWithDrive> {
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithDrive:(FloppyDriveProxy *)proxy exception:(ExceptionWrapper *)ex;

@end


//
// STFileProxy
//

@interface STFileProxy : FloppyFileProxy <MakeWithFile, MakeWithBuffer> {
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;

@end


//
// DMSFileProxy
//

@interface DMSFileProxy : FloppyFileProxy <MakeWithFile, MakeWithBuffer> {
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;

@end


//
// EXEFileProxy
//

@interface EXEFileProxy : FloppyFileProxy <MakeWithFile, MakeWithBuffer> {
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;

@end


//
// Folder
//

@interface FolderProxy : FloppyFileProxy <MakeWithFile> {
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;

@property (readonly) ADFFileProxy *adf;

@end
