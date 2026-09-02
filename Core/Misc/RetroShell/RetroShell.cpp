// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "RetroShell.h"
#include "RSError.h"
#include "Emulator.h"
#include "utl/io.h"
#include <istream>
#include <sstream>
#include <string>

namespace vamiga {

RetroShell::RetroShell(Amiga& ref, isize id) : SubComponent(ref, id)
{
    subComponents = std::vector<CoreComponent *> {
        
        &console
    };

    // The main shell boots into the Commander. The remote shell stays idle
    // until a client connects.
    if (isPrimary()) commands = { InputLine {.input = "commander"} };

    info.bind([this] { return cacheInfo(); } );
}

void
RetroShell::scheduleWakeup(Cycle delay)
{
    switch (objid) {

        case 0:     agnus.scheduleRel<SLOT_RSH0>(delay, RSH_WAKEUP); break;
        case 1:     agnus.scheduleRel<SLOT_RSH1>(delay, RSH_WAKEUP); break;
        case 2:     agnus.scheduleRel<SLOT_RSH2>(delay, RSH_WAKEUP); break;

        default:
            fatalError;
    }
}

void
RetroShell::cancelWakeup()
{
    switch (objid) {

        case 0:     agnus.cancel<SLOT_RSH0>(); break;
        case 1:     agnus.cancel<SLOT_RSH1>(); break;
        case 2:     agnus.cancel<SLOT_RSH2>(); break;

        default:
            fatalError;
    }
}

void
RetroShell::newSession()
{
    /* Called from the server threads. The lock keeps us from rebuilding the
     * command tree while the emulator thread is walking it.
     */
    {   SYNCHRONIZED

        commands = { };
        cancelWakeup();
        console.clear();
        enterCommander();
    }
}

void
RetroShell::_initialize()
{

}

RetroShellInfo
RetroShell::cacheInfo() const
{
    RetroShellInfo info;

    info.console = isize(console.getCommandSet());
    info.cursorRel = console.cursorRel();

    return info;
}

void
RetroShell::enterConsole(CommandSet cs)
{
    // Replace the command tree
    console.setCommandSet(cs);
    
    // Inform the GUI about the change
    msgQueue.put(Msg::RSH_SWITCH, objid, isize(cs));
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
            cancelWakeup();
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

            msgQueue.put(Msg::RSH_WAIT, objid);

        } catch (...) {

            // Remove all remaining commands
            commands = { };

            msgQueue.put(Msg::RSH_ERROR, objid);
        }

        // Print prompt
        if (console.lastLineIsEmpty()) *this << console.prompt();
    }
}

void
RetroShell::exec(const InputLine &cmd)
{
    try {

        // Call the interpreter
        console.exec(cmd);

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
    console << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(const char *value)
{
    console << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(const string &value)
{
    console << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(int value)
{
    console << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(unsigned int value)
{
    console << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(long value)
{
    console << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(unsigned long value)
{
    console << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(long long value)
{
    console << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(unsigned long long value)
{
    console << value;
    return *this;
}

RetroShell &
RetroShell::operator<<(std::stringstream &stream)
{
    console << stream;
    return *this;
}

RetroShell&
RetroShell::operator<<(const vspace &value)
{
    console << value;
    return *this;
}

const char *
RetroShell::text()
{
    return console.text();
}

isize
RetroShell::cursorRel()
{
    return console.cursorRel();
}

void
RetroShell::press(RSKey key, bool shift)
{
    if (shift) {
        
        switch(key) {
                
            case RSKey::TAB:
                
                // Cycle through the available command sets
                switch (console.getCommandSet()) {
                        
                    case CommandSet::Commander:     console.input = "debugger"; break;
                    // case CommandSet::Debugger:      console.input = "navigator"; break;
                    // case CommandSet::Navigator:     console.input = "cbmnavigator"; break;
                    // case CommandSet::CBMNavigator:  console.input = "commander"; break;

                    default:
                        console.input = "commander";
                        break;
                }
                console.pressReturn(false);
                return;
                
            default:
                break;
        }
    }
    
    console.press(key, shift);
}

void
RetroShell::press(char c)
{
    console.press(c);
}

void
RetroShell::press(const string &s)
{
    console.press(s);
}

void
RetroShell::setStream(std::ostream &os)
{
    console.setStream(os);
}

void
RetroShell::serviceEvent()
{
    emulator.put(Command(Cmd::RSH_EXECUTE));
    cancelWakeup();
}

}
