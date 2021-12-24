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
#import "AgnusTypes.h"
#import "AmigaTypes.h"
#import "AmigaFileTypes.h"
#import "BlitterTypes.h"
#import "BootBlockImageTypes.h"
#import "ControlPortTypes.h"
#import "CopperTypes.h"
#import "CPUTypes.h"
#import "CIATypes.h"
#import "DeniseTypes.h"
#import "DiskTypes.h"
#import "DiskControllerTypes.h"
#import "DmaDebuggerTypes.h"
#import "DriveTypes.h"
#import "ErrorTypes.h"
#import "FSTypes.h"
#import "AmigaComponentTypes.h"
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
#import "SchedulerTypes.h"
#import "SerialPortTypes.h"
#import "StateMachineTypes.h"
#import "UARTTypes.h"

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
@class DriveProxy;
@class EXEFileProxy;
@class ExtendedRomFileProxy;
@class FolderProxy;
@class GuardsProxy;
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
@class SchedulerProxy;
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
    DriveProxy *df0;
    DriveProxy *df1;
    DriveProxy *df2;
    DriveProxy *df3;
    GuardsProxy *breakpoints;
    GuardsProxy *watchpoints;
    KeyboardProxy *keyboard;
    MemProxy *mem;
    PaulaProxy *paula;
    RemoteManagerProxy *remoteManager;
    RetroShellProxy *retroShell;
    RtcProxy *rtc;
    RecorderProxy *recorder;
    SchedulerProxy *scheduler;
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
@property (readonly, strong) DriveProxy *df0;
@property (readonly, strong) DriveProxy *df1;
@property (readonly, strong) DriveProxy *df2;
@property (readonly, strong) DriveProxy *df3;
@property (readonly, strong) GuardsProxy *breakpoints;
@property (readonly, strong) GuardsProxy *watchpoints;
@property (readonly, strong) KeyboardProxy *keyboard;
@property (readonly, strong) MemProxy *mem;
@property (readonly, strong) PaulaProxy *paula;
@property (readonly, strong) RemoteManagerProxy *remoteManager;
@property (readonly, strong) RetroShellProxy *retroShell;
@property (readonly, strong) RtcProxy *rtc;
@property (readonly, strong) RecorderProxy *recorder;
@property (readonly, strong) SchedulerProxy *scheduler;
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
// Guards (Breakpoints, Watchpoints)
//

@interface GuardsProxy : Proxy { }
    
@property (readonly) NSInteger count;
- (NSInteger)addr:(NSInteger)nr;
- (BOOL)isEnabled:(NSInteger)nr;
- (BOOL)isDisabled:(NSInteger)nr;
- (void)enable:(NSInteger)nr;
- (void)disable:(NSInteger)nr;
- (void)remove:(NSInteger)nr;
- (void)replace:(NSInteger)nr addr:(NSInteger)addr;

- (BOOL)isSetAt:(NSInteger)addr;
- (BOOL)isSetAndEnabledAt:(NSInteger)addr;
- (BOOL)isSetAndDisabledAt:(NSInteger)addr;
- (void)enableAt:(NSInteger)addr;
- (void)disableAt:(NSInteger)addr;
- (void)addAt:(NSInteger)addr;
- (void)removeAt:(NSInteger)addr;

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
- (AgnusStats)getStats;

@end


//
// Scheduler
//

@interface SchedulerProxy : AmigaComponentProxy { }

@property (readonly) EventInfo info;
- (EventSlotInfo)getEventSlotInfo:(NSInteger)slot;

@end


//
// Copper
//

@interface CopperProxy : AmigaComponentProxy { }

@property (readonly) CopperInfo info;

- (BOOL)isIllegalInstr:(NSInteger)addr;
- (NSString *)disassemble:(NSInteger)addr;
- (NSString *)disassemble:(NSInteger)list instr:(NSInteger)offset;

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
@property (readonly) ScreenBuffer stableBuffer;
@property (readonly) u32 *noise;

@end


//
// Recorder
//

@interface RecorderProxy : Proxy { }

@property (readonly) BOOL hasFFmpeg;
@property (readonly) BOOL recording;
@property (readonly) double duration;
@property (readonly) NSInteger frameRate;
@property (readonly) NSInteger bitRate;
@property (readonly) NSInteger sampleRate;

- (BOOL)startRecording:(NSRect)rect
               bitRate:(NSInteger)rate
               aspectX:(NSInteger)aspectX
               aspectY:(NSInteger)aspectY;
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
// Drive
//

@interface DriveProxy : AmigaComponentProxy { }

@property (readonly) DriveInfo info;

@property (readonly) NSInteger nr;
@property (readonly) BOOL hasDisk;
@property (readonly) BOOL hasDDDisk;
@property (readonly) BOOL hasHDDisk;
- (BOOL)hasWriteProtectedDisk;
- (void)setWriteProtection:(BOOL)value;
- (void)toggleWriteProtection;
- (BOOL)isInsertable:(DiskDiameter)type density:(DiskDensity)density;
- (void)eject;
- (void)swap:(DiskFileProxy *)fileProxy exception:(ExceptionWrapper *)ex;
- (void)insertNew:(ExceptionWrapper *)ex;
@property (getter=isModifiedDisk) BOOL modifiedDisk;
@property (readonly) BOOL motor;
@property (readonly) NSInteger cylinder;
@property (readonly) u64 fnv;

@end


//
// FSDevice
//

@interface FSDeviceProxy : Proxy { }

+ (instancetype)makeWithADF:(ADFFileProxy *)adf exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithHDF:(HDFFileProxy *)hdf exception:(ExceptionWrapper *)ex;

@property (readonly) FSVolumeType dos;
@property (readonly) NSInteger numCyls;
@property (readonly) NSInteger numHeads;
@property (readonly) NSInteger numTracks;
@property (readonly) NSInteger numSectors;
@property (readonly) NSInteger numBlocks;

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
- (void)export:(NSString *)path exception:(ExceptionWrapper *)ex;

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

/*
- (void)start:(ServerType)type;
- (void)start:(ServerType)type port:(NSInteger)port;
- (void)stop:(ServerType)type;
- (void)disconnect:(ServerType)type;
*/

@end

//
// F I L E   T Y P E   P R O X I E S
//

@protocol MakeWithFile
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
@end

@protocol MakeWithBuffer
+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
@end

@protocol MakeWithDrive
+ (instancetype)makeWithDrive:(DriveProxy *)proxy exception:(ExceptionWrapper *)ex;
@end

@protocol MakeWithFileSystem
+ (instancetype)makeWithFileSystem:(FSDeviceProxy *)proxy exception:(ExceptionWrapper *)ex;
@end

//
// AmigaFile
//

@interface AmigaFileProxy : Proxy { }

+ (FileType) typeOfUrl:(NSURL *)url;

@property (readonly) FileType type;
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

@property (readonly) FSVolumeType dos;
@property (readonly) DiskDiameter diskType;
@property (readonly) DiskDensity diskDensity;
@property (readonly) BOOL isSD;
@property (readonly) BOOL isDD;
@property (readonly) BOOL isHD;
@property (readonly) NSInteger numCyls;
@property (readonly) NSInteger numSides;
@property (readonly) NSInteger numTracks;
@property (readonly) NSInteger numSectors;
@property (readonly) NSInteger numBlocks;

@property (readonly) BootBlockType bootBlockType;
@property (readonly) NSString *bootBlockName;
@property (readonly) BOOL hasVirus;

- (void)killVirus;

- (NSInteger)readByte:(NSInteger)block offset:(NSInteger)offset;
- (void)readSector:(unsigned char *)dst block:(NSInteger)block;
- (void)readSectorHex:(char *)dst block:(NSInteger)block count:(NSInteger)count;

@end


//
// ADFFileProxy
//

@interface ADFFileProxy : DiskFileProxy <MakeWithFile, MakeWithBuffer, MakeWithDrive> {
}

+ (instancetype)makeWithDiameter:(DiskDiameter)type density:(DiskDensity)density exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithDrive:(DriveProxy *)drive exception:(ExceptionWrapper *)ex;

- (void)formatDisk:(FSVolumeType)fs bootBlock:(NSInteger)bootBlockID;

@end


//
// HDFFile
//

@interface HDFFileProxy : AmigaFileProxy <MakeWithFile, MakeWithBuffer> {
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;

@property (readonly) NSInteger numBlocks;

@end


//
// EXTFileProxy
//

@interface EXTFileProxy : DiskFileProxy <MakeWithFile, MakeWithBuffer, MakeWithDrive> {
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithDrive:(DriveProxy *)drive exception:(ExceptionWrapper *)ex;

@end


//
// IMGFileProxy
//

@interface IMGFileProxy : DiskFileProxy <MakeWithFile, MakeWithBuffer, MakeWithDrive> {
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithDrive:(DriveProxy *)proxy exception:(ExceptionWrapper *)ex;

@end


//
// DMSFileProxy
//

@interface DMSFileProxy : DiskFileProxy <MakeWithFile, MakeWithBuffer> {
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;

@property (readonly) ADFFileProxy *adf;

@end


//
// EXEFileProxy
//

@interface EXEFileProxy : DiskFileProxy <MakeWithFile, MakeWithBuffer> {
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;

@property (readonly) ADFFileProxy *adf;

@end


//
// Folder
//

@interface FolderProxy : DiskFileProxy <MakeWithFile> {
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;

@property (readonly) ADFFileProxy *adf;

@end
