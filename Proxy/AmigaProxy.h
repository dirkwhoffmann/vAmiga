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

#import "Utils.h"

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
@property (readonly) AgnusProxy *agnus;
@property (readonly) BlitterProxy *blitter;
@property (readonly) CIAProxy *ciaA;
@property (readonly) CIAProxy *ciaB;
@property (readonly) ControlPortProxy *controlPort1;
@property (readonly) ControlPortProxy *controlPort2;
@property (readonly) CopperProxy *copper;
@property (readonly) CPUProxy *cpu;
@property (readonly) DeniseProxy *denise;
@property (readonly) DiskControllerProxy *diskController;
@property (readonly) DmaDebuggerProxy *dmaDebugger;
@property (readonly) DriveProxy *df0;
@property (readonly) DriveProxy *df1;
@property (readonly) DriveProxy *df2;
@property (readonly) DriveProxy *df3;
@property (readonly) JoystickProxy *joystick1;
@property (readonly) JoystickProxy *joystick2;
@property (readonly) KeyboardProxy *keyboard;
@property (readonly) MemProxy *mem;
@property (readonly) MouseProxy *mouse1;
@property (readonly) MouseProxy *mouse2;
@property (readonly) PaulaProxy *paula;
@property (readonly) SerialPortProxy *serialPort;

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
- (NSInteger) getConfig:(ConfigOption)option;
- (NSInteger) getConfig:(ConfigOption)option drive:(NSInteger)nr;
- (BOOL) configure:(ConfigOption)opt value:(NSInteger)val;
- (BOOL) configure:(ConfigOption)opt enable:(BOOL)val;
- (BOOL) configure:(ConfigOption)opt drive:(NSInteger)nr value:(NSInteger)val;
- (BOOL) configure:(ConfigOption)opt drive:(NSInteger)nr  enable:(BOOL)val;

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
// CPU
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
// Agnus
//

@interface AgnusProxy : NSObject {
    
    struct AgnusWrapper *wrapper;
}

- (NSInteger) chipRamLimit;

- (void) dump;
- (void) dumpEvents;

- (AgnusInfo) getInfo;
- (EventSlotInfo) getEventSlotInfo:(NSInteger)slot;
- (EventInfo) getEventInfo;
- (AgnusStats) getStats;

- (BOOL) interlaceMode;

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
- (void) setDevice:(SerialPortDevice)value;

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
- (BOOL) autofire;
- (void) setAutofire:(BOOL)value;
- (NSInteger) autofireBullets;
- (void) setAutofireBullets:(NSInteger)value;
- (float) autofireFrequency;
- (void) setAutofireFrequency:(float)value;

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
- (NSInteger) selectedDrive;
- (DriveState) state;
- (BOOL) spinning;
- (BOOL) connected:(NSInteger)nr;
- (void) setConnected:(NSInteger)nr value:(BOOL)value;
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
// F I L E   T Y P E S
//

//
// AmigaFile
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

- (NSImage *)previewImage;
- (time_t)timeStamp;
- (NSData *)data;

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
