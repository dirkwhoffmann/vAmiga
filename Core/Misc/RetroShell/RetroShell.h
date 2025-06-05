// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RetroShellTypes.h"
#include "SubComponent.h"
#include "Console.h"
#include "TextStorage.h"
#include <sstream>
#include <fstream>
#include <functional>

/* RetroShell is a text-based command shell capable of controlling the emulator.
 * The shell's functionality is split among multiple consoles:
 *
 * 1. Commmand console:
 *
 *    This console is the default console and offers various command for
 *    configuring the emulator and performing actions such as ejecting a disk.
 *
 * 2. Debug console:
 *
 *    This console offers multiple debug command similar to the ones found in
 *    debug monitor. E.g., it is possible to inspect the registers of various
 *    components or generating a memory dump.
 */

namespace vamiga {

class RetroShell final : public SubComponent {

    friend class RshServer;

    Descriptions descriptions = {{

        .type           = Class::RetroShell,
        .name           = "RetroShell",
        .description    = "Retro Shell",
        .shell          = ""
    }};

    Options options = {

    };

public:

    // Consoles
    CommandConsole commander = CommandConsole(amiga, 0);
    DebugConsole debugger = DebugConsole(amiga, 1);

    // Indicates if one of the consoles has new contents
    bool isDirty = false;

private:
    
    // Command queue (stores all pending commands)
    std::vector<QueuedCmd> commands;

    // The currently active console
    Console *current = &commander;

    bool inCommandShell() { return current == &commander; }
    bool inDebugShell() { return current == &debugger; }


    //
    // Initializing
    //

public:

    RetroShell(Amiga& ref);
    RetroShell& operator= (const RetroShell& other) { return *this; }


    //
    // Methods from Serializable
    //

public:

    template <class T> void serialize(T& worker) { } SERIALIZERS(serialize);


    //
    // Methods from CoreComponent
    //

public:

    const Descriptions &getDescriptions() const override { return descriptions; }

private:

    void _dump(Category category, std::ostream &os) const override { }
    void _initialize() override;


    //
    // Methods from Configurable
    //

public:

    const Options &getOptions() const override { return options; }


    //
    // Managing consoles
    //

    void switchConsole();
    void enterDebugger();
    void enterCommander();


    //
    // Executing commands
    //

public:

    // Adds a command to the list of pending commands
    void asyncExec(const string &command, bool append = true);

    // Adds the commands of a shell script to the list of pending commands
    void asyncExecScript(std::stringstream &ss);
    void asyncExecScript(const std::ifstream &fs);
    void asyncExecScript(const string &contents);
    void asyncExecScript(const class MediaFile &script) throws;

    // Aborts the execution of a script
    void abortScript();
    
    // Executes all pending commands
    void exec() throws;

private:

    // Executes a single pending command
    void exec(QueuedCmd cmd) throws;
    

    //
    // Bridge functions
    //

public:
    
    RetroShell &operator<<(char value);
    RetroShell &operator<<(const string &value);
    RetroShell &operator<<(int value);
    RetroShell &operator<<(unsigned int value);
    RetroShell &operator<<(long value);
    RetroShell &operator<<(unsigned long value);
    RetroShell &operator<<(long long value);
    RetroShell &operator<<(unsigned long long value);
    RetroShell &operator<<(std::stringstream &stream);

    const char *text();
    isize cursorRel();
    void press(RetroShellKey key, bool shift = false);
    void press(char c);
    void press(const string &s);
    void setStream(std::ostream &os);
 
    void serviceEvent();
};

}
