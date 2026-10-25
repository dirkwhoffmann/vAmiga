// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#import "vaconfig.h"
#import "EmulatorProxy.h"
#import "VAmiga.h"
#import "Emulator.h"
#import "DiskImage.h"
#import "RomFile.h"
#import "utl/support/Strings.h"
#import "utl/support/Strings.h"

using namespace vamiga;
using namespace vamiga::moira;

//
// Message passing
//

// Declared here because the message queue callback below needs it (implemented further down)
@interface ExceptionWrapper ()

- (void)save:(const std::exception &)exception;

@end

/* Sits between the emulator thread and the GUI. It copies what the message
 * queue only lends out, and hands the message over to the main thread.
 */
@interface MessageBridge : NSObject

@property (nonatomic, weak) id<MessageListener> listener;

@end

@implementation MessageBridge

@synthesize listener = _listener;

@end

// Called by the message queue, on the emulator thread
static void forwardMessage(const void *context, Message msg, std::exception_ptr exception)
{
    // The bridge is owned by the emulator proxy, which outlives the emulator
    MessageBridge *bridge = (__bridge MessageBridge *)context;

    /* The string is only lent out for the duration of this call, so it is
     * copied here. Swift never sees the pointer.
     */
    NSString *attachment = @"";
    if (msg.str) {
        NSString *text = [NSString stringWithUTF8String:msg.str];
        if (text) attachment = text;
    }
    msg.str = nullptr;

    // The exception is handed over as a wrapper, which stays valid for as long as it is needed
    ExceptionWrapper *wrapper = nil;
    if (exception) {
        wrapper = [ExceptionWrapper new];
        try { std::rethrow_exception(exception); }
        catch (const std::exception &e) { [wrapper save:e]; }
        catch (...) { wrapper.fault = 666; wrapper.what = @"Unknown error"; }
    }

    // A main queue is first-in, first-out, so messages keep their order
    dispatch_async(dispatch_get_main_queue(), ^{
        [bridge.listener processMessage:msg attachment:attachment exception:wrapper];
    });
}

@interface EmulatorProxy ()
{
    MessageBridge *messageBridge;
}
@end

const VAmigaConstants VAMIGA = {

    .PAL = {
        .FREQUENCY = {
            .CLK = PAL::CLK_FREQUENCY,
            .DMA = PAL::DMA_FREQUENCY,
            .CPU = PAL::CPU_FREQUENCY
        },
        .VPOS = {
            .CNT_LF = PAL::VPOS_CNT_LF,
            .CNT_SF = PAL::VPOS_CNT_SF,
            .CNT    = PAL::VPOS_CNT,
            .MAX_LF = PAL::VPOS_MAX_LF,
            .MAX_SF = PAL::VPOS_MAX_SF,
            .MAX    = PAL::VPOS_MAX
        },
        .HPOS = {
            .CNT_LL = PAL::HPOS_CNT,
            .CNT_SL = PAL::HPOS_CNT,
            .CNT    = PAL::HPOS_CNT,
            .MAX_LL = PAL::HPOS_MAX,
            .MAX_SL = PAL::HPOS_MAX,
            .MAX    = PAL::HPOS_MAX
        },
        .VBLANK = {
            .MIN = PAL::VBLANK_MIN,
            .MAX = PAL::VBLANK_MAX,
            .CNT = PAL::VBLANK_CNT
        }
    },

    .NTSC = {
        .FREQUENCY = {
            .CLK = NTSC::CLK_FREQUENCY,
            .DMA = NTSC::DMA_FREQUENCY,
            .CPU = NTSC::CPU_FREQUENCY
        },
        .VPOS = {
            .CNT_LF = NTSC::VPOS_CNT_LF,
            .CNT_SF = NTSC::VPOS_CNT_SF,
            .CNT    = NTSC::VPOS_CNT,
            .MAX_LF = NTSC::VPOS_MAX_LF,
            .MAX_SF = NTSC::VPOS_MAX_SF,
            .MAX    = NTSC::VPOS_MAX
        },
        .HPOS = {
            .CNT_LL = NTSC::HPOS_CNT_LL,
            .CNT_SL = NTSC::HPOS_CNT_SL,
            .CNT    = NTSC::HPOS_CNT,
            .MAX_LL = NTSC::HPOS_MAX_LL,
            .MAX_SL = NTSC::HPOS_MAX_SL,
            .MAX    = NTSC::HPOS_MAX
        },
        .VBLANK = {
            .MIN = NTSC::VBLANK_MIN,
            .MAX = NTSC::VBLANK_MAX,
            .CNT = NTSC::VBLANK_CNT
        }
    },

    .VPOS = {
        .CNT = VPOS_CNT,
        .MAX = VPOS_MAX
    },

    .HPOS = {
        .CNT = HPOS_CNT,
        .MAX = HPOS_MAX
    },

    .HBLANK = {
        .MIN = HBLANK_MIN,
        .MAX = HBLANK_MAX,
        .CNT = HBLANK_CNT
    },

    .VPIXELS = VPIXELS,
    .HPIXELS = HPIXELS,
    .PIXELS  = PIXELS,

    .HDR = {
        .C_MIN = HDR_C_MIN,
        .C_MAX = HDR_C_MAX,
        .H_MIN = HDR_H_MIN,
        .H_MAX = HDR_H_MAX,
        .S_MIN = HDR_S_MIN,
        .S_MAX = HDR_S_MAX
    }
};

NSString *EventSlotName(EventSlot slot)
{
    return @(EventSlotEnum::help(slot));
}

ImageInfo scan(const fs::path &url)
{
    if (auto info = DiskImage::about(url)) {
        return *info;
    } else {
        return { ImageType::UNKNOWN, ImageFormat::UNKNOWN };
    }
}

@implementation ExceptionWrapper

@synthesize fault;
@synthesize what;

- (instancetype)init {

    if (self = [super init]) {
        
        fault = 0; // Fault::OK;
        what = @"";
    }
    return self;
}

/*
- (void)save:(const Error &)exception
{
    fault = exception.fault();
    what = @(exception.what());
}
*/

- (void)save:(const std::exception &)exception
{
    if (auto *error = dynamic_cast<const Error *>(&exception)) {

        fault = error->fault();
        what = @(error->what());

    } else {

        // Uncatched C++ error (this is a bug)
        fault = 666;
        what = @(exception.what());
    }
}

@end

//
// Base Proxy
//

@implementation Proxy

- (instancetype)initWith:(void *)ref
{
    if (ref == nil) {
        return nil;
    }
    if (self = [super init]) {
        obj = ref;
    }
    return self;
}

@end

//
// CoreComponent proxy
//

@implementation CoreComponentProxy

@end

//
// Properties
//

@implementation DefaultsProxy

- (DefaultsAPI *)props
{
    return (DefaultsAPI *)obj;
}

- (void)load:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self props]->load([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)save:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self props]->save([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)register:(NSString *)key value:(NSString *)value
{
    [self props]->setFallback(string([key UTF8String]), string([value UTF8String]));
}

- (NSString *)getString:(NSString *)key
{
    auto result = [self props]->getRaw([key UTF8String]);
    return @(result.c_str());
}

- (NSInteger)getInt:(NSString *)key
{
    return [self props]->get([key UTF8String]);
}

- (NSInteger)getOpt:(Opt)option
{
    return [self props]->get(option);
}

- (NSInteger)getOpt:(Opt)option nr:(NSInteger)nr
{
    return [self props]->get(option, nr);
}

- (void)setKey:(NSString *)key value:(NSString *)value
{
    [self props]->set(string([key UTF8String]), string([value UTF8String]));
}

- (void)setOpt:(Opt)option value:(NSInteger)value
{
    [self props]->set(option, value);
}

- (void)setOpt:(Opt)option nr:(NSInteger)nr value:(NSInteger)value
{
    [self props]->set(option, value, { nr });
}

- (void)removeAll
{
    [self props]->remove();
}

- (void)removeKey:(NSString *)key
{
    [self props]->remove(string([key UTF8String]));
}

- (void)remove:(Opt)option
{
    [self props]->remove(option);
}

- (void)remove:(Opt) option nr:(NSInteger)nr
{
    [self props]->remove(option, { nr });
}

@end


//
// Guards (Breakpoints, Watchpoints)
//

@implementation GuardsProxy

- (GuardsAPI *)guards
{
    return (GuardsAPI *)obj;
}

- (NSInteger)count
{
    return [self guards]->elements();
}

- (NSInteger)addr:(NSInteger)nr
{
    auto guard = [self guards]->guardNr(nr);
    return guard ? (*guard).addr : 0;
}

- (BOOL)isSet:(NSInteger)nr
{
    auto guard = [self guards]->guardNr(nr);
    return guard.has_value();
}

- (BOOL)isSetAt:(NSInteger)addr
{
    auto guard = [self guards]->guardAt(u32(addr));
    return guard.has_value();
}

- (void)setAt:(NSInteger)addr
{
    [self guards]->setAt((u32)addr);
}

- (void)remove:(NSInteger)nr
{
    [self guards]->remove(nr);
}

- (void)removeAt:(NSInteger)addr
{
    [self guards]->removeAt((u32)addr);
}

- (void)removeAll
{
    return [self guards]->removeAll();
}

- (void)replace:(NSInteger)nr addr:(NSInteger)addr
{
    [self guards]->moveTo(nr, (u32)addr);
}

- (BOOL)isEnabled:(NSInteger)nr
{
    auto guard = [self guards]->guardNr(nr);
    return guard ? (*guard).enabled : false;
}

- (BOOL)isEnabledAt:(NSInteger)addr
{
    auto guard = [self guards]->guardAt(u32(addr));
    return guard ? (*guard).enabled : false;
}

- (BOOL)isDisabled:(NSInteger)nr
{
    auto guard = [self guards]->guardNr(nr);
    return guard ? (*guard).enabled == false : false;
}

- (BOOL)isDisabledAt:(NSInteger)addr
{
    auto guard = [self guards]->guardAt(u32(addr));
    return guard ? (*guard).enabled == false : false;
}

- (void)enable:(NSInteger)nr
{
    [self guards]->enable(nr);
}

- (void)enableAt:(NSInteger)addr
{
    [self guards]->enableAt((u32)addr);
}

- (void)disable:(NSInteger)nr
{
    [self guards]->disable(nr);
}

- (void)disableAt:(NSInteger)addr
{
    [self guards]->disableAt((u32)addr);
}

@end

//
// CPU proxy
//

@implementation CPUProxy

- (CPUAPI *)cpu
{
    return (CPUAPI *)obj;
}

- (CPUInfo)info
{
    return [self cpu]->getInfo();
}

- (CPUInfo)cachedInfo
{
    return [self cpu]->getCachedInfo();
}

- (NSInteger)loggedInstructions
{
    return [self cpu]->debugger.loggedInstructions();
}

- (void)clearLog
{
    return [self cpu]->debugger.clearLog();
}

- (NSString *)disassemble:(NSInteger)addr format:(NSString *)fmt length:(NSInteger *)len
{
    isize length = 0;

    auto str = [self cpu]->debugger.disassemble((u32)addr, [fmt UTF8String], &length);
    if (len) *len = (NSInteger)length;

    return @(str.c_str());
}

- (NSString *)disassembleRecorded:(NSInteger)nr format:(NSString *)fmt length:(NSInteger *)len
{
    isize length = 0;

    auto str = [self cpu]->debugger.disassembleRecorded(nr, [fmt UTF8String], &length);
    if (len) *len = (NSInteger)length;

    return @(str.c_str());
}

- (NSString *)vectorName:(NSInteger)nr
{
    auto name = [self cpu]->debugger.vectorName(u8(nr));
    return @(name.c_str());
}

@end


//
// CIA proxy
//

@implementation CIAProxy

- (CIAAPI *)cia
{
    return (CIAAPI *)obj;
}

- (CIAInfo)info
{
    return [self cia]->getInfo();
}

- (CIAInfo)cachedInfo
{
    return [self cia]->getCachedInfo();
}

- (CIAMetrics)stats
{
    return [self cia]->getMetrics();
}

@end


//
// Memory proxy
//

@implementation MemProxy

- (MemoryAPI *)mem
{
    return (MemoryAPI *)obj;
}

- (MemConfig)config
{
    return [self mem]->getConfig();
}

- (MemInfo)info
{
    return [self mem]->getInfo();
}

- (MemInfo)cachedInfo
{
    return [self mem]->getCachedInfo();
}

- (MemMetrics)stats
{
    return [self mem]->getMetrics();
}

- (RomTraits)romTraits
{
    return [self mem]->getRomTraits();
}

- (RomTraits)womTraits
{
    return [self mem]->getWomTraits();
}

- (RomTraits)extTraits
{
    return [self mem]->getExtTraits();
}

- (void)deleteRom
{
    [self mem]->deleteRom();
}

- (BOOL)isRom:(NSURL *)url
{
    return RomFile::isCompatible([url fileSystemRepresentation]);
}

- (void)loadRomFromBuffer:(NSData *)data exception:(ExceptionWrapper *)ex
{
    assert(data);
    const u8 *bytes = (const u8 *)[data bytes];
    
    try { return [self mem]->loadRom(bytes, [data length]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)loadRomFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->loadRom([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)deleteExt
{
    [self mem]->deleteExt();
}

- (void)loadExtFromBuffer:(NSData *)data exception:(ExceptionWrapper *)ex
{
    assert(data);
    const u8 *bytes = (const u8 *)[data bytes];
    
    try { return [self mem]->loadExt(bytes, [data length]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)loadExtFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->loadExt([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)saveRom:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->saveRom([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)saveWom:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->saveWom([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)saveExt:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->saveExt([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (MemSrc)memSrc:(Accessor)accessor addr:(NSInteger)addr
{
    assert(accessor == Accessor::CPU || accessor == Accessor::AGNUS);
    
    if (accessor == Accessor::CPU) {
        return [self mem]->debugger.getMemSrc(Accessor::CPU, (u32)addr);
    } else {
        return [self mem]->debugger.getMemSrc(Accessor::AGNUS, (u32)addr);
    }
}

- (NSInteger)spypeek16:(Accessor)accessor addr:(NSInteger)addr
{
    assert(accessor == Accessor::CPU || accessor == Accessor::AGNUS);
    
    if (accessor == Accessor::CPU) {
        return [self mem]->debugger.spypeek16(Accessor::CPU, (u32)addr);
    } else {
        return [self mem]->debugger.spypeek16(Accessor::AGNUS, (u32)addr);
    }
}

- (NSString *)regName:(NSInteger)addr
{
    auto cstr = MemoryDebugger::regName((u32)addr);
    return @(cstr);
}

- (NSString *)ascDump:(Accessor)accessor addr:(NSInteger)addr bytes:(NSInteger)bytes
{
    assert(accessor == Accessor::CPU || accessor == Accessor::AGNUS);
    
    auto str = [self mem]->debugger.ascDump(accessor, (u32)addr, bytes);
    return @(str.c_str());
}

- (NSString *)hexDump:(Accessor)accessor addr: (NSInteger)addr bytes:(NSInteger)bytes
{
    assert(accessor == Accessor::CPU || accessor == Accessor::AGNUS);
    
    auto str = [self mem]->debugger.hexDump(accessor, (u32)addr, bytes);
    return @(str.c_str());
}

@end


//
// AudioPort
//

@implementation AudioPortProxy

- (AudioPortAPI *)port
{
    return (AudioPortAPI *)obj;
}

- (AudioPortMetrics)stats
{
    return [self port]->getMetrics();
}

- (NSInteger)copyMono:(float *)target size:(NSInteger)n
{
    return [self port]->copyMono(target, n);
}

- (NSInteger)copyStereo:(float *)target1 buffer2:(float *)target2 size:(NSInteger)n
{
    return [self port]->copyStereo(target1, target2, n);
}

- (NSInteger)copyInterleaved:(float *)target size:(NSInteger)n
{
    return [self port]->copyInterleaved(target, n);
}

- (void)drawWaveformL:(u32 *)buffer w:(NSInteger)w h:(NSInteger)h color:(u32)c
{
    [self port]->drawL(buffer, w, h, c);
}

- (void)drawWaveformL:(u32 *)buffer size:(NSSize)size color:(u32)c
{
    [self drawWaveformL:buffer
                      w:(NSInteger)size.width
                      h:(NSInteger)size.height
                  color:c];
}

- (void)drawWaveformR:(u32 *)buffer w:(NSInteger)w h:(NSInteger)h color:(u32)c
{
    [self port]->drawR(buffer, w, h, c);
}

- (void)drawWaveformR:(u32 *)buffer size:(NSSize)size color:(u32)c
{
    [self drawWaveformR:buffer
                      w:(NSInteger)size.width
                      h:(NSInteger)size.height
                  color:c];
}

@end


//
// Agnus proxy
//

@implementation AgnusProxy

- (AgnusAPI *)agnus
{
    return (AgnusAPI *)obj;
}

- (AgnusInfo)info
{
    return [self agnus]->getInfo();
}

- (AgnusInfo)cachedInfo
{
    return [self agnus]->getCachedInfo();
}

- (AgnusMetrics)stats
{
    return [self agnus]->getMetrics();
}

- (AgnusTraits)traits
{
    return [self agnus]->getTraits();
}

- (EventSlotInfo)cachedSlotInfo:(NSInteger)slot
{
    return [self agnus]->getCachedInfo().slotInfo[slot];
}

@end


//
// Copper proxy
//

@implementation CopperProxy

- (CopperAPI *)copper
{
    return (CopperAPI *)obj;
}

- (CopperInfo)info
{
    return [self copper]->getInfo();
}

- (CopperInfo)cachedInfo
{
    return [self copper]->getCachedInfo();
}

- (BOOL)isIllegalInstr:(NSInteger)addr
{
    return [self copper]->isIllegalInstr((u32)addr);
}

- (NSString *)disassemble:(NSInteger)addr symbolic:(BOOL)sym
{
    string str = [self copper]->disassemble((u32)addr, sym);
    return @(str.c_str());
}

- (NSString *)disassemble:(NSInteger)list instr:(NSInteger)offset symbolic:(BOOL)sym
{
    string str = [self copper]->disassemble(list, offset, sym);
    return @(str.c_str());
}

@end


//
// Blitter
//

@implementation BlitterProxy

- (BlitterAPI *)blitter
{
    return (BlitterAPI *)obj;
}

- (BlitterInfo)info
{
    return [self blitter]->getInfo();
}

- (BlitterInfo)cachedInfo
{
    return [self blitter]->getCachedInfo();
}

@end


//
// DMA Debugger
//

@implementation XRayProxy

- (XRayAPI *)debugger
{
    return (XRayAPI *)obj;
}

- (XRayInfo)info
{
    return [self debugger]->getInfo();
}

@end


//
// Logic Analyzer
//

@implementation LogicAnalyzerProxy

- (LogicAnalyzerAPI *)la
{
    return (LogicAnalyzerAPI *)obj;
}

- (LogicAnalyzerInfo)info
{
    return [self la]->getInfo();
}

- (NSInteger)traceCount
{
    return [self la]->getTraceCount();
}

- (LogicAnalyzerSample)traceSample:(NSInteger)nr
{
    return [self la]->getTraceSample(nr);
}

@end


//
// Denise proxy
//

@implementation DeniseProxy

- (DeniseAPI *)denise
{
    return (DeniseAPI *)obj;
}

- (DeniseInfo)info
{
    return [self denise]->getInfo();
}

- (DeniseInfo)cachedInfo
{
    return [self denise]->getCachedInfo();
}

- (SpriteInfo)getSpriteInfo:(NSInteger)nr
{
    return [self info].sprite[nr];
}

- (SpriteInfo)getCachedSpriteInfo:(NSInteger)nr
{
    return [self cachedInfo].sprite[nr];
}

@end


//
// Paula proxy
//

@implementation PaulaProxy

- (PaulaAPI *)paula
{
    return (PaulaAPI *)obj;
}

- (PaulaInfo)info
{
    return [self paula]->getInfo();
}

- (PaulaInfo)cachedInfo
{
    return [self paula]->getCachedInfo();
}

- (StateMachineInfo)audioInfo0
{
    return [self paula]->audioChannel0.getInfo();
}

- (StateMachineInfo)audioInfo1
{
    return [self paula]->audioChannel1.getInfo();
}

- (StateMachineInfo)audioInfo2
{
    return [self paula]->audioChannel2.getInfo();
}

- (StateMachineInfo)audioInfo3
{
    return [self paula]->audioChannel3.getInfo();
}

- (UARTInfo)uartInfo
{
    return [self paula]->uart.getInfo();
}

- (UARTInfo)cachedUartInfo
{
    return [self paula]->uart.getCachedInfo();
}

@end


//
// Rtc proxy
//

@implementation RtcProxy

- (RTCAPI *)rtc
{
    return (RTCAPI *)obj;
}

- (void)update
{
    [self rtc]->update();
}

@end


//
// Mouse proxy
//

@implementation MouseProxy

- (MouseAPI *)mouse
{
    return (MouseAPI *)obj;
}

- (void)setXY:(NSPoint)pos
{
    [self mouse]->setXY(pos.x, pos.y);
}

- (void)setDxDy:(NSPoint)pos
{
    [self mouse]->setDxDy((double)pos.x, (double)pos.y);
}

- (void)trigger:(GamePadAction)event
{
    [self mouse]->trigger(event);
}

- (BOOL)detectShakeAbs:(NSPoint)pos
{
    return [self mouse]->detectShakeXY(pos.x, pos.y);
}

- (BOOL)detectShakeRel:(NSPoint)pos
{
    return [self mouse]->detectShakeDxDy(pos.x, pos.y);
}

@end


//
// Joystick
//

@implementation JoystickProxy

- (JoystickAPI *)joystick
{
    return (JoystickAPI *)obj;
}

- (JoystickInfo)info
{
    return [self joystick]->getInfo();
}

- (JoystickInfo)cachedInfo
{
    return [self joystick]->getCachedInfo();
}

- (void)trigger:(GamePadAction)event
{
    [self joystick]->trigger(event);
}

@end


//
// ControlPort proxy
//

@implementation ControlPortProxy

@synthesize mouse;
@synthesize joystick;

- (instancetype)initWith:(void *)ref
{
    if (self = [super init]) {
        
        ControlPortAPI *port = (ControlPortAPI *)ref;
        obj = ref;
        joystick = [[JoystickProxy alloc] initWith:&port->joystick];
        mouse = [[MouseProxy alloc] initWith:&port->mouse];
    }
    return self;
}

- (ControlPortAPI *)cp
{
    return (ControlPortAPI *)obj;
}

- (ControlPortInfo)info
{
    return [self cp]->getInfo();
}

- (ControlPortInfo)cachedInfo
{
    return [self cp]->getCachedInfo();
}

@end


//
// VideoPort proxy
//

@implementation VideoPortProxy

- (VideoPortAPI *)port
{
    return (VideoPortAPI *)obj;
}

- (void)lockTexture
{
    [self port]->lockTexture();
}

- (void)unlockTexture
{
    [self port]->unlockTexture();
}

- (void)texture:(const u32 **)ptr nr:(NSInteger *)nr lof:(bool *)lof prevlof:(bool *)prevlof
{
    isize inr;

    *ptr = [self port]->getTexture(&inr, lof, prevlof);
    *nr = inr;
}

- (const u32 *)xrayTexture
{
    return [self port]->getXrayTexture();
}

- (void)innerArea:(NSInteger *)x1 x2:(NSInteger *)x2 y1:(NSInteger *)y1 y2:(NSInteger *)y2
{
    isize xx1, xx2, yy1, yy2;
    [self port]->findInnerArea(xx1, xx2, yy1, yy2);
        
    *x1 = (NSInteger)xx1;
    *x2 = (NSInteger)xx2;
    *y1 = (NSInteger)yy1;
    *y2 = (NSInteger)yy2;
}

- (void)innerAreaNormalized:(double *)x1 x2:(double *)x2 y1:(double *)y1 y2:(double *)y2
{
    double xx1, xx2, yy1, yy2;
    [self port]->findInnerAreaNormalized(xx1, xx2, yy1, yy2);

    *x1 = (double)xx1;
    *x2 = (double)xx2;
    *y1 = (double)yy1;
    *y2 = (double)yy2;
}

@end


//
// SerialPort proxy
//

@implementation SerialPortProxy

- (SerialPortAPI *)serial
{
    return (SerialPortAPI *)obj;
}

- (SerialPortConfig)config
{
    return [self serial]->getConfig();
}

- (SerialPortInfo)info
{
    return [self serial]->getInfo();
}

- (SerialPortInfo)cachedInfo
{
    return [self serial]->getCachedInfo();
}

- (NSInteger)readIncomingPrintableByte
{
    return [self serial]->readIncomingPrintableByte();
}

- (NSInteger)readOutgoingPrintableByte
{
    return [self serial]->readOutgoingPrintableByte();
}

@end

//
// MidiManager proxy
//

@implementation MidiManagerProxy

- (MidiManager *)midi
{
    return (MidiManager *)obj;
}

+ (NSInteger)outputCount
{
    return (NSInteger)MidiManager::getOutputCount();
}

+ (NSInteger)inputCount
{
    return (NSInteger)MidiManager::getInputCount();
}

+ (NSString *)outputDeviceName:(NSInteger)index
{
    auto name = MidiManager::getOutputName((ItemCount)index);
    return [NSString stringWithUTF8String:name.c_str()];
}

+ (NSString *)inputDeviceName:(NSInteger)index
{
    auto name = MidiManager::getInputName((ItemCount)index);
    return [NSString stringWithUTF8String:name.c_str()];
}

- (NSInteger)selectedOutputDevice
{
    return [self midi]->getOption(Opt::MIDI_DEVICE_OUT);
}

- (NSInteger)selectedInputDevice
{
    return [self midi]->getOption(Opt::MIDI_DEVICE_IN);
}

- (void)setOutputDevice:(NSInteger)index
{
    [self midi]->setOption(Opt::MIDI_DEVICE_OUT, index);
}

- (void)setInputDevice:(NSInteger)index
{
    [self midi]->setOption(Opt::MIDI_DEVICE_IN, index);
}

@end

//
// Keyboard
//

@implementation KeyboardProxy

- (KeyboardAPI *)kb
{
    return (KeyboardAPI *)obj;
}

- (BOOL)isPressed:(NSInteger)keycode
{
    return [self kb]->isPressed((KeyCode)keycode);
}

- (void)press:(NSInteger)keycode
{
    [self press:keycode delay: 0.0];
}

- (void)press:(NSInteger)keycode delay:(double)delay
{
    [self kb]->press((KeyCode)keycode, delay);
}

- (void)press:(NSInteger)keycode delay:(double)delay duration:(double)duration
{
    [self kb]->press((KeyCode)keycode, delay, duration);
}

- (void)release:(NSInteger)keycode
{
    [self release:keycode delay: 0.0];
}

- (void)release:(NSInteger)keycode delay:(double)delay
{
    [self kb]->release((KeyCode)keycode, delay);
}

- (void)toggle:(NSInteger)keycode
{
    [self isPressed: keycode] ? [self release: keycode] : [self press: keycode];
}

- (void)releaseAll
{
    [self kb]->releaseAll();
}

- (BOOL)isLocked:(NSInteger)keycode
{
    return [self kb]->isLocked((KeyCode)keycode);
}

- (void)lock:(NSInteger)keycode
{
    [self lock:keycode delay: 0.0];
}

- (void)lock:(NSInteger)keycode delay:(double)delay
{
    [self kb]->lock((KeyCode)keycode, delay);
}

- (void)unlock:(NSInteger)keycode
{
    [self unlock:keycode delay: 0.0];
}

- (void)unlock:(NSInteger)keycode delay:(double)delay
{
    [self kb]->unlock((KeyCode)keycode, delay);
}

@end


//
// Disk Controller
//

@implementation DiskControllerProxy

- (DiskControllerAPI *)dc
{
    return (DiskControllerAPI *)obj;
}

- (DiskControllerConfig)config
{
    return [self dc]->getConfig();
}

- (DiskControllerInfo)info
{
    return [self dc]->getInfo();
}

- (DiskControllerInfo)cachedInfo
{
    return [self dc]->getCachedInfo();
}

@end


//
// FloppyDrive proxy
//

@implementation FloppyDriveProxy

- (FloppyDriveAPI *)drive
{
    return (FloppyDriveAPI *)obj;
}

- (FloppyDriveConfig)config
{
    return [self drive]->getConfig();
}

- (BOOL)getFlag:(DiskFlags)mask
{
    return [self drive]->getFlag(mask);
}

- (void)setFlag:(DiskFlags)mask value:(BOOL)value
{
    [self drive]->setFlag(mask, value);
}

- (FloppyDriveInfo)info
{
    return [self drive]->getInfo();
}

- (FloppyDriveInfo)cachedInfo
{
    return [self drive]->getCachedInfo();
}

- (BOOL)isInsertable:(Diameter)type density:(Density)density
{
    return [self drive]->isInsertable(type, density);
}

- (void)insertBlankDisk:(FSFormat)fs bootBlock:(BootBlockId)bb name:(NSString *)name url:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try {
        if (url) {
            return [self drive]->insertBlankDisk(fs, bb, [name UTF8String], [url fileSystemRepresentation]);
        } else {
            return [self drive]->insertBlankDisk(fs, bb, [name UTF8String]);
        }
    }
    catch(Error &error) { [ex save:error]; }
}

- (void)insertFile:(NSURL *)url protected:(BOOL)wp exception:(ExceptionWrapper *)ex
{
    try { [self drive]->insert([url fileSystemRepresentation], wp); }
    catch(std::exception &error) { [ex save:error]; }
}

- (void)eject
{
    [self drive]->ejectDisk();
}

- (void)writeToFile:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { [self drive]->writeToFile([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (NSURL *)url
{
    auto path = [self drive]->path();
    return path.empty() ? nil : [NSURL fileURLWithPath:@(path.string().c_str())];
}

- (BOOL)needsPersisting
{
    return [self drive]->needsPersisting();
}

- (void)persist
{
    [self drive]->persist();
}

- (NSString *)readTrackBits:(NSInteger)track
{
    return @([self drive]->readTrackBits(track).c_str());
}

@end


//
// HdController proxy
//

@implementation HdControllerProxy

- (HdControllerAPI *)controller
{
    return (HdControllerAPI *)obj;
}

- (HdcInfo)info
{
    return [self controller]->getInfo();
}

- (HdcMetrics)stats
{
    return [self controller]->getMetrics();
}

@end

//
// FSDevice proxy
//

@implementation FileSystemProxy

- (FileSystem *)fs
{
    return (FileSystem *)obj;
}

+ (instancetype)make:(FileSystem *)volume
{
    if (volume == nullptr) { return nil; }
    
    FileSystemProxy *proxy = [[self alloc] initWith: volume];
    return proxy;
}

+ (instancetype)makeWithImage:(FloppyDiskImageProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try {

        auto *base = (FloppyDiskImage *)proxy->obj;

        if (auto* adf = dynamic_cast<ADFFile *>(base)) {

            auto *vol = new Volume(*adf); // MEMORY LEAK!
            auto *fs = new FileSystem(*vol);
            return [self make:fs];
        }

        throw IOError(IOError::FILE_TYPE_UNSUPPORTED);

    } catch(Error &error) {

        [ex save:error];
        return nil;
    }
}


+ (instancetype)makeWithImage:(HardDiskImageProxy *)proxy partition:(NSInteger)nr exception:(ExceptionWrapper *)ex
{
    try {

        auto *base = (HardDiskImage *)proxy->obj;

        if (auto* hdf = dynamic_cast<HDFFile *>(base)) {

            auto *vol = new Volume(*hdf, hdf->partition(nr)); // MEMORY LEAK!
            auto *fs = new FileSystem(*vol);
            return [self make:fs];
        }

        throw IOError(IOError::FILE_TYPE_UNSUPPORTED);

    } catch(Error &error) {

        [ex save:error];
        return nil;
    }
}

- (NSString *)name
{
    auto str = [self fs]->stat().name;
    return @(str.c_str());
}

- (NSString *)creationDate
{
    auto time = [self fs]->stat().bDate.time();
    if (time == 0) return @"-";

    return [NSDateFormatter localizedStringFromDate:
            [NSDate dateWithTimeIntervalSince1970:NSTimeInterval(time)]
                                          dateStyle:NSDateFormatterMediumStyle
                                          timeStyle:NSDateFormatterMediumStyle];
}

- (NSString *)modificationDate
{
    auto time = [self fs]->stat().mDate.time();
    if (time == 0) return @"-";

    return [NSDateFormatter localizedStringFromDate:
            [NSDate dateWithTimeIntervalSince1970:NSTimeInterval(time)]
                                          dateStyle:NSDateFormatterMediumStyle
                                          timeStyle:NSDateFormatterMediumStyle];
}

- (NSString *)bootBlockName
{
    auto str = [self fs]->bootStat().name;
    return @(str.c_str());
}

- (NSString *)capacityString
{
    auto str = utl::byteCountAsString([self fs]->bytes());
    return @(str.c_str());
}

- (NSString *)fillLevelString
{
    auto st = [self fs]->stat();
    auto str = utl::fillLevelAsString(100.0 * double(st.usedBlocks) / double(st.traits.blocks));
    return @(str.c_str());
}

- (FSFormat)dos
{
    return [self fs]->getTraits().dos;
}

- (BOOL)isOFS
{
    return [self fs]->getTraits().ofs();
}

- (BOOL)isFFS
{
    return [self fs]->getTraits().ffs();
}

- (NSInteger)blockSize
{
    return [self fs]->bsize();
}

- (NSInteger)numBlocks
{
    return [self fs]->blocks();
}

- (NSInteger)numBytes
{
    return [self fs]->bytes();
}

- (NSInteger)usedBlocks
{
    return [self fs]->stat().usedBlocks;
}

- (double)fillLevel
{
    auto st = [self fs]->stat();
    return 100.0 * double(st.usedBlocks) / double(st.traits.blocks);
}

- (BOOL)hasVirus
{
    return [self fs]->bootStat().hasVirus;
}

- (void)killVirus
{
    [self fs]->killVirus();
}

- (FSBlockType)blockType:(NSInteger)blockNr
{
    return [self fs]->typeOf((u32)blockNr);
}

- (FSItemType)itemType:(NSInteger)blockNr pos:(NSInteger)pos
{
    return [self fs]->typeOf((u32)blockNr, pos);
}

- (NSInteger)xrayBlocks:(BOOL)strict
{
    return [self fs]->doctor.xray(strict);
}

- (NSInteger)xrayBitmap:(BOOL)strict
{
    return [self fs]->doctor.xrayBitmap(strict);
}

- (NSArray<NSNumber *> *)xrayBlocks
{
    auto &errors = [self fs]->doctor.diagnosis.blockErrors;

    NSMutableArray<NSNumber *> *array = [NSMutableArray arrayWithCapacity:errors.size()];
    for (BlockNr value : errors) { [array addObject:@(value)]; }

    return [array copy];
}

- (NSArray<NSNumber *> *)xrayBitmap
{
    auto &unusedButAllocated = [self fs]->doctor.diagnosis.unusedButAllocated;
    auto &usedButUnallocated = [self fs]->doctor.diagnosis.usedButUnallocated;
    auto size = unusedButAllocated.size() + usedButUnallocated.size();

    NSMutableArray<NSNumber *> *array = [NSMutableArray arrayWithCapacity:size];
    for (BlockNr value : unusedButAllocated) { [array addObject:@(value)]; }
    for (BlockNr value : usedButUnallocated) { [array addObject:@(value)]; }

    return [array sortedArrayUsingSelector:@selector(compare:)];
}

- (FSBlockError)check:(NSInteger)nr
                  pos:(NSInteger)pos
             expected:(unsigned char *)exp
               strict:(BOOL)strict
{
    std::optional<u8> expected;
    auto result = [self fs]->doctor.xray8((u32)nr, pos, strict, expected);
    if (expected) *exp = *expected;

    return result;
}

- (NSInteger)readByte:(NSInteger)block offset:(NSInteger)offset
{
    if (auto *ptr = [self fs]->tryFetch(BlockNr(block)); ptr && offset < 512) {
        return ptr->data()[offset];
    }
    return 0;
}

- (NSString *)ascii:(NSInteger)block offset:(NSInteger)offset length:(NSInteger)len
{
    return @([self fs]->doctor.ascii(BlockNr(block), offset, len).c_str());
}

- (void)export:(NSString *)path recursive:(BOOL)rec contents:(BOOL)con exception:(ExceptionWrapper *)ex
{
    try { return [self fs]->exporter.exportFiles([path fileSystemRepresentation], rec, con); }
    catch(std::exception &error) { [ex save:error]; }
}

- (void)createUsageMap:(u8 *)buf length:(NSInteger)len
{
    [self fs]->doctor.createUsageMap((u8 *)buf, len);
}

- (void)createAllocationMap:(u8 *)buf length:(NSInteger)len
{
    [self fs]->doctor.createAllocationMap((u8 *)buf, len);
}

- (void)createHealthMap:(u8 *)buf length:(NSInteger)len
{
    [self fs]->doctor.createHealthMap((u8 *)buf, len);
}

- (NSInteger)nextBlockOfType:(FSBlockType)type after:(NSInteger)after
{
    return [self fs]->doctor.nextBlockOfType(type, BlockNr(after));
}

- (void)rectifyAllocationMap
{
    [self fs]->doctor.rectifyBitmap();
}

@end


//
// RemoteManager proxy
//

@implementation RemoteManagerProxy

- (RemoteManagerAPI *)manager
{
    return (RemoteManagerAPI *)obj;
}

+ (instancetype)make:(RemoteManagerAPI *)manager
{
    if (manager == nullptr) { return nil; }

    RemoteManagerProxy *proxy = [[self alloc] initWith: manager];
    return proxy;
}

- (RemoteManagerInfo)info
{
    return [self manager]->getInfo();
}

@end


//
// RetroShell proxy
//

@implementation RetroShellProxy

- (RetroShellAPI *)shell
{
    return (RetroShellAPI *)obj;
}

+ (instancetype)make:(RetroShellAPI *)shell
{
    if (shell == nullptr) { return nil; }
    
    RetroShellProxy *proxy = [[self alloc] initWith: shell];
    return proxy;
}

-(RetroShellInfo)info
{
    return [self shell]->getInfo();
}

-(NSString *)getText
{
    return @([self shell]->text().c_str());
}

- (void)pressKey:(char)c
{
    [self shell]->press(c);
}

- (void)pressSpecialKey:(RSKey)key
{
    [self shell]->press(key);
}

- (void)pressSpecialKey:(RSKey)key shift:(BOOL)shift
{
    [self shell]->press(key, shift);
}

- (void)executeScript:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { [self shell]->execScriptAsync(fs::path(url.fileSystemRepresentation)); }
    catch (std::exception &stdex) { [ex save:stdex]; }
}

- (void)executeString:(NSString *)str
{
    [self shell]->execScriptAsync(std::string([str UTF8String]));
}

@end


//
// HardDrive proxy
//

@implementation HardDriveProxy

@synthesize controller;

- (instancetype)initWith:(void *)ref
{
    if (self = [super init]) {

        HardDriveAPI *hd = (HardDriveAPI *)ref;
        obj = ref;
        controller = [[HdControllerProxy alloc] initWith:&hd->controller];
    }
    return self;
}

- (HardDriveAPI *)drive
{
    return (HardDriveAPI *)obj;
}

- (HardDriveConfig)config
{
    return [self drive]->getConfig();
}

- (HardDriveInfo)info
{
    return [self drive]->getInfo();
}

- (HardDriveTraits)traits
{
    return [self drive]->getTraits();
}

- (PartitionTraits) partitionTraits:(NSInteger)nr
{
    return [self drive]->getPartitionTraits(nr);
}

- (BOOL)getFlag:(DiskFlags)mask
{
    return [self drive]->getFlag(mask);
}

- (void)setFlag:(DiskFlags)mask value:(BOOL)value
{
    [self drive]->setFlag(mask, value);
}

- (void)attachFile:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try {
        [self drive]->attach([url fileSystemRepresentation]);
    }  catch(Error &error) {
        [ex save:error];
    }
}

- (void)importFiles:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try {
        [self drive]->importFolder([url fileSystemRepresentation]);
    } catch(Error &error) {
        [ex save:error];
    }
}

- (void)attach:(NSInteger)c h:(NSInteger)h s:(NSInteger)s b:(NSInteger)b
     exception:(ExceptionWrapper *)ex
{
    try {
        [self drive]->attach(c, h, s, b);
    }  catch(Error &error) {
        [ex save:error];
    }
}

- (void)format:(FSFormat)fs name:(NSString *)name exception:(ExceptionWrapper *)ex
{
    auto str = string([name UTF8String]);

    try {
        [self drive]->format(fs, str);
    }  catch(Error &error) {
        [ex save:error];
    }
}

- (void)changeGeometry:(NSInteger)c h:(NSInteger)h s:(NSInteger)s b:(NSInteger)b exception:(ExceptionWrapper *)ex
{
    try {
        [self drive]->changeGeometry(c, h, s, b);
    }  catch(Error &error) {
        [ex save:error];
    }
}

- (NSMutableArray *)geometries
{
    NSMutableArray *data = [[NSMutableArray alloc] init];

    auto geometries = [self drive]->geometries([self traits].blocks);

    for (auto &g : geometries) {

        auto c = std::get<0>(g);
        auto h = std::get<1>(g);
        auto s = std::get<2>(g);

        NSInteger encoded = c << 32 | h << 16 | s;
        [data addObject: [NSNumber numberWithInteger:encoded]];
    }

    return data;
}

- (void)writeToFile:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self drive]->writeToFile([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (NSURL *)url
{
    auto path = [self drive]->path();
    return path.empty() ? nil : [NSURL fileURLWithPath:@(path.string().c_str())];
}

- (void)loadIntoMemory:(ExceptionWrapper *)ex
{
    try { [self drive]->loadIntoMemory(); }
    catch(Error &error) { [ex save:error]; }
}

- (void)saveAs:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { [self drive]->saveAs([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)persist
{
    [self drive]->persist();
}

@end


//
// AnyFile proxy
//

@implementation AnyFileProxy

- (AnyFile *)file
{
    return (AnyFile *)obj;
}

+ (instancetype)make:(AnyFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

- (NSURL *)path
{
    auto nsPath = @([self file]->path.c_str());
    return [NSURL fileURLWithPath:nsPath];
}

- (NSInteger)size
{
    return [self file]->getSize();
}

- (u64)fnv
{
    return [self file]->fnv64();
}

- (void)setPath:(NSURL *)path
{
    [self file]->path = [path fileSystemRepresentation];
}

- (NSInteger)writeToFile:(NSURL *)path exception:(ExceptionWrapper *)ex
{
    try { return [self file]->writeToFile([path fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; return 0; }
}

- (void)dealloc
{
    delete (AnyFile *)obj;
}

@end


//
// Snapshot
//

@implementation SnapshotProxy

+ (instancetype)makeWithAmiga:(EmulatorProxy *)proxy compressor:(Compressor)c
{
    auto amiga = (VAmiga *)proxy->obj;
    auto snap = amiga->amiga.takeSnapshot(c);

    //Transfer ownership to ObjC via release
    return [self make:snap.release()];
}

- (Snapshot *)file
{
    return (Snapshot *)obj;
}

- (NSInteger)size
{
    return [self file]->getSize();
}

- (u64)fnv
{
    return [self file]->fnv64();
}

- (Compressor)compressor
{
    return [self file]->compressor();
}

- (BOOL)compressed
{
    return [self file]->isCompressed();
}

- (u8 *)data
{
    return [self file]->getData()+ sizeof(SnapshotHeader);
}

- (NSImage *)previewImage
{
    // Return cached image (if any)
    if (preview) { return preview; }

    // Get dimensions and data
    auto size = [self file]->previewImageSize();
    auto data = (unsigned char *)[self file]->previewImageData();

    // Create preview image
    if (data) {

        NSBitmapImageRep *rep = [[NSBitmapImageRep alloc]
                                 initWithBitmapDataPlanes: &data
                                 pixelsWide:size.first
                                 pixelsHigh:size.second
                                 bitsPerSample:8
                                 samplesPerPixel:4
                                 hasAlpha:true
                                 isPlanar:false
                                 colorSpaceName:NSCalibratedRGBColorSpace
                                 bytesPerRow:4*size.first
                                 bitsPerPixel:32];

        preview = [[NSImage alloc] initWithSize:[rep size]];
        [preview addRepresentation:rep];

        // image.makeGlossy()
    }
    return preview;
}

- (time_t)timeStamp
{
    return [self file]->timestamp();
}

@end


//
// DiskFileProxy
//

@implementation DiskImageProxy

+ (ImageInfo)about:(NSURL *)url
{
    if (auto about = DiskImage::about([url fileSystemRepresentation])) {
        return *about;
    } else {
        return { ImageType::UNKNOWN, ImageFormat::UNKNOWN };
    }
}

- (DiskImage *)file
{
    return (DiskImage *)obj;
}

- (NSArray<NSString *> *)describe
{
    const auto vec = [self file]->describeImage();

    NSMutableArray<NSString *> *result =
        [NSMutableArray arrayWithCapacity:vec.size()];

    for (const auto &s : vec) {
        [result addObject:[NSString stringWithUTF8String:s.c_str()]];
    }

    return result;
}

- (NSURL *)path
{
    auto nsPath = @([self file]->path.c_str());
    return [NSURL fileURLWithPath:nsPath];
}

- (NSInteger)size
{
    return [self file]->getSize();
}

- (u64)fnv
{
    return [self file]->fnv64();
}

- (NSInteger)writeToFile:(NSURL *)path exception:(ExceptionWrapper *)ex
{
    try { return [self file]->copy([path fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; return 0; }
}

- (ImageType)type
{
    return [self file]->type();
}

- (ImageFormat)format
{
    return [self file]->format();
}

-(ImageInfo)info
{
    return [self file]->info();
}

- (NSInteger)bsize
{
    return [self file]->bsize();
}

- (NSInteger)numCyls
{
    return [self file]->numCyls();
}

- (NSInteger)numHeads
{
    return [self file]->numHeads();
}

- (NSInteger)numTracks
{
    return [self file]->numTracks();
}

- (NSInteger)numSectors
{
    return [self file]->numSectors(0);
}

- (NSInteger)numBlocks
{
    return [self file]->numBlocks();
}

- (NSInteger)numBytes
{
    return [self file]->numBytes();
}

- (NSInteger)readByte:(NSInteger)b offset:(NSInteger)offset
{
    return [self file]->readByte(b * [self bsize] + offset);
}

- (NSString *)asciidump:(NSInteger)b offset:(NSInteger)offset len:(NSInteger)len
{
    string result;
    auto p = [self file]->byteView(b * [self bsize] + offset, len).data();

    for (isize i = 0; i < len; i++) {
        result += isprint(int(p[i])) ? char(p[i]) : '.';
    }

    return @(result.c_str());
}

@end


//
// DiskFileProxy
//

@implementation FloppyDiskImageProxy

+ (ImageInfo)about:(NSURL *)url
{
    if (auto about = FloppyDiskImage::about([url fileSystemRepresentation])) {
        return *about;
    } else {
        return { ImageType::UNKNOWN, ImageFormat::UNKNOWN };
    }
}

- (FloppyDiskImage *)image
{
    return (FloppyDiskImage *)obj;
}

+ (instancetype)make:(void *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithDrive:(FloppyDriveProxy *)proxy
                       format:(ImageFormat)fmt
                    exception:(ExceptionWrapper *)ex
{
    auto drive = (FloppyDriveAPI *)proxy->obj;
    try { return [self make: drive->drive->exportDisk(fmt).release()]; }
    catch(Error &error) { [ex save:error]; return nil; }
}

- (Diameter)diameter
{
    return [self image]->getDiameter();
}

- (Density)density
{
    return [self image]->getDensity();
}

- (BOOL)isSD
{
    return [self image]->isSD();
}

- (BOOL)isDD
{
    return [self image]->isDD();
}

- (BOOL)isHD
{
    return [self image]->isHD();
}

@end


//
// HardDiskImageProxy
//

@implementation HardDiskImageProxy

+ (ImageInfo)about:(NSURL *)url
{
    if (auto about = HardDiskImage::about([url fileSystemRepresentation])) {
        return *about;
    } else {
        return { ImageType::UNKNOWN, ImageFormat::UNKNOWN };
    }
}

- (HardDiskImage *)image
{
    return (HardDiskImage *)obj;
}

+ (instancetype)make:(void *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithDrive:(HardDriveProxy *)proxy
                       format:(ImageFormat)fmt
                    exception:(ExceptionWrapper *)ex
{
    auto drive = (HardDriveAPI *)proxy->obj;
    try { return [self make: drive->drive->exportDisk(fmt).release()]; }
    catch(Error &error) { [ex save:error]; return nil; }
}

- (NSInteger)writeToFile:(NSURL *)path partition:(NSInteger)nr exception:(ExceptionWrapper *)ex
{
    try { return [self image]->writePartitionToFile([path fileSystemRepresentation], nr); }
    catch(Error &error) { [ex save:error]; return 0; }
}

- (NSInteger)numPartitions
{
    return [self image]->numPartitions();
}

- (NSInteger)lowerCyl:(NSInteger)partition
{
    return [self image]->partition(partition).lower;
}

- (NSInteger)upperCyl:(NSInteger)partition
{
    return [self image]->partition(partition).upper;
}

@end


//
// Amiga proxy
//

@implementation AmigaProxy

- (AmigaAPI *)amiga
{
    return (AmigaAPI *)obj;
}

- (AmigaInfo)info
{
    return [self amiga]->getInfo();
}

- (AmigaInfo)cachedInfo
{
    return [self amiga]->getCachedInfo();
}

- (NSInteger)autoInspectionMask
{
    return (NSInteger)[self amiga]->getAutoInspectionMask();
}

- (void)setAutoInspectionMask:(NSInteger)mask
{
    return [self amiga]->setAutoInspectionMask(u64(mask));
}

- (NSString *)stateString
{
    std::stringstream ss;
    [self amiga]->dump(Category::Trace, ss);
    return @(ss.str().c_str());
}

- (SnapshotProxy *)takeSnapshot:(Compressor)compressor
{
    try {

        auto snap = [self amiga]->takeSnapshot(compressor);

        //Transfer ownership to ObjC via release
        return [SnapshotProxy make:snap.release()];

    } catch(Error &error) {

        return nil;
    }
}

- (void)loadSnapshot:(SnapshotProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { [self amiga]->loadSnapshot(*[proxy file]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)loadSnapshotFromUrl:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { [self amiga]->loadSnapshot([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)saveSnapshotToUrl:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { [self amiga]->saveSnapshot([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)loadWorkspace:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { [self amiga]->loadWorkspace([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)saveWorkspace:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { [self amiga]->saveWorkspace([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)saveWorkspaceAsync:(NSURL *)url
{
    [self amiga]->saveWorkspaceAsync([url fileSystemRepresentation]);
}

- (void)saveSnapshotAsync:(NSURL *)url
{
    [self amiga]->saveSnapshotAsync([url fileSystemRepresentation]);
}

- (BOOL)getMessage:(Message *)msg exception:(ExceptionWrapper *)ex
{
    std::exception_ptr exception;

    if (![self amiga]->getMsg(*msg, exception)) return NO;

    if (exception) {
        try { std::rethrow_exception(exception); }
        catch (const std::exception &e) { [ex save:e]; }
        catch (...) { ex.fault = 666; ex.what = @"Unknown error"; }
    }
    return YES;
}

@end


//
// Emulator
//

@implementation EmulatorProxy

+ (NSString *)keyForOption:(NSInteger)opt
{
    return @(OptEnum::key(Opt(opt)));
}

@synthesize audioPort;
@synthesize agnus;
@synthesize amiga;
@synthesize blitter;
@synthesize breakpoints;
@synthesize ciaA;
@synthesize ciaB;
@synthesize controlPort1;
@synthesize controlPort2;
@synthesize copper;
@synthesize copperBreakpoints;
@synthesize cpu;
@synthesize denise;
@synthesize df0;
@synthesize df1;
@synthesize df2;
@synthesize df3;
@synthesize diskController;
@synthesize xray;
@synthesize hd0;
@synthesize hd1;
@synthesize hd2;
@synthesize hd3;
@synthesize keyboard;
@synthesize logicAnalyzer;
@synthesize mem;
@synthesize midiManager;
@synthesize paula;
@synthesize remoteManager;
@synthesize retroShell;
@synthesize rtc;
@synthesize serialPort;
@synthesize videoPort;
@synthesize watchpoints;

- (instancetype) init
{
    if (!(self = [super init]))
        return self;

    // Create the emulator instance
    auto vamiga = new VAmiga();
    obj = vamiga;

    // Create sub proxys
    audioPort = [[AudioPortProxy alloc] initWith:&vamiga->audioPort];
    agnus = [[AgnusProxy alloc] initWith:&vamiga->agnus];
    amiga = [[AmigaProxy alloc] initWith:&vamiga->amiga];
    blitter = [[BlitterProxy alloc] initWith:&vamiga->agnus.blitter];
    breakpoints = [[GuardsProxy alloc] initWith:&vamiga->cpu.breakpoints];
    ciaA = [[CIAProxy alloc] initWith:&vamiga->ciaA];
    ciaB = [[CIAProxy alloc] initWith:&vamiga->ciaB];
    controlPort1 = [[ControlPortProxy alloc] initWith:&vamiga->controlPort1];
    controlPort2 = [[ControlPortProxy alloc] initWith:&vamiga->controlPort2];
    copper = [[CopperProxy alloc] initWith:&vamiga->agnus.copper];
    copperBreakpoints = [[GuardsProxy alloc] initWith:&vamiga->copperBreakpoints];
    cpu = [[CPUProxy alloc] initWith:&vamiga->cpu];
    denise = [[DeniseProxy alloc] initWith:&vamiga->denise];
    diskController = [[DiskControllerProxy alloc] initWith:&vamiga->paula.diskController];
    xray = [[XRayProxy alloc] initWith:&vamiga->agnus.xray];
    df0 = [[FloppyDriveProxy alloc] initWith:&vamiga->df0];
    df1 = [[FloppyDriveProxy alloc] initWith:&vamiga->df1];
    df2 = [[FloppyDriveProxy alloc] initWith:&vamiga->df2];
    df3 = [[FloppyDriveProxy alloc] initWith:&vamiga->df3];
    hd0 = [[HardDriveProxy alloc] initWith:&vamiga->hd0];
    hd1 = [[HardDriveProxy alloc] initWith:&vamiga->hd1];
    hd2 = [[HardDriveProxy alloc] initWith:&vamiga->hd2];
    hd3 = [[HardDriveProxy alloc] initWith:&vamiga->hd3];
    keyboard = [[KeyboardProxy alloc] initWith:&vamiga->keyboard];
    logicAnalyzer = [[LogicAnalyzerProxy alloc] initWith:&vamiga->agnus.logicAnalyzer];
    mem = [[MemProxy alloc] initWith:&vamiga->mem];
    midiManager = [[MidiManagerProxy alloc] initWith:&vamiga->amiga.amiga->midiManager];
    paula = [[PaulaProxy alloc] initWith:&vamiga->paula];
    retroShell = [[RetroShellProxy alloc] initWith:&vamiga->retroShell];
    rtc = [[RtcProxy alloc] initWith:&vamiga->rtc];
    remoteManager = [[RemoteManagerProxy alloc] initWith:&vamiga->remoteManager];
    serialPort = [[SerialPortProxy alloc] initWith:&vamiga->serialPort];
    videoPort = [[VideoPortProxy alloc] initWith:&vamiga->videoPort];
    watchpoints = [[GuardsProxy alloc] initWith:&vamiga->cpu.watchpoints];

    return self;
}

- (VAmiga *)emu
{
    return (VAmiga *)obj;
}

+ (DefaultsProxy *)defaults
{
    return [[DefaultsProxy alloc] initWith:&VAmiga::defaults];
}

+ (NSString *)build
{
    return @(VAmiga::build().c_str());
}

+ (NSString *)version
{
    return @(VAmiga::version().c_str());
}

- (void)dealloc
{
    
}

- (void)kill
{    
    assert([self emu] != NULL);
    delete [self emu];
    obj = NULL;
}

- (EmulatorInfo)info
{
    return [self emu]->getInfo();
}

- (EmulatorInfo)cachedInfo
{
    return [self emu]->getCachedInfo();
}

- (EmulatorMetrics)stats
{
    return [self emu]->getMetrics();
}

- (BOOL)poweredOn
{
    return [self emu]->isPoweredOn();
}

- (BOOL)poweredOff
{
    return [self emu]->isPoweredOff();
}

- (BOOL)paused
{
    return [self emu]->isPaused();
}

- (BOOL)running
{
    return [self emu]->isRunning();
}

- (BOOL)suspended
{
    return [self emu]->isSuspended();
}

- (BOOL)halted
{
    return [self emu]->isHalted();
}

- (BOOL)warping
{
    return [self emu]->isWarping();
}

- (BOOL)tracking
{
    return [self emu]->isTracking();
}

- (void)stepInto
{
    // Stepping is only possible if the emulator is not running. The GUI
    // validates this, so a rejected request is ignored here
    try { [self emu]->stepInto(); } catch (Error &) { }
}

- (void)stepOver
{
    // Stepping is only possible if the emulator is not running. The GUI
    // validates this, so a rejected request is ignored here
    try { [self emu]->stepOver(); } catch (Error &) { }
}

- (void)finishLine
{
    // Stepping is only possible if the emulator is not running. The GUI
    // validates this, so a rejected request is ignored here
    try { [self emu]->finishLine(); } catch (Error &) { }
}

- (void)finishFrame
{
    // Stepping is only possible if the emulator is not running. The GUI
    // validates this, so a rejected request is ignored here
    try { [self emu]->finishFrame(); } catch (Error &) { }
}

- (void)launch:(ExceptionWrapper *)ex
{
    try { [self emu]->launch(); }
    catch(Error &error) { [ex save:error]; }
}

- (void)launch:(id<MessageListener>)listener exception:(ExceptionWrapper *)ex
{
    messageBridge = [MessageBridge new];
    messageBridge.listener = listener;

    try { [self emu]->launch((__bridge const void *)messageBridge, forwardMessage); }
    catch(Error &error) { [ex save:error]; }
}

- (NSInteger)get:(Opt)opt
{
    return [self emu]->get(opt);
}

- (NSInteger)get:(Opt)opt id:(NSInteger)id
{
    return [self emu]->get(opt, id);
}

- (NSInteger)get:(Opt)opt drive:(NSInteger)id
{
    return [self emu]->get(opt, (long)id);
}

- (BOOL)setAsync:(Opt)opt value:(NSInteger)val
{
    try {
        [self emu]->setAsync(opt, val);
        return true;
    } catch(Error &exception) {
        return false;
    }
}

- (BOOL)setAsync:(Opt)opt enable:(BOOL)val
{
    try {
        [self emu]->setAsync(opt, val ? 1 : 0);
        return true;
    } catch(Error &exception) {
        return false;
    }
}

- (BOOL)setAsync:(Opt)opt id:(NSInteger)id value:(NSInteger)val
{
    try {
        [self emu]->setAsync(opt, val, id);
        return true;
    } catch(Error &exception) {
        return false;
    }
}

- (BOOL)setAsync:(Opt)opt id:(NSInteger)id enable:(BOOL)val
{
    try {
        [self emu]->setAsync(opt, val ? 1 : 0, id);
        return true;
    } catch(Error &exception) {
        return false;
    }
}

- (BOOL)setAsync:(Opt)opt drive:(NSInteger)id value:(NSInteger)val
{
    try {
        [self emu]->setAsync(opt, val, (long)id);
        return true;
    } catch(Error &exception) {
        return false;
    }
}

- (BOOL)setAsync:(Opt)opt drive:(NSInteger)id enable:(BOOL)val
{
    try {
        [self emu]->setAsync(opt, val ? 1 : 0, (long)id);
        return true;
    } catch(Error &exception) {
        return false;
    }
}

- (void)hardResetAsync
{
    [self emu]->hardResetAsync();
}

- (void)softResetAsync
{
    [self emu]->softResetAsync();
}

- (void)checkReady:(ExceptionWrapper *)ex
{
    try { [self emu]->checkReady(); }
    catch(Error &error) { [ex save:error]; }
}

- (void)powerOnAsync
{
    [self emu]->powerOnAsync();
}

- (void)powerOffAsync
{
    [self emu]->powerOffAsync();
}

- (void)runAsync:(ExceptionWrapper *)ex
{
    try { [self emu]->runAsync(); }
    catch(Error &error) { [ex save:error]; }
}

- (void)pauseAsync
{
    [self emu]->pauseAsync();
}

- (void)halt
{
    [self emu]->halt();
}

- (void)suspend
{
    [self emu]->suspend();
}

- (void)resume
{
    [self emu]->resume();
}

- (void)warpOnAsync
{
    [self emu]->warpOnAsync(0);
}

- (void)warpOnAsync:(NSInteger)source
{
    [self emu]->warpOnAsync(source);
}

- (void)warpOffAsync
{
    [self emu]->warpOffAsync(0);
}

- (void)warpOffAsync:(NSInteger)source
{
    [self emu]->warpOffAsync(source);
}

- (void)trackOnAsync
{
    [self emu]->trackOnAsync(0);
}

- (void)trackOnAsync:(NSInteger)source
{
    [self emu]->trackOnAsync(source);
}

- (void)trackOffAsync
{
    [self emu]->trackOffAsync(0);
}

- (void)trackOffAsync:(NSInteger)source
{
    [self emu]->trackOffAsync(source);
}

- (void)wakeUp
{
    [self emu]->wakeUp();
}

- (void)exportConfig:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { [self emu]->exportConfig([url fileSystemRepresentation]); }
    catch(Error &error) { [ex save:error]; }
}

- (void)putAsync:(Cmd)type
{
    [self emu]->putAsync(type, 0);
}

- (void)putAsync:(Cmd)type value:(NSInteger)value
{
    [self emu]->putAsync(type, value);
}

- (void)putAsync:(Cmd)type value:(NSInteger)value value2:(NSInteger)value2
{
    [self emu]->putAsync(type, value, value2);
}

- (void)putAsync:(Cmd)type key:(KeyCmd)cmd
{
    [self emu]->putAsync(type, cmd);
}

- (void)putAsync:(Cmd)type coord:(CoordCmd)cmd
{
    [self emu]->putAsync(type, cmd);
}

- (void)putAsync:(Cmd)type action:(GamePadCmd)cmd
{
    [self emu]->putAsync(type, cmd);
}

@end
