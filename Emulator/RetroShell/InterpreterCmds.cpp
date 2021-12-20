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
             &RetroShell::exec <Token::clear>);
    root.seek("clear")->hidden = true;

    root.add({"close"},
             "command", "Hides the debug console",
             &RetroShell::exec <Token::close>);
    root.seek("close")->hidden = true;

    root.add({"help"},
             "command", "Prints usage information",
             &RetroShell::exec <Token::help>);
    root.seek("help")->hidden = true;
    root.seek("help")->maxArgs = 1;

    root.add({"joshua"},
             "command", "",
             &RetroShell::exec <Token::easteregg>);
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
             &RetroShell::exec <Token::amiga, Token::power, Token::on>);

    root.add({"amiga", "power", "off"},
             "state", "Switches the Amiga off",
             &RetroShell::exec <Token::amiga, Token::power, Token::off>);

    root.add({"amiga", "debug"},
             "command", "Switches debug mode on or off");
    
    root.add({"amiga", "debug", "on"},
             "state", "Switches debug mode on",
             &RetroShell::exec <Token::amiga, Token::debug, Token::on>);

    root.add({"amiga", "debug", "off"},
             "state", "Switches debug mode off",
             &RetroShell::exec <Token::amiga, Token::debug, Token::off>);

    root.add({"amiga", "run"},
             "command", "Starts the emulator thread",
             &RetroShell::exec <Token::amiga, Token::run>);
    
    root.add({"amiga", "pause"},
             "command", "Halts the emulator thread",
             &RetroShell::exec <Token::amiga, Token::pause>);
    
    root.add({"amiga", "reset"},
             "command", "Performs a hard reset",
             &RetroShell::exec <Token::amiga, Token::reset>);
    
    root.add({"amiga", "inspect"},
             "command", "Displays the component state",
             &RetroShell::exec <Token::amiga, Token::inspect>);

    
    //
    // Memory
    //
    
    root.add({"memory"},
             "component", "Ram and Rom");
    
    root.add({"memory", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::memory, Token::config>);

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
             &RetroShell::exec <Token::memory, Token::inspect, Token::state>);

    root.add({"memory", "inspect", "bankmap"},
             "command", "Displays the bank map",
             &RetroShell::exec <Token::memory, Token::inspect, Token::bankmap>);

    root.add({"memory", "inspect", "checksum"},
             "command", "Computes memory checksums",
             &RetroShell::exec <Token::memory, Token::inspect, Token::checksums>);

    
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
             &RetroShell::exec <Token::cpu, Token::inspect, Token::state>);

    root.add({"cpu", "inspect", "registers"},
             "command", "Displays the current register values",
             &RetroShell::exec <Token::cpu, Token::inspect, Token::registers>);

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
             &RetroShell::exec <Token::agnus, Token::config>);
    
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
             &RetroShell::exec <Token::agnus, Token::inspect, Token::state>);

    root.add({"agnus", "inspect", "registers"},
             "category", "Displays the current register values",
             &RetroShell::exec <Token::agnus, Token::inspect, Token::registers>);

    root.add({"agnus", "inspect", "dma"},
             "category", "Displays the scheduled DMA transfers",
             &RetroShell::exec <Token::agnus, Token::inspect, Token::dma>);

    root.add({"agnus", "inspect", "events"},
             "category", "Displays all scheduled events",
             &RetroShell::exec <Token::agnus, Token::inspect, Token::events>);
    
    
    //
    // Blitter
    //
    
    root.add({"blitter"},
             "component", "Custom Chip (Agnus)");
    
    root.add({"blitter", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::blitter, Token::config>);
    
    root.add({"blitter", "set"},
             "command", "Configures the component");
        
    root.add({"blitter", "set", "accuracy"},
             "level", "Selects the emulation accuracy level",
             &RetroShell::exec <Token::blitter, Token::set, Token::accuracy>, 1);

    root.add({"blitter", "inspect"},
             "command", "Displays the internal state");

    root.add({"blitter", "inspect", "state"},
             "category", "Displays the internal state",
             &RetroShell::exec <Token::blitter, Token::inspect, Token::state>);

    root.add({"blitter", "inspect", "registers"},
             "category", "Displays the current register value",
             &RetroShell::exec <Token::blitter, Token::inspect, Token::registers>);

    
    //
    // Copper
    //
    
    root.add({"copper"},
             "component", "Custom Chip (Agnus)");
    
    root.add({"copper", "inspect"},
             "command", "Displays the internal state");

    root.add({"copper", "inspect", "state"},
             "category", "Displays the current state",
             &RetroShell::exec <Token::copper, Token::inspect, Token::state>);

    root.add({"copper", "inspect", "registers"},
             "category", "Displays the current register value",
             &RetroShell::exec <Token::copper, Token::inspect, Token::registers>);

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
             &RetroShell::exec <Token::denise, Token::config>);

    root.add({"denise", "set"},
             "command", "Configures the component");

    root.add({"denise", "set", "revision"},
             "key", "Selects the emulated chip model",
             &RetroShell::exec <Token::denise, Token::set, Token::revision>, 1);
    
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
             &RetroShell::exec <Token::denise, Token::inspect, Token::state>);

    root.add({"denise", "inspect", "registers"},
             "category", "Displays the current register value",
             &RetroShell::exec <Token::denise, Token::inspect, Token::registers>);

    
    //
    // DMA Debugger
    //

    root.add({"dmadebugger"},
             "component", "DMA Debugger");

    root.add({"dmadebugger", "open"},
             "command", "Opens the DMA debugger",
             &RetroShell::exec <Token::dmadebugger, Token::open>);

    root.add({"dmadebugger", "close"},
             "command", "Closes the DMA debugger",
             &RetroShell::exec <Token::dmadebugger, Token::close>);

    root.add({"dmadebugger", "show"},
             "command", "Enables the debugger for a certain DMA channel");

    root.add({"dmadebugger", "show", "copper"},
             "command", "Visualizes Copper DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::copper>);

    root.add({"dmadebugger", "show", "blitter"},
             "command", "Visualizes Blitter DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::blitter>);

    root.add({"dmadebugger", "show", "disk"},
             "command", "Visualizes Disk DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::disk>);

    root.add({"dmadebugger", "show", "audio"},
             "command", "Visualizes Audio DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::audio>);

    root.add({"dmadebugger", "show", "sprites"},
             "command", "Visualizes Sprite DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::sprites>);

    root.add({"dmadebugger", "show", "bitplanes"},
             "command", "Visualizes Bitplane DMA",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::bitplanes>);

    root.add({"dmadebugger", "show", "cpu"},
             "command", "Visualizes CPU accesses",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::cpu>);

    root.add({"dmadebugger", "show", "refresh"},
             "command", "Visualizes memory refresh cycles",
             &RetroShell::exec <Token::dmadebugger, Token::show, Token::refresh>);

    root.add({"dmadebugger", "hide"},
             "command", "Disables the debugger for a certain DMA channel");

    root.add({"dmadebugger", "hide", "copper"},
             "command", "Hides Copper DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::copper>);

    root.add({"dmadebugger", "hide", "blitter"},
             "command", "Hides Blitter DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::blitter>);

    root.add({"dmadebugger", "hide", "disk"},
             "command", "Hides Disk DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::disk>);

    root.add({"dmadebugger", "hide", "audio"},
             "command", "Hides Audio DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::audio>);

    root.add({"dmadebugger", "hide", "sprites"},
             "command", "Hides Sprite DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::sprites>);

    root.add({"dmadebugger", "hide", "bitplanes"},
             "command", "Hides Bitplane DMA",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::bitplanes>);

    root.add({"dmadebugger", "hide", "cpu"},
             "command", "Hides CPU accesses",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::cpu>);

    root.add({"dmadebugger", "hide", "refresh"},
             "command", "Hides memory refresh cycles",
             &RetroShell::exec <Token::dmadebugger, Token::hide, Token::refresh>);

    
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
             &RetroShell::exec <Token::audio, Token::config>);

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
             &RetroShell::exec <Token::audio, Token::inspect, Token::state>);

    root.add({"audio", "inspect", "registers"},
             "category", "Displays the current register value",
             &RetroShell::exec <Token::audio, Token::inspect, Token::registers>);
    
    
    //
    // Paula
    //
    
    root.add({"paula"},
             "component", "Custom chip");

    root.add({"paula", "inspect"},
             "command", "Displays the internal state");

    root.add({"paula", "inspect", "state"},
             "command", "Displays the current register value",
             &RetroShell::exec <Token::paula, Token::inspect, Token::state>);

    root.add({"paula", "inspect", "registers"},
             "command", "Displays the current register value",
             &RetroShell::exec <Token::paula, Token::inspect, Token::registers>);

    
    //
    // RTC
    //

    root.add({"rtc"},
             "component", "Real-time clock");

    root.add({"rtc", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::rtc, Token::config>);

    root.add({"rtc", "set"},
             "command", "Configures the component");
        
    root.add({"rtc", "set", "revision"},
             "key", "Selects the emulated chip model",
             &RetroShell::exec <Token::rtc, Token::set, Token::revision>, 1);

    root.add({"rtc", "inspect"},
             "command", "Displays the internal state");

    root.add({"rtc", "inspect", "registers"},
             "command", "Displays the current register value",
             &RetroShell::exec <Token::rtc, Token::inspect, Token::registers>);

    
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
             &RetroShell::exec <Token::keyboard, Token::config>);
    
    root.add({"keyboard", "set"},
             "command", "Configures the component");
        
    root.add({"keyboard", "set", "accuracy"},
             "key", "Determines the emulation accuracy level",
             &RetroShell::exec <Token::keyboard, Token::set, Token::accuracy>, 1);

    root.add({"keyboard", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::keyboard, Token::inspect>);

    
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
             &RetroShell::exec <Token::serial, Token::config>);

    root.add({"serial", "set"},
             "command", "Configures the component");
        
    root.add({"serial", "set", "device"},
             "key", "",
             &RetroShell::exec <Token::serial, Token::set, Token::device>, 1);

    root.add({"serial", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::serial, Token::inspect>);

    
    //
    // Disk controller
    //
    
    root.add({"diskcontroller"},
             "component", "Disk Controller");

    root.add({"diskcontroller", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::dc, Token::config>);

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
             &RetroShell::exec <Token::dc, Token::inspect>);


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
        
        root.add({df, "set", "defaultfs"},
                 "key", "Determines the default file system type for blank disks",
                 &RetroShell::exec <Token::dfn, Token::set, Token::defaultfs>, 1, i);
        
        root.add({df, "set", "defaultbb"},
                 "key", "Determines the default boot block type for blank disks",
                 &RetroShell::exec <Token::dfn, Token::set, Token::defaultbb>, 1, i);
        
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
                 "command", "Makes disk ejections audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::step>, 1, i);
        
        root.add({df, "audiate", "poll"},
                 "command", "Makes polling clicks audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::poll>, 1, i);
        
        root.add({df, "inspect"},
                 "command", "Displays the internal state",
                 &RetroShell::exec <Token::dfn, Token::inspect>);
    }

    //
    // OS Debugger
    //

    root.add({"os"},
             "component", "AmigaOS debugger");

    root.add({"os", "execbase"},
             "command", "Displays information about the ExecBase struct",
             &RetroShell::exec <Token::os, Token::execbase>);

    root.add({"os", "interrupts"},
             "command", "Lists all interrupt handlers",
             &RetroShell::exec <Token::os, Token::interrupts>);

    root.add({"os", "libraries"},
             "command", "Lists all libraries",
             &RetroShell::exec <Token::os, Token::libraries>);
    root.seek("os")->seek("libraries")->maxArgs = 1;
    
    root.add({"os", "devices"},
             "command", "Lists all devices",
             &RetroShell::exec <Token::os, Token::devices>);
    root.seek("os")->seek("devices")->maxArgs = 1;

    root.add({"os", "resources"},
             "command", "Lists all resources",
             &RetroShell::exec <Token::os, Token::resources>);
    root.seek("os")->seek("resources")->maxArgs = 1;

    root.add({"os", "tasks"},
             "command", "Lists all tasks",
             &RetroShell::exec <Token::os, Token::tasks>);
    root.seek("os")->seek("tasks")->maxArgs = 1;

    root.add({"os", "processes"},
             "command", "Lists all processes",
             &RetroShell::exec <Token::os, Token::processes>);
    root.seek("os")->seek("processes")->maxArgs = 1;

    //
    // Remote server
    //
        
    root.add({"remote"},
             "component", "Remote server");

    root.add({"remote", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::remote, Token::config>);
    
    root.add({"remote", "set"},
             "command", "Configures the component");
        
    root.add({"remote", "set", "mode"},
             "key", "Selects the server protocol",
             &RetroShell::exec <Token::remote, Token::set, Token::mode>, 1);

    root.add({"remote", "set", "port"},
             "key", "Assigns the port number",
             &RetroShell::exec <Token::remote, Token::set, Token::port>, 1);

    root.add({"remote", "start"},
             "command", "Starts the debug server",
             &RetroShell::exec <Token::remote, Token::start>, 0);

    root.add({"remote", "stop"},
             "command", "Stops the debug server",
             &RetroShell::exec <Token::remote, Token::stop>, 0);

    root.add({"remote", "inspect"},
             "command", "Displays the internal state",
             &RetroShell::exec <Token::remote, Token::inspect>, 0);
    
    //
    // GDB server
    //
        
    root.add({"gdbserver"},
             "component", "GDB server");
    root.seek("gdbserver")->hidden = true;

    root.add({"gdbserver", "config"},
             "command", "Displays the current configuration",
             &RetroShell::exec <Token::gdb, Token::config>);
    
    root.add({"gdbserver", "set"},
             "command", "Configures the component");
        
    root.add({"gdbserver", "set", "verbose"},
             "key", "Enables or disables verbose mode",
             &RetroShell::exec <Token::gdb, Token::set, Token::verbose>, 1);
}
