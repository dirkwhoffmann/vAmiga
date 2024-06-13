// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#import "config.h"
#import "AmigaProxy.h"
#import "Amiga.h"
#import "vAmiga-Swift.h"
#import "DMSFile.h"
#import "EXEFile.h"
#import "ExtendedRomFile.h"
#import "EADFFile.h"
#import "Folder.h"
#import "MutableFileSystem.h"
#import "IMGFile.h"
#import "RomFile.h"
#import "Script.h"
#import "Snapshot.h"
#import "STFile.h"
#import "VAmiga.h"
#import "Emulator.h"

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
// CoreComponent proxy
//

@implementation CoreComponentProxy

-(CoreComponent *)component
{
    return (CoreComponent *)obj;
}

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
    auto result = [self props]->getString([key UTF8String]);
    return @(result.c_str());
}

- (NSInteger)getInt:(NSString *)key
{
    return [self props]->getInt([key UTF8String]);
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
    [self props]->setString(string([key UTF8String]), string([value UTF8String]));
}

- (void)setOpt:(Option)option value:(NSInteger)value
{
    [self props]->set(option, value);
}

- (void)setOpt:(Option)option nr:(NSInteger)nr value:(NSInteger)value
{
    [self props]->set(option, nr, value);
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
    [self props]->remove(option, nr);
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
    return [self guards]->guards->elements();
}

- (NSInteger)addr:(NSInteger)nr
{
    auto addr = [self guards]->guards->guardAddr(nr);
    return addr ? *addr : 0;
}

- (BOOL)isSet:(NSInteger)nr
{
    return [self guards]->guards->isSet(nr);
}

- (BOOL)isSetAt:(NSInteger)addr
{
    return [self guards]->guards->isSetAt(u32(addr));
}

- (void)setAt:(NSInteger)addr
{
    [self guards]->guards->setAt((u32)addr);
}

- (void)remove:(NSInteger)nr
{
    return [self guards]->guards->remove(nr);
}

- (void)removeAt:(NSInteger)addr
{
    [self guards]->guards->removeAt((u32)addr);
}

- (void)removeAll
{
    return [self guards]->guards->removeAll();
}

- (void)replace:(NSInteger)nr addr:(NSInteger)addr
{
    [self guards]->guards->replace(nr, (u32)addr);
}

- (BOOL)isEnabled:(NSInteger)nr
{
    return [self guards]->guards->isEnabled(nr);
}

- (BOOL)isEnabledAt:(NSInteger)addr
{
    return [self guards]->guards->isEnabledAt(u32(addr));
}

- (BOOL)isDisabled:(NSInteger)nr
{
    return [self guards]->guards->isDisabled(nr);
}

- (BOOL)isDisabledAt:(NSInteger)addr
{
    return [self guards]->guards->isDisabledAt(u32(addr));
}

- (void)enable:(NSInteger)nr
{
    [self guards]->guards->enable(nr);
}

- (void)enableAt:(NSInteger)addr
{
    [self guards]->guards->enableAt((u32)addr);
}

- (void)disable:(NSInteger)nr
{
    [self guards]->guards->disable(nr);
}

- (void)disableAt:(NSInteger)addr
{
    [self guards]->guards->disableAt((u32)addr);
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
    return [self cia]->cia->getInfo();
}

- (CIAInfo)cachedInfo
{
    return [self cia]->cia->getCachedInfo();
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

- (MemoryConfig)config
{
    return [self mem]->mem->getConfig();
}

- (MemoryStats)getStats
{
    return [self mem]->mem->getStats();
}

- (BOOL)isBootRom:(u32)crc32
{
    return RomFile::isBootRom(crc32);
}

- (BOOL)isArosRom:(u32)crc32
{
    return RomFile::isArosRom(crc32);
}

- (BOOL)isDiagRom:(u32)crc32
{
    return RomFile::isDiagRom(crc32);
}

- (BOOL)isCommodoreRom:(u32)crc32
{
    return RomFile::isCommodoreRom(crc32);
}

- (BOOL)isHyperionRom:(u32)crc32
{
    return RomFile::isHyperionRom(crc32);
}

- (BOOL) isEmutosRom:(u32)crc32
{
    return RomFile::isEmutosRom(crc32);
}

- (BOOL)isPatchedRom:(u32)crc32
{
    return RomFile::isPatchedRom(crc32);
}

- (NSString *) romTitleOf:(u32)crc32
{
    const char *str = RomFile::title(crc32);
    return str ? @(str) : nullptr;
}

- (BOOL)hasRom
{
    return [self mem]->mem->hasKickRom();
}

- (BOOL)hasBootRom
{
    return [self mem]->mem->hasBootRom();
}

- (BOOL)hasKickRom
{
    return [self mem]->mem->hasKickRom();
}

- (void)deleteRom
{
    [self mem]->mem->deleteRom();
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

- (BOOL)isRelocated
{
    return [self mem]->mem->isRelocated();
}

- (u32)romFingerprint
{
    return [self mem]->mem->romFingerprint();
}

- (NSString *)romTitle
{
    const char *str = [self mem]->mem->romTitle();
    return str ? @(str) : nullptr;
}

- (NSString *)romVersion
{
    const char *str = [self mem]->mem->romVersion();
    return str ? @(str) : nullptr;
}

- (NSString *)romReleased
{
    const char *str = [self mem]->mem->romReleased();
    return str ? @(str) : nullptr;
}

- (NSString *)romModel
{
    const char *str = [self mem]->mem->romModel();
    return str ? @(str) : nullptr;
}

- (BOOL)hasExt
{
    return [self mem]->mem->hasExt();
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

- (u32)extFingerprint
{
    return [self mem]->mem->extFingerprint();
}

- (NSString *)extTitle
{
    const char *str = [self mem]->mem->extTitle();
    return str ? @(str) : nullptr;
}

- (NSString *)extVersion
{
    const char *str = [self mem]->mem->extVersion();
    return str ? @(str) : nullptr;
}

- (NSString *)extReleased
{
    const char *str = [self mem]->mem->extReleased();
    return str ? @(str) : nullptr;
}

- (NSString *)extModel
{
    const char *str = [self mem]->mem->extModel();
    return str ? @(str) : nullptr;
}

- (NSInteger)extStart
{
    return [self mem]->mem->getConfigItem(OPT_EXT_START);
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

@end


//
// Agnus proxy
//

@implementation AgnusProxy

- (AgnusAPI *)agnus
{
    return (AgnusAPI *)obj;
}

- (NSInteger)chipRamLimit
{
    return [self agnus]->agnus->chipRamLimit();
}

- (AgnusInfo)info
{
    return [self agnus]->agnus->getInfo();
}

- (AgnusInfo)cachedInfo
{
    return [self agnus]->agnus->getCachedInfo();
}

- (EventInfo)eventInfo
{
    return [self agnus]->agnus->getEventInfo();
}

- (EventSlotInfo)getEventSlotInfo:(NSInteger)slot
{
    return [self agnus]->agnus->getSlotInfo(slot);
}

- (BOOL)isOCS
{
    return [self agnus]->agnus->isOCS();
}

- (BOOL)isECS
{
    return [self agnus]->agnus->isECS();
}

- (BOOL)isPAL
{
    return [self agnus]->agnus->isPAL();
}

- (BOOL)isNTSC
{
    return [self agnus]->agnus->isNTSC();
}

- (NSInteger)frameCount
{
    return [self agnus]->agnus->pos.frame;
}

- (AgnusStats)getStats
{
    return [self agnus]->agnus->getStats();
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
    return [self copper]->copper->getInfo();
}

- (CopperInfo)cachedInfo
{
    return [self copper]->copper->getCachedInfo();
}

- (BOOL)isIllegalInstr:(NSInteger)addr
{
    return [self copper]->copper->isIllegalInstr((u32)addr);
}

- (NSString *)disassemble:(NSInteger)addr symbolic:(BOOL)sym
{
    string str = [self copper]->copper->debugger.disassemble((u32)addr, sym);
    return @(str.c_str());
}

- (NSString *)disassemble:(NSInteger)list instr:(NSInteger)offset symbolic:(BOOL)sym
{
    string str = [self copper]->copper->debugger.disassemble(list, offset, sym);
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
    return [self blitter]->blitter->getInfo();
}

- (BlitterInfo)cachedInfo
{
    return [self blitter]->blitter->getCachedInfo();
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
    return [self debugger]->dmaDebugger->getInfo();
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
    return [self denise]->denise->getInfo();
}

- (DeniseInfo)cachedInfo
{
    return [self denise]->denise->getCachedInfo();
}

- (SpriteInfo)getSpriteInfo:(NSInteger)nr
{
    return [self denise]->denise->debugger.getSpriteInfo(nr);
}

- (NSInteger)sprDataLines:(NSInteger)nr
{
    return [self denise]->denise->debugger.getSpriteHeight(nr);
}

- (u64)sprData:(NSInteger)nr line:(NSInteger)line
{
    return [self denise]->denise->debugger.getSpriteData(nr, line);
}

- (u16)sprColor:(NSInteger)nr reg:(NSInteger)reg
{
    return [self denise]->denise->debugger.getSpriteColor(nr, reg);
}

- (u32 *)noise
{
    return (u32 *)([self denise]->denise->pixelEngine.getNoise());
}

- (void)getStableBuffer:(u32 **)ptr nr:(NSInteger *)nr lof:(bool *)lof prevlof:(bool *)prevlof
{
    auto &frameBuffer = [self denise]->denise->pixelEngine.getStableBuffer();
    *ptr = frameBuffer.pixels.ptr;
    *nr = NSInteger(frameBuffer.nr);
    *lof = frameBuffer.lof;
    *prevlof = frameBuffer.prevlof;
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
    return [self paula]->paula->getInfo();
}

- (PaulaInfo)cachedInfo
{
    return [self paula]->paula->getCachedInfo();
}

- (StateMachineInfo)audioInfo0
{
    return [self paula]->paula->channel0.getInfo();
}

- (StateMachineInfo)audioInfo1
{
    return [self paula]->paula->channel1.getInfo();
}

- (StateMachineInfo)audioInfo2
{
    return [self paula]->paula->channel2.getInfo();
}

- (StateMachineInfo)audioInfo3
{
    return [self paula]->paula->channel3.getInfo();
}

- (UARTInfo)uartInfo
{
    return [self paula]->paula->uart.getInfo();
}

- (UARTInfo)cachedUartInfo
{
    return [self paula]->paula->uart.getCachedInfo();
}

- (MuxerStats)muxerStats
{
    return [self paula]->paula->muxer.getStats();
}

- (void)readMonoSamples:(float *)target size:(NSInteger)n
{
    [self paula]->paula->muxer.copy(target, n);
}

- (void)readStereoSamples:(float *)target1 buffer2:(float *)target2 size:(NSInteger)n
{
    [self paula]->paula->muxer.copy(target1, target2, n);
}

- (void)rampUp
{
    [self paula]->paula->muxer.rampUp();
}

- (void)rampUpFromZero
{
    [self paula]->paula->muxer.rampUpFromZero();
}

- (void)rampDown
{
    [self paula]->paula->muxer.rampDown();
}

- (float)drawWaveformL:(u32 *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(u32)c
{
    return [self paula]->paula->muxer.stream.draw(buffer, w, h, true, s, c);
}

- (float)drawWaveformL:(u32 *)buffer size:(NSSize)size scale:(float)s color:(u32)c
{
    return [self drawWaveformL:buffer
                             w:(NSInteger)size.width
                             h:(NSInteger)size.height
                         scale:s
                         color:c];
}

- (float)drawWaveformR:(u32 *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(u32)c
{
    return [self paula]->paula->muxer.stream.draw(buffer, w, h, false, s, c);
}

- (float)drawWaveformR:(u32 *)buffer size:(NSSize)size scale:(float)s color:(u32)c
{
    return [self drawWaveformR:buffer
                             w:(NSInteger)size.width
                             h:(NSInteger)size.height
                         scale:s
                         color:c];
}

@end


//
// Rtc proxy
//

@implementation RtcProxy

- (RtcAPI *)rtc
{
    return (RtcAPI *)obj;
}

- (void)update
{
    [self rtc]->rtc->update();
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
    return [self mouse]->mouse->detectShakeXY(pos.x, pos.y);
}

- (BOOL)detectShakeRel:(NSPoint)pos
{
    return [self mouse]->mouse->detectShakeDxDy(pos.x, pos.y);
}

- (void)setXY:(NSPoint)pos
{
    [self mouse]->mouse->setXY((double)pos.x, (double)pos.y);
}

- (void)setDxDy:(NSPoint)pos
{
    [self mouse]->mouse->setDxDy((double)pos.x, (double)pos.y);
}

- (void)trigger:(GamePadAction)event
{
    [self mouse]->mouse->trigger(event);
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
    return [self cp]->controlPort->getInfo();
}

- (ControlPortInfo)cachedInfo
{
    return [self cp]->controlPort->getCachedInfo();
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

- (BOOL)keyIsPressed:(NSInteger)keycode
{
    return [self kb]->keyboard->keyIsPressed((KeyCode)keycode);
}

- (void)pressKey:(NSInteger)keycode
{
    [self kb]->keyboard->pressKey((KeyCode)keycode);
}

- (void)releaseKey:(NSInteger)keycode
{
    [self kb]->keyboard->releaseKey((KeyCode)keycode);
}

- (void)toggleKey:(NSInteger)keycode
{
    [self kb]->keyboard->toggleKey((KeyCode)keycode);
}

- (void)releaseAllKeys
{
    [self kb]->keyboard->releaseAllKeys();
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

- (DiskControllerConfig)getConfig
{
    return [self dc]->diskController->getConfig();
}

- (DiskControllerInfo)info
{
    return [self dc]->diskController->getInfo();
}

- (NSInteger)selectedDrive
{
    return [self dc]->diskController->getSelected();
}

- (DriveState)state
{
    return [self dc]->diskController->getState();
}

- (BOOL)isSpinning
{
    return [self dc]->diskController->spinning();
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

- (NSInteger)nr
{
    return [self drive]->drive->getNr();
}

- (BOOL)isConnected
{
    return [self drive]->drive->isConnected();
}

- (NSInteger)currentCyl
{
    return [self drive]->drive->currentCyl();
}

- (NSInteger)currentHead
{
    return [self drive]->drive->currentHead();
}

- (NSInteger)currentOffset
{
    return [self drive]->drive->currentOffset();
}

- (BOOL)hasDisk
{
    return [self drive]->drive->hasDisk();
}

- (BOOL)hasModifiedDisk
{
    return [self drive]->drive->hasModifiedDisk();
}

- (BOOL)hasProtectedDisk
{
    return [self drive]->drive->hasProtectedDisk();
}

- (BOOL)hasUnmodifiedDisk
{
    return [self drive]->drive->hasUnmodifiedDisk();
}

- (BOOL)hasUnprotectedDisk
{
    return [self drive]->drive->hasUnprotectedDisk();
}

- (void)setModificationFlag:(BOOL)value
{
    [self drive]->drive->setModificationFlag(value);
}

- (void)setProtectionFlag:(BOOL)value
{
    [self drive]->drive->setProtectionFlag(value);
}

- (void)markDiskAsModified
{
    [self drive]->drive->markDiskAsModified();
}

- (void)markDiskAsUnmodified
{
    [self drive]->drive->markDiskAsUnmodified();
}

- (void)toggleWriteProtection
{
    [self drive]->drive->toggleWriteProtection();
}

- (FloppyDriveInfo)info
{
    return [self drive]->drive->getInfo();
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

- (BOOL)motor
{
    return [self drive]->drive->getMotor();
}

- (BOOL)selected
{
    return [self drive]->drive->isSelected();
}

- (BOOL)writing
{
    return [self drive]->drive->isWriting();
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

- (HardDriveAPI *)drive
{
    return (HardDriveAPI *)obj;
}

- (NSInteger)nr
{
    return [self drive]->drive->getNr();
}

- (BOOL)isConnected
{
    return [self drive]->drive->isConnected();
}

- (NSInteger)currentCyl
{
    return [self drive]->drive->currentCyl();
}

- (NSInteger)currentHead
{
    return [self drive]->drive->currentHead();
}

- (NSInteger)currentOffset
{
    return [self drive]->drive->currentOffset();
}

- (BOOL)hasDisk
{
    return [self drive]->drive->hasDisk();
}

- (BOOL)hasModifiedDisk
{
    return [self drive]->drive->hasModifiedDisk();
}

- (BOOL)hasProtectedDisk
{
    return [self drive]->drive->hasProtectedDisk();
}

- (BOOL)hasUnmodifiedDisk
{
    return [self drive]->drive->hasUnmodifiedDisk();
}

- (BOOL)hasUnprotectedDisk
{
    return [self drive]->drive->hasUnprotectedDisk();
}

- (void)setModificationFlag:(BOOL)value
{
    [self drive]->drive->setModificationFlag(value);
}

- (void)setProtectionFlag:(BOOL)value
{
    [self drive]->drive->setProtectionFlag(value);
}

- (void)markDiskAsModified
{
    [self drive]->drive->markDiskAsModified();
}

- (void)markDiskAsUnmodified
{
    [self drive]->drive->markDiskAsUnmodified();
}

- (void)toggleWriteProtection
{
    [self drive]->drive->toggleWriteProtection();
}

- (HardDriveInfo)info
{
    return [self drive]->drive->getInfo();
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

- (HdcState)hdcState
{
    return [self drive]->drive->getHdcState();
}

- (BOOL)isCompatible
{
    return [self drive]->drive->isCompatible();
}

- (BOOL)writeThroughEnabled
{
    return [self drive]->drive->writeThroughEnabled();
}

- (NSString *)nameOfPartition:(NSInteger)nr
{
    auto &info = [self drive]->drive->getPartitionInfo(nr);
    return @(info.name.c_str());
}

- (NSInteger)lowerCylOfPartition:(NSInteger)nr
{
    auto &info = [self drive]->drive->getPartitionInfo(nr);
    return info.lowCyl;
}

- (NSInteger)upperCylOfPartition:(NSInteger)nr
{
    auto &info = [self drive]->drive->getPartitionInfo(nr);
    return info.highCyl;
}

- (HardDriveState)state
{
    return [self drive]->drive->getState();
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
        [self drive]->drive->changeGeometry(c, h, s, b);
    }  catch (Error &error) {
        [ex save:error];
    }
}

- (NSMutableArray *)geometries
{
    NSMutableArray *data = [[NSMutableArray alloc] init];
    
    auto geometry = [self drive]->drive->getGeometry();
    auto geometries = GeometryDescriptor::driveGeometries(geometry.numBlocks());
        
    for (auto &g : geometries) {
        
        NSInteger encoded = g.cylinders << 32 | g.heads << 16 | g.sectors;
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
// Debugger proxy
//

@implementation DebuggerProxy

- (vamiga::DebuggerAPI *)debugger
{
    return (vamiga::DebuggerAPI *)obj;
}

+ (instancetype)make:(vamiga::DebuggerAPI *)object
{
    if (object == nullptr) { return nil; }

    DebuggerProxy *proxy = [[self alloc] initWith: object];
    return proxy;
}

- (void)stopAndGo
{
    [self debugger]->debugger->stopAndGo();
}

- (void)stepInto
{
    [self debugger]->debugger->stepInto();
}

- (void)stepOver
{
    [self debugger]->debugger->stepOver();
}

- (NSString *)ascDump:(Accessor)accessor addr:(NSInteger)addr bytes:(NSInteger)bytes
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    const char *str;

    if (accessor == ACCESSOR_CPU) {
        str = [self debugger]->debugger->ascDump <ACCESSOR_CPU> ((u32)addr, bytes);
    } else {
        str = [self debugger]->debugger->ascDump <ACCESSOR_AGNUS> ((u32)addr, bytes);
    }

    return str ? @(str) : nullptr;
}

- (NSString *)hexDump:(Accessor)accessor addr: (NSInteger)addr bytes:(NSInteger)bytes
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    const char *str;

    if (accessor == ACCESSOR_CPU) {
        str = [self debugger]->debugger->hexDump <ACCESSOR_CPU> ((u32)addr, bytes);
    } else {
        str = [self debugger]->debugger->hexDump <ACCESSOR_AGNUS> ((u32)addr, bytes);
    }

    return str ? @(str) : nullptr;
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
    return [self shell]->retroShell->cursorRel();
}

-(NSString *)getText
{
    const char *str = [self shell]->retroShell->text();
    return str ? @(str) : nullptr;
}

- (void)pressUp
{
    [self shell]->retroShell->press(RSKEY_UP);
}

- (void)pressDown
{
    [self shell]->retroShell->press(RSKEY_DOWN);
}

- (void)pressLeft
{
    [self shell]->retroShell->press(RSKEY_LEFT);
}

- (void)pressRight
{
    [self shell]->retroShell->press(RSKEY_RIGHT);
}

- (void)pressHome
{
    [self shell]->retroShell->press(RSKEY_HOME);
}

- (void)pressEnd
{
    [self shell]->retroShell->press(RSKEY_END);
}

- (void)pressBackspace
{
    [self shell]->retroShell->press(RSKEY_BACKSPACE);
}

- (void)pressDelete
{
    [self shell]->retroShell->press(RSKEY_DEL);
}

- (void)pressCut
{
    [self shell]->retroShell->press(RSKEY_CUT);
}

- (void)pressReturn
{
    [self shell]->retroShell->press(RSKEY_RETURN);
}

- (void)pressShiftReturn
{
    [self shell]->retroShell->press(RSKEY_RETURN, true);
}

- (void)pressTab
{
    [self shell]->retroShell->press(RSKEY_TAB);
}

- (void)pressKey:(char)c
{
    [self shell]->retroShell->press(c);
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

-(NSInteger)numLaunching
{
    return [self manager]->remoteManager->numLaunching();
}

-(NSInteger)numListening
{
    return [self manager]->remoteManager->numListening();
}

-(NSInteger)numConnected
{
    return [self manager]->remoteManager->numConnected();
}

-(NSInteger)numErroneous
{
    return [self manager]->remoteManager->numErroneous();
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
    return [self file]->size();
}

- (NSString *)sizeAsString
{
    const string &str = [self file]->sizeAsString();
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

+ (instancetype)make:(Snapshot *)snapshot
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
    Amiga *amiga = (Amiga *)proxy->obj;
    
    amiga->suspend();
    Snapshot *snapshot = new Snapshot(*amiga);
    amiga->resume();
    
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

- (void)execute:(AmigaProxy *)proxy
{
    Amiga *amiga = (Amiga *)proxy->obj;
    
    [self script]->execute(*amiga);
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
// HostProxy
//

@implementation HostProxy

- (HostAPI *)host
{
    return (HostAPI *)obj;
}

+ (instancetype)make:(Host *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

- (double)sampleRate
{
    return [self host]->host->getSampleRate();
}

- (void)setSampleRate:(double)hz
{
    [self host]->host->setSampleRate(hz);
}

- (NSInteger)refreshRate
{
    return (NSInteger)[self host]->host->getHostRefreshRate();
}

- (void)setRefreshRate:(NSInteger)value
{
    [self host]->host->setHostRefreshRate((double)value);
}

- (NSSize)frameBufferSize
{
    auto size = [self host]->host->getFrameBufferSize();
    return NSMakeSize((CGFloat)size.first, (CGFloat)size.second);
}

- (void)setFrameBufferSize:(NSSize)size
{
    [self host]->host->setFrameBufferSize(std::pair<isize, isize>(size.width, size.height));
}

@end


//
// AmigaProxy
//

@implementation AmigaProxy

@synthesize agnus;
@synthesize blitter;
@synthesize breakpoints;
@synthesize ciaA;
@synthesize ciaB;
@synthesize controlPort1;
@synthesize controlPort2;
@synthesize copper;
@synthesize copperBreakpoints;
@synthesize cpu;
@synthesize debugger;
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
@synthesize host;
@synthesize keyboard;
@synthesize mem;
@synthesize paula;
@synthesize remoteManager;
@synthesize retroShell;
@synthesize rtc;
@synthesize serialPort;
@synthesize recorder;
@synthesize watchpoints;

- (instancetype) init
{
    if (!(self = [super init]))
        return self;

    // Create the emulator instance
    auto vamiga = new VAmiga();
    Amiga *amiga = &vamiga->emu->main;
    obj = amiga;

    // Create sub proxys
    agnus = [[AgnusProxy alloc] initWith:&vamiga->agnus];
    blitter = [[BlitterProxy alloc] initWith:&vamiga->blitter];
    breakpoints = [[GuardsProxy alloc] initWith:&vamiga->breakpoints];
    ciaA = [[CIAProxy alloc] initWith:&vamiga->ciaA];
    ciaB = [[CIAProxy alloc] initWith:&vamiga->ciaB];
    controlPort1 = [[ControlPortProxy alloc] initWith:&vamiga->controlPort1];
    controlPort2 = [[ControlPortProxy alloc] initWith:&vamiga->controlPort2];
    copper = [[CopperProxy alloc] initWith:&vamiga->copper];
    copperBreakpoints = [[GuardsProxy alloc] initWith:&vamiga->copperBreakpoints];
    cpu = [[CPUProxy alloc] initWith:&vamiga->cpu];
    debugger = [[DebuggerProxy alloc] initWith:&vamiga->debugger];
    denise = [[DeniseProxy alloc] initWith:&vamiga->denise];
    diskController = [[DiskControllerProxy alloc] initWith:&vamiga->diskController];
    dmaDebugger = [[DmaDebuggerProxy alloc] initWith:&vamiga->dmaDebugger];
    df0 = [[FloppyDriveProxy alloc] initWith:&vamiga->df0];
    df1 = [[FloppyDriveProxy alloc] initWith:&vamiga->df1];
    df2 = [[FloppyDriveProxy alloc] initWith:&vamiga->df2];
    df3 = [[FloppyDriveProxy alloc] initWith:&vamiga->df3];
    hd0 = [[HardDriveProxy alloc] initWith:&vamiga->hd0];
    hd1 = [[HardDriveProxy alloc] initWith:&vamiga->hd1];
    hd2 = [[HardDriveProxy alloc] initWith:&vamiga->hd2];
    hd3 = [[HardDriveProxy alloc] initWith:&vamiga->hd3];
    host = [[HostProxy alloc] initWith:&vamiga->host];
    keyboard = [[KeyboardProxy alloc] initWith:&vamiga->keyboard];
    mem = [[MemProxy alloc] initWith:&vamiga->mem];
    paula = [[PaulaProxy alloc] initWith:&vamiga->paula];
    retroShell = [[RetroShellProxy alloc] initWith:&vamiga->retroShell];
    rtc = [[RtcProxy alloc] initWith:&vamiga->rtc];
    recorder = [[RecorderProxy alloc] initWith:&vamiga->recorder];
    remoteManager = [[RemoteManagerProxy alloc] initWith:&vamiga->remoteManager];
    serialPort = [[SerialPortProxy alloc] initWith:&vamiga->serialPort];
    watchpoints = [[GuardsProxy alloc] initWith:&vamiga->watchpoints];

    return self;
}

- (Amiga *)amiga
{
    return (Amiga *)obj;
}

+ (DefaultsProxy *) defaults
{
    return [[DefaultsProxy alloc] initWith:&Amiga::defaults];
}

- (void)dealloc
{
    NSLog(@"dealloc");
}

- (void)kill
{
    NSLog(@"kill");
    
    assert([self amiga] != NULL);
    delete [self amiga];
    obj = NULL;
}

- (AmigaInfo)info
{
    return [self amiga]->getInfo();
}

- (BOOL)isWarping
{
    return [self amiga]->isWarping();
}

- (BOOL)trackMode
{
    return [self amiga]->isTracking();
}

- (void)setTrackMode:(BOOL)value
{
    if (value) {
        [self amiga]->trackOn();
    } else {
        [self amiga]->trackOff();
    }
}

- (NSInteger)cpuLoad
{
    double load = [self amiga]->getCpuLoad();
    return (NSInteger)(100 * load);
}

- (InspectionTarget)inspectionTarget
{
    return [self amiga]->getInspectionTarget();
}

- (void)setInspectionTarget:(InspectionTarget)target
{
    [self amiga]->setInspectionTarget(target);
}

- (void) removeInspectionTarget
{
    [self amiga]->removeInspectionTarget();
}

- (void)launch:(const void *)listener function:(Callback *)func
{
    [self amiga]->launch(listener, func);
}

- (void)hardReset
{
    [self amiga]->reset(true);
}

- (void)softReset
{
    [self amiga]->reset(false);
}

- (BOOL)poweredOn
{
    return [self amiga]->isPoweredOn();
}

- (BOOL)poweredOff
{
    return [self amiga]->isPoweredOff();
}

- (BOOL)running
{
    return [self amiga]->isRunning();
}

- (BOOL)paused
{
    return [self amiga]->isPaused();
}

- (void)isReady:(ExceptionWrapper *)ex
{
    try { [self amiga]->isReady(); }
    catch (Error &error) { [ex save:error]; }
}

- (void)powerOn
{
    [self amiga]->powerOn();
}

- (void)powerOff
{
    [self amiga]->powerOff();
}

- (void)run:(ExceptionWrapper *)ex
{
    try { [self amiga]->run(); }
    catch (Error &error) { [ex save:error]; }
}

- (void)pause
{
    [self amiga]->pause();
}

- (void)halt
{
    [self amiga]->halt();
}

- (void)wakeUp
{
    [self amiga]->wakeUp();
}

- (void)suspend
{
    return [self amiga]->suspend();
}

- (void)resume
{
    return [self amiga]->resume();
}

- (void)continueScript
{
    [self amiga]->retroShell.continueScript();
}

- (void)requestAutoSnapshot
{
    [self amiga]->requestAutoSnapshot();
}

- (void)requestUserSnapshot
{
    [self amiga]->requestUserSnapshot();
}

- (SnapshotProxy *)latestAutoSnapshot
{
    Snapshot *snapshot = [self amiga]->latestAutoSnapshot();
    return [SnapshotProxy make:snapshot];
}

- (SnapshotProxy *)latestUserSnapshot
{
    Snapshot *snapshot = [self amiga]->latestUserSnapshot();
    return [SnapshotProxy make:snapshot];
}

- (void)loadSnapshot:(SnapshotProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { [self amiga]->loadSnapshot(*[proxy snapshot]); }
    catch (Error &error) { [ex save:error]; }
}

- (NSInteger)getConfig:(Option)opt
{
    return [self amiga]->getConfigItem(opt);
}

- (NSInteger)getConfig:(Option)opt id:(NSInteger)id
{
    return [self amiga]->getConfigItem(opt, id);
}

- (NSInteger)getConfig:(Option)opt drive:(NSInteger)id
{
    return [self amiga]->getConfigItem(opt, (long)id);
}

- (BOOL)configure:(Option)opt value:(NSInteger)val
{
    try {
        [self amiga]->configure(opt, val);
        return true;
    } catch (Error &exception) {
        return false;
    }
}

- (BOOL)configure:(Option)opt enable:(BOOL)val
{
    try {
        [self amiga]->configure(opt, val ? 1 : 0);
        return true;
    } catch (Error &exception) {
        return false;
    }
}

- (BOOL)configure:(Option)opt id:(NSInteger)id value:(NSInteger)val
{
    try {
        [self amiga]->configure(opt, id, val);
        return true;
    } catch (Error &exception) {
        return false;
    }
}

- (BOOL)configure:(Option)opt id:(NSInteger)id enable:(BOOL)val
{
    try {
        [self amiga]->configure(opt, id, val ? 1 : 0);
        return true;
    } catch (Error &exception) {
        return false;
    }
}

- (BOOL)configure:(Option)opt drive:(NSInteger)id value:(NSInteger)val
{
    try {
        [self amiga]->configure(opt, (long)id, val);
        return true;
    } catch (Error &exception) {
        return false;
    }
}

- (BOOL)configure:(Option)opt drive:(NSInteger)id enable:(BOOL)val
{
    try {
        [self amiga]->configure(opt, (long)id, val ? 1 : 0);
        return true;
    } catch (Error &exception) {
        return false;
    }
}

- (void)setListener:(const void *)sender function:(Callback *)func
{
    [self amiga]->msgQueue.setListener(sender, func);
}

- (void)setAlarmAbs:(NSInteger)cycle payload:(NSInteger)value
{
    [self amiga]->setAlarmAbs(cycle, value);
}

- (void)setAlarmRel:(NSInteger)cycle payload:(NSInteger)value
{
    [self amiga]->setAlarmRel(cycle, value);
}

@end
