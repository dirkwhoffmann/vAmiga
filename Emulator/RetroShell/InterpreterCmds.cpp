// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Interpreter.h"
#include "RetroShell.h"

namespace vamiga {

void
Interpreter::initCommons(Command &root)
{
    //
    // Common commands
    //

    root.newGroup("Controlling the shell");

    root.add({"clear"},
             "Clears the console window",
             &RetroShell::exec <Token::clear>, 0);
    // root.seek("clear")->hidden = true;

    root.add({"close"},
             "Hides the console window",
             &RetroShell::exec <Token::close>, 0);
    // root.seek("close")->hidden = true;

    root.add({"help"},
             "Prints usage information",
             &RetroShell::exec <Token::help>, {0, 1});
    // root.seek("help")->hidden = true;

    root.add({"joshua"},
             "",
             &RetroShell::exec <Token::easteregg>, 0);
    root.seek("joshua")->hidden = true;

    root.add({"source"},
             "Processes a command script",
             &RetroShell::exec <Token::source>, 1);
    // root.seek("source")->hidden = true;

    root.add({"wait"},
             "Pauses the execution of a command script",
             &RetroShell::exec <Token::wait>, 2);
    root.seek("wait")->hidden = true;
}

void
Interpreter::initCommandShell(Command &root)
{
    initCommons(root);

    //
    // Top-level commands
    //

    root.newGroup("Regression testing");

    root.add({"regression"},
             "Runs the regression tester");
    root.seek("regression")->hidden = true;


    root.newGroup("Managing components");

    root.add({"amiga"},
             "The virtual Amiga");

    root.add({"memory"},
             "Ram and Rom");

    root.add({"cpu"},
             "Motorola 68k CPU");

    root.add({"ciaa"},
             "Complex Interface Adapter A");

    root.add({"ciab"},
             "Complex Interface Adapter B");

    root.add({"agnus"},
             "Custom chip");

    root.add({"blitter"},
             "Coprocessor");

    root.add({"copper"},
             "Coprocessor");

    /*
    root.add({"paula"},
             "Custom chip");
    */

    root.add({"denise"},
             "Custom chip");

    root.add({"diskcontroller"},
             "Disk Controller");

    root.add({"rtc"},
             "Real-time clock");

    root.add({"controlport1"},
             "Control port 1");

    root.add({"controlport2"},
             "Control port 2");

    root.add({"serial"},
             "Serial port");

    root.add({"zorro"},
             "Expansion boards");

    root.add({"dmadebugger"},
             "DMA Debugger");


    root.newGroup("Managing peripherals");

    root.add({"keyboard"},
             "Keyboard");

    root.add({"mouse1"},
             "Port 1 mouse");

    root.add({"mouse2"},
             "Port 2 mouse");

    root.add({"joystick1"},
             "Port 1 joystick");

    root.add({"joystick2"},
             "Port 2 joystick");

    root.add({"df0"},
             "Floppy drive 0");

    root.add({"df1"},
             "Floppy drive 1");

    root.add({"df2"},
             "Floppy drive 2");

    root.add({"df3"},
             "Floppy drive 3");

    root.add({"dfn"},
             "All connected drives");

    root.add({"hd0"},
             "Hard drive 0");

    root.add({"hd1"},
             "Hard drive 1");

    root.add({"hd2"},
             "Hard drive 2");

    root.add({"hd3"},
             "Hard drive 3");

    root.add({"hdn"},
             "All connected hard drives");


    root.newGroup("Configuring audio and video");

    root.add({"monitor"},
             "Amiga monitor");

    root.add({"audio"},
             "Audio Unit (Paula)");


    root.newGroup("Miscellaneous");

    root.add({"server"},
             "Remote connections");


    //
    // Regression testing
    //

    root.newGroup("");

    root.add({"regression", "setup"},
             "Initializes the test environment",
             &RetroShell::exec <Token::regression, Token::setup>, {2, 3});

    root.add({"regression", "run"},
             "Launches a regression test",
             &RetroShell::exec <Token::regression, Token::run>, 1);
    
    root.add({"screenshot"},
             "Manages regression tests");
    root.seek("screenshot")->hidden = true;
    
    root.add({"screenshot", "set"},
             "Configures the regression test");

    root.add({"screenshot", "set", "filename"},
             "Assigns the screen shot filename",
             &RetroShell::exec <Token::screenshot, Token::set, Token::filename>, 1);

    root.add({"screenshot", "set", "cutout"},
             "Adjusts the texture cutout",
             &RetroShell::exec <Token::screenshot, Token::set, Token::cutout>, 4);

    root.add({"screenshot", "save"},
             "Saves a screenshot and exits the emulator",
             &RetroShell::exec <Token::screenshot, Token::save>, 1);

    
    //
    // Amiga
    //

    root.newGroup("");

    root.add({"amiga", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::amiga, Token::config>, 0);

    root.add({"amiga", "set"},
             "Configures the component");

    root.add({"amiga", "set", "pal"},
             "Emulates a PAL machine",
             &RetroShell::exec <Token::amiga, Token::set, Token::pal>, 0);

    root.add({"amiga", "set", "ntsc"},
             "Emulates a NTSC machine",
             &RetroShell::exec <Token::amiga, Token::set, Token::ntsc>, 0);

    root.add({"amiga", "vsync"},
             "Turns VSYNC on or off");

    root.add({"amiga", "vsync", "on"},
             "Turns VSYNC on",
             &RetroShell::exec <Token::amiga, Token::vsync, Token::on>, 0);

    root.add({"amiga", "vsync", "off"},
             "Turns VSYNC off",
             &RetroShell::exec <Token::amiga, Token::vsync, Token::off>, 0);

    root.add({"amiga", "init"},
             "Initializes the Amiga with a predefined scheme",
             &RetroShell::exec <Token::amiga, Token::init>, 1);

    root.add({"amiga", "power"},
             "Switches the Amiga on or off");
    
    root.add({"amiga", "power", "on"},
             "Switches the Amiga on",
             &RetroShell::exec <Token::amiga, Token::power, Token::on>, 0);

    root.add({"amiga", "power", "off"},
             "Switches the Amiga off",
             &RetroShell::exec <Token::amiga, Token::power, Token::off>, 0);

    root.add({"amiga", "reset"},
             "Performs a hard reset",
             &RetroShell::exec <Token::amiga, Token::reset>, 0);

    root.add({"amiga", "defaults"},
             "Displays the user defaults storage",
             &RetroShell::exec <Token::amiga, Token::defaults>, 0);


    //
    // Memory
    //

    root.newGroup("");

    root.add({"memory", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::memory, Token::config>, 0);

    root.add({"memory", "set"},
             "Configures the component");

    root.add({"memory", "set", "chip"},
             "Configures the amouts of chip memory",
             &RetroShell::exec <Token::memory, Token::set, Token::chip>, 1);

    root.add({"memory", "set", "slow"},
             "Configures the amouts of slow memory",
             &RetroShell::exec <Token::memory, Token::set, Token::slow>, 1);

    root.add({"memory", "set", "fast"},
             "Configures the amouts of flow memory",
             &RetroShell::exec <Token::memory, Token::set, Token::fast>, 1);

    root.add({"memory", "set", "extstart"},
             "Sets the start address for Rom extensions",
             &RetroShell::exec <Token::memory, Token::set, Token::extstart>, 1);

    root.add({"memory", "set", "saveroms"},
             "Determines whether Roms should be stored in snapshots",
             &RetroShell::exec <Token::memory, Token::set, Token::saveroms>, 1);

    root.add({"memory", "set", "slowramdelay"},
             "Enables or disables slow Ram bus delays",
             &RetroShell::exec <Token::memory, Token::set, Token::slowramdelay>, 1);

    root.add({"memory", "set", "bankmap"},
             "Selects the bank mapping scheme",
             &RetroShell::exec <Token::memory, Token::set, Token::bankmap>, 1);

    root.add({"memory", "set", "unmapped"},
             "Determines the behaviour of unmapped memory",
             &RetroShell::exec <Token::memory, Token::set, Token::unmappingtype>, 1);

    root.add({"memory", "set", "raminit"},
             "Determines how Ram is initialized on startup",
             &RetroShell::exec <Token::memory, Token::set, Token::raminitpattern>, 1);
    
    root.add({"memory", "load"},
             "Installs a Rom image");

    root.add({"memory", "load", "rom"},
             "Installs a Kickstart Rom",
             &RetroShell::exec <Token::memory, Token::load, Token::rom>, 1);

    root.add({"memory", "load", "extrom"},
             "Installs a Rom extension",
             &RetroShell::exec <Token::memory, Token::load, Token::extrom>, 1);

    
    //
    // CPU
    //

    root.newGroup("");

    root.add({"cpu", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::cpu, Token::config>, 0);

    root.add({"cpu", "set"},
             "Configures the component");

    root.add({"cpu", "set", "revision"},
             "Selects the emulated chip model",
             &RetroShell::exec <Token::cpu, Token::set, Token::revision>, 1);

    root.add({"cpu", "set", "overclocking"},
             "Overclocks the CPU by the specified factor",
             &RetroShell::exec <Token::cpu, Token::set, Token::overclocking>, 1);

    root.add({"cpu", "set", "regreset"},
             "Selects the reset value of data and address registers",
             &RetroShell::exec <Token::cpu, Token::set, Token::regreset>, 1);

    root.add({"cpu", "callstack" },
             "Prints recorded subroutine calls (DEPRECATED)",
             &RetroShell::exec <Token::cpu, Token::callstack>, 0);

    //
    // MMU
    //

    /*
     root.add({"mmu"},
     "Memory management unit");

     root.add({"mmu", "inspect"},
     "Displays the component state",
     &RetroShell::exec <Token::mmu, Token::inspect>, 0);
     */

    //
    // FPU
    //

    /*
     root.add({"fpu"},
     "Floating point unit");

     root.add({"fpu", "inspect"},
     "Displays the component state",
     &RetroShell::exec <Token::fpu, Token::inspect>, 0);
     */

    //
    // CIA
    //

    root.newGroup("");

    for (isize i = 0; i < 2; i++) {
        
        string cia = (i == 0) ? "ciaa" : "ciab";
        
        root.add({cia, ""},
                 "Displays the current configuration",
                 &RetroShell::exec <Token::cia, Token::config>, 0, i);
        
        root.add({cia, "set"},
                 "Configures the component");
        
        root.add({cia, "set", "revision"},
                 "Selects the emulated chip model",
                 &RetroShell::exec <Token::cia, Token::set, Token::revision>, 1, i);
        
        root.add({cia, "set", "todbug"},
                 "Enables or disables the TOD hardware bug",
                 &RetroShell::exec <Token::cia, Token::set, Token::todbug>, 1, i);
        
        root.add({cia, "set", "esync"},
                 "Turns E-clock syncing on or off",
                 &RetroShell::exec <Token::cia, Token::set, Token::esync>, 1, i);
    }
    
    
    //
    // Agnus
    //

    root.newGroup("");

    root.add({"agnus", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::agnus, Token::config>, 0);
    
    root.add({"agnus", "set"},
             "Configures the component");

    root.add({"agnus", "set", "revision"},
             "Selects the emulated chip model",
             &RetroShell::exec <Token::agnus, Token::set, Token::revision>, 1);

    root.add({"agnus", "set", "slowrammirror"},
             "Enables or disables ECS Slow Ram mirroring",
             &RetroShell::exec <Token::agnus, Token::set, Token::slowrammirror>, 1);

    root.add({"agnus", "set", "ptrdrops"},
             "Emulate dropped register writes",
             &RetroShell::exec <Token::agnus, Token::set, Token::ptrdrops>, 1);

    
    //
    // Blitter
    //

    root.newGroup("");

    root.add({"blitter", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::blitter, Token::config>, 0);
    
    root.add({"blitter", "set"},
             "Configures the component");

    root.add({"blitter", "set", "accuracy"},
             "Selects the emulation accuracy level",
             &RetroShell::exec <Token::blitter, Token::set, Token::accuracy>, 1);


    //
    // Copper
    //

    // TODO: MOVE TO DEBUG SHELL
    root.add({"copper", "break"},
             "Manages breakpoints");

    root.add({"copper", "break", "info"},
             "Lists all breakpoints",
             &RetroShell::exec <Token::copper, Token::bp, Token::info>, 0);

    root.add({"copper", "break", "at"},
             "Sets a breakpoint at the specified address",
             &RetroShell::exec <Token::copper, Token::bp, Token::at>, 1);

    root.add({"copper", "break", "delete"},
             "Deletes a breakpoint",
             &RetroShell::exec <Token::copper, Token::bp, Token::del>, 1);

    root.add({"copper", "break", "enable"},
             "Enables a breakpoint",
             &RetroShell::exec <Token::copper, Token::bp, Token::enable>, 1);

    root.add({"copper", "break", "disable"},
             "Disables a breakpoint",
             &RetroShell::exec <Token::copper, Token::bp, Token::disable>, 1);

    root.add({"copper", "break", "ignore"},
             "Ignores a breakpoint a certain number of times",
             &RetroShell::exec <Token::copper, Token::bp, Token::ignore>, 2);

    root.add({"copper", "watch"},
             "Manages watchpoints");

    root.add({"copper", "watch", "info"},
             "Lists all watchpoints",
             &RetroShell::exec <Token::copper, Token::wp, Token::info>, 0);

    root.add({"copper", "watch", "at"},
             "Sets a watchpoint at the specified address",
             &RetroShell::exec <Token::copper, Token::wp, Token::at>, 1);

    root.add({"copper", "watch", "delete"},
             "Deletes a watchpoint",
             &RetroShell::exec <Token::copper, Token::wp, Token::del>, 1);

    root.add({"copper", "watch", "enable"},
             "Enables a watchpoint",
             &RetroShell::exec <Token::copper, Token::wp, Token::enable>, 1);

    root.add({"copper", "watch", "disable"},
             "Disables a watchpoint",
             &RetroShell::exec <Token::copper, Token::wp, Token::disable>, 1);

    root.add({"copper", "watch", "ignore"},
             "Ignores a watchpoint a certain number of times",
             &RetroShell::exec <Token::copper, Token::wp, Token::ignore>, 2);
    
    
    //
    // Denise
    //

    root.newGroup("");

    root.add({"denise", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::denise, Token::config>, 0);

    root.add({"denise", "set"},
             "Configures the component");

    root.add({"denise", "set", "revision"},
             "Selects the emulated chip model",
             &RetroShell::exec <Token::denise, Token::set, Token::revision>, 1);

    root.add({"denise", "set", "tracking"},
             "Enables or disables viewport tracking",
             &RetroShell::exec <Token::denise, Token::set, Token::tracking>, 1);

    root.add({"denise", "set", "clxsprspr"},
             "Enables or disables sprite-sprite collision detection",
             &RetroShell::exec <Token::denise, Token::set, Token::clxsprspr>, 1);

    root.add({"denise", "set", "clxsprplf"},
             "Enables or disables sprite-playfield collision detection",
             &RetroShell::exec <Token::denise, Token::set, Token::clxsprplf>, 1);

    root.add({"denise", "set", "clxplfplf"},
             "Enables or disables playfield-playfield collision detection",
             &RetroShell::exec <Token::denise, Token::set, Token::clxplfplf>, 1);
    
    root.add({"denise", "hide"},
             "Hides bitplanes, sprites, or layers");

    root.add({"denise", "hide", "bitplanes"},
             "Wipes out certain bitplane data",
             &RetroShell::exec <Token::denise, Token::hide, Token::bitplanes>, 1);

    root.add({"denise", "hide", "sprites"},
             "Wipes out certain sprite data",
             &RetroShell::exec <Token::denise, Token::hide, Token::sprites>, 1);

    root.add({"denise", "hide", "sprites"},
             "Makes certain drawing layers transparent",
             &RetroShell::exec <Token::denise, Token::hide, Token::layers>, 1);

    root.add({"denise", "inspect"},
             "Displays the internal state");

    root.add({"denise", "inspect", "state"},
             "Displays the current state",
             &RetroShell::exec <Token::denise, Token::inspect, Token::state>, 0);

    root.add({"denise", "inspect", "registers"},
             "Displays the current register value",
             &RetroShell::exec <Token::denise, Token::inspect, Token::registers>, 0);

    
    //
    // DMA Debugger
    //

    root.add({"dmadebugger", "open"},
             "Opens the DMA debugger",
             &RetroShell::exec <Token::dmadebugger, Token::open>, 0);

    root.add({"dmadebugger", "close"},
             "Closes the DMA debugger",
             &RetroShell::exec <Token::dmadebugger, Token::close>, 0);

    root.add({"dmadebugger", "show"},
             "Enables the debugger for a certain DMA channel");

    root.add({"dmadebugger", "show", "copper"},
             "Visualizes Copper DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::copper>, 0);

    root.add({"dmadebugger", "show", "blitter"},
             "Visualizes Blitter DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::blitter>, 0);

    root.add({"dmadebugger", "show", "disk"},
             "Visualizes Disk DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::disk>, 0);

    root.add({"dmadebugger", "show", "audio"},
             "Visualizes Audio DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::audio>, 0);

    root.add({"dmadebugger", "show", "sprites"},
             "Visualizes Sprite DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::sprites>, 0);

    root.add({"dmadebugger", "show", "bitplanes"},
             "Visualizes Bitplane DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::bitplanes>, 0);

    root.add({"dmadebugger", "show", "cpu"},
             "Visualizes CPU accesses",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::cpu>, 0);

    root.add({"dmadebugger", "show", "refresh"},
             "Visualizes memory refresh cycles",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::refresh>, 0);

    root.add({"dmadebugger", "hide"},
             "Disables the debugger for a certain DMA channel");

    root.add({"dmadebugger", "hide", "copper"},
             "Hides Copper DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::copper>, 0);

    root.add({"dmadebugger", "hide", "blitter"},
             "Hides Blitter DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::blitter>, 0);

    root.add({"dmadebugger", "hide", "disk"},
             "Hides Disk DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::disk>, 0);

    root.add({"dmadebugger", "hide", "audio"},
             "Hides Audio DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::audio>, 0);

    root.add({"dmadebugger", "hide", "sprites"},
             "Hides Sprite DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::sprites>, 0);

    root.add({"dmadebugger", "hide", "bitplanes"},
             "Hides Bitplane DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::bitplanes>, 0);

    root.add({"dmadebugger", "hide", "cpu"},
             "Hides CPU accesses",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::cpu>, 0);

    root.add({"dmadebugger", "hide", "refresh"},
             "Hides memory refresh cycles",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::refresh>, 0);

    
    //
    // Monitor
    //

    root.add({"monitor", "set"},
             "Configures the component");

    root.add({"monitor", "set", "palette"},
             "Selects the color palette",
             &RetroShell::exec <Token::monitor, Token::set, Token::palette>, 1);

    root.add({"monitor", "set", "brightness"},
             "Adjusts the brightness of the Amiga texture",
             &RetroShell::exec <Token::monitor, Token::set, Token::brightness>, 1);

    root.add({"monitor", "set", "contrast"},
             "Adjusts the contrast of the Amiga texture",
             &RetroShell::exec <Token::monitor, Token::set, Token::contrast>, 1);

    root.add({"monitor", "set", "saturation"},
             "Adjusts the saturation of the Amiga texture",
             &RetroShell::exec <Token::monitor, Token::set, Token::saturation>, 1);

    
    //
    // Audio
    //

    root.newGroup("");

    root.add({"audio", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::audio, Token::config>, 0);

    root.add({"audio", "set"},
             "Configures the component");

    root.add({"audio", "set", "sampling"},
             "Selects the sampling method",
             &RetroShell::exec <Token::audio, Token::set, Token::sampling>, 1);

    root.add({"audio", "set", "filter"},
             "Configures the audio filter",
             &RetroShell::exec <Token::audio, Token::set, Token::filter>, 1);
    
    root.add({"audio", "set", "volume"},
             "Sets the volume");

    root.add({"audio", "set", "volume", "channel0"},
             "Sets the volume for audio channel 0",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 0);
    
    root.add({"audio", "set", "volume", "channel1"},
             "Sets the volume for audio channel 1",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 1);
    
    root.add({"audio", "set", "volume", "channel2"},
             "Sets the volume for audio channel 2",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 2);
    
    root.add({"audio", "set", "volume", "channel3"},
             "Sets the volume for audio channel 3",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 3);
    
    root.add({"audio", "set", "volume", "left"},
             "Sets the master volume for the left speaker",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 4);
    
    root.add({"audio", "set", "volume", "right"},
             "Sets the master volume for the right speaker",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 5);

    root.add({"audio", "set", "pan"},
             "Sets the pan for one of the four audio channels");
    
    root.add({"audio", "set", "pan", "channel0"},
             "Sets the pan for audio channel 0",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 1, 0);
    
    root.add({"audio", "set", "pan", "channel1"},
             "Sets the pan for audio channel 1",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 1, 1);
    
    root.add({"audio", "set", "pan", "channel2"},
             "Sets the pan for audio channel 2",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 1, 2);
    
    root.add({"audio", "set", "pan", "channel3"},
             "Sets the pan for audio channel 3",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 1, 3);

    root.add({"audio", "inspect"},
             "Displays the internal state");

    root.add({"audio", "inspect", "state"},
             "Displays the current state",
             &RetroShell::exec <Token::audio, Token::inspect, Token::state>, 0);

    root.add({"audio", "inspect", "registers"},
             "Displays the current register value",
             &RetroShell::exec <Token::audio, Token::inspect, Token::registers>, 0);
    
    
    //
    // Paula
    //

    /*
    root.add({"paula", "inspect"},
             "Displays the internal state");

    root.add({"paula", "inspect", "state"},
             "Displays the current register value",
             &RetroShell::exec <Token::paula, Token::inspect, Token::state>, 0);

    root.add({"paula", "inspect", "registers"},
             "Displays the current register value",
             &RetroShell::exec <Token::paula, Token::inspect, Token::registers>, 0);
    */


    //
    // RTC
    //

    root.newGroup("");

    root.add({"rtc", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::rtc, Token::config>, 0);

    root.add({"rtc", "set"},
             "Configures the component");

    root.add({"rtc", "set", "revision"},
             "Selects the emulated chip model",
             &RetroShell::exec <Token::rtc, Token::set, Token::revision>, 1);

    root.add({"rtc", "inspect"},
             "Displays the internal state");

    root.add({"rtc", "inspect", "registers"},
             "Displays the current register value",
             &RetroShell::exec <Token::rtc, Token::inspect, Token::registers>, 0);

    
    //
    // Control port
    //

    root.newGroup("");

    for (isize i = 0; i < 2; i++) {

        string port = (i == 0) ? "controlport1" : "controlport2";
        
        root.add({port, ""},
                 "Displays the current configuration",
                 &RetroShell::exec <Token::controlport, Token::config>, 0, i);
        
        root.add({port, "inspect"},
                 "Displays the internal state",
                 &RetroShell::exec <Token::controlport, Token::inspect>, 0, i);
    }


    //
    // Keyboard
    //

    root.newGroup("");

    root.add({"keyboard", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::keyboard, Token::config>, 0);
    
    root.add({"keyboard", "set"},
             "Configures the component");

    root.add({"keyboard", "set", "accuracy"},
             "Determines the emulation accuracy level",
             &RetroShell::exec <Token::keyboard, Token::set, Token::accuracy>, 1);

    root.add({"keyboard", "inspect"},
             "Displays the internal state",
             &RetroShell::exec <Token::keyboard, Token::inspect>, 0);

    root.add({"keyboard", "press"},
             "Sends a keycode to the keyboard",
             &RetroShell::exec <Token::keyboard, Token::press>, 1);

    
    //
    // Mouse
    //

    root.newGroup("");

    for (isize i = 0; i < 2; i++) {

        string mouse = (i == 0) ? "mouse1" : "mouse2";
        
        root.add({mouse, ""},
                 "Displays the current configuration",
                 &RetroShell::exec <Token::mouse, Token::config>, 0, i);
        
        root.add({mouse, "set"},
                 "Configures the component");
        
        root.add({mouse, "set", "pullup"},
                 "Enables or disables the emulation of pull-up resistors",
                 &RetroShell::exec <Token::mouse, Token::set, Token::pullup>, 1, i);
        
        root.add({mouse, "set", "shakedetector"},
                 "Enables or disables the shake detector",
                 &RetroShell::exec <Token::mouse, Token::set, Token::shakedetector>, 1, i);
        
        root.add({mouse, "set", "velocity"},
                 "Sets the horizontal and vertical mouse velocity",
                 &RetroShell::exec <Token::mouse, Token::set, Token::velocity>, 1, i);
        
        root.add({mouse, "inspect"},
                 "Displays the internal state",
                 &RetroShell::exec <Token::mouse, Token::inspect>, 0, i);

        root.add({mouse, "press"},
                 "Presses a mouse button");

        root.add({mouse, "press", "left"},
                 "Presses the left mouse button",
                 &RetroShell::exec <Token::mouse, Token::press, Token::left>, 0, i);
        
        root.add({mouse, "press", "right"},
                 "Presses the right mouse button",
                 &RetroShell::exec <Token::mouse, Token::press, Token::right>, 0, i);
    }

    
    //
    // Joystick
    //

    root.newGroup("");

    for (isize i = 0; i < 2; i++) {

        string joystick = (i == 0) ? "joystick1" : "joystick2";
        
        root.add({joystick, ""},
                 "Displays the current configuration",
                 &RetroShell::exec <Token::joystick, Token::config>, 0, i);
        
        root.add({joystick, "set"},
                 "Configures the component");
        
        root.add({joystick, "set", "autofire"},
                 "Enables or disables auto-fire mode",
                 &RetroShell::exec <Token::joystick, Token::set, Token::autofire>, 1, i);
        
        root.add({joystick, "set", "bullets"},
                 "Sets the number of bullets per auto-fire shot",
                 &RetroShell::exec <Token::joystick, Token::set, Token::bullets>, 1, i);
        
        root.add({joystick, "set", "velocity"},
                 "Configures the auto-fire delay",
                 &RetroShell::exec <Token::joystick, Token::set, Token::delay>, 1, i);
        
        root.add({joystick, "inspect"},
                 "Displays the internal state",
                 &RetroShell::exec <Token::joystick, Token::inspect>, 0, i);

        root.add({joystick, "press"},
                 "Presses a joystick button",
                 &RetroShell::exec <Token::joystick, Token::press>, 1, i);

        root.add({joystick, "unpress"},
                 "Releases a joystick button",
                 &RetroShell::exec <Token::joystick, Token::unpress>, 1, i);

        root.add({joystick, "pull"},
                 "Pulls the joystick");

        root.add({joystick, "pull", "left"},
                 "Pulls the joystick left",
                 &RetroShell::exec <Token::joystick, Token::pull, Token::left>, 0, i);
        
        root.add({joystick, "pull", "right"},
                 "Pulls the joystick right",
                 &RetroShell::exec <Token::joystick, Token::pull, Token::right>, 0, i);

        root.add({joystick, "pull", "up"},
                 "Pulls the joystick up",
                 &RetroShell::exec <Token::joystick, Token::pull, Token::up>, 0, i);

        root.add({joystick, "pull", "down"},
                 "Pulls the joystick down",
                 &RetroShell::exec <Token::joystick, Token::pull, Token::down>, 0, i);

        root.add({joystick, "release"},
                 "Release a joystick axis");

        root.add({joystick, "release", "x"},
                 "Releases the x-axis",
                 &RetroShell::exec <Token::joystick, Token::release, Token::xaxis>, 0, i);

        root.add({joystick, "release", "y"},
                 "Releases the y-axis",
                 &RetroShell::exec <Token::joystick, Token::release, Token::yaxis>, 0, i);
    }
    
    
    //
    // Serial port
    //

    root.newGroup("");

    root.add({"serial", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::serial, Token::config>, 0);

    root.add({"serial", "set"},
             "Configures the component");

    root.add({"serial", "set", "device"},
             "",
             &RetroShell::exec <Token::serial, Token::set, Token::device>, 1);

    root.add({"serial", "inspect"},
             "Displays the internal state",
             &RetroShell::exec <Token::serial, Token::inspect>, 0);

    
    //
    // Disk controller
    //

    root.newGroup("");

    root.add({"diskcontroller", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::dc, Token::config>, 0);

    root.add({"diskcontroller", "set"},
             "Configures the component");

    root.add({"diskcontroller", "set", "speed"},
             "Configures the drive speed",
             &RetroShell::exec <Token::dc, Token::speed>, 1);

    root.add({"diskcontroller", "dsksync"},
             "Secures the DSKSYNC register");

    root.add({"diskcontroller", "dsksync", "auto"},
             "Always receive a SYNC signal",
             &RetroShell::exec <Token::dc, Token::dsksync, Token::autosync>, 1);

    root.add({"diskcontroller", "dsksync", "lock"},
             "Prevents writes to DSKSYNC",
             &RetroShell::exec <Token::dc, Token::dsksync, Token::lock>, 1);

    root.add({"diskcontroller", "inspect"},
             "Displays the internal state",
             &RetroShell::exec <Token::dc, Token::inspect>, 0);


    //
    // Df0, Df1, Df2, Df3
    //

    root.newGroup("");

    for (isize i = 0; i < 4; i++) {

        string df = "df" + std::to_string(i);

        root.add({df, ""},
                 "Displays the current configuration",
                 &RetroShell::exec <Token::dfn, Token::config>, 0, i);
        
        root.add({df, "connect"},
                 "Connects the drive",
                 &RetroShell::exec <Token::dfn, Token::connect>, 0, i);
        root.seek("df0")->remove("connect");
        
        root.add({df, "disconnect"},
                 "Disconnects the drive",
                 &RetroShell::exec <Token::dfn, Token::disconnect>, 0, i);
        root.seek("df0")->remove("disconnect");
        
        root.add({df, "eject"},
                 "Ejects a floppy disk",
                 &RetroShell::exec <Token::dfn, Token::eject>, 0, i);
        
        root.add({df, "insert"},
                 "Inserts a floppy disk",
                 &RetroShell::exec <Token::dfn, Token::insert>, 1, i);
    }
    
    for (isize i = 0; i < 5; i++) {

        string df = i < 4 ? "df" + std::to_string(i) : "dfn";

        root.add({df, "set"},
                 "Configures the component");
        
        root.add({df, "set", "model"},
                 "Selects the drive model",
                 &RetroShell::exec <Token::dfn, Token::set, Token::model>, 1, i);
        
        root.add({df, "set", "mechanics"},
                 "Enables or disables the emulation of mechanical delays",
                 &RetroShell::exec <Token::dfn, Token::set, Token::mechanics>, 1, i);
        
        root.add({df, "set", "searchpath"},
                 "Sets the search path for media files",
                 &RetroShell::exec <Token::dfn, Token::set, Token::searchpath>, 1, i);

        root.add({df, "set", "swapdelay"},
                 "Sets the disk change delay",
                 &RetroShell::exec <Token::dfn, Token::set, Token::swapdelay>, 1, i);

        root.add({df, "set", "pan"},
                 "Sets the pan for drive sounds",
                 &RetroShell::exec <Token::dfn, Token::set, Token::pan>, 1, i);
        
        root.add({df, "audiate"},
                 "Sets the volume of drive sounds");
        
        root.add({df, "audiate", "insert"},
                 "Makes disk insertions audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::insert>, 1, i);
        
        root.add({df, "audiate", "eject"},
                 "Makes disk ejections audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::eject>, 1, i);
        
        root.add({df, "audiate", "step"},
                 "Makes head steps audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::step>, 1, i);
        
        root.add({df, "audiate", "poll"},
                 "Makes polling clicks audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::poll>, 1, i);
    }
    
    for (isize i = 0; i < 4; i++) {

        string df = "df" + std::to_string(i);
        
        root.add({df, "inspect"},
                 "Displays the component state",
                 &RetroShell::exec <Token::dfn, Token::inspect>, 0);

        root.add({df, "catch"},
                 "Creates a catchpoint for the specfied file",
                 &RetroShell::exec <Token::dfn, Token::cp>, 1);
    }

    //
    // Hd0, Hd1, Hd2, Hd3
    //

    root.newGroup("");

    for (isize i = 0; i < 4; i++) {
        
        string hd = "hd" + std::to_string(i);

        root.add({hd, ""},
                 "Displays the current configuration",
                 &RetroShell::exec <Token::hdn, Token::config>, 0, i);
    }
    
    for (isize i = 0; i < 5; i++) {

        string hd = i < 4 ? "hd" + std::to_string(i) : "hdn";

        root.add({hd, "set"},
                 "Configures the component");
        
        root.add({hd, "set", "pan"},
                 "Sets the pan for drive sounds",
                 &RetroShell::exec <Token::hdn, Token::set, Token::pan>, 1, i);
        
        root.add({hd, "audiate"},
                 "Sets the volume of drive sounds");

        root.add({hd, "audiate", "step"},
                 "Makes head steps audible",
                 &RetroShell::exec <Token::hdn, Token::audiate, Token::step>, 1, i);
    }
    
    for (isize i = 0; i < 4; i++) {

        string hd = "hd" + std::to_string(i);
        
        root.add({hd, "inspect"},
                 "Displays the component state");

        root.add({hd, "inspect", "drive"},
                 "Displays hard drive parameters",
                 &RetroShell::exec <Token::hdn, Token::inspect, Token::drive>, 0, i);

        root.add({hd, "inspect", "volumes"},
                 "Displays summarized volume information",
                 &RetroShell::exec <Token::hdn, Token::inspect, Token::volumes>, 0, i);

        root.add({hd, "inspect", "partitions"},
                 "Displays information about all partitions",
                 &RetroShell::exec <Token::hdn, Token::inspect, Token::partition>, 0, i);

        root.add({hd, "inspect", "state"},
                 "Displays the internal state",
                 &RetroShell::exec <Token::hdn, Token::inspect, Token::state>, 0, i);

        root.add({hd, "geometry"},
                 "Changes the disk geometry",
                 &RetroShell::exec <Token::hdn, Token::geometry>, 3, i);
    }
    
    //
    // Zorro boards
    //

    root.add({"zorro", "list"},
             "Lists all connected boards",
             &RetroShell::exec <Token::zorro, Token::list>, 0);

    root.add({"zorro", "inspect"},
             "Inspects a specific Zorro board",
             &RetroShell::exec <Token::zorro, Token::inspect>, 1);

    
    //
    // Remote server
    //

    root.newGroup("");

    root.add({"server", ""},
             "Displays a server status summary",
             &RetroShell::exec <Token::server>, 0);

    root.add({"server", "serial"},
             "Serial port server");

    root.add({"server", "serial", "config"},
             "Displays the current configuration",
             &RetroShell::exec <Token::server, Token::serial, Token::config>, 0);

    root.add({"server", "serial", "set"},
             "Configures the component");

    root.add({"server", "serial", "set", "port"},
             "Assigns the port number",
             &RetroShell::exec <Token::server, Token::serial, Token::set, Token::port>, 1);

    root.add({"server", "serial", "set", "verbose"},
             "Switches verbose mode on or off",
             &RetroShell::exec <Token::server, Token::serial, Token::set, Token::verbose>, 1);

    root.add({"server", "serial", "inspect"},
             "Displays the internal state",
             &RetroShell::exec <Token::server, Token::serial, Token::inspect>, 0);

    root.add({"server", "rshell"},
             "Retro shell server");

    root.add({"server", "rshell", "start"},
             "Starts the retro shell server",
             &RetroShell::exec <Token::server, Token::rshell, Token::start>, 0);

    root.add({"server", "rshell", "stop"},
             "Stops the retro shell server",
             &RetroShell::exec <Token::server, Token::rshell, Token::stop>, 0);

    root.add({"server", "rshell", "disconnect"},
             "Disconnects a client",
             &RetroShell::exec <Token::server, Token::rshell, Token::disconnect>, 0);

    root.add({"server", "rshell", "config"},
             "Displays the current configuration",
             &RetroShell::exec <Token::server, Token::rshell, Token::config>, 0);

    root.add({"server", "rshell", "set"},
             "Configures the component");

    root.add({"server", "rshell", "set", "port"},
             "Assigns the port number",
             &RetroShell::exec <Token::server, Token::rshell, Token::set, Token::port>, 1);

    root.add({"server", "serial", "set", "verbose"},
             "Switches verbose mode on or off",
             &RetroShell::exec <Token::server, Token::rshell, Token::set, Token::verbose>, 1);

    root.add({"server", "rshell", "inspect"},
             "Displays the internal state",
             &RetroShell::exec <Token::server, Token::rshell, Token::inspect>, 0, 0);

    root.add({"server", "gdb"},
             "GDB server");

    root.add({"server", "gdb", "attach"},
             "Attaches the GDB server to a process",
             &RetroShell::exec <Token::server, Token::gdb, Token::attach>, 1);

    root.add({"server", "gdb", "detach"},
             "Detaches the GDB server from a process",
             &RetroShell::exec <Token::server, Token::gdb, Token::detach>, 0);

    root.add({"server", "gdb", "config"},
             "Displays the current configuration",
             &RetroShell::exec <Token::server, Token::gdb, Token::config>, 0);

    root.add({"server", "gdb", "set"},
             "Configures the component");

    root.add({"server", "gdb", "set", "port"},
             "Assigns the port number",
             &RetroShell::exec <Token::server, Token::gdb, Token::set, Token::port>, 1);

    root.add({"server", "gdb", "set", "verbose"},
             "Switches verbose mode on or off",
             &RetroShell::exec <Token::server, Token::gdb, Token::set, Token::verbose>, 1);

    root.add({"server", "gdb", "inspect"},
             "Displays the internal state",
             &RetroShell::exec <Token::server, Token::gdb, Token::inspect>, 0);
}

}
