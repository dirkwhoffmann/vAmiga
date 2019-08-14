// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _VATYPES_H
#define _VATYPES_H

#include "va_aliases.h"

#include "AmigaTypes.h"
#include "MessageQueueTypes.h"
#include "EventHandlerTypes.h"
#include "MemoryTypes.h"
#include "CPUTypes.h"
#include "CIATypes.h"
#include "AgnusTypes.h"
#include "DeniseTypes.h"
#include "PaulaTypes.h"
#include "PortTypes.h"
#include "DriveTypes.h"
#include "DiskTypes.h"
#include "FileTypes.h"

// 2 bit binary value
typedef uint8_t uint2_t;
inline bool is_uint2_t(uint2_t value) { return value <= 0x03; }

// 24 bit binary value
typedef uint32_t uint24_t;
inline bool is_uint24_t(uint24_t value) { return value <= 0xFFFFFF; }

#endif
