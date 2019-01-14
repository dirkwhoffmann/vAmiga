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
// @class MyController;
// @class AmigaMemoryProxy;


// Forward declarations of C++ class wrappers.
// We wrap classes into normal C structs to avoid any reference to C++.
struct AmigaWrapper;


//
// Amiga proxy
//

@interface AmigaProxy : NSObject {
    
    struct AmigaWrapper *wrapper;
    
    // AmigaMemoryProxy *mem;
}

@property (readonly) struct AmigaWrapper *wrapper;
// @property (readonly) AmigaMemoryProxy *mem;

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
