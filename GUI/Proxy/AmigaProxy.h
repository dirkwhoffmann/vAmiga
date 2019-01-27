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
@class MemProxy;
@class DMAControllerProxy;
@class DeniseProxy;
@class PaulaProxy;
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
struct MemWrapper;
struct DMAControllerWrapper;
struct DeniseWrapper;
struct PaulaWrapper;
struct AmigaKeyboardWrapper;
struct DiskControllerWrapper;
struct AmigaDriveWrapper;
struct AmigaFileWrapper;

//
// Amiga proxy
//

@interface AmigaProxy : NSObject {
    
    struct AmigaWrapper *wrapper;
    
    MemProxy *mem;
    DMAControllerProxy *dma;
    DeniseProxy *denise;
    PaulaProxy *paula;
    AmigaKeyboardProxy *keyboard;
    DiskControllerProxy *diskController;
    AmigaDriveProxy *df0;
    AmigaDriveProxy *df1;
}

@property (readonly) struct AmigaWrapper *wrapper;
@property (readonly) MemProxy *mem;
@property (readonly) DMAControllerProxy *dma;
@property (readonly) DeniseProxy *denise;
@property (readonly) PaulaProxy *paula;
@property (readonly) AmigaKeyboardProxy *keyboard;
@property (readonly) DiskControllerProxy *diskController;
@property (readonly) AmigaDriveProxy *df0;
@property (readonly) AmigaDriveProxy *df1;


// Called when quitting the app
- (void) kill;

- (BOOL) releaseBuild;

- (uint64_t) masterClock;

- (void) powerOn;
- (void) powerOff;
- (void) powerOnOrOff;
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
- (void) runOrPause;

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

- (BOOL) alwaysWarp;
- (void) setAlwaysWarp:(BOOL)value;
- (BOOL) warpLoad;
- (void) setWarpLoad:(BOOL)value;

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
// Memory Proxy
//

@interface MemProxy : NSObject {
    
    struct MemWrapper *wrapper;
}

- (void) dump;

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
- (NSInteger) volume;
- (NSInteger) bufferUnderflows;
- (NSInteger) bufferOverflows;
- (double) fillLevel;

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

