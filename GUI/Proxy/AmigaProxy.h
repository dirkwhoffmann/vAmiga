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

#import "vastd.h"

// Forward declarations
@class MemProxy;
@class DMAControllerProxy;
@class DeniseProxy;


/* Forward declarations of C++ class wrappers.
 * We wrap classes into normal C structs to avoid any reference to C++.
 */
struct AmigaWrapper;
struct MemWrapper;
struct DMAControllerWrapper;
struct DeniseWrapper;


//
// Amiga proxy
//

@interface AmigaProxy : NSObject {
    
    struct AmigaWrapper *wrapper;
    
    MemProxy *mem;
    DMAControllerProxy *dma;
    DeniseProxy *denise;
}

@property (readonly) struct AmigaWrapper *wrapper;
@property (readonly) MemProxy *mem;
@property (readonly) DMAControllerProxy *dma;
@property (readonly) DeniseProxy *denise;

// Called when quitting the app
- (void) kill;

- (BOOL) releaseBuild;

- (void) powerOn;
- (void) powerOff;
- (void) powerOnOrOff;
- (void) reset;
- (void) ping;
- (void) dump;

- (BOOL) readyToPowerUp;
- (BOOL) isRunning;
- (BOOL) isPaused;
- (void) run;
- (void) pause;
- (void) runOrPause;

- (void) suspend;
- (void) resume;

- (AmigaConfiguration) config;
- (BOOL) configureModel:(NSInteger)model;
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


// - (BOOL) warp;
- (BOOL) alwaysWarp;
- (void) setAlwaysWarp:(BOOL)value;
- (BOOL) warpLoad;
- (void) setWarpLoad:(BOOL)value;

// Handling ROMs

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
- (void) fakeFrame;
- (void) initFakePictures:(void *)fake1 fake2:(void *)fake2;
- (void *) screenBuffer;

@end
