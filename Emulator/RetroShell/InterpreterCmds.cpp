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

void
Interpreter::registerInstructions()
{
    //
    // Commands
    //
    
    root.add({"clear"},
             "command", "Clears the console window",
             &RetroShell::exec <Token::clear>, 0);
    root.seek("clear")->hidden = true;

    root.add({"close"},
             "command", "Hides the debug console",
             &RetroShell::exec <Token::close>, 0);
    root.seek("close")->hidden = true;

    root.add({"help"},
             "command", "Prints usage information",
             &RetroShell::exec <Token::help>, {0, 1});
    root.seek("help")->hidden = true;

    root.add({"joshua"},
             "command", "",
             &RetroShell::exec <Token::easteregg>, 0);
    root.seek("joshua")->hidden = true;
    
    root.add({"source"},
             "command", "Processes a command script",
             &RetroShell::exec <Token::source>, 1);
    
    root.add({"wait"},
             "command", "Pauses the execution of a command script",
             &RetroShell::exec <Token::wait>, 2);

    
    //
    // Regression testing
    //
    
    root.add({"regression"},
             "component", "");
    root.seek("regression")->hidden = true;

    root.add({"regression", "setup"},
             "command", "Initializes the test environment",
             &RetroShell::exec <Token::regression, Token::setup>, 2);

    root.add({"regression", "run"},
             "command", "Launches a regression test",
             &RetroShell::exec <Token::regression, Token::run>, 1);
    
    root.add({"screenshot"},
             "component", "Manages regression tests");
    root.seek("screenshot")->hidden = true;
    
    root.add({"screenshot", "set"},
             "command", "Configures the regression test");
        
    root.add({"screenshot", "set", "filename"},
             "key", "Assigns the screen shot filename",
             &RetroShell::exec <Token::screenshot, Token::set, Token::filename>, 1);

    root.add({"screenshot", "set", "cutout"},
             "key", "Adjusts the texture cutout",
             &RetroShell::exec <Token::screenshot, Token::set, Token::cutout>, 4);

    root.add({"screenshot", "save"},
             "key", "Saves a screenshot and exits the emulator",
             &RetroShell::exec <Token::screenshot, Token::save>, 1);

    
    //
    // Amiga
    //
    
    root.add({"amiga"},
             "component", "The virtual Amiga");
        
    root.add({"amiga", "init"},
             "command", "Initializes the Amiga with a predefined scheme",
             &RetroShell::exec <Token::amiga, Token::init>, 1);

    root.add({"amiga", "power"},
             "command", "Switches the Amiga on or off");
    
    root.add({"amiga", "power", "on"},
             "state", "Switches the Amiga on",
             &RetroShell::exec <Token::amiga, Token::power, Token::on>, 0);

    root.add({"amiga", "power", "off"},
             "state", "Switches the Amiga off",
             &RetroShell::exec <Token::amiga, Token::power, Token::off>, 0);

    root.add({"amiga", "debug"},
             "command", "Switches debug mode on or off");
    
    root.add({"amiga", "debug", "on"},
             "state", "Switches debug mode on",
             &RetroShell::exec <Token::amiga, Token::debug, Token::on>, 0);

    root.add({"amiga", "debug", "off"},
             "state", "Switches debug mode off",
             &RetroShell::exec <Token::amiga, Token::debug, Token::off>, 0);

    root.add({"amiga", "run"},
             "command", "Starts the emulator thread",
             &RetroShell::exec <Token::amiga, Token::run>, 0);
    
    root.add({"amiga", "pause"},
             "command", "Halts the emulator thread",
             &RetroShell::exec <Token::amiga, Token::pause>, 0);
    
    root.add({"amiga", "reset"},
             "command", "Performs a hard reset",
             &RetroShell::exec <Token::amiga, Token::reset>, 0);
    
    root.add({"amiga", "inspect"},
             "command", "Displays the component state",
             &RetroShell::exec <Token::amiga, Token::inspect>, 0);

    
    //
    // Memory
    //
    
    root.add({"memory"},
             "component", "Ram and Rom");
    
    root.add({"memory", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::memory, Token::config>, 0);

    root.add({"memory", "set"},
             "command", "Configures the component");
        
    root.add({"memory", "set", "chip"},
             "key", "Configures the amouts of chip memory",
             &RetroShell::exec <Token::memory, Token::set, Token::chip>, 1);

    root.add({"memory", "set", "slow"},
             "key", "Configures the amouts of slow memory",
             &RetroShell::exec <Token::memory, Token::set, Token::slow>, 1);

    root.add({"memory", "set", "fast"},
             "key", "Configures the amouts of flow memory",
             &RetroShell::exec <Token::memory, Token::set, Token::fast>, 1);

    root.add({"memory", "set", "extstart"},
             "key", "Sets the start address for Rom extensions",
             &RetroShell::exec <Token::memory, Token::set, Token::extstart>, 1);

    root.add({"memory", "set", "saveroms"},
             "key", "Determines whether Roms should be stored in snapshots",
             &RetroShell::exec <Token::memory, Token::set, Token::saveroms>, 1);

    root.add({"memory", "set", "slowramdelay"},
             "key", "Enables or disables slow Ram bus delays",
             &RetroShell::exec <Token::memory, Token::set, Token::slowramdelay>, 1);

    root.add({"memory", "set", "bankmap"},
             "key", "Selects the bank mapping scheme",
             &RetroShell::exec <Token::memory, Token::set, Token::bankmap>, 1);

    root.add({"memory", "set", "unmapped"},
             "key", "Determines the behaviour of unmapped memory",
             &RetroShell::exec <Token::memory, Token::set, Token::unmappingtype>, 1);

    root.add({"memory", "set", "raminit"},
             "key", "Determines how Ram is initialized on startup",
             &RetroShell::exec <Token::memory, Token::set, Token::raminitpattern>, 1);
    
    root.add({"memory", "load"},
             "command", "Installs a Rom image");
            
    root.add({"memory", "load", "rom"},
             "command", "Installs a Kickstart Rom",
             &RetroShell::exec <Token::memory, Token::load, Token::rom>, 1);

    root.add({"memory", "load", "extrom"},
             "command", "Installs a Rom extension",
             &RetroShell::exec <Token::memory, Token::load, Token::extrom>, 1);

    root.add({"memory", "inspect"},
             "command", "Displays the component state");

    root.add({"memory", "inspect", "state"},
             "command", "Displays the current state",
             &RetroShell::exec <Token::memory, Token::inspect, Token::state>, 0);

    root.add({"memory", "inspect", "bankmap"},
             "command", "Displays the bank map",
             &RetroShell::exec <Token::memory, Token::inspect, Token::bankmap>, 0);

    root.add({"memory", "inspect", "checksum"},
             "command", "Computes memory checksums",
             &RetroShell::exec <Token::memory, Token::inspect, Token::checksums>, 0);

    
    //
    // CPU
    //
    
    root.add({"cpu"},
             "component", "Motorola 68k CPU");
    
    root.add({"cpu", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::cpu, Token::config>, 0);

    root.add({"cpu", "set"},
             "command", "Configures the component");
    
    root.add({"cpu", "set", "regreset"},
             "key", "Selects the reset value of data and address registers",
             &RetroShell::exec <Token::cpu, Token::set, Token::regreset>, 1);

    root.add({"cpu", "inspect"},
             "command", "Displays the component state");

    root.add({"cpu", "inspect", "state"},
             "command", "Displays the current state",
             &RetroShell::exec <Token::cpu, Token::inspect, Token::state>, 0);

    root.add({"cpu", "inspect", "registers"},
             "command", "Displays the current register values",
             &RetroShell::exec <Token::cpu, Token::inspect, Token::registers>, 0);

    root.add({"cpu", "jump"},
             "command", "Jumps to the specified address",
             &RetroShell::exec <Token::cpu, Token::jump>, 1);

    
    //
    // CIA
    //
    
    root.add({"ciaa"},
             "component", "Complex Interface Adapter A");

    root.add({"ciab"},
             "component", "Complex Interface Adapter B");

    for (isize i = 0; i < 2; i++) {
        
        string cia = (i == 0) ? "ciaa" : "ciab";
        
        root.add({cia, "config"},
                 "command", "Displays the current configuration",
                 &RetroShell::exec <Token::cia, Token::config>, 0, i);
        
        root.add({cia, "set"},
                 "command", "Configures the component");
        
        root.add({cia, "set", "revision"},
                 "key", "Selects the emulated chip model",
                 &RetroShell::exec <Token::cia, Token::set, Token::revision>, 1, i);
        
        root.add({cia, "set", "todbug"},
                 "key", "Enables or disables the TOD hardware bug",
                 &RetroShell::exec <Token::cia, Token::set, Token::todbug>, 1, i);
        
        root.add({cia, "set", "esync"},
                 "key", "Turns E-clock syncing on or off",
                 &RetroShell::exec <Token::cia, Token::set, Token::esync>, 1, i);
        
        root.add({cia, "inspect"},
                 "command", "Displays the component state", 0, i);
        
        root.add({cia, "inspect", "state"},
                 "category", "Displays the current state",
                 &RetroShell::exec <Token::cia, Token::inspect, Token::state>, 0, i);
        
        root.add({cia, "inspect", "registers"},
                 "category", "Displays the current register values",
                 &RetroShell::exec <Token::cia, Token::inspect, Token::registers>, 0, i);
        
        root.add({cia, "inspect", "tod"},
                 "category", "Displays the state of the 24-bit counter",
                 &RetroShell::exec <Token::cia, Token::inspect, Token::tod>, 0, i);
    }
    
    
    //
    // Agnus
    //

    root.add({"agnus"},
             "component", "Custom chip");

    root.add({"agnus", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::agnus, Token::config>, 0);
    
    root.add({"agnus", "set"},
             "command", "Configures the component");
        
    root.add({"agnus", "set", "revision"},
             "key", "Selects the emulated chip model",
             &RetroShell::exec <Token::agnus, Token::set, Token::revision>, 1);

    root.add({"agnus", "set", "slowrammirror"},
             "key", "Enables or disables ECS Slow Ram mirroring",
             &RetroShell::exec <Token::agnus, Token::set, Token::slowrammirror>, 1);

    root.add({"agnus", "inspect"},
             "command", "Displays the internal state");

    root.add({"agnus", "inspect", "state"},
             "category", "Displays the current state",
             &RetroShell::exec <Token::agnus, Token::inspect, Token::state>, 0);

    root.add({"agnus", "inspect", "registers"},
             "category", "Displays the current register values",
             &RetroShell::exec <Token::agnus, Token::inspect, Token::registers>, 0);

    root.add({"agnus", "inspect", "dma"},
             "category", "Displays the scheduled DMA transfers",
             &RetroShell::exec <Token::agnus, Token::inspect, Token::dma>, 0);

    root.add({"agnus", "inspect", "events"},
             "category", "Displays all scheduled events",
             &RetroShell::exec <Token::agnus, Token::inspect, Token::events>, 0);
    
    
    //
    // Blitter
    //
    
    root.add({"blitter"},
             "component", "Custom Chip (Agnus)");
    
    root.add({"blitter", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::blitter, Token::config>, 0);
    
    root.add({"blitter", "set"},
             "command", "Configures the component");
        
    root.add({"blitter", "set", "accuracy"},
             "level", "Selects the emulation accuracy level",
             &RetroShell::exec <Token::blitter, Token::set, Token::accuracy>, 1);

    root.add({"blitter", "inspect"},
             "command", "Displays the internal state");

    root.add({"blitter", "inspect", "state"},
             "category", "Displays the internal state",
             &RetroShell::exec <Token::blitter, Token::inspect, Token::state>, 0);

    root.add({"blitter", "inspect", "registers"},
             "category", "Displays the current register value",
             &RetroShell::exec <Token::blitter, Token::inspect, Token::registers>, 0);

    
    //
    // Copper
    //
    
    root.add({"copper"},
             "component", "Custom Chip (Agnus)");
    
    root.add({"copper", "inspect"},
             "command", "Displays the internal state");

    root.add({"copper", "inspect", "state"},
             "category", "Displays the current state",
             &RetroShell::exec <Token::copper, Token::inspect, Token::state>, 0);

    root.add({"copper", "inspect", "registers"},
             "category", "Displays the current register value",
             &RetroShell::exec <Token::copper, Token::inspect, Token::registers>, 0);

    root.add({"copper", "list"},
             "command", "Disassembles a Copper list",
             &RetroShell::exec <Token::copper, Token::list>, 1);

    
    //
    // Denise
    //
    
    root.add({"denise"},
             "component", "Custom chip");
    
        root.add({"denise", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::denise, Token::config>, 0);

    root.add({"denise", "set"},
             "command", "Configures the component");

    root.add({"denise", "set", "revision"},
             "key", "Selects the emulated chip model",
             &RetroShell::exec <Token::denise, Token::set, Token::revision>, 1);

    root.add({"denise", "set", "tracking"},
             "key", "Enables or disables viewport tracking",
             &RetroShell::exec <Token::denise, Token::set, Token::tracking>, 1);

    root.add({"denise", "set", "clxsprspr"},
             "key", "Enables or disables sprite-sprite collision detection",
             &RetroShell::exec <Token::denise, Token::set, Token::clxsprspr>, 1);

    root.add({"denise", "set", "clxsprplf"},
             "key", "Enables or disables sprite-playfield collision detection",
             &RetroShell::exec <Token::denise, Token::set, Token::clxsprplf>, 1);

    root.add({"denise", "set", "clxplfplf"},
             "key", "Enables or disables playfield-playfield collision detection",
             &RetroShell::exec <Token::denise, Token::set, Token::clxplfplf>, 1);
    
    root.add({"denise", "hide"},
             "command", "Hides bitplanes, sprites, or layers");

    root.add({"denise", "hide", "bitplanes"},
             "key", "Wipes out certain bitplane data",
             &RetroShell::exec <Token::denise, Token::hide, Token::bitplanes>, 1);

    root.add({"denise", "hide", "sprites"},
             "key", "Wipes out certain sprite data",
             &RetroShell::exec <Token::denise, Token::hide, Token::sprites>, 1);

    root.add({"denise", "hide", "sprites"},
             "key", "Makes certain drawing layers transparent",
             &RetroShell::exec <Token::denise, Token::hide, Token::layers>, 1);

    root.add({"denise", "inspect"},
             "command", "Displays the internal state");

    root.add({"denise", "inspect", "state"},
             "category", "Displays the current state",
             &RetroShell::exec <Token::denise, Token::inspect, Token::state>, 0);

    root.add({"denise", "inspect", "registers"},
             "category", "Displays the current register value",
             &RetroShell::exec <Token::denise, Token::inspect, Token::registers>, 0);

    
    //
    // DMA Debugger
    //

    root.add({"dmadebugger"},
             "component", "DMA Debugger");

    root.add({"dmadebugger", "open"},
             "command", "Opens the DMA debugger",
             &RetroShell::exec <Token::dmadebugger, Token::open>, 0);

    root.add({"dmadebugger", "close"},
             "command", "Closes the DMA debugger",
             &RetroShell::exec <Token::dmadebugger, Token::close>, 0);

    root.add({"dmadebugger", "show"},
             "command", "Enables the debugger for a certain DMA channel");

    root.add({"dmadebugger", "show", "copper"},
             "command", "Visualizes Copper DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::copper>, 0);

    root.add({"dmadebugger", "show", "blitter"},
             "command", "Visualizes Blitter DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::blitter>, 0);

    root.add({"dmadebugger", "show", "disk"},
             "command", "Visualizes Disk DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::disk>, 0);

    root.add({"dmadebugger", "show", "audio"},
             "command", "Visualizes Audio DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::audio>, 0);

    root.add({"dmadebugger", "show", "sprites"},
             "command", "Visualizes Sprite DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::sprites>, 0);

    root.add({"dmadebugger", "show", "bitplanes"},
             "command", "Visualizes Bitplane DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::bitplanes>, 0);

    root.add({"dmadebugger", "show", "cpu"},
             "command", "Visualizes CPU accesses",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::cpu>, 0);

    root.add({"dmadebugger", "show", "refresh"},
             "command", "Visualizes memory refresh cycles",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::refresh>, 0);

    root.add({"dmadebugger", "hide"},
             "command", "Disables the debugger for a certain DMA channel");

    root.add({"dmadebugger", "hide", "copper"},
             "command", "Hides Copper DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::copper>, 0);

    root.add({"dmadebugger", "hide", "blitter"},
             "command", "Hides Blitter DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::blitter>, 0);

    root.add({"dmadebugger", "hide", "disk"},
             "command", "Hides Disk DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::disk>, 0);

    root.add({"dmadebugger", "hide", "audio"},
             "command", "Hides Audio DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::audio>, 0);

    root.add({"dmadebugger", "hide", "sprites"},
             "command", "Hides Sprite DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::sprites>, 0);

    root.add({"dmadebugger", "hide", "bitplanes"},
             "command", "Hides Bitplane DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::bitplanes>, 0);

    root.add({"dmadebugger", "hide", "cpu"},
             "command", "Hides CPU accesses",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::cpu>, 0);

    root.add({"dmadebugger", "hide", "refresh"},
             "command", "Hides memory refresh cycles",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::refresh>, 0);

    
    //
    // Monitor
    //

    root.add({"monitor"},
             "component", "Amiga monitor");

    root.add({"monitor", "set"},
             "command", "Configures the component");

    root.add({"monitor", "set", "palette"},
             "key", "Selects the color palette",
             &RetroShell::exec <Token::monitor, Token::set, Token::palette>, 1);

    root.add({"monitor", "set", "brightness"},
             "key", "Adjusts the brightness of the Amiga texture",
             &RetroShell::exec <Token::monitor, Token::set, Token::brightness>, 1);

    root.add({"monitor", "set", "contrast"},
             "key", "Adjusts the contrast of the Amiga texture",
             &RetroShell::exec <Token::monitor, Token::set, Token::contrast>, 1);

    root.add({"monitor", "set", "saturation"},
             "key", "Adjusts the saturation of the Amiga texture",
             &RetroShell::exec <Token::monitor, Token::set, Token::saturation>, 1);

    
    //
    // Audio
    //
    
    root.add({"audio"},
             "component", "Audio Unit (Paula)");
    
    root.add({"audio", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::audio, Token::config>, 0);

    root.add({"audio", "set"},
             "command", "Configures the component");

    root.add({"audio", "set", "sampling"},
             "key", "Selects the sampling method",
             &RetroShell::exec <Token::audio, Token::set, Token::sampling>, 1);

    root.add({"audio", "set", "filter"},
             "key", "Configures the audio filter",
             &RetroShell::exec <Token::audio, Token::set, Token::filter>, 1);
    
    root.add({"audio", "set", "volume"},
             "key", "Sets the volume");

    root.add({"audio", "set", "volume", "channel0"},
             "key", "Sets the volume for audio channel 0",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 0);
    
    root.add({"audio", "set", "volume", "channel1"},
             "key", "Sets the volume for audio channel 1",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 1);
    
    root.add({"audio", "set", "volume", "channel2"},
             "key", "Sets the volume for audio channel 2",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 2);
    
    root.add({"audio", "set", "volume", "channel3"},
             "key", "Sets the volume for audio channel 3",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 3);
    
    root.add({"audio", "set", "volume", "left"},
             "key", "Sets the master volume for the left speaker",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 4);
    
    root.add({"audio", "set", "volume", "right"},
             "key", "Sets the master volume for the right speaker",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1, 5);

    root.add({"audio", "set", "pan"},
             "key", "Sets the pan for one of the four audio channels");
    
    root.add({"audio", "set", "pan", "channel0"},
             "key", "Sets the pan for audio channel 0",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 1, 0);
    
    root.add({"audio", "set", "pan", "channel1"},
             "key", "Sets the pan for audio channel 1",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 1, 1);
    
    root.add({"audio", "set", "pan", "channel2"},
             "key", "Sets the pan for audio channel 2",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 1, 2);
    
    root.add({"audio", "set", "pan", "channel3"},
             "key", "Sets the pan for audio channel 3",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 1, 3);

    root.add({"audio", "inspect"},
             "command", "Displays the internal state");

    root.add({"audio", "inspect", "state"},
             "category", "Displays the current state",
             &RetroShell::exec <Token::audio, Token::inspect, Token::state>, 0);

    root.add({"audio", "inspect", "registers"},
             "category", "Displays the current register value",
             &RetroShell::exec <Token::audio, Token::inspect, Token::registers>, 0);
    
    
    //
    // Paula
    //
    
    root.add({"paula"},
             "component", "Custom chip");

    root.add({"paula", "inspect"},
             "command", "Displays the internal state");

    root.add({"paula", "inspect", "state"},
             "command", "Displays the current register value",
             &RetroShell::exec <Token::paula, Token::inspect, Token::state>, 0);

    root.add({"paula", "inspect", "registers"},
             "command", "Displays the current register value",
             &RetroShell::exec <Token::paula, Token::inspect, Token::registers>, 0);

    
    //
    // RTC
    //

    root.add({"rtc"},
             "component", "Real-time clock");

    root.add({"rtc", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::rtc, Token::config>, 0);

    root.add({"rtc", "set"},
             "command", "Configures the component");
        
    root.add({"rtc", "set", "revision"},
             "key", "Selects the emulated chip model",
             &RetroShell::exec <Token::rtc, Token::set, Token::revision>, 1);

    root.add({"rtc", "inspect"},
             "command", "Displays the internal state");

    root.add({"rtc", "inspect", "registers"},
             "command", "Displays the current register value",
             &RetroShell::exec <Token::rtc, Token::inspect, Token::registers>, 0);

    
    //
    // Control port
    //
    
    root.add({"controlport1"},
             "component", "Control port 1");
    
    root.add({"controlport2"},
             "component", "Control port 2");

    for (isize i = 0; i < 2; i++) {

        string port = (i == 0) ? "controlport1" : "controlport2";
        
        root.add({port, "config"},
                 "command", "Displays the current configuration",
                 &RetroShell::exec <Token::controlport, Token::config>, 0, i);
        
        root.add({port, "inspect"},
                 "command", "Displays the internal state",
                 &RetroShell::exec <Token::controlport, Token::inspect>, 0, i);
    }


    //
    // Keyboard
    //

    root.add({"keyboard"},
             "component", "Keyboard");

    root.add({"keyboard", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::keyboard, Token::config>, 0);
    
    root.add({"keyboard", "set"},
             "command", "Configures the component");
        
    root.add({"keyboard", "set", "accuracy"},
             "key", "Determines the emulation accuracy level",
             &RetroShell::exec <Token::keyboard, Token::set, Token::accuracy>, 1);

    root.add({"keyboard", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::keyboard, Token::inspect>, 0);

    root.add({"keyboard", "press"},
             "command", "Sends a keycode to the keyboard",
             &RetroShell::exec <Token::keyboard, Token::press>, 1);

    
    //
    // Mouse
    //

    root.add({"mouse1"},
             "component", "Port 1 mouse");

    root.add({"mouse2"},
             "component", "Port 2 mouse");

    for (isize i = 0; i < 2; i++) {

        string mouse = (i == 0) ? "mouse1" : "mouse2";
        
        root.add({mouse, "config"},
                 "command", "Displays the current configuration",
                 &RetroShell::exec <Token::mouse, Token::config>, 0, i);
        
        root.add({mouse, "set"},
                 "command", "Configures the component");
        
        root.add({mouse, "set", "pullup"},
                 "key", "Enables or disables the emulation of pull-up resistors",
                 &RetroShell::exec <Token::mouse, Token::set, Token::pullup>, 1, i);
        
        root.add({mouse, "set", "shakedetector"},
                 "key", "Enables or disables the shake detector",
                 &RetroShell::exec <Token::mouse, Token::set, Token::shakedetector>, 1, i);
        
        root.add({mouse, "set", "velocity"},
                 "key", "Sets the horizontal and vertical mouse velocity",
                 &RetroShell::exec <Token::mouse, Token::set, Token::velocity>, 1, i);
        
        root.add({mouse, "inspect"},
                 "command", "Displays the internal state",
                 &RetroShell::exec <Token::mouse, Token::inspect>, 0, i);

        root.add({mouse, "press"},
                 "command", "Presses a mouse button");

        root.add({mouse, "press", "left"},
                 "command", "Presses the left mouse button",
                 &RetroShell::exec <Token::mouse, Token::press, Token::left>, 0, i);
        
        root.add({mouse, "press", "right"},
                 "command", "Presses the right mouse button",
                 &RetroShell::exec <Token::mouse, Token::press, Token::right>, 0, i);
    }

    
    //
    // Serial port
    //
    
    root.add({"serial"},
             "component", "Serial port");

    root.add({"serial", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::serial, Token::config>, 0);

    root.add({"serial", "set"},
             "command", "Configures the component");
        
    root.add({"serial", "set", "device"},
             "key", "",
             &RetroShell::exec <Token::serial, Token::set, Token::device>, 1);

    root.add({"serial", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::serial, Token::inspect>, 0);

    
    //
    // Disk controller
    //
    
    root.add({"diskcontroller"},
             "component", "Disk Controller");

    root.add({"diskcontroller", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::dc, Token::config>, 0);

    root.add({"diskcontroller", "set"},
             "command", "Configures the component");
        
    root.add({"diskcontroller", "set", "speed"},
             "key", "Configures the drive speed",
             &RetroShell::exec <Token::dc, Token::speed>, 1);

    root.add({"diskcontroller", "dsksync"},
             "command", "Secures the DSKSYNC register");

    root.add({"diskcontroller", "dsksync", "auto"},
             "key", "Always receive a SYNC signal",
             &RetroShell::exec <Token::dc, Token::dsksync, Token::autosync>, 1);

    root.add({"diskcontroller", "dsksync", "lock"},
             "key", "Prevents writes to DSKSYNC",
             &RetroShell::exec <Token::dc, Token::dsksync, Token::lock>, 1);
        
    root.add({"diskcontroller", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::dc, Token::inspect>, 0);


    //
    // Df0, Df1, Df2, Df3
    //
    
    root.add({"df0"},
             "component", "Floppy drive 0");

    root.add({"df1"},
             "component", "Floppy drive 1");

    root.add({"df2"},
             "component", "Floppy drive 2");

    root.add({"df3"},
             "component", "Floppy drive 3");

    root.add({"dfn"},
             "component", "All connected drives");

    for (isize i = 0; i < 4; i++) {

        string df = "df" + std::to_string(i);
                
        root.add({df, "config"},
                 "command", "Displays the current configuration",
                 &RetroShell::exec <Token::dfn, Token::config>, 0, i);
        
        root.add({df, "connect"},
                 "command", "Connects the drive",
                 &RetroShell::exec <Token::dfn, Token::connect>, 0, i);
        root.seek("df0")->remove("connect");
        
        root.add({df, "disconnect"},
                 "command", "Disconnects the drive",
                 &RetroShell::exec <Token::dfn, Token::disconnect>, 0, i);
        root.seek("df0")->remove("disconnect");
        
        root.add({df, "eject"},
                 "command", "Ejects a floppy disk",
                 &RetroShell::exec <Token::dfn, Token::eject>, 0, i);
        
        root.add({df, "insert"},
                 "command", "Inserts a floppy disk",
                 &RetroShell::exec <Token::dfn, Token::insert>, 1, i);
    }
    
    for (isize i = 0; i < 5; i++) {

        string df = i < 4 ? "df" + std::to_string(i) : "dfn";
                
        root.add({df, "set"},
                 "command", "Configures the component");
        
        root.add({df, "set", "model"},
                 "key", "Selects the drive model",
                 &RetroShell::exec <Token::dfn, Token::set, Token::model>, 1, i);
        
        root.add({df, "set", "mechanics"},
                 "key", "Enables or disables the emulation of mechanical delays",
                 &RetroShell::exec <Token::dfn, Token::set, Token::mechanics>, 1, i);
        
        root.add({df, "set", "searchpath"},
                 "key", "Sets the search path for media files",
                 &RetroShell::exec <Token::dfn, Token::set, Token::searchpath>, 1, i);
                
        root.add({df, "set", "swapdelay"},
                 "key", "Sets the disk change delay",
                 &RetroShell::exec <Token::dfn, Token::set, Token::swapdelay>, 1, i);

        root.add({df, "set", "pan"},
                 "key", "Sets the pan for drive sounds",
                 &RetroShell::exec <Token::dfn, Token::set, Token::pan>, 1, i);
        
        root.add({df, "audiate"},
                 "command", "Sets the volume of drive sounds");
        
        root.add({df, "audiate", "insert"},
                 "command", "Makes disk insertions audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::insert>, 1, i);
        
        root.add({df, "audiate", "eject"},
                 "command", "Makes disk ejections audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::eject>, 1, i);
        
        root.add({df, "audiate", "step"},
                 "command", "Makes head steps audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::step>, 1, i);
        
        root.add({df, "audiate", "poll"},
                 "command", "Makes polling clicks audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::poll>, 1, i);
    }
    
    for (isize i = 0; i < 4; i++) {

        string df = "df" + std::to_string(i);
        
        root.add({df, "inspect"},
                 "command", "Displays the component state",
                 &RetroShell::exec <Token::dfn, Token::inspect>, 0);
    }

    //
    // Hd0, Hd1, Hd2, Hd3
    //
    
    root.add({"hd0"},
             "component", "Hard drive 0");

    root.add({"hd1"},
             "component", "Hard drive 1");

    root.add({"hd2"},
             "component", "Hard drive 2");

    root.add({"hd3"},
             "component", "Hard drive 3");

    root.add({"hdn"},
             "component", "All connected hard drives");

    for (isize i = 0; i < 4; i++) {
        
        string hd = "hd" + std::to_string(i);

        root.add({hd, "config"},
                 "command", "Displays the current configuration",
                 &RetroShell::exec <Token::hdn, Token::config>, 0, i);
    }
    
    for (isize i = 0; i < 5; i++) {

        string hd = i < 4 ? "hd" + std::to_string(i) : "hdn";
                
        root.add({hd, "set"},
                 "command", "Configures the component");
        
        root.add({hd, "set", "pan"},
                 "key", "Sets the pan for drive sounds",
                 &RetroShell::exec <Token::hdn, Token::set, Token::pan>, 1, i);
        
        root.add({hd, "audiate"},
                 "command", "Sets the volume of drive sounds");
                
        root.add({hd, "audiate", "step"},
                 "command", "Makes head steps audible",
                 &RetroShell::exec <Token::hdn, Token::audiate, Token::step>, 1, i);
    }
    
    for (isize i = 0; i < 4; i++) {
            
        string hd = "hd" + std::to_string(i);
        
        root.add({hd, "inspect"},
                 "command", "Displays the component state");

        root.add({hd, "inspect", "drive"},
                 "command", "Displays hard drive parameters",
                 &RetroShell::exec <Token::hdn, Token::inspect, Token::drive>, 0, i);

        root.add({hd, "inspect", "volumes"},
                 "command", "Displays summarized volume information",
                 &RetroShell::exec <Token::hdn, Token::inspect, Token::volumes>, 0, i);

        root.add({hd, "inspect", "partitions"},
                 "command", "Displays information about all partitions",
                 &RetroShell::exec <Token::hdn, Token::inspect, Token::partition>, 0, i);

        root.add({hd, "inspect", "state"},
                 "command", "Displays the internal state",
                 &RetroShell::exec <Token::hdn, Token::inspect, Token::state>, 0, i);

        root.add({hd, "geometry"},
                 "command", "Changes the disk geometry",
                 &RetroShell::exec <Token::hdn, Token::geometry>, 3, i);
    }
    
    //
    // Zorro boards
    //
    
    root.add({"zorro"},
             "component", "Expansion boards");

    root.add({"zorro", "list"},
             "command", "Lists all connected boards",
             &RetroShell::exec <Token::zorro, Token::list>, 0);

    root.add({"zorro", "inspect"},
             "command", "Inspects a specific Zorro board",
             &RetroShell::exec <Token::zorro, Token::inspect>, 1);

    //
    // OS Debugger
    //

    root.add({"os"},
             "component", "AmigaOS debugger");

    root.add({"os", "info"},
             "command", "Displays basic system information",
             &RetroShell::exec <Token::os, Token::info>, 0);

    root.add({"os", "execbase"},
             "command", "Displays information about the ExecBase struct",
             &RetroShell::exec <Token::os, Token::execbase>, 0);

    root.add({"os", "interrupts"},
             "command", "Lists all interrupt handlers",
             &RetroShell::exec <Token::os, Token::interrupts>, 0);

    root.add({"os", "libraries"},
             "command", "Lists all libraries",
             &RetroShell::exec <Token::os, Token::libraries>, {0, 1});
    
    root.add({"os", "devices"},
             "command", "Lists all devices",
             &RetroShell::exec <Token::os, Token::devices>, {0, 1});

    root.add({"os", "resources"},
             "command", "Lists all resources",
             &RetroShell::exec <Token::os, Token::resources>, {0, 1});

    root.add({"os", "tasks"},
             "command", "Lists all tasks",
             &RetroShell::exec <Token::os, Token::tasks>, {0, 1});

    root.add({"os", "processes"},
             "command", "Lists all processes",
             &RetroShell::exec <Token::os, Token::processes>, {0, 1});

    //
    // Remote server
    //
    
    root.add({"server"},
             "component", "Remote connections");

    root.add({"server", "serial"},
             "component", "Serial port server");
        
    root.add({"server", "serial", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::server, Token::serial, Token::config>, 0);

    root.add({"server", "serial", "set"},
             "command", "Configures the component");
        
    root.add({"server", "serial", "set", "port"},
             "key", "Assigns the port number",
             &RetroShell::exec <Token::server, Token::serial, Token::set, Token::port>, 1);

    root.add({"server", "serial", "set", "verbose"},
             "key", "Switches verbose mode on or off",
             &RetroShell::exec <Token::server, Token::serial, Token::set, Token::verbose>, 1);

    root.add({"server", "serial", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::server, Token::serial, Token::inspect>, 0);
  
    root.add({"server", "rshell"},
             "component", "Retro shell server");

    root.add({"server", "rshell", "start"},
             "command", "Starts the retro shell server",
             &RetroShell::exec <Token::server, Token::rshell, Token::start>, 0);

    root.add({"server", "rshell", "stop"},
             "command", "Stops the retro shell server",
             &RetroShell::exec <Token::server, Token::rshell, Token::stop>, 0);

    root.add({"server", "rshell", "disconnect"},
             "command", "Disconnects a client",
             &RetroShell::exec <Token::server, Token::rshell, Token::disconnect>, 0);

    root.add({"server", "rshell", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::server, Token::rshell, Token::config>, 0);

    root.add({"server", "rshell", "set"},
             "command", "Configures the component");
        
    root.add({"server", "rshell", "set", "port"},
             "key", "Assigns the port number",
             &RetroShell::exec <Token::server, Token::rshell, Token::set, Token::port>, 1);

    root.add({"server", "serial", "set", "verbose"},
             "key", "Switches verbose mode on or off",
             &RetroShell::exec <Token::server, Token::rshell, Token::set, Token::verbose>, 1);

    root.add({"server", "rshell", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::server, Token::rshell, Token::inspect>, 0, 0);
   
    root.add({"server", "gdb"},
             "component", "GDB server");

    root.add({"server", "gdb", "attach"},
             "command", "Attaches the GDB server to a process",
             &RetroShell::exec <Token::server, Token::gdb, Token::attach>, 1);

    root.add({"server", "gdb", "detach"},
             "command", "Detaches the GDB server from a process",
             &RetroShell::exec <Token::server, Token::gdb, Token::detach>, 0);

    root.add({"server", "gdb", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::server, Token::gdb, Token::config>, 0);

    root.add({"server", "gdb", "set"},
             "command", "Configures the component");
        
    root.add({"server", "gdb", "set", "port"},
             "key", "Assigns the port number",
             &RetroShell::exec <Token::server, Token::gdb, Token::set, Token::port>, 1);

    root.add({"server", "gdb", "set", "verbose"},
             "key", "Switches verbose mode on or off",
             &RetroShell::exec <Token::server, Token::gdb, Token::set, Token::verbose>, 1);

    root.add({"server", "gdb", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::server, Token::gdb, Token::inspect>, 0);

    root.add({"server", "list"},
             "command", "Displays a server status summary",
             &RetroShell::exec <Token::server, Token::list>, 0);
}
