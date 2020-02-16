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

#import "va_std.h"

// Forward declarations
@class CPUProxy;
@class CIAProxy;
@class MemProxy;
@class AgnusProxy;
@class DeniseProxy;
@class PaulaProxy;
@class ControlPortProxy;
@class SerialPortProxy;
@class MouseProxy;
@class JoystickProxy;
@class KeyboardProxy;
@class DiskControllerProxy;
@class DriveProxy;
@class AmigaFileProxy;
@class ADFFileProxy;
@class SnapshotProxy;

/* Forward declarations of C++ class wrappers.
 * We wrap classes into normal C structs to avoid any reference to C++.
 */
struct AmigaWrapper;
struct CPUWrapper;
struct CIAWrapper;
struct MemWrapper;
struct AgnusWrapper;
struct DeniseWrapper;
struct PaulaWrapper;
struct AmigaControlPortWrapper;
struct AmigaSerialPortWrapper;
struct KeyboardWrapper;
struct DiskControllerWrapper;
struct AmigaDriveWrapper;
struct AmigaFileWrapper;

//
// Amiga proxy
//

@interface AmigaProxy : NSObject {
    
    struct AmigaWrapper *wrapper;
    
    CPUProxy *cpu;
    CIAProxy *ciaA;
    CIAProxy *ciaB;
    MemProxy *mem;
    AgnusProxy *agnus;
    DeniseProxy *denise;
    PaulaProxy *paula;
    ControlPortProxy *controlPort1;
    ControlPortProxy *controlPort2;
    SerialPortProxy *serialPort;
    MouseProxy *mouse;
    JoystickProxy *joystick1;
    JoystickProxy *joystick2;
    KeyboardProxy *keyboard;
    DiskControllerProxy *diskController;
    DriveProxy *df0;
    DriveProxy *df1;
    DriveProxy *df2;
    DriveProxy *df3;
}

@property (readonly) struct AmigaWrapper *wrapper;
@property (readonly) CPUProxy *cpu;
@property (readonly) CIAProxy *ciaA;
@property (readonly) CIAProxy *ciaB;
@property (readonly) MemProxy *mem;
@property (readonly) AgnusProxy *agnus;
@property (readonly) DeniseProxy *denise;
@property (readonly) PaulaProxy *paula;
@property (readonly) ControlPortProxy *controlPort1;
@property (readonly) ControlPortProxy *controlPort2;
@property (readonly) SerialPortProxy *serialPort;
@property (readonly) MouseProxy *mouse;
@property (readonly) JoystickProxy *joystick1;
@property (readonly) JoystickProxy *joystick2;
@property (readonly) KeyboardProxy *keyboard;
@property (readonly) DiskControllerProxy *diskController;
@property (readonly) DriveProxy *df0;
@property (readonly) DriveProxy *df1;
@property (readonly) DriveProxy *df2;
@property (readonly) DriveProxy *df3;

// - (DriveProxy *)df:(NSInteger)n;

- (void) kill;

- (BOOL) releaseBuild;

- (void) setInspectionTarget:(EventID)id;
- (void) clearInspectionTarget;
- (BOOL) debugMode;
- (void) enableDebugging;
- (void) disableDebugging;
- (void) setDebugLevel:(NSInteger)value;

- (void) powerOn;
- (void) powerOff;
- (void) reset;
- (void) ping;
- (void) dump;

- (AmigaInfo) getInfo;
- (AmigaStats) getStats;

// - (BOOL) readyToPowerUp;
- (BOOL) isPoweredOn;
- (BOOL) isPoweredOff;
- (BOOL) isRunning;
- (BOOL) isPaused;
- (void) run;
- (void) pause;

- (void) suspend;
- (void) resume;

- (AmigaConfiguration) config;
- (BOOL) configure:(ConfigOption)option value:(NSInteger)value;
- (BOOL) configure:(ConfigOption)option enable:(BOOL)value;
- (BOOL) configureDrive:(NSInteger)nr connected:(BOOL)value;
- (BOOL) configureDrive:(NSInteger)nr type:(NSInteger)value;

// Message queue
- (void) addListener:(const void *)sender function:(Callback *)func;
- (void) removeListener:(const void *)sender;
- (Message)message;

- (void) stopAndGo;
- (void) stepInto;
- (void) stepOver;

- (BOOL) warp;
- (void) warpOn;
- (void) warpOff;

// Handling snapshots
- (BOOL) takeAutoSnapshots;
- (void) setTakeAutoSnapshots:(BOOL)b;
- (void) suspendAutoSnapshots;
- (void) resumeAutoSnapshots;
- (NSInteger) snapshotInterval;
- (void) setSnapshotInterval:(NSInteger)value;
 
- (void) loadFromSnapshot:(SnapshotProxy *)proxy;

- (BOOL) restoreAutoSnapshot:(NSInteger)nr;
- (BOOL) restoreUserSnapshot:(NSInteger)nr;
- (BOOL) restoreLatestAutoSnapshot;
- (BOOL) restoreLatestUserSnapshot;
- (NSInteger) numAutoSnapshots;
- (NSInteger) numUserSnapshots;

- (NSData *) autoSnapshotData:(NSInteger)nr;
- (NSData *) userSnapshotData:(NSInteger)nr;
- (unsigned char *) autoSnapshotImageData:(NSInteger)nr;
- (unsigned char *) userSnapshotImageData:(NSInteger)nr;
- (NSSize) autoSnapshotImageSize:(NSInteger)nr;
- (NSSize) userSnapshotImageSize:(NSInteger)nr;
- (time_t) autoSnapshotTimestamp:(NSInteger)nr;
- (time_t) userSnapshotTimestamp:(NSInteger)nr;

- (void) takeUserSnapshot;

- (void) deleteAutoSnapshot:(NSInteger)nr;
- (void) deleteUserSnapshot:(NSInteger)nr;

@end


//
// CPU Proxy
//

@interface CPUProxy : NSObject {
    
    struct CPUWrapper *wrapper;
}

- (void) dump;
- (CPUInfo) getInfo;
- (DisassembledInstr) getInstrInfo:(NSInteger)index;
- (DisassembledInstr) getLoggedInstrInfo:(NSInteger)index;

- (int64_t) clock;
- (int64_t) cycles;

- (NSInteger) numberOfBreakpoints;
- (uint32_t) breakpointAddr:(NSInteger)nr;
- (BOOL) breakpointIsEnabled:(NSInteger)nr;
- (BOOL) breakpointIsDisabled:(NSInteger)nr;
- (void) breakpointSetEnable:(NSInteger)nr value:(BOOL)val;
- (void) removeBreakpoint:(NSInteger)nr;

- (BOOL) breakpointIsSetAt:(uint32_t)addr;
- (BOOL) breakpointIsSetAndEnabledAt:(uint32_t)addr;
- (BOOL) breakpointIsSetAndDisabledAt:(uint32_t)addr;
- (void) breakpointSetEnableAt:(uint32_t)addr value:(BOOL)val;
- (void) addBreakpointAt:(uint32_t)addr;
- (void) removeBreakpointAt:(uint32_t)addr;

- (NSInteger) numberOfWatchpoints;
- (uint32_t) watchpointAddr:(NSInteger)nr;
- (BOOL) watchpointIsEnabled:(NSInteger)nr;
- (BOOL) watchpointIsDisabled:(NSInteger)nr;
- (void) watchpointSetEnable:(NSInteger)nr value:(BOOL)val;
- (void) removeWatchpoint:(NSInteger)nr;

- (BOOL) watchpointIsSetAt:(uint32_t)addr;
- (BOOL) watchpointIsSetAndEnabledAt:(uint32_t)addr;
- (BOOL) watchpointIsSetAndDisabledAt:(uint32_t)addr;
- (void) addWatchpointAt:(uint32_t)addr;
- (void) removeWatchpointAt:(uint32_t)addr;

// - (NSInteger) traceBufferCapacity;
// - (void) truncateTraceBuffer:(NSInteger)count;
- (NSInteger) loggedInstructions;
- (void) clearLog;

@end


//
// CIA Proxy
//

@interface CIAProxy : NSObject {
    
    struct CIAWrapper *wrapper;
}

- (void) dumpConfig;
- (void) dump;
- (CIAInfo) getInfo;

@end


//
// Memory Proxy
//

@interface MemProxy : NSObject {
    
    struct MemWrapper *wrapper;
}

- (void) dump;

- (BOOL) isBootRom:(RomRevision)rev;
- (BOOL) isArosRom:(RomRevision)rev;
- (BOOL) isDiagRom:(RomRevision)rev;
- (BOOL) isOrigRom:(RomRevision)rev;
- (BOOL) isHyperionRom:(RomRevision)rev;

- (BOOL) hasRom;
- (BOOL) hasBootRom;
- (BOOL) hasKickRom;
- (void) deleteRom;
- (BOOL) isRom:(NSURL *)url;
- (BOOL) loadRomFromBuffer:(NSData *)buffer;
- (BOOL) loadRomFromFile:(NSURL *)url;
- (uint64_t) romFingerprint;
- (RomRevision) romRevision;
- (NSString *) romTitle;
- (NSString *) romVersion;
- (NSString *) romReleased;

- (BOOL) hasExt;
- (void) deleteExt;
- (BOOL) isExt:(NSURL *)url;
- (BOOL) loadExtFromBuffer:(NSData *)buffer;
- (BOOL) loadExtFromFile:(NSURL *)url;
- (uint64_t) extFingerprint;
- (RomRevision) extRevision;
- (NSString *) extTitle;
- (NSString *) extVersion;
- (NSString *) extReleased;
- (NSInteger) extStart;

- (MemorySource *) getMemSrcTable; 
- (MemorySource) memSrc:(NSInteger)addr;
- (NSInteger) spypeek8:(NSInteger)addr;
- (NSInteger) spypeek16:(NSInteger)addr;

- (NSString *) ascii:(NSInteger)addr;
- (NSString *) hex:(NSInteger)addr bytes:(NSInteger)bytes;

@end


//
// AgnusProxy Proxy
//

@interface AgnusProxy : NSObject {
    
    struct AgnusWrapper *wrapper;
}

- (NSInteger) chipRamLimit;

- (void) dump;
- (void) dumpEvents;
- (void) dumpCopper;
- (void) dumpBlitter;

- (AgnusInfo) getInfo;
- (DMADebuggerInfo) getDebuggerInfo;
- (EventSlotInfo) getEventSlotInfo:(NSInteger)slot;
- (EventInfo) getEventInfo;
- (CopperInfo) getCopperInfo;
- (BlitterInfo) getBlitterInfo;

- (BOOL) interlaceMode;
- (BOOL) isLongFrame;
- (BOOL) isShortFrame;

- (BOOL) isIllegalInstr:(NSInteger)addr;
- (NSInteger) instrCount:(NSInteger)list;
- (NSString *) disassemble:(NSInteger)addr;
- (NSString *) disassemble:(NSInteger)list instr:(NSInteger)offset;

- (void) dmaDebugSetEnable:(BOOL)value;
- (void) dmaDebugSetVisualize:(BusOwner)owner value:(BOOL)value;
- (void) dmaDebugSetColor:(BusOwner)owner r:(double)r g:(double)g b:(double)b;
- (void) dmaDebugSetOpacity:(double)value;
- (void) dmaDebugSetDisplayMode:(NSInteger)mode;

@end


//
// Denise Proxy
//

@interface DeniseProxy : NSObject {
    
    struct DeniseWrapper *wrapper;
}

- (void) dump;
- (DeniseInfo) getInfo;
- (SpriteInfo) getSpriteInfo:(NSInteger)nr;
- (void) inspect;

/*
- (void) pokeColorReg:(NSInteger)reg value:(UInt16)value;
*/
- (double) palette;
- (void) setPalette:(Palette)p;
- (double) brightness;
- (void) setBrightness:(double)value;
- (double) saturation;
- (void) setSaturation:(double)value;
- (double) contrast;
- (void) setContrast:(double)value;

- (void) setBPU:(NSInteger)count;
- (void) setBPLCONx:(NSInteger)x value:(NSInteger)value;
- (void) setBPLCONx:(NSInteger)x bit:(NSInteger)bit value:(BOOL)value;
- (void) setBPLCONx:(NSInteger)x nibble:(NSInteger)nibble value:(NSInteger)value;

- (ScreenBuffer) stableLongFrame;
- (ScreenBuffer) stableShortFrame;
- (int32_t *) noise;

@end


//
// Paula Proxy
//

@interface PaulaProxy : NSObject {
    
    struct PaulaWrapper *wrapper;
}

- (void) dump;
- (PaulaInfo) getInfo;
- (AudioInfo) getAudioInfo;
- (AudioStats) getAudioStats;
- (DiskControllerConfig) getDiskControllerConfig;
- (DiskControllerInfo) getDiskControllerInfo;
- (UARTInfo) getUARTInfo;

- (uint32_t) sampleRate;
- (void) setSampleRate:(double)rate;

- (NSInteger) ringbufferSize;
- (double) ringbufferDataL:(NSInteger)offset;
- (double) ringbufferDataR:(NSInteger)offset;
- (double) fillLevel;

- (void) readMonoSamples:(float *)target size:(NSInteger)n;
- (void) readStereoSamples:(float *)target1 buffer2:(float *)target2 size:(NSInteger)n;
- (void) readStereoSamplesInterleaved:(float *)target size:(NSInteger)n;

- (void) rampUp;
- (void) rampUpFromZero;
- (void) rampDown;

@end


//
// ControlPort Proxy
//

@interface ControlPortProxy : NSObject {
    
    struct AmigaControlPortWrapper *wrapper;
}

- (void) dump;
- (ControlPortInfo) getInfo;
- (void) connectDevice:(ControlPortDevice)value;

@end


//
// SerialPort Proxy
//

@interface SerialPortProxy : NSObject {

    struct AmigaSerialPortWrapper *wrapper;
}

- (void) dump;
- (SerialPortInfo) getInfo;
- (void) setDevice:(SerialPortDevice)value;

@end


//
// Mouse Proxy
//

@interface MouseProxy : NSObject {
    
    struct MouseWrapper *wrapper;
}

- (void) dump;

- (void) setXY:(NSPoint)pos;
- (void) setLeftButton:(BOOL)value;
- (void) setRightButton:(BOOL)value;
- (void) trigger:(GamePadAction)event;

@end


//
// Joystick Proxy
//

@interface JoystickProxy : NSObject {
    
    struct JoystickWrapper *wrapper;
}

- (void) dump;

- (void) trigger:(GamePadAction)event;
- (BOOL) autofire;
- (void) setAutofire:(BOOL)value;
- (NSInteger) autofireBullets;
- (void) setAutofireBullets:(NSInteger)value;
- (float) autofireFrequency;
- (void) setAutofireFrequency:(float)value;

@end


//
// Keyboard Proxy
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
// DiskController Proxy
//

@interface DiskControllerProxy : NSObject {
    
    struct DiskControllerWrapper *wrapper;
}

- (void) dump;

- (BOOL) spinning:(NSInteger)nr;
- (BOOL) spinning;

// - (BOOL) isConnected:(NSInteger)nr;
- (void) setConnected:(NSInteger)nr value:(BOOL)value;

- (void) eject:(NSInteger)nr;
- (void) insert:(NSInteger)nr adf:(ADFFileProxy *)fileProxy;
- (void) setWriteProtection:(NSInteger)nr value:(BOOL)value;

@end


//
// AmigaDrive Proxy
//

@interface DriveProxy : NSObject {
    
    struct AmigaDriveWrapper *wrapper;
}

@property (readonly) struct AmigaDriveWrapper *wrapper;

- (void) dump;

- (NSInteger) nr;
- (DriveType) type;

- (BOOL) hasDisk;
- (BOOL) hasWriteProtectedDisk;
- (void) setWriteProtection:(BOOL)value;
- (void) toggleWriteProtection;

- (BOOL) hasModifiedDisk;
- (void) setModifiedDisk:(BOOL)value;

// - (void) ejectDisk;
// - (void) insertDisk:(ADFFileProxy *)file;

- (ADFFileProxy *)convertDisk;

@end


//
// F I L E   T Y P E   P R O X Y S
//

//
// AmigaFile proxy
//

@interface AmigaFileProxy : NSObject {
    
    struct AmigaFileWrapper *wrapper;
}

- (struct AmigaFileWrapper *)wrapper;

- (AmigaFileType)type;
- (void)setPath:(NSString *)path;
- (NSInteger)sizeOnDisk;

- (void)seek:(NSInteger)offset;
- (NSInteger)read;

- (void)readFromBuffer:(const void *)buffer length:(NSInteger)length;
- (NSInteger)writeToBuffer:(void *)buffer;

@end


//
// Snapshot proxy
//

@interface SnapshotProxy : AmigaFileProxy {
}

+ (BOOL)isSupportedSnapshot:(const void *)buffer length:(NSInteger)length;
+ (BOOL)isUnsupportedSnapshot:(const void *)buffer length:(NSInteger)length;
+ (BOOL)isSupportedSnapshotFile:(NSString *)path;
+ (BOOL)isUnsupportedSnapshotFile:(NSString *)path;
+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)length;
+ (instancetype)makeWithFile:(NSString *)path;
+ (instancetype)makeWithAmiga:(AmigaProxy *)amiga;

@end


//
// ADFFile proxy
//

@interface ADFFileProxy : AmigaFileProxy {
}

+ (BOOL)isADFFile:(NSString *)path;
+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)length;
+ (instancetype)makeWithFile:(NSString *)path;
+ (instancetype)makeWithDiskType:(DiskType)type;
+ (instancetype)makeWithDrive:(DriveProxy *)drive;

- (DiskType)diskType;
- (NSInteger)numCylinders;
- (NSInteger)numHeads;
- (NSInteger)numTracks;
- (NSInteger)numSectors;
- (NSInteger)numSectorsPerTrack;
- (void)formatDisk:(FileSystemType)fs;
- (void)seekTrack:(NSInteger)nr;
- (void)seekSector:(NSInteger)nr;

@end

