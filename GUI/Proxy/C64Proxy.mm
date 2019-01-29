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
struct MemoryWrapper { C64Memory *mem; };
struct VicWrapper { VIC *vic; };
struct SidBridgeWrapper { SIDBridge *sid; };
struct ExpansionPortWrapper { ExpansionPort *expansionPort; };
struct DiskWrapper { Disk *disk; };
struct DriveWrapper { VC1541 *drive; };
struct DatasetteWrapper { Datasette *datasette; };
struct AnyC64FileWrapper { AnyC64File *file; };



//
// Memory proxy
//

@implementation MemoryProxy

// Constructing
- (instancetype) initWithMemory:(C64Memory *)mem
{
    if (self = [super init]) {
        wrapper = new MemoryWrapper();
        wrapper->mem = mem;
    }
    return self;
}

- (MemoryType) peekSource:(uint16_t)addr
{
    return wrapper->mem->getPeekSource(addr);
}
- (MemoryType) pokeTarget:(uint16_t)addr
{
    return wrapper->mem->getPokeTarget(addr);
}
- (uint8_t) spypeek:(uint16_t)addr source:(MemoryType)source
{
    return wrapper->mem->spypeek(addr, source);
}
- (uint8_t) spypeek:(uint16_t)addr
{
    return wrapper->mem->spypeek(addr);
}
- (uint8_t) spypeekIO:(uint16_t)addr
{
    return wrapper->mem->spypeekIO(addr);
}
- (void) poke:(uint16_t)addr value:(uint8_t)value target:(MemoryType)target
{
    wrapper->mem->suspend();
    wrapper->mem->poke(addr, value, target);
    wrapper->mem->resume();
}
- (void) poke:(uint16_t)addr value:(uint8_t)value
{
    wrapper->mem->suspend();
    wrapper->mem->poke(addr, value);
    wrapper->mem->resume();
}
- (void) pokeIO:(uint16_t)addr value:(uint8_t)value
{
    wrapper->mem->suspend();
    wrapper->mem->pokeIO(addr, value);
    wrapper->mem->resume();
}

@end


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
// VIC proxy
//

// Constructing

@implementation VICProxy

- (instancetype) initWithVIC:(VIC *)vic
{
    if (self = [super init]) {
        wrapper = new VicWrapper();
        wrapper->vic = vic;
    }
    return self;
}

- (NSInteger) videoPalette
{
    return (NSInteger)wrapper->vic->videoPalette();
}
- (void) setVideoPalette:(NSInteger)value
{
    wrapper->vic->setVideoPalette((VICPalette)value);
}
- (double)brightness
{
    return wrapper->vic->getBrightness();
}
- (void)setBrightness:(double)value
{
    wrapper->vic->setBrightness(value);
}
- (double)contrast
{
    return wrapper->vic->getContrast();
}
- (void)setContrast:(double)value
{
    wrapper->vic->setContrast(value);
}
- (double)saturation
{
    return wrapper->vic->getSaturation();
}
- (void)setSaturation:(double)value
{
    wrapper->vic->setSaturation(value);
}
@end


//
// SID proxy
//

/*
@implementation SIDProxy

- (instancetype) initWithSID:(SIDBridge *)sid
{
    if (self = [super init]) {
        wrapper = new SidBridgeWrapper();
        wrapper->sid = sid;
    }
    return self;
}

- (void) dump
{
    wrapper->sid->dump();
}
- (SIDInfo) getInfo
{
    return wrapper->sid->getInfo();
}
- (VoiceInfo) getVoiceInfo:(NSInteger)voice
{
    return wrapper->sid->getVoiceInfo((unsigned)voice);
}
- (BOOL) reSID
{
    return wrapper->sid->getReSID();
}
- (void) setReSID:(BOOL)b
{
    wrapper->sid->setReSID(b);
}
- (BOOL) audioFilter
{
    return wrapper->sid->getAudioFilter();
}
- (void) setAudioFilter:(BOOL)b
{
    wrapper->sid->setAudioFilter(b);
}
- (NSInteger) samplingMethod
{
    return (NSInteger)(wrapper->sid->getSamplingMethod());
}
- (void) setSamplingMethod:(NSInteger)value
{
    wrapper->sid->setSamplingMethod((SamplingMethod)value);
}
- (NSInteger) model
{
    return (int)(wrapper->sid->getModel());
}
- (void) setModel:(NSInteger)value
{
    wrapper->sid->setModel((SIDModel)value);
}
- (uint32_t) sampleRate
{
    return wrapper->sid->getSampleRate();
}
- (void) setSampleRate:(uint32_t)rate
{
    wrapper->sid->setSampleRate(rate);
}
- (NSInteger) ringbufferSize
{
    return wrapper->sid->ringbufferSize();
}
- (float) ringbufferData:(NSInteger)offset
{
    return wrapper->sid->ringbufferData(offset);
}
- (void) readMonoSamples:(float *)target size:(NSInteger)n
{
    wrapper->sid->readMonoSamples(target, n);
}
- (void) readStereoSamples:(float *)target1 buffer2:(float *)target2 size:(NSInteger)n
{
    wrapper->sid->readStereoSamples(target1, target2, n);
}
- (void) readStereoSamplesInterleaved:(float *)target size:(NSInteger)n
{
    wrapper->sid->readStereoSamplesInterleaved(target, n);
}
- (void) rampUp
{
    wrapper->sid->rampUp();
}
- (void) rampUpFromZero
{
    wrapper->sid->rampUpFromZero();
}
- (void) rampDown
{
    wrapper->sid->rampDown();
}

@end
 */


//
// C64
//

@implementation C64Proxy

@synthesize wrapper;
@synthesize mem, vic;

- (instancetype) init
{
	NSLog(@"C64Proxy::init");
	
    if (!(self = [super init]))
        return self;
    
    C64 *c64 = new C64();
    wrapper = new C64Wrapper();
    wrapper->c64 = c64;
	
    // Create sub proxys
    mem = [[MemoryProxy alloc] initWithMemory:&c64->mem];
    vic = [[VICProxy alloc] initWithVIC:&c64->vic];

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
*/
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
