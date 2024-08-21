// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#import "config.h"
#import "EmulatorProxy.h"
#import "VAmiga.h"
#import "Emulator.h"
#import "MutableFileSystem.h"
#import "vAmiga-Swift.h"

using namespace vamiga;
using namespace vamiga::moira;

@implementation ExceptionWrapper

@synthesize errorCode;
@synthesize what;

- (instancetype)init {

    if (self = [super init]) {
        
        errorCode = ERROR_OK;
        what = @"";
    }
    return self;
}

- (void)save:(const Error &)exception
{
    errorCode = exception.data;
    what = @(exception.what());
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
// Constants
//

@implementation Constants

+ (NSInteger)hpixels { return HPIXELS; }
+ (NSInteger)vpixels { return VPIXELS; }

+ (NSInteger)hblank_cnt { return HBLANK_CNT; }
+ (NSInteger)hblank_min { return HBLANK_MIN; }
+ (NSInteger)hblank_max { return HBLANK_MAX; }

+ (NSInteger)vblank_cnt { return VBLANK_CNT; }
+ (NSInteger)vblank_min { return VBLANK_MIN; }
+ (NSInteger)vblank_max { return VBLANK_MAX; }

+ (NSInteger)vpos_cnt_pal { return VPOS_CNT_PAL; }
+ (NSInteger)vpos_max_pal { return VPOS_MAX_PAL; }

+ (NSInteger)vpos_cnt_ntsc { return VPOS_CNT_NTSC; }
+ (NSInteger)vpos_max_ntsc { return VPOS_MAX_NTSC; }

+ (NSInteger)vpos_cnt { return VPOS_CNT; }
+ (NSInteger)vpos_max { return VPOS_MAX; }

+ (NSInteger)hpos_cnt_pal { return HPOS_CNT_PAL; }
+ (NSInteger)hpos_max_pal { return HPOS_MAX_PAL; }

+ (NSInteger)hpos_cnt_ntsc { return HPOS_CNT_NTSC; }
+ (NSInteger)hpos_max_ntsc { return HPOS_MAX_NTSC; }

+ (NSInteger)hpos_max { return HPOS_MAX; }
+ (NSInteger)hpos_cnt { return HPOS_CNT; }

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
    catch (Error &error) { [ex save:error]; }
}

- (void)save:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self props]->save([url fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; }
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

- (NSInteger)getOpt:(Option)option
{
    return [self props]->get(option);
}

- (NSInteger)getOpt:(Option)option nr:(NSInteger)nr
{
    return [self props]->get(option, nr);
}

- (void)setKey:(NSString *)key value:(NSString *)value
{
    [self props]->set(string([key UTF8String]), string([value UTF8String]));
}

- (void)setOpt:(Option)option value:(NSInteger)value
{
    [self props]->set(option, value);
}

- (void)setOpt:(Option)option nr:(NSInteger)nr value:(NSInteger)value
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

- (void)remove:(Option)option
{
    [self props]->remove(option);
}

- (void)remove:(Option) option nr:(NSInteger)nr
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

- (NSString *)disassembleRecordedInstr:(NSInteger)i length:(NSInteger *)len
{
    isize result;
    const char *str = [self cpu]->debugger.disassembleRecordedInstr((int)i, &result);
    *len = (NSInteger)result;
    
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleRecordedBytes:(NSInteger)i length:(NSInteger)len
{
    const char *str = [self cpu]->debugger.disassembleRecordedWords(i, len);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleRecordedFlags:(NSInteger)i
{
    const char *str = [self cpu]->debugger.disassembleRecordedFlags((int)i);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleRecordedPC:(NSInteger)i
{
    const char *str = [self cpu]->debugger.disassembleRecordedPC((int)i);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleWord:(NSInteger)value
{
    const char *str = [self cpu]->debugger.disassembleWord((u16)value);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleAddr:(NSInteger)addr
{
    const char *str = [self cpu]->debugger.disassembleAddr((u32)addr);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleInstr:(NSInteger)addr length:(NSInteger *)len
{
    isize result;
    const char *str = [self cpu]->debugger.disassembleInstr((u32)addr, &result);
    *len = result;
    
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleWords:(NSInteger)addr length:(NSInteger)len
{
    const char *str = [self cpu]->debugger.disassembleWords((u32)addr, len);
    return str ? @(str) : nullptr;
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

- (CIAStats)stats
{
    return [self cia]->getStats();
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

- (MemStats)stats
{
    return [self mem]->getStats();
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
    return MediaFile::type([url fileSystemRepresentation]) == FILETYPE_ROM;
}

- (void)loadRom:(MediaFileProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->loadRom(*(MediaFile *)proxy->obj); }
    catch (Error &error) { [ex save:error]; }
}

- (void)loadRomFromBuffer:(NSData *)data exception:(ExceptionWrapper *)ex
{
    assert(data);
    const u8 *bytes = (const u8 *)[data bytes];
    
    try { return [self mem]->loadRom(bytes, [data length]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)loadRomFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->loadRom([url fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)deleteExt
{
    [self mem]->deleteExt();
}

- (BOOL)isExt:(NSURL *)url
{
    return MediaFile::type([url fileSystemRepresentation]) == FILETYPE_EXTENDED_ROM;
}

- (void)loadExt:(MediaFileProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->loadExt(*(MediaFile *)proxy->obj); }
    catch (Error &error) { [ex save:error]; }
}

- (void)loadExtFromBuffer:(NSData *)data exception:(ExceptionWrapper *)ex
{
    assert(data);
    const u8 *bytes = (const u8 *)[data bytes];
    
    try { return [self mem]->loadExt(bytes, [data length]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)loadExtFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->loadExt([url fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)saveRom:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->saveRom([url fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)saveWom:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->saveWom([url fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)saveExt:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->saveExt([url fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; }
}

- (MemorySource)memSrc:(Accessor)accessor addr:(NSInteger)addr
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    
    if (accessor == ACCESSOR_CPU) {
        return [self mem]->debugger.getMemSrc(ACCESSOR_CPU, (u32)addr);
    } else {
        return [self mem]->debugger.getMemSrc(ACCESSOR_AGNUS, (u32)addr);
    }
}

- (NSInteger)spypeek16:(Accessor)accessor addr:(NSInteger)addr
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    
    if (accessor == ACCESSOR_CPU) {
        return [self mem]->debugger.spypeek16(ACCESSOR_CPU, (u32)addr);
    } else {
        return [self mem]->debugger.spypeek16(ACCESSOR_AGNUS, (u32)addr);
    }
}

- (NSString *)ascDump:(Accessor)accessor addr:(NSInteger)addr bytes:(NSInteger)bytes
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    auto str = [self mem]->debugger.ascDump(accessor, (u32)addr, bytes);

    return @(str.c_str());
}

- (NSString *)hexDump:(Accessor)accessor addr: (NSInteger)addr bytes:(NSInteger)bytes
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
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

- (AudioPortStats)stats
{
    return [self port]->getStats();
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

- (AgnusStats)stats
{
    return [self agnus]->getStats();
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

@implementation DmaDebuggerProxy

- (DmaDebuggerAPI *)debugger
{
    return (DmaDebuggerAPI *)obj;
}

- (DmaDebuggerInfo)info
{
    return [self debugger]->getInfo();
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
// Recorder
//

@implementation RecorderProxy

- (RecorderAPI *)recorder
{
    return (RecorderAPI *)obj;
}

- (NSString *)path
{
    auto path = [self recorder]->getExecPath(); // FFmpeg::getExecPath();
    return @(path.c_str());
}

- (void)setPath:(NSString *)path
{
    if ([path length] == 0) {
        [self recorder]->setExecPath("");
    } else {
        [self recorder]->setExecPath([path fileSystemRepresentation]);
    }
}

- (NSString *)findFFmpeg:(NSInteger)nr
{
    auto &paths = [self recorder]->paths();

    if (nr < (NSInteger)paths.size()) {
        return @(paths[nr].c_str());
    } else {
        return nil;
    }
}

- (BOOL)hasFFmpeg
{
    return [self recorder]->hasFFmpeg();
}

- (BOOL)recording
{
    return [self recorder]->isRecording();
}

- (double)duration
{
    return [self recorder]->getDuration();
}

- (NSInteger)frameRate
{
    return [self recorder]->getFrameRate();
}

- (NSInteger)bitRate
{
    return [self recorder]->getBitRate();
}

- (NSInteger)sampleRate
{
    return [self recorder]->getSampleRate();
}

- (void)startRecording:(NSRect)rect
               bitRate:(NSInteger)rate
               aspectX:(NSInteger)aspectX
               aspectY:(NSInteger)aspectY
             exception:(ExceptionWrapper *)ex
{
    auto x1 = isize(rect.origin.x);
    auto y1 = isize(rect.origin.y);
    auto x2 = isize(x1 + (int)rect.size.width);
    auto y2 = isize(y1 + (int)rect.size.height);
    
    try { return [self recorder]->startRecording(x1, y1, x2, y2, rate, aspectX, aspectY); }
    catch (Error &error) { [ex save:error]; }
}

- (void)stopRecording
{
    [self recorder]->stopRecording();
}

- (BOOL)exportAs:(NSString *)path
{
    return [self recorder]->exportAs(string([path fileSystemRepresentation]));
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

- (BOOL)detectShakeAbs:(NSPoint)pos
{
    return [self mouse]->detectShakeXY(pos.x, pos.y);
}

- (BOOL)detectShakeRel:(NSPoint)pos
{
    return [self mouse]->detectShakeDxDy(pos.x, pos.y);
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

@end


//
// Joystick
//

@implementation JoystickProxy

- (JoystickAPI *)joystick
{
    return (JoystickAPI *)obj;
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

- (void)texture:(const u32 **)ptr nr:(NSInteger *)nr lof:(bool *)lof prevlof:(bool *)prevlof
{
    isize inr;

    *ptr = [self port]->getTexture(&inr, lof, prevlof);
    *nr = inr;
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

- (void)insertBlankDisk:(FSVolumeType)fs bootBlock:(BootBlockId)bb name:(NSString *)name exception:(ExceptionWrapper *)ex
{
    try { return [self drive]->insertBlankDisk(fs, bb, [name UTF8String]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)insertMedia:(MediaFileProxy *)proxy protected:(BOOL)wp exception:(ExceptionWrapper *)ex
{
    try { [self drive]->insertMedia(*(MediaFile *)proxy->obj, wp); }
    catch (Error &error) { [ex save:error]; }
}

- (void)eject
{
    [self drive]->ejectDisk();
}

- (MediaFileProxy *)exportDisk:(FileType)type exception:(ExceptionWrapper *)ex
{
    try { return [MediaFileProxy make:[self drive]->exportDisk(type)]; }
    catch (Error &error) { [ex save:error]; }
    return nil;
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

- (HdcStats)stats
{
    return [self controller]->getStats();
}

@end

//
// FSDevice proxy
//

@implementation FileSystemProxy

- (MutableFileSystem *)fs
{
    return (MutableFileSystem *)obj;
}

+ (instancetype)make:(MutableFileSystem *)volume
{
    if (volume == nullptr) { return nil; }
    
    FileSystemProxy *proxy = [[self alloc] initWith: volume];
    return proxy;
}

+ (instancetype)makeWithMedia:(MediaFileProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try {

        auto file = (MediaFile *)(proxy->obj);
        auto dev = new MutableFileSystem(*file);
        return [self make:dev];

    }  catch (Error &error) {

        [ex save:error];
        return nil;
    }
}

+ (instancetype)makeWithMedia:(MediaFileProxy *)proxy partition:(NSInteger)nr exception:(ExceptionWrapper *)ex
{
    try {

        auto file = (MediaFile *)(proxy->obj);
        auto dev = new MutableFileSystem(*file, nr);
        return [self make:dev];

    }  catch (Error &error) {

        [ex save:error];
        return nil;
    }
}

- (NSString *)name
{
    auto str = [self fs]->getName();
    return @(str.c_str());
}

- (NSString *)creationDate
{
    auto str = [self fs]->getCreationDate();
    return @(str.c_str());
}

- (NSString *)modificationDate
{
    auto str = [self fs]->getModificationDate();
    return @(str.c_str());
}

- (NSString *)bootBlockName
{
    auto str = [self fs]->getBootBlockName();
    return @(str.c_str());
}

- (NSString *)capacityString
{
    auto str = util::byteCountAsString([self fs]->numBytes());
    return @(str.c_str());
}

- (NSString *)fillLevelString
{
    auto str = util::fillLevelAsString([self fs]->fillLevel());
    return @(str.c_str());
}

- (FSVolumeType)dos
{
    return [self fs]->getDos();
}

- (BOOL)isOFS
{
    return [self fs]->isOFS();
}

- (BOOL)isFFS
{
    return [self fs]->isFFS();
}

- (NSInteger)blockSize
{
    return [self fs]->blockSize();
}

- (NSInteger)numBlocks
{
    return [self fs]->numBlocks();
}

- (NSInteger)numBytes
{
    return [self fs]->numBytes();
}

- (NSInteger)usedBlocks
{
    return [self fs]->usedBlocks();
}

- (double)fillLevel
{
    return [self fs]->fillLevel();
}

- (BOOL)hasVirus
{
    return [self fs]->hasVirus();
}

- (void)killVirus
{
    [self fs]->killVirus();
}

- (FSBlockType)blockType:(NSInteger)blockNr
{
    return [self fs]->blockType((u32)blockNr);
}

- (FSItemType)itemType:(NSInteger)blockNr pos:(NSInteger)pos
{
    return [self fs]->itemType((u32)blockNr, pos);
}

- (FSErrorReport)check:(BOOL)strict
{
    return [self fs]->check(strict);
}

- (ErrorCode)check:(NSInteger)nr
               pos:(NSInteger)pos
          expected:(unsigned char *)exp
            strict:(BOOL)strict
{
    return [self fs]->check((u32)nr, pos, exp, strict);
}

- (BOOL)isCorrupted:(NSInteger)blockNr
{
    return [self fs]->isCorrupted((u32)blockNr);
}

- (NSInteger)getCorrupted:(NSInteger)blockNr
{
    return [self fs]->getCorrupted((u32)blockNr);
}

- (NSInteger)nextCorrupted:(NSInteger)blockNr
{
    return [self fs]->nextCorrupted((u32)blockNr);
}

- (NSInteger)prevCorrupted:(NSInteger)blockNr
{
    return [self fs]->prevCorrupted((u32)blockNr);
}

- (void)printDirectory:(BOOL) recursive
{
    return [self fs]->printDirectory(recursive);
}

- (NSInteger)readByte:(NSInteger)block offset:(NSInteger)offset
{
    return [self fs]->readByte((u32)block, offset);
}

- (NSString *)ascii:(NSInteger)block offset:(NSInteger)offset length:(NSInteger)len
{
    return @([self fs]->ascii(Block(block), offset, len).c_str());
}

- (void)export:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self fs]->exportDirectory(string([path fileSystemRepresentation])); }
    catch (Error &error) { [ex save:error]; }
}

- (FSBlockType)getDisplayType:(NSInteger)column
{
    return [self fs]->getDisplayType(column);
}

- (NSInteger)diagnoseImageSlice:(NSInteger)column
{
    return [self fs]->diagnoseImageSlice(column);
}

- (NSInteger)nextBlockOfType:(FSBlockType)type after:(NSInteger)after
{
    return [self fs]->nextBlockOfType(type, after);
}

- (NSInteger)nextCorruptedBlock:(NSInteger)after
{
    return [self fs]->nextCorruptedBlock(after);
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

-(NSInteger)cursorRel
{
    return [self shell]->cursorRel();
}

-(NSString *)getText
{
    const char *str = [self shell]->text();
    return str ? @(str) : nullptr;
}

- (void)pressKey:(char)c
{
    [self shell]->press(c);
}

- (void)pressSpecialKey:(RetroShellKey)key
{
    [self shell]->press(key);
}

- (void)pressSpecialKey:(RetroShellKey)key shift:(BOOL)shift
{
    [self shell]->press(key, shift);
}

- (void)executeScript:(MediaFileProxy *)file
{
    [self shell]->execScript(*(MediaFile *)file->obj);
}

@end


//
// MediaFile
//

@implementation MediaFileProxy

- (MediaFile *)file
{
    return (MediaFile *)obj;
}

+ (instancetype)make:(void *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (FileType)typeOfUrl:(NSURL *)url
{
    return MediaFile::type([url fileSystemRepresentation]);
}

+ (instancetype)makeWithFile:(NSString *)path
                   exception:(ExceptionWrapper *)ex
{
    try { return [self make: MediaFile::make([path fileSystemRepresentation])]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithFile:(NSString *)path
                        type:(FileType)type
                   exception:(ExceptionWrapper *)ex
{
    try { return [self make: MediaFile::make([path fileSystemRepresentation], type)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len
                          type:(FileType)type
                     exception:(ExceptionWrapper *)ex
{
    try { return [self make: MediaFile::make((u8 *)buf, len, type)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithAmiga:(EmulatorProxy *)proxy
{
    auto amiga = (VAmiga *)proxy->obj;
    return [self make:amiga->amiga.takeSnapshot()];
}

+ (instancetype)makeWithDrive:(FloppyDriveProxy *)proxy
                         type:(FileType)type
                    exception:(ExceptionWrapper *)ex
{
    auto drive = (FloppyDriveAPI *)proxy->obj;
    try { return [self make: MediaFile::make(*drive, type)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithHardDrive:(HardDriveProxy *)proxy
                             type:(FileType)type
                        exception:(ExceptionWrapper *)ex
{
    auto drive = (HardDriveAPI *)proxy->obj;
    try { return [self make: MediaFile::make(*drive, type)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithFileSystem:(FileSystemProxy *)proxy
                              type:(FileType)type
                         exception:(ExceptionWrapper *)ex
{
    auto fs = (MutableFileSystem *)proxy->obj;
    try { return [self make: MediaFile::make(*fs, type)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

- (FileType)type
{
    return [self file]->type();
}

/*
- (NSString *)name
{
    return @([self file]->getName().c_str());
}
*/

- (u64)fnv
{
    return [self file]->fnv64();
}

- (NSInteger)size
{
    return [self file]->getSize();
}

- (u8 *)data
{
    return [self file]->getData();
}

- (void)writeToFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { [self file]->writeToFile(string([path fileSystemRepresentation])); }
    catch (Error &err) { [ex save:err]; }
}

- (void)writeToFile:(NSString *)path partition:(NSInteger)part exception:(ExceptionWrapper *)ex
{
    try { [self file]->writePartitionToFile(string([path fileSystemRepresentation]), part); }
    catch (Error &err) { [ex save:err]; }
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

- (DiskInfo)diskInfo
{
    return [self file]->getDiskInfo();
}

- (FloppyDiskInfo)floppyDiskInfo
{
    return [self file]->getFloppyDiskInfo();
}

-(HDFInfo)hdfInfo
{
    return [self file]->getHDFInfo();
}

- (NSInteger)readByte:(NSInteger)b offset:(NSInteger)offset
{
    return [self file]->readByte(b, offset);
}

- (void)readSector:(NSInteger)b destination:(unsigned char *)buf
{
    [self file]->readSector(buf, b);
}

- (NSString *)hexdump:(NSInteger)b offset:(NSInteger)offset len:(NSInteger)len
{
    return @([self file]->hexdump(b, offset, len).c_str());
}

- (NSString *)asciidump:(NSInteger)b offset:(NSInteger)offset len:(NSInteger)len
{
    return @([self file]->asciidump(b, offset, len).c_str());
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
    }  catch (Error &error) {
        [ex save:error];
    }
}

- (void)attach:(MediaFileProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try {
        [self drive]->attach(*(MediaFile *)proxy->obj);
    }  catch (Error &error) {
        [ex save:error];
    }
}

- (void)attach:(NSInteger)c h:(NSInteger)h s:(NSInteger)s b:(NSInteger)b
     exception:(ExceptionWrapper *)ex
{
    try {
        [self drive]->attach(c, h, s, b);
    }  catch (Error &error) {
        [ex save:error];
    }
}

- (void)format:(FSVolumeType)fs name:(NSString *)name exception:(ExceptionWrapper *)ex
{
    auto str = string([name UTF8String]);

    try {
        [self drive]->format(fs, str);
    }  catch (Error &error) {
        [ex save:error];
    }
}

- (void)changeGeometry:(NSInteger)c h:(NSInteger)h s:(NSInteger)s b:(NSInteger)b exception:(ExceptionWrapper *)ex
{
    try {
        [self drive]->changeGeometry(c, h, s, b);
    }  catch (Error &error) {
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
    catch (Error &error) { [ex save:error]; }
}

- (void)enableWriteThrough:(ExceptionWrapper *)ex
{
    auto id = [self drive]->getInfo().nr;
    try { return [self drive]->emu->set(OPT_HDR_WRITE_THROUGH, true, { id }); }
    catch (Error &error) { [ex save:error]; }
}

- (void)disableWriteThrough
{
    auto id = [self drive]->getInfo().nr;
    [self drive]->emu->set(OPT_HDR_WRITE_THROUGH, false, { id });
}

@end


//
// AmigaFile proxy
//

@implementation AmigaFileProxy

- (AmigaFile *)file
{
    return (AmigaFile *)obj;
}

+ (instancetype)make:(AmigaFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (FileType)typeOfUrl:(NSURL *)url
{
    return AmigaFile::type([url fileSystemRepresentation]);
}

- (FileType)type
{
    return [self file]->type();
}

- (NSInteger)size
{
    return [self file]->getSize();
}

- (NSString *)getSizeAsString
{
    const string &str = [self file]->getSizeAsString();
    return @(str.c_str());
}

- (u64)fnv
{
    return [self file]->fnv64();
}

- (void)setPath:(NSString *)path
{
    [self file]->path = [path fileSystemRepresentation];
}

- (NSInteger)writeToFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self file]->writeToFile([path fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; return 0; }
}

- (void)dealloc
{
    delete (AmigaFile *)obj;
}

@end


//
// DiskFileProxy
//

@implementation DiskFileProxy

- (DiskFile *)file
{
    return (DiskFile *)obj;
}

- (NSInteger)numCyls
{
    return [self file]->numCyls();
}

- (NSInteger)numHeads
{
    return [self file]->numHeads();
}

- (NSInteger)bsize
{
    return [self file]->bsize();
}

- (NSInteger)numTracks
{
    return [self file]->numTracks();
}

- (NSInteger)numSectors
{
    return [self file]->numSectors();
}

- (NSInteger)numBlocks
{
    return [self file]->numBlocks();
}

- (NSInteger)readByte:(NSInteger)b offset:(NSInteger)offset
{
    return [self file]->readByte(b, offset);
}

- (void)readSector:(NSInteger)b destination:(unsigned char *)buf
{
    [self file]->readSector(buf, b);
}

- (NSString *) describeGeometry
{
    return @([self file]->describeGeometry().c_str());
}

- (NSString *) describeCapacity
{
    return @([self file]->describeCapacity().c_str());
}

- (NSString *)hexdump:(NSInteger)b offset:(NSInteger)offset len:(NSInteger)len
{
    return @([self file]->hexdump(b, offset, len).c_str());
}

- (NSString *)asciidump:(NSInteger)b offset:(NSInteger)offset len:(NSInteger)len
{
    return @([self file]->asciidump(b, offset, len).c_str());
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

- (MediaFileProxy *)takeSnapshot
{
    MediaFile *file = [self amiga]->takeSnapshot();
    return [MediaFileProxy make:file];
}

- (void)loadSnapshot:(MediaFileProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { [self amiga]->loadSnapshot(*[proxy file]); }
    catch (Error &error) { [ex save:error]; }
}

@end


//
// Emulator
//

@implementation EmulatorProxy

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
@synthesize dmaDebugger;
@synthesize hd0;
@synthesize hd1;
@synthesize hd2;
@synthesize hd3;
@synthesize keyboard;
@synthesize mem;
@synthesize paula;
@synthesize remoteManager;
@synthesize retroShell;
@synthesize rtc;
@synthesize serialPort;
@synthesize recorder;
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
    dmaDebugger = [[DmaDebuggerProxy alloc] initWith:&vamiga->agnus.dma.debugger];
    df0 = [[FloppyDriveProxy alloc] initWith:&vamiga->df0];
    df1 = [[FloppyDriveProxy alloc] initWith:&vamiga->df1];
    df2 = [[FloppyDriveProxy alloc] initWith:&vamiga->df2];
    df3 = [[FloppyDriveProxy alloc] initWith:&vamiga->df3];
    hd0 = [[HardDriveProxy alloc] initWith:&vamiga->hd0];
    hd1 = [[HardDriveProxy alloc] initWith:&vamiga->hd1];
    hd2 = [[HardDriveProxy alloc] initWith:&vamiga->hd2];
    hd3 = [[HardDriveProxy alloc] initWith:&vamiga->hd3];
    keyboard = [[KeyboardProxy alloc] initWith:&vamiga->keyboard];
    mem = [[MemProxy alloc] initWith:&vamiga->mem];
    paula = [[PaulaProxy alloc] initWith:&vamiga->paula];
    retroShell = [[RetroShellProxy alloc] initWith:&vamiga->retroShell];
    rtc = [[RtcProxy alloc] initWith:&vamiga->rtc];
    recorder = [[RecorderProxy alloc] initWith:&vamiga->recorder];
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

+ (DefaultsProxy *) defaults
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
    NSLog(@"dealloc");
}

- (void)kill
{
    NSLog(@"kill");
    
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

- (EmulatorStats)stats
{
    return [self emu]->getStats();
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

/*
- (void)setTrackMode:(BOOL)value
{
    if (value) {
        [self emu]->emu->trackOn();
    } else {
        [self emu]->emu->trackOff();
    }
}
*/

- (void)stepInto
{
    [self emu]->stepInto();
}

- (void)stepOver
{
    [self emu]->stepOver();
}

- (void)launch:(const void *)listener function:(Callback *)func
{
    [self emu]->launch(listener, func);
}

- (NSInteger)get:(Option)opt
{
    return [self emu]->get(opt);
}

- (NSInteger)get:(Option)opt id:(NSInteger)id
{
    return [self emu]->get(opt, id);
}

- (NSInteger)get:(Option)opt drive:(NSInteger)id
{
    return [self emu]->get(opt, (long)id);
}

- (BOOL)set:(Option)opt value:(NSInteger)val
{
    try {
        [self emu]->set(opt, val);
        return true;
    } catch (Error &exception) {
        return false;
    }
}

- (BOOL)set:(Option)opt enable:(BOOL)val
{
    try {
        [self emu]->set(opt, val ? 1 : 0);
        return true;
    } catch (Error &exception) {
        return false;
    }
}

- (BOOL)set:(Option)opt id:(NSInteger)id value:(NSInteger)val
{
    try {
        [self emu]->set(opt, val, id);
        return true;
    } catch (Error &exception) {
        return false;
    }
}

- (BOOL)set:(Option)opt id:(NSInteger)id enable:(BOOL)val
{
    try {
        [self emu]->set(opt, val ? 1 : 0, id);
        return true;
    } catch (Error &exception) {
        return false;
    }
}

- (BOOL)set:(Option)opt drive:(NSInteger)id value:(NSInteger)val
{
    try {
        [self emu]->set(opt, val, (long)id);
        return true;
    } catch (Error &exception) {
        return false;
    }
}

- (BOOL)set:(Option)opt drive:(NSInteger)id enable:(BOOL)val
{
    try {
        [self emu]->set(opt, val ? 1 : 0, (long)id);
        return true;
    } catch (Error &exception) {
        return false;
    }
}

- (void)hardReset
{
    [self emu]->hardReset();
}

- (void)softReset
{
    [self emu]->softReset();
}

- (void)isReady:(ExceptionWrapper *)ex
{
    try { [self emu]->isReady(); }
    catch (Error &error) { [ex save:error]; }
}

- (void)powerOn
{
    [self emu]->powerOn();
}

- (void)powerOff
{
    [self emu]->powerOff();
}

- (void)run:(ExceptionWrapper *)ex
{
    try { [self emu]->run(); }
    catch (Error &error) { [ex save:error]; }
}

- (void)pause
{
    [self emu]->pause();
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

- (void)warpOn
{
    [self emu]->warpOn();
}

- (void)warpOn:(NSInteger)source
{
    [self emu]->warpOn(source);
}

- (void)warpOff
{
    [self emu]->warpOff();
}

- (void)warpOff:(NSInteger)source
{
    [self emu]->warpOff(source);
}

- (void)trackOn
{
    [self emu]->trackOn();
}

- (void)trackOn:(NSInteger)source
{
    [self emu]->trackOn(source);
}

- (void)trackOff
{
    [self emu]->trackOff();
}

- (void)trackOff:(NSInteger)source
{
    [self emu]->trackOff(source);
}

- (void)wakeUp
{
    [self emu]->wakeUp();
}

- (void)exportConfig:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { [self emu]->exportConfig([url fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)put:(CmdType)type
{
    [self emu]->put(type, 0);
}

- (void)put:(CmdType)type value:(NSInteger)value
{
    [self emu]->put(type, value);
}

- (void)put:(CmdType)type value:(NSInteger)value value2:(NSInteger)value2
{
    [self emu]->put(type, value, value2);
}

- (void)put:(CmdType)type key:(KeyCmd)cmd
{
    [self emu]->put(type, cmd);
}

- (void)put:(CmdType)type coord:(CoordCmd)cmd
{
    [self emu]->put(type, cmd);
}

- (void)put:(CmdType)type action:(GamePadCmd)cmd
{
    [self emu]->put(type, cmd);
}

@end
