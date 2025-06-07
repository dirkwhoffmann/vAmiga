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
#include "MutableFileSystem.h"
#include "HDFFile.h"
#include "HDZFile.h"
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
    setFlag(DiskFlags::MODIFIED, FORCE_HDR_MODIFIED);
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
    
    // Add a default partition spanning the entire disk
    auto partition = PartitionDescriptor(geometry);
    
    // Make the partition bootable
    partition.flags |= 1;
    
    // Add the descriptor to the partition table
    ptable.push_back(partition);

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
        
    // Update the partition table
    ptable[0].name = fs.getName().cpp_str();
    ptable[0].dosType = 0x444F5300 | (u32)fs.getDos();
    
    // Copy over all blocks
    fs.exportVolume(data.ptr, geometry.numBytes());
}

void 
HardDrive::init(const MediaFile &file)
{
    if (const auto *hdf = dynamic_cast<const HDFFile *>(&file)) {
        
        init(*hdf);
        return;
    }
    
    if (const auto *hdz = dynamic_cast<const HDZFile *>(&file)) {
        
        init(*hdz);
        return;
    }
    
    throw AppError(Fault::FILE_TYPE_UNSUPPORTED);
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
    geometry = hdf.geometry;
    
    // Copy partition table
    ptable = hdf.ptable;
    
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
        
    // Print some debug information
    debug(HDR_DEBUG, "%zu (needed) file system drivers\n", drivers.size());
    if (HDR_DEBUG) {
        for (auto &driver : drivers) driver.dump();
    }
}

void
HardDrive::init(const HDZFile &hdz) throws
{
    init(hdz.hdf);
}

void
HardDrive::init(const fs::path &path) throws
{
    if (!fs::exists(path)) {

        throw AppError(Fault::FILE_NOT_FOUND, path);
    }

    if (fs::is_directory(path)) {
        
        debug(HDR_DEBUG, "Importing directory...\n");
        
        importFolder(path);
        
    } else {
        
        try { init(HDFFile(path)); return; } catch(...) { }
        try { init(HDZFile(path)); return; } catch(...) { }
        
        throw AppError(Fault::FILE_TYPE_UNSUPPORTED);
    }
}

void
HardDrive::_initialize()
{

}

void
HardDrive::_didReset(bool hard)
{
    if (FORCE_HDR_MODIFIED) { setFlag(DiskFlags::MODIFIED, true); }

    // Mark all blocks as dirty
    dirty.clear(true);
}

i64
HardDrive::getOption(Opt option) const
{
    switch (option) {
            
        case Opt::HDR_TYPE:          return (long)config.type;
        case Opt::HDR_PAN:           return (long)config.pan;
        case Opt::HDR_STEP_VOLUME:   return (long)config.stepVolume;

        default:
            fatalError;
    }
}

void
HardDrive::checkOption(Opt opt, i64 value)
{
    switch (opt) {

        case Opt::HDR_TYPE:

            if (!HardDriveTypeEnum::isValid(value)) {
                throw AppError(Fault::OPT_INV_ARG, HardDriveTypeEnum::keyList());
            }
            return;

        case Opt::HDR_PAN:
        case Opt::HDR_STEP_VOLUME:
            
            return;

        default:
            throw(Fault::OPT_UNSUPPORTED);
    }
}

void
HardDrive::setOption(Opt option, i64 value)
{
    switch (option) {

        case Opt::HDR_TYPE:
            
            if (!HardDriveTypeEnum::isValid(value)) {
                throw AppError(Fault::OPT_INV_ARG, HardDriveTypeEnum::keyList());
            }
            config.type = (HardDriveType)value;
            return;

        case Opt::HDR_PAN:

            config.pan = (i16)value;
            return;

        case Opt::HDR_STEP_VOLUME:

            config.stepVolume = (u8)value;
            return;

        default:
            fatalError;
    }
}

void
HardDrive::connect()
{
    // Attach a small default disk
    if (!hasDisk()) {
        
        debug(WT_DEBUG, "Creating default disk...\n");
        init(MB(10));
        format(FSVolumeType::OFS, defaultName());
    }
}

void
HardDrive::disconnect()
{
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

bool
HardDrive::isBootable()
{
    try {
        
        if (FileSystem(*this).exists("s/startup-sequence")) {

            debug(HDR_DEBUG, "Bootable drive\n");
            return true;
        }
        
    } catch (...) {
        
        debug(HDR_DEBUG, "No file system found\n");
    }
    
    debug(HDR_DEBUG, "Unbootable drive\n");
    return false;
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
        info.writeProtected = getFlag(DiskFlags::PROTECTED);
        info.modified = getFlag(DiskFlags::MODIFIED);

        // State
        info.state = state;
        info.head = head;
    }
}

void
HardDrive::_didLoad()
{
    // Mark all blocks as dirty
    dirty.clear(true);
}

void
HardDrive::_dump(Category category, std::ostream &os) const
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
    return amiga.hdcon[objid]->getOption(Opt::HDC_CONNECT);
}

bool
HardDrive::hasDisk() const
{
    return data.ptr != nullptr;
}

bool 
HardDrive::getFlag(DiskFlags mask) const
{
    return (flags & long(mask)) == long(mask);
}

void 
HardDrive::setFlag(DiskFlags mask, bool value)
{
    value ? flags |= long(mask) : flags &= ~long(mask);
}

bool
HardDrive::hasModifiedDisk() const
{
    return hasDisk() ? getFlag(DiskFlags::MODIFIED) : false;
}

bool
HardDrive::hasProtectedDisk() const
{
    return hasDisk() && getFlag(DiskFlags::PROTECTED);
}

void
HardDrive::setModificationFlag(bool value)
{
    if (hasDisk()) setFlag(DiskFlags::MODIFIED, value);
}
void
HardDrive::setProtectionFlag(bool value)
{
    if (hasDisk()) setFlag(DiskFlags::PROTECTED, value);
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

    if (fsType != FSVolumeType::NODOS) {
        
        // Create a device descriptor matching this drive
        auto layout = FileSystemDescriptor(geometry, fsType);

        // Create an empty file system
        auto fs = MutableFileSystem(layout);
        
        // Name the file system
        fs.setName(name);
        
        // Copy the file system over
        init(fs);
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
        
        throw AppError(Fault::HDR_UNMATCHED_GEOMETRY);
    }
}

i8
HardDrive::read(isize offset, isize length, u32 addr)
{
    debug(HDR_DEBUG, "read(%ld, %ld, %u)\n", offset, length, addr);

    // Check arguments
    auto error = verify(offset, length, addr);
    
    if (!error) {

        state = HardDriveState::READING;

        // Move the drive head to the specified location
        moveHead(offset / geometry.bsize);

        // Perform the read operation
        mem.patch(addr, data.ptr + offset, length);

        // Inform the GUI
        msgQueue.put(Msg::HDR_READ);
        
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

        state = HardDriveState::WRITING;

        // Move the drive head to the specified location
        moveHead(offset / geometry.bsize);

        if (!getFlag(DiskFlags::PROTECTED)) {

            // Perform the write operation
            mem.spypeek <Accessor::CPU> (addr, length, data.ptr + offset);
            
            // Mark disk as modified
            setFlag(DiskFlags::MODIFIED, true);
        }
        
        // Inform the GUI
        msgQueue.put(Msg::HDR_WRITE);
        
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
        msgQueue.put(Msg::HDR_STEP, DriveMsg {
            i16(objid), i16(c), config.stepVolume, config.pan
        });
    }
}

void
HardDrive::importFolder(const fs::path &path) throws
{
    if (!fs::exists(path)) {

        throw AppError(Fault::FILE_NOT_FOUND, path);
    }
    
    if (fs::is_directory(path)) {
        
        debug(HDR_DEBUG, "Importing directory...\n");

        // Retrieve some information about the first partition
        auto traits = getPartitionTraits(0);
                
        // Create a device descriptor matching this drive
        FileSystemDescriptor layout(geometry, traits.fsType);
        
        // Create a new file system
        auto fs = MutableFileSystem(layout);
        
        // Import all files and name the partition
        fs.importDirectory(path);

        // Name the file system
        fs.setName(traits.name);
        
        // Copy the file system back to the disk
        init(fs);
    }
}

void
HardDrive::writeToFile(const fs::path &path) throws
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
    state = HardDriveState::IDLE;
    msgQueue.put(Msg::HDR_IDLE, objid);
}

template void HardDrive::serviceHdrEvent <SLOT_HD0> ();
template void HardDrive::serviceHdrEvent <SLOT_HD1> ();
template void HardDrive::serviceHdrEvent <SLOT_HD2> ();
template void HardDrive::serviceHdrEvent <SLOT_HD3> ();

}
