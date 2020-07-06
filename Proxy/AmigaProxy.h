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

#import "AmigaUtils.h"

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
    MouseProxy *mouse1;
    MouseProxy *mouse2;
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
@property (readonly) MouseProxy *mouse1;
@property (readonly) MouseProxy *mouse2;
@property (readonly) JoystickProxy *joystick1;
@property (readonly) JoystickProxy *joystick2;
@property (readonly) KeyboardProxy *keyboard;
@property (readonly) DiskControllerProxy *diskController;
@property (readonly) DriveProxy *df0;
@property (readonly) DriveProxy *df1;
@property (readonly) DriveProxy *df2;
@property (readonly) DriveProxy *df3;

- (void) kill;

- (BOOL) releaseBuild;

- (void) setInspectionTarget:(EventID)id;
- (void) clearInspectionTarget;
- (BOOL) debugMode;
- (void) enableDebugging;
- (void) disableDebugging;

- (BOOL) isReady:(ErrorCode *)error;
- (BOOL) isReady;
- (void) powerOn;
- (void) powerOff;
- (void) hardReset;
- (void) softReset;
- (void) ping;
- (void) dump;

- (AmigaInfo) getInfo;

- (BOOL) isPoweredOn;
- (BOOL) isPoweredOff;
- (BOOL) isRunning;
- (BOOL) isPaused;
- (void) run;
- (void) pause;

- (void) suspend;
- (void) resume;

- (void) requestAutoSnapshot;
- (void) requestUserSnapshot;
- (SnapshotProxy *) latestAutoSnapshot;
- (SnapshotProxy *) latestUserSnapshot;

- (AmigaConfiguration) config;
- (BOOL) configure:(ConfigOption)opt value:(NSInteger)val;
- (BOOL) configure:(ConfigOption)opt enable:(BOOL)val;
- (BOOL) configure:(ConfigOption)opt drive:(NSInteger)nr value:(NSInteger)val;
- (BOOL) configure:(ConfigOption)opt drive:(NSInteger)nr  enable:(BOOL)val;

- (NSInteger) getConfig:(ConfigOption)option;
- (NSInteger) getConfig:(ConfigOption)option drive:(NSInteger)nr;


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

- (void) loadFromSnapshot:(SnapshotProxy *)proxy;

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

- (i64) clock;
- (i64) cycles;
- (bool) isHalted;

- (NSInteger) numberOfBreakpoints;
- (u32) breakpointAddr:(NSInteger)nr;
- (BOOL) breakpointIsEnabled:(NSInteger)nr;
- (BOOL) breakpointIsDisabled:(NSInteger)nr;
- (void) breakpointSetEnable:(NSInteger)nr value:(BOOL)val;
- (void) removeBreakpoint:(NSInteger)nr;
- (void) replaceBreakpoint:(NSInteger)nr addr:(u32)addr;

- (BOOL) breakpointIsSetAt:(u32)addr;
- (BOOL) breakpointIsSetAndEnabledAt:(u32)addr;
- (BOOL) breakpointIsSetAndDisabledAt:(u32)addr;
- (void) breakpointSetEnableAt:(u32)addr value:(BOOL)val;
- (void) addBreakpointAt:(u32)addr;
- (void) removeBreakpointAt:(u32)addr;

- (NSInteger) numberOfWatchpoints;
- (u32) watchpointAddr:(NSInteger)nr;
- (BOOL) watchpointIsEnabled:(NSInteger)nr;
- (BOOL) watchpointIsDisabled:(NSInteger)nr;
- (void) watchpointSetEnable:(NSInteger)nr value:(BOOL)val;
- (void) removeWatchpoint:(NSInteger)nr;
- (void) replaceWatchpoint:(NSInteger)nr addr:(u32)addr;

- (BOOL) watchpointIsSetAt:(u32)addr;
- (BOOL) watchpointIsSetAndEnabledAt:(u32)addr;
- (BOOL) watchpointIsSetAndDisabledAt:(u32)addr;
- (void) addWatchpointAt:(u32)addr;
- (void) removeWatchpointAt:(u32)addr;

- (NSInteger) loggedInstructions;
- (void) clearLog;

- (DisassembledInstr) disassembleInstr:(u32)addr;

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

- (MemoryConfig) getConfig;
- (MemoryStats) getStats;
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
- (u64) romFingerprint;
- (RomRevision) romRevision;
- (NSString *) romTitle;
- (NSString *) romVersion;
- (NSString *) romReleased;

- (BOOL) hasExt;
- (void) deleteExt;
- (BOOL) isExt:(NSURL *)url;
- (BOOL) loadExtFromBuffer:(NSData *)buffer;
- (BOOL) loadExtFromFile:(NSURL *)url;
- (u64) extFingerprint;
- (RomRevision) extRevision;
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
- (AgnusStats) getStats;

- (BOOL) interlaceMode;

- (BOOL) isIllegalInstr:(NSInteger)addr;
- (NSInteger) instrCount:(NSInteger)list;
- (void) adjustInstrCount:(NSInteger)list offset:(NSInteger)offset;
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

- (NSInteger) sprDataLines:(NSInteger)nr;
- (u64) sprData:(NSInteger)nr line:(NSInteger)line;
- (u16) sprColor:(NSInteger)nr reg:(NSInteger)reg;

- (double) palette;
- (void) setPalette:(Palette)p;
- (double) brightness;
- (void) setBrightness:(double)value;
- (double) saturation;
- (void) setSaturation:(double)value;
- (double) contrast;
- (void) setContrast:(double)value;

- (ScreenBuffer) stableLongFrame;
- (ScreenBuffer) stableShortFrame;
- (u32 *) noise;

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
- (UARTInfo) getUARTInfo;

- (u32) sampleRate;
- (void) setSampleRate:(double)rate;

- (double) vol:(NSInteger)nr;
- (void) setVol:(NSInteger)nr value:(double)value;
- (double) pan:(NSInteger)nr;
- (void) setPan:(NSInteger)nr value:(double)value;
- (double) volL;
- (void) setVolL:(double)value;
- (double) volR;
- (void) setVolR:(double)value;

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

- (float) drawWaveformL:(unsigned *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(unsigned)c;
- (float) drawWaveformL:(unsigned *)buffer size:(NSSize)size scale:(float)s color:(unsigned)c;
- (float) drawWaveformR:(unsigned *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(unsigned)c;
- (float) drawWaveformR:(unsigned *)buffer size:(NSSize)size scale:(float)s color:(unsigned)c;

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
- (void) setDeltaXY:(NSPoint)pos;
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
- (DiskControllerConfig) getConfig;
- (DiskControllerInfo) getInfo;
- (NSInteger) selectedDrive;
- (DriveState) state;
- (BOOL) spinning;
- (BOOL) connected:(NSInteger)nr;
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

- (DriveInfo) getInfo;
- (NSInteger) nr;
- (DriveType) type;

- (BOOL) hasDisk;
- (BOOL) hasWriteProtectedDisk;
- (void) setWriteProtection:(BOOL)value;
- (void) toggleWriteProtection;

- (BOOL) hasModifiedDisk;
- (void) setModifiedDisk:(BOOL)value;

- (BOOL) motor;
- (NSInteger) cylinder;

- (u64)  fnv;

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
    
    NSImage *preview;
}

// @property NSImage *preview;

+ (BOOL)isSupportedSnapshot:(const void *)buffer length:(NSInteger)length;
+ (BOOL)isUnsupportedSnapshot:(const void *)buffer length:(NSInteger)length;
+ (BOOL)isSupportedSnapshotFile:(NSString *)path;
+ (BOOL)isUnsupportedSnapshotFile:(NSString *)path;
+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)length;
+ (instancetype)makeWithFile:(NSString *)path;
+ (instancetype)makeWithAmiga:(AmigaProxy *)amiga;

- (NSImage *)previewImage;
- (time_t)timeStamp;
- (NSData *)data;

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
- (u64) fnv;

- (void)formatDisk:(FileSystemType)fs;
- (void)seekTrack:(NSInteger)nr;
- (void)seekSector:(NSInteger)nr;

@end

