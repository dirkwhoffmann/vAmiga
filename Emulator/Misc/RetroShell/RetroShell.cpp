// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RetroShell.h"
#include "Emulator.h"
#include "Parser.h"
#include <istream>
#include <sstream>
#include <string>

namespace vamiga {

RetroShell::RetroShell(Amiga& ref) : SubComponent(ref)
{
    subComponents = std::vector<CoreComponent *> {

        &commander,
        &debugger
    };
}

void
RetroShell::_initialize()
{

}

void
RetroShell::switchConsole() {

    if (inCommandShell()) {

        current = &debugger;
        emulator.trackOn(1);
        msgQueue.put(MSG_CONSOLE_DEBUGGER, true);

    } else {

        current = &commander;
        emulator.trackOff(1);
        msgQueue.put(MSG_CONSOLE_DEBUGGER, false);
    }
}


RetroShell &
RetroShell::operator<<(char value)
{
    *current << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(const string &value)
{
    *current << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(int value)
{
    *current << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(unsigned int value)
{
    *current << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(long value)
{
    *current << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(unsigned long value)
{
    *current << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(long long value)
{
    *current << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(unsigned long long value)
{
    *current << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(std::stringstream &stream)
{
    *current << stream;
    return *this;
}

const char *
RetroShell::text()
{
    return current->text();
}

isize
RetroShell::cursorRel()
{
    return current->cursorRel();
}

void
RetroShell::press(RetroShellKey key, bool shift)
{
    current->press(key, shift);
}

void
RetroShell::press(char c)
{
    current->press(c);
}

void
RetroShell::press(const string &s)
{
    current->press(s);
}

void
RetroShell::setStream(std::ostream &os)
{
    current->setStream(os);
}

void
RetroShell::exec()
{
    current->exec();
}

void
RetroShell::exec(const string &command)
{
    current->asyncExec(command);
}

void
RetroShell::execScript(std::stringstream &ss)
{
    current->asyncExecScript(ss);
}

void
RetroShell::execScript(const std::ifstream &fs)
{
    current->asyncExecScript(fs);
}

void
RetroShell::execScript(const string &contents)
{
    current->asyncExecScript(contents);
}

void
RetroShell::serviceEvent()
{
    emulator.put(Cmd(CMD_RSH_EXECUTE));
    agnus.cancel<SLOT_RSH>();
}

}
