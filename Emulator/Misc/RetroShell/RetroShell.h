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

namespace vamiga {

class RetroShell : public SubComponent {

    friend class RshServer;
    friend class Interpreter;

    Descriptions descriptions = {{

        .name           = "RetroShell",
        .description    = "Retro Shell",
        .shell          = ""
    }};

    ConfigOptions options = {

    };

    enum class Shell { Command, Debug };

    // The currently active shell
    Shell shell = Shell::Command;

    // Consoles
    Console commander = Console(amiga);


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

    void _dump(Category category, std::ostream& os) const override { }
    void _initialize() override;
    // void _pause() override;


    //
    // Methods from Configurable
    //

public:

    const ConfigOptions &getOptions() const override { return options; }


    //
    // Bridge functions
    //

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
    void exec();
    void exec(const string &command);
    void execScript(std::stringstream &ss);
    void execScript(const std::ifstream &fs);
    void execScript(const string &contents);

    void serviceEvent();
};

}
