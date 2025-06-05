// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "HttpServer.h"
#include "httplib.h"

namespace vamiga {

void
HttpServer::_dump(Category category, std::ostream &os) const
{
    using namespace util;

    if (category == Category::State) {

        RemoteServer::_dump(category, os);
        // os << tab("...");
        // os << dec(...) << std::endl;

    } else {

        RemoteServer::_dump(category, os);
    }
}

void
HttpServer::disconnect()
{
    debug(SRV_DEBUG, "Disconnecting...\n");
    if (srv) srv->stop();
}

}
