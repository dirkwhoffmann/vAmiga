// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FloppyDisk.h"

namespace vamiga {

class DiskEncoder {

    //
    // ADF
    //

public:

    // TODO: MOVE TO FloppyDisk
    static void encode(const class ADFFile &source, FloppyDisk &target);
    static void decode(class ADFFile &target, const FloppyDisk &source);


    //
    // IMG
    //

public:

    // TODO: MOVE TO FloppyDisk
    static void encode(const class IMGFile &source, FloppyDisk &target);
    static void decode(class IMGFile &target, const FloppyDisk &source);


    //
    // ST
    //

public:

    // TODO: MOVE TO FloppyDisk
    static void encode(const class STFile &source, FloppyDisk &target);
    static void decode(class STFile &target, const FloppyDisk &source);
};

}
