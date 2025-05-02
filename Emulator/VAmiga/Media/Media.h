// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Workspace.h"
#include "Snapshot.h"
#include "Script.h"
#include "RomFiles/RomFile.h"
#include "DiskFiles/DiskFile.h"
#include "DiskFiles/ADFFile.h"
#include "DiskFiles/ADZFile.h"
#include "DiskFiles/EADFFile.h"
#include "DiskFiles/HDFFile.h"
#include "DiskFiles/HDZFile.h"
#include "DiskFiles/FloppyFile.h"
#include "DiskFiles/IMGFile.h"
#include "DiskFiles/DMSFile.h"
#include "DiskFiles/EXEFile.h"
#include "DiskFiles/STFile.h"
#include "DiskFiles/Folder.h"

/* All media files are organized in the class hierarchy displayed below.
 * MediaFile is a wrapper class which is referenced in the public API. Below
 * the public API, three abstract classes are involed: AnyFile, DiskFile,
 * and FloppyFile. AnyFile provides basic functionalities for reading and
 * writing files, streams, and buffers. DiskFile provides an abstract interface
 * for accessing files that represent hard disks and floppy disks. FloppyFile
 * is the base class of all floppy disk file formats.
 *
 *  ------------
 * | MediaFile  |     Public API
 *  ------------
 *         |
 *   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *         |
 *  ------------
 * | AnyFile  |     Internal classes
 *  ------------
 *       |
 *       |--------------------------------------------
 *       |       |            |            |          |
 *       |  -----------   ----------   --------   ---------
 *       | | Workspace | | Snapshot | | Script | | RomFile |
 *       |  -----------   ----------   --------   ---------
 *       |
 *  ------------
 * |  DiskFile  |
 *  ------------
 *       |
 *       |------
 *       |      |
 *       |  ---------
 *       | | HDFFile |
 *       |  ---------
 *       |
 *  ------------
 * | FloppyFile |
 *  ------------
 *       |
 *       |-----------------------------------------------------------
 *       |           |            |           |           |          |
 *   ---------   ----------   ---------   ---------   ---------   --------
 *  | ADFFile | | EADFFile | | IMGFile | | DMSFile | | EXEFile | | Folder |
 *   ---------   ----------   ---------   ---------   ---------   --------
 */
