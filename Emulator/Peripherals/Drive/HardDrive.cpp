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

std::fstream HardDrive::wtStream[4];

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

HardDrive::~HardDrive()
{
    disableWriteThrough();
}

void
HardDrive::init()
{
    data.dealloc();

    diskVendor = "VAMIGA";
    diskProduct = "VDRIVE";
    diskRevision = "1.0";
    controllerVendor = amiga.hdcon[nr]->vendorName();
    controllerProduct = amiga.hdcon[nr]->productName();
    controllerRevision = amiga.hdcon[nr]->revisionName();
    geometry = GeometryDescriptor();
    ptable.clear();
    drivers.clear();
    head = {};
    modified = bool(FORCE_HDR_MODIFIED);
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
    if (writeThrough) {
        
        // Delete the existing image
        disableWriteThrough();
        
        // Recreate the image with the new disk
        enableWriteThrough();
    }
    
    // Print some debug information
    debug(HDR_DEBUG, "%zu (needed) file system drivers\n", drivers.size());
    if constexpr (HDR_DEBUG) {
        for (auto &driver : drivers) driver.dump();
    }
}

void
HardDrive::init(const string &path) throws
{
    HDFFile hdf(path);
    init(hdf);
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

void
HardDrive::resetConfig()
{
    assert(isPoweredOff());
    auto &defaults = amiga.properties;

    std::vector <Option> options = {
        
        OPT_HDR_TYPE,
        OPT_HDR_PAN,
        OPT_HDR_STEP_VOLUME,
    };

    for (auto &option : options) {
        setConfigItem(option, defaults.get(option, nr));
    }
}

i64
HardDrive::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_HDR_TYPE:          return (long)config.type;
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

        } catch (VAError &e) {
    
            warn("Error: %s\n", e.what());
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
HardDrive::getPartitionInfo(isize nr)
{
    assert(nr >= 0 && nr < numPartitions());
    return ptable[nr];
}

HdcState
HardDrive::getHdcState()
{
    return amiga.hdcon[nr]->getHdcState();
}

bool
HardDrive::isCompatible()
{
    return amiga.hdcon[nr]->isCompatible();
}

void
HardDrive::_inspect() const
{
    {   SYNCHRONIZED
        
        info.modified = isModified();
        info.head = head;
    }
}

isize
HardDrive::didLoadFromBuffer(const u8 *buffer)
{
    disableWriteThrough();
    return 0;
}

void
HardDrive::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Config) {
        
        os << tab("Nr");
        os << dec(nr) << std::endl;
        os << tab("Type");
        os << HardDriveTypeEnum::key(config.type) << std::endl;
        os << tab("Step volume");
        os << dec(config.stepVolume) << std::endl;
        os << tab("Pan");
        os << dec(config.pan) << std::endl;
    }
    
    if (category == Category::Drive) {
        
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
    
    if (category == Category::Volumes) {
        
        os << "Type   Size            Used    Free    Full  Name" << std::endl;
        
        for (isize i = 0; i < isize(ptable.size()); i++) {
            
            auto fs = MutableFileSystem(*this, i);
            fs.dump(Category::Summary, os);
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
    
    if (category == Category::State) {
        
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
        os << tab("Bootable");
        if (bootable) {
            os << bol(*bootable) << std::endl;
        } else {
            os << "Unknown" << std::endl;
        }
    }
}

bool
HardDrive::isConnected() const
{
    return amiga.hdcon[nr]->getConfigItem(OPT_HDC_CONNECT);
}

u64
HardDrive::fnv() const
{
    return hasDisk() ? util::fnv64(data.ptr, geometry.numBytes()) : 0;
}

bool
HardDrive::hasDisk() const
{
    return data.ptr != nullptr;
}

bool
HardDrive::hasModifiedDisk() const
{
    return hasDisk() ? modified : false;
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

void
HardDrive::enableWriteThrough()
{
    debug(WT_DEBUG, "enableWriteThrough()\n");
    
    if (!writeThrough) {
    
        saveWriteThroughImage();
      
        debug(WT_DEBUG, "Write-through mode enabled\n");
        writeThrough = true;
    }
}

void
HardDrive::disableWriteThrough()
{
    if (writeThrough) {
        
        // Close file
        wtStream[nr].close();
        
        debug(WT_DEBUG, "Write-through mode disabled\n");
        writeThrough = false;
    }
}

string
HardDrive::writeThroughPath()
{
    return Amiga::properties.getString("HD" + std::to_string(nr) + "_PATH");
}

void
HardDrive::saveWriteThroughImage()
{
    auto path = writeThroughPath();
    
    // Only proceed if a storage file is given
    if (path.empty()) {
        throw VAError(ERROR_WT, "No storage path specified");
    }
    
    // Only proceed if no other emulator instance is using the storage file
    if (wtStream[nr].is_open()) {
        throw VAError(ERROR_WT_BLOCKED);
    }
    
    // Delete the old storage file
    fs::remove(path);
    
    // Recreate the storage file with the contents of this disk
    writeToFile(path);
    if (!util::fileExists(path)) {
        throw VAError(ERROR_WT, "Can't create storage file");
    }
    // Open file
    wtStream[nr].open(path, std::ios::binary | std::ios::in | std::ios::out);
    if (!wtStream[nr].is_open()) {
        throw VAError(ERROR_WT, "Can't open storage file");
    }
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

        if (!writeProtected) {

            // Perform the write operation
            mem.spypeek <ACCESSOR_CPU> (addr, length, data.ptr + offset);
            
            // Handle write-through mode
            if (writeThrough) {
                wtStream[nr].seekp(offset);
                wtStream[nr].write((char *)(data.ptr + offset), length);
            }
            
            modified = true;
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
        msgQueue.put(MSG_HDR_STEP, i16(nr), i16(c), config.stepVolume, config.pan);
    }
}

/*
bool
HardDrive::persistDisk() throws
{
    if (!backup.empty()) try {
        
        auto hdf = HDFFile(*this);
        hdf.writeToFile(backup);
        msg("HD%ld persisted at %s\n", nr, backup.c_str());
        
    } catch (...) {
        
        warn("Failed to persist HD%ld at %s\n", nr, backup.c_str());
        return false;
    }
    
    return true;
}
*/

void
HardDrive::writeToFile(const string &path) throws
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
    agnus.cancel <s> ();
    state = HDR_STATE_IDLE;
    msgQueue.put(MSG_HDR_IDLE, nr);
}

template void HardDrive::serviceHdrEvent <SLOT_HD0> ();
template void HardDrive::serviceHdrEvent <SLOT_HD1> ();
template void HardDrive::serviceHdrEvent <SLOT_HD2> ();
template void HardDrive::serviceHdrEvent <SLOT_HD3> ();
