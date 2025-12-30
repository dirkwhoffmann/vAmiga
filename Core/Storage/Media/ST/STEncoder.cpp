// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "STEncoder.h"
#include "STFactory.h"
#include "DiskEncoder.h"
#include "IMGEncoder.h"
#include "IMGFactory.h"

namespace vamiga {

/*
void
STEncoder::encode(const class STFile &source, FloppyDisk &target)
{
    auto img = IMGFactory::make(source.data.ptr, source.data.size);
    DiskEncoder::encode(*img, target);
}

void
STEncoder::decode(class STFile &target, const FloppyDisk &source)
{
    auto img = IMGFactory::make(target.data.ptr, target.data.size);
    DiskEncoder::decode(*img, source);
    target.data = img->data;
}
*/

};
