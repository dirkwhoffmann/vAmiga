// -----------------------------------------------------------------------------
// This file is part of RetroVault
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Images/AnyImage.h"

namespace retro::vault {

class LinearDevice;

/* An image that is a contiguous block of bytes.
 *
 * This is what almost every format in this library is: a floppy image, a hard
 * drive image, an executable. init() reads the file into memory, and
 * everything below -- sizing, copying, hashing, dumping, exporting -- is a
 * statement about those bytes.
 *
 * The bytes themselves are private. Everything, subclasses included, reaches
 * them through two views:
 *
 *   byteView()         read-only
 *   mutableByteView()  writable. Ask for it only when writing: storage that
 *                      loads lazily (see utl::BackedBuffer) takes the region
 *                      as modified.
 *
 * Keeping to these two is what allows the storage behind them to change.
 *
 * Formats that are not a buffer derive from AnyImage directly (see SVMFile)
 * and simply do not have these members.
 */
class BinaryImage : public AnyImage, public utl::Dumpable {

    // The raw data of this file
    utl::Buffer<u8> data;


    //
    // Initializing
    //

public:

    void init(isize len);
    void init(const u8 *buf, isize len);
    void init(const fs::path& p);

    /* Initializes the image with the contents of a device.
     *
     * The device is asked for its bytes rather than handing over a pointer to
     * them, so this works for any device, including ones that do not keep the
     * whole image in memory.
     */
    void init(const LinearDevice& device);

protected:

    /* Changing the size after loading.
     *
     * Some formats are not stored as the image itself: an .adz or .hdz file is
     * compressed, and a short ADF lacks cylinders. Subclasses fix that up in
     * didInitialize() with these two.
     */

    // Replaces the contents with their gunzipped form
    void gunzip();

    // Grows or shrinks the image. Added bytes are zero.
    void resize(isize newSize);


    //
    // Methods from Hashable
    //

public:

    u64 hash(HashAlgorithm algorithm) const override {
        return byteView(0, getSize()).hash(algorithm);
    }


    //
    // Methods from Dumpable
    //

public:

    Dumpable::DataProvider dataProvider() const override {
        return byteView(0, getSize()).dataProvider();
    }


    //
    // Querying meta information
    //

public:

    isize getSize() const { return data.size; }
    bool empty() const { return data.empty(); }


    //
    // Accessing data
    //

public:

    // Returns a view of bytes [offset, offset + len), which must lie within the image
    utl::ByteView byteView(isize offset, isize len) const;
    utl::MutableByteView mutableByteView(isize offset, isize len);

    // Copies the file contents into a buffer
    virtual void copy(u8 *dst, isize offset, isize len) const;
    virtual void copy(u8 *dst, isize offset = 0) const;


    //
    // Exporting
    //

public:

    // Update the image or a portion of the image on disk
    void save() override;
    void save(const utl::Range<isize> range);
    void save(const std::vector<utl::Range<isize>> ranges);

    // Create a new image file on disk and update it with the current contents
    void saveAs(const fs::path &path);

    virtual isize writeToStream(std::ostream &stream) const;
    virtual isize writeToFile(const fs::path &path) const;

    virtual isize writeToStream(std::ostream &stream, isize offset, isize len) const;
    virtual isize writeToFile(const fs::path &path, isize offset, isize len) const;

private:

    // Called at the end of init()
    virtual void didInitialize() {};
};

}
