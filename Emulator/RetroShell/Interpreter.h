// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "Command.h"
#include "Exception.h"
#include "Error.h"
#include "Parser.h"

namespace vamiga {

enum class Token
{
    about, accuracy, activation, agnus, amiga, at, attach, audiate, audio,
    autofire, autosync, bankmap, beam, bitplanes, blitter, bp, brightness,
    bullets, cbp, channel, checksums, chip, cia, ciaa, ciab, clear, close,
    clxsprspr, clxsprplf, clxplfplf, color, config, connect, contrast,
    controlport, copper, cp, cpu, cutout, cwp, dasm, dc, debug, defaults,
    delay, del, denise, detach, device, devices, dfn, diagboard, disassemble,
    down, disable, disconnect, disk, dma, dmadebugger, drive, dsksync,
    easteregg, eject, enable, esync, events, execbase, extrom, extstart, fast,
    filename, filesystem, filter, fpu, gdb, geometry, hdn, help, hide, host,
    ignore, init, info, insert, inspect, interrupt, interrupts, joystick, jump,
    keyboard, keyset, layers, left, library, libraries, list, load, lock,
    mechanics, memdump, memory, mmu, mode, model, monitor, mouse, next, none,
    opacity, open, os, overclocking, palette, pan, partition, path,
    paula, pause, ptrdrops, poll, port, ports, power, press, process,
    processes, pull, pullup, raminitpattern, refresh, registers, regreset,
    regression, release, reset, resource, resources, revision, right, rom, rpm,
    rshell, rtc, run, sampling, saturation, save, saveroms, screenshot,
    searchpath, serial, server, set, setup, shakedetector, show, slow,
    slowramdelay, slowrammirror, source, speed, sprites, start, status, step,
    stop, swapdelay, swtraps, syntax, task, tasks, tod, todbug, tracking,
    translate, trap, type, uart, unmappingtype, unpress, up, vector, vectors,
    verbose, velocity, volume, volumes, vsync, wait, watch, watchpoint, wom,
    wp, write, xaxis, yaxis, zorro
};

struct TooFewArgumentsError : public util::ParseError {
    using ParseError::ParseError;
};

struct TooManyArgumentsError : public util::ParseError {
    using ParseError::ParseError;
};

struct ScriptInterruption: util::Exception {
    using Exception::Exception;
};

class Interpreter: SubComponent
{
    enum class Shell { Command, Debug };

    // The currently active shell
    Shell shell = Shell::Command;

    // Commands of the command shell
    Command commandShellRoot;

    // Commands of the debug shell
    Command debugShellRoot;

    
    //
    // Initializing
    //

public:
    
    Interpreter(Amiga &ref);

private:
    
    void initCommons(Command &root);
    void initCommandShell(Command &root);
    void initDebugShell(Command &root);

    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "Interpreter"; }
    void _dump(Category category, std::ostream& os) const override { }

    
    //
    // Methods from AmigaComponent
    //
    
private:
    
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

};

}
