//
// This file is part of VirtualC64 - A cycle accurate Commodore 64 emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
//


#import "C64Proxy.h"
#import "C64.h"
#import "vAmiga-Swift.h"

struct C64Wrapper { C64 *c64; };





//
// CIA proxy
//

/*
@implementation CIAProxy

// Constructing
- (instancetype) initWithCIA:(CIA *)cia
{
    if (self = [super init]) {
        wrapper = new CiaWrapper();
        wrapper->cia = cia;
    }
    return self;
}

// Proxy functions
- (CIAInfo) getInfo
{
    return wrapper->cia->getInfo();
}
- (void) dump
{
    wrapper->cia->dump();
}
- (BOOL) tracing
{
    return wrapper->cia->tracingEnabled();
}
- (void) setTracing:(BOOL)b
{
    b ? wrapper->cia->startTracing() : wrapper->cia->stopTracing();
}
- (NSInteger) model
{
    return (NSInteger)wrapper->cia->getModel();
}
- (void) setModel:(NSInteger)value
{
    wrapper->cia->setModel((CIAModel)value);
}
- (BOOL) emulateTimerBBug
{
    return wrapper->cia->getEmulateTimerBBug();
}
- (void) setEmulateTimerBBug:(BOOL)value
{
    wrapper->cia->setEmulateTimerBBug(value);
}
- (void) poke:(uint16_t)addr value:(uint8_t)value {
    wrapper->cia->suspend();
    wrapper->cia->poke(addr, value);
    wrapper->cia->resume();
}

@end
*/


//
// C64
//

@implementation C64Proxy

@synthesize wrapper;

- (instancetype) init
{
	NSLog(@"C64Proxy::init");
	
    if (!(self = [super init]))
        return self;
    
    C64 *c64 = new C64();
    wrapper = new C64Wrapper();
    wrapper->c64 = c64;
	
    return self;
}

- (void) kill
{
    assert(wrapper->c64 != NULL);
    NSLog(@"C64Proxy::kill");
    
    // Kill the emulator
    delete wrapper->c64;
    wrapper->c64 = NULL;
}

/*
- (void) ping
{
    wrapper->c64->ping();
}
- (void) dump
{
    wrapper->c64->dump();
}
- (BOOL) developmentMode
{
    return wrapper->c64->developmentMode();
}

// Configuring the emulator
- (NSInteger) model
{
    return wrapper->c64->getModel();
}
- (void) setModel:(NSInteger)value
{
    wrapper->c64->setModel((C64Model)value);
}
*/

// Accessing the message queue
- (Message)message
{
    return wrapper->c64->getMessage();
}
- (void) addListener:(const void *)sender function:(Callback *)func
{
    wrapper->c64->addListener(sender, func);
}
- (void) removeListener:(const void *)sender
{
    wrapper->c64->removeListener(sender);
}

// Running the emulator
- (void) powerUp
{
    wrapper->c64->powerUp();
}
- (void) run
{
    wrapper->c64->run();
}
- (void) halt
{
    wrapper->c64->halt();
}
- (void) suspend
{
    wrapper->c64->suspend();
}
- (void) resume
{
    wrapper->c64->resume();
}
- (BOOL) isRunnable
{
    return wrapper->c64->isRunnable();
}
- (BOOL) isRunning
{
    return wrapper->c64->isRunning();
}
- (BOOL) isHalted
{
    return wrapper->c64->isHalted();
}
- (void) step
{
    wrapper->c64->step();
}
- (void) stepOver
{
    wrapper->c64->stepOver();
}

// Managing the execution thread
/*
- (BOOL) warp
{
    return wrapper->c64->getWarp();
}
- (BOOL) alwaysWarp
{
    return wrapper->c64->getAlwaysWarp();
}
- (void) setAlwaysWarp:(BOOL)b
{
    wrapper->c64->setAlwaysWarp(b);
}
- (BOOL) warpLoad
{
    return wrapper->c64->getWarpLoad();
}
- (void) setWarpLoad:(BOOL)b
{
    wrapper->c64->setWarpLoad(b);
}
*/


// Handling ROMs
- (BOOL) isBasicRom:(NSURL *)url
{
    return ROMFile::isBasicRomFile([[url path] UTF8String]);
}
- (BOOL) loadBasicRom:(NSURL *)url
{
    return [self isBasicRom:url] && wrapper->c64->loadRom([[url path] UTF8String]);
}
- (BOOL) isBasicRomLoaded
{
    return wrapper->c64->mem.basicRomIsLoaded();
}
- (uint64_t) basicRomFingerprint
{
    return wrapper->c64->mem.basicRomFingerprint();
}
- (BOOL) isCharRom:(NSURL *)url
{
    return ROMFile::isCharRomFile([[url path] UTF8String]);
}
- (BOOL) loadCharRom:(NSURL *)url
{
    return [self isCharRom:url] && wrapper->c64->loadRom([[url path] UTF8String]);
}
- (BOOL) isCharRomLoaded
{
    return wrapper->c64->mem.characterRomIsLoaded();
}
- (uint64_t) charRomFingerprint
{
    return wrapper->c64->mem.characterRomFingerprint();
}
- (BOOL) isKernalRom:(NSURL *)url
{
    return ROMFile::isKernalRomFile([[url path] UTF8String]);
}
- (BOOL) loadKernalRom:(NSURL *)url
{
    return [self isKernalRom:url] && wrapper->c64->loadRom([[url path] UTF8String]);
}
- (BOOL) isKernalRomLoaded
{
    return wrapper->c64->mem.kernalRomIsLoaded();
}
- (uint64_t) kernalRomFingerprint
{
    return wrapper->c64->mem.kernalRomFingerprint();
}
- (BOOL) isVC1541Rom:(NSURL *)url
{
    return ROMFile::isVC1541RomFile([[url path] UTF8String]);
}
- (BOOL) loadVC1541Rom:(NSURL *)url
{
    return [self isVC1541Rom:url] && wrapper->c64->loadRom([[url path] UTF8String]);
}
- (BOOL) isVC1541RomLoaded
{
    return wrapper->c64->drive1.mem.romIsLoaded() && wrapper->c64->drive2.mem.romIsLoaded();
}
- (uint64_t) vc1541RomFingerprint
{
    return wrapper->c64->drive1.mem.romFingerprint();
}
- (BOOL) isRom:(NSURL *)url
{
    return [self isBasicRom:url] || [self isCharRom:url] || [self isKernalRom:url] || [self isVC1541Rom:url];
}
- (BOOL) loadRom:(NSURL *)url
{
    return [self loadBasicRom:url] || [self loadCharRom:url] || [self loadKernalRom:url] || [self loadVC1541Rom:url];
}


@end
