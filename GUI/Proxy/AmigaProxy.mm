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
- (DisassembledInstruction) getInstrInfo:(NSInteger)index
{
    return wrapper->cpu->getInstrInfo(index);
}
- (DisassembledInstruction) getTracedInstrInfo:(NSInteger)index
{
    return wrapper->cpu->getTracedInstrInfo(index);
}
- (int64_t) clock
{
    return wrapper->cpu->clock; 
}
- (int64_t) cycles
{
    return wrapper->cpu->cycles();
}
- (BOOL) hasBreakpointAt:(uint32_t)addr
{
    return wrapper->cpu->bpManager.hasBreakpointAt(addr);
}
- (BOOL) hasDisabledBreakpointAt:(uint32_t)addr
{
    return wrapper->cpu->bpManager.hasDisabledBreakpointAt(addr);
}
- (BOOL) hasConditionalBreakpointAt:(uint32_t)addr
{
    return wrapper->cpu->bpManager.hasConditionalBreakpointAt(addr);
}
- (void) setBreakpointAt:(uint32_t)addr
{
    return wrapper->cpu->bpManager.setBreakpointAt(addr);
}
- (void) deleteBreakpointAt:(uint32_t)addr
{
    return wrapper->cpu->bpManager.deleteBreakpointAt(addr);
}
- (void) enableBreakpointAt:(uint32_t)addr
{
    return wrapper->cpu->bpManager.enableBreakpointAt(addr);
}
- (void) disableBreakpointAt:(uint32_t)addr
{
    return wrapper->cpu->bpManager.disableBreakpointAt(addr);
}
- (NSInteger) numberOfBreakpoints
{
    return wrapper->cpu->bpManager.numberOfBreakpoints();
}
- (void) deleteBreakpoint:(NSInteger)nr
{
    return wrapper->cpu->bpManager.deleteBreakpoint(nr);
}
- (BOOL) isDisabled:(NSInteger)nr
{
    return wrapper->cpu->bpManager.isDisabled(nr);
}
- (BOOL) hasCondition:(NSInteger)nr
{
    return wrapper->cpu->bpManager.hasCondition(nr);
}
- (BOOL) hasSyntaxError:(NSInteger)nr
{
    return wrapper->cpu->bpManager.hasSyntaxError(nr);
}
- (uint32_t) breakpointAddr:(NSInteger)nr
{
    return wrapper->cpu->bpManager.getAddr(nr);
}
- (BOOL) setBreakpointAddr:(NSInteger)nr addr:(uint32_t)addr
{
    return wrapper->cpu->bpManager.setAddr(nr, addr);
}
- (NSString *) breakpointCondition:(NSInteger)nr
{
    const char *str = wrapper->cpu->bpManager.getCondition(nr);
    return str ? [NSString stringWithUTF8String:str] : NULL;
}
- (BOOL) setBreakpointCondition:(NSInteger)nr cond:(NSString *)cond
{
    return wrapper->cpu->bpManager.setCondition(nr, [cond UTF8String]);
}
- (NSInteger) traceBufferCapacity
{
    return wrapper->cpu->traceBufferCapacity; 
}
/*
- (NSInteger) recordedInstructions
{
    return wrapper->cpu->recordedInstructions();
}
*/
/*
- (RecordedInstruction) readRecordedInstruction:(NSInteger)nr
{
    return wrapper->cpu->readRecordedInstruction(nr);
}
*/
/*
- (void) clearTraceBuffer
{
    wrapper->cpu->clearTraceBuffer();
}
*/
- (void) truncateTraceBuffer:(NSInteger)count
{
    wrapper->cpu->truncateTraceBuffer((unsigned)count);
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
- (void) dump
{
    wrapper->cia->dump();
}
- (void) poke:(uint16_t)addr value:(uint8_t)value {
    wrapper->cia->amiga->suspend();
    wrapper->cia->poke(addr, value);
    wrapper->cia->amiga->resume();
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
- (void) dump
{
    wrapper->mem->dump();
}
- (BOOL) hasBootRom
{
    return wrapper->mem->hasBootRom();
}
- (void) deleteBootRom
{
    wrapper->mem->deleteBootRom();
}
- (BOOL) isBootRom:(NSURL *)url
{
    return BootRom::isBootRomFile([[url path] UTF8String]);
}
- (BOOL) loadBootRomFromBuffer:(NSData *)data
{
    if (data == NULL) return NO;
    const uint8_t *bytes = (const uint8_t *)[data bytes];
    return wrapper->mem->loadBootRomFromBuffer(bytes, [data length]);
}
- (BOOL) loadBootRomFromFile:(NSURL *)url
{
    return wrapper->mem->loadBootRomFromFile([[url path] UTF8String]);
}
- (uint64_t) bootRomFingerprint
{
    return wrapper->mem->bootRomFingerprint();
}
- (BOOL) hasKickRom
{
    return wrapper->mem->hasKickRom();
}
- (void) deleteKickRom
{
    wrapper->mem->deleteKickRom();
}
- (BOOL) isKickRom:(NSURL *)url
{
    return KickRom::isKickRomFile([[url path] UTF8String]);
}
- (BOOL) loadKickRomFromBuffer:(NSData *)data
{
    if (data == NULL) return NO;
    const uint8_t *bytes = (const uint8_t *)[data bytes];
    return wrapper->mem->loadKickRomFromBuffer(bytes, [data length]);
}
- (BOOL) loadKickRomFromFile:(NSURL *)url
{
    return wrapper->mem->loadKickRomFromFile([[url path] UTF8String]);
}
- (uint64_t) kickRomFingerprint
{
    return wrapper->mem->kickRomFingerprint();
}
- (BOOL) hasExtRom
{
    return wrapper->mem->hasExtRom();
}
- (void) deleteExtRom
{
    wrapper->mem->deleteExtRom();
}
- (BOOL) isExtRom:(NSURL *)url
{
    return ExtRom::isExtRomFile([[url path] UTF8String]);
}
- (BOOL) loadExtRomFromBuffer:(NSData *)data
{
    if (data == NULL) return NO;
    const uint8_t *bytes = (const uint8_t *)[data bytes];
    return wrapper->mem->loadExtRomFromBuffer(bytes, [data length]);
}
- (BOOL) loadExtRomFromFile:(NSURL *)url
{
    return wrapper->mem->loadExtRomFromFile([[url path] UTF8String]);
}
- (uint64_t) extRomFingerprint
{
    return wrapper->mem->extRomFingerprint();
}
- (MemorySource *) getMemSrcTable
{
    return wrapper->mem->getMemSrcTable();
}
- (MemorySource) memSrc:(NSInteger)addr
{
    return wrapper->mem->getMemSrc((uint32_t)addr);
}
- (NSInteger) spypeek8:(NSInteger)addr
{
    return wrapper->mem->spypeek8((uint32_t)addr);
}
- (NSInteger) spypeek16:(NSInteger)addr
{
    return wrapper->mem->spypeek16((uint32_t)addr);
}
/*
- (void) poke8:(NSInteger)addr value:(NSInteger)value
{
    wrapper->mem->poke8((uint32_t)addr, value);
}
- (void) poke16:(NSInteger)addr value:(NSInteger)value
{
    wrapper->mem->poke16((uint32_t)addr, value);
}
- (void) pokeCustom16:(NSInteger)addr value:(NSInteger)value
{
    wrapper->mem->pokeCustom16<POKE_CPU>((uint32_t)addr, value);
}
*/
- (NSString *) ascii:(NSInteger)addr
{
    const char *str = wrapper->mem->ascii((uint32_t)addr);
    return str ? [NSString stringWithUTF8String:str] : NULL;
}
- (NSString *) hex:(NSInteger)addr bytes:(NSInteger)bytes
{
    const char *str = wrapper->mem->hex((uint32_t)addr, bytes);
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
- (NSInteger) instrCount:(NSInteger)list
{
    return wrapper->agnus->copper.instrCount(list);
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
    return wrapper->denise->getSprInfo(nr);
}
- (void) pokeColorReg:(NSInteger)reg value:(UInt16)value
{
    wrapper->denise->pokeColorReg(0x180 + 2*reg, value);
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
- (void) setBPU:(NSInteger)count
{
    wrapper->denise->debugSetBPU(count);
}
- (void) setBPLCONx:(NSInteger)x value:(NSInteger)value
{
    wrapper->denise->debugSetBPLCONx(x, (uint16_t)value);
}
- (void) setBPLCONx:(NSInteger)x bit:(NSInteger)bit value:(BOOL)value
{
    wrapper->denise->debugSetBPLCONxBit(x, bit, value);
}
- (void) setBPLCONx:(NSInteger)x nibble:(NSInteger)nibble value:(NSInteger)value
{
    wrapper->denise->debugSetBPLCONxNibble(x, nibble, (uint8_t)value);
}
- (ScreenBuffer) stableLongFrame
{
    return wrapper->denise->pixelEngine.getStableLongFrame();
}
- (ScreenBuffer) stableShortFrame
{
    return wrapper->denise->pixelEngine.getStableShortFrame();
}
- (BOOL) interlaceMode
{
    return wrapper->denise->agnus->frameInfo.interlaced;
}
- (BOOL) isLongFrame
{
    return wrapper->denise->agnus->isLongFrame();
}
- (BOOL) isShortFrame
{
    return wrapper->denise->agnus->isShortFrame();
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
- (DiskControllerConfig) getDiskControllerConfig
{
    return wrapper->paula->diskController.getConfig();
}
- (DiskControllerInfo) getDiskControllerInfo
{
    return wrapper->paula->diskController.getInfo();
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
- (uint32_t) sampleRate
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
- (float) ringbufferDataL:(NSInteger)offset
{
    return wrapper->paula->audioUnit.ringbufferDataL(offset);
}
- (float) ringbufferDataR:(NSInteger)offset
{
    return wrapper->paula->audioUnit.ringbufferDataR(offset);
}
- (float) ringbufferData:(NSInteger)offset
{
    return wrapper->paula->audioUnit.ringbufferData(offset);
}
- (double) fillLevel
{
    return wrapper->paula->audioUnit.fillLevel();
}
- (NSInteger) bufferUnderflows
{
    return wrapper->paula->audioUnit.bufferUnderflows;
}
- (NSInteger) bufferOverflows
{
    return wrapper->paula->audioUnit.bufferOverflows;
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
- (void) connectDevice:(SerialPortDevice)value
{
    wrapper->port->connectDevice(value);
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
    wrapper->mouse->setXY((int64_t)pos.x, (int64_t)pos.y);
}
- (void) setLeftButton:(BOOL)value
{
    wrapper->mouse->setLeftButton(value);
}
- (void) setRightButton:(BOOL)value
{
    wrapper->mouse->setRightButton(value);
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
- (void) trigger:(JoystickEvent)event
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
- (void) dump
{
    wrapper->controller->dump();
}
- (BOOL) spinning:(NSInteger)nr
{
    return wrapper->controller->spinning(nr);
}
- (BOOL) spinning
{
    return wrapper->controller->spinning();
}
/*
- (BOOL) isConnected:(NSInteger)nr
{
    return wrapper->controller->isConnected(nr);
}
*/
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
- (NSInteger) nr
{
    return wrapper->drive->getNr();
}
- (DriveType) type
{
    return wrapper->drive->getType();
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
/*
- (void) ejectDisk
{
    wrapper->drive->ejectDisk();
}
- (void) insertDisk:(ADFFileProxy *)fileProxy
{
    AmigaFileWrapper *fileWrapper = [fileProxy wrapper];
    wrapper->drive->insertDisk((ADFFile *)(fileWrapper->file));
}
*/
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
    wrapper->file->readFromBuffer((const uint8_t *)buffer, length);
}
- (NSInteger) writeToBuffer:(void *)buffer
{
    return wrapper->file->writeToBuffer((uint8_t *)buffer);
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

+ (BOOL) isSupportedSnapshot:(const void *)buffer length:(NSInteger)length
{
    return Snapshot::isSupportedSnapshot((uint8_t *)buffer, length);
}
+ (BOOL) isUnsupportedSnapshot:(const void *)buffer length:(NSInteger)length
{
    return Snapshot::isUnsupportedSnapshot((uint8_t *)buffer, length);
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
    if (snapshot == NULL) {
        return nil;
    }
    return [[self alloc] initWithFile:snapshot];
}
+ (instancetype) makeWithBuffer:(const void *)buffer length:(NSInteger)length
{
    Snapshot *snapshot = Snapshot::makeWithBuffer((uint8_t *)buffer, length);
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
    ADFFile *archive = ADFFile::makeWithBuffer((const uint8_t *)buffer, length);
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
    return ((ADFFile *)wrapper->file)->getNumCyclinders();
}
- (NSInteger)numHeads
{
    return 2;
}
- (NSInteger)numTracks
{
    return ((ADFFile *)wrapper->file)->getNumTracks();
}
- (NSInteger)numSectors
{
    return ((ADFFile *)wrapper->file)->getNumSectors();
}
- (NSInteger)numSectorsPerTrack
{
    return ((ADFFile *)wrapper->file)->getNumSectorsPerTrack();
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
- (void) powerOn
{
    return wrapper->amiga->powerOn();
}
- (void) powerOff
{
    return wrapper->amiga->powerOff();
}
- (void) reset
{
    return wrapper->amiga->reset();
}
- (void) ping
{
    return wrapper->amiga->ping();
}
- (void) dump
{
    return wrapper->amiga->dump();
}
- (AmigaInfo) getInfo
{
    return wrapper->amiga->getInfo();
}
- (BOOL) readyToPowerUp
{
    return wrapper->amiga->readyToPowerUp();
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

- (AmigaConfiguration) config
{
    return wrapper->amiga->getConfig();
}
- (AmigaMemConfiguration) memConfig
{
    return wrapper->amiga->getMemConfig();
}
- (BOOL) configure:(ConfigOption)option value:(NSInteger)value
{
    return wrapper->amiga->configure(option, value);
}
- (BOOL) configure:(ConfigOption)option enable:(BOOL)value
{
    return wrapper->amiga->configure(option, value ? 1 : 0);
}
- (BOOL) configureModel:(NSInteger)model
{
    return wrapper->amiga->configureModel((AmigaModel)model);
}
- (BOOL) configureLayout:(NSInteger)value
{
    return wrapper->amiga->configureLayout(value);
}
- (BOOL) configureChipMemory:(NSInteger)size
{
    return wrapper->amiga->configureChipMemory((unsigned)size);
}
- (BOOL) configureSlowMemory:(NSInteger)size
{
    return wrapper->amiga->configureSlowMemory((unsigned)size);
}
- (BOOL) configureFastMemory:(NSInteger)size
{
    return wrapper->amiga->configureFastMemory((unsigned)size);
}
- (void) configureRealTimeClock:(BOOL)value
{
    wrapper->amiga->configureRealTimeClock(value);
}
- (BOOL) configureDrive:(NSInteger)nr connected:(BOOL)value
{
    return wrapper->amiga->configureDrive(nr, VA_DRIVE_CONNECT, value);
}
- (BOOL) configureDrive:(NSInteger)nr type:(NSInteger)type
{
    return wrapper->amiga->configureDrive(nr, VA_DRIVE_TYPE, type);
}
- (BOOL) configureDrive:(NSInteger)nr speed:(NSInteger)value
{
    return wrapper->amiga->configureDrive(nr, VA_DRIVE_SPEED, value);
}
/*
- (void) configureFifoBuffering:(BOOL)value
{
    wrapper->amiga->configureFifoBuffering(value);
}
*/
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
- (BOOL) takeAutoSnapshots
{
    return wrapper->amiga->getTakeAutoSnapshots();
}
- (void) setTakeAutoSnapshots:(BOOL)b
{
    wrapper->amiga->setTakeAutoSnapshots(b);
}
- (void) suspendAutoSnapshots
{
    wrapper->amiga->suspendAutoSnapshots();
}
- (void) resumeAutoSnapshots
{
    wrapper->amiga->resumeAutoSnapshots();
}
- (NSInteger) snapshotInterval
{
    return wrapper->amiga->getSnapshotInterval();
}
- (void) loadFromSnapshot:(SnapshotProxy *)proxy
{
    Snapshot *snapshot = (Snapshot *)([proxy wrapper]->file);
    wrapper->amiga->loadFromSnapshotSafe(snapshot);
}
- (void) setSnapshotInterval:(NSInteger)value
{
    wrapper->amiga->setSnapshotInterval(value);
}
- (BOOL)restoreAutoSnapshot:(NSInteger)nr
{
    return wrapper->amiga->restoreAutoSnapshot((unsigned)nr);
}
- (BOOL)restoreUserSnapshot:(NSInteger)nr
{
    return wrapper->amiga->restoreUserSnapshot((unsigned)nr);
}
- (BOOL)restoreLatestUserSnapshot
{
    return wrapper->amiga->restoreLatestUserSnapshot();
}
- (BOOL)restoreLatestAutoSnapshot
{
    return wrapper->amiga->restoreLatestAutoSnapshot();
}
- (NSInteger) numAutoSnapshots
{
    return wrapper->amiga->numAutoSnapshots();
}
- (NSInteger) numUserSnapshots
{
    return wrapper->amiga->numUserSnapshots();
}
- (NSData *)autoSnapshotData:(NSInteger)nr {
    Snapshot *snapshot = wrapper->amiga->autoSnapshot((unsigned)nr);
    return [NSData dataWithBytes: (void *)snapshot->getHeader()
                          length: snapshot->sizeOnDisk()];
}
- (NSData *)userSnapshotData:(NSInteger)nr {
    Snapshot *snapshot = wrapper->amiga->userSnapshot((unsigned)nr);
    return [NSData dataWithBytes: (void *)snapshot->getHeader()
                          length: snapshot->sizeOnDisk()];
}
- (unsigned char *)autoSnapshotImageData:(NSInteger)nr
{
    Snapshot *s = wrapper->amiga->autoSnapshot((int)nr);
    return s ? s->getImageData() : NULL;
}
- (unsigned char *)userSnapshotImageData:(NSInteger)nr
{
    Snapshot *s = wrapper->amiga->userSnapshot((int)nr);
    return s ? s->getImageData() : NULL;
}
- (NSSize) autoSnapshotImageSize:(NSInteger)nr {
    Snapshot *s = wrapper->amiga->autoSnapshot((int)nr);
    return s ? NSMakeSize(s->getImageWidth(), s->getImageHeight()) : NSMakeSize(0,0);
}
- (NSSize) userSnapshotImageSize:(NSInteger)nr {
    Snapshot *s = wrapper->amiga->userSnapshot((int)nr);
    return s ? NSMakeSize(s->getImageWidth(), s->getImageHeight()) : NSMakeSize(0,0);
}
- (time_t)autoSnapshotTimestamp:(NSInteger)nr {
    Snapshot *s = wrapper->amiga->autoSnapshot((int)nr);
    return s ? s->getTimestamp() : 0;
}
- (time_t)userSnapshotTimestamp:(NSInteger)nr {
    Snapshot *s = wrapper->amiga->userSnapshot((int)nr);
    return s ? s->getTimestamp() : 0;
}
- (void)takeUserSnapshot
{
    wrapper->amiga->takeUserSnapshotSafe();
}
- (void)deleteAutoSnapshot:(NSInteger)nr
{
    wrapper->amiga->deleteAutoSnapshot((unsigned)nr);
}
- (void)deleteUserSnapshot:(NSInteger)nr
{
    wrapper->amiga->deleteUserSnapshot((unsigned)nr);
}

@end

