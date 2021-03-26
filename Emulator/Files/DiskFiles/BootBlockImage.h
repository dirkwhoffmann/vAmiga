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

#pragma once

#include "Aliases.h"

//
// Enumerations
//

enum_long(BB_TYPE)
{
    BB_STANDARD,
    BB_VIRUS,
    BB_CUSTOM,
    
    BB_COUNT
};
typedef BB_TYPE BootBlockType;

enum_long(BB_ID)
{
    BB_NONE,
    BB_AMIGADOS_13,
    BB_AMIGADOS_20,
    BB_SCA,
    BB_BYTE_BANDIT
};
typedef BB_ID BootBlockId;
