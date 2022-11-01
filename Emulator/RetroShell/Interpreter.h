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

enum class Token
{
    about, accuracy, agnus, amiga, at, attach, audiate, audio, autofire,
    autosync, bankmap, beam, bitplanes, blitter, bp, brightness, bullets,
    callstack, channel, checksums, chip, cia, clear, close, clxsprspr,
    clxsprplf, clxplfplf, color, config, connect, contrast, controlport,
    copper, cp, cpu, cutout, dc, debug, defaults, delay, del, denise, detach,
    device, devices, dfn, diagboard, down, hdn, disable, disconnect, disk, dma,
    dmadebugger, drive, dsksync, easteregg, eject, enable, esync, events,
    execbase, extrom, extstart, fast, filename, filesystem, filter, fpu, gdb,
    geometry, help, hide, ignore, init, info, insert, inspect, interrupt,
    interrupts, joystick, jump, keyboard, keyset, layers, left, library,
    libraries, list, load, lock, mechanics, memory, mmu, mode, model, monitor,
    mouse, none, ntsc, off, on, opacity, open, os, overclocking, pal, palette,
    pan, partition, path, paula, pause, ptrdrops, poll, port, ports, power,
    press, process, processes, pull, pullup, raminitpattern, refresh,
    registers, regreset, regression, release, reset, resource, resources,
    revision, right, rom, rshell, rtc, run, sampling, saturation, save,
    saveroms, screenshot, searchpath, serial, server, set, setup, shakedetector,
    show, slow, slowramdelay, slowrammirror, source, speed, sprites, start,
    state, status, step, stop, swapdelay, swtraps, task, tasks, tod, todbug,
    tracking, translate, trap, unmappingtype, unpress, up, vector, verbose,
    velocity, volume, volumes, vsync, wait, watch, watchpoint, wom, wp, xaxis,
    yaxis, zorro
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
    // The registered instruction set
    Command root;
    
    
    //
    // Initializing
    //

public:
    
    Interpreter(Amiga &ref) : SubComponent(ref) { registerInstructions(); }

private:
    
    void registerInstructions();

    
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
