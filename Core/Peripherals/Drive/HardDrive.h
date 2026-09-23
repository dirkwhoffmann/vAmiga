// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "HardDriveTypes.h"
#include "HdControllerTypes.h"
#include "FileSystems/Amiga/FSTypes.h"
#include "ImageTypes.h"
#include "FileSystems/Amiga/FSObjects.h"
#include "AgnusTypes.h"
#include "Drive.h"
#include "HardDiskImage.h"
#include "HDFFile.h"
#include "TrackDevice.h"
#include "utl/storage.h"
#include "utl/wrappers.h"
#include <memory>

namespace retro::vault::amiga { class FileSystem; }

namespace vamiga {

using retro::vault::HDFFile;

class HardDrive final : public Drive, public TrackDevice {

    friend class Codec;
    friend class retro::vault::HDFFile;
    friend class HdController;

    Descriptions descriptions = {
        {
            .type           = Class::HardDrive,
            .name           = "HardDrive0",
            .description    = "Hard Drive 0",
            .shell          = "hd0"
        },
        {
            .type           = Class::HardDrive,
            .name           = "HardDrive1",
            .description    = "Hard Drive 1",
            .shell          = "hd1"
        },
        {
            .type           = Class::HardDrive,
            .name           = "HardDrive2",
            .description    = "Hard Drive 2",
            .shell          = "hd2"
        },
        {
            .type           = Class::HardDrive,
            .name           = "HardDrive3",
            .description    = "Hard Drive 3",
            .shell          = "hd3"
        }
    };

    Options options = {

        Opt::HDR_TYPE,
        Opt::HDR_PAN,
        Opt::HDR_STEP_VOLUME,
        Opt::HDR_WRITE_THROUGH
    };

public:

    // Result of the latest inspection
    utl::Backed<HardDriveInfo> info;

private:

    // Current configuration
    HardDriveConfig config = {};

    // Product information
    string diskVendor;
    string diskProduct;
    string diskRevision;
    string controllerVendor;
    string controllerProduct;
    string controllerRevision;

    // Hard disk geometry
    GeometryDescriptor geometry;

    // Partition table
    std::vector <PartitionDescriptor> ptable;

    // Loadable file system drivers
    std::vector <DriverDescriptor> drivers;

    /* The disk (nullptr if there is none)
     *
     * An image, usually on top of a file, that the drive owns together with
     * its run-ahead counterpart. Cloning the drive into the run-ahead instance
     * copies the pointer, not the disk: both instances look at the same bytes.
     *
     * Only the main instance writes to it. The run-ahead instance runs ahead
     * of the main instance, which performs each of its writes later on, so
     * the run-ahead instance simply drops them. Whenever the main instance
     * writes, it has the run-ahead instance recreated, which keeps the two
     * from drifting apart.
     *
     * What this gives up: the run-ahead instance does not see its own writes.
     * A block it writes and reads back within the look-ahead window still
     * reads as it was, until the main instance has caught up.
     */
    std::shared_ptr<HDFFile> image;

public:

    /* The file this drive's image is loaded from, empty when it was built in
     * memory. Amiga::saveWorkspace() uses it to recognize a drive that is
     * already sitting on one of the workspace's own files.
     */
    fs::path imagePath() const { return image ? image->path : fs::path(); }

private:

    // Current position of the read/write head
    DriveHead head;

    // Current drive state
    HardDriveState state = HardDriveState::IDLE;

    // Disk state flags
    long flags = 0;

    /* Set when the disk was written since the drive last went idle
     *
     * The idle handler acts on it: it has the run-ahead instance recreated
     * and, in WriteThroughMode::ON_IDLE, brings the file up to date. Only the
     * main instance sets it; it is part of no snapshot, since it says
     * something about this run, not about the emulated machine.
     */
    bool pending = false;


    //
    // Initializing
    //

public:

    HardDrive(Amiga& ref, isize nr);
    ~HardDrive();

    HardDrive& operator= (const HardDrive& other);

    // Creates a hard drive with a certain geometry
    void init(const GeometryDescriptor &geometry);

    // Creates a hard drive with a certain capacity in bytes
    void init(i64 size);

    // Creates a hard drive with the contents of a file system
    void init(const amiga::FileSystem &fs);

    // Creates a hard drive on top of an HDF or HDZ image (the drive takes it over)
    void init(std::unique_ptr<HDFFile> hdf);

    /* Creates a hard drive with the contents of an HDF file
     *
     * FILE_BACKED keeps the disk on top of the file. MEMORY_BACKED loads it
     * into memory and lets go of the file, as loadIntoMemory() does, and
     * throws if the disk is larger than the controller keeps in memory. A
     * directory is imported into the current disk, whatever the mode.
     */
    void init(const fs::path &path, StorageMode mode = StorageMode::FILE_BACKED);

    const HardDriveTraits &getTraits() const {

        static HardDriveTraits traits;

        traits.nr = objid;

        traits.diskVendor = diskVendor.c_str();
        traits.diskProduct = diskProduct.c_str();
        traits.diskRevision = diskRevision.c_str();
        traits.controllerVendor = controllerVendor.c_str();
        traits.controllerProduct = controllerProduct.c_str();
        traits.controllerRevision = controllerRevision.c_str();

        traits.cylinders = geometry.cylinders;
        traits.heads = geometry.heads;
        traits.sectors = geometry.sectors;
        traits.bsize = geometry.bsize;

        traits.tracks = geometry.numTracks();
        traits.blocks = geometry.numBlocks();
        traits.bytes = geometry.numBytes();
        traits.upperCyl = geometry.upperCyl();
        traits.upperHead = geometry.upperHead();
        traits.upperTrack = geometry.upperTrack();

        return traits;
    }

    const PartitionTraits &getPartitionTraits(isize nr) const {

        using amiga::FSFormat;

        static PartitionTraits traits;

        auto descr = getPartitionDescriptor(nr);
        traits.nr = nr;
        traits.name = descr.name;
        traits.lowerCyl = descr.lowCyl;
        traits.upperCyl = descr.highCyl;

        switch (descr.dosType) {

            case 0x444F5300: traits.fsType = FSFormat::OFS; break;
            case 0x444F5301: traits.fsType = FSFormat::FFS; break;
            case 0x444F5302: traits.fsType = FSFormat::OFS_INTL; break;
            case 0x444F5303: traits.fsType = FSFormat::FFS_INTL; break;
            case 0x444F5304: traits.fsType = FSFormat::OFS_DC; break;
            case 0x444F5305: traits.fsType = FSFormat::FFS_DC; break;
            case 0x444F5306: traits.fsType = FSFormat::OFS_LNFS; break;
            case 0x444F5307: traits.fsType = FSFormat::FFS_LNFS; break;
            default:         traits.fsType = FSFormat::NODOS; break;
        }

        return traits;
    }

private:

    // Restors the initial state
    void init();

    // Describes a drive of the given geometry, without providing a disk yet
    void setup(const GeometryDescriptor &geometry);

    // Throws if a disk of the given size must not be held in memory
    void checkMemoryLimit(i64 bytes) const;

    // Describes the drive after a file system was built on it, keeping the disk
    void describe(const amiga::FileSystem &fs);


    //
    // Methods from CoreObject
    //

private:

    void _dump(Category category, std::ostream &os) const override;


    //
    // Methods from CoreComponent
    //

private:

    void _initialize() override;

    template <class T>
    void serialize(T& worker)
    {
        if (isSoftResetter(worker)) return;

        worker

        << head.cylinder
        << head.head
        << head.offset
        << state;

        if (isResetter(worker)) return;

        worker

        << config.type
        << config.pan
        << config.stepVolume
        << diskVendor
        << diskProduct
        << diskRevision
        << controllerVendor
        << controllerProduct
        << controllerRevision
        << geometry
        << ptable
        << drivers;

        serializeDisk(worker);

        worker

        << flags;

    } SERIALIZERS(serialize);

    /* Serializes the disk
     *
     * A disk that lives in a file is stored as the path of that file and
     * nothing else. Restoring a snapshot opens the file again, as it is then:
     * changes that were never written to the file are not part of the
     * snapshot. If the file cannot be opened anymore, or no longer covers the
     * geometry, the drive is left without a disk (see _didLoad()).
     *
     * A disk built in memory has no file to go back to and is stored in full.
     *
     * Layout: an i64 comes first. -1 announces a path, stored as an i64 length
     * and its characters (not as a serialized string, which is limited to 255
     * characters). Any other value is the size of the disk, followed by its
     * bytes. That is the format of a utl::Buffer<u8>, in which the drive used
     * to store every disk, so older snapshots still load.
     */
    void serializeDisk(SerCounter &worker);
    void serializeDisk(SerChecker &worker);
    void serializeDisk(SerReader &worker);
    void serializeDisk(SerWriter &worker);
    void serializeDisk(SerResetter &worker) { }

    // Returns true if the disk lives in a file
    bool fileBacked() const { return image && image->getStorageMode() == StorageMode::FILE_BACKED; }

    void _didReset(bool hard) override;
    void _didLoad() override;

public:

    const Descriptions &getDescriptions() const override { return descriptions; }


    //
    // Methods from Drive
    //

public:

    string getDiskVendor() const override { return diskVendor; }
    string getDiskProduct() const override { return diskProduct; }
    string getDiskRevision() const override { return diskRevision; }
    string getControllerVendor() const override { return controllerVendor; }
    string getControllerProduct() const override { return controllerProduct; }
    string getControllerRevision() const override { return controllerRevision; }

    bool isConnected() const override;

    /* Largest capacity this drive may have, in MB (0 = no limit)
     *
     * Read from the controller the drive hangs off, since that is what
     * imposes it (see Opt::HDC_MB_LIMIT).
     */
    isize mbLimit() const;

    /* Largest capacity a disk in memory may have, in MB (0 = no limit)
     *
     * The second limit, and usually the stricter one: a disk that does not
     * live in a file is part of every snapshot (see Opt::HDC_MEM_LIMIT).
     */
    isize memLimit() const;

    CylNr currentCyl() const override { return head.cylinder; }
    HeadNr currentHead() const override { return head.head; }
    isize currentOffset() const override { return head.offset; }

    StorageMode getStorageMode() const override {
        return fileBacked() ? StorageMode::FILE_BACKED : StorageMode::MEMORY_BACKED;
    }
    fs::path getPath() const override { return fileBacked() ? image->path : fs::path(); }
    bool needsPersisting() const override { return fileBacked() && image->modified(); }

    /* Writes all changes back to the file the disk lives in
     *
     * Only what has changed is written. A disk built in memory has no file to
     * go back to; for such a disk, and for the run-ahead instance, the call
     * does nothing. A failure is logged and otherwise ignored: the changes
     * stay in memory and are written again with the next call.
     *
     * Opt::HDR_WRITE_THROUGH has this called on its own (see WriteThroughMode).
     */
    void persist() override;

    bool getFlag(DiskFlags mask) const override;
    void setFlag(DiskFlags mask, bool value) override;

    bool hasDisk() const override;
    bool hasModifiedDisk() const override;
    bool hasProtectedDisk() const override;
    void setModificationFlag(bool value) override;
    void setProtectionFlag(bool value) override;


    //
    // Methods from Configurable
    //

public:

    const HardDriveConfig &getConfig() const { return config; }
    const Options &getOptions() const override { return options; }
    i64 getOption(Opt option) const override;
    void checkOption(Opt opt, i64 value) override;
    void setOption(Opt option, i64 value) override;

private:

    void disconnect();


    //
    // Methods from LinearDevice
    //

public:

    // The capacity given by the geometry (the image may be larger)
    isize size() const override { return image ? geometry.numBytes() : 0; }
    void read(u8 *dst, isize offset, isize count) const override;
    void write(const u8 *src, isize offset, isize count) override;


    //
    // Methods from BlockDevice
    //

public:

    isize bsize() const override { return geometry.bsize; }


    //
    // Methods from TrackDevice
    //

    isize numCyls() const override { return geometry.cylinders; }
    isize numHeads() const override { return geometry.heads; }
    isize numSectors(isize t) const override { return geometry.sectors; }


    //
    // Analyzing
    //

public:

    // Returns information about the disk
    HardDriveInfo cacheInfo() const;

    // Returns information about a specific partition
    const PartitionDescriptor &getPartitionDescriptor(isize nr) const;

    // Returns the disk geometry
    const GeometryDescriptor &getGeometry() const { return geometry; }

    // Returns the number of partitions
    isize numPartitions() const { return isize(ptable.size()); }

    // Returns the number of loadable file system drivers
    isize numDrivers() const { return isize(drivers.size()); }

    // Returns the current drive state
    HardDriveState getState() const { return state; }

    /* Loads the disk into memory and lets go of the file it lived in.
     *
     * The disk keeps its contents, unsaved changes included, and becomes a
     * disk like one that was created from scratch: part of every snapshot,
     * and gone when the emulator is switched off without one. Throws if the
     * disk is larger than the controller keeps in memory (see
     * memLimit). A disk that is in memory already, and a drive without a
     * disk, are left alone.
     */
    void loadIntoMemory();

    /* Writes the disk to a file and continues on top of it.
     *
     * From then on the disk lives in that file, which holds it in full --
     * unlike writeToFile(), which exports a copy and leaves the drive where
     * it is. The file is written before the drive switches over, so a
     * failure changes nothing.
     */
    void saveAs(const fs::path &path);

    // Gets or sets the 'modification' flag
    bool isModified() const { return flags & long(DiskFlags::MODIFIED); }
    void setModified(bool value) { value ? flags |= long(DiskFlags::MODIFIED) : flags &= ~long(DiskFlags::MODIFIED); }

    // Returns the current controller state
    HdcState getHdcState() const;

    // Checks whether the drive will work with the currently installed Rom
    bool isCompatible() const;

    // Checks whether the drive is marked as bootable
    bool isBootable();

    //
    // Formatting
    //

    // Formats the disk
    void format(amiga::FSFormat fs, amiga::FSName name);

    // Change the drive geometry
    void changeGeometry(isize c, isize h, isize s, isize b = 512);
    void changeGeometry(const GeometryDescriptor &geometry);


    //
    // Reading and writing
    //

public:

    // Reads a data block from the hard drive and copies it into RAM
    i8 read(i64 offset, i64 length, u32 addr);

    // Reads a data block from RAM and writes it onto the hard drive
    i8 write(i64 offset, i64 length, u32 addr);

    // Reads a loadable file system
    void readDriver(isize nr, utl::Buffer<u8> &driver);

private:

    // Checks the given argument list for consistency
    i8 verify(i64 offset, i64 length, u32 addr);

    // Moves the drive head to the specified block
    void moveHead(i64 lba);
    void moveHead(isize c, isize h, isize s);



    //
    // Importing and exporting
    //

public:

    // Imports files from a folder (deletes existing files)
    void importFolder(const fs::path &path);

    // Exports the disk to a file
    void writeToFile(const fs::path &path);

    // Exports the disk to a HardDiskImage
    std::unique_ptr<HardDiskImage> exportDisk(ImageFormat fmt) const;


    //
    // Scheduling and serving events
    //

public:

    // Schedules an event to revert to idle state
    void scheduleIdleEvent();

    // Services a hard drive event
    template <EventSlot s> void serviceHdrEvent();

};

}
