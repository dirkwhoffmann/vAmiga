// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#import "AmigaProxy.h"
#import "Amiga.h"
#import "vAmiga-Swift.h"

struct AmigaWrapper { Amiga *amiga; };
// struct AmigaMemoryWrapper { AmigaMemory *mem; };


//
// Amiga
//

@implementation AmigaProxy

@synthesize wrapper;
// @synthesize mem;

- (instancetype) init
{
    NSLog(@"AmigaProxy::init");
    
    if (!(self = [super init]))
        return self;
    
    Amiga *amiga = new Amiga();
    wrapper = new AmigaWrapper();
    wrapper->amiga = amiga;
    
    // Create sub proxys
    // mem = [[AmigaMemoryProxy alloc] initWithMemory:&amiga->mem];
    
    return self;
}

- (void) kill
{
    assert(wrapper->amiga != NULL);
    NSLog(@"AmigaProxy::kill");
    
    // Kill the emulator
    delete wrapper->amiga;
    wrapper->amiga = NULL;
}

- (BOOL) releaseBuild
{
    return releaseBuild(); // see vastd.h
}

- (void) powerOn
{
    return wrapper->amiga->powerOn();
}
- (void) powerOff
{
    return wrapper->amiga->powerOff();
}
- (void) powerOnOrOff
{
    return wrapper->amiga->powerOnOrOff();
}
- (void) reset
{
    return wrapper->amiga->reset();
}
- (void) ping
{
    return wrapper->amiga->ping();
}
- (void) dump
{
    return wrapper->amiga->dump();
}

- (void) suspend
{
    return wrapper->amiga->suspend();
}
- (void) resume
{
    return wrapper->amiga->resume();
}

- (NSInteger) model
{
    return wrapper->amiga->getModel();
}
- (void) setModel:(NSInteger)model
{
    wrapper->amiga->setModel((AmigaModel)model);
}

- (void) addListener:(const void *)sender function:(Callback *)func
{
    wrapper->amiga->addListener(sender, func);
}
- (void) removeListener:(const void *)sender
{
    wrapper->amiga->removeListener(sender);
}
- (Message)message
{
    return wrapper->amiga->getMessage();
}

- (BOOL) isReadyToGo
{
    return wrapper->amiga->isReadyToGo();
}
- (BOOL) isRunning
{
    return wrapper->amiga->isRunning();
}
- (BOOL) isHalted
{
    return wrapper->amiga->isHalted();
}
- (void) run
{
    wrapper->amiga->run();
}
- (void) halt
{
    wrapper->amiga->halt();
}
- (void) runOrHalt
{
    wrapper->amiga->runOrHalt();
}

- (BOOL) alwaysWarp
{
    return wrapper->amiga->getAlwaysWarp();
}
- (void) setAlwaysWarp:(BOOL)value
{
    wrapper->amiga->setAlwaysWarp(value);
}
- (BOOL) warpLoad
{
    return wrapper->amiga->getWarpLoad();
}
- (void) setWarpLoad:(BOOL)value
{
    wrapper->amiga->setWarpLoad(value);
}

@end
