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
#include "HdrControllerTypes.h"
#include "IOUtils.h"
#include "Memory.h"
#include "MsgQueue.h"

HardDrive::HardDrive(Amiga& ref, isize nr)
: Drive(ref, nr)
{
    string path;
    
    switch (nr) {
    
        case 0: path = INITIAL_DH0; break;
        case 1: path = INITIAL_DH1; break;
        case 2: path = INITIAL_DH2; break;
        case 3: path = INITIAL_DH3; break;

        default:
            fatalError;
    }
    
    if (path != "") {
            
        // Preload the specified HDF file
        try {
            
            auto hdf = HDFFile(path);
            init(hdf);
            
            msg("HDF file %s loaded successfully\n", path.c_str());
            
        } catch (...) {
            
            warn("Cannot open HDF file %s\n", path.c_str());
        }

    } else {

        // Atach a default disk
        init(MB(10));
    }
}

HardDrive::~HardDrive()
{
    dealloc();
}

void
HardDrive::dealloc()
{
    if (data) delete [] data;

    desc = HdrvDescriptor();
    ptable.clear();
    
    driveSpec = HardDriveSpec();
    data = nullptr;
    head = {};
    modified = false;
}

void
HardDrive::init(const Geometry &geometry)
{
    // Throw an exception if the geometry is not supported
    geometry.checkCompatibility();
    
    // Wipe out the old drive
    dealloc();
    
    // Create the drive description
    desc = HdrvDescriptor(geometry);
    ptable.push_back(PartitionDescriptor(geometry));
    
    // Set product descriptions
    driveSpec.hdrv.conVendor = "VAMIGA";
    driveSpec.hdrv.conProduct = "HRDDRVCON";
    driveSpec.hdrv.conRevision = "1.0";
    driveSpec.hdrv.dskVendor = "VAMIGA";
    driveSpec.hdrv.dskProduct = "HRDDRV";
    driveSpec.hdrv.dskRevision = "1.0";

    // Save the geometry and create the new drive
    this->driveSpec.hdrv.geometry = geometry;
    data = new u8[geometry.numBytes()];
}

void
HardDrive::init(isize size)
{
    init(Geometry(size));
}

void
HardDrive::init(const MutableFileSystem &fs)
{
    auto geometry = Geometry(fs.numBytes());
    
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

    // Copy the partition table
    ptable = hdf.getPartitionDescriptors();
    
    // Copy the drive spec
    driveSpec.hdrv.geometry = hdf.getGeometry();
    driveSpec.partitions = hdf.getPartitionDescriptors();
    
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
    return driveSpec.partitions[nr];
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
    
    if (category & dump::Parameters) {

        auto &geometry = driveSpec.hdrv.geometry;
        
        auto cap1 = geometry.numBytes() / MB(1);
        auto cap2 = ((100 * geometry.numBytes()) / MB(1)) % 100;
        
        os << tab("Nr");
        os << dec(nr) << std::endl;
        os << tab("Capacity");
        os << dec(cap1) << "." << dec(cap2) << " MB" << std::endl;
        os << tab("Geometry");
        os << dec(geometry.cylinders) << " - ";
        os << dec(geometry.heads) << " - ";
        os << dec(geometry.sectors) << std::endl;
        os << tab("Block size");
        os << dec(geometry.bsize) << std::endl;
        os << tab("Disk vendor");
        os << driveSpec.hdrv.dskVendor << std::endl;
        os << tab("Disk product");
        os << driveSpec.hdrv.dskProduct << std::endl;
        os << tab("Disk revision");
        os << driveSpec.hdrv.dskRevision << std::endl;
        os << tab("Controller vendor");
        os << driveSpec.hdrv.conVendor << std::endl;
        os << tab("Controller product");
        os << driveSpec.hdrv.conProduct << std::endl;
        os << tab("Controller revision");
        os << driveSpec.hdrv.conRevision << std::endl;
    }

    if (category & dump::Volumes) {

        os << "Type   Size            Used    Free    Full  Name" << std::endl;

        for (isize i = 0; i < isize(driveSpec.partitions.size()); i++) {
            
            auto fs = MutableFileSystem(*this, i);
            fs.dump(dump::Summary, os);
        }
        
        for (isize i = 0; i < isize(driveSpec.partitions.size()); i++) {

            os << std::endl;
            os << tab("Partition");
            os << dec(i) << std::endl;
            auto fs = MutableFileSystem(*this, i);
            fs.dump(dump::Properties, os);
        }
    }
    
    if (category & dump::Partitions) {
        
        for (usize i = 0; i < driveSpec.partitions.size(); i++) {
            
            auto &part = driveSpec.partitions[i];
            
            if (i != 0) os << std::endl;
            os << tab("Partition");
            os << dec(i) << std::endl;
            
            os << tab("Name");
            os << part.name << std::endl;
            os << tab("Flags");
            os << dec(part.flags) << std::endl;
            os << tab("SizeBlock");
            os << dec(part.sizeBlock) << std::endl;
            os << tab("Heads");
            os << dec(part.heads) << std::endl;
            os << tab("Sectors");
            os << dec(part.sectors) << std::endl;
            os << tab("Reserved");
            os << dec(part.reserved) << std::endl;
            os << tab("Interleave");
            os << dec(part.interleave) << std::endl;
            os << tab("LowCyl");
            os << dec(part.lowCyl) << std::endl;
            os << tab("HighCyl");
            os << dec(part.highCyl) << std::endl;
            os << tab("NumBuffers");
            os << dec(part.numBuffers) << std::endl;
            os << tab("BufMemType");
            os << dec(part.bufMemType) << std::endl;
            os << tab("MaxTransfer");
            os << dec(part.maxTransfer) << std::endl;
            os << tab("Mask");
            os << dec(part.mask) << std::endl;
            os << tab("BootPrio");
            os << dec(part.bootPri) << std::endl;
            os << tab("DosType");
            os << dec(part.dosType) << std::endl;
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

    applyToPersistentItems(counter);
    applyToResetItems(counter);
    
    // Add the disk size
    counter.count += driveSpec.hdrv.geometry.numBytes();
    
    return counter.count;
}

isize
HardDrive::didLoadFromBuffer(const u8 *buffer)
{
    util::SerReader reader(buffer);

    // Create the drive
    init(driveSpec.hdrv.geometry);
    
    // Load disk data
    reader.copy(data, driveSpec.hdrv.geometry.numBytes());

    return (isize)(reader.ptr - buffer);
}

isize
HardDrive::didSaveToBuffer(u8 *buffer) const
{
    util::SerWriter writer(buffer);

    // Save memory contents
    writer.copy(data, driveSpec.hdrv.geometry.numBytes());
        
    return (isize)(writer.ptr - buffer);
}

void
HardDrive::format(FSVolumeType fsType, BootBlockId bb)
{
    debug(HDR_DEBUG, "Formatting drive\n");
    debug(HDR_DEBUG, "    File system: %s\n", FSVolumeTypeEnum::key(fsType));
    debug(HDR_DEBUG, "     Boot block: %s\n", BootBlockIdEnum::key(bb));

    // Only proceed if a disk is present
    if (!data) return;

    if (fsType != FS_NODOS) {
        
        // Create a device descriptor matching this drive
        auto layout = FileSystemDescriptor(driveSpec.hdrv.geometry, fsType);

        // Create a file system
        auto fs = MutableFileSystem(layout);
        
        // Add a boot block
        fs.makeBootable(bb);
        
        // REMOVE ASAP
        msg("File system:\n");
        fs.dump();
        
        // Copy all blocks over
        fs.exportVolume(data, driveSpec.hdrv.geometry.numBytes());
    }
}

void
HardDrive::changeGeometry(isize c, isize h, isize s, isize b)
{
    auto geometry = Geometry(c, h, s, b);
    changeGeometry(geometry);
}

void
HardDrive::changeGeometry(const Geometry &geometry)
{
    geometry.checkCompatibility();
        
    if (this->driveSpec.hdrv.geometry.numBytes() == driveSpec.hdrv.geometry.numBytes()) {
        
        this->driveSpec.hdrv.geometry = geometry;
    
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
        moveHead(offset / driveSpec.hdrv.geometry.bsize);

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
        moveHead(offset / driveSpec.hdrv.geometry.bsize);

        // Perform the read operation
        mem.spypeek <ACCESSOR_CPU> (addr, length, data + offset);
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

    if (offset + length > driveSpec.hdrv.geometry.numBytes()) {
        
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
    isize c = lba / (driveSpec.hdrv.geometry.heads * driveSpec.hdrv.geometry.sectors);
    isize h = (lba / driveSpec.hdrv.geometry.sectors) % driveSpec.hdrv.geometry.heads;
    isize s = lba % driveSpec.hdrv.geometry.sectors;

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
