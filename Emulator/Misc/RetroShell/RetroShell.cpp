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

        &commander
    };
}

void
RetroShell::_initialize()
{

}

RetroShell &
RetroShell::operator<<(char value)
{
    commander << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(const string &value)
{
    commander << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(int value)
{
    commander << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(unsigned int value)
{
    commander << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(long value)
{
    commander << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(unsigned long value)
{
    commander << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(long long value)
{
    commander << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(unsigned long long value)
{
    commander << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(std::stringstream &stream)
{
    commander << stream;
    return *this;
}

const char *
RetroShell::text()
{
    return commander.text();
}

isize
RetroShell::cursorRel()
{
    return commander.cursorRel();
}

void
RetroShell::press(RetroShellKey key, bool shift)
{
    commander.press(key, shift);
}

void
RetroShell::press(char c)
{
    commander.press(c);
}

void
RetroShell::press(const string &s)
{
    commander.press(s);
}

void
RetroShell::setStream(std::ostream &os)
{
    commander.setStream(os);
}

void
RetroShell::exec()
{
    commander.exec();
}

void
RetroShell::exec(const string &command)
{
    commander.asyncExec(command);
}

void
RetroShell::execScript(std::stringstream &ss)
{
    commander.asyncExecScript(ss);
}

void
RetroShell::execScript(const std::ifstream &fs)
{
    commander.asyncExecScript(fs);
}

void
RetroShell::execScript(const string &contents)
{
    commander.asyncExecScript(contents);
}

void 
RetroShell::serviceEvent()
{
    commander.serviceEvent();
}
}
