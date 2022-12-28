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
#include "Amiga.h"

namespace vamiga {

void
Interpreter::initCommons(Command &root)
{
    //
    // Common commands
    //

    root.newGroup("Controlling the shell");

    root.oldadd({"."},
             "Enters or exists the debugger",
             &RetroShell::exec <Token::debug>);

    root.oldadd({"clear"},
             "Clears the console window",
             &RetroShell::exec <Token::clear>);

    root.oldadd({"close"},
             "Hides the console window",
             &RetroShell::exec <Token::close>);

    root.oldadd({"help"}, { }, {Arg::command},
             "Prints usage information",
             &RetroShell::exec <Token::help>);

    root.oldadd({"joshua"},
             "",
             &RetroShell::exec <Token::easteregg>);

    root.oldadd({"source"}, {Arg::path},
             "Processes a command script",
             &RetroShell::exec <Token::source>);

    root.oldadd({"wait"}, {Arg::value, Arg::seconds},
             "Pauses the execution of a command script",
             &RetroShell::exec <Token::wait>);

    root.hide({"joshua"});
    root.hide({"wait"});
}

void
Interpreter::initCommandShell(Command &root)
{
    initCommons(root);

    //
    // Top-level commands
    //

    root.newGroup("Regression testing");

    root.add({"regression"},    "Runs the regression tester");
    root.add({"screenshot"},    "Manages screenshots");

    root.newGroup("Controlling components");

    root.add({"amiga"},         "The virtual Amiga");
    root.add({"memory"},        "Ram and Rom");
    root.add({"cpu"},           "Motorola 68k CPU");
    root.add({"ciaa"},          "Complex Interface Adapter A");
    root.add({"ciab"},          "Complex Interface Adapter B");
    root.add({"agnus"},         "Custom chip");
    root.add({"blitter"},       "Coprocessor");
    root.add({"denise"},        "Custom chip");
    root.add({"paula"},         "Custom chip");
    root.add({"rtc"},           "Real-time clock");
    root.add({"serial"},        "Serial port");
    root.add({"dmadebugger"},   "DMA Debugger");

    root.newGroup("Controlling peripherals");

    root.add({"monitor"},       "Amiga monitor");
    root.add({"keyboard"},      "Keyboard");
    root.add({"mouse"},         "Mouse");
    root.add({"joystick"},      "Joystick");
    root.add({"dfn"},           "All floppy drives");
    root.add({"df0"},           "Floppy drive 0");
    root.add({"df1"},           "Floppy drive 1");
    root.add({"df2"},           "Floppy drive 2");
    root.add({"df3"},           "Floppy drive 3");
    root.add({"hdn"},           "All hard drives");
    root.add({"hd0"},           "Hard drive 0");
    root.add({"hd1"},           "Hard drive 1");
    root.add({"hd2"},           "Hard drive 2");
    root.add({"hd3"},           "Hard drive 3");

    root.newGroup("Miscellaneous");

    root.add({"server"},        "Remote connections");


    //
    // Regression testing
    //

    root.newGroup("");

    root.oldadd({"regression", "setup"}, { ConfigSchemeEnum::argList(), Arg::path }, { Arg::path },
             "Initializes the test environment",
             &RetroShell::exec <Token::regression, Token::setup>);

    root.oldadd({"regression", "run"}, { Arg::path },
             "Launches a regression test",
             &RetroShell::exec <Token::regression, Token::run>);

    root.add({"screenshot", "set"},
             "Configures the screenshot");

    root.oldadd({"screenshot", "set", "filename"}, { Arg::path },
             "Assigns the screen shot filename",
             &RetroShell::exec <Token::screenshot, Token::set, Token::filename>);

    root.oldadd({"screenshot", "set", "cutout"}, { Arg::value, Arg::value, Arg::value, Arg::value },
             "Adjusts the texture cutout",
             &RetroShell::exec <Token::screenshot, Token::set, Token::cutout>);

    root.oldadd({"screenshot", "save"}, { Arg::path },
             "Saves a screenshot and exits the emulator",
             &RetroShell::exec <Token::screenshot, Token::save>);

    
    //
    // Amiga
    //

    root.newGroup("");

    root.oldadd({"amiga", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::amiga, Token::config>);

    root.add({"amiga", "set"},
             "Configures the component");

    root.oldadd({"amiga", "set", "type"}, { VideoFormatEnum::argList() },
             "Selects the video standard",
             &RetroShell::exec <Token::amiga, Token::set, Token::type>);

    root.oldadd({"amiga", "set", "fpsmode"}, { FpsModeEnum::argList() },
             "Selects the frame mode",
             &RetroShell::exec <Token::amiga, Token::set, Token::fpsmode>);

    root.oldadd({"amiga", "set", "fps"}, { Arg::value },
             "Sets the frames per seconds",
             &RetroShell::exec <Token::amiga, Token::set, Token::fps>);

    root.oldadd({"amiga", "init"}, { ConfigSchemeEnum::argList() },
             "Initializes the Amiga with a predefined scheme",
             &RetroShell::exec <Token::amiga, Token::init>);

    root.oldadd({"amiga", "power"}, { Arg::onoff },
             "Switches the Amiga on or off",
             &RetroShell::exec <Token::amiga, Token::power>);

    root.oldadd({"amiga", "reset"},
             "Performs a hard reset",
             &RetroShell::exec <Token::amiga, Token::reset>);

    root.oldadd({"amiga", "defaults"},
             "Displays the user defaults storage",
             &RetroShell::exec <Token::amiga, Token::defaults>);


    //
    // Memory
    //

    root.newGroup("");

    root.oldadd({"memory", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::memory, Token::config>);

    root.add({"memory", "set"},
             "Configures the component");

    root.oldadd({"memory", "set", "chip"}, { Arg::kb },
             "Configures the amouts of chip memory",
             &RetroShell::exec <Token::memory, Token::set, Token::chip>);

    root.oldadd({"memory", "set", "slow"},  { Arg::kb },
             "Configures the amouts of slow memory",
             &RetroShell::exec <Token::memory, Token::set, Token::slow>);

    root.oldadd({"memory", "set", "fast"}, { Arg::kb },
             "Configures the amouts of flow memory",
             &RetroShell::exec <Token::memory, Token::set, Token::fast>);

    root.oldadd({"memory", "set", "extstart"}, { Arg::address },
             "Sets the start address for Rom extensions",
             &RetroShell::exec <Token::memory, Token::set, Token::extstart>);

    root.oldadd({"memory", "set", "saveroms"}, { Arg::boolean },
             "Determines whether Roms should be stored in snapshots",
             &RetroShell::exec <Token::memory, Token::set, Token::saveroms>);

    root.oldadd({"memory", "set", "slowramdelay"}, { Arg::boolean },
             "Enables or disables slow Ram bus delays",
             &RetroShell::exec <Token::memory, Token::set, Token::slowramdelay>);

    root.oldadd({"memory", "set", "bankmap"}, { BankMapEnum::argList() },
             "Selects the bank mapping scheme",
             &RetroShell::exec <Token::memory, Token::set, Token::bankmap>);

    root.oldadd({"memory", "set", "raminit"}, { RamInitPatternEnum::argList() },
             "Determines how Ram is initialized on startup",
             &RetroShell::exec <Token::memory, Token::set, Token::raminitpattern>);

    root.oldadd({"memory", "set", "unmapped"}, { UnmappedMemoryEnum::argList() },
             "Determines the behaviour of unmapped memory",
             &RetroShell::exec <Token::memory, Token::set, Token::unmappingtype>);

    root.add({"memory", "load"},
             "Installs a Rom image");

    root.oldadd({"memory", "load", "rom"}, { Arg::path },
             "Installs a Kickstart Rom",
             &RetroShell::exec <Token::memory, Token::load, Token::rom>);

    root.oldadd({"memory", "load", "extrom"}, { Arg::path },
             "Installs a Rom extension",
             &RetroShell::exec <Token::memory, Token::load, Token::extrom>);

    
    //
    // CPU
    //

    root.newGroup("");

    root.oldadd({"cpu", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::cpu, Token::config>);

    root.add({"cpu", "set"},
             "Configures the component");

    root.oldadd({"cpu", "set", "revision"}, { CPURevisionEnum::argList() },
             "Selects the emulated chip model",
             &RetroShell::exec <Token::cpu, Token::set, Token::revision>);

    root.add({"cpu", "set", "dasm"},
             "Configures the disassembler");

    root.oldadd({"cpu", "set", "dasm", "revision"}, { DasmRevisionEnum::argList() },
             "Selects the disassembler instruction set",
             &RetroShell::exec <Token::cpu, Token::set, Token::dasm, Token::revision>);

    root.oldadd({"cpu", "set", "dasm", "syntax"}, {  DasmSyntaxEnum::argList() },
             "Selects the disassembler syntax style",
             &RetroShell::exec <Token::cpu, Token::set, Token::dasm, Token::syntax>);

    root.oldadd({"cpu", "set", "overclocking"}, { Arg::value },
             "Overclocks the CPU by the specified factor",
             &RetroShell::exec <Token::cpu, Token::set, Token::overclocking>);

    root.oldadd({"cpu", "set", "regreset"}, { Arg::value },
             "Selects the reset value of data and address registers",
             &RetroShell::exec <Token::cpu, Token::set, Token::regreset>);


    //
    // CIA
    //

    root.newGroup("");

    for (isize i = 0; i < 2; i++) {
        
        string cia = (i == 0) ? "ciaa" : "ciab";
        
        root.oldadd({cia, ""},
                 "Displays the current configuration",
                 &RetroShell::exec <Token::cia, Token::config>, i);
        
        root.add({cia, "set"},
                 "Configures the component");
        
        root.oldadd({cia, "set", "revision"}, { CIARevisionEnum::argList() },
                 "Selects the emulated chip model",
                 &RetroShell::exec <Token::cia, Token::set, Token::revision>, i);
        
        root.oldadd({cia, "set", "todbug"}, { Arg::boolean },
                 "Enables or disables the TOD hardware bug",
                 &RetroShell::exec <Token::cia, Token::set, Token::todbug>, i);
        
        root.oldadd({cia, "set", "esync"}, { Arg::boolean },
                 "Turns E-clock syncing on or off",
                 &RetroShell::exec <Token::cia, Token::set, Token::esync>, i);
    }
    
    
    //
    // Agnus
    //

    root.newGroup("");

    root.oldadd({"agnus", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::agnus, Token::config>);
    
    root.add({"agnus", "set"},
             "Configures the component");

    root.oldadd({"agnus", "set", "revision"}, { AgnusRevisionEnum::argList() },
             "Selects the emulated chip model",
             &RetroShell::exec <Token::agnus, Token::set, Token::revision>);

    root.oldadd({"agnus", "set", "slowrammirror"}, { Arg::boolean },
             "Enables or disables ECS Slow Ram mirroring",
             &RetroShell::exec <Token::agnus, Token::set, Token::slowrammirror>);

    root.oldadd({"agnus", "set", "ptrdrops"}, { Arg::boolean },
             "Emulate dropped register writes",
             &RetroShell::exec <Token::agnus, Token::set, Token::ptrdrops>);

    
    //
    // Blitter
    //

    root.newGroup("");

    root.oldadd({"blitter", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::blitter, Token::config>);
    
    root.add({"blitter", "set"},
             "Configures the component");

    root.oldadd({"blitter", "set", "accuracy"}, { "1..3" },
             "Selects the emulation accuracy level",
             &RetroShell::exec <Token::blitter, Token::set, Token::accuracy>);

    
    //
    // Denise
    //

    root.newGroup("");

    root.oldadd({"denise", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::denise, Token::config>);

    root.add({"denise", "set"},
             "Configures the component");

    root.oldadd({"denise", "set", "revision"}, { DeniseRevisionEnum::argList() },
             "Selects the emulated chip model",
             &RetroShell::exec <Token::denise, Token::set, Token::revision>);

    root.oldadd({"denise", "set", "tracking"}, { Arg::boolean },
             "Enables or disables viewport tracking",
             &RetroShell::exec <Token::denise, Token::set, Token::tracking>);

    root.oldadd({"denise", "set", "clxsprspr"}, { Arg::boolean },
             "Switches sprite-sprite collision detection on or off",
             &RetroShell::exec <Token::denise, Token::set, Token::clxsprspr>);

    root.oldadd({"denise", "set", "clxsprplf"}, { Arg::boolean },
             "Switches sprite-playfield collision detection on or off",
             &RetroShell::exec <Token::denise, Token::set, Token::clxsprplf>);

    root.oldadd({"denise", "set", "clxplfplf"}, { Arg::boolean },
             "Switches playfield-playfield collision detection on or off",
             &RetroShell::exec <Token::denise, Token::set, Token::clxplfplf>);
    
    root.add({"denise", "set", "hidden"},
             "Hides bitplanes, sprites, or layers");

    root.oldadd({"denise", "set", "hidden", "bitplanes"}, { Arg::value },
             "Wipes out certain bitplane data",
             &RetroShell::exec <Token::denise, Token::set, Token::hide, Token::bitplanes>);

    root.oldadd({"denise", "set", "hidden", "sprites"}, { Arg::value },
             "Wipes out certain sprite data",
             &RetroShell::exec <Token::denise, Token::set, Token::hide, Token::sprites>);

    root.oldadd({"denise", "set", "hidden", "layers"}, { Arg::value },
             "Makes certain drawing layers transparent",
             &RetroShell::exec <Token::denise, Token::set, Token::hide, Token::layers>);

    
    //
    // DMA Debugger
    //

    root.oldadd({"dmadebugger", "open"},
             "Opens the DMA debugger",
             &RetroShell::exec <Token::dmadebugger, Token::open>);

    root.oldadd({"dmadebugger", "close"},
             "Closes the DMA debugger",
             &RetroShell::exec <Token::dmadebugger, Token::close>);

    root.oldadd({"dmadebugger", "copper"}, { Arg::onoff },
             "Turns Copper DMA visualization on or off",
             &RetroShell::exec <Token::dmadebugger, Token::copper>);

    root.oldadd({"dmadebugger", "blitter"}, { Arg::onoff },
             "Turns Blitter DMA visualization on or off",
             &RetroShell::exec <Token::dmadebugger, Token::blitter>);

    root.oldadd({"dmadebugger", "disk"}, { Arg::onoff },
             "Turns Disk DMA visualization on or off",
             &RetroShell::exec <Token::dmadebugger, Token::disk>);

    root.oldadd({"dmadebugger", "audio"}, { Arg::onoff },
             "Turns Audio DMA visualization on or off",
             &RetroShell::exec <Token::dmadebugger, Token::audio>);

    root.oldadd({"dmadebugger", "sprites"}, { Arg::onoff },
             "Turns Sprite DMA visualization on or off",
             &RetroShell::exec <Token::dmadebugger, Token::sprites>);

    root.oldadd({"dmadebugger", "bitplanes"}, { Arg::onoff },
             "Turns Bitplane DMA visualization on or off",
             &RetroShell::exec <Token::dmadebugger, Token::bitplanes>);

    root.oldadd({"dmadebugger", "cpu"}, { Arg::onoff },
             "Turns CPU bus usage visualization on or off",
             &RetroShell::exec <Token::dmadebugger, Token::cpu>);

    root.oldadd({"dmadebugger", "refresh"}, { Arg::onoff },
             "Turn memory refresh visualization on or off",
             &RetroShell::exec <Token::dmadebugger, Token::refresh>);


    //
    // Monitor
    //

    root.oldadd({"monitor", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::monitor, Token::config>);

    root.add({"monitor", "set"},
             "Configures the component");

    root.oldadd({"monitor", "set", "palette"}, { PaletteEnum::argList() },
             "Selects the color palette",
             &RetroShell::exec <Token::monitor, Token::set, Token::palette>);

    root.oldadd({"monitor", "set", "brightness"}, { Arg::value },
             "Adjusts the brightness of the Amiga texture",
             &RetroShell::exec <Token::monitor, Token::set, Token::brightness>);

    root.oldadd({"monitor", "set", "contrast"}, { Arg::value },
             "Adjusts the contrast of the Amiga texture",
             &RetroShell::exec <Token::monitor, Token::set, Token::contrast>);

    root.oldadd({"monitor", "set", "saturation"}, { Arg::value },
             "Adjusts the saturation of the Amiga texture",
             &RetroShell::exec <Token::monitor, Token::set, Token::saturation>);

    
    //
    // Paula (Audio)
    //

    root.newGroup("");

    root.add({"paula", "audio"},
             "Audio unit");

    root.oldadd({"paula", "audio", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::audio, Token::config>);

    root.oldadd({"paula", "audio", "filter"},
             "Displays the current filter configuration",
             &RetroShell::exec <Token::audio, Token::filter, Token::config>);

    root.add({"paula", "audio", "filter", "set"},
             "Configures the audio filter");

    root.oldadd({"paula", "audio", "filter", "set", "type"}, { FilterTypeEnum::argList() },
             "Configures the audio filter type",
             &RetroShell::exec <Token::audio, Token::filter, Token::set, Token::type>);

    root.oldadd({"paula", "audio", "filter", "set", "activation"}, { FilterActivationEnum::argList() },
             "Selects the filter activation condition",
             &RetroShell::exec <Token::audio, Token::filter, Token::set, Token::activation>);

    root.add({"paula", "audio", "set"},
             "Configures the component");

    root.oldadd({"paula", "audio", "set", "sampling"}, { SamplingMethodEnum::argList() },
             "Selects the sampling method",
             &RetroShell::exec <Token::audio, Token::set, Token::sampling>);

    root.add({"paula", "audio", "set", "volume"},
             "Sets the volume");

    root.oldadd({"paula", "audio", "set", "volume", "channel0"}, { Arg::volume },
             "Sets the volume for audio channel 0",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 0);
    
    root.oldadd({"paula", "audio", "set", "volume", "channel1"}, { Arg::volume },
             "Sets the volume for audio channel 1",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 1);
    
    root.oldadd({"paula", "audio", "set", "volume", "channel2"}, { Arg::volume },
             "Sets the volume for audio channel 2",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 2);
    
    root.oldadd({"paula", "audio", "set", "volume", "channel3"}, { Arg::volume },
             "Sets the volume for audio channel 3",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 3);
    
    root.oldadd({"paula", "audio", "set", "volume", "left"}, { Arg::volume },
             "Sets the master volume for the left speaker",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 4);
    
    root.oldadd({"paula", "audio", "set", "volume", "right"}, { Arg::volume },
             "Sets the master volume for the right speaker",
             &RetroShell::exec <Token::audio, Token::set, Token::volume>, 5);

    root.add({"paula", "audio", "set", "pan"},
             "Sets the pan for one of the four audio channels");
    
    root.oldadd({"paula", "audio", "set", "pan", "channel0"}, { Arg::value },
             "Sets the pan for audio channel 0",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 0);
    
    root.oldadd({"paula", "audio", "set", "pan", "channel1"}, { Arg::value },
             "Sets the pan for audio channel 1",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 1);
    
    root.oldadd({"paula", "audio", "set", "pan", "channel2"}, { Arg::value },
             "Sets the pan for audio channel 2",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 2);
    
    root.oldadd({"paula", "audio", "set", "pan", "channel3"}, { Arg::value },
             "Sets the pan for audio channel 3",
             &RetroShell::exec <Token::audio, Token::set, Token::pan>, 3);


    //
    // Paula (Disk controller)
    //

    root.add({"paula", "dc"},
             "Disk controller");

    root.oldadd({"paula", "dc", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::dc, Token::config>);

    root.add({"paula", "dc", "set"},
             "Configures the component");

    root.oldadd({"paula", "dc", "set", "speed"}, { Arg::value },
             "Configures the data transfer speed",
             &RetroShell::exec <Token::dc, Token::speed>);

    root.add({"paula", "dc", "dsksync"},
             "Secures the DSKSYNC register");

    root.oldadd({"paula", "dc", "dsksync", "auto"}, { Arg::boolean },
             "Always receive a SYNC signal",
             &RetroShell::exec <Token::dc, Token::dsksync, Token::autosync>);

    root.oldadd({"paula", "dc", "dsksync", "lock"}, { Arg::boolean },
             "Prevents writes to DSKSYNC",
             &RetroShell::exec <Token::dc, Token::dsksync, Token::lock>);


    //
    // RTC
    //

    root.newGroup("");

    root.oldadd({"rtc", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::rtc, Token::config>);

    root.add({"rtc", "set"},
             "Configures the component");

    root.oldadd({"rtc", "set", "revision"}, { RTCRevisionEnum::argList() },
             "Selects the emulated chip model",
             &RetroShell::exec <Token::rtc, Token::set, Token::revision>);


    //
    // Keyboard
    //

    root.newGroup("");

    root.oldadd({"keyboard", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::keyboard, Token::config>);
    
    root.add({"keyboard", "set"},
             "Configures the component");

    root.oldadd({"keyboard", "set", "accuracy"}, { Arg::value },
             "Determines the emulation accuracy level",
             &RetroShell::exec <Token::keyboard, Token::set, Token::accuracy>);

    root.oldadd({"keyboard", "press"}, { Arg::value },
             "Sends a keycode to the keyboard",
             &RetroShell::exec <Token::keyboard, Token::press>);

    
    //
    // Mouse
    //

    root.newGroup("");

    for (isize i = 1; i <= 2; i++) {

        string nr = (i == 1) ? "1" : "2";

        root.add({"mouse", nr},
                 "Mouse in port " + nr);

        root.oldadd({"mouse", nr, ""},
                 "Displays the current configuration",
                 &RetroShell::exec <Token::mouse, Token::config>, i);
        
        root.add({"mouse", nr, "set"},
                 "Configures the component");
        
        root.oldadd({"mouse", nr, "set", "pullup"}, { Arg::boolean },
                 "Enables or disables pull-up resistors",
                 &RetroShell::exec <Token::mouse, Token::set, Token::pullup>, i);
        
        root.oldadd({"mouse", nr, "set", "shakedetector"}, { Arg::boolean },
                 "Enables or disables the shake detector",
                 &RetroShell::exec <Token::mouse, Token::set, Token::shakedetector>, i);
        
        root.oldadd({"mouse", nr, "set", "velocity"}, { Arg::value },
                 "Sets the horizontal and vertical mouse velocity",
                 &RetroShell::exec <Token::mouse, Token::set, Token::velocity>, i);
        
        root.add({"mouse", nr, "press"},
                 "Presses a mouse button");

        root.oldadd({"mouse", nr, "press", "left"},
                 "Presses the left mouse button",
                 &RetroShell::exec <Token::mouse, Token::press, Token::left>, i);
        
        root.oldadd({"mouse", nr, "press", "right"},
                 "Presses the right mouse button",
                 &RetroShell::exec <Token::mouse, Token::press, Token::right>, i);
    }

    
    //
    // Joystick
    //

    root.newGroup("");

    for (isize i = 1; i <= 2; i++) {

        string nr = (i == 1) ? "1" : "2";

        root.add({"joystick", nr},
                 "Joystick in port " + nr);

        root.oldadd({"joystick", nr, ""},
                 "Displays the current configuration",
                 &RetroShell::exec <Token::joystick, Token::config>, i);
        
        root.add({"joystick", nr, "set"},
                 "Configures the component");
        
        root.oldadd({"joystick", nr, "set", "autofire"}, { Arg::boolean },
                 "Enables or disables auto-fire mode",
                 &RetroShell::exec <Token::joystick, Token::set, Token::autofire>, i);
        
        root.oldadd({"joystick", nr, "set", "bullets"},  { Arg::value },
                 "Sets the number of bullets per auto-fire shot",
                 &RetroShell::exec <Token::joystick, Token::set, Token::bullets>, i);
        
        root.oldadd({"joystick", nr, "set", "velocity"}, { Arg::value },
                 "Configures the auto-fire delay",
                 &RetroShell::exec <Token::joystick, Token::set, Token::delay>, i);

        root.oldadd({"joystick", nr, "press"}, { Arg::value },
                 "Presses a joystick button",
                 &RetroShell::exec <Token::joystick, Token::press>, i);

        root.oldadd({"joystick", nr, "unpress"}, { Arg::value },
                 "Releases a joystick button",
                 &RetroShell::exec <Token::joystick, Token::unpress>, i);

        root.add({"joystick", nr, "pull"},
                 "Pulls the joystick");

        root.oldadd({"joystick", nr, "pull", "left"},
                 "Pulls the joystick left",
                 &RetroShell::exec <Token::joystick, Token::pull, Token::left>, i);
        
        root.oldadd({"joystick", nr, "pull", "right"},
                 "Pulls the joystick right",
                 &RetroShell::exec <Token::joystick, Token::pull, Token::right>, i);

        root.oldadd({"joystick", nr, "pull", "up"},
                 "Pulls the joystick up",
                 &RetroShell::exec <Token::joystick, Token::pull, Token::up>, i);

        root.oldadd({"joystick", nr, "pull", "down"},
                 "Pulls the joystick down",
                 &RetroShell::exec <Token::joystick, Token::pull, Token::down>, i);

        root.add({"joystick", nr, "release"},
                 "Release a joystick axis");

        root.oldadd({"joystick", nr, "release", "x"},
                 "Releases the x-axis",
                 &RetroShell::exec <Token::joystick, Token::release, Token::xaxis>, i);

        root.oldadd({"joystick", nr, "release", "y"},
                 "Releases the y-axis",
                 &RetroShell::exec <Token::joystick, Token::release, Token::yaxis>, i);
    }
    
    
    //
    // Serial port
    //

    root.newGroup("");

    root.oldadd({"serial", ""},
             "Displays the current configuration",
             &RetroShell::exec <Token::serial, Token::config>);

    root.add({"serial", "set"},
             "Configures the component");

    root.oldadd({"serial", "set", "device"}, { SerialPortDeviceEnum::argList() },
             "Connects a device",
             &RetroShell::exec <Token::serial, Token::set, Token::device>);


    //
    // Df0, Df1, Df2, Df3
    //

    root.newGroup("");

    for (isize i = 0; i < 4; i++) {

        string df = "df" + std::to_string(i);

        root.oldadd({df, ""},
                 "Displays the current configuration",
                 &RetroShell::exec <Token::dfn, Token::config>, i);
        
        root.oldadd({df, "connect"},
                 "Connects the drive",
                 &RetroShell::exec <Token::dfn, Token::connect>, i);
        root.seek("df0")->remove("connect");
        
        root.oldadd({df, "disconnect"},
                 "Disconnects the drive",
                 &RetroShell::exec <Token::dfn, Token::disconnect>, i);
        root.seek("df0")->remove("disconnect");
        
        root.oldadd({df, "eject"},
                 "Ejects a floppy disk",
                 &RetroShell::exec <Token::dfn, Token::eject>, i);
        
        root.oldadd({df, "insert"}, { Arg::path },
                 "Inserts a floppy disk",
                 &RetroShell::exec <Token::dfn, Token::insert>, i);
    }
    
    for (isize i = 0; i < 5; i++) {

        string df = i < 4 ? "df" + std::to_string(i) : "dfn";

        root.add({df, "set"},
                 "Configures the component");
        
        root.oldadd({df, "set", "model"}, { FloppyDriveTypeEnum::argList() },
                 "Selects the drive model",
                 &RetroShell::exec <Token::dfn, Token::set, Token::model>, i);

        root.oldadd({df, "set", "rpm"}, { "rpm" },
                 "Sets the disk rotation speed",
                 &RetroShell::exec <Token::dc, Token::set, Token::rpm>);

        root.oldadd({df, "set", "mechanics"}, { Arg::boolean },
                 "Enables or disables the emulation of mechanical delays",
                 &RetroShell::exec <Token::dfn, Token::set, Token::mechanics>, i);
        
        root.oldadd({df, "set", "searchpath"}, { Arg::path },
                 "Sets the search path for media files",
                 &RetroShell::exec <Token::dfn, Token::set, Token::searchpath>, i);

        root.oldadd({df, "set", "swapdelay"}, { Arg::value },
                 "Sets the disk change delay",
                 &RetroShell::exec <Token::dfn, Token::set, Token::swapdelay>, i);

        root.oldadd({df, "set", "pan"}, { Arg::value },
                 "Sets the pan for drive sounds",
                 &RetroShell::exec <Token::dfn, Token::set, Token::pan>, i);
        
        root.add({df, "audiate"},
                 "Sets the volume of drive sounds");
        
        root.oldadd({df, "audiate", "insert"}, { Arg::volume },
                 "Makes disk insertions audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::insert>, i);
        
        root.oldadd({df, "audiate", "eject"}, { Arg::volume },
                 "Makes disk ejections audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::eject>, i);
        
        root.oldadd({df, "audiate", "step"},  { Arg::volume },
                 "Makes head steps audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::step>, i);
        
        root.oldadd({df, "audiate", "poll"},  { Arg::volume },
                 "Makes polling clicks audible",
                 &RetroShell::exec <Token::dfn, Token::audiate, Token::poll>, i);
    }


    //
    // Hd0, Hd1, Hd2, Hd3
    //

    root.newGroup("");

    for (isize i = 0; i < 4; i++) {
        
        string hd = "hd" + std::to_string(i);

        root.oldadd({hd, ""},
                 "Displays the current configuration",
                 &RetroShell::exec <Token::hdn, Token::config>, i);
    }
    
    for (isize i = 0; i < 5; i++) {

        string hd = i < 4 ? "hd" + std::to_string(i) : "hdn";

        root.add({hd, "set"},
                 "Configures the component");
        
        root.oldadd({hd, "set", "pan"}, { Arg::value },
                 "Sets the pan for drive sounds",
                 &RetroShell::exec <Token::hdn, Token::set, Token::pan>, i);
        
        root.add({hd, "audiate"},
                 "Sets the volume of drive sounds");

        root.oldadd({hd, "audiate", "step"}, { Arg::volume },
                 "Makes head steps audible",
                 &RetroShell::exec <Token::hdn, Token::audiate, Token::step>, i);
    }
    
    for (isize i = 0; i < 4; i++) {

        string hd = "hd" + std::to_string(i);

        root.oldadd({hd, "set", "geometry"},  { Arg::value, Arg::value, Arg::value },
                 "Changes the disk geometry",
                 &RetroShell::exec <Token::hdn, Token::geometry>, i);
    }


    //
    // Remote server
    //

    root.newGroup("");

    root.oldadd({"server", ""},
             "Displays a server status summary",
             &RetroShell::exec <Token::server>);

    root.add({"server", "serial"},
             "Serial port server");

    root.oldadd({"server", "serial", "config"},
             "Displays the current configuration",
             &RetroShell::exec <Token::server, Token::serial, Token::config>);

    root.add({"server", "serial", "set"},
             "Configures the component");

    root.oldadd({"server", "serial", "set", "port"}, { Arg::value },
             "Assigns the port number",
             &RetroShell::exec <Token::server, Token::serial, Token::set, Token::port>);

    root.oldadd({"server", "serial", "set", "verbose"}, { Arg::boolean },
             "Switches verbose mode on or off",
             &RetroShell::exec <Token::server, Token::serial, Token::set, Token::verbose>);

    root.oldadd({"server", "serial", "inspect"},
             "Displays the internal state",
             &RetroShell::exec <Token::server, Token::serial, Token::inspect>);

    root.add({"server", "rshell"},
             "Retro shell server");

    root.oldadd({"server", "rshell", "start"},
             "Starts the retro shell server",
             &RetroShell::exec <Token::server, Token::rshell, Token::start>);

    root.oldadd({"server", "rshell", "stop"},
             "Stops the retro shell server",
             &RetroShell::exec <Token::server, Token::rshell, Token::stop>);

    root.oldadd({"server", "rshell", "disconnect"},
             "Disconnects a client",
             &RetroShell::exec <Token::server, Token::rshell, Token::disconnect>);

    root.oldadd({"server", "rshell", "config"},
             "Displays the current configuration",
             &RetroShell::exec <Token::server, Token::rshell, Token::config>);

    root.add({"server", "rshell", "set"},
             "Configures the component");

    root.oldadd({"server", "rshell", "set", "port"}, { Arg::value },
             "Assigns the port number",
             &RetroShell::exec <Token::server, Token::rshell, Token::set, Token::port>);

    root.oldadd({"server", "serial", "set", "verbose"}, { Arg::boolean },
             "Switches verbose mode on or off",
             &RetroShell::exec <Token::server, Token::rshell, Token::set, Token::verbose>);

    root.oldadd({"server", "rshell", "inspect"},
             "Displays the internal state",
             &RetroShell::exec <Token::server, Token::rshell, Token::inspect>);

    root.add({"server", "gdb"},
             "GDB server");

    root.oldadd({"server", "gdb", "attach"}, { Arg::process },
             "Attaches the GDB server to a process",
             &RetroShell::exec <Token::server, Token::gdb, Token::attach>);

    root.oldadd({"server", "gdb", "detach"},
             "Detaches the GDB server from a process",
             &RetroShell::exec <Token::server, Token::gdb, Token::detach>);

    root.oldadd({"server", "gdb", "config"},
             "Displays the current configuration",
             &RetroShell::exec <Token::server, Token::gdb, Token::config>);

    root.add({"server", "gdb", "set"},
             "Configures the component");

    root.oldadd({"server", "gdb", "set", "port"}, { Arg::value },
             "Assigns the port number",
             &RetroShell::exec <Token::server, Token::gdb, Token::set, Token::port>);

    root.oldadd({"server", "gdb", "set", "verbose"}, { Arg::boolean },
             "Switches verbose mode on or off",
             &RetroShell::exec <Token::server, Token::gdb, Token::set, Token::verbose>);

    root.oldadd({"server", "gdb", "inspect"},
             "Displays the internal state",
             &RetroShell::exec <Token::server, Token::gdb, Token::inspect>);

    // Hide some commands
    root.hide({"regression"});
    root.hide({"screenshot"});
    /*
    root.hide({"df0"});
    root.hide({"df1"});
    root.hide({"df2"});
    root.hide({"df3"});
    root.hide({"hd0"});
    root.hide({"hd1"});
    root.hide({"hd2"});
    root.hide({"hd3"});
    */
}

}
