// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#import "config.h"
#import "VAmigaTypes.h"
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

using namespace vamiga;

//
// Forward declarations
//

@class AudioPortProxy;
@class AgnusProxy;
@class AmigaProxy;
@class AnyFileProxy;
@class BlitterProxy;
@class CIAProxy;
@class ControlPortProxy;
@class CopperProxy;
@class CPUProxy;
@class DeniseProxy;
@class DiskControllerProxy;
@class DiskFileProxy;
@class DmaDebuggerProxy;
@class FloppyDriveProxy;
@class GuardsProxy;
@class HardDriveProxy;
@class HdControllerProxy;
@class JoystickProxy;
@class KeyboardProxy;
@class LogicAnalyzerProxy;
@class MediaFileProxy;
@class MemProxy;
@class MouseProxy;
@class PaulaProxy;
@class DefaultsProxy;
@class RemoteManagerProxy;
@class RetroShellProxy;
@class RtcProxy;
@class SerialPortProxy;
@class VideoPortProxy;

//
// Constants
//

typedef struct {
    
    struct {
        
        struct {
            NSInteger CLK;
            NSInteger DMA;
            NSInteger CPU;
        } FREQUENCY;
        
        struct {
            NSInteger CNT_LF;
            NSInteger CNT_SF;
            NSInteger CNT;
            NSInteger MAX_LF;
            NSInteger MAX_SF;
            NSInteger MAX;
        } VPOS;
        
        struct {
            NSInteger CNT_LL;
            NSInteger CNT_SL;
            NSInteger CNT;
            NSInteger MAX_LL;
            NSInteger MAX_SL;
            NSInteger MAX;
        } HPOS;
        
        struct {
            NSInteger MIN;
            NSInteger MAX;
            NSInteger CNT;
        } VBLANK;
    } PAL, NTSC;
    
    struct {
        NSInteger CNT;
        NSInteger MAX;
    } VPOS;
    
    struct {
        NSInteger CNT;
        NSInteger MAX;
    } HPOS;
    
    struct {
        NSInteger MIN;
        NSInteger MAX;
        NSInteger CNT;
    } HBLANK;
    
    NSInteger VPIXELS;
    NSInteger HPIXELS;
    NSInteger PIXELS;
    
    struct {
        NSInteger C_MIN;
        NSInteger C_MAX;
        NSInteger H_MIN;
        NSInteger H_MAX;
        NSInteger S_MIN;
        NSInteger S_MAX;
    } HDR;
    
} VAmigaConstants;

extern const VAmigaConstants VAMIGA;

NSString *EventSlotName(EventSlot slot);

//
// Exception wrapper
//

@interface ExceptionWrapper : NSObject {
    
    Fault fault;
    NSString *what;
}

@property Fault fault;
@property NSString *what;

@end


//
// Base proxies
//

@interface Proxy : NSObject {
    
    // Reference to the wrapped C++ object
    @public void *obj;    
}

- (instancetype) initWith:(void *)ref;

@end

@interface CoreComponentProxy : Proxy { }

@property (readonly) NSInteger objid;

@end


//
// Emulator
//

@interface EmulatorProxy : CoreComponentProxy {
        
    AudioPortProxy *audioPort;
    AgnusProxy *agnus;
    AmigaProxy *amiga;
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
    LogicAnalyzerProxy *logicAnalyzer;
    MemProxy *mem;
    PaulaProxy *paula;
    DefaultsProxy *properties;
    RemoteManagerProxy *remoteManager;
    RetroShellProxy *retroShell;
    RtcProxy *rtc;
    SerialPortProxy *serialPort;
    VideoPortProxy *videoPort;
}

@property (readonly, strong) AudioPortProxy *audioPort;
@property (readonly, strong) AgnusProxy *agnus;
@property (readonly, strong) AmigaProxy *amiga;
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
@property (readonly, strong) LogicAnalyzerProxy *logicAnalyzer;
@property (readonly, strong) MemProxy *mem;
@property (readonly, strong) PaulaProxy *paula;
@property (readonly, strong) RemoteManagerProxy *remoteManager;
@property (readonly, strong) RetroShellProxy *retroShell;
@property (readonly, strong) RtcProxy *rtc;
@property (readonly, strong) SerialPortProxy *serialPort;
@property (readonly, strong) VideoPortProxy *videoPort;

@property (class, readonly, strong) DefaultsProxy *defaults;

- (void)dealloc;
- (void)kill;

@property (class, readonly) NSString *build;
@property (class, readonly) NSString *version;

@property (readonly) EmulatorInfo info;
@property (readonly) EmulatorInfo cachedInfo;
@property (readonly) EmulatorStats stats;

@property (readonly) BOOL poweredOn;
@property (readonly) BOOL poweredOff;
@property (readonly) BOOL paused;
@property (readonly) BOOL running;
@property (readonly) BOOL suspended;
@property (readonly) BOOL halted;
@property (readonly) BOOL warping;
@property (readonly) BOOL tracking;

- (void)launch:(ExceptionWrapper *)ex;
- (void)launch:(const void *)listener function:(Callback *)func exception:(ExceptionWrapper *)ex;
- (void)wakeUp;

- (void)isReady:(ExceptionWrapper *)ex;
- (void)powerOn;
- (void)powerOff;
- (void)run:(ExceptionWrapper *)ex;
- (void)pause;
- (void)halt;
- (void)suspend;
- (void)resume;
- (void)warpOn;
- (void)warpOn:(NSInteger)source;
- (void)warpOff;
- (void)warpOff:(NSInteger)source;
- (void)trackOn;
- (void)trackOn:(NSInteger)source;
- (void)trackOff;
- (void)trackOff:(NSInteger)source;

- (void)hardReset;
- (void)softReset;

- (void)stepInto;
- (void)stepOver;
- (void)finishLine;
- (void)finishFrame;

- (NSInteger)get:(Opt)opt;
- (NSInteger)get:(Opt)opt id:(NSInteger)id;
- (NSInteger)get:(Opt)opt drive:(NSInteger)id;
- (BOOL)set:(Opt)opt value:(NSInteger)val;
- (BOOL)set:(Opt)opt enable:(BOOL)val;
- (BOOL)set:(Opt)opt id:(NSInteger)id value:(NSInteger)val;
- (BOOL)set:(Opt)opt id:(NSInteger)id enable:(BOOL)val;
- (BOOL)set:(Opt)opt drive:(NSInteger)id value:(NSInteger)val;
- (BOOL)set:(Opt)opt drive:(NSInteger)id enable:(BOOL)val;
- (void)exportConfig:(NSURL *)url exception:(ExceptionWrapper *)ex;

- (void)put:(Cmd)cmd;
- (void)put:(Cmd)type value:(NSInteger)value;
- (void)put:(Cmd)type value:(NSInteger)value value2:(NSInteger)value2;
- (void)put:(Cmd)type key:(KeyCmd)cmd;
- (void)put:(Cmd)type action:(GamePadCmd)cmd;
- (void)put:(Cmd)type coord:(CoordCmd)cmd;

@end


//
// Defaults
//

@interface DefaultsProxy : Proxy { }
    
- (void)load:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)save:(NSURL *)url exception:(ExceptionWrapper *)ex;

- (void)register:(NSString *)key value:(NSString *)value;

- (NSString *)getString:(NSString *)key;
- (NSInteger)getInt:(NSString *)key;
- (NSInteger)getOpt:(Opt)option;
- (NSInteger)getOpt:(Opt)option nr:(NSInteger)nr;

- (void)setKey:(NSString *)key value:(NSString *)value;
- (void)setOpt:(Opt)option value:(NSInteger)value;
- (void)setOpt:(Opt)option nr:(NSInteger)nr value:(NSInteger)value;

- (void)removeAll;
- (void)removeKey:(NSString *)key;
- (void)remove:(Opt)option;
- (void)remove:(Opt) option nr:(NSInteger)nr;

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
@property (readonly) AmigaInfo cachedInfo;
@property NSInteger autoInspectionMask;
@property (readonly) NSString *stateString;

- (MediaFileProxy *) takeSnapshot:(Compressor)compressor;
- (void)loadSnapshot:(MediaFileProxy *)proxy exception:(ExceptionWrapper *)ex;
- (void)loadSnapshotFromUrl:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)saveSnapshotToUrl:(NSURL *)url exception:(ExceptionWrapper *)ex;

- (void)loadWorkspace:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)saveWorkspace:(NSURL *)url exception:(ExceptionWrapper *)ex;

- (BOOL) getMessage:(Message *)msg;

@end


//
// CIA
//

@interface CIAProxy : CoreComponentProxy { }
 
@property (readonly) CIAInfo info;
@property (readonly) CIAInfo cachedInfo;
@property (readonly) CIAStats stats;

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
- (void)loadRom:(MediaFileProxy *)proxy exception:(ExceptionWrapper *)ex;
- (void)loadRomFromBuffer:(NSData *)buffer exception:(ExceptionWrapper *)ex;
- (void)loadRomFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex;

- (void)deleteExt;
// - (BOOL)isExt:(NSURL *)url;
- (void)loadExt:(MediaFileProxy *)proxy exception:(ExceptionWrapper *)ex;
- (void)loadExtFromBuffer:(NSData *)buffer exception:(ExceptionWrapper *)ex;
- (void)loadExtFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex;

- (void)saveRom:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)saveWom:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)saveExt:(NSURL *)url exception:(ExceptionWrapper *)ex;

- (MemSrc)memSrc:(Accessor)accessor addr:(NSInteger)addr;
- (NSInteger)spypeek16:(Accessor)accessor addr:(NSInteger)addr;

- (NSString *)regName:(NSInteger)addr;
- (NSString *)ascDump:(Accessor)accessor addr:(NSInteger)addr bytes:(NSInteger)bytes;
- (NSString *)hexDump:(Accessor)accessor addr:(NSInteger)addr bytes:(NSInteger)bytes;

@end


//
// Audio port
//

@interface AudioPortProxy : CoreComponentProxy { }

@property (readonly) AudioPortStats stats;

- (NSInteger)copyMono:(float *)target size:(NSInteger)n;
- (NSInteger)copyStereo:(float *)target1 buffer2:(float *)target2 size:(NSInteger)n;
- (NSInteger)copyInterleaved:(float *)target size:(NSInteger)n;

- (void)drawWaveformL:(u32 *)buffer w:(NSInteger)w h:(NSInteger)h color:(u32)c;
- (void)drawWaveformL:(u32 *)buffer size:(NSSize)size color:(u32)c;
- (void)drawWaveformR:(u32 *)buffer w:(NSInteger)w h:(NSInteger)h color:(u32)c;
- (void)drawWaveformR:(u32 *)buffer size:(NSSize)size color:(u32)c;

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
// LogicAnalyzer
//

@interface LogicAnalyzerProxy : Proxy { }

@property (readonly) LogicAnalyzerInfo info;

@end


//
// Denise
//

@interface DeniseProxy : CoreComponentProxy { }

@property (readonly) DeniseInfo info;
@property (readonly) DeniseInfo cachedInfo;
- (SpriteInfo)getSpriteInfo:(NSInteger)nr;
- (SpriteInfo)getCachedSpriteInfo:(NSInteger)nr;

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
// @property (readonly) AudioPortStats audioPortStats;

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

- (void)lockTexture;
- (void)unlockTexture;
- (void)texture:(const u32 **)ptr nr:(NSInteger *)nr lof:(bool *)lof prevlof:(bool *)prevlof;
- (void)innerArea:(NSInteger *)x1 x2:(NSInteger *)x2 y1:(NSInteger *)y1 y2:(NSInteger *)y2;
- (void)innerAreaNormalized:(double *)x1 x2:(double *)x2 y1:(double *)y1 y2:(double *)y2;

@end


//
// SerialPort
//

@interface SerialPortProxy : CoreComponentProxy { }

@property (readonly) SerialPortConfig config;
@property (readonly) SerialPortInfo info;
@property (readonly) SerialPortInfo cachedInfo;

- (NSInteger)readIncomingPrintableByte;
- (NSInteger)readOutgoingPrintableByte;

@end


//
// Mouse
//

@interface MouseProxy : CoreComponentProxy { }

- (void)setXY:(NSPoint)pos;
- (void)setDxDy:(NSPoint)pos;
- (void)trigger:(GamePadAction)event;
- (BOOL)detectShakeAbs:(NSPoint)pos;
- (BOOL)detectShakeRel:(NSPoint)pos;

@end


//
// Joystick
//

@interface JoystickProxy : CoreComponentProxy { }

@property (readonly) JoystickInfo info;
@property (readonly) JoystickInfo cachedInfo;

- (void)trigger:(GamePadAction)event;

@end


//
// Keyboard
//

@interface KeyboardProxy : CoreComponentProxy { }

- (BOOL)isPressed:(NSInteger)keycode;
- (void)press:(NSInteger)keycode;
- (void)press:(NSInteger)keycode delay:(double)delay;
- (void)press:(NSInteger)keycode delay:(double)delay duration:(double)duration;
- (void)release:(NSInteger)keycode;
- (void)release:(NSInteger)keycode delay:(double)delay;
- (void)toggle:(NSInteger)keycode;
- (void)releaseAll;

@end


//
// DiskController
//

@interface DiskControllerProxy : CoreComponentProxy { }

@property (readonly) DiskControllerConfig config;
@property (readonly) DiskControllerInfo info;
@property (readonly) DiskControllerInfo cachedInfo;

@end


//
// FloppyDriveProxy
//

@interface FloppyDriveProxy : CoreComponentProxy { }

@property (readonly) FloppyDriveConfig config;
@property (readonly) FloppyDriveInfo info;
@property (readonly) FloppyDriveInfo cachedInfo;

- (BOOL)getFlag:(DiskFlags)mask;
- (void)setFlag:(DiskFlags)mask value:(BOOL)value;

- (BOOL)isInsertable:(Diameter)type density:(Density)density;
- (void)insertBlankDisk:(FSFormat)fs bootBlock:(BootBlockId)bb name:(NSString *)name url:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)insertMedia:(MediaFileProxy *)proxy protected:(BOOL)wp exception:(ExceptionWrapper *)ex;
- (void)insertFile:(NSURL *)url protected:(BOOL)wp exception:(ExceptionWrapper *)ex;
- (void)eject;
- (MediaFileProxy *)exportDisk:(FileType)type exception:(ExceptionWrapper *)ex;

- (NSString *)readTrackBits:(NSInteger)track;

@end


//
// HdController
//

@interface HdControllerProxy : CoreComponentProxy { }

@property (readonly) HdcInfo info;
@property (readonly) HdcStats stats;

@end


//
// HardDrive
//

@interface HardDriveProxy : CoreComponentProxy { 

    HdControllerProxy *controller;
}

@property (readonly, strong) HdControllerProxy *controller;

@property (readonly) HardDriveConfig config;
@property (readonly) HardDriveInfo info;
@property (readonly) HardDriveTraits traits;
- (PartitionTraits) partitionTraits:(NSInteger)nr;

- (BOOL)getFlag:(DiskFlags)mask;
- (void)setFlag:(DiskFlags)mask value:(BOOL)value;

- (void)attach:(MediaFileProxy *)proxy exception:(ExceptionWrapper *)ex;
- (void)attach:(NSInteger)c h:(NSInteger)h s:(NSInteger)s b:(NSInteger)b exception:(ExceptionWrapper *)ex;
- (void)attachFile:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)importFiles:(NSURL *)url exception:(ExceptionWrapper *)ex;
- (void)format:(FSFormat)fs name:(NSString *)name exception:(ExceptionWrapper *)ex;
- (void)changeGeometry:(NSInteger)c h:(NSInteger)h s:(NSInteger)s b:(NSInteger)b exception:(ExceptionWrapper *)ex;
- (NSArray *) geometries;
- (void)writeToFile:(NSURL *)url exception:(ExceptionWrapper *)ex;

@end

//
// FileSystem
//

@interface FileSystemProxy : Proxy { }

+ (instancetype)makeWithMedia:(MediaFileProxy *)proxy partition:(NSInteger)nr exception:(ExceptionWrapper *)ex;

@property (readonly) NSString *name;
@property (readonly) NSString *creationDate;
@property (readonly) NSString *modificationDate;
@property (readonly) NSString *bootBlockName;
@property (readonly) NSString *capacityString;
@property (readonly) NSString *fillLevelString;
@property (readonly) FSFormat dos;
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

- (NSInteger)xrayBlocks:(BOOL)strict;
- (NSInteger)xrayBitmap:(BOOL)strict;
@property (readonly) NSArray<NSNumber *> *xrayBlocks;
@property (readonly) NSArray<NSNumber *> *xrayBitmap;

- (FSBlockError)check:(NSInteger)nr pos:(NSInteger)pos expected:(unsigned char *)exp strict:(BOOL)strict;
- (NSInteger)readByte:(NSInteger)block offset:(NSInteger)offset;
- (NSString *)ascii:(NSInteger)block offset:(NSInteger)offset length:(NSInteger)len;
- (void)export:(NSString *)path recursive:(BOOL)rec contents:(BOOL)con exception:(ExceptionWrapper *)ex;

- (void)createUsageMap:(u8 *)buf length:(NSInteger)len;
- (void)createAllocationMap:(u8 *)buf length:(NSInteger)len;
- (void)createHealthMap:(u8 *)buf length:(NSInteger)len;
- (NSInteger)nextBlockOfType:(FSBlockType)type after:(NSInteger)after;

- (void)rectifyAllocationMap;

@end


//
// RemoteManager
//

@interface RemoteManagerProxy : Proxy { }

@property (readonly) RemoteManagerInfo info;

@end


//
// RetroShell
//

@interface RetroShellProxy : Proxy { }

@property (readonly) RetroShellInfo info;

- (NSString *)getText;
- (void)pressKey:(char)c;
- (void)pressSpecialKey:(RSKey)key;
- (void)pressSpecialKey:(RSKey)key shift:(BOOL)shift;
- (void)executeScript:(MediaFileProxy *)file;
- (void)executeString:(NSString *)string;

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
// MediaFile
//

@interface MediaFileProxy : Proxy
{
    NSImage *preview;
}

+ (FileType) typeOfUrl:(NSURL *)url;

+ (instancetype)make:(void *)file;
+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFile:(NSString *)path type:(FileType)t exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len type:(FileType)t exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithAmiga:(EmulatorProxy *)proxy compressor:(Compressor)c;
+ (instancetype)makeWithDrive:(FloppyDriveProxy *)proxy type:(FileType)t exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithHardDrive:(HardDriveProxy *)proxy type:(FileType)t exception:(ExceptionWrapper *)ex;
+ (instancetype)makeWithFileSystem:(FileSystemProxy *)proxy type:(FileType)t exception:(ExceptionWrapper *)ex;

@property (readonly) FileType type;
@property (readonly) NSInteger size;
@property (readonly) u64 fnv;
@property (readonly) Compressor compressor;
@property (readonly) BOOL compressed;

@property (readonly) u8 *data;

- (void)writeToFile:(NSString *)path exception:(ExceptionWrapper *)ex;
- (void)writeToFile:(NSString *)path partition:(NSInteger)part exception:(ExceptionWrapper *)ex;

@property (readonly, strong) NSImage *previewImage;
@property (readonly) time_t timeStamp;
@property (readonly) DiskInfo diskInfo;
@property (readonly) FloppyDiskInfo floppyDiskInfo;
@property (readonly) HDFInfo hdfInfo;
@property (readonly) NSString *describeCapacity;

- (NSInteger)readByte:(NSInteger)b offset:(NSInteger)offset;
- (void)readSector:(NSInteger)b destination:(unsigned char *)buf;

- (NSString *)hexdump:(NSInteger)b offset:(NSInteger)offset len:(NSInteger)len;
- (NSString *)asciidump:(NSInteger)b offset:(NSInteger)offset len:(NSInteger)len;

@end


//
// AnyFile
//

@interface AnyFileProxy : Proxy { }

+ (FileType) typeOfUrl:(NSURL *)url;

@property (readonly) FileType type;
@property (readonly) NSURL *path;
@property (readonly) NSInteger size;
@property (readonly) NSString *getSizeAsString;
@property (readonly) u64 fnv;

- (void)setPath:(NSString *)path;
- (NSInteger)writeToFile:(NSString *)path exception:(ExceptionWrapper *)ex;

@end


//
// DiskFileProxy
//

@interface DiskFileProxy : AnyFileProxy {
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
