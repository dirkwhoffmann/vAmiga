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

using namespace moira;

//
// Base Proxy
//

@implementation Proxy

- (instancetype) initWith:(void *)ref
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
// HardwareComponent proxy
//

@implementation HardwareComponentProxy

-(HardwareComponent *)hwc
{
    return (HardwareComponent *)obj;
}

-(void)dump
{
    [self hwc]->dump();
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

- (NSInteger) count
{
    return [self guards]->elements();
}

- (NSInteger) addr:(NSInteger)nr
{
    return [self guards]->guardAddr(nr);
}

- (BOOL) isEnabled:(NSInteger)nr
{
    return [self guards]->isEnabled(nr);
}

- (BOOL) isDisabled:(NSInteger)nr
{
    return [self guards]->isDisabled(nr);
}

- (void) enable:(NSInteger)nr
{
    [self guards]->enable(nr);
}

- (void) disable:(NSInteger)nr
{
    [self guards]->disable(nr);
}

- (void) remove:(NSInteger)nr
{
    return [self guards]->remove(nr);
}

- (void) replace:(NSInteger)nr addr:(NSInteger)addr
{
    [self guards]->replace(nr, (u32)addr);
}

- (BOOL) isSetAt:(NSInteger)addr
{
    return [self guards]->isSetAt((u32)addr);
}

- (BOOL) isSetAndEnabledAt:(NSInteger)addr
{
    return [self guards]->isSetAndEnabledAt((u32)addr);
}

- (BOOL) isSetAndDisabledAt:(NSInteger)addr
{
    return [self guards]->isSetAndDisabledAt((u32)addr);
}

- (void) enableAt:(NSInteger)addr
{
    [self guards]->enableAt((u32)addr);
}

- (void) disableAt:(NSInteger)addr
{
    [self guards]->disableAt((u32)addr);
}

- (void) addAt:(NSInteger)addr
{
    [self guards]->addAt((u32)addr);
}

- (void) removeAt:(NSInteger)addr
{
    [self guards]->removeAt((u32)addr);
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

- (CPUInfo) getInfo
{
    return [self cpu]->getInfo();
}

- (i64) clock
{
    return [self cpu]->getMasterClock();
}

- (i64) cycles
{
    return [self cpu]->getCpuClock();
}

- (bool) isHalted
{
    return [self cpu]->isHalted();
}

- (NSInteger) loggedInstructions
{
    return [self cpu]->debugger.loggedInstructions();
}

- (void) clearLog
{
    return [self cpu]->debugger.clearLog();
}

- (NSString *) disassembleRecordedInstr:(NSInteger)i length:(NSInteger *)len
{
    const char *str = [self cpu]->disassembleRecordedInstr((int)i, len);
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (NSString *) disassembleRecordedBytes:(NSInteger)i length:(NSInteger)len
{
    const char *str = [self cpu]->disassembleRecordedWords((int)i, len);
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (NSString *) disassembleRecordedFlags:(NSInteger)i
{
    const char *str = [self cpu]->disassembleRecordedFlags((int)i);
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (NSString *) disassembleRecordedPC:(NSInteger)i
{
    const char *str = [self cpu]->disassembleRecordedPC((int)i);
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (NSString *) disassembleInstr:(NSInteger)addr length:(NSInteger *)len
{
    const char *str = [self cpu]->disassembleInstr(addr, len);
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (NSString *) disassembleWords:(NSInteger)addr length:(NSInteger)len
{
    const char *str = [self cpu]->disassembleWords(addr, len);
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (NSString *) disassembleAddr:(NSInteger)addr
{
    const char *str = [self cpu]->disassembleAddr(addr);
    return str ? [NSString stringWithUTF8String:str] : nullptr;
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

- (CIAInfo) getInfo
{
    return [self cia]->getInfo();
}

- (void) dumpConfig
{
    [self cia]->dumpConfig();
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

- (MemoryConfig) getConfig
{
    return [self mem]->getConfig();
}

- (MemoryStats) getStats
{
    return [self mem]->getStats();
}

- (BOOL) isBootRom:(RomIdentifier)rev
{
    return RomFile::isBootRom(rev);
}

- (BOOL) isArosRom:(RomIdentifier)rev
{
    return RomFile::isArosRom(rev);
}

- (BOOL) isDiagRom:(RomIdentifier)rev
{
    return RomFile::isDiagRom(rev);
}

- (BOOL) isCommodoreRom:(RomIdentifier)rev
{
    return RomFile::isCommodoreRom(rev);
}

- (BOOL) isHyperionRom:(RomIdentifier)rev
{
    return RomFile::isHyperionRom(rev);
}

- (BOOL) hasRom
{
    return [self mem]->hasKickRom();
}

- (BOOL) hasBootRom
{
    return [self mem]->hasBootRom();
}

- (BOOL) hasKickRom
{
    return [self mem]->hasKickRom();
}

- (void) deleteRom
{
    [self mem]->deleteRom();
}

- (BOOL) isRom:(NSURL *)url
{
    return RomFile::isRomFile([url fileSystemRepresentation]);
}

- (BOOL) loadRomFromBuffer:(NSData *)data
{
    if (data == nullptr) return NO;
    const u8 *bytes = (const u8 *)[data bytes];
    return [self mem]->loadRomFromBuffer(bytes, [data length]);
}

- (BOOL) loadRomFromFile:(NSURL *)url error:(ErrorCode *)err
{
    return [self mem]->loadRomFromFile([url fileSystemRepresentation], err);
}

- (u64) romFingerprint
{
    return [self mem]->romFingerprint();
}

- (RomIdentifier) romIdentifier
{
    return [self mem]->romIdentifier();
}

- (NSString *) romTitle
{
    const char *str = [self mem]->romTitle();
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (NSString *) romVersion
{
    const char *str = [self mem]->romVersion();
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (NSString *) romReleased
{
    const char *str = [self mem]->romReleased();
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (BOOL) hasExt
{
    return [self mem]->hasExt();
}

- (void) deleteExt
{
    [self mem]->deleteExt();
}

- (BOOL) isExt:(NSURL *)url
{
    return ExtendedRomFile::isExtendedRomFile([url fileSystemRepresentation]);
}

- (BOOL) loadExtFromBuffer:(NSData *)data
{
    if (data == nullptr) return NO;
    const u8 *bytes = (const u8 *)[data bytes];
    return [self mem]->loadExtFromBuffer(bytes, [data length]);
}

- (BOOL) loadExtFromFile:(NSURL *)url
{
    return [self mem]->loadExtFromFile([url fileSystemRepresentation]);
}

- (u64) extFingerprint
{
    return [self mem]->extFingerprint();
}

- (RomIdentifier) extIdentifier
{
    return [self mem]->extIdentifier();
}

- (NSString *) extTitle
{
    const char *str = [self mem]->extTitle();
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (NSString *) extVersion
{
    const char *str = [self mem]->extVersion();
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (NSString *) extReleased
{
    const char *str = [self mem]->extReleased();
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (NSInteger) extStart
{
    return [self mem]->getConfigItem(OPT_EXT_START);
}

- (BOOL) saveWom:(NSURL *)url
{
    return [self mem]->saveWom([url fileSystemRepresentation]);
}

- (BOOL) saveRom:(NSURL *)url
{
    return [self mem]->saveRom([url fileSystemRepresentation]);
}

- (BOOL) saveExt:(NSURL *)url
{
    return [self mem]->saveExt([url fileSystemRepresentation]);
}

- (void) updateRTC
{
    [self mem]->rtc.update();
}

- (MemorySource) memSrc:(Accessor)accessor addr:(NSInteger)addr
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    
    if (accessor == ACCESSOR_CPU) {
        return [self mem]->getMemSrc <ACCESSOR_CPU> ((u32)addr);
    } else {
        return [self mem]->getMemSrc <ACCESSOR_AGNUS> ((u32)addr);
    }
}

- (NSInteger) spypeek16:(Accessor)accessor addr:(NSInteger)addr
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    
    if (accessor == ACCESSOR_CPU) {
        return [self mem]->spypeek16 <ACCESSOR_CPU> ((u32)addr);
    } else {
        return [self mem]->spypeek16 <ACCESSOR_AGNUS> ((u32)addr);
    }
}

- (NSString *) ascii:(Accessor)accessor addr:(NSInteger)addr
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    const char *str;

    if (accessor == ACCESSOR_CPU) {
        str = [self mem]->ascii <ACCESSOR_CPU> ((u32)addr);
    } else {
        str = [self mem]->ascii <ACCESSOR_AGNUS> ((u32)addr);
    }
    
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (NSString *) hex:(Accessor)accessor addr: (NSInteger)addr bytes:(NSInteger)bytes
{
    assert(accessor == ACCESSOR_CPU || accessor == ACCESSOR_AGNUS);
    const char *str;

    if (accessor == ACCESSOR_CPU) {
        str = [self mem]->hex <ACCESSOR_CPU> ((u32)addr, bytes);
    } else {
        str = [self mem]->hex <ACCESSOR_AGNUS> ((u32)addr, bytes);
    }
    
    return str ? [NSString stringWithUTF8String:str] : nullptr;
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

- (NSInteger) chipRamLimit
{
    return [self agnus]->chipRamLimit();
}

- (AgnusInfo) getInfo
{
    return [self agnus]->getInfo();
}

- (EventSlotInfo) getEventSlotInfo:(NSInteger)slot
{
    return [self agnus]->getEventSlotInfo(slot);
}

- (EventInfo) getEventInfo
{
    return [self agnus]->getEventInfo();
}

- (AgnusStats) getStats
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

- (CopperInfo) getInfo
{
    return [self copper]->getInfo();
}

- (NSInteger) instrCount:(NSInteger)list
{
    return [self copper]->instrCount(list);
}

- (void) adjustInstrCount:(NSInteger)list offset:(NSInteger)offset
{
    [self copper]->adjustInstrCount(list, offset);
}

- (BOOL) isIllegalInstr:(NSInteger)addr
{
    return [self copper]->isIllegalInstr(addr);
}

- (NSString *) disassemble:(NSInteger)addr
{
    const char *str = [self copper]->disassemble(addr);
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (NSString *) disassemble:(NSInteger)list instr:(NSInteger)offset
{
    const char *str = [self copper]->disassemble(list, offset);
    return str ? [NSString stringWithUTF8String:str] : nullptr;
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

- (BlitterInfo) getInfo
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

- (DMADebuggerInfo) getInfo
{
    return [self debugger]->getInfo();
}

- (void) setEnable:(BOOL)value
{
    [self debugger]->setEnabled(value);
}

- (void) visualizeCopper:(BOOL)value
{
    [self debugger]->visualizeCopper(value);
}

- (void) visualizeBlitter:(BOOL)value
{
    [self debugger]->visualizeBlitter(value);
}

- (void) visualizeDisk:(BOOL)value
{
    [self debugger]->visualizeDisk(value);
}

- (void) visualizeAudio:(BOOL)value
{
    [self debugger]->visualizeAudio(value);
}

- (void) visualizeSprite:(BOOL)value
{
    [self debugger]->visualizeSprite(value);
}

- (void) visualizeBitplane:(BOOL)value
{
    [self debugger]->visualizeBitplane(value);
}

- (void) visualizeCpu:(BOOL)value
{
    [self debugger]->visualizeCpu(value);
}

- (void) visualizeRefresh:(BOOL)value
{
    [self debugger]->visualizeRefresh(value);
}

- (void) setOpacity:(double)value
{
    [self debugger]->setOpacity(value);
}

- (void) setDisplayMode:(NSInteger)mode
{
    [self debugger]->setDisplayMode((DmaDisplayMode)mode);
}

- (void) setCopperColor:(double)r g:(double)g b:(double)b
{
    [self debugger]->setCopperColor(r, g, b);
}

- (void) setBlitterColor:(double)r g:(double)g b:(double)b
{
    [self debugger]->setBlitterColor(r, g, b);
}

- (void) setDiskColor:(double)r g:(double)g b:(double)b
{
    [self debugger]->setDiskColor(r, g, b);
}

- (void) setAudioColor:(double)r g:(double)g b:(double)b
{
    [self debugger]->setAudioColor(r, g, b);
}

- (void) setSpriteColor:(double)r g:(double)g b:(double)b
{
    [self debugger]->setSpriteColor(r, g, b);
}

- (void) setBitplaneColor:(double)r g:(double)g b:(double)b
{
    [self debugger]->setBitplaneColor(r, g, b);
}

- (void) setCpuColor:(double)r g:(double)g b:(double)b
{
    [self debugger]->setCpuColor(r, g, b);
}

- (void) setRefreshColor:(double)r g:(double)g b:(double)b
{
    [self debugger]->setRefreshColor(r, g, b);
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

- (DeniseInfo) getInfo
{
    return [self denise]->getInfo();
}

- (SpriteInfo) getSpriteInfo:(NSInteger)nr
{
    return [self denise]->getSpriteInfo(nr);
}

- (NSInteger) sprDataLines:(NSInteger)nr
{
    return [self denise]->getSpriteHeight(nr);
}

- (u64) sprData:(NSInteger)nr line:(NSInteger)line
{
    return [self denise]->getSpriteData(nr, line);
}

- (u16) sprColor:(NSInteger)nr reg:(NSInteger)reg
{
    return [self denise]->getSpriteColor(nr, reg);
}

- (Palette) palette
{
   return [self denise]->pixelEngine.getPalette();
}

- (void) setPalette:(Palette)p
{
    [self denise]->pixelEngine.setPalette(p);
}

- (double) brightness
{
    return [self denise]->pixelEngine.getBrightness();
}

- (void) setBrightness:(double)value
{
    [self denise]->pixelEngine.setBrightness(value);
}

- (double) saturation
{
    return [self denise]->pixelEngine.getSaturation();
}

- (void) setSaturation:(double)value
{
    [self denise]->pixelEngine.setSaturation(value);
}

- (double) contrast
{
    return [self denise]->pixelEngine.getContrast();
}

- (void) setContrast:(double)value
{
    [self denise]->pixelEngine.setContrast(value);
}

- (ScreenBuffer) stableBuffer
{
    return [self denise]->pixelEngine.getStableBuffer();
}

- (u32 *) noise
{
    return [self denise]->pixelEngine.getNoise();
}

@end


//
// ScreenRecorder
//

@implementation ScreenRecorderProxy

- (ScreenRecorder *)recorder
{
    return (ScreenRecorder *)obj;
}

- (BOOL) hasFFmpeg
{
    return [self recorder]->hasFFmpeg();
}

- (BOOL) recording
{
    return [self recorder]->isRecording();
}

- (NSInteger) recordCounter
{
    return [self recorder]->getRecordCounter();
}

- (BOOL) startRecording:(NSRect)rect
                bitRate:(NSInteger)rate
                aspectX:(NSInteger)aspectX
                aspectY:(NSInteger)aspectY
{
    int x1 = (int)rect.origin.x;
    int y1 = (int)rect.origin.y;
    int x2 = x1 + (int)rect.size.width;
    int y2 = y1 + (int)rect.size.height;
    
    return [self recorder]->startRecording(x1, y1, x2, y2,
                                           rate,
                                           aspectX,
                                           aspectY);
}

- (void) stopRecording
{
    [self recorder]->stopRecording();
}

- (BOOL) exportAs:(NSString *)path
{
    return [self recorder]->exportAs([path fileSystemRepresentation]);
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

- (PaulaInfo) getInfo
{
    return [self paula]->getInfo();
}

- (AudioInfo) getAudioInfo
{
    return [self paula]->getAudioInfo();
}

- (MuxerStats) getMuxerStats
{
    return [self paula]->muxer.getStats();
}

- (UARTInfo) getUARTInfo
{
    return [self paula]->uart.getInfo();
}

- (u32) sampleRate
{
    return (u32)[self paula]->muxer.getSampleRate();
}

- (void) setSampleRate:(double)rate
{
    [self paula]->muxer.setSampleRate(rate);
}

- (void) readMonoSamples:(float *)target size:(NSInteger)n
{
    [self paula]->muxer.copyMono(target, n);
}

- (void) readStereoSamples:(float *)target1 buffer2:(float *)target2 size:(NSInteger)n
{
    [self paula]->muxer.copyStereo(target1, target2, n);
}

- (void) rampUp
{
    [self paula]->muxer.rampUp();
}

- (void) rampUpFromZero
{
    [self paula]->muxer.rampUpFromZero();
}

- (void) rampDown
{
    [self paula]->muxer.rampDown();
}

- (float) drawWaveformL:(unsigned *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(unsigned)c
{
    return [self paula]->muxer.stream.draw(buffer, w, h, true, s, c);
}

- (float) drawWaveformL:(unsigned *)buffer size:(NSSize)size scale:(float)s color:(unsigned)c
{
    return [self drawWaveformL:buffer
                             w:(NSInteger)size.width
                             h:(NSInteger)size.height
                         scale:s
                         color:c];
}

- (float) drawWaveformR:(unsigned *)buffer w:(NSInteger)w h:(NSInteger)h scale:(float)s color:(unsigned)c
{
    return [self paula]->muxer.stream.draw(buffer, w, h, false, s, c);
}

- (float) drawWaveformR:(unsigned *)buffer size:(NSSize)size scale:(float)s color:(unsigned)c
{
    return [self drawWaveformR:buffer
                             w:(NSInteger)size.width
                             h:(NSInteger)size.height
                         scale:s
                         color:c];
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

- (void) setXY:(NSPoint)pos
{
    [self mouse]->setXY((double)pos.x, (double)pos.y);
}

- (void) setDeltaXY:(NSPoint)pos
{
    [self mouse]->setDeltaXY((double)pos.x, (double)pos.y);
}

- (void) trigger:(GamePadAction)event
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

- (void) trigger:(GamePadAction)event
{
    [self joystick]->trigger(event);
}

- (BOOL) autofire
{
    return [self joystick]->getAutofire();
}

- (void) setAutofire:(BOOL)value
{
    return [self joystick]->setAutofire(value);
}

- (NSInteger) autofireBullets
{
    return (NSInteger)[self joystick]->getAutofireBullets();
}

- (void) setAutofireBullets:(NSInteger)value
{
    [self joystick]->setAutofireBullets((int)value);
}

- (float) autofireFrequency
{
    return [self joystick]->getAutofireFrequency();
}

- (void) setAutofireFrequency:(float)value
{
    [self joystick]->setAutofireFrequency(value);
}

@end


//
// ControlPort proxy
//

@implementation ControlPortProxy

@synthesize mouse;
@synthesize joystick;

- (instancetype) initWith:(void *)ref
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

- (ControlPortInfo) getInfo
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

- (SerialPortInfo) getInfo
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

- (BOOL) keyIsPressed:(NSInteger)keycode
{
    return [self kb]->keyIsPressed(keycode);
}

- (void) pressKey:(NSInteger)keycode
{
    [self kb]->pressKey(keycode);
}

- (void) releaseKey:(NSInteger)keycode
{
    [self kb]->releaseKey(keycode);
}

- (void) releaseAllKeys
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

- (DiskControllerConfig) getConfig
{
    return [self dc]->getConfig();
}

- (DiskControllerInfo) getInfo
{
    return [self dc]->getInfo();
}

- (NSInteger) selectedDrive
{
    return [self dc]->getSelected();
}

- (DriveState) state
{
    return [self dc]->getState();
}

- (BOOL) isSpinning
{
    return [self dc]->spinning();
}

- (void) eject:(NSInteger)nr
{
    [self dc]->ejectDisk(nr);
}

- (void) insert:(NSInteger)nr file:(DiskFileProxy *)fileProxy
{
    [self dc]->insertDisk((DiskFile *)fileProxy->obj, nr);
}

- (void) setWriteProtection:(NSInteger)nr value:(BOOL)value
{
    [self dc]->setWriteProtection(nr, value);
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

- (DriveInfo) getInfo
{
    return [self drive]->getInfo();
}

- (NSInteger) nr
{
    return [self drive]->getNr();
}

- (BOOL) hasDisk
{
    return [self drive]->hasDisk();
}

- (BOOL) hasDDDisk
{
    return [self drive]->hasDDDisk();
}

- (BOOL) hasHDDisk
{
    return [self drive]->hasHDDisk();
}

- (BOOL) hasWriteProtectedDisk
{
    return [self drive]->hasWriteProtectedDisk();
}

- (void) setWriteProtection:(BOOL)value
{
    [self drive]->setWriteProtection(value);
}

- (void) toggleWriteProtection
{
    [self drive]->toggleWriteProtection();
}

- (BOOL) isInsertable:(DiskDiameter)type density:(DiskDensity)density
{
    return [self drive]->isInsertable(type, density);
}

- (BOOL) isModifiedDisk
{
    return [self drive]->hasModifiedDisk();
}

- (void) setModifiedDisk:(BOOL)value
{
    [self drive]->setModifiedDisk(value);
}

- (BOOL) motor
{
    return [self drive]->getMotor();
}

- (NSInteger) cylinder
{
    return [self drive]->getCylinder();
}

- (u64) fnv
{
    return [self drive]->fnv();
}

@end


//
// FSDevice proxy
//

@implementation FSDeviceProxy

- (FSDevice *)fs
{
    return (FSDevice *)obj;
}

+ (instancetype) make:(FSDevice *)volume
{
    if (volume == nullptr) { return nil; }
    
    FSDeviceProxy *proxy = [[self alloc] initWith: volume];
    return proxy;
}

+ (instancetype) makeWithADF:(ADFFileProxy *)proxy
{
    ErrorCode error;
    FSDevice *volume = FSDevice::makeWithADF((ADFFile *)(proxy->obj), &error);
    return [self make:volume];
}

+ (instancetype) makeWithHDF:(HDFFileProxy *)proxy
{
    ErrorCode error;
    FSDevice *volume = FSDevice::makeWithHDF((HDFFile *)(proxy->obj), &error);
    return [self make:volume];
}

- (FSVolumeType) dos
{
    return [self fs]->dos();
}

- (NSInteger) numCyls
{
    return [self fs]->getNumCyls();
}

- (NSInteger) numHeads
{
    return [self fs]->getNumHeads();
}

- (NSInteger) numTracks
{
    return [self fs]->getNumTracks();
}

- (NSInteger) numSectors
{
    return [self fs]->getNumSectors();
}

- (NSInteger) numBlocks
{
    return [self fs]->getNumBlocks();
}

- (void)killVirus
{
    [self fs]->killVirus();
}

- (FSBlockType) blockType:(NSInteger)blockNr
{
    return [self fs]->blockType(blockNr);
}

- (FSItemType) itemType:(NSInteger)blockNr pos:(NSInteger)pos
{
    return [self fs]->itemType(blockNr, pos);
}

- (FSErrorReport) check:(BOOL)strict
{
    return [self fs]->check(strict);
}

- (ErrorCode) check:(NSInteger)nr
                pos:(NSInteger)pos
           expected:(unsigned char *)exp
             strict:(BOOL)strict
{
    return [self fs]->check(nr, pos, exp, strict);
}

- (BOOL) isCorrupted:(NSInteger)blockNr
{
    return [self fs]->isCorrupted((u32)blockNr);
}

- (NSInteger) getCorrupted:(NSInteger)blockNr
{
    return [self fs]->getCorrupted((u32)blockNr);
}

- (NSInteger) nextCorrupted:(NSInteger)blockNr
{
    return [self fs]->nextCorrupted((u32)blockNr);
}

- (NSInteger) prevCorrupted:(NSInteger)blockNr
{
    return [self fs]->prevCorrupted((u32)blockNr);
}

- (void) printDirectory:(BOOL) recursive
{
    return [self fs]->printDirectory(recursive);
}

- (NSInteger) readByte:(NSInteger)block offset:(NSInteger)offset
{
    return [self fs]->readByte(block, (u32)offset);
}

- (ErrorCode) export:(NSString *)path
{
    return [self fs]->exportDirectory([path fileSystemRepresentation]);
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

- (void)setPath:(NSString *)path
{
    [self file]->setPath([path fileSystemRepresentation]);
}

- (FileType)type
{
    return [self file]->fileType();
}

- (NSInteger) sizeOnDisk
{
    return [self file]->sizeOnDisk();
}

- (u64) fnv
{
    return [self file]->fnv();
}

- (void) readFromBuffer:(const void *)buffer length:(NSInteger)length
{
    ErrorCode error;
    [self file]->readFromBuffer((const u8 *)buffer, length, &error);
}

- (NSInteger) writeToBuffer:(void *)buffer
{
    return [self file]->writeToBuffer((u8 *)buffer);
}

- (void) dealloc
{
    delete (AmigaFile *)obj;
}

@end


//
// Snapshot
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

+ (instancetype) makeWithBuffer:(const void *)buf length:(NSInteger)len
{
    Snapshot *snapshot = AmigaFile::make <Snapshot> ((u8 *)buf, len);
    return [self make:snapshot];
}

+ (instancetype) makeWithBuffer:(const void *)buf length:(NSInteger)len error:(ErrorCode *)err
{
    Snapshot *snapshot = AmigaFile::make <Snapshot> ((u8 *)buf, len, err);
    return [self make:snapshot];
}

+ (instancetype) makeWithFile:(NSString *)path
{
    Snapshot *snapshot = AmigaFile::make <Snapshot> ([path UTF8String]);
    return [self make:snapshot];
}

+ (instancetype) makeWithFile:(NSString *)path error:(ErrorCode *)err
{
    Snapshot *snapshot = AmigaFile::make <Snapshot> ([path UTF8String], err);
    return [self make:snapshot];
}

+ (instancetype) makeWithAmiga:(AmigaProxy *)proxy
{
    Amiga *amiga = (Amiga *)proxy->obj;
    
    amiga->suspend();
    Snapshot *snapshot = Snapshot::makeWithAmiga(amiga);
    amiga->resume();
    
    return [self make:snapshot];
}

- (NSImage *)previewImage
{
    if (preview != nullptr) { return preview; }
    
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

/*
- (time_t)timeStamp __attribute__ ((deprecated))
{
    return ((Snapshot *)wrapper->file)->getThumbnail().timestamp;
}
*/

- (NSData *)data
{
    return [NSData dataWithBytes: (void *)[self snapshot]->getHeader()
                          length: [self snapshot]->sizeOnDisk()];
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

+ (instancetype)make:(DiskFile *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

- (FSVolumeType)dos
{
    return [self file]->getDos();
}

- (DiskDiameter)diskType
{
    return [self file]->getDiskDiameter();
}

- (DiskDensity)diskDensity
{
    return [self file]->getDiskDensity();
}

- (NSInteger)numCyls
{
    return [self file]->numCyls();
}

- (NSInteger)numSides
{
    return [self file]->numSides();
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

- (BootBlockType)bootBlockType
{
    return [self file]->bootBlockType();
}

- (NSString *)bootBlockName
{
    const char *str = [self file]->bootBlockName();
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (BOOL)hasVirus
{
    return [self file]->hasVirus();
}

- (void)killVirus
{
    [self file]->killVirus();
}

- (NSInteger)readByte:(NSInteger)block offset:(NSInteger)offset
{
    return [self file]->readByte(block, offset);
}

- (void)readSector:(unsigned char *)dst block:(NSInteger)nr
{
    [self file]->readSector(dst, nr);
}

- (void)readSectorHex:(char *)dst block:(NSInteger)block count:(NSInteger)count
{
    [self file]->readSectorHex(dst, block, count);
}

@end


//
// ADFFileProxy
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

+ (BOOL)isADFFile:(NSString *)path
{
    return ADFFile::isADFFile([path fileSystemRepresentation]);
}

+ (instancetype)makeWithBuffer:(const void *)buffer length:(NSInteger)length
{
    ADFFile *archive = AmigaFile::make <ADFFile> ((const u8 *)buffer, length);
    return [self make: archive];
}

+ (instancetype)makeWithFile:(NSString *)path error:(ErrorCode *)err
{
    ADFFile *archive = AmigaFile::make <ADFFile> ([path fileSystemRepresentation], err);
    return [self make: archive];
}

+ (instancetype)makeWithDiameter:(DiskDiameter)type density:(DiskDensity)density
{
    ADFFile *archive = ADFFile::makeWithType(type, density);
    return [self make: archive];
}

+ (instancetype)makeWithDrive:(DriveProxy *)proxy
{
    Drive *drive = [proxy drive];
    ADFFile *archive = ADFFile::makeWithDrive(drive);
    return archive ? [self make: archive] : nil;
}

- (void)formatDisk:(FSVolumeType)fs bootBlock:(NSInteger)bootBlockID
{
    [self adf]->formatDisk(fs, bootBlockID);
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

+ (BOOL)isHDFFile:(NSString *)path
{
    return HDFFile::isHDFFile([path fileSystemRepresentation]);
}

+ (instancetype) makeWithBuffer:(const void *)buffer length:(NSInteger)len
{
    HDFFile *archive = AmigaFile::make <HDFFile> ((const u8 *)buffer, len);
    return [self make: archive];
}

+ (instancetype) makeWithFile:(NSString *)path error:(ErrorCode *)err
{
    HDFFile *archive = AmigaFile::make <HDFFile> ([path fileSystemRepresentation], err);
    return [self make: archive];
}

- (NSInteger) numBlocks
{
    return [self hdf]->numBlocks();
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

+ (BOOL)isEXTFile:(NSString *)path
{
    return EXTFile::isEXTFile([path fileSystemRepresentation]);
}

+ (instancetype) makeWithBuffer:(const void *)buffer length:(NSInteger)len
{
    EXTFile *archive = AmigaFile::make <EXTFile> ((const u8 *)buffer, len);
    return [self make: archive];
}

+ (instancetype) makeWithFile:(NSString *)path error:(ErrorCode *)err
{
    EXTFile *archive = AmigaFile::make <EXTFile> ([path fileSystemRepresentation], err);
    return [self make: archive];
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

+ (BOOL)isIMGFile:(NSString *)path
{
    return IMGFile::isIMGFile([path fileSystemRepresentation]);
}

+ (instancetype) makeWithBuffer:(const void *)buffer length:(NSInteger)len
{
    IMGFile *archive = AmigaFile::make <IMGFile> ((const u8 *)buffer, len);
    return [self make: archive];
}

+ (instancetype) makeWithFile:(NSString *)path error:(ErrorCode *)err
{
    IMGFile *archive = AmigaFile::make <IMGFile> ([path fileSystemRepresentation], err);
    return [self make: archive];
}

+ (instancetype) makeWithDrive:(DriveProxy *)drive
{
    IMGFile *archive = IMGFile::makeWithDisk([drive drive]->disk);
    return archive ? [self make: archive] : nil;
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

+ (BOOL)isDMSFile:(NSString *)path
{
    return DMSFile::isDMSFile([path fileSystemRepresentation]);
}

+ (instancetype) makeWithBuffer:(const void *)buffer length:(NSInteger)len
{
    DMSFile *archive = AmigaFile::make <DMSFile> ((const u8 *)buffer, len);
    return [self make: archive];
}

+ (instancetype) makeWithFile:(NSString *)path error:(ErrorCode *)err
{
    DMSFile *archive = AmigaFile::make <DMSFile> ([path fileSystemRepresentation], err);
    return [self make: archive];
}

- (ADFFileProxy *)adf
{
    return [ADFFileProxy make:[self dms]->adf];
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

+ (BOOL)isEXEFile:(NSString *)path
{
    return EXEFile::isEXEFile([path fileSystemRepresentation]);
}

+ (instancetype) makeWithBuffer:(const void *)buffer length:(NSInteger)len
{
    EXEFile *archive = AmigaFile::make <EXEFile> ((const u8 *)buffer, len);
    return [self make: archive];
}

+ (instancetype) makeWithFile:(NSString *)path error:(ErrorCode *)err
{
    EXEFile *archive = AmigaFile::make <EXEFile> ([path fileSystemRepresentation], err);
    return [self make: archive];
}

- (ADFFileProxy *)adf
{
    return [ADFFileProxy make:[self exe]->adf];
}

@end


//
// DIRFileProxy
//

@implementation DIRFileProxy

- (Folder *)dir
{
    return (Folder *)obj;
}

+ (instancetype)make:(Folder *)file
{
    return file ? [[self alloc] initWith:file] : nil;
}

+ (BOOL)isDIRFile:(NSString *)path
{
    return Folder::isDIRFile([path fileSystemRepresentation]);
}

+ (instancetype) makeWithFile:(NSString *)path error:(ErrorCode *)err
{
    Folder *archive = AmigaFile::make <Folder> ([path fileSystemRepresentation], err);
    return [self make: archive];
}

- (ADFFileProxy *)adf
{
    return [ADFFileProxy make:[self dir]->adf];
}

@end


//
// Amiga
//

@implementation AmigaProxy

@synthesize wrapper;
@synthesize agnus;
@synthesize blitter;
@synthesize breakpoints;
@synthesize ciaA;
@synthesize ciaB;
@synthesize controlPort1;
@synthesize controlPort2;
@synthesize copper;
@synthesize cpu;
@synthesize denise;
@synthesize df0;
@synthesize df1;
@synthesize df2;
@synthesize df3;
@synthesize diskController;
@synthesize dmaDebugger;
@synthesize keyboard;
@synthesize mem;
@synthesize paula;
@synthesize serialPort;
@synthesize screenRecorder;
@synthesize watchpoints;

- (instancetype) init
{
    NSLog(@"AmigaProxy::init");
    
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
    cpu = [[CPUProxy alloc] initWith:&amiga->cpu];
    denise = [[DeniseProxy alloc] initWith:&amiga->denise];
    df0 = [[DriveProxy alloc] initWith:&amiga->df0];
    df1 = [[DriveProxy alloc] initWith:&amiga->df1];
    df2 = [[DriveProxy alloc] initWith:&amiga->df2];
    df3 = [[DriveProxy alloc] initWith:&amiga->df3];
    diskController = [[DiskControllerProxy alloc] initWith:&amiga->paula.diskController];
    dmaDebugger = [[DmaDebuggerProxy alloc] initWith:&amiga->agnus.dmaDebugger];
    keyboard = [[KeyboardProxy alloc] initWith:&amiga->keyboard];
    mem = [[MemProxy alloc] initWith:&amiga->mem];
    paula = [[PaulaProxy alloc] initWith:&amiga->paula];
    screenRecorder = [[ScreenRecorderProxy alloc] initWith:&amiga->denise.screenRecorder];
    serialPort = [[SerialPortProxy alloc] initWith:&amiga->serialPort];
    watchpoints = [[GuardsProxy alloc] initWith:&amiga->cpu.debugger.watchpoints];

    return self;
}

- (Amiga *)amiga
{
    return (Amiga *)obj;
}

- (void) kill
{
    NSLog(@"AmigaProxy::kill");
    assert(obj);

    delete [self amiga];
}

- (BOOL) isReleaseBuild
{
    return releaseBuild();
}

- (BOOL) debugMode
{
    return [self amiga]->inDebugMode();
}

- (void) setDebugMode:(BOOL)enable
{
    [self amiga]->setDebug(enable);
}

- (void) enableDebugging
{
    [self amiga]->enableDebugMode();
}

- (void) disableDebugging
{
    [self amiga]->disableDebugMode();
}

- (void) setInspectionTarget:(EventID)id
{
    [self amiga]->setInspectionTarget(id);
}

- (void) clearInspectionTarget
{
    [self amiga]->clearInspectionTarget();
}

- (BOOL) isReady:(ErrorCode *)error
{
    return [self amiga]->isReady(error);
}

- (BOOL) isReady
{
    return [self amiga]->isReady();
}

- (void) powerOn
{
    [self amiga]->powerOn();
}

- (void) powerOff
{
    [self amiga]->powerOff();
}

- (void) hardReset
{
    [self amiga]->reset(true);
}

- (void) softReset
{
    [self amiga]->reset(false);
}

- (AmigaInfo) getInfo
{
    return [self amiga]->getInfo();
}

- (BOOL) isPoweredOn
{
    return [self amiga]->isPoweredOn();
}

- (BOOL) isPoweredOff
{
    return [self amiga]->isPoweredOff();
}

- (BOOL) isRunning
{
    return [self amiga]->isRunning();
}

- (BOOL) isPaused
{
    return [self amiga]->isPaused();
}

- (void) run
{
    [self amiga]->run();
}

- (void) pause
{
    [self amiga]->pause();
}

- (void) suspend
{
    return [self amiga]->suspend();
}

- (void) resume
{
    return [self amiga]->resume();
}

- (void) requestAutoSnapshot
{
    [self amiga]->requestAutoSnapshot();
}

- (void) requestUserSnapshot
{
    [self amiga]->requestUserSnapshot();
}

- (SnapshotProxy *) latestAutoSnapshot
{
    Snapshot *snapshot = [self amiga]->latestAutoSnapshot();
    return [SnapshotProxy make:snapshot];
}

- (SnapshotProxy *) latestUserSnapshot
{
    Snapshot *snapshot = [self amiga]->latestUserSnapshot();
    return [SnapshotProxy make:snapshot];
}

- (void) loadFromSnapshot:(SnapshotProxy *)proxy
{
    [self amiga]->loadFromSnapshotSafe([proxy snapshot]);
}

- (NSInteger) getConfig:(Option)opt
{
    return [self amiga]->getConfigItem(opt);
}

- (NSInteger) getConfig:(Option)opt id:(NSInteger)id
{
    return [self amiga]->getConfigItem(opt, id);
}

- (NSInteger) getConfig:(Option)opt drive:(NSInteger)id
{
    return [self amiga]->getConfigItem(opt, (long)id);
}

- (BOOL) configure:(Option)opt value:(NSInteger)val
{
    return [self amiga]->configure(opt, val);
}

- (BOOL) configure:(Option)opt enable:(BOOL)val
{
    return [self amiga]->configure(opt, val ? 1 : 0);
}

- (BOOL) configure:(Option)opt id:(NSInteger)id value:(NSInteger)val
{
    return [self amiga]->configure(opt, id, val);
}

- (BOOL) configure:(Option)opt id:(NSInteger)id enable:(BOOL)val
{
    return [self amiga]->configure(opt, id, val ? 1 : 0);
}

- (BOOL) configure:(Option)opt drive:(NSInteger)id value:(NSInteger)val
{
    return [self amiga]->configure(opt, (long)id, val);
}

- (BOOL) configure:(Option)opt drive:(NSInteger)id enable:(BOOL)val
{
    return [self amiga]->configure(opt, (long)id, val ? 1 : 0);
}

- (void) addListener:(const void *)sender function:(Callback *)func
{
    [self amiga]->messageQueue.addListener(sender, func);
}

- (void) removeListener:(const void *)sender
{
    [self amiga]->messageQueue.removeListener(sender);
}

- (Message)message
{
    return [self amiga]->messageQueue.get();
}

- (void) stopAndGo
{
    [self amiga]->stopAndGo();
}

- (void) stepInto
{
    [self amiga]->stepInto();
}

- (void) stepOver
{
    [self amiga]->stepOver();
}

- (BOOL) warp
{
    return [self amiga]->inWarpMode();
}

- (void) warpOn
{
    [self amiga]->setWarp(true);
}

- (void) warpOff
{
    [self amiga]->setWarp(false);
}

@end
