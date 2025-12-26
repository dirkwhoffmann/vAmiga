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
#include "RSError.h"
#include "Emulator.h"
#include "MediaFile.h"
#include "utl/io.h"
#include <istream>
#include <sstream>
#include <string>

namespace vamiga {

RetroShell::RetroShell(Amiga& ref) : SubComponent(ref)
{
    subComponents = std::vector<CoreComponent *> {
        
        &commander,
        &debugger,
        &navigator
    };

    info.bind([this] { return cacheInfo(); } );
}

void
RetroShell::_initialize()
{

}

RetroShellInfo
RetroShell::cacheInfo() const
{
    RetroShellInfo info;

    info.console = current->objid;
    info.cursorRel = current->cursorRel();

    return info;
}

void
RetroShell::enterConsole(isize nr)
{
    Console *newConsole = nullptr;
    
    switch (nr) {
            
        case 0: newConsole = &commander; break;
        case 1: newConsole = &debugger; break;
        case 2: newConsole = &navigator; break;
            
        default:
            fatalError;
    }

    // Switch to the new console
    if (current) current->didDeactivate();
    current = newConsole;
    current->didActivate();
    
    // Inform the GUI about the change
    msgQueue.put(Msg::RSH_SWITCH, nr);
}

void
RetroShell::asyncExec(const string &command, bool append)
{
    asyncExec(InputLine { .type = InputLine::Source::USER, .input = command });
}

void
RetroShell::asyncExec(const InputLine &command, bool append)
{
    // Feed the command into the command queue
    if (append) {
        commands.push_back(command);
    } else {
        commands.insert(commands.begin(), command);
    }
    
    // Process the command queue in the next update cycle
    emulator.put(Command(Cmd::RSH_EXECUTE));
}

void
RetroShell::asyncExecScript(const fs::path &path)
{
    auto stream = std::ifstream(path);
    if (!stream.is_open()) throw IOError(IOError::FILE_NOT_FOUND, path);
    asyncExecScript(stream);
}

void
RetroShell::asyncExecScript(std::stringstream &ss)
{
    {   SYNCHRONIZED

        std::string line;
        isize nr = 1;

        while (std::getline(ss, line)) {

            commands.push_back(InputLine {

                .id    = nr++,
                .type  = InputLine::Source::SCRIPT,
                .input = line
            });
        }
    
        emulator.put(Command(Cmd::RSH_EXECUTE));
    }
}

void
RetroShell::asyncExecScript(const std::ifstream &fs)
{
    std::stringstream ss;
    ss << fs.rdbuf();
    asyncExecScript(ss);
}

void
RetroShell::asyncExecScript(const string &contents)
{
    std::stringstream ss;
    ss << contents;
    asyncExecScript(ss);
}

void
RetroShell::abortScript()
{
    {   SYNCHRONIZED
        
        if (!commands.empty()) {
            
            commands.clear();
            agnus.cancel<SLOT_RSH>();
        }
    }
}

void
RetroShell::exec()
{
    {   SYNCHRONIZED

        // Only proceed if there is anything to process
        if (commands.empty()) return;

        try {

            while (!commands.empty()) {

                InputLine cmd = commands.front();
                commands.erase(commands.begin());
                exec(cmd);
            }

        } catch (ScriptInterruption &) {

            msgQueue.put(Msg::RSH_WAIT);

        } catch (...) {

            // Remove all remaining commands
            commands = { };

            msgQueue.put(Msg::RSH_ERROR);
        }

        // Print prompt
        if (current->lastLineIsEmpty()) *this << current->prompt();
    }
}

void
RetroShell::exec(const InputLine &cmd)
{
    try {

        // Call the interpreter
        current->exec(cmd);

    } catch (ScriptInterruption &) {

        // Rethrow the exception
        throw;

    } catch (std::exception &) {

        // Rethrow the exception if the command is not prefixed with 'try'
        if (cmd.input.rfind("try", 0)) throw;
    }
}

RetroShell &
RetroShell::operator<<(char value)
{
    *current << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(const char *value)
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

RetroShell&
RetroShell::operator<<(const vspace &value)
{
    *current << value;
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
RetroShell::press(RSKey key, bool shift)
{
    if (shift) {
        
        switch(key) {
                
            case RSKey::TAB:
                
                if (current->objid == 0) current->input = "debugger";
                if (current->objid == 1) current->input = "navigator";
                if (current->objid == 2) current->input = "commander";
                current->pressReturn(false);
                return;
                
            default:
                break;
        }
    }
    
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
    commander.setStream(os);
    debugger.setStream(os);
    navigator.setStream(os);
}

void
RetroShell::serviceEvent()
{
    emulator.put(Command(Cmd::RSH_EXECUTE));
    agnus.cancel<SLOT_RSH>();
}

}
