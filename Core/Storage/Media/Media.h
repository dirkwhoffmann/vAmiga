// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MediaFile.h"
#include "Workspace.h"
#include "Snapshot.h"
#include "Script.h"
#include "RomFile.h"
#include "DiskImage.h"
#include "HDFFile.h"
#include "HDZFile.h"

/* All media files are organized in the class hierarchy displayed below.
 * MediaFile is a wrapper class which is referenced in the public API. Below
 * the public API, three abstract classes are involed: AnyFile, DiskImage,
 * and FloppyDiskImage. AnyFile provides basic functionalities for reading and
 * writing files, streams, and buffers. DiskImage provides an abstract interface
 * for accessing files that represent hard disks and floppy disks. FloppyDiskImage
 * is the base class of all floppy disk file formats.
 *
 *  -----------
 * | MediaFile |     Public API
 *  -----------
 *       |
 *   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *       |
 *  -----------
 * |  AnyFile  |      Internal classes
 *  -----------
 *       |
 *       |--------------------------------------------
 *       |       |            |            |          |
 *       |  -----------   ----------   --------   ---------
 *       | | Workspace | | Snapshot | | Script | | RomFile |
 *       |  -----------   ----------   --------   ---------
 *       |
 *  -----------
 * | DiskImage |
 *  -----------
 *       |
 *       |--------------------------------------------
 *       |                                            |
 *  -----------------                          ---------------
 * | FloppyDiskImage |                        | HardDiskImage |
 *  -----------------                          ---------------
 *       |                                            |
 *       |---------------------------                 |
 *       |           |               |                |
 *   ---------   ----------       ---------       ---------
 *  |   ADF   | |   EADF   | ... |   DMS   |     |   HDF   |
 *   ---------   ----------       ---------       ---------
 */
