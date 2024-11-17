// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "HardDrive.h"
#include "Emulator.h"
#include "HdControllerTypes.h"
#include "IOUtils.h"
#include "Memory.h"
#include "MsgQueue.h"

namespace vamiga {

std::fstream HardDrive::wtStream[4];

HardDrive::HardDrive(Amiga& ref, isize nr) : Drive(ref, nr)
{

}

HardDrive::~HardDrive()
{
    disableWriteThrough();
}

HardDrive& 
HardDrive::operator= (const HardDrive& other) {

    CLONE(config)

    CLONE(diskVendor)
    CLONE(diskProduct)
    CLONE(diskRevision)
    CLONE(controllerVendor)
    CLONE(controllerProduct)
    CLONE(controllerRevision)
    CLONE(geometry)
    CLONE(ptable)
    CLONE(drivers)
    CLONE(head)
    CLONE(state)
    CLONE(flags)
    CLONE(bootable)

    if (RUA_ON_STEROIDS) {

        // Clone all blocks
        CLONE(data)

    } else {

        // Clone dirty blocks
        data.resize(other.data.size);
        for (isize i = 0; i < other.dirty.size; i++) {

            if (other.dirty[i]) {

                debug(RUA_DEBUG, "Cloning block %ld\n", i);
                memcpy(data.ptr + 512 * i, other.data.ptr + 512 * i, 512);
            }
        }
    }

    return *this;
}

void
HardDrive::init()
{
    data.dealloc();
    dirty.dealloc();

    diskVendor = "VAMIGA";
    diskProduct = "VDRIVE";
    diskRevision = "1.0";
    controllerVendor = "RASTEC";
    controllerProduct = "HD controller";
    controllerRevision = "0.3";
    geometry = GeometryDescriptor();
    ptable.clear();
    drivers.clear();
    head = {};
    setFlag(FLAG_MODIFIED, FORCE_HDR_MODIFIED);
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
    data.resize(geometry.numBytes());
    dirty.resize(geometry.numBytes() / 512, true);
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
    fs.exportVolume(data.ptr, geometry.numBytes());
}

void 
HardDrive::init(const MediaFile &file)
{
    try {

        const HDFFile &hdf = dynamic_cast<const HDFFile &>(file);
        init(hdf);

    } catch (...) {

        throw Error(VAERROR_FILE_TYPE_MISMATCH);
    }
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
    geometry = hdf.getGeometryDescriptor(); // TODO: Replace by " = hdf.geometry" (?!)
    
    // Copy partition table
    ptable = hdf.getPartitionDescriptors();  // TODO: Replace by " = hdf.ptable" (?!)

    // Copy over all needed file system drivers
    for (const auto &driver : hdf.drivers) {

        bool needed = HDR_FS_LOAD_ALL;

        for (const auto &part : ptable) {
            if (driver.dosType == part.dosType) {

                needed = true;
                break;
            }
        }
        if (needed) { drivers.push_back(driver); }
    }
    
    // Check the drive geometry against the file size
    auto numBytes = hdf.data.size;
    
    if (data.size < numBytes) {
        
        debug(HDR_DEBUG, "HDF is too large. Ignoring excess bytes.\n");
        numBytes = data.size;
    }
    if (data.size > hdf.data.size) {
        
        debug(HDR_DEBUG, "HDF is too small. Padding with zeroes.");
        data.clear(0, hdf.data.size);
    }
    
    // Copy over all blocks
    hdf.flash(data.ptr, 0, numBytes);
    
    // Replace the write-through image on disk
    if (config.writeThrough) {

        // Delete the existing image
        disableWriteThrough();
        
        // Recreate the image with the new disk
        enableWriteThrough();
    }
    
    // Print some debug information
    debug(HDR_DEBUG, "%zu (needed) file system drivers\n", drivers.size());
    if (HDR_DEBUG) {
        for (auto &driver : drivers) driver.dump();
    }
}

void
HardDrive::init(const std::filesystem::path &path) throws
{
    HDFFile hdf(path);
    init(hdf);
}

void
HardDrive::_initialize()
{

}

void
HardDrive::_didReset(bool hard)
{
    if (FORCE_HDR_MODIFIED) { setFlag(FLAG_MODIFIED, true); }

    // Mark all blocks as dirty
    dirty.clear(true);
}

i64
HardDrive::getOption(Option option) const
{
    switch (option) {
            
        case OPT_HDR_TYPE:          return (long)config.type;
        case OPT_HDR_WRITE_THROUGH: return (long)config.writeThrough;
        case OPT_HDR_PAN:           return (long)config.pan;
        case OPT_HDR_STEP_VOLUME:   return (long)config.stepVolume;

        default:
            fatalError;
    }
}

void
HardDrive::checkOption(Option opt, i64 value)
{
    switch (opt) {

        case OPT_HDR_TYPE:

            if (!HardDriveTypeEnum::isValid(value)) {
                throw Error(VAERROR_OPT_INV_ARG, HardDriveTypeEnum::keyList());
            }
            return;

        case OPT_HDR_WRITE_THROUGH:

            return;

        case OPT_HDR_PAN:
        case OPT_HDR_STEP_VOLUME:
            
            return;

        default:
            throw(VAERROR_OPT_UNSUPPORTED);
    }
}

void
HardDrive::setOption(Option option, i64 value)
{
    switch (option) {

        case OPT_HDR_TYPE:
            
            if (!HardDriveTypeEnum::isValid(value)) {
                throw Error(VAERROR_OPT_INV_ARG, HardDriveTypeEnum::keyList());
            }
            config.type = (HardDriveType)value;
            return;

        case OPT_HDR_WRITE_THROUGH:

            value ? enableWriteThrough() : disableWriteThrough();
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

void
HardDrive::connect()
{
    auto path = writeThroughPath();
    
    if (!path.empty()) {
        
        try {
            
            debug(WT_DEBUG, "Reading disk from %s...\n", path.c_str());
            auto hdf = HDFFile(path);
            init(hdf);

            debug(WT_DEBUG, "Trying to enable write-through mode...\n");
            enableWriteThrough();

            debug(WT_DEBUG, "Success\n");

        } catch (Error &e) {

            warn("%s\n", e.what());
        }
    }
    
    // Attach a small default disk
    if (!hasDisk()) {
        
        debug(WT_DEBUG, "Creating default disk...\n");
        init(MB(10));
        format(FS_OFS, defaultName());
        bootable = false;
    }
}

void
HardDrive::disconnect()
{
    disableWriteThrough();
    init();
}

const PartitionDescriptor &
HardDrive::getPartitionDescriptor(isize nr) const
{
    assert(nr >= 0 && nr < numPartitions());
    return ptable[nr];
}

HdcState
HardDrive::getHdcState() const
{
    return amiga.hdcon[objid]->getHdcState();
}

bool
HardDrive::isCompatible() const
{
    return amiga.hdcon[objid]->isCompatible();
}

void
HardDrive::cacheInfo(HardDriveInfo &info) const
{
    {   SYNCHRONIZED
        
        info.nr = objid;
        
        info.isConnected = isConnected();
        info.isCompatible = isCompatible();

        info.hasDisk = hasDisk();
        info.hasModifiedDisk = hasModifiedDisk();
        info.hasUnmodifiedDisk = hasUnmodifiedDisk();
        info.hasProtectedDisk = hasProtectedDisk();
        info.hasUnprotectedDisk = hasUnprotectedDisk();

        info.partitions = numPartitions();

        // Flags
        info.writeProtected = getFlag(FLAG_PROTECTED);
        info.modified = getFlag(FLAG_MODIFIED);

        // State
        info.state = state;
        info.head = head;
    }
}

void
HardDrive::_didLoad()
{
    disableWriteThrough();

    // Mark all blocks as dirty
    dirty.clear(true);
}

void
HardDrive::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Config) {
        
        dumpConfig(os);
    }
    
    if (category == Category::State) {
        
        auto cap1 = geometry.numBytes() / MB(1);
        auto cap2 = ((100 * geometry.numBytes()) / MB(1)) % 100;
        
        os << tab("Hard drive");
        os << dec(objid) << std::endl;
        os << tab("Head");
        os << dec(head.cylinder) << ":" << dec(head.head) << ":" << dec(head.offset);
        os << std::endl;
        os << tab("State");
        os << HardDriveStateEnum::key(state) << std::endl;
        os << tab("Flags");
        os << DiskFlagsEnum::mask(flags) << std::endl;
        os << tab("Bootable");
        if (bootable) {
            os << bol(*bootable) << std::endl;
        } else {
            os << "Unknown" << std::endl;
        }
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
    
    if (category == Category::Volumes) {
        
        os << "Type   Size            Used    Free    Full  Name" << std::endl;
        
        for (isize i = 0; i < isize(ptable.size()); i++) {
            
            auto fs = MutableFileSystem(*this, i);
            fs.dump(Category::State, os);
        }
        
        for (isize i = 0; i < isize(ptable.size()); i++) {
            
            os << std::endl;
            os << tab("Partition");
            os << dec(i) << std::endl;
            auto fs = MutableFileSystem(*this, i);
            fs.dump(Category::Properties, os);
        }
    }
    
    if (category == Category::Partitions) {
        
        for (usize i = 0; i < ptable.size(); i++) {
            
            auto &part = ptable[i];
            
            if (i != 0) os << std::endl;
            os << tab("Partition");
            os << dec(i) << std::endl;
            part.dump(os);
        }
    }
}

bool
HardDrive::isConnected() const
{
    return amiga.hdcon[objid]->getOption(OPT_HDC_CONNECT);
}

bool
HardDrive::hasDisk() const
{
    return data.ptr != nullptr;
}

bool 
HardDrive::getFlag(DiskFlags mask) const
{
    return (flags & mask) == mask;
}

void 
HardDrive::setFlag(DiskFlags mask, bool value)
{
    value ? flags |= mask : flags &= ~mask;
}

bool
HardDrive::hasModifiedDisk() const
{
    return hasDisk() ? getFlag(FLAG_MODIFIED) : false;
}

bool
HardDrive::hasProtectedDisk() const
{
    return hasDisk() && getFlag(FLAG_PROTECTED);
}

void
HardDrive::setModificationFlag(bool value)
{
    if (hasDisk()) setFlag(FLAG_MODIFIED, value);
}
void
HardDrive::setProtectionFlag(bool value)
{
    if (hasDisk()) setFlag(FLAG_PROTECTED, value);
}

void
HardDrive::enableWriteThrough()
{
    debug(WT_DEBUG, "enableWriteThrough()\n");
    
    if (!config.writeThrough) {

        saveWriteThroughImage();

        debug(WT_DEBUG, "Write-through mode enabled\n");
        config.writeThrough = true;
    }
}

void
HardDrive::disableWriteThrough()
{
    if (config.writeThrough) {

        // Close file
        wtStream[objid].close();
        
        debug(WT_DEBUG, "Write-through mode disabled\n");
        config.writeThrough = false;
    }
}

string
HardDrive::writeThroughPath()
{
    return Emulator::defaults.getRaw("HD" + std::to_string(objid) + "_PATH");
}

void
HardDrive::saveWriteThroughImage()
{
    auto path = writeThroughPath();
    
    // Only proceed if a storage file is given
    if (path.empty()) {
        throw Error(VAERROR_WT, "No storage path specified");
    }
    
    // Only proceed if no other emulator instance is using the storage file
    if (wtStream[objid].is_open()) {
        throw Error(VAERROR_WT_BLOCKED);
    }
    
    // Delete the old storage file
    fs::remove(path);
    
    // Recreate the storage file with the contents of this disk
    writeToFile(path);
    if (!util::fileExists(path)) {
        throw Error(VAERROR_WT, "Can't create storage file");
    }

    // Open file
    wtStream[objid].open(path, std::ios::binary | std::ios::in | std::ios::out);
    if (!wtStream[objid].is_open()) {
        throw Error(VAERROR_WT, "Can't open storage file");
    }
}

string
HardDrive::defaultName(isize partition) const
{
    if (objid >= 1) partition += amiga.hd0.numPartitions();
    if (objid >= 2) partition += amiga.hd1.numPartitions();
    if (objid >= 3) partition += amiga.hd2.numPartitions();

    return "DH" + std::to_string(partition);
}

void
HardDrive::format(FSVolumeType fsType, string name)
{
    if (HDR_DEBUG) {

        msg("Formatting hard drive\n");
        msg("    File system : %s\n", FSVolumeTypeEnum::key(fsType));
        msg("           Name : %s\n", name.c_str());
    }
    
    // Only proceed if a disk is present
    if (!data.ptr) return;

    if (fsType != FS_NODOS) {
        
        // Create a device descriptor matching this drive
        auto layout = FileSystemDescriptor(geometry, fsType);

        // Create a file system
        auto fs = MutableFileSystem(layout);

        // Add name and bootblock
        fs.setName(name);

        // Copy all blocks over
        fs.exportVolume(data.ptr, geometry.numBytes());
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
        
        throw Error(VAERROR_HDR_UNMATCHED_GEOMETRY);
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
        mem.patch(addr, data.ptr + offset, length);

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

        if (!getFlag(FLAG_PROTECTED)) {

            // Perform the write operation
            mem.spypeek <ACCESSOR_CPU> (addr, length, data.ptr + offset);
            
            // Handle write-through mode
            if (config.writeThrough) {
                
                wtStream[objid].seekp(offset);
                wtStream[objid].write((char *)(data.ptr + offset), length);
            }
            
            setFlag(FLAG_MODIFIED, true);
        }
        
        // Inform the GUI
        msgQueue.put(MSG_HDR_WRITE);
        
        // Go back to IDLE state after some time
        scheduleIdleEvent();
    }
    
    return error;
}

void
HardDrive::readDriver(isize nr, Buffer<u8> &driver)
{
    assert(usize(nr) < drivers.size());
    
    auto &segList = drivers[nr].blocks;
    auto bytesPerBlock = geometry.bsize - 20;

    driver.init(isize(segList.size()) * bytesPerBlock);
    
    isize bytesRead = 0;
    for (auto &seg : segList) {

        auto offset = isize(seg * geometry.bsize + 20);

        assert(offset >= 0);
        assert(offset + bytesPerBlock <= data.size);
        
        memcpy(driver.ptr + bytesRead, data.ptr + offset, bytesPerBlock);
        bytesRead += bytesPerBlock;
    }
}

i8
HardDrive::verify(isize offset, isize length, u32 addr)
{
    assert(data.ptr);

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
        msgQueue.put(MSG_HDR_STEP, DriveMsg {
            i16(objid), i16(c), config.stepVolume, config.pan
        });
    }
}

void
HardDrive::writeToFile(const std::filesystem::path &path) throws
{
    if (!path.empty()) {

        auto hdf = HDFFile(*this);
        hdf.writeToFile(path);
    }
}

void
HardDrive::scheduleIdleEvent()
{
    auto delay = MSEC(100);
    
    switch (objid) {
            
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
    agnus.cancel <s> ();
    state = HDR_STATE_IDLE;
    msgQueue.put(MSG_HDR_IDLE, objid);
}

template void HardDrive::serviceHdrEvent <SLOT_HD0> ();
template void HardDrive::serviceHdrEvent <SLOT_HD1> ();
template void HardDrive::serviceHdrEvent <SLOT_HD2> ();
template void HardDrive::serviceHdrEvent <SLOT_HD3> ();

}
