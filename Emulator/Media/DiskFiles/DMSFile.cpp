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
unsigned short extractDMS(FILE *fi, FILE *fo);
}

bool
DMSFile::isCompatible(const string &path)
{
    auto suffix = util::extractSuffix(path);
    return suffix == "dms" || suffix == "DMS";
}

bool
DMSFile::isCompatible(std::istream &stream)
{                                                                                            
    return util::matchingStreamHeader(stream, "DMS!");
}

isize
DMSFile::readFromStream(std::istream &stream)
{
    FILE *fpi, *fpo;
    char *pi, *po;
    size_t si, so;
    
    isize result = AmigaFile::readFromStream(stream);
        
    /* We use a third-party tool called xdms to convert the DMS into an ADF.
     * Originally, xdms is a command line utility designed to work on files.
     * To ease the integration of this tool, we utilize memory streams for
     * passing data in and out.
     */

    // Setup input stream
    fpi = open_memstream(&pi, &si);
    for (isize i = 0; i < size; i++) putc(data[i], fpi);
    fclose(fpi);
    
    // Setup output stream
    fpi = fmemopen(pi, si, "r");
    fpo = open_memstream(&po, &so);
    
    // Extract the DMS
    extractDMS(fpi, fpo);
    fclose(fpi);
    fclose(fpo);
    
    // Create the ADF
    fpo = fmemopen(po, so, "r");
    adf = new ADFFile(fpo);
    fclose(fpo);
    
    if (!adf) throw VAError(ERROR_UNKNOWN);
    return result;
}
