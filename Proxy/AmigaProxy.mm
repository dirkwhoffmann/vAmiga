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
struct CPUWrapper { CPU *cpu; };
struct CIAWrapper { CIA *cia; };
struct MemWrapper { Memory *mem; };
struct AgnusWrapper { Agnus *agnus; };
struct DeniseWrapper { Denise *denise; };
struct PaulaWrapper { Paula *paula; };
struct AmigaControlPortWrapper { ControlPort *port; };
struct AmigaSerialPortWrapper { SerialPort *port; };
struct MouseWrapper { Mouse *mouse; };
struct JoystickWrapper { Joystick *joystick; };
struct KeyboardWrapper { Keyboard *keyboard; };
struct DiskControllerWrapper { DiskController *controller; };
struct AmigaDriveWrapper { Drive *drive; };
struct AmigaFileWrapper { AmigaFile *file; };
struct ADFFileWrapper { ADFFile *adf; };


//
// CPU proxy
//

@implementation CPUProxy

// Constructing
- (instancetype) initWithCPU:(CPU *)cpu
{
    if (self = [super init]) {
        wrapper = new CPUWrapper();
        wrapper->cpu = cpu;
    }
    return self;
}

- (void) dump
{
    wrapper->cpu->dump();
}
- (CPUInfo) getInfo
{
    return wrapper->cpu->getInfo();
}
- (u32) getPC
{
    return wrapper->cpu->getPC();
}
- (DisassembledInstr) getInstrInfo:(NSInteger)index
{
    return wrapper->cpu->getInstrInfo(index);
}
- (DisassembledInstr) getLoggedInstrInfo:(NSInteger)index
{
    return wrapper->cpu->getLoggedInstrInfo(index);
}
- (i64) clock
{
    return wrapper->cpu->getMasterClock(); 
}
- (i64) cycles
{
    return wrapper->cpu->getCpuClock();
}
- (NSInteger) numberOfBreakpoints
{
    return wrapper->cpu->debugger.breakpoints.elements();
}
- (u32) breakpointAddr:(NSInteger)nr
{
    return wrapper->cpu->debugger.breakpoints.guardAddr(nr);
}
- (BOOL) breakpointIsEnabled:(NSInteger)nr
{
    return wrapper->cpu->debugger.breakpoints.isEnabled(nr);
}
- (BOOL) breakpointIsDisabled:(NSInteger)nr
{
    return wrapper->cpu->debugger.breakpoints.isDisabled(nr);
}
- (void) removeBreakpoint:(NSInteger)nr
{
    return wrapper->cpu->debugger.breakpoints.remove(nr);
}
- (void) breakpointSetEnable:(NSInteger)nr value:(BOOL)val
{
    wrapper->cpu->debugger.breakpoints.setEnable(nr, val);
}
- (BOOL) breakpointIsSetAt:(u32)addr
{
    return wrapper->cpu->debugger.breakpoints.isSetAt(addr);
}
- (BOOL) breakpointIsSetAndEnabledAt:(u32)addr
{
    return wrapper->cpu->debugger.breakpoints.isSetAndEnabledAt(addr);
}
- (BOOL) breakpointIsSetAndDisabledAt:(u32)addr
{
    return wrapper->cpu->debugger.breakpoints.isSetAndDisabledAt(addr);
}
- (void) breakpointSetEnableAt:(u32)addr value:(BOOL)val
{
    wrapper->cpu->debugger.breakpoints.setEnableAt(addr, val);
}
- (void) addBreakpointAt:(u32)addr
{
    wrapper->cpu->debugger.breakpoints.addAt(addr);
}
- (void) removeBreakpointAt:(u32)addr
{
    wrapper->cpu->debugger.breakpoints.removeAt(addr);
}
- (NSInteger) numberOfWatchpoints
{
    return wrapper->cpu->debugger.watchpoints.elements();
}
- (u32) watchpointAddr:(NSInteger)nr
{
    return wrapper->cpu->debugger.watchpoints.guardAddr(nr);
}
- (BOOL) watchpointIsEnabled:(NSInteger)nr
{
    return wrapper->cpu->debugger.watchpoints.isEnabled(nr);
}
- (BOOL) watchpointIsDisabled:(NSInteger)nr
{
    return wrapper->cpu->debugger.watchpoints.isDisabled(nr);
}
- (void) removeWatchpoint:(NSInteger)nr
{
    return wrapper->cpu->debugger.watchpoints.remove(nr);
}
- (void) watchpointSetEnable:(NSInteger)nr value:(BOOL)val
{
    wrapper->cpu->debugger.watchpoints.setEnable(nr, val);
}
- (BOOL) watchpointIsSetAt:(u32)addr
{
    return wrapper->cpu->debugger.watchpoints.isSetAt(addr);
}
- (BOOL) watchpointIsSetAndEnabledAt:(u32)addr
{
    return wrapper->cpu->debugger.watchpoints.isSetAndEnabledAt(addr);
}
- (BOOL) watchpointIsSetAndDisabledAt:(u32)addr
{
    return wrapper->cpu->debugger.watchpoints.isSetAndDisabledAt(addr);
}
- (void) addWatchpointAt:(u32)addr
{
    wrapper->cpu->debugger.watchpoints.addAt(addr);
}
- (void) removeWatchpointAt:(u32)addr
{
    wrapper->cpu->debugger.watchpoints.removeAt(addr);
}
- (NSInteger) loggedInstructions
{
    return wrapper->cpu->debugger.loggedInstructions();
}
- (void) clearLog
{
    return wrapper->cpu->debugger.clearLog();
}
- (DisassembledInstr)disassembleInstr:(u32)addr
{
    return wrapper->cpu->disassembleInstr(addr);
}

@end


//
// CIA proxy
//

@implementation CIAProxy

// Constructing
- (instancetype) initWithCIA:(CIA *)cia
{
    if (self = [super init]) {
        wrapper = new CIAWrapper();
        wrapper->cia = cia;
    }
    return self;
}
- (CIAInfo) getInfo
{
    return wrapper->cia->getInfo();
}
- (void) dumpConfig
{
    wrapper->cia->dumpConfig();
}
- (void) dump
{
    wrapper->cia->dump();
}

@end


//
// Memory proxy
//

@implementation MemProxy

- (instancetype) initWithMemory:(Memory *)mem
{
    if (self = [super init]) {
        wrapper = new MemWrapper();
        wrapper->mem = mem;
    }
    return self;
}
- (MemoryConfig) getConfig
{
    return wrapper->mem->getConfig();
}
- (MemoryStats) getStats
{
    return wrapper->mem->getStats();
}
- (void) dump
{
    wrapper->mem->dump();
}
- (BOOL) isBootRom:(RomRevision)rev
{
    return wrapper->mem->isBootRom(rev);
}
- (BOOL) isArosRom:(RomRevision)rev
{
    return wrapper->mem->isArosRom(rev);
}
- (BOOL) isDiagRom:(RomRevision)rev
{
    return wrapper->mem->isDiagRom(rev);
}
- (BOOL) isOrigRom:(RomRevision)rev
{
    return wrapper->mem->isOrigRom(rev);
}
- (BOOL) isHyperionRom:(RomRevision)rev
{
    return wrapper->mem->isHyperionRom(rev);
}
- (BOOL) hasRom
{
    return wrapper->mem->hasKickRom();
}
- (BOOL) hasBootRom
{
    return wrapper->mem->hasBootRom();
}
- (BOOL) hasKickRom
{
    return wrapper->mem->hasKickRom();
}
- (void) deleteRom
{
    wrapper->mem->deleteRom();
}
- (BOOL) isRom:(NSURL *)url
{
    return RomFile::isRomFile([[url path] UTF8String]);
}
- (BOOL) loadRomFromBuffer:(NSData *)data
{
    if (data == NULL) return NO;
    const u8 *bytes = (const u8 *)[data bytes];
    return wrapper->mem->loadRomFromBuffer(bytes, [data length]);
}
- (BOOL) loadRomFromFile:(NSURL *)url
{
    return wrapper->mem->loadRomFromFile([[url path] UTF8String]);
}
- (u64) romFingerprint
{
    return wrapper->mem->romFingerprint();
}
- (RomRevision) romRevision
{
    return wrapper->mem->romRevision();
}
- (NSString *) romTitle
{
    const char *str = wrapper->mem->romTitle();
    return str ? [NSString stringWithUTF8String:str] : NULL;
}
- (NSString *) romVersion
{
    const char *str = wrapper->mem->romVersion();
    return str ? [NSString stringWithUTF8String:str] : NULL;
}
- (NSString *) romReleased
{
    const char *str = wrapper->mem->romReleased();
    return str ? [NSString stringWithUTF8String:str] : NULL;
}
- (BOOL) hasExt
{
    return wrapper->mem->hasExt();
}
- (void) deleteExt
{
    wrapper->mem->deleteExt();
}
- (BOOL) isExt:(NSURL *)url
{
    return ExtFile::isExtFile([[url path] UTF8String]);
}
- (BOOL) loadExtFromBuffer:(NSData *)data
{
    if (data == NULL) return NO;
    const u8 *bytes = (const u8 *)[data bytes];
    return wrapper->mem->loadExtFromBuffer(bytes, [data length]);
}
- (BOOL) loadExtFromFile:(NSURL *)url
{
    return wrapper->mem->loadExtFromFile([[url path] UTF8String]);
}
- (u64) extFingerprint
{
    return wrapper->mem->extFingerprint();
}
- (RomRevision) extRevision
{
    return wrapper->mem->extRevision();
}
- (NSString *) extTitle
{
    const char *str = wrapper->mem->extTitle();
    return str ? [NSString stringWithUTF8String:str] : NULL;
}
- (NSString *) extVersion
{
    const char *str = wrapper->mem->extVersion();
    return str ? [NSString stringWithUTF8String:str] : NULL;
}
- (NSString *) extReleased
{
    const char *str = wrapper->mem->extReleased();
    return str ? [NSString stringWithUTF8String:str] : NULL;
}
- (NSInteger) extStart
{
    return wrapper->mem->getExtStart(); 
}
- (MemorySource *) getMemSrcTable
{
    return wrapper->mem->getMemSrcTable();
}
- (MemorySource) memSrc:(NSInteger)addr
{
    return wrapper->mem->getMemSrc((u32)addr);
}
- (NSInteger) spypeek8:(NSInteger)addr
{
    return wrapper->mem->spypeek8((u32)addr);
}
- (NSInteger) spypeek16:(NSInteger)addr
{
    return wrapper->mem->spypeek16((u32)addr);
}
- (NSString *) ascii:(NSInteger)addr
{
    const char *str = wrapper->mem->ascii((u32)addr);
    return str ? [NSString stringWithUTF8String:str] : NULL;
}
- (NSString *) hex:(NSInteger)addr bytes:(NSInteger)bytes
{
    const char *str = wrapper->mem->hex((u32)addr, bytes);
    return str ? [NSString stringWithUTF8String:str] : NULL;
}

@end


//
// Agnus proxy
//

@implementation AgnusProxy

- (instancetype) initWithAgnus:(Agnus *)agnus
{
    if (self = [super init]) {
        wrapper = new AgnusWrapper();
        wrapper->agnus = agnus;
    }
    return self;
}
- (NSInteger) chipRamLimit
{
    return wrapper->agnus->chipRamLimit();
}
- (void) dump
{
    wrapper->agnus->dump();
}
- (void) dumpEvents
{
    wrapper->agnus->dumpEvents();
}
- (void) dumpCopper
{
    wrapper->agnus->copper.dump();
}
- (void) dumpBlitter
{
    wrapper->agnus->blitter.dump();
}
- (AgnusInfo) getInfo
{
    return wrapper->agnus->getInfo();
}
- (DMADebuggerInfo) getDebuggerInfo
{
    return wrapper->agnus->dmaDebugger.getInfo();
}
- (EventSlotInfo) getEventSlotInfo:(NSInteger)slot
{
    return wrapper->agnus->getEventSlotInfo(slot);
}
- (EventInfo) getEventInfo
{
    return wrapper->agnus->getEventInfo();
}
- (CopperInfo) getCopperInfo
{
    return wrapper->agnus->copper.getInfo();
}
- (BlitterInfo) getBlitterInfo
{
    return wrapper->agnus->blitter.getInfo();
}
- (AgnusStats) getStats
{
    return wrapper->agnus->getStats();
}
- (BOOL) interlaceMode
{
    return wrapper->agnus->frame.interlaced;
}
- (NSInteger) instrCount:(NSInteger)list
{
    return wrapper->agnus->copper.instrCount(list);
}
- (void) adjustInstrCount:(NSInteger)list offset:(NSInteger)offset
{
    wrapper->agnus->copper.adjustInstrCount(list, offset);
}
- (BOOL) isIllegalInstr:(NSInteger)addr
{
    return wrapper->agnus->copper.isIllegalInstr(addr);
}
- (NSString *) disassemble:(NSInteger)addr
{
    const char *str = wrapper->agnus->copper.disassemble(addr);
    return str ? [NSString stringWithUTF8String:str] : NULL;
}
- (NSString *) disassemble:(NSInteger)list instr:(NSInteger)offset
{
    const char *str = wrapper->agnus->copper.disassemble(list, offset);
    return str ? [NSString stringWithUTF8String:str] : NULL;
}
- (void) dmaDebugSetEnable:(BOOL)value
{
    wrapper->agnus->dmaDebugger.setEnabled(value);
}
- (void) dmaDebugSetVisualize:(BusOwner)owner value:(BOOL)value
{
    wrapper->agnus->dmaDebugger.setVisualized(owner, value);
}
- (void) dmaDebugSetColor:(BusOwner)owner r:(double)r g:(double)g b:(double)b
{
    wrapper->agnus->dmaDebugger.setColor(owner, r, g, b);
}
- (void) dmaDebugSetOpacity:(double)value
{
    wrapper->agnus->dmaDebugger.setOpacity(value);
}
- (void) dmaDebugSetDisplayMode:(NSInteger)mode
{
    wrapper->agnus->dmaDebugger.setDisplayMode((DmaDebuggerDisplayMode)mode);
}

@end


//
// Denise proxy
//

@implementation DeniseProxy

- (instancetype) initWithDenise:(Denise *)denise
{
    if (self = [super init]) {
        wrapper = new DeniseWrapper();
        wrapper->denise = denise;
    }
    return self;
}
- (void) dump
{
    wrapper->denise->dump();
}
- (DeniseInfo) getInfo
{
    return wrapper->denise->getInfo();
}
- (void) inspect
{
    wrapper->denise->inspect();
}
- (SpriteInfo) getSpriteInfo:(NSInteger)nr
{
    return wrapper->denise->getSpriteInfo(nr);
}
- (NSInteger) sprDataLines:(NSInteger)nr
{
    return wrapper->denise->getSpriteHeight(nr);
}
- (u64) sprData:(NSInteger)nr line:(NSInteger)line
{
    return wrapper->denise->getSpriteData(nr, line);
}
- (u16) sprColor:(NSInteger)nr reg:(NSInteger)reg
{
    return wrapper->denise->getSpriteColor(nr, reg);
}
- (double) palette
{
   return wrapper->denise->pixelEngine.getPalette();
}
- (void) setPalette:(Palette)p
{
    wrapper->denise->pixelEngine.setPalette(p);
}
- (double) brightness
{
    return wrapper->denise->pixelEngine.getBrightness();
}
- (void) setBrightness:(double)value
{
    wrapper->denise->pixelEngine.setBrightness(value);
}
- (double) saturation
{
    return wrapper->denise->pixelEngine.getSaturation();
}
- (void) setSaturation:(double)value
{
    wrapper->denise->pixelEngine.setSaturation(value);
}
- (double) contrast
{
    return wrapper->denise->pixelEngine.getContrast();
}
- (void) setContrast:(double)value
{
    wrapper->denise->pixelEngine.setContrast(value);
}
- (ScreenBuffer) stableLongFrame
{
    return wrapper->denise->pixelEngine.getStableLongFrame();
}
- (ScreenBuffer) stableShortFrame
{
    return wrapper->denise->pixelEngine.getStableShortFrame();
}
- (i32 *) noise
{
    return wrapper->denise->pixelEngine.getNoise(); 
}

@end


//
// Paula proxy
//

@implementation PaulaProxy

- (instancetype) initWithPaula:(Paula *)paula
{
    if (self = [super init]) {
        wrapper = new PaulaWrapper();
        wrapper->paula = paula;
    }
    return self;
}
- (PaulaInfo) getInfo
{
    return wrapper->paula->getInfo();
}
- (AudioInfo) getAudioInfo
{
    return wrapper->paula->audioUnit.getInfo();
}
- (AudioStats) getAudioStats
{
    return wrapper->paula->audioUnit.getStats();
}
- (UARTInfo) getUARTInfo
{
    return wrapper->paula->uart.getInfo();
}


//
// Audio unit
//

- (void) dump
{
    wrapper->paula->audioUnit.dump();
}
- (u32) sampleRate
{
    return wrapper->paula->audioUnit.getSampleRate();
}
- (void) setSampleRate:(double)rate
{
    wrapper->paula->audioUnit.setSampleRate(rate);
}
- (FilterActivation) filterActivation
{
    return wrapper->paula->audioUnit.getFilterActivation();
}
- (void) setFilterActivation:(FilterActivation)activation
{
    wrapper->paula->audioUnit.setFilterActivation(activation);
}
- (FilterType) filterType
{
    return wrapper->paula->audioUnit.getFilterType();
}
- (void) setFilterType:(FilterType)type
{
    wrapper->paula->audioUnit.setFilterType(type);
}
- (NSInteger) ringbufferSize
{
    return wrapper->paula->audioUnit.ringbufferSize();
}
- (double) ringbufferDataL:(NSInteger)offset
{
    return (double)wrapper->paula->audioUnit.ringbufferDataL(offset);
}
- (double) ringbufferDataR:(NSInteger)offset
{
    return wrapper->paula->audioUnit.ringbufferDataR(offset);
}
/*
- (double) ringbufferData:(NSInteger)offset
{
    return wrapper->paula->audioUnit.ringbufferData(offset);
}
*/
- (double) fillLevel
{
    return wrapper->paula->audioUnit.fillLevel();
}
- (void) readMonoSamples:(float *)target size:(NSInteger)n
{
    wrapper->paula->audioUnit.readMonoSamples(target, n);
}
- (void) readStereoSamples:(float *)target1 buffer2:(float *)target2 size:(NSInteger)n
{
    wrapper->paula->audioUnit.readStereoSamples(target1, target2, n);
}
- (void) readStereoSamplesInterleaved:(float *)target size:(NSInteger)n
{
    wrapper->paula->audioUnit.readStereoSamplesInterleaved(target, n);
}
- (void) rampUp
{
    wrapper->paula->audioUnit.rampUp();
}
- (void) rampUpFromZero
{
    wrapper->paula->audioUnit.rampUpFromZero();
}
- (void) rampDown
{
    wrapper->paula->audioUnit.rampDown();
}
- (float) drawWaveformL:(unsigned *)buffer size:(NSSize)size scale:(float)s
{
    int w = int(size.width);
    int h = int(size.height);
    return wrapper->paula->audioUnit.drawWaveform(buffer, w, h, true, s, 0xFFFFFFFF);
}

@end


//
// Control port
//

@implementation ControlPortProxy

- (instancetype) initWithControlPort:(ControlPort *)port
{
    if (self = [super init]) {
        wrapper = new AmigaControlPortWrapper();
        wrapper->port = port;
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
- (void) connectDevice:(ControlPortDevice)value
{
    wrapper->port->connectDevice(value);
}

@end


//
// Serial port
//

@implementation SerialPortProxy

- (instancetype) initWithSerialPort:(SerialPort *)port
{
    if (self = [super init]) {
        wrapper = new AmigaSerialPortWrapper();
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
- (void) setDevice:(SerialPortDevice)value
{
    wrapper->port->setDevice(value);
}

@end


//
// Mouse proxy
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
    wrapper->mouse->setXY((i64)pos.x, (i64)pos.y);
}
- (void) setLeftButton:(BOOL)value
{
    wrapper->mouse->setLeftButton(value);
}
- (void) setRightButton:(BOOL)value
{
    wrapper->mouse->setRightButton(value);
}
- (void) trigger:(GamePadAction)event
{
    wrapper->mouse->trigger(event);
}

@end


//
// Joystick proxy
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
// Keyboard proxy
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
// DiskController proxy
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
- (void) inspect
{
    wrapper->controller->inspect();
}
- (DiskControllerConfig) getConfig
{
    return wrapper->controller->getConfig();
}
- (DiskControllerInfo) getInfo
{
    return wrapper->controller->getInfo();
}
- (void) dump
{
    wrapper->controller->dump();
}
- (BOOL) spinning
{
    return wrapper->controller->spinning();
}
- (void) setConnected:(NSInteger)nr value:(BOOL)value;
{
    wrapper->controller->setConnected(nr, value);
}
- (void) eject:(NSInteger)nr
{
    wrapper->controller->ejectDisk(nr);
}
- (void) insert:(NSInteger)nr adf:(ADFFileProxy *)fileProxy
{
    AmigaFileWrapper *fileWrapper = [fileProxy wrapper];
    wrapper->controller->insertDisk((ADFFile *)(fileWrapper->file), nr);
}
- (void) setWriteProtection:(NSInteger)nr value:(BOOL)value
{
    wrapper->controller->setWriteProtection(nr, value);
}

@end


//
// AmigaDrive proxy
//

@implementation DriveProxy

@synthesize wrapper;

- (instancetype) initWithDrive:(Drive *)drive
{
    if (self = [super init]) {
        wrapper = new AmigaDriveWrapper();
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
- (DriveType) type
{
    return wrapper->drive->getType();
}
- (void) inspect
{
    wrapper->drive->inspect();
}
- (void) dump
{
    wrapper->drive->dump();
}
- (BOOL) hasDisk
{
    return wrapper->drive->hasDisk();
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
- (BOOL) hasModifiedDisk
{
    return wrapper->drive->hasModifiedDisk();
}
- (void) setModifiedDisk:(BOOL)value
{
    wrapper->drive->setModifiedDisk(value);
}
- (u64) fnv
{
    return wrapper->drive->fnv();
}
- (ADFFileProxy *)convertDisk
{
    return NULL;
}


@end


//
// AmigaFile proxy
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
    file->setPath([path UTF8String]);
}
- (AmigaFileWrapper *)wrapper
{
    return wrapper;
}
- (AmigaFileType)type
{
    return wrapper->file->fileType();
}
- (NSInteger) sizeOnDisk
{
    return wrapper->file->sizeOnDisk();
}
- (void) seek:(NSInteger)offset
{
    wrapper->file->seek(offset);
}
- (NSInteger)read
{
    return wrapper->file->read();
}
- (void) readFromBuffer:(const void *)buffer length:(NSInteger)length
{
    wrapper->file->readFromBuffer((const u8 *)buffer, length);
}
- (NSInteger) writeToBuffer:(void *)buffer
{
    return wrapper->file->writeToBuffer((u8 *)buffer);
}

- (void) dealloc
{
    // NSLog(@"AmigaFileProxy::dealloc");
    
    if (wrapper) {
        if (wrapper->file) delete wrapper->file;
        delete wrapper;
    }
}

@end


//
// Snapshot proxy
//

@implementation SnapshotProxy

// @synthesize preview;

+ (BOOL) isSupportedSnapshot:(const void *)buffer length:(NSInteger)length
{
    return Snapshot::isSupportedSnapshot((u8 *)buffer, length);
}
+ (BOOL) isUnsupportedSnapshot:(const void *)buffer length:(NSInteger)length
{
    return Snapshot::isUnsupportedSnapshot((u8 *)buffer, length);
}
+ (BOOL) isSupportedSnapshotFile:(NSString *)path
{
    return Snapshot::isSupportedSnapshotFile([path UTF8String]);
}
+ (BOOL) isUnsupportedSnapshotFile:(NSString *)path
{
    return Snapshot::isUnsupportedSnapshotFile([path UTF8String]);
}
+ (instancetype) make:(Snapshot *)snapshot
{
    if (snapshot == NULL) { return nil; }
    
    SnapshotProxy *proxy = [[self alloc] initWithFile:snapshot];
    proxy->preview = NULL;
    
    return proxy;
}
+ (instancetype) makeWithBuffer:(const void *)buffer length:(NSInteger)length
{
    Snapshot *snapshot = Snapshot::makeWithBuffer((u8 *)buffer, length);
    return [self make:snapshot];
}
+ (instancetype) makeWithFile:(NSString *)path
{
    Snapshot *snapshot = Snapshot::makeWithFile([path UTF8String]);
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
    if (preview != NULL) { return preview; }
    
    // Create preview image
    
    // var bitmap = data
    Snapshot *snapshot = (Snapshot *)wrapper->file;
    
    NSInteger width = snapshot->getImageWidth();
    NSInteger height = snapshot->getImageHeight();
    unsigned char *data = snapshot->getImageData();
    
    
    NSBitmapImageRep *rep = [[NSBitmapImageRep alloc]
                             initWithBitmapDataPlanes:&data
                             pixelsWide:width
                             pixelsHigh:height
                             bitsPerSample:8
                             samplesPerPixel:4
                             hasAlpha:true
                             isPlanar:false
                             colorSpaceName:NSCalibratedRGBColorSpace
                             bytesPerRow:4*width
                             bitsPerPixel:32];
    
    preview = [[NSImage alloc] initWithSize:[rep size]];
    [preview addRepresentation:rep];
    
    // image.makeGlossy()

    return preview;
}
- (time_t)timeStamp
{
    return ((Snapshot *)wrapper->file)->getTimestamp();
}
- (NSData *)data
{
    Snapshot *snapshot = (Snapshot *)wrapper->file;
    return [NSData dataWithBytes: (void *)snapshot->getHeader()
                          length: snapshot->sizeOnDisk()];
}
    
@end


//
// ADFFile proxy
//

@implementation ADFFileProxy

+ (BOOL)isADFFile:(NSString *)path
{
    return ADFFile::isADFFile([path UTF8String]);
}
+ (instancetype) make:(ADFFile *)archive
{
    if (archive == NULL) return nil;
    return [[self alloc] initWithFile:archive];
}
+ (instancetype) makeWithBuffer:(const void *)buffer length:(NSInteger)length
{
    ADFFile *archive = ADFFile::makeWithBuffer((const u8 *)buffer, length);
    return [self make: archive];
}
+ (instancetype) makeWithFile:(NSString *)path
{
    ADFFile *archive = ADFFile::makeWithFile([path UTF8String]);
    return [self make: archive];
}
+ (instancetype) makeWithDiskType:(DiskType)type
{
    ADFFile *archive = ADFFile::makeWithDiskType(type);
    return [self make: archive];
}
+ (instancetype) makeWithDrive:(DriveProxy *)drive
{
    Drive *d = [drive wrapper]->drive;
    ADFFile *archive = ADFFile::makeWithDisk(d->disk);
    return archive ? [self make: archive] : nil;
}
- (DiskType)diskType
{
    return ((ADFFile *)wrapper->file)->getDiskType();
}
- (NSInteger)numCylinders
{
    return ((ADFFile *)wrapper->file)->numCyclinders();
}
- (NSInteger)numHeads
{
    return 2;
}
- (NSInteger)numTracks
{
    return ((ADFFile *)wrapper->file)->numTracks();
}
- (NSInteger)numSectors
{
    return ((ADFFile *)wrapper->file)->numSectorsTotal();
}
- (NSInteger)numSectorsPerTrack
{
    return ((ADFFile *)wrapper->file)->numSectorsPerTrack();
}
- (u64) fnv
{
    return ((ADFFile *)wrapper->file)->fnv();
}
- (void)formatDisk:(FileSystemType)fs
{
    ((ADFFile *)wrapper->file)->formatDisk(fs);
}
- (void)seekTrack:(NSInteger)nr
{
    ((ADFFile *)wrapper->file)->seekTrack(nr);
}
- (void)seekSector:(NSInteger)nr
{
    ((ADFFile *)wrapper->file)->seekSector(nr);
}

@end


//
// Amiga
//

@implementation AmigaProxy

@synthesize wrapper;
@synthesize cpu;
@synthesize ciaA;
@synthesize ciaB;
@synthesize mem;
@synthesize agnus;
@synthesize denise;
@synthesize paula;
@synthesize controlPort1;
@synthesize controlPort2;
@synthesize serialPort;
@synthesize mouse;
@synthesize joystick1;
@synthesize joystick2;
@synthesize keyboard;
@synthesize diskController;
@synthesize df0;
@synthesize df1;
@synthesize df2;
@synthesize df3;

- (instancetype) init
{
    NSLog(@"AmigaProxy::init");
    
    if (!(self = [super init]))
        return self;
    
    Amiga *amiga = new Amiga();
    wrapper = new AmigaWrapper();
    wrapper->amiga = amiga;
    
    // Create sub proxys
    cpu = [[CPUProxy alloc] initWithCPU:&amiga->cpu];
    ciaA = [[CIAProxy alloc] initWithCIA:&amiga->ciaA];
    ciaB = [[CIAProxy alloc] initWithCIA:&amiga->ciaB];
    mem = [[MemProxy alloc] initWithMemory:&amiga->mem];
    agnus = [[AgnusProxy alloc] initWithAgnus:&amiga->agnus];
    denise = [[DeniseProxy alloc] initWithDenise:&amiga->denise];
    paula = [[PaulaProxy alloc] initWithPaula:&amiga->paula];
    controlPort1 = [[ControlPortProxy alloc] initWithControlPort:&amiga->controlPort1];
    controlPort2 = [[ControlPortProxy alloc] initWithControlPort:&amiga->controlPort2];
    serialPort = [[SerialPortProxy alloc] initWithSerialPort:&amiga->serialPort];
    mouse = [[MouseProxy alloc] initWithMouse:&amiga->mouse];
    joystick1 = [[JoystickProxy alloc] initWithJoystick:&amiga->joystick1];
    joystick2 = [[JoystickProxy alloc] initWithJoystick:&amiga->joystick2];
    keyboard = [[KeyboardProxy alloc] initWithKeyboard:&amiga->keyboard];
    diskController = [[DiskControllerProxy alloc] initWithDiskController:&amiga->paula.diskController];
    df0 = [[DriveProxy alloc] initWithDrive:&amiga->df0];
    df1 = [[DriveProxy alloc] initWithDrive:&amiga->df1];
    df2 = [[DriveProxy alloc] initWithDrive:&amiga->df2];
    df3 = [[DriveProxy alloc] initWithDrive:&amiga->df3];

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
- (void) setInspectionTarget:(EventID)id
{
    wrapper->amiga->setInspectionTarget(id);
}
- (void) clearInspectionTarget
{
    wrapper->amiga->clearInspectionTarget();
}
- (BOOL) debugMode
{
    return wrapper->amiga->getDebugMode();
}
- (void) enableDebugging
{
    wrapper->amiga->enableDebugging();
}
- (void) disableDebugging
{
    wrapper->amiga->disableDebugging();
}
- (void) setDebugLevel:(NSInteger)value
{
    wrapper->amiga->debugLevel = value;
    wrapper->amiga->agnus.debugLevel = value;
    wrapper->amiga->denise.debugLevel = value;
}
- (ErrorCode) readyToPowerOn
{
    return wrapper->amiga->readyToPowerOn();
}
- (void) powerOn
{
    wrapper->amiga->powerOn();
}
- (void) powerOff
{
    wrapper->amiga->powerOff();
}
- (void) reset
{
    wrapper->amiga->reset();
}
- (void) ping
{
    wrapper->amiga->ping();
}
- (void) dump
{
    wrapper->amiga->dump();
}
- (AmigaInfo) getInfo
{
    return wrapper->amiga->getInfo();
}
- (AmigaStats) getStats
{
   return wrapper->amiga->getStats();
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
- (AmigaConfiguration) config
{
    return wrapper->amiga->getConfig();
}
- (BOOL) configure:(ConfigOption)option value:(NSInteger)value
{
    return wrapper->amiga->configure(option, value);
}
- (BOOL) configure:(ConfigOption)option enable:(BOOL)value
{
    return wrapper->amiga->configure(option, value ? 1 : 0);
}
- (BOOL) configureDrive:(NSInteger)nr connected:(BOOL)value
{
    return wrapper->amiga->configureDrive(nr, VA_DRIVE_CONNECT, value);
}
- (BOOL) configureDrive:(NSInteger)nr type:(NSInteger)type
{
    return wrapper->amiga->configureDrive(nr, VA_DRIVE_TYPE, type);
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
    return wrapper->amiga->getWarp();
}
- (void) warpOn
{
    wrapper->amiga->warpOn();
}
- (void) warpOff
{
    wrapper->amiga->warpOff();
}
- (void) loadFromSnapshot:(SnapshotProxy *)proxy
{
    Snapshot *snapshot = (Snapshot *)([proxy wrapper]->file);
    wrapper->amiga->loadFromSnapshotSafe(snapshot);
}

@end
