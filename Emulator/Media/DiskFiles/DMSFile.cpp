// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DMSFile.h"
#include "AmigaFile.h"

extern "C" {
unsigned short extractDMS(const unsigned char *in, size_t inSize,
                          unsigned char **out, size_t *outSize, int verbose);
}

DMSFile::~DMSFile()
{
    if (adf) delete adf;
}

bool
DMSFile::isCompatible(const string &path)
{
    auto suffix = util::uppercased(util::extractSuffix(path));
    return suffix == "DMS";
}

bool
DMSFile::isCompatible(std::istream &stream)
{                                                                                            
    return util::matchingStreamHeader(stream, "DMS!");
}

void
DMSFile::finalizeRead()
{
    assert(adf == nullptr);
    
    u8* adfData;
    size_t adfSize;
    
    if (extractDMS(data, (size_t)size, &adfData, &adfSize, DMS_DEBUG) == 0) {

        if constexpr (!FORCE_DMS_CANT_CREATE) {
            adf = new ADFFile(adfData, (isize)adfSize);
        }
    }
    
    free(adfData);
    
    if (!adf) throw VAError(ERROR_DMS_CANT_CREATE);
}
