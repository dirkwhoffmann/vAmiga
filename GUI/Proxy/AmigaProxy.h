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
@class DMAControllerProxy;
@class DeniseProxy;
@class PaulaProxy;
@class AmigaControlPortProxy;
@class AmigaKeyboardProxy;
@class DiskControllerProxy;
@class AmigaDriveProxy;
@class AmigaFileProxy;
@class ADFFileProxy;
@class AmigaSnapshotProxy;

/* Forward declarations of C++ class wrappers.
 * We wrap classes into normal C structs to avoid any reference to C++.
 */
struct AmigaWrapper;
struct CPUWrapper;
struct CIAWrapper;
struct MemWrapper;
struct DMAControllerWrapper;
struct DeniseWrapper;
struct PaulaWrapper;
struct AmigaControlPortWrapper;
struct AmigaKeyboardWrapper;
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
    DMAControllerProxy *dma;
    DeniseProxy *denise;
    PaulaProxy *paula;
    AmigaControlPortProxy *controlPort1;
    AmigaControlPortProxy *controlPort2;
    AmigaKeyboardProxy *keyboard;
    DiskControllerProxy *diskController;
    AmigaDriveProxy *df0;
    AmigaDriveProxy *df1;
}

@property (readonly) struct AmigaWrapper *wrapper;
@property (readonly) CPUProxy *cpu;
@property (readonly) CIAProxy *ciaA;
@property (readonly) CIAProxy *ciaB;
@property (readonly) MemProxy *mem;
@property (readonly) DMAControllerProxy *dma;
@property (readonly) DeniseProxy *denise;
@property (readonly) PaulaProxy *paula;
@property (readonly) AmigaControlPortProxy *controlPort1;
@property (readonly) AmigaControlPortProxy *controlPort2;
@property (readonly) AmigaKeyboardProxy *keyboard;
@property (readonly) DiskControllerProxy *diskController;
@property (readonly) AmigaDriveProxy *df0;
@property (readonly) AmigaDriveProxy *df1;


- (void) makeActiveInstance;
- (void) kill;

- (BOOL) releaseBuild;

- (uint64_t) masterClock;

- (void) powerOn;
- (void) powerOff;
- (void) reset;
- (void) ping;
- (void) dump;

- (BOOL) readyToPowerUp;
- (BOOL) isPoweredOn;
- (BOOL) isPoweredOff;
- (BOOL) isRunning;
- (BOOL) isPaused;
- (void) run;
- (void) pause;
// - (void) runOrPause;

- (void) suspend;
- (void) resume;

- (AmigaConfiguration) config;
- (BOOL) configureModel:(NSInteger)model;
- (BOOL) configureLayout:(NSInteger)value;
- (BOOL) configureChipMemory:(NSInteger)size;
- (BOOL) configureSlowMemory:(NSInteger)size;
- (BOOL) configureFastMemory:(NSInteger)size;
- (BOOL) configureRealTimeClock:(BOOL)value;
- (BOOL) configureDrive:(NSInteger)driveNr connected:(BOOL)value;
- (BOOL) configureDrive:(NSInteger)driveNr type:(NSInteger)value;

- (BOOL) hasBootRom;
- (void) deleteBootRom;
- (BOOL) isBootRom:(NSURL *)url;
- (BOOL) loadBootRomFromBuffer:(NSData *)buffer;
- (BOOL) loadBootRomFromFile:(NSURL *)url;
- (uint64_t) bootRomFingerprint;

- (BOOL) hasKickRom;
- (void) deleteKickRom;
- (BOOL) isKickRom:(NSURL *)url;
- (BOOL) loadKickRomFromBuffer:(NSData *)buffer;
- (BOOL) loadKickRomFromFile:(NSURL *)url;
- (uint64_t) kickRomFingerprint;

// Message queue
- (void) addListener:(const void *)sender function:(Callback *)func;
- (void) removeListener:(const void *)sender;
- (Message)message;

- (BOOL) warp;
- (void) setWarp:(BOOL)value;

/*
- (BOOL) alwaysWarp;
- (void) setAlwaysWarp:(BOOL)value;
- (BOOL) warpLoad;
- (void) setWarpLoad:(BOOL)value;
*/

// Handling snapshots
- (BOOL) takeAutoSnapshots;
- (void) setTakeAutoSnapshots:(BOOL)b;
- (void) suspendAutoSnapshots;
- (void) resumeAutoSnapshots;
- (NSInteger) snapshotInterval;
- (void) setSnapshotInterval:(NSInteger)value;
 
- (void) loadFromSnapshot:(AmigaSnapshotProxy *)proxy;

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

- (BOOL) tracing;
- (void) setTracing:(BOOL)b;
- (NSInteger) disassemble:(char *) str pc:(NSInteger)pc;

- (BOOL) hasBreakpointAt:(uint32_t)addr;
- (BOOL) hasConditionalBreakpointAt:(uint32_t)addr;
- (void) toggleBreakpointAt:(uint32_t)addr;

- (NSInteger) numberOfBreakpoints;
- (BOOL) hasCondition:(NSInteger)nr;
- (BOOL) hasSyntaxError:(NSInteger)nr;
- (uint32_t) breakpointAddr:(NSInteger)nr;
- (BOOL) setBreakpointAddr:(NSInteger)nr addr:(uint32_t)addr;
- (NSString *) breakpointCondition:(NSInteger)nr;
- (BOOL) setBreakpointCondition:(NSInteger)nr cond:(NSString *)cond;

@end


//
// CIA Proxy
//

@interface CIAProxy : NSObject {
    
    struct CIAWrapper *wrapper;
}

- (void) dump;
- (CIAInfo) getInfo;
- (void) poke:(uint16_t)addr value:(uint8_t)value;

@end


//
// Memory Proxy
//

@interface MemProxy : NSObject {
    
    struct MemWrapper *wrapper;
}

- (void) dump;
- (MemorySource *) getMemSrcTable; 
- (MemorySource) memSrc:(NSInteger)addr;
- (NSInteger) spypeek8:(NSInteger)addr;
- (NSInteger) spypeek16:(NSInteger)addr;
- (void) poke8:(NSInteger)addr value:(NSInteger)value;
- (void) poke16:(NSInteger)addr value:(NSInteger)value;

- (NSString *) ascii:(NSInteger)addr;
- (NSString *) hex:(NSInteger)addr bytes:(NSInteger)bytes;

@end


//
// DMAController Proxy
//

@interface DMAControllerProxy : NSObject {
    
    struct DMAControllerWrapper *wrapper;
}

- (void) dump;

@end


//
// Denise Proxy
//

@interface DeniseProxy : NSObject {
    
    struct DeniseWrapper *wrapper;
}

- (void) dump;
- (void) initFakePictures:(void *)fake1 fake2:(void *)fake2;
- (void *) screenBuffer;

@end


//
// Paula Proxy
//

@interface PaulaProxy : NSObject {
    
    struct PaulaWrapper *wrapper;
}

- (void) dump;

- (uint32_t) sampleRate;
- (void) setSampleRate:(double)rate;

- (NSInteger) ringbufferSize;
- (float) ringbufferData:(NSInteger)offset;
- (double) fillLevel;
- (NSInteger) bufferUnderflows;
- (NSInteger) bufferOverflows;

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

@interface AmigaControlPortProxy : NSObject {
    
    struct AmigaControlPortWrapper *wrapper;
}

- (void) dump;

- (void) trigger:(JoystickEvent)event;
- (BOOL) autofire;
- (void) setAutofire:(BOOL)value;
- (NSInteger) autofireBullets;
- (void) setAutofireBullets:(NSInteger)value;
- (float) autofireFrequency;
- (void) setAutofireFrequency:(float)value;

- (void) connectMouse:(BOOL)value;

- (void) setXY:(NSPoint)pos;
- (void) setLeftMouseButton:(BOOL)pressed;
- (void) setRightMouseButton:(BOOL)pressed;

@end


//
// Keyboard Proxy
//

@interface AmigaKeyboardProxy : NSObject {
    
    struct AmigaKeyboardWrapper *wrapper;
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
- (BOOL) doesDMA:(NSInteger)nr;

@end


//
// AmigaDrive Proxy
//

@interface AmigaDriveProxy : NSObject {
    
    struct AmigaDriveWrapper *wrapper;
}

- (void) dump;

- (NSInteger) nr;

- (BOOL) isConnected;
- (void) setConnected:(BOOL)value;
- (void) toggleConnected;

- (BOOL) hasDisk;
- (BOOL) hasWriteProtectedDisk;
- (BOOL) hasModifiedDisk;
- (void) setModifiedDisk:(BOOL)value; 
- (void) ejectDisk;
- (void) insertDisk:(ADFFileProxy *)file;
- (void) toggleWriteProtection;

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

@interface AmigaSnapshotProxy : AmigaFileProxy {
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
+ (instancetype) makeWithBuffer:(const void *)buffer length:(NSInteger)length;
+ (instancetype) makeWithFile:(NSString *)path;
+ (instancetype) make;

- (void)seekTrack:(NSInteger)nr;
- (void)seekSector:(NSInteger)nr;

@end

