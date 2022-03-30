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
#import "AmigaComponentTypes.h"
#import "AmigaTypes.h"
#import "AmigaFileTypes.h"
#import "BlitterTypes.h"
#import "BootBlockImageTypes.h"
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
#import "JoystickTypes.h"
#import "KeyboardTypes.h"
#import "MemoryTypes.h"
#import "MsgQueueTypes.h"
#import "MuxerTypes.h"
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
@class AmigaFileProxy;
@class BlitterProxy;
@class CIAProxy;
@class ControlPortProxy;
@class CopperProxy;
@class CPUProxy;
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
@class RemoteManagerProxy;
@class RetroShellProxy;
@class RomFileProxy;
@class RtcProxy;
@class RecorderProxy;
@class ScriptProxy;
@class SerialPortProxy;
@class SnapshotProxy;

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

@interface AmigaComponentProxy : Proxy { }

@end

//
// Amiga
//

@interface AmigaProxy : AmigaComponentProxy {
        
    AgnusProxy *agnus;
    CIAProxy *ciaA;
    CIAProxy *ciaB;
    ControlPortProxy *controlPort1;
    ControlPortProxy *controlPort2;
    CopperProxy *copper;
    CPUProxy *cpu;
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
    RemoteManagerProxy *remoteManager;
    RetroShellProxy *retroShell;
    RtcProxy *rtc;
    RecorderProxy *recorder;
    SerialPortProxy *serialPort;
}

@property (readonly, strong) AgnusProxy *agnus;
@property (readonly, strong) BlitterProxy *blitter;
@property (readonly, strong) CIAProxy *ciaA;
@property (readonly, strong) CIAProxy *ciaB;
@property (readonly, strong) ControlPortProxy *controlPort1;
@property (readonly, strong) ControlPortProxy *controlPort2;
@property (readonly, strong) CopperProxy *copper;
@property (readonly, strong) CPUProxy *cpu;
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

- (void)dealloc;
- (void)kill;

@property (readonly) AmigaInfo info;

@property BOOL warpMode;
@property BOOL debugMode;
@property (readonly) NSInteger cpuLoad;

@property InspectionTarget inspectionTarget;
- (void) removeInspectionTarget;

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

- (void)stopAndGo;
- (void)stepInto;
- (void)stepOver;

- (void)suspend;
- (void)resume;
- (void)continueScript;

- (void)requestAutoSnapshot;
- (void)requestUserSnapshot;
@property (readonly) SnapshotProxy *latestAutoSnapshot;
@property (readonly) SnapshotProxy *latestUserSnapshot;
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

@interface CPUProxy : AmigaComponentProxy { }
    
@property (readonly) CPUInfo info;
@property (readonly) i64 clock;
@property (readonly) BOOL halted;

@property (readonly) NSInteger loggedInstructions;
- (void)clearLog;

- (NSString *)disassembleRecordedInstr:(NSInteger)i length:(NSInteger *)len;
- (NSString *)disassembleRecordedBytes:(NSInteger)i length:(NSInteger)len;
- (NSString *)disassembleRecordedFlags:(NSInteger)i;
- (NSString *)disassembleRecordedPC:(NSInteger)i;

- (NSString *)disassembleInstr:(NSInteger)addr length:(NSInteger *)len;
- (NSString *)disassembleWords:(NSInteger)addr length:(NSInteger)len;
- (NSString *)disassembleAddr:(NSInteger)addr;

- (NSString *)vectorName:(NSInteger)nr;

@end


//
// CIA
//

@interface CIAProxy : AmigaComponentProxy { }
 
@property (readonly) CIAInfo info;

@end


//
// Memory
//

@interface MemProxy : AmigaComponentProxy { }

@property (readonly) MemoryConfig config;
- (MemoryStats) getStats;

- (BOOL) isBootRom:(RomIdentifier)rev;
- (BOOL) isArosRom:(RomIdentifier)rev;
- (BOOL) isDiagRom:(RomIdentifier)rev;
- (BOOL) isCommodoreRom:(RomIdentifier)rev;
- (BOOL) isHyperionRom:(RomIdentifier)rev;
- (BOOL) isPatchedRom:(RomIdentifier)rev;

@property (readonly) BOOL hasRom;
@property (readonly) BOOL hasBootRom;
@property (readonly) BOOL hasKickRom;
- (void)deleteRom;
- (BOOL)isRom:(NSURL *)url;
- (void)loadRom:(RomFileProxy *)proxy exception:(ExceptionWrapper *)ex;
- (void)loadRomFromBuffer:(NSData *)buffer exception:(ExceptionWrapper *)ex;
- (void)loadRomFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex;
@property (readonly) u64 romFingerprint;
@property (readonly) RomIdentifier romIdentifier;
@property (readonly, copy) NSString *romTitle;
@property (readonly, copy) NSString *romVersion;
@property (readonly, copy) NSString *romReleased;
@property (readonly, copy) NSString *romModel;

- (BOOL)hasExt;
- (void)deleteExt;
- (BOOL)isExt:(NSURL *)url;
- (void)loadExt:(ExtendedRomFileProxy *)proxy exception:(ExceptionWrapper *)ex;
- (void)loadExtFromBuffer:(NSData *)buffer exception:(ExceptionWrapper *)ex;
- (void)loadExtFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex;
@property (readonly) u64 extFingerprint;
@property (readonly) RomIdentifier extIdentifier;
@property (readonly, copy) NSString *extTitle;
@property (readonly, copy) NSString *extVersion;
@property (readonly, copy) NSString *extReleased;
@property (readonly, copy) NSString *extModel;
@property (readonly) NSInteger extStart;

- (void)saveRom:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)saveWom:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)saveExt:(NSURL *)url exception:(ExceptionWrapper *)ex;

- (MemorySource)memSrc:(Accessor)accessor addr:(NSInteger)addr;
- (NSInteger)spypeek16:(Accessor)accessor addr:(NSInteger)addr;

- (NSString *)ascii:(Accessor)accessor addr:(NSInteger)addr;
- (NSString *)hex:(Accessor)accessor addr:(NSInteger)addr bytes:(NSInteger)bytes;

@end


//
// Agnus
//

@interface AgnusProxy : AmigaComponentProxy { }

@property (readonly) NSInteger chipRamLimit;
@property (readonly) AgnusInfo info;
@property (readonly) EventInfo eventInfo;
- (EventSlotInfo)getEventSlotInfo:(NSInteger)slot;
@property (readonly) BOOL isOCS;
@property (readonly) BOOL isECS;
- (AgnusStats)getStats;

@end


//
// Copper
//

@interface CopperProxy : AmigaComponentProxy { }

@property (readonly) CopperInfo info;

- (BOOL)isIllegalInstr:(NSInteger)addr;
- (NSString *)disassemble:(NSInteger)addr symbolic:(BOOL)sym;
- (NSString *)disassemble:(NSInteger)list instr:(NSInteger)offset symbolic:(BOOL)sym;

@end


//
// Blitter
//

@interface BlitterProxy : AmigaComponentProxy { }

@property (readonly) BlitterInfo info;

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

@interface DeniseProxy : AmigaComponentProxy { }

@property (readonly) DeniseInfo info;
- (SpriteInfo)getSpriteInfo:(NSInteger)nr;

- (NSInteger)sprDataLines:(NSInteger)nr;
- (u64)sprData:(NSInteger)nr line:(NSInteger)line;
- (u16)sprColor:(NSInteger)nr reg:(NSInteger)reg;

- (void)lockStableBuffer;
- (void)unlockStableBuffer;
@property (readonly) BOOL longFrame;
@property (readonly) u32 *stableBuffer;
@property (readonly) u32 *noise;

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

@interface PaulaProxy : AmigaComponentProxy { }

@property (readonly) PaulaInfo info;
@property (readonly) StateMachineInfo audioInfo0;
@property (readonly) StateMachineInfo audioInfo1;
@property (readonly) StateMachineInfo audioInfo2;
@property (readonly) StateMachineInfo audioInfo3;
@property (readonly) UARTInfo uartInfo;
@property (readonly) MuxerStats muxerStats;

- (double)sampleRate;
- (void)setSampleRate:(double)rate;

- (void)readMonoSamples:(float *)target size:(NSInteger)n;
- (void)readStereoSamples:(float *)target1 buffer2:(float *)target2 size:(NSInteger)n;

- (void)rampUp;
- (void)rampUpFromZero;
- (void)rampDown;

- (float)drawWaveformL:(u32 *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(u32)c;
- (float)drawWaveformL:(u32 *)buffer size:(NSSize)size scale:(float)s color:(u32)c;
- (float)drawWaveformR:(u32 *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(u32)c;
- (float)drawWaveformR:(u32 *)buffer size:(NSSize)size scale:(float)s color:(u32)c;

@end


//
// RTC
//

@interface RtcProxy : AmigaComponentProxy { }

- (void)update;

@end


//
// ControlPort
//

@interface ControlPortProxy : AmigaComponentProxy {
        
    MouseProxy *mouse;
    JoystickProxy *joystick;
}

@property (readonly, strong) MouseProxy *mouse;
@property (readonly, strong) JoystickProxy *joystick;

@property (readonly) ControlPortInfo info;

@end


//
// SerialPort
//

@interface SerialPortProxy : AmigaComponentProxy { }

@property (readonly) SerialPortInfo info;

@end


//
// Mouse
//

@interface MouseProxy : AmigaComponentProxy { }

- (BOOL)detectShakeAbs:(NSPoint)pos;
- (BOOL)detectShakeRel:(NSPoint)pos;
- (void)setXY:(NSPoint)pos;
- (void)setDxDy:(NSPoint)pos;
- (void)trigger:(GamePadAction)event;

@end


//
// Joystick
//

@interface JoystickProxy : AmigaComponentProxy { }

- (void)trigger:(GamePadAction)event;

@end


//
// Keyboard
//

@interface KeyboardProxy : AmigaComponentProxy { }

- (BOOL)keyIsPressed:(NSInteger)keycode;
- (void)pressKey:(NSInteger)keycode;
- (void)releaseKey:(NSInteger)keycode;
- (void)toggleKey:(NSInteger)keycode;
- (void)releaseAllKeys;

@end


//
// DiskController
//

@interface DiskControllerProxy : AmigaComponentProxy { }

- (DiskControllerConfig)getConfig;
@property (readonly) DiskControllerInfo info;
@property (readonly) NSInteger selectedDrive;
@property (readonly) DriveState state;
@property (readonly, getter=isSpinning) BOOL spinning;

@end


//
// DriveProxy
//

@interface DriveProxy : AmigaComponentProxy { }

@property (readonly) NSInteger nr;
@property (readonly) BOOL isConnected;
@property (readonly) NSInteger currentCyl;
@property (readonly) NSInteger currentHead;
@property (readonly) NSInteger currentOffset;
@property (readonly) u64 fnv;

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

@end


//
// FloppyDriveProxy
//

@interface FloppyDriveProxy : DriveProxy { }

@property (readonly) FloppyDriveInfo info;

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

@interface HardDriveProxy : DriveProxy { }

@property (readonly) HardDriveInfo info;
@property (readonly) NSInteger capacity;
@property (readonly) NSInteger partitions;
@property (readonly) NSInteger cylinders;
@property (readonly) NSInteger heads;
@property (readonly) NSInteger sectors;
@property (readonly) NSInteger bsize;
@property (readonly) BOOL uniqueGeometry;
- (NSString *)nameOfPartition:(NSInteger)nr;
- (NSInteger)lowerCylOfPartition:(NSInteger)nr;
- (NSInteger)upperCylOfPartition:(NSInteger)nr;
@property (readonly) HardDriveState state;
- (void)attach:(HDFFileProxy *)hdf exception:(ExceptionWrapper *)ex;
- (void)attach:(NSInteger)c h:(NSInteger)h s:(NSInteger)s b:(NSInteger)b exception:(ExceptionWrapper *)ex;
- (void)format:(FSVolumeType)fs name:(NSString *)name exception:(ExceptionWrapper *)ex;
- (void)changeGeometry:(NSInteger)c h:(NSInteger)h s:(NSInteger)s b:(NSInteger)b exception:(ExceptionWrapper *)ex;
- (NSArray *) geometries;

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
- (void)pressReturn;
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
 
- (void)execute:(AmigaProxy *)proxy;

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

- (NSInteger)writeToFile:(NSString *)path partition:(NSInteger)nr exception:(ExceptionWrapper *)ex;

@end


//
// EXTFileProxy
//

@interface EXTFileProxy : FloppyFileProxy <MakeWithFile, MakeWithBuffer, MakeWithDrive> {
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
