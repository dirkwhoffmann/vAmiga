// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------
// THIS FILE MUST CONFORM TO ANSI-C TO BE COMPATIBLE WITH SWIFT
// -----------------------------------------------------------------------------

/* This file defines all constants and data types that are exposed to the GUI.
 * All definitions comply to standard ANSI-C to make the file accessible by
 * Swift. Note that the Swift GUI does not interact directly with any of the
 * public API methods of the emulator. Since Swift cannot deal with C++ code
 * directly yet, all API accesses are routed through the proxy layer written in
 * Objective-C.
 */

#pragma once

#include "Commons.h"
#include "AgnusPublicTypes.h"
#include "AmigaPublicTypes.h"
#include "CPUPublicTypes.h"
#include "CIAPublicTypes.h"
#include "DenisePublicTypes.h"
#include "DiskPublicTypes.h"
#include "DmaDebuggerPublicTypes.h"
#include "DrivePublicTypes.h"
#include "EventHandlerPublicTypes.h"
#include "FilePublicTypes.h"
#include "FSPublicTypes.h"
#include "KeyboardPublicTypes.h"
#include "MemoryPublicTypes.h"
#include "MsgQueuePublicTypes.h"
#include "PaulaPublicTypes.h"
#include "PortPublicTypes.h"
#include "RTCPublicTypes.h"
