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

- (Defaults *)props
{
    return (Defaults *)obj;
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

- (SnapshotProxy *)takeSnapshot
{
    Snapshot *snapshot = [self amiga]->takeSnapshot();
    return [SnapshotProxy make:snapshot];
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
    return [self cpu]->cpu->getInfo();
}

- (CPUInfo)cachedInfo
{
    return [self cpu]->cpu->getCachedInfo();
}

- (i64)clock
{
    return [self cpu]->cpu->getCpuClock();
}

- (BOOL)halted
{
    return [self cpu]->cpu->isHalted();
}

- (NSInteger)loggedInstructions
{
    return [self cpu]->cpu->debugger.loggedInstructions();
}

- (void)clearLog
{
    return [self cpu]->cpu->debugger.clearLog();
}

- (NSString *)disassembleRecordedInstr:(NSInteger)i length:(NSInteger *)len
{
    isize result;
    const char *str = [self cpu]->cpu->disassembleRecordedInstr((int)i, &result);
    *len = (NSInteger)result;
    
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleRecordedBytes:(NSInteger)i length:(NSInteger)len
{
    const char *str = [self cpu]->cpu->disassembleRecordedWords(i, len);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleRecordedFlags:(NSInteger)i
{
    const char *str = [self cpu]->cpu->disassembleRecordedFlags((int)i);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleRecordedPC:(NSInteger)i
{
    const char *str = [self cpu]->cpu->disassembleRecordedPC((int)i);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleWord:(NSInteger)value
{
    const char *str = [self cpu]->cpu->disassembleWord((u16)value);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleAddr:(NSInteger)addr
{
    const char *str = [self cpu]->cpu->disassembleAddr((u32)addr);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleInstr:(NSInteger)addr length:(NSInteger *)len
{
    isize result;
    const char *str = [self cpu]->cpu->disassembleInstr((u32)addr, &result);
    *len = result;
    
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleWords:(NSInteger)addr length:(NSInteger)len
{
    const char *str = [self cpu]->cpu->disassembleWords((u32)addr, len);
    return str ? @(str) : nullptr;
}

- (NSString *)vectorName:(NSInteger)nr
{
    auto name = [self cpu]->cpu->debugger.vectorName(u8(nr));
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
    return RomFile::isRomFile([url fileSystemRepresentation]);
}

- (void)loadRom:(RomFileProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->mem->loadRom(*(RomFile *)proxy->obj); }
    catch (Error &error) { [ex save:error]; }
}

- (void)loadRomFromBuffer:(NSData *)data exception:(ExceptionWrapper *)ex
{
    assert(data);
    const u8 *bytes = (const u8 *)[data bytes];
    
    try { return [self mem]->mem->loadRom(bytes, [data length]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)loadRomFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->mem->loadRom([url fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)deleteExt
{
    [self mem]->mem->deleteExt();
}

- (BOOL)isExt:(NSURL *)url
{
    return ExtendedRomFile::isExtendedRomFile([url fileSystemRepresentation]);
}

- (void)loadExt:(ExtendedRomFileProxy *)proxy
{
    [self mem]->mem->loadExt(*(ExtendedRomFile *)proxy->obj);
}

- (void)loadExt:(ExtendedRomFileProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->mem->loadExt(*(ExtendedRomFile *)proxy->obj); }
    catch (Error &error) { [ex save:error]; }
}

- (void)loadExtFromBuffer:(NSData *)data exception:(ExceptionWrapper *)ex
{
    assert(data);
    const u8 *bytes = (const u8 *)[data bytes];
    
    try { return [self mem]->mem->loadExt(bytes, [data length]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)loadExtFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->mem->loadExt([url fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)saveRom:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->mem->saveRom([url fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)saveWom:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->mem->saveWom([url fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)saveExt:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->mem->saveExt([url fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; }
}

- (MemorySource)memSrc:(Accessor)accessor addr:(NSInteger)addr
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    
    if (accessor == ACCESSOR_CPU) {
        return [self mem]->mem->getMemSrc <ACCESSOR_CPU> ((u32)addr);
    } else {
        return [self mem]->mem->getMemSrc <ACCESSOR_AGNUS> ((u32)addr);
    }
}

- (NSInteger)spypeek16:(Accessor)accessor addr:(NSInteger)addr
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    
    if (accessor == ACCESSOR_CPU) {
        return [self mem]->mem->spypeek16 <ACCESSOR_CPU> ((u32)addr);
    } else {
        return [self mem]->mem->spypeek16 <ACCESSOR_AGNUS> ((u32)addr);
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
// Audio port
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
    auto path = FFmpeg::getExecPath();
    return @(path.c_str());
}

- (void)setPath:(NSString *)path
{
    if ([path length] == 0) {
        FFmpeg::setExecPath("");
    } else {
        FFmpeg::setExecPath(string([path fileSystemRepresentation]));
    }
}

- (NSString *)findFFmpeg:(NSInteger)nr
{
    if (nr < (NSInteger)FFmpeg::paths.size()) {
        return @(FFmpeg::paths[nr].c_str());
    } else {
        return nil;
    }
}

- (BOOL)hasFFmpeg
{
    return FFmpeg::available();
}

- (BOOL)recording
{
    return [self recorder]->recorder->isRecording();
}

- (double)duration
{
    return [self recorder]->recorder->getDuration().asSeconds();
}

- (NSInteger)frameRate
{
    return [self recorder]->recorder->getFrameRate();
}

- (NSInteger)bitRate
{
    return [self recorder]->recorder->getBitRate();
}

- (NSInteger)sampleRate
{
    return [self recorder]->recorder->getSampleRate();
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
    
    try { return [self recorder]->recorder->startRecording(x1, y1, x2, y2, rate, aspectX, aspectY); }
    catch (Error &error) { [ex save:error]; }
}

- (void)stopRecording
{
    [self recorder]->recorder->stopRecording();
}

- (BOOL)exportAs:(NSString *)path
{
    return [self recorder]->recorder->exportAs(string([path fileSystemRepresentation]));
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
    return [self paula]->paula->uart.getInfo();
}

- (UARTInfo)cachedUartInfo
{
    return [self paula]->paula->uart.getCachedInfo();
}

- (AudioPortStats)audioPortStats
{
    return [self paula]->paula->emulator.main.audioPort.getStats();
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
    [self joystick]->joystick->trigger(event);
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

- (void)texture:(u32 **)ptr nr:(NSInteger *)nr lof:(bool *)lof prevlof:(bool *)prevlof
{
    auto &frameBuffer = [self port]->getTexture();
    *ptr = frameBuffer.pixels.ptr;
    *nr = NSInteger(frameBuffer.nr);
    *lof = frameBuffer.lof;
    *prevlof = frameBuffer.prevlof;
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

- (SerialPortInfo)info
{
    return [self serial]->serialPort->getInfo();
}

- (SerialPortInfo)cachedInfo
{
    return [self serial]->serialPort->getCachedInfo();
}

- (NSInteger)readIncomingPrintableByte
{
    return [self serial]->serialPort->readIncomingPrintableByte();
}

- (NSInteger)readOutgoingPrintableByte
{
    return [self serial]->serialPort->readOutgoingPrintableByte();
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
    [self kb]->press((KeyCode)keycode);
}

- (void)release:(NSInteger)keycode
{
    [self kb]->release((KeyCode)keycode);
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
    return [self drive]->drive->isInsertable(type, density);
}

- (void)eject
{
    [self drive]->drive->ejectDisk();
}

- (void)swap:(FloppyFileProxy *)fileProxy exception:(ExceptionWrapper *)ex
{
    try { return [self drive]->drive->swapDisk(*(FloppyFile *)fileProxy->obj); }
    catch (Error &error) { [ex save:error]; }
}

- (void)insertNew:(FSVolumeType)fs bootBlock:(BootBlockId)bb name:(NSString *)name exception:(ExceptionWrapper *)ex
{
    try { return [self drive]->drive->insertNew(fs, bb, [name UTF8String]); }
    catch (Error &error) { [ex save:error]; }
}

- (NSString *)readTrackBits:(NSInteger)track
{
    if (![self drive]->drive->hasDisk()) return @("");
    return @([self drive]->drive->disk->readTrackBits(track).c_str());
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

- (HardDriveTraits)traits
{
    return [self drive]->getTraits();
}

- (PartitionTraits) partitionTraits:(NSInteger)nr
{
    return [self drive]->getPartitionTraits(nr);
}

- (HardDriveInfo)info
{
    return [self drive]->getInfo();
}

- (BOOL)getFlag:(DiskFlags)mask
{
    return [self drive]->getFlag(mask);
}

- (void)setFlag:(DiskFlags)mask value:(BOOL)value
{
    [self drive]->setFlag(mask, value);
}

- (NSInteger)capacity
{
    return [self drive]->drive->getGeometry().numBytes();
}

- (NSInteger)partitions
{
    return [self drive]->drive->numPartitions();
}

- (NSInteger)cylinders
{
    return [self drive]->drive->getGeometry().cylinders;
}

- (NSInteger)heads
{
    return [self drive]->drive->getGeometry().heads;
}

- (NSInteger)sectors
{
    return [self drive]->drive->getGeometry().sectors;
}

- (NSInteger)bsize
{
    return [self drive]->drive->getGeometry().bsize;
}

- (void)attachFile:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try {
        [self drive]->drive->init([url fileSystemRepresentation]);
    }  catch (Error &error) {
        [ex save:error];
    }
}

- (void)attach:(HDFFileProxy *)hdf exception:(ExceptionWrapper *)ex
{
    try {
        [self drive]->drive->init(*(HDFFile *)hdf->obj);
    }  catch (Error &error) {
        [ex save:error];
    }
}

- (void)attach:(NSInteger)c h:(NSInteger)h s:(NSInteger)s b:(NSInteger)b
        exception:(ExceptionWrapper *)ex
{
    GeometryDescriptor geometry;
    geometry.cylinders = c;
    geometry.heads = h;
    geometry.sectors = s;
    geometry.bsize = b;
    
    try {
        [self drive]->drive->init(geometry);
    }  catch (Error &error) {
        [ex save:error];
    }
}

- (void)format:(FSVolumeType)fs name:(NSString *)name exception:(ExceptionWrapper *)ex
{
    auto str = string([name UTF8String]);
    
    try {
        [self drive]->drive->format(fs, str);
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
    try { return [self drive]->drive->writeToFile([url fileSystemRepresentation]); }
    catch (Error &error) { [ex save:error]; }
}

- (void)enableWriteThrough:(ExceptionWrapper *)ex
{
    try { return [self drive]->drive->enableWriteThrough(); }
    catch (Error &error) { [ex save:error]; }
}

- (void)disableWriteThrough
{
    [self drive]->drive->disableWriteThrough();
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

+ (instancetype)makeWithADF:(ADFFileProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try {
        
        auto adf = (ADFFile *)(proxy->obj);
        auto dev = new MutableFileSystem(*adf);
        return [self make:dev];
        
    }  catch (Error &error) {
        
        [ex save:error];
        return nil;
    }
}

+ (instancetype)makeWithHDF:(HDFFileProxy *)proxy partition:(NSInteger)nr exception:(ExceptionWrapper *)ex
{
    try {
        
        auto hdf = (HDFFile *)(proxy->obj);
        auto dev = new MutableFileSystem(*hdf, nr);
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

/*
- (void)executeScript:(MediaFileProxy *)file
{
    [self shell]->execScript(*(MediaFile *)file->obj);
}
*/

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
// Snapshot proxy
//

@implementation SnapshotProxy

- (Snapshot *)snapshot
{
    return (Snapshot *)obj;
}

+ (instancetype)make:(void *)snapshot
{
    SnapshotProxy *proxy = [[self alloc] initWith:snapshot];
    if (proxy) { proxy->preview = nullptr; }
    return proxy;
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self make: new Snapshot([path fileSystemRepresentation])]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new Snapshot((u8 *)buf, len)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithAmiga:(AmigaProxy *)proxy
{
    Snapshot *snapshot = ((AmigaAPI *)proxy->obj)->takeSnapshot();
    return [self make:snapshot];
}

- (NSImage *)previewImage
{
    // Return cached image (if any)
    if (preview) { return preview; }

    // Create preview image
    const Thumbnail &thumbnail = [self snapshot]->getThumbnail();
    unsigned char *data = (unsigned char *)thumbnail.screen;
    
    NSBitmapImageRep *rep = [[NSBitmapImageRep alloc]
                             initWithBitmapDataPlanes:&data
                             pixelsWide:thumbnail.width
                             pixelsHigh:thumbnail.height
                             bitsPerSample:8
                             samplesPerPixel:4
                             hasAlpha:true
                             isPlanar:false
                             colorSpaceName:NSCalibratedRGBColorSpace
                             bytesPerRow:4*thumbnail.width
                             bitsPerPixel:32];
    
    preview = [[NSImage alloc] initWithSize:[rep size]];
    [preview addRepresentation:rep];
    
    // image.makeGlossy()

    return preview;
}

- (time_t)timeStamp
{
    return [self snapshot]->getThumbnail().timestamp;
}

@end


//
// Script proxy
//

@implementation ScriptProxy

- (Script *)script
{
    return (Script *)obj;
}

+ (instancetype)make:(Script *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self make: new Script([path fileSystemRepresentation])]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new Script((const u8 *)buf, len)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

- (void)execute:(EmulatorProxy *)proxy
{
    auto *am = [proxy amiga];
    auto *amiga = (AmigaAPI *)am->obj;

    [self script]->execute(*(amiga->amiga));
}

@end


//
// RomFile proxy
//

@implementation RomFileProxy

+ (instancetype)make:(RomFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self make: new RomFile([path fileSystemRepresentation])]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new RomFile((const u8 *)buf, len)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

@end


//
// ExtendedRomFile proxy
//

@implementation ExtendedRomFileProxy

+ (instancetype)make:(ExtendedRomFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self make: new ExtendedRomFile([path fileSystemRepresentation])]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new ExtendedRomFile((const u8 *)buf, len)]; }
    catch (Error &error) { [ex save:error]; return nil; }
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
// FloppyFileProxy
//

@implementation FloppyFileProxy

- (FloppyFile *)file
{
    return (FloppyFile *)obj;
}

+ (instancetype)make:(FloppyFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

- (FSVolumeType)dos
{
    return [self file]->getDos();
}

- (Diameter)diskType
{
    return [self file]->getDiameter();
}

- (Density)diskDensity
{
    return [self file]->getDensity();
}

- (BOOL)isSD
{
    return [self file]->isSD();
}

- (BOOL)isDD
{
    return [self file]->isDD();
}

- (BOOL)isHD
{
    return [self file]->isHD();
}

- (BootBlockType)bootBlockType
{
    return [self file]->bootBlockType();
}

- (NSString *)bootBlockName
{
    const char *str = [self file]->bootBlockName();
    return str ? @(str) : nullptr;
}

- (BOOL)hasVirus
{
    return [self file]->hasVirus();
}

- (void)killVirus
{
    [self file]->killVirus();
}

@end


//
// ADFFile proxy
//

@implementation ADFFileProxy

- (ADFFile *)adf
{
    return (ADFFile *)obj;
}

+ (instancetype)make:(ADFFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self make: new ADFFile([path fileSystemRepresentation])]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new ADFFile((const u8 *)buf, len)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithDrive:(FloppyDriveProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self make: new ADFFile(*[proxy drive]->drive)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithDiameter:(Diameter)dia density:(Density)den exception:(ExceptionWrapper *)ex
{
    try { return [self make: new ADFFile(dia, den)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

- (void)format:(FSVolumeType)fs bootBlock:(NSInteger)bb name:(NSString *)name
{
    auto str = string([name UTF8String]);
    [self adf]->formatDisk(fs, bb, str);
}

@end


//
// HDFFileProxy
//

@implementation HDFFileProxy

- (HDFFile *)hdf
{
    return (HDFFile *)obj;
}

+ (instancetype)make:(HDFFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self make: new HDFFile([path fileSystemRepresentation])]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new HDFFile((const u8 *)buf, len)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithHardDrive:(HardDriveProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self make: new HDFFile(*[proxy drive]->drive)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

- (BOOL)hasRDB
{
    return [self hdf]->hasRDB();
}

- (NSInteger)numPartitions
{
    return [self hdf]->numPartitions();
}

- (NSInteger)numDrivers
{
    return [self hdf]->numDrivers();
}

- (NSInteger)writeToFile:(NSString *)path partition:(NSInteger)nr exception:(ExceptionWrapper *)ex
{
    try { return [self hdf]->writePartitionToFile([path fileSystemRepresentation], nr); }
    catch (Error &error) { [ex save:error]; return 0; }
}

@end


//
// EADFFileProxy
//

@implementation EADFFileProxy

- (EADFFile *)ext
{
    return (EADFFile *)obj;
}

+ (instancetype)make:(EADFFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self make: new EADFFile([path fileSystemRepresentation])]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new EADFFile((const u8 *)buf, len)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithDrive:(FloppyDriveProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self make: new EADFFile(*[proxy drive]->drive)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

@end


//
// IMGFileProxy
//

@implementation IMGFileProxy

- (IMGFile *)img
{
    return (IMGFile *)obj;
}

+ (instancetype)make:(IMGFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self make: new IMGFile([path fileSystemRepresentation])]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new IMGFile((const u8 *)buf, len)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithDrive:(FloppyDriveProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self make: new IMGFile(*[proxy drive]->drive->disk)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

@end


//
// STFileProxy
//

@implementation STFileProxy

- (STFile *)img
{
    return (STFile *)obj;
}

+ (instancetype)make:(STFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self make: new STFile([path fileSystemRepresentation])]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new STFile((const u8 *)buf, len)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithDrive:(FloppyDriveProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self make: new STFile(*[proxy drive]->drive->disk)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

@end


//
// DMSFileProxy
//

@implementation DMSFileProxy

- (DMSFile *)dms
{
    return (DMSFile *)obj;
}

+ (instancetype)make:(DMSFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self make: new DMSFile([path fileSystemRepresentation])]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new DMSFile((const u8 *)buf, len)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

@end


//
// EXEFileProxy
//

@implementation EXEFileProxy

- (EXEFile *)exe
{
    return (EXEFile *)obj;
}

+ (instancetype)make:(EXEFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self make: new EXEFile([path fileSystemRepresentation])]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new EXEFile((const u8 *)buf, len)]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

@end


//
// Folder Proxy
//

@implementation FolderProxy

- (Folder *)dir
{
    return (Folder *)obj;
}

+ (instancetype)make:(Folder *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self make: new Folder([path fileSystemRepresentation])]; }
    catch (Error &error) { [ex save:error]; return nil; }
}

- (ADFFileProxy *)adf
{
    return [ADFFileProxy make:[self dir]->adf];
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
    return [[DefaultsProxy alloc] initWith:&Emulator::defaults];
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

- (NSInteger)autoInspectionMask
{
    return [self emu]->emu->main.getAutoInspectionMask();
}

- (void)setAutoInspectionMask:(NSInteger)mask
{
    [self emu]->emu->main.setAutoInspectionMask(mask);
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
    [self emu]->emu->stepInto();
}

- (void)stepOver
{
    [self emu]->emu->stepOver();
}

- (SnapshotProxy *)takeSnapshot
{
    Amiga *amiga = (Amiga *)[self amiga]->obj;
    auto *snapshot = amiga->takeSnapshot();
    return [SnapshotProxy make:snapshot];
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

- (void)loadSnapshot:(SnapshotProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { [self emu]->emu->main.loadSnapshot(*[proxy snapshot]); }
    catch (Error &error) { [ex save:error]; }
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
