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
#include "AmigaTypes.h"

// Forward declarations
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
@class DmaDebuggerProxy;
@class DMSFileProxy;
@class DriveProxy;
@class GuardsProxy;
@class IMGFileProxy;
@class JoystickProxy;
@class KeyboardProxy;
@class MemProxy;
@class MouseProxy;
@class PaulaProxy;
@class SerialPortProxy;
@class SnapshotProxy;

/* Forward declarations of C++ class wrappers.
 * We wrap classes into normal C structs to avoid any reference to C++.
 */
struct AgnusWrapper;
struct AmigaFileWrapper;
struct AmigaWrapper;
struct BlitterWrapper;
struct CIAWrapper;
struct ControlPortWrapper;
struct CopperWrapper;
struct CPUWrapper;
struct DeniseWrapper;
struct DiskControllerWrapper;
struct DmaDebuggerWrapper;
struct DriveWrapper;
struct GuardsWrapper;
struct KeyboardWrapper;
struct MemWrapper;
struct PaulaWrapper;
struct SerialPortWrapper;

//
// Amiga
//

@interface AmigaProxy : NSObject {
    
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
    JoystickProxy *joystick1;
    JoystickProxy *joystick2;
    KeyboardProxy *keyboard;
    MemProxy *mem;
    MouseProxy *mouse1;
    MouseProxy *mouse2;
    PaulaProxy *paula;
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
@property (readonly, strong) JoystickProxy *joystick1;
@property (readonly, strong) JoystickProxy *joystick2;
@property (readonly, strong) KeyboardProxy *keyboard;
@property (readonly, strong) MemProxy *mem;
@property (readonly, strong) MouseProxy *mouse1;
@property (readonly, strong) MouseProxy *mouse2;
@property (readonly, strong) PaulaProxy *paula;
@property (readonly, strong) SerialPortProxy *serialPort;

- (void) kill;

@property (readonly, getter=isReleaseBuild) BOOL releaseBuild;
- (void) enableDebugging;
- (void) disableDebugging;
- (void) setInspectionTarget:(EventID)id;
- (void) clearInspectionTarget;
@property (readonly) BOOL debugMode;

- (BOOL) isReady:(ErrorCode *)error;
- (BOOL) isReady;
- (void) powerOn;
- (void) powerOff;
- (void) hardReset;
- (void) softReset;
- (void) dump;

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

@property (readonly) AmigaConfiguration config;
- (NSInteger) getConfig:(ConfigOption)opt;
- (NSInteger) getConfig:(ConfigOption)opt drive:(NSInteger)nr;
- (BOOL) configure:(ConfigOption)opt value:(NSInteger)val;
- (BOOL) configure:(ConfigOption)opt enable:(BOOL)val;
- (BOOL) configure:(ConfigOption)opt drive:(NSInteger)nr value:(NSInteger)val;
- (BOOL) configure:(ConfigOption)opt drive:(NSInteger)nr enable:(BOOL)val;

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

@interface GuardsProxy : NSObject {
    
    struct GuardsWrapper *wrapper;
}

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

@interface CPUProxy : NSObject {
    
    struct CPUWrapper *wrapper;
}

- (void) dump;
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
// CIA
//

@interface CIAProxy : NSObject {
    
    struct CIAWrapper *wrapper;
}

- (void) dumpConfig;
- (void) dump;
- (CIAInfo) getInfo;

@end


//
// Memory
//

@interface MemProxy : NSObject {
    
    struct MemWrapper *wrapper;
}

- (MemoryConfig) getConfig;
- (MemoryStats) getStats;
- (void) dump;

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
- (BOOL) isEncryptedRom:(NSURL *)url;
- (BOOL) loadRomFromBuffer:(NSData *)buffer;
- (BOOL) loadRomFromFile:(NSURL *)url;
// - (BOOL) loadEncryptedRomFromFile:(NSURL *)url;
- (BOOL) loadEncryptedRomFromFile:(NSURL *)url error:(DecryptionError *)error;
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

- (MemorySource *) getMemSrcTable; 
- (MemorySource) memSrc:(NSInteger)addr;
- (NSInteger) spypeek16:(NSInteger)addr;

- (NSString *) ascii:(NSInteger)addr;
- (NSString *) hex:(NSInteger)addr bytes:(NSInteger)bytes;

@end


//
// Agnus
//

@interface AgnusProxy : NSObject {
    
    struct AgnusWrapper *wrapper;
}

- (NSInteger) chipRamLimit;

- (void) dump;

- (AgnusInfo) getInfo;
- (EventSlotInfo) getEventSlotInfo:(NSInteger)slot;
- (EventInfo) getEventInfo;
- (AgnusStats) getStats;

@end


//
// Copper
//

@interface CopperProxy : NSObject {
    
    struct CopperWrapper *wrapper;
}

- (void) dump;
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

@interface BlitterProxy : NSObject {
    
    struct BlitterWrapper *wrapper;
}

- (void) dump;
- (BlitterInfo) getInfo;

@end


//
// DMA Debugger
//

@interface DmaDebuggerProxy : NSObject {
    
    struct DmaDebuggerWrapper *wrapper;
}

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

@interface DeniseProxy : NSObject {
    
    struct DeniseWrapper *wrapper;
}

- (void) dump;
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
// Paula
//

@interface PaulaProxy : NSObject {
    
    struct PaulaWrapper *wrapper;
}

- (void) dump;
- (PaulaInfo) getInfo;
- (AudioInfo) getAudioInfo;
- (AudioStats) getAudioStats;
- (UARTInfo) getUARTInfo;

- (u32) sampleRate;
- (void) setSampleRate:(double)rate;

@property (readonly) NSInteger ringbufferSize;
- (double) ringbufferDataL:(NSInteger)offset;
- (double) ringbufferDataR:(NSInteger)offset;
- (double) fillLevel;

- (void) readMonoSamples:(float *)target size:(NSInteger)n;
- (void) readStereoSamples:(float *)target1 buffer2:(float *)target2 size:(NSInteger)n;
- (void) readStereoSamplesInterleaved:(float *)target size:(NSInteger)n;

- (void) rampUp;
- (void) rampUpFromZero;
- (void) rampDown;

- (float) drawWaveformL:(unsigned *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(unsigned)c;
- (float) drawWaveformL:(unsigned *)buffer size:(NSSize)size scale:(float)s color:(unsigned)c;
- (float) drawWaveformR:(unsigned *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(unsigned)c;
- (float) drawWaveformR:(unsigned *)buffer size:(NSSize)size scale:(float)s color:(unsigned)c;

@end


//
// ControlPort
//

@interface ControlPortProxy : NSObject {
    
    struct ControlPortWrapper *wrapper;
}

- (void) dump;
- (ControlPortInfo) getInfo;
- (void) connectDevice:(ControlPortDevice)value;

@end


//
// SerialPort
//

@interface SerialPortProxy : NSObject {

    struct SerialPortWrapper *wrapper;
}

- (void) dump;
- (SerialPortInfo) getInfo;

@end


//
// Mouse
//

@interface MouseProxy : NSObject {
    
    struct MouseWrapper *wrapper;
}

- (void) dump;

- (void) setXY:(NSPoint)pos;
- (void) setDeltaXY:(NSPoint)pos;
- (void) trigger:(GamePadAction)event;

@end


//
// Joystick
//

@interface JoystickProxy : NSObject {
    
    struct JoystickWrapper *wrapper;
}

- (void) dump;

- (void) trigger:(GamePadAction)event;
@property BOOL autofire;
@property NSInteger autofireBullets;
@property float autofireFrequency;

@end


//
// Keyboard
//

@interface KeyboardProxy : NSObject {
    
    struct KeyboardWrapper *wrapper;
}

- (void) dump;

- (BOOL) keyIsPressed:(NSInteger)keycode;
- (void) pressKey:(NSInteger)keycode;
- (void) releaseKey:(NSInteger)keycode;
- (void) releaseAllKeys;

@end


//
// DiskController
//

@interface DiskControllerProxy : NSObject {
    
    struct DiskControllerWrapper *wrapper;
}

- (void) dump;
- (DiskControllerConfig) getConfig;
- (DiskControllerInfo) getInfo;
@property (readonly) NSInteger selectedDrive;
@property (readonly) DriveState state;
@property (readonly, getter=isSpinning) BOOL spinning;
/*
- (BOOL) connected:(NSInteger)nr;
- (BOOL) disconnected:(NSInteger)nr;
- (void) setConnected:(NSInteger)nr value:(BOOL)value;
*/
 - (void) eject:(NSInteger)nr;
- (void) insert:(NSInteger)nr adf:(ADFFileProxy *)fileProxy;
- (void) insert:(NSInteger)nr dms:(DMSFileProxy *)fileProxy;
- (void) setWriteProtection:(NSInteger)nr value:(BOOL)value;

@end


//
// AmigaDrive
//

@interface DriveProxy : NSObject {
    
    struct DriveWrapper *wrapper;
}

@property (readonly) struct DriveWrapper *wrapper;

- (void) dump;

- (DriveInfo) getInfo;
@property (readonly) NSInteger nr;
@property (readonly) BOOL hasDisk;
- (BOOL) hasWriteProtectedDisk;
- (void) setWriteProtection:(BOOL)value;
- (void) toggleWriteProtection;

@property (getter=isModifiedDisk) BOOL modifiedDisk;

@property (readonly) BOOL motor;
@property (readonly) NSInteger cylinder;

@property (readonly) u64 fnv;

- (ADFFileProxy *)convertDisk;

@end


//
// F I L E   T Y P E S
//

//
// AmigaFile
//

@interface AmigaFileProxy : NSObject {
    
    struct AmigaFileWrapper *wrapper;
}

- (struct AmigaFileWrapper *)wrapper;

@property (readonly) AmigaFileType type;
- (void)setPath:(NSString *)path;
@property (readonly) NSInteger sizeOnDisk;
@property (readonly) u64 fnv;

- (void)seek:(NSInteger)offset;
- (NSInteger)read;

- (void)readFromBuffer:(const void *)buffer length:(NSInteger)length;
- (NSInteger)writeToBuffer:(void *)buffer;

@end

//
// Snapshot
//

@interface SnapshotProxy : AmigaFileProxy {
    
    NSImage *preview;
}

+ (BOOL)isSupportedSnapshot:(const void *)buffer length:(NSInteger)length;
+ (BOOL)isUnsupportedSnapshot:(const void *)buffer length:(NSInteger)length;
+ (BOOL)isSupportedSnapshotFile:(NSString *)path;
+ (BOOL)isUnsupportedSnapshotFile:(NSString *)path;
+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)length;
+ (instancetype)makeWithFile:(NSString *)path;
+ (instancetype)makeWithAmiga:(AmigaProxy *)amiga;

@property (readonly, strong) NSImage *previewImage;
@property (readonly) time_t timeStamp;
@property (readonly, copy) NSData *data;

@end


//
// ADFFile
//

@interface ADFFileProxy : AmigaFileProxy {
}

+ (BOOL)isADFFile:(NSString *)path;
+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)length;
+ (instancetype)makeWithFile:(NSString *)path;
+ (instancetype)makeWithDiskType:(DiskType)type;
+ (instancetype)makeWithDrive:(DriveProxy *)drive;

@property (readonly) DiskType diskType;
@property (readonly) NSInteger numCylinders;
@property (readonly) NSInteger numHeads;
@property (readonly) NSInteger numTracks;
@property (readonly) NSInteger numSectors;
@property (readonly) NSInteger numSectorsPerTrack;
// @property (readonly) u64 fnv;

- (void)formatDisk:(FileSystemType)fs;
// - (void)seekTrack:(NSInteger)nr;
// - (void)seekSector:(NSInteger)nr;

@end


//
// DMSFile
//

@interface DMSFileProxy : AmigaFileProxy {
}

+ (BOOL)isDMSFile:(NSString *)path;
+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)length;
+ (instancetype)makeWithFile:(NSString *)path;

- (ADFFileProxy *)adf;

@end


//
// IMGFile
//

@interface IMGFileProxy : AmigaFileProxy {
}

+ (BOOL)isIMGFile:(NSString *)path;
+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)length;
+ (instancetype)makeWithFile:(NSString *)path;

@property (readonly) DiskType diskType;
@property (readonly) NSInteger numCylinders;
@property (readonly) NSInteger numHeads;
@property (readonly) NSInteger numTracks;
@property (readonly) NSInteger numSectors;
@property (readonly) NSInteger numSectorsPerTrack;
@property (readonly) u64 fnv;

@end
