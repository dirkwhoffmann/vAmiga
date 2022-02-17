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
#include "IOUtils.h"

HardDrive::HardDrive(Amiga& ref, isize n) : SubComponent(ref), nr(n)
{
    assert(usize(nr) < 4);
}

void
HardDrive::dealloc()
{
    // Wipe out disk data
    if (data) delete [] data;

    // Wipe out geometry information
    geometry = DiskGeometry();
}

const char *
HardDrive::getDescription() const
{
    assert(usize(nr) < 4);
    return nr == 0 ? "Dh0" : nr == 1 ? "Dh1" : nr == 2 ? "Dh2" : "Dh3";
}

void
HardDrive::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
}

void
HardDrive::_inspect() const
{
    {   SYNCHRONIZED
        /*
        info.attached = ???;
        info.cylinder = ???;
        */
    }
}

void
HardDrive::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
        
    if (category & dump::State) {
        
        os << tab("Nr");
        os << dec(nr) << std::endl;
    }
}

isize
HardDrive::_size()
{
    util::SerCounter counter;

    applyToPersistentItems(counter);
    applyToResetItems(counter);
    
    /*
    // Add the size of the boolean indicating whether a disk is inserted
    counter.count += sizeof(bool);

    if (hasDisk()) {

        // Add the disk type and disk state
        counter << disk->getDiameter() << disk->getDensity();
        disk->applyToPersistentItems(counter);
    }
    */
    
    return counter.count;
}

isize
HardDrive::_load(const u8 *buffer)
{
    util::SerReader reader(buffer);
    isize result;
    
    // Read own state
    applyToPersistentItems(reader);
    applyToResetItems(reader);

    /*
    // Check if the snapshot includes a disk
    bool diskInSnapshot; reader << diskInSnapshot;
    
    if (diskInSnapshot) {
        
        DiskDiameter type;
        DiskDensity density;
        reader << type << density;
        disk = std::make_unique<Disk>(reader, type, density);

    } else {
        
        disk = nullptr;
    }
    */
    
    result = (isize)(reader.ptr - buffer);
    trace(SNP_DEBUG, "Recreated from %ld bytes\n", result);
    return result;
}

isize
HardDrive::_save(u8 *buffer)
{
    util::SerWriter writer(buffer);
    isize result;
    
    // Write own state
    applyToPersistentItems(writer);
    applyToResetItems(writer);

    /*
    // Indicate whether this drive has a disk is inserted
    writer << hasDisk();

    if (hasDisk()) {

        // Write the disk type
        writer << disk->getDiameter() << disk->getDensity();

        // Write the disk's state
        disk->applyToPersistentItems(writer);
    }
    */
    
    result = (isize)(writer.ptr - buffer);
    trace(SNP_DEBUG, "Serialized to %ld bytes\n", result);
    return result;
}

void
HardDrive::attach(const DiskGeometry &geometry)
{
    // Throw an exception if the geometry is not supported
    checkCompatibility(geometry);

}

void
HardDrive::attach(const HDFFile &hdf)
{
    // Throw an exception if the HDF is not supported
    checkCompatibility(hdf);
}

void
HardDrive::checkCompatibility(const DiskGeometry &geometry)
{
    if (geometry.numBytes() > MAX_HDF_SIZE) {
        
        throw VAError(ERROR_HDR_TOO_LARGE);
    }
    if (geometry.bsize != 512) {
        
        throw VAError(ERROR_HDR_UNSUPPORTED_BSIZE);
    }
}

void
HardDrive::checkCompatibility(const HDFFile &hdf)
{
    // TODO
}
