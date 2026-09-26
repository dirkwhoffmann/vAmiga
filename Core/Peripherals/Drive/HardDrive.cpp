// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "HardDrive.h"
#include "Emulator.h"
#include "Codecs.h"
#include "DeviceError.h"
#include "Memory.h"
#include "MsgQueue.h"
#include "utl/io.h"

namespace vamiga {

using namespace retro::vault;

HardDrive::HardDrive(Amiga& ref, isize nr) : Drive(ref, nr)
{
    info.bind([this] { return cacheInfo(); } );
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

    // Share the disk rather than copying it (see 'image')
    image = other.image;

    return *this;
}

void
HardDrive::init()
{
    image = nullptr;
    pending = false;

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
    setFlag(DiskFlags::BOOTABLE, false);
    setFlag(DiskFlags::MODIFIED, HDR_MODIFIED);
}

void
HardDrive::setup(const GeometryDescriptor &geometry)
{
    // Throw an exception if the geometry is not supported
    geometry.checkCompatibility(mbLimit());

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

    // User-provided disks are bootable by default
    setFlag(DiskFlags::BOOTABLE, true);
}

void
HardDrive::init(const GeometryDescriptor &geometry)
{
    // The disk will live in memory, so it must not be too large
    checkMemoryLimit(geometry.numBytes());

    setup(geometry);

    // Create an empty disk in memory
    image = std::make_shared<HDFFile>(geometry.numBytes());
}

void
HardDrive::checkMemoryLimit(i64 bytes) const
{
    auto limit = i64(memLimit());

    if (limit && bytes > limit * MB(1)) {
        throw DeviceError(DeviceError::HDR_TOO_LARGE_FOR_MEM, std::to_string(limit));
    }
}

void
HardDrive::loadIntoMemory()
{
    if (!fileBacked()) return;

    checkMemoryLimit(size());
    image->detach();
}

void
HardDrive::init(i64 size)
{
    init(GeometryDescriptor(size));
}

void
HardDrive::init(const FileSystem &fs)
{
    auto geometry = GeometryDescriptor(fs.bytes());

    // Create the drive
    init(geometry);

    // Update the partition table
    ptable[0].name = fs.stat().name.cpp_str();
    ptable[0].dosType = 0x444F5300 | (u32)fs.getTraits().dos;

    // Copy over all blocks
    auto bytes = image->mutableByteView(0, geometry.numBytes());
    fs.exporter.exportVolume(bytes.data(), bytes.size());
}

void
HardDrive::describe(const FileSystem &fs)
{
    /* The file system was built on top of this drive, so the disk holds it
     * already. Only the description changes, to a single partition spanning
     * the file system. The disk is kept: a drive that lives in a file stays
     * there.
     */
    auto disk = image;
    setup(GeometryDescriptor(fs.bytes()));
    image = disk;

    // Update the partition table
    ptable[0].name = fs.stat().name.cpp_str();
    ptable[0].dosType = 0x444F5300 | (u32)fs.getTraits().dos;
}

void
HardDrive::init(std::unique_ptr<HDFFile> hdf)
{
    assert(hdf);

    auto geometry = hdf->getGeometry();

    /* The image has to cover the geometry. HDFFile pads short files to the
     * size their RDB describes, so a well-formed image always does. Checked
     * before anything else, so that a failure leaves the old drive in place.
     */
    if (hdf->getSize() < geometry.numBytes()) {
        throw DeviceError(DeviceError::HDR_UNMATCHED_GEOMETRY);
    }
    if (hdf->getSize() > geometry.numBytes()) {
        logmsg(LOG_HDR, "HDF is too large. Ignoring excess bytes.\n");
    }

    // Describe the drive
    setup(geometry);

    // Copy the product description (if provided by the HDF)
    if (auto value = hdf->getDiskProduct(); value) diskProduct = *value;
    if (auto value = hdf->getDiskVendor(); value) diskVendor = *value;
    if (auto value = hdf->getDiskRevision(); value) diskRevision = *value;
    if (auto value = hdf->getControllerProduct(); value) controllerProduct = *value;
    if (auto value = hdf->getControllerVendor(); value) controllerVendor = *value;
    if (auto value = hdf->getControllerRevision(); value) controllerRevision = *value;

    // Copy partition table
    ptable = hdf->ptable;

    // Copy over all needed file system drivers
    for (const auto &driver : hdf->drivers) {

        bool needed = HDR_FS_LOAD_ALL;

        for (const auto &part : ptable) {
            if (driver.dosType == part.dosType) {

                needed = true;
                break;
            }
        }
        if (needed) { drivers.push_back(driver); }
    }

    // Take over the image. Nothing is copied, and nothing is read yet.
    image = std::move(hdf);

    // Print some debug information
    logmsg(LOG_HDR, "%zu (needed) file system drivers\n", drivers.size());
    if CONSTEXPR (LOG_HDR != LOG_OFF) {
        for (auto &driver : drivers) driver.dump();
    }
}

void
HardDrive::init(const fs::path &path, StorageMode mode)
{
    if (!fs::exists(path)) {

        throw IOError(IOError::FILE_NOT_FOUND, path);
    }

    if (fs::is_directory(path)) {

        logmsg(LOG_HDR, "Importing directory...\n");

        importFolder(path);
        return;
    }

    auto hdf = std::make_unique<HDFFile>(path);

    /* Take the disk into memory before the drive takes it over, so that a
     * disk that is too large, or cannot be read in full, leaves the old
     * drive in place.
     */
    if (mode == StorageMode::MEMORY_BACKED) {

        checkMemoryLimit(hdf->getGeometry().numBytes());
        hdf->detach();
    }

    init(std::move(hdf));
}

void
HardDrive::_initialize()
{

}

void
HardDrive::_didReset(bool hard)
{
    if CONSTEXPR (HDR_MODIFIED)
        setFlag(DiskFlags::MODIFIED, true);
}

namespace {

// First value of a serialized disk that is stored as a path (see serializeDisk)
constexpr i64 diskIsPath = -1;

// ... and of one the limit kept out of the snapshot
constexpr i64 diskIsOmitted = -2;

}

bool
HardDrive::snapshotable() const
{
    return image && size() <= config.snapshotLimit * 1024 * 1024;
}

void
HardDrive::serializeDisk(SerCounter &worker)
{
    // Counting and writing have to agree, so they ask the same question
    if (fileBacked()) {
        worker.count += 8 + 8 + isize(image->path.string().size());
    } else if (snapshotable()) {
        worker.count += 8 + size();
    } else {
        worker.count += 8;
    }
}

void
HardDrive::serializeDisk(SerChecker &worker)
{
    /* The bytes a snapshot actually carries, checked the way a utl::Buffer<u8>
     * is checked (older snapshots were written with one). A disk that lives
     * in a file counts as empty: the snapshot stores its path only, and
     * whether the file still opens when the snapshot is restored must not
     * decide whether the snapshot is taken for intact. So does a disk past
     * HDR_SNAPSHOT_LIMIT -- what is not written cannot come back, and
     * checking it would declare every such snapshot corrupt. The run-ahead instance
     * shares the disk with the main instance, so comparing the two is not
     * affected either way.
     */
    auto bytes = snapshotable() ? image->byteView(0, size()) : utl::ByteView();
    worker.hash = utl::Hashable::fnvIt64(worker.hash, bytes.fnv64());
}

void
HardDrive::serializeDisk(SerWriter &worker)
{
    if (fileBacked()) {

        // Store the path of the file
        auto path = image->path.string();
        i64 marker = diskIsPath, len = i64(path.size());
        worker << marker << len;
        std::memcpy(worker.ptr, path.data(), path.size());
        worker.ptr += len;

    } else if (snapshotable()) {

        // Store the disk in full
        i64 len = size();
        worker << len;

        if (len) {

            std::memcpy(worker.ptr, image->byteView(0, len).data(), size_t(len));
            worker.ptr += len;
        }

    } else {

        // Say that it was left out, so the reader knows what it is missing
        i64 marker = diskIsOmitted;
        worker << marker;

        if (image) {
            logmsg(LOG_HDR, "hd%ld: Disk left out of the snapshot (%ld MB)\n",
                   (long)objid, (long)(size() / (1024 * 1024)));
        }
    }
}

void
HardDrive::serializeDisk(SerReader &worker)
{
    i64 len;
    worker << len;

    image = nullptr;

    if (len == diskIsOmitted) {

        // The drive comes back empty (see serializeDisk)
        logmsg(LOG_HDR, "hd%ld: Snapshot carries no disk\n", (long)objid);
        return;
    }

    if (len == diskIsPath) {

        i64 plen;
        worker << plen;
        auto path = fs::path(string((const char *)worker.ptr, size_t(plen)));
        worker.ptr += plen;

        // Open the file again, as it is now
        try {

            auto hdf = std::make_shared<HDFFile>(path);

            if (hdf->getSize() >= geometry.numBytes()) {
                image = hdf;
            } else {
                logmsg(LOG_HDR, "%s no longer covers the drive\n", path.string().c_str());
            }

        } catch (...) {

            logmsg(LOG_HDR, "Cannot open %s\n", path.string().c_str());
        }

        // If the file cannot be used, the drive is left without a disk (see _didLoad)

    } else if (len > 0) {

        // Restore the disk in memory
        auto restored = std::make_shared<HDFFile>(isize(len));
        std::memcpy(restored->mutableByteView(0, len).data(), worker.ptr, size_t(len));
        worker.ptr += len;
        image = restored;
    }
}

void
HardDrive::_didLoad()
{
    // A drive whose disk could not be restored has none (see serializeDisk)
    if (!image) init();
}

i64
HardDrive::getOption(Opt option) const
{
    switch (option) {

        case Opt::HDR_TYPE:          return (long)config.type;
        case Opt::HDR_PAN:           return (long)config.pan;
        case Opt::HDR_STEP_VOLUME:   return (long)config.stepVolume;
        case Opt::HDR_WRITE_THROUGH: return (long)config.writeThrough;
        case Opt::HDR_SNAPSHOT_LIMIT: return (long)config.snapshotLimit;

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
                throw CoreError(CoreError::OPT_INV_ARG, HardDriveTypeEnum::keyList());
            }
            return;

        case Opt::HDR_PAN:
        case Opt::HDR_STEP_VOLUME:

            return;

        case Opt::HDR_SNAPSHOT_LIMIT:

            // A limit of zero is allowed: it keeps every disk out
            if (value < 0) throw CoreError(CoreError::OPT_INV_ARG, "0...");
            return;

        case Opt::HDR_WRITE_THROUGH:

            if (!WriteThroughModeEnum::isValid(value)) {
                throw CoreError(CoreError::OPT_INV_ARG, WriteThroughModeEnum::keyList());
            }
            return;

        default:
            throw CoreError(CoreError::OPT_UNSUPPORTED);
    }
}

void
HardDrive::setOption(Opt option, i64 value)
{
    switch (option) {

        case Opt::HDR_TYPE:

            if (!HardDriveTypeEnum::isValid(value)) {
                throw CoreError(CoreError::OPT_INV_ARG, HardDriveTypeEnum::keyList());
            }
            config.type = (HardDriveType)value;
            return;

        case Opt::HDR_PAN:

            config.pan = (i16)value;
            return;

        case Opt::HDR_STEP_VOLUME:

            config.stepVolume = (u8)value;
            return;

        case Opt::HDR_SNAPSHOT_LIMIT:

            config.snapshotLimit = isize(value);
            return;

        case Opt::HDR_WRITE_THROUGH:

            if (!WriteThroughModeEnum::isValid(value)) {
                throw CoreError(CoreError::OPT_INV_ARG, WriteThroughModeEnum::keyList());
            }
            config.writeThrough = WriteThroughMode(value);

            /* From now on the file follows the disk. It starts out holding
             * what has changed so far, so that it does not stay behind by an
             * amount nobody can tell.
             */
            if (config.writeThrough != WriteThroughMode::NEVER) persist();
            return;

        default:
            fatalError;
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
    return hasDisk() && getFlag(DiskFlags::BOOTABLE);
}

HardDriveInfo
HardDrive::cacheInfo() const
{
    HardDriveInfo info;

    info.nr = objid;

    info.isConnected = isConnected();
    info.isCompatible = isCompatible();

    info.hasDisk = hasDisk();
    info.hasModifiedDisk = hasModifiedDisk();
    info.hasUnmodifiedDisk = hasUnmodifiedDisk();
    info.hasProtectedDisk = hasProtectedDisk();
    info.hasUnprotectedDisk = hasUnprotectedDisk();
    info.snapshotable = snapshotable();

    info.partitions = numPartitions();

    // Flags
    info.writeProtected = getFlag(DiskFlags::PROTECTED);
    info.modified = getFlag(DiskFlags::MODIFIED);

    // State
    info.state = state;
    info.head = head;

    return info;
}

void
HardDrive::_dump(Category category, std::ostream &os) const
{
    using namespace utl;

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

    /*
    if (category == Category::Volumes) {

        for (isize i = 0; i < isize(ptable.size()); i++) {

            auto dev = make_unique<Device>(getGeometry());
            auto fs = FileSystemFactory::fromHardDrive(*dev, *this, i);
            i == 0 ? fs->dumpInfo(os) : fs->dumpState(os);
        }

        for (isize i = 0; i < isize(ptable.size()); i++) {

            os << std::endl;
            os << tab("Partition");
            os << dec(i) << std::endl;
            auto dev = make_unique<Device>(getGeometry());
            auto fs = FileSystemFactory::fromHardDrive(*dev, *this, i);
            fs->dumpProps(os);
        }
    }
    */

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

void
HardDrive::read(u8 *dst, isize offset, isize count) const
{
    assert(offset + count <= size());
    memcpy((void *)dst, (const void *)image->byteView(offset, count).data(), count);
}

void
HardDrive::write(const u8 *src, isize offset, isize count)
{
    assert(offset + count <= size());

    // The run-ahead instance leaves the shared disk alone (see 'image')
    if (isRunAheadInstance()) return;

    memcpy((void *)image->mutableByteView(offset, count).data(), (const void *)src, count);

    /* Have the run-ahead instance recreated, so that it sees the change.
     * Writes through this interface come from the host (formatting, importing
     * a folder), not from the emulated machine, so no idle event follows that
     * could take care of it (see serviceHdrEvent).
     */
    emulator.markAsDirty();

    /* Bring the file up to date. In ON_IDLE mode this is left to whoever
     * started the operation: writing after each block would rewrite the file
     * thousands of times while a file system is being built.
     */
    if (config.writeThrough == WriteThroughMode::ALWAYS) persist();
}

bool
HardDrive::isConnected() const
{
    return amiga.hdcon[objid]->getOption(Opt::HDC_CONNECT);
}

isize
HardDrive::mbLimit() const
{
    return isize(amiga.hdcon[objid]->getOption(Opt::HDC_MB_LIMIT));
}

isize
HardDrive::memLimit() const
{
    return isize(amiga.hdcon[objid]->getOption(Opt::HDC_MEM_LIMIT));
}

bool
HardDrive::hasDisk() const
{
    return image != nullptr;
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

void
HardDrive::format(amiga::FSFormat fsType, FSName name)
{
    using amiga::FSFormat;

    if CONSTEXPR (LOG_HDR != LOG_OFF) {

        logmsg(LOG_HDR, "Formatting hard drive\n");
        logmsg(LOG_HDR, "    File system : %s\n", amiga::FSFormatEnum::key(fsType));
        logmsg(LOG_HDR, "           Name : %s\n", name.c_str());
    }

    // Only proceed if a disk is present
    if (!image) return;

    if (fsType != FSFormat::NODOS) {

        /* Create a file system on top of this drive
         *
         * This used to detour via a full in-memory copy of the drive (an
         * HDF). There is nothing the copy provided: Volume only needs a block
         * device, and the drive is one.
         */
        auto vol = Volume(*this);
        auto fs = FileSystem(vol);

        // Format the file system and name it
        fs.format(fsType);
        fs.setName(name);

        // Write back all changes
        fs.flush();

        // Describe the drive by the created file system (the disk holds it already)
        describe(fs);

        // Bring the file up to date (see write)
        if (config.writeThrough != WriteThroughMode::NEVER) persist();
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
    geometry.checkCompatibility(mbLimit());

    if (this->geometry.numBytes() == geometry.numBytes()) {

        this->geometry = geometry;

    } else {

        throw DeviceError(DeviceError::HDR_UNMATCHED_GEOMETRY);
    }
}

i8
HardDrive::read(i64 offset, i64 length, u32 addr)
{
    logmsg(LOG_HDR, "read(%lld, %lld, %u)\n", (long long)offset, (long long)length, addr);

    // Check arguments
    auto error = verify(offset, length, addr);

    if (!error) {

        state = HardDriveState::READING;

        // Move the drive head to the specified location
        moveHead(offset / geometry.bsize);

        // Perform the read operation
        mem.patch(addr, image->byteView(offset, length).data(), length);

        // Inform the GUI
        msgQueue.put(Msg::HDR_READ);

        // Go back to IDLE state after some time
        scheduleIdleEvent();
    }

    return error;
}

i8
HardDrive::write(i64 offset, i64 length, u32 addr)
{
    logmsg(LOG_HDR, "write(%lld, %lld, %u)\n", (long long)offset, (long long)length, addr);

    // Check arguments
    auto error = verify(offset, length, addr);

    if (!error) {

        state = HardDriveState::WRITING;

        // Move the drive head to the specified location
        moveHead(offset / geometry.bsize);

        if (getFlag(DiskFlags::PROTECTED)) {

            /* Say so, rather than dropping the data and reporting success.
             * A file system told that its write went through, when it did
             * not, writes on top of that lie.
             */
            logmsg(LOG_HDR, "Disk is write protected\n");
            error = TDERR_WRITEPROT;

        } else {

            /* Only the main instance writes. The run-ahead instance leaves
             * the shared disk alone; the main instance will perform this very
             * write once it gets here (see 'image').
             */
            if (!isRunAheadInstance()) {

                // Perform the write operation
                auto bytes = image->mutableByteView(offset, length);
                mem.spypeek <Accessor::CPU> (addr, length, bytes.data());

                /* Let the idle handler deal with the change: it has the
                 * run-ahead instance recreated and, in ON_IDLE mode, brings
                 * the file up to date. Waiting for the end of the access
                 * keeps a long transfer from recreating the run-ahead
                 * instance, or rewriting the file, block by block.
                 */
                pending = true;

                if (config.writeThrough == WriteThroughMode::ALWAYS) persist();
            }

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

        auto offset = i64(seg) * geometry.bsize + 20;

        assert(offset >= 0);
        assert(offset + bytesPerBlock <= size());

        memcpy(driver.ptr + bytesRead, image->byteView(offset, bytesPerBlock).data(), bytesPerBlock);
        bytesRead += bytesPerBlock;
    }
}

i8
HardDrive::verify(i64 offset, i64 length, u32 addr)
{
    // A drive can lose its disk when a snapshot is restored (see _didLoad)
    if (!image) {

        logmsg(LOG_HDR, "No disk in drive\n");
        return TDERR_DISKCHANGED;
    }

    if (length % 512) {

        logmsg(LOG_HDR, "Length must be a multiple of 512 bytes");
        return IOERR_BADLENGTH;
    }

    if (offset % 512) {

        logmsg(LOG_HDR, "Offset is not aligned");
        return IOERR_BADADDRESS;
    }

    if (offset + length > geometry.numBytes()) {

        logmsg(LOG_HDR, "Invalid block location");
        return IOERR_BADADDRESS;
    }

    /* The last byte of the buffer, not the one behind it: a transfer that
     * ends exactly at the end of the last Ram bank is perfectly fine, and
     * checking one byte too far would refuse it.
     */
    auto last = u32(addr + std::max(i64(1), length) - 1);

    if (!mem.inRam(addr) || !mem.inRam(last)) {

        logmsg(LOG_HDR, "Invalid RAM location");
        return IOERR_BADADDRESS;
    }

    return 0;
}

void
HardDrive::moveHead(i64 lba)
{
    isize c = isize(lba / (geometry.heads * geometry.sectors));
    isize h = isize((lba / geometry.sectors) % geometry.heads);
    isize s = isize(lba % geometry.sectors);

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
HardDrive::importFolder(const fs::path &path)
{
    if (!fs::exists(path)) {

        throw IOError(IOError::FILE_NOT_FOUND, path);
    }

    if (fs::is_directory(path)) {

        logmsg(LOG_HDR, "Importing directory...\n");

        // Retrieve some information about the first partition
        auto traits = getPartitionTraits(0);

        // Create a file system on top of the drive
        auto vol = Volume(*this);
        auto fs = FileSystem(vol);

        // Import all files
        fs.importer.import(fs.root(), path, true, true);

        // Write back
        fs.flush();


        // Describe the drive by the file system (the disk holds it already)
        describe(fs);

        // Bring the file up to date (see write)
        if (config.writeThrough != WriteThroughMode::NEVER) persist();
    }
}

void
HardDrive::persist()
{
    if (isRunAheadInstance() || !fileBacked()) return;

    try {

        image->save();

    } catch (std::exception &err) {

        logmsg(LOG_HDR, "Cannot write to %s: %s\n", image->path.string().c_str(), err.what());
    }
}

void
HardDrive::saveAs(const fs::path &path)
{
    if (!image || path.empty()) return;

    image->saveAs(path);
}

void
HardDrive::writeToFile(const fs::path &path)
{
    // Write straight from the image, in the format the file name asks for
    if (image && !path.empty()) image->writeToFile(path, 0, size());
}

std::unique_ptr<HardDiskImage>
HardDrive::exportDisk(ImageFormat fmt) const
{
    switch (fmt) {

        case ImageFormat::HDF: return Codec::makeHDF(*this);
        // case ImageFormat::HDZ: return Codec::makeHDZ(*this);

        default:
            throw IOError(IOError::FILE_TYPE_UNSUPPORTED);
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

    // Deal with what has been written since the drive was last idle
    if (pending && !isRunAheadInstance()) {

        pending = false;

        // Have the run-ahead instance recreated, so that it sees the changes
        emulator.markAsDirty();

        // Update the file if requested
        if (config.writeThrough == WriteThroughMode::ON_IDLE) persist();
    }

    msgQueue.put(Msg::HDR_IDLE, objid);
}

template void HardDrive::serviceHdrEvent <SLOT_HD0> ();
template void HardDrive::serviceHdrEvent <SLOT_HD1> ();
template void HardDrive::serviceHdrEvent <SLOT_HD2> ();
template void HardDrive::serviceHdrEvent <SLOT_HD3> ();

}
