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
    none,
    
    // Components
    agnus, amiga, audio, blitter, cia, controlport, copper, cpu, dc, denise,
    dfn, dmadebugger, keyboard, memory, monitor, mouse, paula, screenshot,
    serial, rtc,

    // Commands
    about, audiate, autosync, clear, config, connect, debug, disable,
    disconnect, dsksync, easteregg, eject, enable, close, hide, init, insert,
    inspect, list, load, lock, off, on, open, pause, power, reset, run, save,
    set, show, source, wait,
    
    // Categories
    checksums, devices, dma, events, registers, state,
        
    // Keys
    accuracy, bankmap, bitplanes, brightness, channel, chip, clxsprspr,
    clxsprplf, clxplfplf, color, contrast, cutout, defaultbb, defaultfs, delay,
    device, disk, esync, extrom, extstart, fast, filename, filter, joystick,
    keyset, mechanics, mode, model, opacity, palette, pan, path, poll, pullup,
    raminitpattern, refresh, revision, rom, sampling, saturation, searchpath,
    shakedetector, slow, slowramdelay, slowrammirror, speed, sprites, step,
    tod, todbug, unmappingtype, velocity, volume, wom
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
    void _dump(dump::Category category, std::ostream& os) const override { }

    
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
