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

struct AgnusWrapper { Agnus *agnus; };
struct AmigaFileWrapper { AmigaFile *file; };
struct AmigaWrapper { Amiga *amiga; };
struct BlitterWrapper { Blitter *blitter; };
struct CIAWrapper { CIA *cia; };
struct ControlPortWrapper { ControlPort *port; };
struct CopperWrapper { Copper *copper; };
struct CPUWrapper { CPU *cpu; };
struct DiskControllerWrapper { DiskController *controller; };
struct DriveWrapper { Drive *drive; };
struct DmaDebuggerWrapper { DmaDebugger *dmaDebugger; };
struct DeniseWrapper { Denise *denise; };
struct FSDeviceWrapper { FSDevice *device; };
struct JoystickWrapper { Joystick *joystick; };
struct KeyboardWrapper { Keyboard *keyboard; };
struct MemWrapper { Memory *mem; };
struct MouseWrapper { Mouse *mouse; };
struct PaulaWrapper { Paula *paula; };
struct ScreenRecorderWrapper { ScreenRecorder *screenRecorder; };
struct SerialPortWrapper { SerialPort *port; };

//
// Base Proxy
//

@implementation Proxy

- (instancetype) initWith:(void *)ref
{
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

- (void) dump
{
    [self agnus]->dump();
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

- (void) dump
{
    [self copper]->dump();
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

- (void) dump
{
    [self blitter]->dump();
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
// Paula
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

- (void) dump
{
    [self paula]->muxer.dump();
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
// Mouse
//

@implementation MouseProxy

- (instancetype) initWithMouse:(Mouse *)mouse
{
    if (self = [super init]) {
        wrapper = new MouseWrapper();
        wrapper->mouse = mouse;
    }
    return self;
}

- (void) dump
{
    wrapper->mouse->dump();
}

- (void) setXY:(NSPoint)pos
{
    wrapper->mouse->setXY((double)pos.x, (double)pos.y);
}

- (void) setDeltaXY:(NSPoint)pos
{
    wrapper->mouse->setDeltaXY((double)pos.x, (double)pos.y);
}

- (void) trigger:(GamePadAction)event
{
    wrapper->mouse->trigger(event);
}

@end


//
// Joystick
//

@implementation JoystickProxy

- (instancetype) initWithJoystick:(Joystick *)joystick
{
    if (self = [super init]) {
        wrapper = new JoystickWrapper();
        wrapper->joystick = joystick;
    }
    return self;
}

- (void) dump
{
    wrapper->joystick->dump();
}

- (void) trigger:(GamePadAction)event
{
    wrapper->joystick->trigger(event);
}

- (BOOL) autofire
{
    return wrapper->joystick->getAutofire();
}

- (void) setAutofire:(BOOL)value
{
    return wrapper->joystick->setAutofire(value);
}

- (NSInteger) autofireBullets
{
    return (NSInteger)wrapper->joystick->getAutofireBullets();
}

- (void) setAutofireBullets:(NSInteger)value
{
    wrapper->joystick->setAutofireBullets((int)value);
}

- (float) autofireFrequency
{
    return wrapper->joystick->getAutofireFrequency();
}

- (void) setAutofireFrequency:(float)value
{
    wrapper->joystick->setAutofireFrequency(value);
}

@end


//
// Control port
//

@implementation ControlPortProxy

@synthesize mouse;
@synthesize joystick;

- (instancetype) initWithControlPort:(ControlPort *)port
{
    if (self = [super init]) {
        wrapper = new ControlPortWrapper();
        wrapper->port = port;
        joystick = [[JoystickProxy alloc] initWithJoystick:&port->joystick];
        mouse = [[MouseProxy alloc] initWithMouse:&port->mouse];
    }
    return self;
}

- (void) dump
{
    wrapper->port->dump();
}

- (ControlPortInfo) getInfo
{
    return wrapper->port->getInfo();
}

@end


//
// Serial port
//

@implementation SerialPortProxy

- (instancetype) initWithSerialPort:(SerialPort *)port
{
    if (self = [super init]) {
        wrapper = new SerialPortWrapper();
        wrapper->port = port;
    }
    return self;
}

- (void) dump
{
    wrapper->port->dump();
}

- (SerialPortInfo) getInfo
{
    return wrapper->port->getInfo();
}

@end


//
// Keyboard
//

@implementation KeyboardProxy

- (instancetype) initWithKeyboard:(Keyboard *)keyboard
{
    if (self = [super init]) {
        wrapper = new KeyboardWrapper();
        wrapper->keyboard = keyboard;
    }
    return self;
}

- (void) dump
{
    wrapper->keyboard->dump();
}

- (BOOL) keyIsPressed:(NSInteger)keycode
{
    return wrapper->keyboard->keyIsPressed(keycode);
}

- (void) pressKey:(NSInteger)keycode
{
    wrapper->keyboard->pressKey(keycode);
}

- (void) releaseKey:(NSInteger)keycode
{
    wrapper->keyboard->releaseKey(keycode);
}

- (void) releaseAllKeys
{
    wrapper->keyboard->releaseAllKeys();
}

@end


//
// Disk Controller
//

@implementation DiskControllerProxy

- (instancetype) initWithDiskController:(DiskController *)controller
{
    if (self = [super init]) {
        wrapper = new DiskControllerWrapper();
        wrapper->controller = controller;
    }
    return self;
}

- (void) dump
{
    wrapper->controller->dump();
}

- (DiskControllerConfig) getConfig
{
    return wrapper->controller->getConfig();
}

- (DiskControllerInfo) getInfo
{
    return wrapper->controller->getInfo();
}

- (NSInteger) selectedDrive
{
    return wrapper->controller->getSelected();
}

- (DriveState) state
{
    return wrapper->controller->getState();
}

- (BOOL) isSpinning
{
    return wrapper->controller->spinning();
}

- (void) eject:(NSInteger)nr
{
    wrapper->controller->ejectDisk(nr);
}

- (void) insert:(NSInteger)nr file:(DiskFileProxy *)fileProxy
{
    AmigaFileWrapper *fileWrapper = [fileProxy wrapper];
    wrapper->controller->insertDisk((DiskFile *)(fileWrapper->file), nr);
}

- (void) setWriteProtection:(NSInteger)nr value:(BOOL)value
{
    wrapper->controller->setWriteProtection(nr, value);
}

@end


//
// Drive
//

@implementation DriveProxy

@synthesize wrapper;

- (instancetype) initWithDrive:(Drive *)drive
{
    if (self = [super init]) {
        wrapper = new DriveWrapper();
        wrapper->drive = drive;
    }
    return self;
}

- (DriveInfo) getInfo
{
    return wrapper->drive->getInfo();
}

- (NSInteger) nr
{
    return wrapper->drive->getNr();
}

- (void) dump
{
    wrapper->drive->dump();
}

- (BOOL) hasDisk
{
    return wrapper->drive->hasDisk();
}

- (BOOL) hasDDDisk
{
    return wrapper->drive->hasDDDisk();
}

- (BOOL) hasHDDisk
{
    return wrapper->drive->hasHDDisk();
}

- (BOOL) hasWriteProtectedDisk
{
    return wrapper->drive->hasWriteProtectedDisk();
}

- (void) setWriteProtection:(BOOL)value
{
    wrapper->drive->setWriteProtection(value);
}

- (void) toggleWriteProtection
{
    wrapper->drive->toggleWriteProtection();
}

- (BOOL) isInsertable:(DiskDiameter)type density:(DiskDensity)density
{
    return wrapper->drive->isInsertable(type, density);
}

- (BOOL) isModifiedDisk
{
    return wrapper->drive->hasModifiedDisk();
}

- (void) setModifiedDisk:(BOOL)value
{
    wrapper->drive->setModifiedDisk(value);
}

- (BOOL) motor
{
    return wrapper->drive->getMotor();
}

- (NSInteger) cylinder
{
    return wrapper->drive->getCylinder();
}

- (u64) fnv
{
    return wrapper->drive->fnv();
}

- (ADFFileProxy *)convertDisk
{
    return nullptr;
}
@end


//
// FSDevice
//

@implementation FSDeviceProxy

- (instancetype) initWithDevice:(FSDevice *)volume
{
    if (self = [super init]) {
        wrapper = new FSDeviceWrapper();
        wrapper->device = volume;
    }
    return self;
}

+ (instancetype) make:(FSDevice *)volume
{
    if (volume == nullptr) { return nil; }
    
    FSDeviceProxy *proxy = [[self alloc] initWithDevice: volume];
    return proxy;
}

+ (instancetype) makeWithADF:(ADFFileProxy *)fileProxy
{
    ErrorCode error;
    AmigaFileWrapper *adf = [fileProxy wrapper];

    FSDevice *volume = FSDevice::makeWithADF((ADFFile *)(adf->file), &error);
    return [self make:volume];
}

+ (instancetype) makeWithHDF:(HDFFileProxy *)fileProxy
{
    ErrorCode error;
    AmigaFileWrapper *hdf = [fileProxy wrapper];

    FSDevice *volume = FSDevice::makeWithHDF((HDFFile *)(hdf->file), &error);
    return [self make:volume];
}

- (FSVolumeType) dos
{
    return wrapper->device->dos();
}

- (NSInteger) numCyls
{
    return wrapper->device->getNumCyls();
}

- (NSInteger) numHeads
{
    return wrapper->device->getNumHeads();
}

- (NSInteger) numTracks
{
    return wrapper->device->getNumTracks();
}

- (NSInteger) numSectors
{
    return wrapper->device->getNumSectors();
}

- (NSInteger) numBlocks
{
    return wrapper->device->getNumBlocks();
}

- (void)killVirus
{
    wrapper->device->killVirus();
}

- (FSBlockType) blockType:(NSInteger)blockNr
{
    return wrapper->device->blockType(blockNr);
}

- (FSItemType) itemType:(NSInteger)blockNr pos:(NSInteger)pos
{
    return wrapper->device->itemType(blockNr, pos);
}

- (FSErrorReport) check:(BOOL)strict
{
    return wrapper->device->check(strict);
}

- (ErrorCode) check:(NSInteger)nr
                pos:(NSInteger)pos
           expected:(unsigned char *)exp
             strict:(BOOL)strict
{
    return wrapper->device->check(nr, pos, exp, strict);
}

- (BOOL) isCorrupted:(NSInteger)blockNr
{
    return wrapper->device->isCorrupted((u32)blockNr);
}

- (NSInteger) getCorrupted:(NSInteger)blockNr
{
    return wrapper->device->getCorrupted((u32)blockNr);
}

- (NSInteger) nextCorrupted:(NSInteger)blockNr
{
    return wrapper->device->nextCorrupted((u32)blockNr);
}

- (NSInteger) prevCorrupted:(NSInteger)blockNr
{
    return wrapper->device->prevCorrupted((u32)blockNr);
}

- (void) printDirectory:(BOOL) recursive
{
    return wrapper->device->printDirectory(recursive);
}

- (NSInteger) readByte:(NSInteger)block offset:(NSInteger)offset
{
    return wrapper->device->readByte(block, (u32)offset);
}

- (ErrorCode) export:(NSString *)path
{
    return wrapper->device->exportDirectory([path fileSystemRepresentation]);
}

- (void) dump
{
    wrapper->device->dump();
}
@end

//
// AmigaFile
//

@implementation AmigaFileProxy

- (instancetype) initWithFile:(AmigaFile *)file
{
    if (file == nil) {
        return nil;
    }
    if (self = [super init]) {
        wrapper = new AmigaFileWrapper();
        wrapper->file = file;
    }
    return self;
}

+ (AmigaFileProxy *) makeWithFile:(AmigaFile *)file
{
    if (file == nil) {
        return nil;
    }
    return [[self alloc] initWithFile:file];
}

- (void)setPath:(NSString *)path
{
    AmigaFile *file = (AmigaFile *)([self wrapper]->file);
    file->setPath([path fileSystemRepresentation]);
}

- (AmigaFileWrapper *)wrapper
{
    return wrapper;
}

- (FileType)type
{
    return wrapper->file->fileType();
}

- (NSInteger) sizeOnDisk
{
    return wrapper->file->sizeOnDisk();
}

- (u64) fnv
{
    return wrapper->file->fnv();
}

- (void) readFromBuffer:(const void *)buffer length:(NSInteger)length
{
    ErrorCode error;
    wrapper->file->readFromBuffer((const u8 *)buffer, length, &error);
}

- (NSInteger) writeToBuffer:(void *)buffer
{
    return wrapper->file->writeToBuffer((u8 *)buffer);
}

- (void) dealloc
{
    if (wrapper) {
        if (wrapper->file) delete wrapper->file;
        delete wrapper;
    }
}

@end


//
// Snapshot
//

@implementation SnapshotProxy

+ (instancetype) make:(Snapshot *)snapshot
{
    if (snapshot == nullptr) { return nil; }
    
    SnapshotProxy *proxy = [[self alloc] initWithFile:snapshot];
    proxy->preview = nullptr;
    
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
    Amiga *amiga = [proxy wrapper]->amiga;
    
    amiga->suspend();
    Snapshot *snapshot = Snapshot::makeWithAmiga(amiga);
    amiga->resume();
    
    return [self make:snapshot];
}

- (NSImage *)previewImage
{
    if (preview != nullptr) { return preview; }
    
    // Create preview image
    const Thumbnail &thumbnail = ((Snapshot *)wrapper->file)->getThumbnail();
    
    // NSInteger width = thumbnail.width;
    // NSInteger height = thumbnail.height;
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

- (time_t)timeStamp __attribute__ ((deprecated))
{
    return ((Snapshot *)wrapper->file)->getThumbnail().timestamp;
}

- (NSData *)data
{
    Snapshot *snapshot = (Snapshot *)wrapper->file;
    return [NSData dataWithBytes: (void *)snapshot->getHeader()
                          length: snapshot->sizeOnDisk()];
}
    
@end


//
// DiskFileProxy
//

@implementation DiskFileProxy

- (FSVolumeType)dos
{
    return ((DiskFile *)wrapper->file)->getDos();
}

- (DiskDiameter)diskType
{
    return ((DiskFile *)wrapper->file)->getDiskDiameter();
}

- (DiskDensity)diskDensity
{
    return ((DiskFile *)wrapper->file)->getDiskDensity();
}

- (NSInteger)numCyls
{
    return ((DiskFile *)wrapper->file)->numCyls();
}

- (NSInteger)numSides
{
    return ((DiskFile *)wrapper->file)->numSides();
}

- (NSInteger)numTracks
{
    return ((DiskFile *)wrapper->file)->numTracks();
}

- (NSInteger)numSectors
{
    return ((DiskFile *)wrapper->file)->numSectors();
}

- (NSInteger)numBlocks
{
    return ((DiskFile *)wrapper->file)->numBlocks();
}

- (BootBlockType)bootBlockType
{
    return ((DiskFile *)wrapper->file)->bootBlockType();
}

- (NSString *)bootBlockName
{
    const char *str = ((DiskFile *)wrapper->file)->bootBlockName();
    return str ? [NSString stringWithUTF8String:str] : nullptr;
}

- (BOOL)hasVirus
{
    return ((DiskFile *)wrapper->file)->hasVirus();
}

- (void)killVirus
{
    ((DiskFile *)wrapper->file)->killVirus();
}

- (NSInteger)readByte:(NSInteger)block offset:(NSInteger)offset
{
    return ((DiskFile *)wrapper->file)->readByte(block, offset);
}

- (void)readSector:(unsigned char *)dst block:(NSInteger)nr
{
    ((DiskFile *)wrapper->file)->readSector(dst, nr);
}

- (void)readSectorHex:(char *)dst block:(NSInteger)block count:(NSInteger)count
{
    ((DiskFile *)wrapper->file)->readSectorHex(dst, block, count);
}

@end


//
// ADFFileProxy
//

@implementation ADFFileProxy

+ (BOOL)isADFFile:(NSString *)path
{
    return ADFFile::isADFFile([path fileSystemRepresentation]);
}

+ (instancetype)make:(ADFFile *)archive
{
    if (archive == nullptr) return nil;
    return [[self alloc] initWithFile:archive];
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
    Drive *drive = [proxy wrapper]->drive;
    ADFFile *archive = ADFFile::makeWithDrive(drive);
    return archive ? [self make: archive] : nil;
}

- (void)formatDisk:(FSVolumeType)fs bootBlock:(NSInteger)bootBlockID
{
    ((ADFFile *)wrapper->file)->formatDisk(fs, bootBlockID);
}

@end


//
// HDFFileProxy
//

@implementation HDFFileProxy

+ (BOOL)isHDFFile:(NSString *)path
{
    return HDFFile::isHDFFile([path fileSystemRepresentation]);
}

+ (instancetype) make:(HDFFile *)archive
{
    if (archive == nullptr) return nil;
    return [[self alloc] initWithFile:archive];
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
    return ((HDFFile *)wrapper->file)->numBlocks();
}

@end


//
// EXTFileProxy
//

@implementation EXTFileProxy

+ (BOOL)isEXTFile:(NSString *)path
{
    return EXTFile::isEXTFile([path fileSystemRepresentation]);
}

+ (instancetype) make:(EXTFile *)archive
{
    if (archive == nullptr) return nil;
    return [[self alloc] initWithFile:archive];
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

+ (BOOL)isIMGFile:(NSString *)path
{
    return IMGFile::isIMGFile([path fileSystemRepresentation]);
}

+ (instancetype) make:(IMGFile *)archive
{
    if (archive == nullptr) return nil;
    return [[self alloc] initWithFile:archive];
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
    Drive *d = [drive wrapper]->drive;
    IMGFile *archive = IMGFile::makeWithDisk(d->disk);
    return archive ? [self make: archive] : nil;
}

@end


//
// DMSFileProxy
//

@implementation DMSFileProxy

+ (BOOL)isDMSFile:(NSString *)path
{
    return DMSFile::isDMSFile([path fileSystemRepresentation]);
}

+ (instancetype) make:(DMSFile *)archive
{
    if (archive == nullptr) return nil;
    return [[self alloc] initWithFile:archive];
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
    DMSFile *archive = (DMSFile *)wrapper->file;
    return [ADFFileProxy make:archive->adf];
}

@end


//
// EXEFileProxy
//

@implementation EXEFileProxy

+ (BOOL)isEXEFile:(NSString *)path
{
    return EXEFile::isEXEFile([path fileSystemRepresentation]);
}
+ (instancetype) make:(EXEFile *)archive
{
    if (archive == nullptr) return nil;
    return [[self alloc] initWithFile:archive];
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
    EXEFile *archive = (EXEFile *)wrapper->file;
    return [ADFFileProxy make:archive->adf];
}

@end


//
// DIRFileProxy
//

@implementation DIRFileProxy

+ (BOOL)isDIRFile:(NSString *)path
{
    return DIRFile::isDIRFile([path fileSystemRepresentation]);
}

+ (instancetype) make:(DIRFile *)archive
{
    if (archive == nullptr) return nil;
    return [[self alloc] initWithFile:archive];
}

+ (instancetype) makeWithFile:(NSString *)path error:(ErrorCode *)err
{
    DIRFile *archive = AmigaFile::make <DIRFile> ([path fileSystemRepresentation], err);
    return [self make: archive];
}

- (ADFFileProxy *)adf
{
    DIRFile *archive = (DIRFile *)wrapper->file;
    return [ADFFileProxy make:archive->adf];
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
    wrapper = new AmigaWrapper();
    wrapper->amiga = amiga;
    
    // Create sub proxys
    agnus = [[AgnusProxy alloc] initWith:&amiga->agnus];
    blitter = [[BlitterProxy alloc] initWith:&amiga->agnus.blitter];
    breakpoints = [[GuardsProxy alloc] initWith:&amiga->cpu.debugger.breakpoints];
    ciaA = [[CIAProxy alloc] initWith:&amiga->ciaA];
    ciaB = [[CIAProxy alloc] initWith:&amiga->ciaB];
    controlPort1 = [[ControlPortProxy alloc] initWithControlPort:&amiga->controlPort1];
    controlPort2 = [[ControlPortProxy alloc] initWithControlPort:&amiga->controlPort2];
    copper = [[CopperProxy alloc] initWith:&amiga->agnus.copper];
    cpu = [[CPUProxy alloc] initWith:&amiga->cpu];
    denise = [[DeniseProxy alloc] initWith:&amiga->denise];
    df0 = [[DriveProxy alloc] initWithDrive:&amiga->df0];
    df1 = [[DriveProxy alloc] initWithDrive:&amiga->df1];
    df2 = [[DriveProxy alloc] initWithDrive:&amiga->df2];
    df3 = [[DriveProxy alloc] initWithDrive:&amiga->df3];
    diskController = [[DiskControllerProxy alloc] initWithDiskController:&amiga->paula.diskController];
    dmaDebugger = [[DmaDebuggerProxy alloc] initWith:&amiga->agnus.dmaDebugger];
    keyboard = [[KeyboardProxy alloc] initWithKeyboard:&amiga->keyboard];
    mem = [[MemProxy alloc] initWith:&amiga->mem];
    paula = [[PaulaProxy alloc] initWith:&amiga->paula];
    screenRecorder = [[ScreenRecorderProxy alloc] initWith:&amiga->denise.screenRecorder];
    serialPort = [[SerialPortProxy alloc] initWithSerialPort:&amiga->serialPort];
    watchpoints = [[GuardsProxy alloc] initWith:&amiga->cpu.debugger.watchpoints];

    return self;
}

- (void) kill
{
    assert(wrapper->amiga != nullptr);
    NSLog(@"AmigaProxy::kill");
    
    // Kill the emulator
    delete wrapper->amiga;
    wrapper->amiga = nullptr;
}

- (BOOL) isReleaseBuild
{
    return releaseBuild();
}

- (BOOL) debugMode
{
    return wrapper->amiga->inDebugMode();
}

- (void) setDebug:(BOOL)enable
{
    wrapper->amiga->setDebug(enable);
}

- (void) enableDebugging
{
    wrapper->amiga->enableDebugMode();
}

- (void) disableDebugging
{
    wrapper->amiga->disableDebugMode();
}

- (void) setInspectionTarget:(EventID)id
{
    wrapper->amiga->setInspectionTarget(id);
}

- (void) clearInspectionTarget
{
    wrapper->amiga->clearInspectionTarget();
}

- (BOOL) isReady:(ErrorCode *)error
{
    return wrapper->amiga->isReady(error);
}

- (BOOL) isReady
{
    return wrapper->amiga->isReady();
}

- (void) powerOn
{
    wrapper->amiga->powerOn();
}

- (void) powerOff
{
    wrapper->amiga->powerOff();
}

- (void) hardReset
{
    wrapper->amiga->reset(true);
}

- (void) softReset
{
    wrapper->amiga->reset(false);
}

- (void) dump
{
    wrapper->amiga->dump();
}

- (AmigaInfo) getInfo
{
    return wrapper->amiga->getInfo();
}

- (BOOL) isPoweredOn
{
    return wrapper->amiga->isPoweredOn();
}

- (BOOL) isPoweredOff
{
    return wrapper->amiga->isPoweredOff();
}

- (BOOL) isRunning
{
    return wrapper->amiga->isRunning();
}

- (BOOL) isPaused
{
    return wrapper->amiga->isPaused();
}

- (void) run
{
    wrapper->amiga->run();
}

- (void) pause
{
    wrapper->amiga->pause();
}

- (void) suspend
{
    return wrapper->amiga->suspend();
}

- (void) resume
{
    return wrapper->amiga->resume();
}

- (void) requestAutoSnapshot
{
    wrapper->amiga->requestAutoSnapshot();
}

- (void) requestUserSnapshot
{
    wrapper->amiga->requestUserSnapshot();
}

- (SnapshotProxy *) latestAutoSnapshot
{
    Snapshot *snapshot = wrapper->amiga->latestAutoSnapshot();
    return [SnapshotProxy make:snapshot];
}

- (SnapshotProxy *) latestUserSnapshot
{
    Snapshot *snapshot = wrapper->amiga->latestUserSnapshot();
    return [SnapshotProxy make:snapshot];
}

- (void) loadFromSnapshot:(SnapshotProxy *)proxy
{
    Snapshot *snapshot = (Snapshot *)([proxy wrapper]->file);
    wrapper->amiga->loadFromSnapshotSafe(snapshot);
}

- (NSInteger) getConfig:(Option)opt
{
    return wrapper->amiga->getConfigItem(opt);
}

- (NSInteger) getConfig:(Option)opt id:(NSInteger)id
{
    return wrapper->amiga->getConfigItem(opt, id);
}

- (NSInteger) getConfig:(Option)opt drive:(NSInteger)id
{
    return wrapper->amiga->getConfigItem(opt, (long)id);
}

- (BOOL) configure:(Option)opt value:(NSInteger)val
{
    return wrapper->amiga->configure(opt, val);
}

- (BOOL) configure:(Option)opt enable:(BOOL)val
{
    return wrapper->amiga->configure(opt, val ? 1 : 0);
}

- (BOOL) configure:(Option)opt id:(NSInteger)id value:(NSInteger)val
{
    return wrapper->amiga->configure(opt, id, val);
}

- (BOOL) configure:(Option)opt id:(NSInteger)id enable:(BOOL)val
{
    return wrapper->amiga->configure(opt, id, val ? 1 : 0);
}

- (BOOL) configure:(Option)opt drive:(NSInteger)id value:(NSInteger)val
{
    return wrapper->amiga->configure(opt, (long)id, val);
}

- (BOOL) configure:(Option)opt drive:(NSInteger)id enable:(BOOL)val
{
    return wrapper->amiga->configure(opt, (long)id, val ? 1 : 0);
}

- (void) addListener:(const void *)sender function:(Callback *)func
{
    wrapper->amiga->messageQueue.addListener(sender, func);
}

- (void) removeListener:(const void *)sender
{
    wrapper->amiga->messageQueue.removeListener(sender);
}

- (Message)message
{
    return wrapper->amiga->messageQueue.get();
}

- (void) stopAndGo
{
    wrapper->amiga->stopAndGo();
}

- (void) stepInto
{
    wrapper->amiga->stepInto();
}

- (void) stepOver
{
    wrapper->amiga->stepOver();
}

- (BOOL) warp
{
    return wrapper->amiga->inWarpMode();
}

- (void) warpOn
{
    wrapper->amiga->setWarp(true);
}

- (void) warpOff
{
    wrapper->amiga->setWarp(false);
}

@end
