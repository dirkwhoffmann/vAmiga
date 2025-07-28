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
#include "RSCommand.h"
#include "Parser.h"
#include "TextStorage.h"
#include "MutableFileSystem.h"

namespace vamiga {

typedef std::pair<isize, string> QueuedCmd;

struct TooFewArgumentsError : public util::ParseError {
    using ParseError::ParseError;
};

struct TooManyArgumentsError : public util::ParseError {
    using ParseError::ParseError;
};

struct UnknownFlagError : public util::ParseError {
    using ParseError::ParseError;
};

struct UnknownKeyValueError : public util::ParseError {
    using ParseError::ParseError;
};

struct ScriptInterruption: AppException {
    using AppException::AppException;
};

class HistoryBuffer {

    // History buffer storing old input strings and cursor positions
    std::vector<std::pair<string,isize>> history = { { "", 0 } };

    // The currently active input string
    isize ipos = 0;

public:

    // Returns the current selection
    std::pair<string,isize> current() const { return history[ipos]; }

    // Iterate through the buffer
    void up(string &input, isize &ipos);
    void down(string &input, isize &ipos);

    // Add an entry to the buffer
    void add(const string &input);
};

class Console : public SubComponent {

    friend class RetroShell;
    friend class RshServer;
    friend class Interpreter;

    Descriptions descriptions = {
        {
            .type           = Class::Console,
            .name           = "CmdConsole",
            .description    = "Commander",
            .shell          = ""
        },
        {
            .type           = Class::Console,
            .name           = "DbgConsole",
            .description    = "Debugger",
            .shell          = ""
        },
        {
            .type           = Class::Console,
            .name           = "NavConsole",
            .description    = "Navigator",
            .shell          = ""
        }
    };

    Options options = {

    };

protected:

    // Root node of the command tree
    RSCommand root;

    // Memory pointer for commands accpeting default addresses
    u32 current = 0;

    //
    // Text storage
    //

protected:
    
    // The text storage
    TextStorage &storage;

    // History buffer storing old input strings and cursor positions
    static HistoryBuffer historyBuffer;

    // Additional output inserted before and after command execution
    string vdelim = RSH_DEBUG ? "[DEBUG]\n" : "\n";

    //
    // User input
    //

protected:

    // Input line
    string input;

    // Cursor position
    isize cursor {};

    // Indicates how often TAB was pressed in a row
    isize tabPressed {};


    //
    // Initializing
    //

public:

    // using SubComponent::SubComponent;

    Console(Amiga &amiga, isize id, TextStorage &storage) : SubComponent(amiga, id), storage(storage) { };

    Console& operator= (const Console& other) { return *this; }

protected:

    virtual void initCommands(RSCommand &root);
    const char *registerComponent(CoreComponent &c, bool shadowed = false);
    const char *registerComponent(CoreComponent &c, RSCommand &root, bool shadowed = false);


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

protected:

    void _dump(Category category, std::ostream &os) const override { }
    void _initialize() override;


    //
    // Methods from Configurable
    //

public:

    const Options &getOptions() const override { return options; }


    //
    // Working with the text storage
    //

public:

    // Prints a message
    Console &operator<<(char value);
    Console &operator<<(const string &value);
    Console &operator<<(const char *value);
    Console &operator<<(int value);
    Console &operator<<(unsigned int value);
    Console &operator<<(long value);
    Console &operator<<(unsigned long value);
    Console &operator<<(long long value);
    Console &operator<<(unsigned long long value);
    Console &operator<<(const std::vector<string> &vec);
    Console &operator<<(std::stringstream &stream);
    Console &operator<<(const FSName &value);
    Console &operator<<(const FSBlock &value);
    Console &operator<<(const vspace &value);

    // Returns the prompt
    virtual string getPrompt() = 0;

    // Returns the contents of the whole storage as a single C string
    const char *text();

    // Moves the cursor forward to a certain column
    void tab(isize pos);

    // Assigns an additional output stream
    void setStream(std::ostream &os);

    // Marks the text storage as dirty
    void needsDisplay();

protected:

    // Clears the console window
    void clear();

    // Returns true if the console is cleared
    bool isEmpty();

    // Returns true if the last line contains no text
    bool lastLineIsEmpty();

    // Prints the welcome message
    virtual void welcome() = 0;

    // Prints the status summary
    virtual void summary() = 0;

    // Prints the help line
    virtual void printHelp(isize tab = 0);


    //
    // Managing user input
    //

public:

    // Returns the size of the current user-input string
    isize inputLength() { return (isize)input.length(); }

    // Presses a key or a series of keys
    void press(RSKey key, bool shift = false);
    void press(char c);
    void press(const string &s);

    // Returns the cursor position relative to the line end
    isize cursorRel();

protected:

    virtual void pressReturn(bool shift);


    //
    // Parsing input
    //

public:

    // Auto-completes a user command
    string autoComplete(const string& userInput);

protected:

    // Splits an input string into an argument list
    Tokens split(const string& userInput);

    // Auto-completes an argument list
    virtual void autoComplete(Tokens &argv);

    // Strips off the command tokens and returns a pointer to the command
    std::pair<RSCommand *, std::vector<string>> seekCommand(const string &argv);
    std::pair<RSCommand *, std::vector<string>> seekCommand(const std::vector<string> &argv);

    // Parses an argument list
    std::map<string,string> parse(const RSCommand &cmd, const Tokens &args);

    // Checks or parses an argument of a certain type
    bool isBool(const string &argv) const;
    bool parseBool(const string  &argv) const;
    bool parseBool(const string  &argv, bool fallback) const;
    bool parseBool(const Arguments &argv, const string &key) const;
    bool parseBool(const Arguments &argv, const string &key, long fallback) const;

    bool isOnOff(const string &argv) const;
    bool parseOnOff(const string &argv) const;
    bool parseOnOff(const string &argv, bool fallback) const;
    bool parseOnOff(const Arguments &argv, const string &key) const;
    bool parseOnOff(const Arguments &argv, const string &key, long fallback) const;

    long isNum(const string &argv) const;
    long parseNum(const string &argv) const;
    long parseNum(const string &argv, long fallback) const;
    long parseNum(const Arguments &argv, const string &key) const;
    long parseNum(const Arguments &argv, const string &key, long fallback) const;

    u32 parseAddr(const string &argv) const;
    u32 parseAddr(const string &argv, long fallback) const;
    u32 parseAddr(const Arguments &argv, const string &key) const;
    u32 parseAddr(const Arguments &argv, const string &key, long fallback) const;

    string parseSeq(const string &argv) const;
    string parseSeq(const string &argv, const string &fallback) const;

    template <typename T> long parseEnum(const string &argv) {
        return util::parseEnum<T>(argv);
    }
    template <typename T> long parseEnum(const string &argv, long fallback) {
        try { return util::parseEnum<T>(argv); } catch(...) { return fallback; }
    }
    template <typename E, typename T> E parseEnum(const string &argv) {
        return (E)util::parseEnum<T>(argv);
    }
    template <typename E, typename T> long parseEnum(const string &argv, E fallback) {
        try { return (E)util::parseEnum<T>(argv); } catch(...) { return fallback; }
    }


    //
    // Managing the interpreter
    //

public:

    // Returns the root node of the instruction tree
    RSCommand &getRoot() { return root; }

protected:

    // Executes a single command
    void exec(const string& userInput, bool verbose = false) throws;
    void exec(const Tokens &argv, bool verbose = false) throws;

    // Prints a usage string for a command
    void cmdUsage(const RSCommand &cmd, const string &prefix);
    void argUsage(const RSCommand &cmd, const string &prefix);

    // Displays a help text for a (partially typed in) command
    virtual void help(std::ostream &os, const string &cmd, isize tabs);

    // Creates a textual description of an error
    void describe(const std::exception &exc, isize line = 0, const string &cmd = "");
    void describe(std::ostream &os, const std::exception &exc, isize line = 0, const string &cmd = "");


    //
    // Command handlers
    //

public:

    void dump(std::ostream &os, CoreObject &component, std::vector <Category> categories);
    void dump(std::ostream &os, CoreObject &component, Category category);

protected:

    void _dump(std::ostream &os, CoreObject &component, Category category);
};

class CommanderConsole final : public Console
{
    using Console::Console;

    virtual void initCommands(RSCommand &root) override;
    void _pause() override;
    string getPrompt() override;
    void welcome() override;
    void summary() override;
    void printHelp(isize tab = 0) override;
    void pressReturn(bool shift) override;
};

class DebuggerConsole final : public Console
{
    using Console::Console;

    //
    // Methods from Console
    //

    virtual void initCommands(RSCommand &root) override;
    void _pause() override;
    string getPrompt() override;
    void welcome() override;
    void summary() override;
    void printHelp(isize tab = 0) override;
    void pressReturn(bool shift) override;
};

class NavigatorConsole final : public Console
{
    MutableFileSystem fs;

    using Console::Console;

    //
    // Methods from Console
    //

    virtual void initCommands(RSCommand &root) override;
    void _pause() override;
    string getPrompt() override;
    void welcome() override;
    void summary() override;
    void printHelp(isize tab = 0) override;
    void pressReturn(bool shift) override;
    void autoComplete(Tokens &argv) override;
    void help(std::ostream &os, const string &argv, isize tabs) override;
    string autoCompleteFilename(const string &input, usize flags) const;

    //
    // Parsing input
    //

    Block parseBlock(const string &arg);
    Block parseBlock(const Arguments &argv, const string &token);
    Block parseBlock(const Arguments &argv, const string &token, Block fallback);
    FSBlock &parsePath(const Arguments &argv, const string &token);
    FSBlock &parsePath(const Arguments &argv, const string &token, FSBlock &fallback);
    FSBlock &parseFile(const Arguments &argv, const string &token);
    FSBlock &parseFile(const Arguments &argv, const string &token, FSBlock &fallback);
    FSBlock &parseDirectory(const Arguments &argv, const string &token);
    FSBlock &parseDirectory(const Arguments &argv, const string &token, FSBlock &fallback);

    util::DumpOpt parseDumpOpts(const Arguments &argv);

    // Experimental
    FSBlock &matchPath(const Arguments &argv, const string &token, Tokens &notFound);
    FSBlock &matchPath(const Arguments &argv, const string &token, Tokens &notFound, FSBlock &fallback);
    FSBlock &matchPath(const string &path, Tokens &notFound);

public:

    // Imports the file system from a floppy drive or hard drive
    void import(const FloppyDrive &dfn);
    void import(const HardDrive &hdn, isize part);
    void importDf(isize n);
    void importHd(isize n, isize part);
    void import(const fs::path &path, bool recursive = true, bool contents = false);

    // Exports the file system
    void exportBlocks(fs::path path);
};

}
