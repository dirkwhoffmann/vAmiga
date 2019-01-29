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
@class VICProxy;

// Forward declarations of wrappers for C++ classes.
// We wrap classes into normal C structs to avoid any reference to C++.

struct C64Wrapper;
struct VicWrapper;


// -----------------------------------------------------------------------------
//                                   C64 proxy
// -----------------------------------------------------------------------------

@interface C64Proxy : NSObject {
    
    struct C64Wrapper *wrapper;
    
    VICProxy *vic;
}

@property (readonly) struct C64Wrapper *wrapper;
@property (readonly) VICProxy *vic;

- (void) kill;

// - (void) ping;
- (void) dump;
- (BOOL) developmentMode;

// Configuring the emulator
- (NSInteger) model;
- (void) setModel:(NSInteger)value;

// Accessing the message queue
- (Message)message;
- (void) addListener:(const void *)sender function:(Callback *)func;
- (void) removeListener:(const void *)sender;

// Running the emulator
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

// Managing the execution thread
- (BOOL) warp;
- (BOOL) alwaysWarp;
- (void) setAlwaysWarp:(BOOL)b;
- (BOOL) warpLoad;
- (void) setWarpLoad:(BOOL)b;

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
//                                 Memory proxy
// -----------------------------------------------------------------------------

/*
@interface MemoryProxy : NSObject {
    
    struct MemoryWrapper *wrapper;
}

- (MemoryType) peekSource:(uint16_t)addr;
- (MemoryType) pokeTarget:(uint16_t)addr;

- (uint8_t) spypeek:(uint16_t)addr source:(MemoryType)source;
- (uint8_t) spypeek:(uint16_t)addr;
- (uint8_t) spypeekIO:(uint16_t)addr;

- (void) poke:(uint16_t)addr value:(uint8_t)value target:(MemoryType)target;
- (void) poke:(uint16_t)addr value:(uint8_t)value;
- (void) pokeIO:(uint16_t)addr value:(uint8_t)value;

@end
*/


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

// -----------------------------------------------------------------------------
//                                VICII proxy
// -----------------------------------------------------------------------------


@interface VICProxy : NSObject {
    
	struct VicWrapper *wrapper;
}

- (NSInteger) videoPalette;
- (void) setVideoPalette:(NSInteger)value;

- (double)brightness;
- (void)setBrightness:(double)value;
- (double)contrast;
- (void)setContrast:(double)value;
- (double)saturation;
- (void)setSaturation:(double)value;

@end
