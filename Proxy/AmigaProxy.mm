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
#import "EXTFile.h"
#import "Folder.h"
#import "MutableFileSystem.h"
#import "IMGFile.h"
#import "RomFile.h"
#import "Script.h"
#import "Snapshot.h"

using namespace moira;

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

- (void)save:(const VAError &)exception
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
// AmigaComponent proxy
//

@implementation AmigaComponentProxy

-(AmigaComponent *)component
{
    return (AmigaComponent *)obj;
}

@end

//
// Guards (Breakpoints, Watchpoints)
//

@implementation GuardsProxy

- (Guards *)guards
{
    return (Guards *)obj;
}

- (NSInteger)count
{
    return [self guards]->elements();
}

- (NSInteger)addr:(NSInteger)nr
{
    auto addr = [self guards]->guardAddr(nr);
    return addr ? *addr : 0;
}

- (BOOL)isSet:(NSInteger)nr
{
    return [self guards]->isSet(nr);
}

- (BOOL)isSetAt:(NSInteger)addr
{
    return [self guards]->isSetAt(u32(addr));
}

- (void)setAt:(NSInteger)addr
{
    [self guards]->setAt((u32)addr);
}

- (void)remove:(NSInteger)nr
{
    return [self guards]->remove(nr);
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
    [self guards]->replace(nr, (u32)addr);
}

- (BOOL)isEnabled:(NSInteger)nr
{
    return [self guards]->isEnabled(nr);
}

- (BOOL)isEnabledAt:(NSInteger)addr
{
    return [self guards]->isEnabledAt(u32(addr));
}

- (BOOL)isDisabled:(NSInteger)nr
{
    return [self guards]->isDisabled(nr);
}

- (BOOL)isDisabledAt:(NSInteger)addr
{
    return [self guards]->isDisabledAt(u32(addr));
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

- (CPU *)cpu
{
    return (CPU *)obj;
}

- (CPUInfo)info
{
    return [self cpu]->getInfo();
}

- (i64)clock
{
    return [self cpu]->getCpuClock();
}

- (BOOL)halted
{
    return [self cpu]->isHalted();
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
    const char *str = [self cpu]->disassembleRecordedInstr((int)i, &result);
    *len = (NSInteger)result;
    
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleRecordedBytes:(NSInteger)i length:(NSInteger)len
{
    const char *str = [self cpu]->disassembleRecordedWords(i, len);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleRecordedFlags:(NSInteger)i
{
    const char *str = [self cpu]->disassembleRecordedFlags((int)i);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleRecordedPC:(NSInteger)i
{
    const char *str = [self cpu]->disassembleRecordedPC((int)i);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleInstr:(NSInteger)addr length:(NSInteger *)len
{
    isize result;
    const char *str = [self cpu]->disassembleInstr((u32)addr, &result);
    *len = result;
    
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleWords:(NSInteger)addr length:(NSInteger)len
{
    const char *str = [self cpu]->disassembleWords((u32)addr, len);
    return str ? @(str) : nullptr;
}

- (NSString *)disassembleAddr:(NSInteger)addr
{
    const char *str = [self cpu]->disassembleAddr((u32)addr);
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

- (CIA *)cia
{
    return (CIA *)obj;
}

- (CIAInfo)info
{
    return [self cia]->getInfo();
}

@end


//
// Memory proxy
//

@implementation MemProxy

- (Memory *)mem
{
    return (Memory *)obj;
}

- (MemoryConfig)config
{
    return [self mem]->getConfig();
}

- (MemoryStats)getStats
{
    return [self mem]->getStats();
}

- (BOOL)isBootRom:(RomIdentifier)rev
{
    return RomFile::isBootRom(rev);
}

- (BOOL)isArosRom:(RomIdentifier)rev
{
    return RomFile::isArosRom(rev);
}

- (BOOL)isDiagRom:(RomIdentifier)rev
{
    return RomFile::isDiagRom(rev);
}

- (BOOL)isCommodoreRom:(RomIdentifier)rev
{
    return RomFile::isCommodoreRom(rev);
}

- (BOOL)isHyperionRom:(RomIdentifier)rev
{
    return RomFile::isHyperionRom(rev);
}

- (BOOL)isPatchedRom:(RomIdentifier)rev
{
    return RomFile::isPatchedRom(rev);
}

- (BOOL)hasRom
{
    return [self mem]->hasKickRom();
}

- (BOOL)hasBootRom
{
    return [self mem]->hasBootRom();
}

- (BOOL)hasKickRom
{
    return [self mem]->hasKickRom();
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
    try { return [self mem]->loadRom(*(RomFile *)proxy->obj); }
    catch (VAError &error) { [ex save:error]; }
}

- (void)loadRomFromBuffer:(NSData *)data exception:(ExceptionWrapper *)ex
{
    assert(data);
    const u8 *bytes = (const u8 *)[data bytes];
    
    try { return [self mem]->loadRom(bytes, [data length]); }
    catch (VAError &error) { [ex save:error]; }
}

- (void)loadRomFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->loadRom([url fileSystemRepresentation]); }
    catch (VAError &error) { [ex save:error]; }
}

- (u64)romFingerprint
{
    return [self mem]->romFingerprint();
}

- (RomIdentifier)romIdentifier
{
    return [self mem]->romIdentifier();
}

- (NSString *)romTitle
{
    const char *str = [self mem]->romTitle();
    return str ? @(str) : nullptr;
}

- (NSString *)romVersion
{
    const char *str = [self mem]->romVersion();
    return str ? @(str) : nullptr;
}

- (NSString *)romReleased
{
    const char *str = [self mem]->romReleased();
    return str ? @(str) : nullptr;
}

- (NSString *)romModel
{
    const char *str = [self mem]->romModel();
    return str ? @(str) : nullptr;
}

- (BOOL)hasExt
{
    return [self mem]->hasExt();
}

- (void)deleteExt
{
    [self mem]->deleteExt();
}

- (BOOL)isExt:(NSURL *)url
{
    return ExtendedRomFile::isExtendedRomFile([url fileSystemRepresentation]);
}

- (void)loadExt:(ExtendedRomFileProxy *)proxy
{
    [self mem]->loadExt(*(ExtendedRomFile *)proxy->obj);
}

- (void)loadExt:(ExtendedRomFileProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->loadExt(*(ExtendedRomFile *)proxy->obj); }
    catch (VAError &error) { [ex save:error]; }
}

- (void)loadExtFromBuffer:(NSData *)data exception:(ExceptionWrapper *)ex
{
    assert(data);
    const u8 *bytes = (const u8 *)[data bytes];
    
    try { return [self mem]->loadExt(bytes, [data length]); }
    catch (VAError &error) { [ex save:error]; }
}

- (void)loadExtFromFile:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->loadExt([url fileSystemRepresentation]); }
    catch (VAError &error) { [ex save:error]; }
}

- (u64)extFingerprint
{
    return [self mem]->extFingerprint();
}

- (RomIdentifier)extIdentifier
{
    return [self mem]->extIdentifier();
}

- (NSString *)extTitle
{
    const char *str = [self mem]->extTitle();
    return str ? @(str) : nullptr;
}

- (NSString *)extVersion
{
    const char *str = [self mem]->extVersion();
    return str ? @(str) : nullptr;
}

- (NSString *)extReleased
{
    const char *str = [self mem]->extReleased();
    return str ? @(str) : nullptr;
}

- (NSString *)extModel
{
    const char *str = [self mem]->extModel();
    return str ? @(str) : nullptr;
}

- (NSInteger)extStart
{
    return [self mem]->getConfigItem(OPT_EXT_START);
}

- (void)saveRom:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->saveRom([url fileSystemRepresentation]); }
    catch (VAError &error) { [ex save:error]; }
}

- (void)saveWom:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->saveWom([url fileSystemRepresentation]); }
    catch (VAError &error) { [ex save:error]; }
}

- (void)saveExt:(NSURL *)url exception:(ExceptionWrapper *)ex
{
    try { return [self mem]->saveExt([url fileSystemRepresentation]); }
    catch (VAError &error) { [ex save:error]; }
}

- (MemorySource)memSrc:(Accessor)accessor addr:(NSInteger)addr
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    
    if (accessor == ACCESSOR_CPU) {
        return [self mem]->getMemSrc <ACCESSOR_CPU> ((u32)addr);
    } else {
        return [self mem]->getMemSrc <ACCESSOR_AGNUS> ((u32)addr);
    }
}

- (NSInteger)spypeek16:(Accessor)accessor addr:(NSInteger)addr
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    
    if (accessor == ACCESSOR_CPU) {
        return [self mem]->spypeek16 <ACCESSOR_CPU> ((u32)addr);
    } else {
        return [self mem]->spypeek16 <ACCESSOR_AGNUS> ((u32)addr);
    }
}

- (NSString *)ascii:(Accessor)accessor addr:(NSInteger)addr
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    const char *str;

    if (accessor == ACCESSOR_CPU) {
        str = [self mem]->ascii <ACCESSOR_CPU> ((u32)addr);
    } else {
        str = [self mem]->ascii <ACCESSOR_AGNUS> ((u32)addr);
    }
    
    return str ? @(str) : nullptr;
}

- (NSString *)hex:(Accessor)accessor addr: (NSInteger)addr bytes:(NSInteger)bytes
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    const char *str;

    if (accessor == ACCESSOR_CPU) {
        str = [self mem]->hex <ACCESSOR_CPU> ((u32)addr, bytes);
    } else {
        str = [self mem]->hex <ACCESSOR_AGNUS> ((u32)addr, bytes);
    }
    
    return str ? @(str) : nullptr;
}

@end


//
// Agnus proxy
//

@implementation AgnusProxy

- (Agnus *)agnus
{
    return (Agnus *)obj;
}

- (NSInteger)chipRamLimit
{
    return [self agnus]->chipRamLimit();
}

- (AgnusInfo)info
{
    return [self agnus]->getInfo();
}

- (EventInfo)eventInfo
{
    return [self agnus]->getEventInfo();
}

- (EventSlotInfo)getEventSlotInfo:(NSInteger)slot
{
    return [self agnus]->getSlotInfo(slot);
}

- (BOOL)isOCS
{
    return [self agnus]->isOCS();
}

- (BOOL)isECS
{
    return [self agnus]->isECS();
}

- (AgnusStats)getStats
{
    return [self agnus]->getStats();
}

@end


//
// Copper proxy
//

@implementation CopperProxy

- (Copper *)copper
{
    return (Copper *)obj;
}

- (CopperInfo)info
{
    return [self copper]->getInfo();
}

- (BOOL)isIllegalInstr:(NSInteger)addr
{
    return [self copper]->isIllegalInstr((u32)addr);
}

- (NSString *)disassemble:(NSInteger)addr symbolic:(BOOL)sym
{
    string str = [self copper]->debugger.disassemble((u32)addr, sym);
    return @(str.c_str());
}

- (NSString *)disassemble:(NSInteger)list instr:(NSInteger)offset symbolic:(BOOL)sym
{
    string str = [self copper]->debugger.disassemble(list, offset, sym);
    return @(str.c_str());
}

@end


//
// Blitter
//

@implementation BlitterProxy

- (Blitter *)blitter
{
    return (Blitter *)obj;
}

- (BlitterInfo)info
{
    return [self blitter]->getInfo();
}

@end


//
// DMA Debugger
//

@implementation DmaDebuggerProxy

- (DmaDebugger *)debugger
{
    return (DmaDebugger *)obj;
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

- (Denise *)denise
{
    return (Denise *)obj;
}

- (DeniseInfo)info
{
    return [self denise]->getInfo();
}

- (SpriteInfo)getSpriteInfo:(NSInteger)nr
{
    return [self denise]->debugger.getSpriteInfo(nr);
}

- (NSInteger)sprDataLines:(NSInteger)nr
{
    return [self denise]->debugger.getSpriteHeight(nr);
}

- (u64)sprData:(NSInteger)nr line:(NSInteger)line
{
    return [self denise]->debugger.getSpriteData(nr, line);
}

- (u16)sprColor:(NSInteger)nr reg:(NSInteger)reg
{
    return [self denise]->debugger.getSpriteColor(nr, reg);
}

- (void)lockStableBuffer
{
    [self denise]->pixelEngine.lockStableBuffer();
}

- (void)unlockStableBuffer
{
    [self denise]->pixelEngine.unlockStableBuffer();
}

- (BOOL)longFrame
{
    return [self denise]->pixelEngine.getStableBuffer().longFrame;
}

- (u32 *)stableBuffer
{
    return [self denise]->pixelEngine.getStableBuffer().ptr;
}

- (u32 *)noise
{
    return [self denise]->pixelEngine.getNoise();
}

@end


//
// Recorder
//

@implementation RecorderProxy

- (Recorder *)recorder
{
    return (Recorder *)obj;
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
    return [self recorder]->isRecording();
}

- (double)duration
{
    return [self recorder]->getDuration().asSeconds();
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
    int x1 = (int)rect.origin.x;
    int y1 = (int)rect.origin.y;
    int x2 = x1 + (int)rect.size.width;
    int y2 = y1 + (int)rect.size.height;
    
    try { return [self recorder]->startRecording(x1, y1, x2, y2, rate, aspectX, aspectY); }
    catch (VAError &error) { [ex save:error]; }
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

- (Paula *)paula
{
    return (Paula *)obj;
}

- (PaulaInfo)info
{
    return [self paula]->getInfo();
}

- (StateMachineInfo)audioInfo0
{
    return [self paula]->channel0.getInfo();
}

- (StateMachineInfo)audioInfo1
{
    return [self paula]->channel1.getInfo();
}

- (StateMachineInfo)audioInfo2
{
    return [self paula]->channel2.getInfo();
}

- (StateMachineInfo)audioInfo3
{
    return [self paula]->channel3.getInfo();
}

- (UARTInfo)uartInfo
{
    return [self paula]->uart.getInfo();
}

- (MuxerStats)muxerStats
{
    return [self paula]->muxer.getStats();
}

- (double)sampleRate
{
    return [self paula]->muxer.getSampleRate();
}

- (void)setSampleRate:(double)rate
{
    [self paula]->muxer.setSampleRate(rate);
}

- (void)readMonoSamples:(float *)target size:(NSInteger)n
{
    [self paula]->muxer.copy(target, n);
}

- (void)readStereoSamples:(float *)target1 buffer2:(float *)target2 size:(NSInteger)n
{
    [self paula]->muxer.copy(target1, target2, n);
}

- (void)rampUp
{
    [self paula]->muxer.rampUp();
}

- (void)rampUpFromZero
{
    [self paula]->muxer.rampUpFromZero();
}

- (void)rampDown
{
    [self paula]->muxer.rampDown();
}

- (float)drawWaveformL:(u32 *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(u32)c
{
    return [self paula]->muxer.stream.draw(buffer, w, h, true, s, c);
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
    return [self paula]->muxer.stream.draw(buffer, w, h, false, s, c);
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

- (RTC *)rtc
{
    return (RTC *)obj;
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

- (Mouse *)mouse
{
    return (Mouse *)obj;
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
    [self mouse]->setXY((double)pos.x, (double)pos.y);
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

- (Joystick *)joystick
{
    return (Joystick *)obj;
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
        
        ControlPort *port = (ControlPort *)ref;
        obj = ref;
        joystick = [[JoystickProxy alloc] initWith:&port->joystick];
        mouse = [[MouseProxy alloc] initWith:&port->mouse];
    }
    return self;
}

- (ControlPort *)cp
{
    return (ControlPort *)obj;
}

- (ControlPortInfo)info
{
    return [self cp]->getInfo();
}

@end


//
// SerialPort proxy
//

@implementation SerialPortProxy

- (SerialPort *)serial
{
    return (SerialPort *)obj;
}

- (SerialPortInfo)info
{
    return [self serial]->getInfo();
}

@end


//
// Keyboard
//

@implementation KeyboardProxy

- (Keyboard *)kb
{
    return (Keyboard *)obj;
}

- (BOOL)keyIsPressed:(NSInteger)keycode
{
    return [self kb]->keyIsPressed((KeyCode)keycode);
}

- (void)pressKey:(NSInteger)keycode
{
    [self kb]->pressKey((KeyCode)keycode);
}

- (void)releaseKey:(NSInteger)keycode
{
    [self kb]->releaseKey((KeyCode)keycode);
}

- (void)toggleKey:(NSInteger)keycode
{
    [self kb]->toggleKey((KeyCode)keycode);
}

- (void)releaseAllKeys
{
    [self kb]->releaseAllKeys();
}

@end


//
// Disk Controller
//

@implementation DiskControllerProxy

- (DiskController *)dc
{
    return (DiskController *)obj;
}

- (DiskControllerConfig)getConfig
{
    return [self dc]->getConfig();
}

- (DiskControllerInfo)info
{
    return [self dc]->getInfo();
}

- (NSInteger)selectedDrive
{
    return [self dc]->getSelected();
}

- (DriveState)state
{
    return [self dc]->getState();
}

- (BOOL)isSpinning
{
    return [self dc]->spinning();
}

@end


//
// Drive proxy
//

@implementation DriveProxy

- (Drive *)drive
{
    return (Drive *)obj;
}

- (NSInteger)nr
{
    return [self drive]->getNr();
}

- (BOOL)isConnected
{
    return [self drive]->isConnected();
}

- (NSInteger)currentCyl
{
    return [self drive]->currentCyl();
}

- (NSInteger)currentHead
{
    return [self drive]->currentHead();
}

- (NSInteger)currentOffset
{
    return [self drive]->currentOffset();
}

- (u64)fnv
{
    return [self drive]->fnv();
}

- (BOOL)hasDisk
{
    return [self drive]->hasDisk();
}

- (BOOL)modified
{
    return [self drive]->hasModifiedDisk();
}

- (BOOL)hasProtectedDisk
{
    return [self drive]->hasProtectedDisk();
}

- (BOOL)hasUnmodifiedDisk
{
    return [self drive]->hasUnmodifiedDisk();
}

- (BOOL)hasUnprotectedDisk
{
    return [self drive]->hasUnprotectedDisk();
}

- (void)setModificationFlag:(BOOL)value
{
    [self drive]->setModificationFlag(value);
}

- (void)setProtectionFlag:(BOOL)value
{
    [self drive]->setProtectionFlag(value);
}

- (void)markDiskAsModified
{
    [self drive]->markDiskAsModified();
}

- (void)markDiskAsUnmodified
{
    [self drive]->markDiskAsUnmodified();
}
- (void)toggleWriteProtection
{
    [self drive]->toggleWriteProtection();
}

@end


//
// FloppyDrive proxy
//

@implementation FloppyDriveProxy

- (FloppyDrive *)drive
{
    return (FloppyDrive *)obj;
}

- (FloppyDriveInfo)info
{
    return [self drive]->getInfo();
}

- (BOOL)isInsertable:(Diameter)type density:(Density)density
{
    return [self drive]->isInsertable(type, density);
}

- (void)eject
{
    [self drive]->ejectDisk();
}

- (void)swap:(FloppyFileProxy *)fileProxy exception:(ExceptionWrapper *)ex
{
    try { return [self drive]->swapDisk(*(FloppyFile *)fileProxy->obj); }
    catch (VAError &error) { [ex save:error]; }
}

- (void)insertNew:(FSVolumeType)fs bootBlock:(BootBlockId)bb name:(NSString *)name exception:(ExceptionWrapper *)ex
{
    try { return [self drive]->insertNew(fs, bb, [name UTF8String]); }
    catch (VAError &error) { [ex save:error]; }
}

- (BOOL)motor
{
    return [self drive]->getMotor();
}

- (BOOL)selected
{
    return [self drive]->isSelected();
}

- (BOOL)writing
{
    return [self drive]->isWriting();
}

- (NSString *)readTrackBits:(NSInteger)track
{
    if (![self drive]->hasDisk()) return @("");
    return @([self drive]->disk->readTrackBits(track).c_str());
}

@end

//
// HardDrive proxy
//

@implementation HardDriveProxy

- (HardDrive *)drive
{
    return (HardDrive *)obj;
}

- (HardDriveInfo)info
{
    return [self drive]->getInfo();
}

- (NSInteger)capacity
{
    return [self drive]->getGeometry().numBytes();
}

- (NSInteger)partitions
{
    return [self drive]->numPartitions();
}

- (NSInteger)cylinders
{
    return [self drive]->getGeometry().cylinders;
}

- (NSInteger)heads
{
    return [self drive]->getGeometry().heads;
}

- (NSInteger)sectors
{
    return [self drive]->getGeometry().sectors;
}

- (NSInteger)bsize
{
    return [self drive]->getGeometry().bsize;
}

- (BOOL)uniqueGeometry
{
    return [self drive]->getGeometry().unique();
}

- (NSString *)nameOfPartition:(NSInteger)nr
{
    auto &info = [self drive]->getPartitionInfo(nr);
    return @(info.name.c_str());
}

- (NSInteger)lowerCylOfPartition:(NSInteger)nr
{
    auto &info = [self drive]->getPartitionInfo(nr);
    return info.lowCyl;
}

- (NSInteger)upperCylOfPartition:(NSInteger)nr
{
    auto &info = [self drive]->getPartitionInfo(nr);
    return info.highCyl;
}

- (HardDriveState)state
{
    return [self drive]->getState();
}

- (void)attach:(HDFFileProxy *)hdf exception:(ExceptionWrapper *)ex
{
    try {
        [self drive]->init(*(HDFFile *)hdf->obj);
    }  catch (VAError &error) {
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
        [self drive]->init(geometry);
    }  catch (VAError &error) {
        [ex save:error];
    }
}

- (void)format:(FSVolumeType)fs name:(NSString *)name exception:(ExceptionWrapper *)ex
{
    auto str = string([name UTF8String]);
    
    try {
        [self drive]->format(fs, str);
    }  catch (VAError &error) {
        [ex save:error];
    }
}

- (void)changeGeometry:(NSInteger)c h:(NSInteger)h s:(NSInteger)s b:(NSInteger)b exception:(ExceptionWrapper *)ex
{
    try {
        [self drive]->changeGeometry(c, h, s, b);
    }  catch (VAError &error) {
        [ex save:error];
    }
}

- (NSMutableArray *)geometries
{
    NSMutableArray *data = [[NSMutableArray alloc] init];
    
    auto geometry = [self drive]->getGeometry();
    auto geometries = GeometryDescriptor::driveGeometries(geometry.numBytes());
        
    for (auto &g : geometries) {
        
        NSInteger encoded = g.cylinders << 32 | g.heads << 16 | g.sectors;
        [data addObject: [NSNumber numberWithInteger:encoded]];
    }
    
    return data;
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
        
    }  catch (VAError &error) {
        
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
                
    }  catch (VAError &error) {
        
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
    catch (VAError &error) { [ex save:error]; }
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

- (RetroShell *)shell
{
    return (RetroShell *)obj;
}

+ (instancetype)make:(RetroShell *)shell
{
    if (shell == nullptr) { return nil; }
    
    RetroShellProxy *proxy = [[self alloc] initWith: shell];
    return proxy;
}

/*
-(NSInteger)cpos
{
    return [self shell]->cposAbs();
}
*/

-(NSInteger)cursorRel
{
    return [self shell]->cursorRel();
}

-(NSString *)getText
{
    const char *str = [self shell]->text();
    return str ? @(str) : nullptr;
}

- (void)pressUp
{
    [self shell]->press(RSKEY_UP);
}

- (void)pressDown
{
    [self shell]->press(RSKEY_DOWN);
}

- (void)pressLeft
{
    [self shell]->press(RSKEY_LEFT);
}

- (void)pressRight
{
    [self shell]->press(RSKEY_RIGHT);
}

- (void)pressHome
{
    [self shell]->press(RSKEY_HOME);
}

- (void)pressEnd
{
    [self shell]->press(RSKEY_END);
}

- (void)pressBackspace
{
    [self shell]->press(RSKEY_BACKSPACE);
}

- (void)pressDelete
{
    [self shell]->press(RSKEY_DEL);
}

- (void)pressReturn
{
    [self shell]->press(RSKEY_RETURN);
}

- (void)pressTab
{
    [self shell]->press(RSKEY_TAB);
}

- (void)pressKey:(char)c
{
    [self shell]->press(c);
}

@end

//
// RemoteManager proxy
//

@implementation RemoteManagerProxy

- (RemoteManager *)manager
{
    return (RemoteManager *)obj;
}

+ (instancetype)make:(RemoteManager *)manager
{
    if (manager == nullptr) { return nil; }
    
    RemoteManagerProxy *proxy = [[self alloc] initWith: manager];
    return proxy;
}

-(NSInteger)numLaunching
{
    return [self manager]->numLaunching();
}

-(NSInteger)numListening
{
    return [self manager]->numListening();
}

-(NSInteger)numConnected
{
    return [self manager]->numConnected();
}

-(NSInteger)numErroneous
{
    return [self manager]->numErroneous();
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

- (NSString *)sizeAsString
{
    const string &str = [self file]->sizeAsString();
    return @(str.c_str());
}

- (u64)fnv
{
    return [self file]->fnv();
}

- (void)setPath:(NSString *)path
{
    [self file]->path = [path fileSystemRepresentation];
}

- (NSInteger)writeToFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self file]->writeToFile([path fileSystemRepresentation]); }
    catch (VAError &error) { [ex save:error]; return 0; }
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
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new Snapshot((u8 *)buf, len)]; }
    catch (VAError &error) { [ex save:error]; return nil; }
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
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new Script((const u8 *)buf, len)]; }
    catch (VAError &error) { [ex save:error]; return nil; }
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
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new RomFile((const u8 *)buf, len)]; }
    catch (VAError &error) { [ex save:error]; return nil; }
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
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new ExtendedRomFile((const u8 *)buf, len)]; }
    catch (VAError &error) { [ex save:error]; return nil; }
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
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new ADFFile((const u8 *)buf, len)]; }
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithDrive:(FloppyDriveProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self make: new ADFFile(*[proxy drive])]; }
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithDiameter:(Diameter)dia density:(Density)den exception:(ExceptionWrapper *)ex
{
    try { return [self make: new ADFFile(dia, den)]; }
    catch (VAError &error) { [ex save:error]; return nil; }
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
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new HDFFile((const u8 *)buf, len)]; }
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithHardDrive:(HardDriveProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self make: new HDFFile(*[proxy drive])]; }
    catch (VAError &error) { [ex save:error]; return nil; }
}

- (BOOL)hasRDB
{
    return [self hdf]->hasRDB();
}

- (NSInteger)numPartitions
{
    return [self hdf]->numPartitions();
}

- (NSInteger)writeToFile:(NSString *)path partition:(NSInteger)nr exception:(ExceptionWrapper *)ex
{
    try { return [self hdf]->writePartitionToFile([path fileSystemRepresentation], nr); }
    catch (VAError &error) { [ex save:error]; return 0; }
}

@end


//
// EXTFileProxy
//

@implementation EXTFileProxy

- (EXTFile *)ext
{
    return (EXTFile *)obj;
}

+ (instancetype)make:(EXTFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (instancetype)makeWithFile:(NSString *)path exception:(ExceptionWrapper *)ex
{
    try { return [self make: new EXTFile([path fileSystemRepresentation])]; }
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new EXTFile((const u8 *)buf, len)]; }
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithDrive:(FloppyDriveProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self make: new EXTFile(*[proxy drive])]; }
    catch (VAError &error) { [ex save:error]; return nil; }
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
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new IMGFile((const u8 *)buf, len)]; }
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithDrive:(FloppyDriveProxy *)proxy exception:(ExceptionWrapper *)ex
{
    try { return [self make: new IMGFile(*[proxy drive]->disk)]; }
    catch (VAError &error) { [ex save:error]; return nil; }
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
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new DMSFile((const u8 *)buf, len)]; }
    catch (VAError &error) { [ex save:error]; return nil; }
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
    catch (VAError &error) { [ex save:error]; return nil; }
}

+ (instancetype)makeWithBuffer:(const void *)buf length:(NSInteger)len exception:(ExceptionWrapper *)ex
{
    try { return [self make: new EXEFile((const u8 *)buf, len)]; }
    catch (VAError &error) { [ex save:error]; return nil; }
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
    catch (VAError &error) { [ex save:error]; return nil; }
}

- (ADFFileProxy *)adf
{
    return [ADFFileProxy make:[self dir]->adf];
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
@synthesize denise;
@synthesize df0;
@synthesize df1;
@synthesize df2;
@synthesize df3;
@synthesize hd0;
@synthesize hd1;
@synthesize hd2;
@synthesize hd3;
@synthesize diskController;
@synthesize dmaDebugger;
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
    
    Amiga *amiga = new Amiga();
    obj = amiga;
    
    // Create sub proxys
    agnus = [[AgnusProxy alloc] initWith:&amiga->agnus];
    blitter = [[BlitterProxy alloc] initWith:&amiga->agnus.blitter];
    breakpoints = [[GuardsProxy alloc] initWith:&amiga->cpu.debugger.breakpoints];
    ciaA = [[CIAProxy alloc] initWith:&amiga->ciaA];
    ciaB = [[CIAProxy alloc] initWith:&amiga->ciaB];
    controlPort1 = [[ControlPortProxy alloc] initWith:&amiga->controlPort1];
    controlPort2 = [[ControlPortProxy alloc] initWith:&amiga->controlPort2];
    copper = [[CopperProxy alloc] initWith:&amiga->agnus.copper];
    copperBreakpoints = [[GuardsProxy alloc] initWith:&amiga->agnus.copper.debugger.breakpoints];
    cpu = [[CPUProxy alloc] initWith:&amiga->cpu];
    denise = [[DeniseProxy alloc] initWith:&amiga->denise];
    df0 = [[FloppyDriveProxy alloc] initWith:&amiga->df0];
    df1 = [[FloppyDriveProxy alloc] initWith:&amiga->df1];
    df2 = [[FloppyDriveProxy alloc] initWith:&amiga->df2];
    df3 = [[FloppyDriveProxy alloc] initWith:&amiga->df3];
    hd0 = [[HardDriveProxy alloc] initWith:&amiga->hd0];
    hd1 = [[HardDriveProxy alloc] initWith:&amiga->hd1];
    hd2 = [[HardDriveProxy alloc] initWith:&amiga->hd2];
    hd3 = [[HardDriveProxy alloc] initWith:&amiga->hd3];
    diskController = [[DiskControllerProxy alloc] initWith:&amiga->paula.diskController];
    dmaDebugger = [[DmaDebuggerProxy alloc] initWith:&amiga->agnus.dmaDebugger];
    keyboard = [[KeyboardProxy alloc] initWith:&amiga->keyboard];
    mem = [[MemProxy alloc] initWith:&amiga->mem];
    paula = [[PaulaProxy alloc] initWith:&amiga->paula];
    retroShell = [[RetroShellProxy alloc] initWith:&amiga->retroShell];
    rtc = [[RtcProxy alloc] initWith:&amiga->rtc];
    recorder = [[RecorderProxy alloc] initWith:&amiga->denise.screenRecorder];
    remoteManager = [[RemoteManagerProxy alloc] initWith:&amiga->remoteManager];
    serialPort = [[SerialPortProxy alloc] initWith:&amiga->serialPort];
    watchpoints = [[GuardsProxy alloc] initWith:&amiga->cpu.debugger.watchpoints];

    return self;
}

- (Amiga *)amiga
{
    return (Amiga *)obj;
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

- (BOOL)warpMode
{
    return [self amiga]->inWarpMode();
}

- (void)setWarpMode:(BOOL)value
{
    if (value) {
        [self amiga]->warpOn();
    } else {
        [self amiga]->warpOff();
    }
}

- (BOOL)debugMode
{
    return [self amiga]->inDebugMode();
}

- (void)setDebugMode:(BOOL)value
{
    if (value) {
        [self amiga]->debugOn();
    } else {
        [self amiga]->debugOff();
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
    catch (VAError &error) { [ex save:error]; }
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
    catch (VAError &error) { [ex save:error]; }
}

- (void)pause
{
    [self amiga]->pause();
}

- (void)halt
{
    [self amiga]->halt();
}

- (void)stopAndGo
{
    [self amiga]->stopAndGo();
}

- (void)stepInto
{
    [self amiga]->stepInto();
}

- (void)stepOver
{
    [self amiga]->stepOver();
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
    catch (VAError &error) { [ex save:error]; }
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
    } catch (VAError &exception) {
        return false;
    }
}

- (BOOL)configure:(Option)opt enable:(BOOL)val
{
    try {
        [self amiga]->configure(opt, val ? 1 : 0);
        return true;
    } catch (VAError &exception) {
        return false;
    }
}

- (BOOL)configure:(Option)opt id:(NSInteger)id value:(NSInteger)val
{
    try {
        [self amiga]->configure(opt, id, val);
        return true;
    } catch (VAError &exception) {
        return false;
    }
}

- (BOOL)configure:(Option)opt id:(NSInteger)id enable:(BOOL)val
{
    try {
        [self amiga]->configure(opt, id, val ? 1 : 0);
        return true;
    } catch (VAError &exception) {
        return false;
    }
}

- (BOOL)configure:(Option)opt drive:(NSInteger)id value:(NSInteger)val
{
    try {
        [self amiga]->configure(opt, (long)id, val);
        return true;
    } catch (VAError &exception) {
        return false;
    }
}

- (BOOL)configure:(Option)opt drive:(NSInteger)id enable:(BOOL)val
{
    try {
        [self amiga]->configure(opt, (long)id, val ? 1 : 0);
        return true;
    } catch (VAError &exception) {
        return false;
    }
}

- (void)setListener:(const void *)sender function:(Callback *)func
{
    [self amiga]->msgQueue.setListener(sender, func);
}

@end
