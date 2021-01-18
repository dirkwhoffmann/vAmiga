// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#include "AmigaConstants.h"
#include "AmigaPublicTypes.h"

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
@class DmaDebuggerProxy;
@class FolderProxy;
@class DMSFileProxy;
@class DriveProxy;
@class EXEFileProxy;
@class GuardsProxy;
@class HDFFileProxy;
@class IMGFileProxy;
@class JoystickProxy;
@class KeyboardProxy;
@class MemProxy;
@class MouseProxy;
@class PaulaProxy;
@class ScreenRecorderProxy;
@class SerialPortProxy;
@class SnapshotProxy;

//
// Base proxies
//

@interface Proxy : NSObject {
    
    // Reference to the wrapped C++ object
    @public void *obj;
}

@end

@interface HardwareComponentProxy : Proxy { }

- (void)dump;

@end

//
// Amiga
//

@interface AmigaProxy : HardwareComponentProxy {
    
    struct AmigaWrapper *wrapper;
    
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
    ScreenRecorderProxy *screenRecorder;
    SerialPortProxy *serialPort;
}

@property (readonly) struct AmigaWrapper *wrapper;
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
@property (readonly, strong) ScreenRecorderProxy *screenRecorder;
@property (readonly, strong) SerialPortProxy *serialPort;

- (void) kill;

@property (readonly) BOOL isReleaseBuild;
@property BOOL debugMode;
- (void) enableDebugging __attribute__ ((deprecated));
- (void) disableDebugging __attribute__ ((deprecated));
- (void) setInspectionTarget:(EventID)id;
- (void) clearInspectionTarget;

- (BOOL) isReady:(ErrorCode *)error;
- (BOOL) isReady;
- (void) powerOn;
- (void) powerOff;
- (void) hardReset;
- (void) softReset;

- (AmigaInfo) getInfo;

@property (readonly, getter=isPoweredOn) BOOL poweredOn;
@property (readonly, getter=isPoweredOff) BOOL poweredOff;
@property (readonly, getter=isRunning) BOOL running;
@property (readonly, getter=isPaused) BOOL paused;

- (void) run;
- (void) pause;
- (void) suspend;
- (void) resume;

- (void) requestAutoSnapshot;
- (void) requestUserSnapshot;
@property (readonly) SnapshotProxy *latestAutoSnapshot;
@property (readonly) SnapshotProxy *latestUserSnapshot;
- (void) loadFromSnapshot:(SnapshotProxy *)proxy;

- (NSInteger) getConfig:(Option)opt;
- (NSInteger) getConfig:(Option)opt id:(NSInteger)id;
- (NSInteger) getConfig:(Option)opt drive:(NSInteger)id;
- (BOOL) configure:(Option)opt value:(NSInteger)val;
- (BOOL) configure:(Option)opt enable:(BOOL)val;
- (BOOL) configure:(Option)opt id:(NSInteger)id value:(NSInteger)val;
- (BOOL) configure:(Option)opt id:(NSInteger)id enable:(BOOL)val;
- (BOOL) configure:(Option)opt drive:(NSInteger)id value:(NSInteger)val;
- (BOOL) configure:(Option)opt drive:(NSInteger)id enable:(BOOL)val;

// Message queue
- (void) addListener:(const void *)sender function:(Callback *)func;
- (void) removeListener:(const void *)sender;
- (Message)message;

- (void) stopAndGo;
- (void) stepInto;
- (void) stepOver;

@property (readonly) BOOL warp;
- (void) warpOn;
- (void) warpOff;

@end


//
// Guards (Breakpoints, Watchpoints)
//

@interface GuardsProxy : Proxy { }
    
@property (readonly) NSInteger count;
- (NSInteger) addr:(NSInteger)nr;
- (BOOL) isEnabled:(NSInteger)nr;
- (BOOL) isDisabled:(NSInteger)nr;
- (void) enable:(NSInteger)nr;
- (void) disable:(NSInteger)nr;
- (void) remove:(NSInteger)nr;
- (void) replace:(NSInteger)nr addr:(NSInteger)addr;

- (BOOL) isSetAt:(NSInteger)addr;
- (BOOL) isSetAndEnabledAt:(NSInteger)addr;
- (BOOL) isSetAndDisabledAt:(NSInteger)addr;
- (void) enableAt:(NSInteger)addr;
- (void) disableAt:(NSInteger)addr;
- (void) addAt:(NSInteger)addr;
- (void) removeAt:(NSInteger)addr;

@end


//
// CPU
//

@interface CPUProxy : HardwareComponentProxy { }
    
- (CPUInfo) getInfo;

@property (readonly) i64 clock;
@property (readonly) i64 cycles;
@property (readonly, getter=isHalted) bool halted;

@property (readonly) NSInteger loggedInstructions;
- (void) clearLog;

- (NSString *) disassembleRecordedInstr:(NSInteger)i length:(NSInteger *)len;
- (NSString *) disassembleRecordedBytes:(NSInteger)i length:(NSInteger)len;
- (NSString *) disassembleRecordedFlags:(NSInteger)i;
- (NSString *) disassembleRecordedPC:(NSInteger)i;

- (NSString *) disassembleInstr:(NSInteger)addr length:(NSInteger *)len;
- (NSString *) disassembleWords:(NSInteger)addr length:(NSInteger)len;
- (NSString *) disassembleAddr:(NSInteger)addr;

@end


//
// CIA proxy
//

@interface CIAProxy : HardwareComponentProxy { }
    
- (void) dumpConfig;
- (CIAInfo) getInfo;

@end


//
// Memory proxy
//

@interface MemProxy : HardwareComponentProxy { }

@property (readonly) MemoryConfig config;
- (MemoryStats) getStats;

- (BOOL) isBootRom:(RomIdentifier)rev;
- (BOOL) isArosRom:(RomIdentifier)rev;
- (BOOL) isDiagRom:(RomIdentifier)rev;
- (BOOL) isCommodoreRom:(RomIdentifier)rev;
- (BOOL) isHyperionRom:(RomIdentifier)rev;

@property (readonly) BOOL hasRom;
@property (readonly) BOOL hasBootRom;
@property (readonly) BOOL hasKickRom;
- (void) deleteRom;
- (BOOL) isRom:(NSURL *)url;
- (BOOL) loadRomFromBuffer:(NSData *)buffer;
- (BOOL) loadRomFromFile:(NSURL *)url error:(ErrorCode *)err;
- (u64) romFingerprint;
- (RomIdentifier) romIdentifier;
@property (readonly, copy) NSString *romTitle;
@property (readonly, copy) NSString *romVersion;
@property (readonly, copy) NSString *romReleased;

- (BOOL) hasExt;
- (void) deleteExt;
- (BOOL) isExt:(NSURL *)url;
- (BOOL) loadExtFromBuffer:(NSData *)buffer;
- (BOOL) loadExtFromFile:(NSURL *)url;
- (u64) extFingerprint;
@property (readonly) RomIdentifier extIdentifier;
- (NSString *) extTitle;
- (NSString *) extVersion;
- (NSString *) extReleased;
- (NSInteger) extStart;

- (BOOL) saveWom:(NSURL *)url;
- (BOOL) saveRom:(NSURL *)url;
- (BOOL) saveExt:(NSURL *)url;

- (void) updateRTC;
- (MemorySource) memSrc:(Accessor)accessor addr:(NSInteger)addr;
- (NSInteger) spypeek16:(Accessor)accessor addr:(NSInteger)addr;

- (NSString *) ascii:(Accessor)accessor addr:(NSInteger)addr;
- (NSString *) hex:(Accessor)accessor addr:(NSInteger)addr bytes:(NSInteger)bytes;

@end


//
// Agnus
//

@interface AgnusProxy : HardwareComponentProxy { }

- (NSInteger) chipRamLimit;

- (AgnusInfo) getInfo;
- (EventSlotInfo) getEventSlotInfo:(NSInteger)slot;
- (EventInfo) getEventInfo;
- (AgnusStats) getStats;

@end


//
// Copper
//

@interface CopperProxy : HardwareComponentProxy { }

- (CopperInfo) getInfo;

- (BOOL) isIllegalInstr:(NSInteger)addr;
- (NSInteger) instrCount:(NSInteger)list;
- (void) adjustInstrCount:(NSInteger)list offset:(NSInteger)offset;
- (NSString *) disassemble:(NSInteger)addr;
- (NSString *) disassemble:(NSInteger)list instr:(NSInteger)offset;

@end


//
// Blitter
//

@interface BlitterProxy : HardwareComponentProxy { }

- (BlitterInfo) getInfo;

@end


//
// DMA Debugger
//

@interface DmaDebuggerProxy : Proxy { }

- (DMADebuggerInfo) getInfo;

- (void) setEnable:(BOOL)value;
- (void) visualizeCopper:(BOOL)value;
- (void) visualizeBlitter:(BOOL)value;
- (void) visualizeDisk:(BOOL)value;
- (void) visualizeAudio:(BOOL)value;
- (void) visualizeSprite:(BOOL)value;
- (void) visualizeBitplane:(BOOL)value;
- (void) visualizeCpu:(BOOL)value;
- (void) visualizeRefresh:(BOOL)value;

- (void) setOpacity:(double)value;
- (void) setDisplayMode:(NSInteger)mode;
- (void) setCopperColor:(double)r g:(double)g b:(double)b;
- (void) setBlitterColor:(double)r g:(double)g b:(double)b;
- (void) setDiskColor:(double)r g:(double)g b:(double)b;
- (void) setAudioColor:(double)r g:(double)g b:(double)b;
- (void) setSpriteColor:(double)r g:(double)g b:(double)b;
- (void) setBitplaneColor:(double)r g:(double)g b:(double)b;
- (void) setCpuColor:(double)r g:(double)g b:(double)b;
- (void) setRefreshColor:(double)r g:(double)g b:(double)b;

@end

//
// Denise
//

@interface DeniseProxy : HardwareComponentProxy { }

- (DeniseInfo) getInfo;
- (SpriteInfo) getSpriteInfo:(NSInteger)nr;

- (NSInteger) sprDataLines:(NSInteger)nr;
- (u64) sprData:(NSInteger)nr line:(NSInteger)line;
- (u16) sprColor:(NSInteger)nr reg:(NSInteger)reg;

@property Palette palette;
@property double brightness;
@property double saturation;
@property double contrast;

- (ScreenBuffer) stableBuffer;
- (u32 *) noise;

@end


//
// ScreenRecorder
//

@interface ScreenRecorderProxy : Proxy { }

@property (readonly) BOOL hasFFmpeg;
@property (readonly) BOOL recording;
@property (readonly) NSInteger recordCounter;

- (BOOL) startRecording:(NSRect)rect
                bitRate:(NSInteger)rate
                aspectX:(NSInteger)aspectX
                aspectY:(NSInteger)aspectY;
- (void) stopRecording;
- (BOOL) exportAs:(NSString *)path;

@end


//
// Paula proxy
//

@interface PaulaProxy : HardwareComponentProxy { }

- (PaulaInfo) getInfo;
- (AudioInfo) getAudioInfo;
- (MuxerStats) getMuxerStats;
- (UARTInfo) getUARTInfo;

- (u32) sampleRate;
- (void) setSampleRate:(double)rate;

- (void) readMonoSamples:(float *)target size:(NSInteger)n;
- (void) readStereoSamples:(float *)target1 buffer2:(float *)target2 size:(NSInteger)n;

- (void) rampUp;
- (void) rampUpFromZero;
- (void) rampDown;

- (float) drawWaveformL:(unsigned *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(unsigned)c;
- (float) drawWaveformL:(unsigned *)buffer size:(NSSize)size scale:(float)s color:(unsigned)c;
- (float) drawWaveformR:(unsigned *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(unsigned)c;
- (float) drawWaveformR:(unsigned *)buffer size:(NSSize)size scale:(float)s color:(unsigned)c;

@end


//
// ControlPort proxy
//

@interface ControlPortProxy : HardwareComponentProxy {
        
    MouseProxy *mouse;
    JoystickProxy *joystick;
}

@property (readonly, strong) MouseProxy *mouse;
@property (readonly, strong) JoystickProxy *joystick;

- (ControlPortInfo) getInfo;

@end


//
// SerialPort
//

@interface SerialPortProxy : HardwareComponentProxy { }

- (SerialPortInfo) getInfo;

@end


//
// Mouse proxy
//

@interface MouseProxy : HardwareComponentProxy { }

- (void) setXY:(NSPoint)pos;
- (void) setDeltaXY:(NSPoint)pos;
- (void) trigger:(GamePadAction)event;

@end


//
// Joystick proxy
//

@interface JoystickProxy : HardwareComponentProxy { }

- (void) trigger:(GamePadAction)event;
@property BOOL autofire;
@property NSInteger autofireBullets;
@property float autofireFrequency;

@end


//
// Keyboard proxy
//

@interface KeyboardProxy : HardwareComponentProxy { }

- (BOOL) keyIsPressed:(NSInteger)keycode;
- (void) pressKey:(NSInteger)keycode;
- (void) releaseKey:(NSInteger)keycode;
- (void) releaseAllKeys;

@end


//
// DiskController
//

@interface DiskControllerProxy : HardwareComponentProxy { }

- (DiskControllerConfig) getConfig;
- (DiskControllerInfo) getInfo;
@property (readonly) NSInteger selectedDrive;
@property (readonly) DriveState state;
@property (readonly, getter=isSpinning) BOOL spinning;
- (void) eject:(NSInteger)nr;
- (void) insert:(NSInteger)nr file:(DiskFileProxy *)fileProxy;
 - (void) setWriteProtection:(NSInteger)nr value:(BOOL)value;

@end


//
// Drive
//

@interface DriveProxy : HardwareComponentProxy { }

- (DriveInfo) getInfo;

@property (readonly) NSInteger nr;
@property (readonly) BOOL hasDisk;
@property (readonly) BOOL hasDDDisk;
@property (readonly) BOOL hasHDDisk;
- (BOOL) hasWriteProtectedDisk;
- (void) setWriteProtection:(BOOL)value;
- (void) toggleWriteProtection;
- (BOOL) isInsertable:(DiskDiameter)type density:(DiskDensity)density;
@property (getter=isModifiedDisk) BOOL modifiedDisk;
@property (readonly) BOOL motor;
@property (readonly) NSInteger cylinder;
@property (readonly) u64 fnv;

@end


//
// FSDevice
//

@interface FSDeviceProxy : Proxy { }

+ (instancetype)makeWithADF:(ADFFileProxy *)adf;
+ (instancetype)makeWithHDF:(HDFFileProxy *)hdf;

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
- (ErrorCode)export:(NSString *)path;

@end


//
// F I L E   T Y P E S
//

//
// AmigaFile
//

@interface AmigaFileProxy : Proxy { }

@property (readonly) FileType type;
- (void)setPath:(NSString *)path;
- (NSInteger)writeToFile:(NSString *)path error:(ErrorCode *)err;
@property (readonly) u64 fnv;

/*
- (void)readFromBuffer:(const void *)buffer length:(NSInteger)length __attribute__ ((deprecated));
- (NSInteger)writeToBuffer:(void *)buffer;
*/

@end

//
// Snapshot
//

@interface SnapshotProxy : AmigaFileProxy {
    
    NSImage *preview;
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len;
+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len error:(ErrorCode *)err;
+ (instancetype)makeWithFile:(NSString *)path;
+ (instancetype)makeWithFile:(NSString *)path error:(ErrorCode *)err;
+ (instancetype)makeWithAmiga:(AmigaProxy *)amiga;

@property (readonly, strong) NSImage *previewImage;
@property (readonly) time_t timeStamp;
// @property (readonly, copy) NSData *data;

@end


//
// DiskFileProxy
//

@interface DiskFileProxy : AmigaFileProxy {
}

@property (readonly) FSVolumeType dos;
@property (readonly) DiskDiameter diskType;
@property (readonly) DiskDensity diskDensity;
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

@interface ADFFileProxy : DiskFileProxy {
}

+ (BOOL)isADFFile:(NSString *)path;
+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)length;
+ (instancetype)makeWithFile:(NSString *)path error:(ErrorCode *)err;
+ (instancetype)makeWithDiameter:(DiskDiameter)type density:(DiskDensity)density;
+ (instancetype)makeWithDrive:(DriveProxy *)drive;

- (void)formatDisk:(FSVolumeType)fs bootBlock:(NSInteger)bootBlockID;

@end


//
// HDFFile proxy
//

@interface HDFFileProxy : AmigaFileProxy {
}

+ (BOOL)isHDFFile:(NSString *)path;
+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)len;
+ (instancetype)makeWithFile:(NSString *)path error:(ErrorCode *)err;

@property (readonly) NSInteger numBlocks;

@end


//
// EXTFileProxy
//

@interface EXTFileProxy : DiskFileProxy {
}

+ (BOOL)isEXTFile:(NSString *)path;
+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)len;
+ (instancetype)makeWithFile:(NSString *)path error:(ErrorCode *)err;

@end


//
// IMGFileProxy
//

@interface IMGFileProxy : DiskFileProxy {
}

+ (BOOL)isIMGFile:(NSString *)path;
+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)len;
+ (instancetype)makeWithFile:(NSString *)path error:(ErrorCode *)err;
+ (instancetype)makeWithDrive:(DriveProxy *)drive;

@end


//
// DMSFileProxy
//

@interface DMSFileProxy : DiskFileProxy {
}

+ (BOOL)isDMSFile:(NSString *)path;
+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)len;
+ (instancetype)makeWithFile:(NSString *)path error:(ErrorCode *)err;

- (ADFFileProxy *)adf;

@end


//
// EXEFileProxy
//

@interface EXEFileProxy : DiskFileProxy {
}

+ (BOOL)isEXEFile:(NSString *)path;
+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)len;
+ (instancetype)makeWithFile:(NSString *)path error:(ErrorCode *)err;

- (ADFFileProxy *)adf;

@end


//
// Folder proxy
//

@interface FolderProxy : DiskFileProxy {
}

+ (BOOL)isFolder:(NSString *)path;
+ (instancetype)makeWithFile:(NSString *)path error:(ErrorCode *)err;

- (ADFFileProxy *)adf;

@end
