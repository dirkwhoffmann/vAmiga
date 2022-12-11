// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RetroShell.h"
#include "Amiga.h"
#include "BootBlockImage.h"
#include "FSTypes.h"
#include "IOUtils.h"
#include "Parser.h"
#include <fstream>
#include <sstream>

namespace vamiga {

template <> void
RetroShell::exec <Token::memdump> (Arguments& argv, long param)
{
    std::stringstream ss;

    mem.memDump<ACCESSOR_CPU>(ss, u32(util::parseNum(argv.front())));

    string line;
    while(std::getline(ss, line)) *this << line << '\n';
}

template <> void
RetroShell::exec <Token::amiga, Token::state> (Arguments &argv, long param)
{
    dump(amiga, Category::State);
}

template <> void
RetroShell::exec <Token::cpu, Token::state> (Arguments &argv, long param)
{
    dump(cpu, Category::State);
}

template <> void
RetroShell::exec <Token::cia, Token::state> (Arguments &argv, long param)
{
    dump(ciaa, Category::State);
    dump(ciab, Category::State);
}

template <> void
RetroShell::exec <Token::agnus, Token::state> (Arguments &argv, long param)
{
    dump(agnus, Category::State);
}

template <> void
RetroShell::exec <Token::paula, Token::state> (Arguments &argv, long param)
{
    dump(paula, Category::State);
}

template <> void
RetroShell::exec <Token::denise, Token::state> (Arguments &argv, long param)
{
    dump(denise, Category::State);
}

}
