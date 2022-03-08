// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "HardDrive.h"
#include "Amiga.h"
#include "HdControllerTypes.h"
#include "IOUtils.h"
#include "Memory.h"
#include "MsgQueue.h"

HardDrive::HardDrive(Amiga& ref, isize nr) : Drive(ref, nr)
{
    string path;
    
    if (nr == 0) path = INITIAL_HD0;
    if (nr == 1) path = INITIAL_HD1;
    if (nr == 2) path = INITIAL_HD2;
    if (nr == 3) path = INITIAL_HD3;
    
    if (path != "") {
            
        try {
            
            auto hdf = HDFFile(path);
            init(hdf);
                        
        } catch (...) {
            
            warn("Cannot open HDF file %s\n", path.c_str());
        }
    }
}

void
HardDrive::alloc(isize size)
{
    dealloc();
    if (size) data = new u8[size];
}

void
HardDrive::dealloc()
{
    if (data) delete [] data;
    data = nullptr;
}

void
HardDrive::init()
{
    dealloc();

    diskVendor = "VAMIGA";
    diskProduct = "VDRIVE";
    diskRevision = "1.0";
    controllerVendor = amiga.hdcon[nr]->vendorName();
    controllerProduct = amiga.hdcon[nr]->productName();
    controllerRevision = amiga.hdcon[nr]->revisionName();
    geometry = GeometryDescriptor();
    ptable.clear();
    head = {};
    modified = false;
}

void
HardDrive::init(const GeometryDescriptor &geometry)
{
    // Throw an exception if the geometry is not supported
    geometry.checkCompatibility();
    
    // Wipe out the old drive
    init();
    
    // Create the drive description
    this->geometry = geometry;
    ptable.push_back(PartitionDescriptor(geometry));
        
    // Create the new drive
    data = new u8[geometry.numBytes()];
}

void
HardDrive::init(isize size)
{
    init(GeometryDescriptor(size));
}

void
HardDrive::init(const MutableFileSystem &fs)
{
    auto geometry = GeometryDescriptor(fs.numBytes());
    
    // Create the drive
    init(geometry);
    
    // Copy over all blocks
    fs.exportVolume(data, geometry.numBytes());
}

void
HardDrive::init(const HDFFile &hdf)
{
    auto geometry = hdf.getGeometry();
  
    // Create the drive
    init(geometry);

    // Copy the product description (if provided by the HDF)
    if (auto value = hdf.getDiskProduct(); value) diskProduct = *value;
    if (auto value = hdf.getDiskVendor(); value) diskVendor = *value;
    if (auto value = hdf.getDiskRevision(); value) diskRevision = *value;
    if (auto value = hdf.getControllerProduct(); value) controllerProduct = *value;
    if (auto value = hdf.getControllerVendor(); value) controllerVendor = *value;
    if (auto value = hdf.getControllerRevision(); value) controllerRevision = *value;
    
    // Copy geometry
    geometry = hdf.getGeometryDescriptor();
    
    // Copy the partition table
    ptable = hdf.getPartitionDescriptors();

    // Check the drive geometry against the file size
    auto numBytes = hdf.size;
    
    if (geometry.numBytes() < numBytes) {
        
        debug(XFILES, "HDF is too large. Ignoring excess bytes.\n");
        numBytes = geometry.numBytes();
    }
    if (geometry.numBytes() > hdf.size) {
        
        debug(XFILES, "HDF is too small. Padding with zeroes.");
        std::memset(data + hdf.size, 0, geometry.numBytes() - hdf.size);
    }
    
    // Copy over all blocks
    hdf.flash(data, 0, numBytes);
}

const char *
HardDrive::getDescription() const
{
    assert(usize(nr) < 4);
    return nr == 0 ? "Hd0" : nr == 1 ? "Hd1" : nr == 2 ? "Hd2" : "Hd3";
}

void
HardDrive::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    if constexpr (FORCE_HDR_MODIFIED) { modified = true; }
}

HardDriveConfig
HardDrive::getDefaultConfig(isize nr)
{
    HardDriveConfig defaults;
    
    defaults.type = HDR_GENERIC;
    defaults.connected = false;
    defaults.pan = IS_EVEN(nr) ? 100 : -100;
    defaults.stepVolume = 128;

    return defaults;
}

void
HardDrive::resetConfig()
{
    auto defaults = getDefaultConfig(nr);
    
    setConfigItem(OPT_HDR_TYPE, defaults.type);
    setConfigItem(OPT_HDR_CONNECT, defaults.connected);
    setConfigItem(OPT_HDR_PAN, defaults.pan);
    setConfigItem(OPT_HDR_STEP_VOLUME, defaults.stepVolume);
}

i64
HardDrive::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_HDR_TYPE:          return (long)config.type;
        case OPT_HDR_CONNECT:       return (long)config.connected;
        case OPT_HDR_PAN:           return (long)config.pan;
        case OPT_HDR_STEP_VOLUME:   return (long)config.stepVolume;

        default:
            fatalError;
    }
}

void
HardDrive::setConfigItem(Option option, i64 value)
{
    switch (option) {
         
        case OPT_HDR_TYPE:
            
            if (!HardDriveTypeEnum::isValid(value)) {
                throw VAError(ERROR_OPT_INVARG, HardDriveTypeEnum::keyList());
            }
            
            config.type = (HardDriveType)value;
            return;

        case OPT_HDR_CONNECT:
            
            if (!isPoweredOff()) {
                throw VAError(ERROR_OPT_LOCKED);
            }
            
            if (bool(value) != config.connected) {
                
                config.connected = bool(value);
                
                if (value) {

                    // Attach a default disk when the drive gets connected
                    init(MB(10));
                    format(FS_OFS, defaultName());
                    
                } else {
                    
                    init();
                }
                
                msgQueue.put(value ? MSG_HDR_CONNECT : MSG_HDR_DISCONNECT, nr);
            }
            return;

        case OPT_HDR_PAN:

            config.pan = (i16)value;
            return;

        case OPT_HDR_STEP_VOLUME:

            config.stepVolume = (u8)value;
            return;

        default:
            fatalError;
    }
}

const PartitionDescriptor &
HardDrive::getPartitionInfo(isize nr)
{
    assert(nr >= 0 && nr < numPartitions());
    return ptable[nr];
}

void
HardDrive::_inspect() const
{
    {   SYNCHRONIZED
        
        info.modified = isModified();
        info.head = head;
    }
}

void
HardDrive::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;

    if (category & dump::Config) {
        
        os << tab("Nr");
        os << dec(nr) << std::endl;
        os << tab("Type");
        os << HardDriveTypeEnum::key(config.type) << std::endl;
        os << tab("Connected");
        os << bol(config.connected) << std::endl;
        os << tab("Step volume");
        os << dec(config.stepVolume) << std::endl;
        os << tab("Pan");
        os << dec(config.pan) << std::endl;
    }
    
    if (category & dump::Drive) {
        
        auto cap1 = geometry.numBytes() / MB(1);
        auto cap2 = ((100 * geometry.numBytes()) / MB(1)) % 100;
        
        os << tab("Hard drive");
        os << dec(nr) << std::endl;
        os << tab("Capacity");
        os << dec(cap1) << "." << dec(cap2) << " MB" << std::endl;
        geometry.dump(os);
        os << tab("Disk vendor");
        os << diskVendor << std::endl;
        os << tab("Disk Product");
        os << diskProduct << std::endl;
        os << tab("Disk Revision");
        os << diskRevision << std::endl;
        os << tab("Controller vendor");
        os << controllerVendor << std::endl;
        os << tab("Controller Product");
        os << controllerProduct << std::endl;
        os << tab("Controller Revision");
        os << controllerRevision << std::endl;
    }

    if (category & dump::Volumes) {

        os << "Type   Size            Used    Free    Full  Name" << std::endl;

        for (isize i = 0; i < isize(ptable.size()); i++) {
            
            auto fs = MutableFileSystem(*this, i);
            fs.dump(dump::Summary, os);
        }
        
        for (isize i = 0; i < isize(ptable.size()); i++) {

            os << std::endl;
            os << tab("Partition");
            os << dec(i) << std::endl;
            auto fs = MutableFileSystem(*this, i);
            fs.dump(dump::Properties, os);
        }
    }
    
    if (category & dump::Partitions) {
        
        for (usize i = 0; i < ptable.size(); i++) {
            
            auto &part = ptable[i];
            
            if (i != 0) os << std::endl;
            os << tab("Partition");
            os << dec(i) << std::endl;
            part.dump(os);
        }
    }

    if (category & dump::State) {
        
        os << tab("Nr");
        os << dec(nr) << std::endl;
        os << tab("Drive head");
        os << dec(head.cylinder) << ":" << dec(head.head) << ":" << dec(head.offset);
        os << std::endl;
        os << tab("State");
        os << HardDriveStateEnum::key(state) << std::endl;
        os << tab("Modified");
        os << bol(modified) << std::endl;
        os << tab("Write protected");
        os << bol(writeProtected) << std::endl;
    }
}

isize
HardDrive::_size()
{
    util::SerCounter counter;

    // Determine size information
    auto dataSize = geometry.numBytes() + 8;

    applyToPersistentItems(counter);
    applyToResetItems(counter);
    
    counter.count += dataSize;
    return counter.count;
}

isize
HardDrive::didLoadFromBuffer(const u8 *buffer)
{
    util::SerReader reader(buffer);
    isize dataSize;

    // Load size information
    reader << dataSize;

    // Allocate memory
    if (dataSize > MB(504)) throw VAError(ERROR_SNAP_CORRUPTED);
    alloc(isize(dataSize));

    // Load data
    // debug(true, "data = %p size = %ld\n", (void *)data, dataSize);
    // debug(true, "numBytes = %ld\n", desc.geometry.numBytes());
    assert(dataSize == geometry.numBytes());
    reader.copy(data, dataSize);

    return (isize)(reader.ptr - buffer);
}

isize
HardDrive::didSaveToBuffer(u8 *buffer)
{
    util::SerWriter writer(buffer);

    // Determine size information
    isize dataSize = geometry.numBytes();

    // Save size information
    writer << dataSize;
 
    // Write data
    writer.copy(data, dataSize);

    return (isize)(writer.ptr - buffer);
}

bool
HardDrive::isConnected() const
{
    return config.connected;
}

u64
HardDrive::fnv() const
{
    return hasDisk() ? util::fnv_1a_64(data, geometry.numBytes()) : 0;
}

bool
HardDrive::hasDisk() const
{
    return data != nullptr;
}

bool
HardDrive::hasModifiedDisk() const
{
    return hasDisk() && modified;
}

bool
HardDrive::hasProtectedDisk() const
{
    return hasDisk() && writeProtected;
}

void
HardDrive::setModificationFlag(bool value)
{
    if (hasDisk()) modified = value;
}
void
HardDrive::setProtectionFlag(bool value)
{
    if (hasDisk()) writeProtected = value;
}

string
HardDrive::defaultName(isize partition)
{
    if (nr >= 1) partition += amiga.hd0.numPartitions();
    if (nr >= 2) partition += amiga.hd1.numPartitions();
    if (nr >= 3) partition += amiga.hd2.numPartitions();

    return "DH" + std::to_string(partition);
}

void
HardDrive::format(FSVolumeType fsType, string name)
{
    if constexpr (HDR_DEBUG) {

        msg("Formatting hard drive\n");
        msg("    File system : %s\n", FSVolumeTypeEnum::key(fsType));
        msg("           Name : %s\n", name.c_str());
    }
    
    // Only proceed if a disk is present
    if (!data) return;

    if (fsType != FS_NODOS) {
        
        // Create a device descriptor matching this drive
        auto layout = FileSystemDescriptor(geometry, fsType);

        // Create a file system
        auto fs = MutableFileSystem(layout);

        // Add name and bootblock
        fs.setName(name);
                
        // Copy all blocks over
        fs.exportVolume(data, geometry.numBytes());
    }
}

void
HardDrive::changeGeometry(isize c, isize h, isize s, isize b)
{
    auto geometry = GeometryDescriptor(c, h, s, b);
    changeGeometry(geometry);
}

void
HardDrive::changeGeometry(const GeometryDescriptor &geometry)
{
    geometry.checkCompatibility();
        
    if (this->geometry.numBytes() == geometry.numBytes()) {
        
        this->geometry = geometry;
    
    } else {
        
        throw VAError(ERROR_HDR_UNMATCHED_GEOMETRY);
    }
}

i8
HardDrive::read(isize offset, isize length, u32 addr)
{
    debug(HDR_DEBUG, "read(%ld, %ld, %u)\n", offset, length, addr);

    // Check arguments
    auto error = verify(offset, length, addr);
    
    if (!error) {

        state = HDR_STATE_READING;

        // Move the drive head to the specified location
        moveHead(offset / geometry.bsize);

        // Perform the read operation
        mem.patch(addr, data + offset, length);
                
        // Inform the GUI
        msgQueue.put(MSG_HDR_READ);
        
        // Go back to IDLE state after some time
        scheduleIdleEvent();
    }
    
    return error;
}

i8
HardDrive::write(isize offset, isize length, u32 addr)
{
    debug(HDR_DEBUG, "write(%ld, %ld, %u)\n", offset, length, addr);

    // Check arguments
    auto error = verify(offset, length, addr);
    
    if (!error) {
    
        state = HDR_STATE_WRITING;

        // Move the drive head to the specified location
        moveHead(offset / geometry.bsize);

        // Perform the write operation
        if (!writeProtected) {
            mem.spypeek <ACCESSOR_CPU> (addr, length, data + offset);
        }
        
        // Inform the GUI
        msgQueue.put(MSG_HDR_WRITE);
        
        // Go back to IDLE state after some time
        scheduleIdleEvent();
    }
    
    return error;
}

i8
HardDrive::verify(isize offset, isize length, u32 addr)
{
    assert(data);

    if (length % 512) {
        
        debug(HDR_DEBUG, "Length must be a multiple of 512 bytes");
        return IOERR_BADLENGTH;
    }

    if (offset % 512) {
        
        debug(HDR_DEBUG, "Offset is not aligned");
        return IOERR_BADADDRESS;
    }

    if (offset + length > geometry.numBytes()) {
        
        debug(HDR_DEBUG, "Invalid block location");
        return IOERR_BADADDRESS;
    }

    if (!mem.inRam(addr) || !mem.inRam(u32(addr + length))) {
        
        debug(HDR_DEBUG, "Invalid RAM location");
        return IOERR_BADADDRESS;
    }

    return 0;
}
 
void
HardDrive::moveHead(isize lba)
{
    isize c = lba / (geometry.heads * geometry.sectors);
    isize h = (lba / geometry.sectors) % geometry.heads;
    isize s = lba % geometry.sectors;

    moveHead(c, h, s);
}

void
HardDrive::moveHead(isize c, isize h, isize s)
{
    bool step = head.cylinder != c;
    
    head.cylinder = c;
    head.head = h;
    head.offset = geometry.bsize * s;
    
    if (step) {
        msgQueue.put(MSG_HDR_STEP, i16(nr), i16(c), config.stepVolume, config.pan);
    }
}

void
HardDrive::scheduleIdleEvent()
{
    auto delay = MSEC(100);
    
    switch (nr) {
            
        case 0: agnus.scheduleRel <SLOT_HD0> (delay, HDR_IDLE); break;
        case 1: agnus.scheduleRel <SLOT_HD1> (delay, HDR_IDLE); break;
        case 2: agnus.scheduleRel <SLOT_HD2> (delay, HDR_IDLE); break;
        case 3: agnus.scheduleRel <SLOT_HD3> (delay, HDR_IDLE); break;

        default: fatalError;
    }
}

template <EventSlot s> void
HardDrive::serviceHdrEvent()
{
    scheduler.cancel <s> ();
    state = HDR_STATE_IDLE;
    msgQueue.put(MSG_HDR_IDLE, nr);
}

template void HardDrive::serviceHdrEvent <SLOT_HD0> ();
template void HardDrive::serviceHdrEvent <SLOT_HD1> ();
template void HardDrive::serviceHdrEvent <SLOT_HD2> ();
template void HardDrive::serviceHdrEvent <SLOT_HD3> ();
