//
// This file is part of VirtualC64 - A cycle accurate Commodore 64 emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
//

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>
#import "C64_types.h"
#import "basic.h"

// Forward declarations of proxy classes
@class MyController;
@class C64Proxy;


// Forward declarations of wrappers for C++ classes.
// We wrap classes into normal C structs to avoid any reference to C++.

struct C64Wrapper;



// -----------------------------------------------------------------------------
//                                   C64 proxy
// -----------------------------------------------------------------------------

@interface C64Proxy : NSObject {
    
    struct C64Wrapper *wrapper;
}

@property (readonly) struct C64Wrapper *wrapper;

- (void) kill;


// Running the emulator
/*
- (void) powerUp;
- (void) run;
- (void) halt;
- (void) suspend;
- (void) resume;
- (BOOL) isRunnable;
- (BOOL) isRunning;
- (BOOL) isHalted;
- (void) step;
- (void) stepOver;
*/

// Managing the execution thread
/*
- (BOOL) warp;
- (BOOL) alwaysWarp;
- (void) setAlwaysWarp:(BOOL)b;
- (BOOL) warpLoad;
- (void) setWarpLoad:(BOOL)b;
*/

// Handling ROMs
- (BOOL) isBasicRom:(NSURL *)url;
- (BOOL) loadBasicRom:(NSURL *)url;
- (BOOL) isBasicRomLoaded;
- (uint64_t) basicRomFingerprint;
- (BOOL) isCharRom:(NSURL *)url;
- (BOOL) loadCharRom:(NSURL *)url;
- (BOOL) isCharRomLoaded;
- (uint64_t) charRomFingerprint;
- (BOOL) isKernalRom:(NSURL *)url;
- (BOOL) loadKernalRom:(NSURL *)url;
- (BOOL) isKernalRomLoaded;
- (uint64_t) kernalRomFingerprint;
- (BOOL) isVC1541Rom:(NSURL *)url;
- (BOOL) loadVC1541Rom:(NSURL *)url;
- (BOOL) isVC1541RomLoaded;
- (uint64_t) vc1541RomFingerprint;

- (BOOL) isRom:(NSURL *)url;
- (BOOL) loadRom:(NSURL *)url;

@end



// -----------------------------------------------------------------------------
//                                 CIA proxy
// -----------------------------------------------------------------------------
/*
@interface CIAProxy : NSObject {
    
    struct CiaWrapper *wrapper;
}

- (CIAInfo) getInfo;
- (void) dump;
- (BOOL) tracing;
- (void) setTracing:(BOOL)b;

- (NSInteger) model;
- (void) setModel:(NSInteger)value;
- (BOOL) emulateTimerBBug;
- (void) setEmulateTimerBBug:(BOOL)value;

- (void) poke:(uint16_t)addr value:(uint8_t)value;

@end

*/
