// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "Command.h"
#include "Exception.h"
#include "Error.h"
#include "Parser.h"

namespace vamiga {

struct TooFewArgumentsError : public util::ParseError {
    using ParseError::ParseError;
};

struct TooManyArgumentsError : public util::ParseError {
    using ParseError::ParseError;
};

struct ScriptInterruption: util::Exception {
    using Exception::Exception;
};

class Interpreter: public SubComponent
{
    enum class Shell { Command, Debug };

    // The currently active shell
    Shell shell = Shell::Command;

    // Commands of the command shell
    Command commandShellRoot; //  = new Command();

    // Commands of the debug shell
    Command debugShellRoot; //  = new Command();


    //
    // Initializing
    //

public:

    using SubComponent::SubComponent;

private:

    void initCommons(Command &root);
    void initCommandShell(Command &root);
    void initDebugShell(Command &root);


    //
    // Methods from CoreObject
    //

private:

    const char *getDescription() const override { return "Interpreter"; }
    void _dump(Category category, std::ostream& os) const override { }


    //
    // Methods from CoreComponent
    //

private:

    void _initialize() override;
    void _reset(bool hard) override { }


    //
    // Serializing
    //

private:

    isize _size() override { return 0; }
    u64 _checksum() override { return 0; }
    isize _load(const u8 *buffer) override {return 0; }
    isize _save(u8 *buffer) override { return 0; }


    //
    // Parsing input
    //

public:

    // Auto-completes a user command
    string autoComplete(const string& userInput);

private:

    // Splits an input string into an argument list
    Arguments split(const string& userInput);

    // Auto-completes an argument list
    void autoComplete(Arguments &argv);

    // Checks or parses an argument of a certain type
    bool isBool(const string &argv);
    bool isOnOff(const string &argv);
    long isNum(const string &argv);
    bool parseBool(const string  &argv);
    bool parseBool(const string  &argv, bool fallback);
    bool parseOnOff(const string &argv);
    bool parseOnOff(const string &argv, bool fallback);
    long parseNum(const string &argv);
    long parseNum(const string &argv, long fallback);
    u32 parseAddr(const string &argv) { return (u32)parseNum(argv); }
    u32 parseAddr(const string &argv, long fallback) { return (u32)parseNum(argv, fallback); }
    string parseSeq(const string &argv);
    string parseSeq(const string &argv, const string &fallback);
    template <typename T> long parseEnum(const string &argv) {
        return util::parseEnum<T>(argv);
    }
    template <typename T> long parseEnum(const string &argv, long fallback) {
        try { return util::parseEnum<T>(argv); } catch(...) { return fallback; }
    }


    //
    // Managing the interpreter
    //

public:

    // Returns the root node of the currently active instruction tree
    Command &getRoot();

    // Toggles between the command shell and the debug shell
    void switchInterpreter();

    bool inCommandShell() { return shell == Shell::Command; }
    bool inDebugShell() { return shell == Shell::Debug; }


    //
    // Executing commands
    //

public:

    // Executes a single command
    void exec(const string& userInput, bool verbose = false) throws;
    void exec(const Arguments &argv, bool verbose = false) throws;

    // Prints a usage string for a command
    void usage(const Command &command);

    // Displays a help text for a (partially typed in) command
    void help(const string &userInput);
    void help(const Arguments &argv);
    void help(const Command &command);

private:

    // Execution handlers (debug shell)
    void execRead(Arguments &argv, isize sz);
    void execWrite(Arguments &argv, isize sz);
    void execCopy(Arguments &argv, isize sz);
    void execFind(Arguments &argv, isize sz);
};

}
