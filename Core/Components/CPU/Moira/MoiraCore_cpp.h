// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

/* Body of the per-core translation units.
 *
 * Building a jump table is the only place where the instruction handler
 * templates are instantiated, since registering a handler takes its address.
 * Instantiating all three cores in a single translation unit produces over
 * 10,000 functions, which pushes the compiler's peak memory consumption beyond
 * what some build environments provide. This file is therefore compiled once
 * per core, with MOIRA_CORE set to the core to instantiate.
 *
 * See MoiraCore68000.cpp, MoiraCore68010.cpp and MoiraCore68020.cpp.
 */

#ifndef MOIRA_CORE
#error "MOIRA_CORE must be defined before including this file"
#endif

#include "vaconfig.h"
#include "MoiraConfig.h"
#include "Moira.h"
#include "MoiraMacros.h"

#include <cassert>
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <bit>
#include <vector>
#include <stdexcept>

namespace vamiga::moira {

using namespace Flag;

#include "MoiraInit_cpp.h"
#include "MoiraALU_cpp.h"
#include "MoiraDataflow_cpp.h"
#include "MoiraExceptions_cpp.h"
#include "MoiraExec_cpp.h"

/* Note: StrWriter_cpp.h and MoiraDasm_cpp.h are deliberately not included
 * here. The disassembler is not templated, so it is compiled once, in
 * Moira.cpp. Registering a disassembler handler only takes its address,
 * for which the declaration in MoiraInit.h is enough.
 */

template void Moira::createJumpTable<MOIRA_CORE>(Model, bool);

}
