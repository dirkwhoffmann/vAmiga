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

    desc = HdrvDescriptor();
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
    desc = HdrvDescriptor(geometry);
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

    // Copy the drive properties
    desc = hdf.getHdrvDescriptor();
    
    // Copy the partition table
    ptable = hdf.getPartitionDescriptors();

    // Copy over all blocks
    hdf.flash(data);
    
    // REMOVE ASAP
    dump(dump::Drive);
    dump(dump::Partitions);
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
    defaults.connected = nr == 0;
    
    return defaults;
}

void
HardDrive::resetConfig()
{
    auto defaults = getDefaultConfig(nr);
    
    setConfigItem(OPT_HDR_TYPE, defaults.type);
    setConfigItem(OPT_HDR_CONNECT, defaults.connected);
}

i64
HardDrive::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_HDR_TYPE:      return (long)config.type;
        case OPT_HDR_CONNECT:   return (long)config.connected;

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

            config.connected = bool(value);
            
            // Attach a default disk when the drive gets connected
            if (value) {
                
                init(MB(10));
                format(FS_OFS, defaultName());
                
            } else {
                
                init();
            }
            
            msgQueue.put(value ? MSG_HDR_CONNECT : MSG_HDR_DISCONNECT, nr);
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
        
        // info.attached = isAttached();
        info.modified = isModified();
        info.head.c = head.c;
        info.head.h = head.h;
        info.head.s = head.s;
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
    }
    
    if (category & dump::Drive) {

        auto &geometry = desc.geometry;
        
        auto cap1 = geometry.numBytes() / MB(1);
        auto cap2 = ((100 * geometry.numBytes()) / MB(1)) % 100;
        
        os << tab("Hard drive");
        os << dec(nr) << std::endl;
        os << tab("Capacity");
        os << dec(cap1) << "." << dec(cap2) << " MB" << std::endl;
        desc.dump(os);
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
        os << tab("Head position");
        os << dec(head.c) << ":" << dec(head.h) << ":" << dec(head.s);
        os << std::endl;
        os << tab("Modified");
        os << bol(modified) << std::endl;
    }
}

isize
HardDrive::_size()
{
    util::SerCounter counter;

    // Determine size information
    auto dataSize = i64(desc.geometry.numBytes()) + 8;

    applyToPersistentItems(counter);
    applyToResetItems(counter);
    
    counter.count += dataSize;
    return counter.count;
}

isize
HardDrive::didLoadFromBuffer(const u8 *buffer)
{
    util::SerReader reader(buffer);
    i64 dataSize;

    // Load size information
    reader << dataSize;

    // Allocate memory
    if (dataSize > MB(504)) throw VAError(ERROR_SNAP_CORRUPTED);
    alloc(dataSize);

    // Load data
    debug(true, "data = %p size = %lld\n", (void *)data, dataSize);
    debug(true, "numBytes = %ld\n", desc.geometry.numBytes());
    assert(dataSize == desc.geometry.numBytes());
    reader.copy(data, dataSize);

    return (isize)(reader.ptr - buffer);
}

isize
HardDrive::didSaveToBuffer(u8 *buffer)
{
    util::SerWriter writer(buffer);

    // Determine size information
    i64 dataSize = desc.geometry.numBytes();

    // Save size information
    printf("Saving dataSize = %lld\n", dataSize);
    writer << dataSize;
 
    // Write data
    writer.copy(data, dataSize);

    return (isize)(writer.ptr - buffer);
}

u64
HardDrive::fnv() const
{
    return hasDisk() ? util::fnv_1a_64(data, desc.geometry.numBytes()) : 0;
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
        auto layout = FileSystemDescriptor(desc.geometry, fsType);

        // Create a file system
        auto fs = MutableFileSystem(layout);

        // Add name and bootblock
        fs.setName(name);
        
        // REMOVE ASAP
        msg("File system:\n");
        fs.dump();
        
        // Copy all blocks over
        fs.exportVolume(data, desc.geometry.numBytes());
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
        
    if (this->desc.geometry.numBytes() == desc.geometry.numBytes()) {
        
        this->desc.geometry = geometry;
    
    } else {
        
        throw VAError(ERROR_HDR_UNMATCHED_GEOMETRY);
    }
}

i8
HardDrive::read(isize partition, isize block, isize length, u32 addr)
{
    debug(HDR_DEBUG, "read(%ld, %ld, %ld, %u)\n", partition, block, length, addr);

    return read(offset(partition, block), length, addr);
}

i8
HardDrive::read(isize offset, isize length, u32 addr)
{
    debug(HDR_DEBUG, "read(%ld, %ld, %u)\n", offset, length, addr);

    // Check arguments
    auto error = verify(offset, length, addr);
    
    if (!error) {
        
        // Move the drive head to the specified location
        moveHead(offset / desc.geometry.bsize);

        // Perform the read operation
        mem.patch(addr, data + offset, length);
    }
    
    return error;
}

i8
HardDrive::write(isize partition, isize block, isize length, u32 addr)
{
    debug(HDR_DEBUG, "write(%ld, %ld, %ld, %u)\n", partition, block, length, addr);

    return write(offset(partition, block), length, addr);
}

i8
HardDrive::write(isize offset, isize length, u32 addr)
{
    debug(HDR_DEBUG, "write(%ld, %ld, %u)\n", offset, length, addr);

    // Check arguments
    auto error = verify(offset, length, addr);
    
    if (!error) {
    
        // Move the drive head to the specified location
        moveHead(offset / desc.geometry.bsize);

        // Perform the write operation
        if (!writeProtected) {
            mem.spypeek <ACCESSOR_CPU> (addr, length, data + offset);
        }
    }
    
    return error;
}

isize
HardDrive::offset(isize partition, isize block)
{
    return 512 * block;
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

    if (offset + length > desc.geometry.numBytes()) {
        
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
    isize c = lba / (desc.geometry.heads * desc.geometry.sectors);
    isize h = (lba / desc.geometry.sectors) % desc.geometry.heads;
    isize s = lba % desc.geometry.sectors;

    moveHead(c, h, s);
}

void
HardDrive::moveHead(isize c, isize h, isize s)
{
    bool step = head.c != c;
    
    head.c = c;
    head.h = h;
    head.s = s;
    
    if (step) msgQueue.put(MSG_HDR_STEP, c);
}
