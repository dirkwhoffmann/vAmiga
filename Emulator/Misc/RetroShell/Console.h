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
#include "Command.h"
#include "Parser.h"
#include "SubComponent.h"
#include "TextStorage.h"
#include <sstream>
#include <fstream>

namespace vamiga {

typedef std::pair<isize, string> QueuedCmd;

struct TooFewArgumentsError : public util::ParseError {
    using ParseError::ParseError;
};

struct TooManyArgumentsError : public util::ParseError {
    using ParseError::ParseError;
};

struct ScriptInterruption: util::Exception {
    using Exception::Exception;
};

class Console : public SubComponent {

    friend class RshServer;
    friend class Interpreter;

    Descriptions descriptions = {{

        .name           = "Console",
        .description    = "Command shell",
        .shell          = ""
    }};

    ConfigOptions options = {

    };

public:
    enum class Shell { Command, Debug };

    // The currently active shell
    Shell shell = Shell::Command;
private:
    
    // Commands of the command shell
    Command commandShellRoot;

    // Commands of the debug shell
    Command debugShellRoot;


    //
    // Text storage
    //

    // The text storage
    TextStorage storage;

    // History buffer storing old input strings and cursor positions
    std::vector<std::pair<string,isize>> history;

    // The currently active input string
    isize ipos = 0;


    //
    // User input
    //

    // Input line
    string input;

    // Command queue (stores all pending commands)
    std::vector<QueuedCmd> commands;

    // Input prompt
    string prompt = "vAmiga% ";

    // Cursor position
    isize cursor = 0;

    // Indicates if TAB was the most recently pressed key
    bool tabPressed = false;


    //
    // Initializing
    //

public:

    using SubComponent::SubComponent;
    Console& operator= (const Console& other) { return *this; }

private:

    void initCommons(Command &root);
    void initCommandShell(Command &root);
    void initDebugShell(Command &root);

    void initSetters(Command &root, const CoreComponent &c);


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
    void _pause() override;


    //
    // Methods from Configurable
    //

public:

    const ConfigOptions &getOptions() const override { return options; }


    //
    // Working with the text storage
    //

public:

    // Prints a message
    Console &operator<<(char value);
    Console &operator<<(const string &value);
    Console &operator<<(int value);
    Console &operator<<(unsigned int value);
    Console &operator<<(long value);
    Console &operator<<(unsigned long value);
    Console &operator<<(long long value);
    Console &operator<<(unsigned long long value);
    Console &operator<<(std::stringstream &stream);

    // Returns the prompt
    const string &getPrompt();

    // Updates the prompt according to the current shell mode
    void updatePrompt();

    // Returns the contents of the whole storage as a single C string
    const char *text();

    // Moves the cursor forward to a certain column
    void tab(isize pos);

    // Assigns an additional output stream
    void setStream(std::ostream &os);

    // Marks the text storage as dirty
    void needsDisplay();

private:

    // Clears the console window
    void clear();

    // Prints the welcome message
    void welcome();

    // Prints the help line
    void printHelp();

    // Prints a state summary (used by the debug shell)
    void printState();


    //
    // Managing user input
    //

public:

    // Returns the size of the current user-input string
    isize inputLength() { return (isize)input.length(); }

    // Presses a key or a series of keys
    void press(RetroShellKey key, bool shift = false);
    void press(char c);
    void press(const string &s);

    // Returns the cursor position relative to the line end
    isize cursorRel();

private:

    void pressReturn(bool shift);


    //
    // Working with the history buffer
    //

public:

    isize historyLength() { return (isize)history.size(); }


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
    bool parseBool(const string  &argv);
    bool parseBool(const string  &argv, bool fallback);
    bool parseBool(const Arguments &argv, long nr, long fallback);

    bool isOnOff(const string &argv);
    bool parseOnOff(const string &argv);
    bool parseOnOff(const string &argv, bool fallback);
    bool parseOnOff(const Arguments &argv, long nr, long fallback);

    long isNum(const string &argv);
    long parseNum(const string &argv);
    long parseNum(const string &argv, long fallback);
    long parseNum(const Arguments &argv, long nr, long fallback);

    u32 parseAddr(const string &argv) { return (u32)parseNum(argv); }
    u32 parseAddr(const string &argv, long fallback) { return (u32)parseNum(argv, fallback); }
    u32 parseAddr(const Arguments &argv, long nr, long fallback) { return (u32)parseNum(argv, nr, fallback); }

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

    bool inCommandShell() { return shell == Shell::Command; }
    bool inDebugShell() { return shell == Shell::Debug; }


    //
    // Executing commands
    //

public:

    // Adds a command to the list of pending commands
    void asyncExec(const string &command);

    // Adds the commands of a shell script to the list of pending commands
    void asyncExecScript(std::stringstream &ss) throws;
    void asyncExecScript(const std::ifstream &fs) throws;
    void asyncExecScript(const string &contents) throws;
    // void asyncExecScript(const class MediaFile &script) throws;

    // Aborts the execution of a script
    void abortScript();

    // Executes all pending commands
    void exec() throws;

    // Executes a single pending command
    void exec(QueuedCmd cmd) throws;

private:

    // Executes a single command
    void exec(const string& userInput, bool verbose = false) throws;
    void exec(const Arguments &argv, bool verbose = false) throws;

    // Prints a usage string for a command
    void usage(const Command &command);

    // Displays a help text for a (partially typed in) command
    void help(const string &command);
    void help(const Arguments &argv);
    void help(const Command &command);

    // Prints a textual description of an error in the console
    void describe(const std::exception &exception, isize line = 0, const string &cmd = "");


    //
    // Command handlers
    //

public:

    void dump(CoreObject &component, std::vector <Category> categories);
    void dump(CoreObject &component, Category category);

private:

    void _dump(CoreObject &component, Category category);
};

}
