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
