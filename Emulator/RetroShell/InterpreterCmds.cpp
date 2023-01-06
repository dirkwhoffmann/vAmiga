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

    root.add({"."},
             "Enters or exists the debugger",
             [this](Arguments& argv, long value) {

        retroShell.clear();
        switchInterpreter();
        retroShell.welcome();
    });

    root.add({"clear"},
             "Clears the console window",
             [this](Arguments& argv, long value) {

        retroShell.clear();
    });

    root.add({"close"},
             "Hides the console window",
             [this](Arguments& argv, long value) {

        msgQueue.put(MSG_CLOSE_CONSOLE);
    });

    root.add({"help"}, { }, {Arg::command},
             "Prints usage information",
             [this](Arguments& argv, long value) {

        retroShell.help(argv.empty() ? "" : argv.front());
    });

    root.add({"joshua"},
             "",
             [this](Arguments& argv, long value) {

        retroShell << "\nGREETINGS PROFESSOR HOFFMANN.\n";
        retroShell << "THE ONLY WINNING MOVE IS NOT TO PLAY.\n";
        retroShell << "HOW ABOUT A NICE GAME OF CHESS?\n\n";
    });

    root.add({"source"}, {Arg::path},
             "Processes a command script",
             [this](Arguments& argv, long value) {

        auto stream = std::ifstream(argv.front());
        if (!stream.is_open()) throw VAError(ERROR_FILE_NOT_FOUND, argv.front());
        retroShell.execScript(stream);
    });

    root.add({"wait"}, {Arg::value, Arg::seconds},
             "Pauses the execution of a command script",
             [this](Arguments& argv, long value) {

        auto seconds = parseNum(argv);
        retroShell.wakeUp = agnus.clock + SEC(seconds);
        throw ScriptInterruption(seconds);
    });
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
    root.add({"joystick"},      "Joystick");
    root.add({"mouse"},         "Mouse");
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

    root.add({"regression", "setup"}, { ConfigSchemeEnum::argList() }, { Arg::path, Arg::path },
             "Initializes the test environment",
             [this](Arguments& argv, long value) {

        auto scheme = parseEnum<ConfigSchemeEnum>(argv);
        auto rom = argv.size() > 1 ? argv[1] : "";
        auto ext = argv.size() > 2 ? argv[2] : "";

        amiga.regressionTester.prepare(scheme, rom, ext);
    });

    root.add({"regression", "run"}, { Arg::path },
             "Launches a regression test",
             [this](Arguments& argv, long value) {

        amiga.regressionTester.run(argv.front());
    });

    root.add({"screenshot", "set"},
             "Configures the screenshot");

    root.add({"screenshot", "set", "filename"}, { Arg::path },
             "Assigns the screen shot filename",
             [this](Arguments& argv, long value) {

        amiga.regressionTester.dumpTexturePath = argv.front();
    });

    root.add({"screenshot", "set", "cutout"}, { Arg::value, Arg::value, Arg::value, Arg::value },
             "Adjusts the texture cutout",
             [this](Arguments& argv, long value) {

        isize x1 = parseNum(argv, 0);
        isize y1 = parseNum(argv, 1);
        isize x2 = parseNum(argv, 2);
        isize y2 = parseNum(argv, 3);

        amiga.regressionTester.x1 = x1;
        amiga.regressionTester.y1 = y1;
        amiga.regressionTester.x2 = x2;
        amiga.regressionTester.y2 = y2;
    });

    root.add({"screenshot", "save"}, { Arg::path },
             "Saves a screenshot and exits the emulator",
             [this](Arguments& argv, long value) {

        amiga.regressionTester.dumpTexture(amiga, argv.front());
    });

    
    //
    // Amiga
    //

    root.newGroup("");

    root.add({"amiga", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(amiga);
    });

    root.add({"amiga", "defaults"},
             "Displays the user defaults storage",
             [this](Arguments& argv, long value) {

        retroShell.dump(amiga, Category::Defaults);
    });

    root.add({"amiga", "set"},
             "Configures the component");

    root.add({"amiga", "set", "type"}, { VideoFormatEnum::argList() },
             "Selects the video standard",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_VIDEO_FORMAT, parseEnum <VideoFormatEnum> (argv));
    });

    root.add({"amiga", "set", "syncmode"}, { SyncModeEnum::argList() },
             "Selects the synchronization mode",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_SYNC_MODE, parseEnum <SyncModeEnum> (argv));
    });

    root.add({"amiga", "set", "fps"}, { Arg::value },
             "Sets the frames per seconds",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_PROPOSED_FPS, parseNum(argv));
        amiga.configure(OPT_SYNC_MODE, SYNC_FIXED_FPS);
    });

    root.add({"amiga", "init"}, { ConfigSchemeEnum::argList() },
             "Initializes the Amiga with a predefined scheme",
             [this](Arguments& argv, long value) {

        auto scheme = parseEnum <ConfigSchemeEnum> (argv);

        amiga.revertToFactorySettings();
        amiga.configure(scheme);
    });

    root.add({"amiga", "power"}, { Arg::onoff },
             "Switches the Amiga on or off",
             [this](Arguments& argv, long value) {

        parseOnOff(argv) ? amiga.run() : amiga.powerOff();
    });

    root.add({"amiga", "reset"},
             "Performs a hard reset",
             [this](Arguments& argv, long value) {

        amiga.reset(true);
    });


    //
    // Memory
    //

    root.newGroup("");

    root.add({"memory", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(amiga.mem);
    });

    root.add({"memory", "set"},
             "Configures the component");

    root.add({"memory", "set", "chip"}, { Arg::kb },
             "Configures the amouts of chip memory",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_CHIP_RAM, parseNum(argv));
    });

    root.add({"memory", "set", "slow"},  { Arg::kb },
             "Configures the amouts of slow memory",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_SLOW_RAM, parseNum(argv));
    });

    root.add({"memory", "set", "fast"}, { Arg::kb },
             "Configures the amouts of flow memory",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_FAST_RAM, parseNum(argv));
    });

    root.add({"memory", "set", "extstart"}, { Arg::address },
             "Sets the start address for Rom extensions",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_EXT_START, parseNum(argv));
    });

    root.add({"memory", "set", "saveroms"}, { Arg::boolean },
             "Determines whether Roms should be stored in snapshots",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_SAVE_ROMS, parseBool(argv));
    });

    root.add({"memory", "set", "slowramdelay"}, { Arg::boolean },
             "Enables or disables slow Ram bus delays",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_SLOW_RAM_DELAY, parseBool(argv));
    });

    root.add({"memory", "set", "bankmap"}, { BankMapEnum::argList() },
             "Selects the bank mapping scheme",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_BANKMAP, parseEnum <BankMapEnum> (argv));
    });

    root.add({"memory", "set", "raminit"}, { RamInitPatternEnum::argList() },
             "Determines how Ram is initialized on startup",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_RAM_INIT_PATTERN, parseEnum <RamInitPatternEnum> (argv));
    });

    root.add({"memory", "set", "unmapped"}, { UnmappedMemoryEnum::argList() },
             "Determines the behaviour of unmapped memory",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_UNMAPPING_TYPE, parseEnum <UnmappedMemoryEnum> (argv));
    });

    root.add({"memory", "load"},
             "Installs a Rom image");

    root.add({"memory", "load", "rom"}, { Arg::path },
             "Installs a Kickstart Rom",
             [this](Arguments& argv, long value) {

        amiga.mem.loadRom(argv.front());
    });

    root.add({"memory", "load", "extrom"}, { Arg::path },
             "Installs a Rom extension",
             [this](Arguments& argv, long value) {

        amiga.mem.loadExt(argv.front());
    });

    
    //
    // CPU
    //

    root.newGroup("");

    root.add({"cpu", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(amiga.cpu);
    });

    root.add({"cpu", "set"},
             "Configures the component");

    root.add({"cpu", "set", "revision"}, { CPURevisionEnum::argList() },
             "Selects the emulated chip model",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_CPU_REVISION, parseEnum <CPURevisionEnum> (argv));
    });

    root.add({"cpu", "set", "dasm"},
             "Configures the disassembler");

    root.add({"cpu", "set", "dasm", "revision"}, { DasmRevisionEnum::argList() },
             "Selects the disassembler instruction set",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_CPU_DASM_REVISION, parseEnum <DasmRevisionEnum> (argv));
    });

    root.add({"cpu", "set", "dasm", "syntax"}, {  DasmSyntaxEnum::argList() },
             "Selects the disassembler syntax style",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_CPU_DASM_SYNTAX, parseEnum <DasmSyntaxEnum> (argv));
    });

    root.add({"cpu", "set", "overclocking"}, { Arg::value },
             "Overclocks the CPU by the specified factor",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_CPU_OVERCLOCKING, parseNum(argv));
    });

    root.add({"cpu", "set", "regreset"}, { Arg::value },
             "Selects the reset value of data and address registers",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_CPU_RESET_VAL, parseNum(argv));
    });


    //
    // CIA
    //

    root.newGroup("");

    for (isize i = 0; i < 2; i++) {
        
        string cia = (i == 0) ? "ciaa" : "ciab";
        
        root.add({cia, ""},
                 "Displays the current configuration",
                 [this](Arguments& argv, long value) {

            if (value == 0) retroShell.dumpConfig(amiga.ciaA);
            if (value == 1) retroShell.dumpConfig(amiga.ciaB);

        }, i);

        root.add({cia, "set"},
                 "Configures the component");
        
        root.add({cia, "set", "revision"}, { CIARevisionEnum::argList() },
                 "Selects the emulated chip model",
                 [this](Arguments& argv, long value) {

            auto parsed = parseEnum <CIARevisionEnum> (argv);
            amiga.configure(OPT_CIA_REVISION, value, parsed);

        }, i);

        root.add({cia, "set", "todbug"}, { Arg::boolean },
                 "Enables or disables the TOD hardware bug",
                 [this](Arguments& argv, long value) {

            auto parsed = parseBool(argv);
            amiga.configure(OPT_TODBUG, value, parsed);

        }, i);

        root.add({cia, "set", "esync"}, { Arg::boolean },
                 "Turns E-clock syncing on or off",
                 [this](Arguments& argv, long value) {

            auto parsed = parseBool(argv);
            amiga.configure(OPT_ECLOCK_SYNCING, value, parsed);

        }, i);
    }
    
    
    //
    // Agnus
    //

    root.newGroup("");

    root.add({"agnus", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(agnus);
    });

    root.add({"agnus", "set"},
             "Configures the component");

    root.add({"agnus", "set", "revision"}, { AgnusRevisionEnum::argList() },
             "Selects the emulated chip model",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_AGNUS_REVISION, parseEnum <AgnusRevisionEnum> (argv));
    });

    root.add({"agnus", "set", "slowrammirror"}, { Arg::boolean },
             "Enables or disables ECS Slow Ram mirroring",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_SLOW_RAM_MIRROR, parseBool(argv));
    });

    root.add({"agnus", "set", "ptrdrops"}, { Arg::boolean },
             "Emulate dropped register writes",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_PTR_DROPS, parseBool(argv));
    });

    
    //
    // Blitter
    //

    root.newGroup("");

    root.add({"blitter", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(blitter);
    });

    root.add({"blitter", "set"},
             "Configures the component");

    root.add({"blitter", "set", "accuracy"}, { "1..3" },
             "Selects the emulation accuracy level",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_BLITTER_ACCURACY, parseNum(argv));
    });

    
    //
    // Denise
    //

    root.newGroup("");

    root.add({"denise", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(amiga.denise);
    });

    root.add({"denise", "set"},
             "Configures the component");

    root.add({"denise", "set", "revision"}, { DeniseRevisionEnum::argList() },
             "Selects the emulated chip model",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_DENISE_REVISION, parseEnum <DeniseRevisionEnum> (argv));
    });

    root.add({"denise", "set", "tracking"}, { Arg::boolean },
             "Enables or disables viewport tracking",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_VIEWPORT_TRACKING, parseBool(argv));
    });

    root.add({"denise", "set", "clxsprspr"}, { Arg::boolean },
             "Switches sprite-sprite collision detection on or off",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_CLX_SPR_SPR, parseBool(argv));
    });

    root.add({"denise", "set", "clxsprplf"}, { Arg::boolean },
             "Switches sprite-playfield collision detection on or off",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_CLX_SPR_PLF, parseBool(argv));
    });

    root.add({"denise", "set", "clxplfplf"}, { Arg::boolean },
             "Switches playfield-playfield collision detection on or off",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_CLX_PLF_PLF, parseBool(argv));
    });

    root.add({"denise", "set", "hidden"},
             "Hides bitplanes, sprites, or layers");

    root.add({"denise", "set", "hidden", "bitplanes"}, { Arg::value },
             "Wipes out certain bitplane data",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_HIDDEN_BITPLANES, parseNum(argv));
    });

    root.add({"denise", "set", "hidden", "sprites"}, { Arg::value },
             "Wipes out certain sprite data",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_HIDDEN_SPRITES, parseNum(argv));
    });

    root.add({"denise", "set", "hidden", "layers"}, { Arg::value },
             "Makes certain drawing layers transparent",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_HIDDEN_LAYERS, parseNum(argv));
    });

    
    //
    // DMA Debugger
    //

    root.add({"dmadebugger", "open"},
             "Opens the DMA debugger",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_DMA_DEBUG_ENABLE, true);
    });

    root.add({"dmadebugger", "close"},
             "Closes the DMA debugger",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_DMA_DEBUG_ENABLE, false);
    });

    root.add({"dmadebugger", "copper"}, { Arg::onoff },
             "Turns Copper DMA visualization on or off",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_COPPER, parseBool(argv));
    });

    root.add({"dmadebugger", "blitter"}, { Arg::onoff },
             "Turns Blitter DMA visualization on or off",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_BLITTER, parseBool(argv));
    });

    root.add({"dmadebugger", "disk"}, { Arg::onoff },
             "Turns Disk DMA visualization on or off",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_DISK, parseBool(argv));
    });

    root.add({"dmadebugger", "audio"}, { Arg::onoff },
             "Turns Audio DMA visualization on or off",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_AUDIO, parseBool(argv));
    });

    root.add({"dmadebugger", "sprites"}, { Arg::onoff },
             "Turns Sprite DMA visualization on or off",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_SPRITE, parseBool(argv));
    });

    root.add({"dmadebugger", "bitplanes"}, { Arg::onoff },
             "Turns Bitplane DMA visualization on or off",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_BITPLANE, parseBool(argv));
    });

    root.add({"dmadebugger", "cpu"}, { Arg::onoff },
             "Turns CPU bus usage visualization on or off",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_CPU, parseBool(argv));
    });

    root.add({"dmadebugger", "refresh"}, { Arg::onoff },
             "Turn memory refresh visualization on or off",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_DMA_DEBUG_CHANNEL, DMA_CHANNEL_REFRESH, parseBool(argv));
    });


    //
    // Monitor
    //

    root.add({"monitor", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(amiga.denise.pixelEngine);
    });

    root.add({"monitor", "set"},
             "Configures the component");

    root.add({"monitor", "set", "palette"}, { PaletteEnum::argList() },
             "Selects the color palette",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_PALETTE, parseEnum <PaletteEnum> (argv));
    });

    root.add({"monitor", "set", "brightness"}, { Arg::value },
             "Adjusts the brightness of the Amiga texture",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_BRIGHTNESS, parseNum(argv));
    });

    root.add({"monitor", "set", "contrast"}, { Arg::value },
             "Adjusts the contrast of the Amiga texture",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_CONTRAST, parseNum(argv));
    });

    root.add({"monitor", "set", "saturation"}, { Arg::value },
             "Adjusts the saturation of the Amiga texture",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_SATURATION, parseNum(argv));
    });

    
    //
    // Paula (Audio)
    //

    root.newGroup("");

    root.add({"paula", "audio"},
             "Audio unit");

    root.add({"paula", "audio", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(amiga.paula.muxer);
    });

    root.add({"paula", "audio", "filter"},
             "Displays the current filter configuration",
             [this](Arguments& argv, long value) {

        retroShell << '\n';

        retroShell << "Left channel:" << '\n';
        retroShell.dumpConfig(paula.muxer.filterL);

        retroShell << "Right channel:" << '\n';
        retroShell.dumpConfig(paula.muxer.filterR);
    });

    root.add({"paula", "audio", "filter", "set"},
             "Configures the audio filter");

    root.add({"paula", "audio", "filter", "set", "type"}, { FilterTypeEnum::argList() },
             "Configures the audio filter type",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_FILTER_TYPE, parseEnum <FilterTypeEnum> (argv));
    });

    root.add({"paula", "audio", "filter", "set", "activation"}, { FilterActivationEnum::argList() },
             "Selects the filter activation condition",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_FILTER_ACTIVATION, parseEnum <FilterActivationEnum> (argv));
    });

    root.add({"paula", "audio", "set"},
             "Configures the component");

    root.add({"paula", "audio", "set", "sampling"}, { SamplingMethodEnum::argList() },
             "Selects the sampling method",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_SAMPLING_METHOD, parseEnum <SamplingMethodEnum> (argv));
    });

    root.add({"paula", "audio", "set", "volume"},
             "Sets the volume");

    root.add({"paula", "audio", "set", "volume", "channel0"}, { Arg::volume },
             "Sets the volume for audio channel 0",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_AUDVOL, 0, parseNum(argv));
    });

    root.add({"paula", "audio", "set", "volume", "channel1"}, { Arg::volume },
             "Sets the volume for audio channel 1",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_AUDVOL, 1, parseNum(argv));
    });

    root.add({"paula", "audio", "set", "volume", "channel2"}, { Arg::volume },
             "Sets the volume for audio channel 2",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_AUDVOL, 2, parseNum(argv));
    });

    root.add({"paula", "audio", "set", "volume", "channel3"}, { Arg::volume },
             "Sets the volume for audio channel 3",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_AUDVOL, 3, parseNum(argv));
    });

    root.add({"paula", "audio", "set", "volume", "left"}, { Arg::volume },
             "Sets the master volume for the left speaker",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_AUDVOLL, parseNum(argv));
    });

    root.add({"paula", "audio", "set", "volume", "right"}, { Arg::volume },
             "Sets the master volume for the right speaker",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_AUDVOLR, parseNum(argv));
    });

    root.add({"paula", "audio", "set", "pan"},
             "Sets the pan for one of the four audio channels");
    
    root.add({"paula", "audio", "set", "pan", "channel0"}, { Arg::value },
             "Sets the pan for audio channel 0",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_AUDPAN, 0, parseNum(argv));
    });

    root.add({"paula", "audio", "set", "pan", "channel1"}, { Arg::value },
             "Sets the pan for audio channel 1",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_AUDPAN, 1, parseNum(argv));
    });

    root.add({"paula", "audio", "set", "pan", "channel2"}, { Arg::value },
             "Sets the pan for audio channel 2",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_AUDPAN, 2, parseNum(argv));
    });

    root.add({"paula", "audio", "set", "pan", "channel3"}, { Arg::value },
             "Sets the pan for audio channel 3",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_AUDPAN, 3, parseNum(argv));
    });


    //
    // Paula (Disk controller)
    //

    root.add({"paula", "dc"},
             "Disk controller");

    root.add({"paula", "dc", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(amiga.paula.diskController);
    });

    root.add({"paula", "dc", "set"},
             "Configures the component");

    root.add({"paula", "dc", "set", "speed"}, { Arg::value },
             "Configures the data transfer speed",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_DRIVE_SPEED, parseNum(argv));
    });

    root.add({"paula", "dc", "dsksync"},
             "Secures the DSKSYNC register");

    root.add({"paula", "dc", "dsksync", "auto"}, { Arg::boolean },
             "Always receive a SYNC signal",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_AUTO_DSKSYNC, parseBool(argv));
    });

    root.add({"paula", "dc", "dsksync", "lock"}, { Arg::boolean },
             "Prevents writes to DSKSYNC",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_LOCK_DSKSYNC, parseBool(argv));
    });


    //
    // RTC
    //

    root.newGroup("");

    root.add({"rtc", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(amiga.rtc);
    });

    root.add({"rtc", "set"},
             "Configures the component");

    root.add({"rtc", "set", "revision"}, { RTCRevisionEnum::argList() },
             "Selects the emulated chip model",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_RTC_MODEL, parseEnum <RTCRevisionEnum> (argv));
    });


    //
    // Keyboard
    //

    root.newGroup("");

    root.add({"keyboard", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(amiga.keyboard);
    });

    root.add({"keyboard", "set"},
             "Configures the component");

    root.add({"keyboard", "set", "accuracy"}, { Arg::value },
             "Determines the emulation accuracy level",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_ACCURATE_KEYBOARD, parseBool(argv));
    });

    root.add({"keyboard", "press"}, { Arg::value },
             "Sends a keycode to the keyboard",
             [this](Arguments& argv, long value) {

        keyboard.autoType(KeyCode(parseNum(argv)));
    });


    //
    // Joystick
    //

    root.newGroup("");

    for (isize i = ControlPort::PORT1; i <= ControlPort::PORT2; i++) {

        string nr = (i == ControlPort::PORT1) ? "1" : "2";

        root.add({"joystick", nr},
                 "Joystick in port " + nr);

        root.add({"joystick", nr, ""},
                 "Displays the current configuration",
                 [this](Arguments& argv, long value) {

            auto &port = (value == ControlPort::PORT1) ? amiga.controlPort1 : amiga.controlPort2;
            retroShell.dumpConfig(port.joystick);

        }, i);

        root.add({"joystick", nr, "set"},
                 "Configures the component");
        
        root.add({"joystick", nr, "set", "autofire"}, { Arg::boolean },
                 "Enables or disables auto-fire mode",
                 [this](Arguments& argv, long value) {

            auto port = (value == ControlPort::PORT1) ? ControlPort::PORT1 : ControlPort::PORT2;
            amiga.configure(OPT_AUTOFIRE, port, parseBool(argv));

        }, i);

        root.add({"joystick", nr, "set", "bullets"},  { Arg::value },
                 "Sets the number of bullets per auto-fire shot",
                 [this](Arguments& argv, long value) {

            auto port = (value == ControlPort::PORT1) ? ControlPort::PORT1 : ControlPort::PORT2;
            amiga.configure(OPT_AUTOFIRE_BULLETS, port, parseNum(argv));

        }, i);

        root.add({"joystick", nr, "set", "delay"}, { Arg::value },
                 "Configures the auto-fire delay",
                 [this](Arguments& argv, long value) {

            auto port = (value == ControlPort::PORT1) ? ControlPort::PORT1 : ControlPort::PORT2;
            amiga.configure(OPT_AUTOFIRE_DELAY, port, parseNum(argv));

        }, i);

        root.add({"joystick", nr, "press"}, { Arg::value },
                 "Presses a joystick button",
                 [this](Arguments& argv, long value) {

            auto &port = (value == ControlPort::PORT1) ? amiga.controlPort1 : amiga.controlPort2;
            auto nr = parseNum(argv);

            switch (nr) {

                case 1: port.joystick.trigger(PRESS_FIRE); break;
                case 2: port.joystick.trigger(PRESS_FIRE2); break;
                case 3: port.joystick.trigger(PRESS_FIRE3); break;

                default:
                    throw VAError(ERROR_OPT_INVARG, "1...3");
            }

        }, i);

        root.add({"joystick", nr, "unpress"}, { Arg::value },
                 "Releases a joystick button",
                 [this](Arguments& argv, long value) {

            auto &port = (value == ControlPort::PORT1) ? amiga.controlPort1 : amiga.controlPort2;
            auto nr = parseNum(argv);

            switch (nr) {

                case 1: port.joystick.trigger(RELEASE_FIRE); break;
                case 2: port.joystick.trigger(RELEASE_FIRE2); break;
                case 3: port.joystick.trigger(RELEASE_FIRE3); break;

                default:
                    throw VAError(ERROR_OPT_INVARG, "1...3");
            }

        }, i);

        root.add({"joystick", nr, "pull"},
                 "Pulls the joystick");

        root.add({"joystick", nr, "pull", "left"},
                 "Pulls the joystick left",
                 [this](Arguments& argv, long value) {

            auto &port = (value == ControlPort::PORT1) ? amiga.controlPort1 : amiga.controlPort2;
            port.joystick.trigger(PULL_LEFT);

        }, i);

        root.add({"joystick", nr, "pull", "right"},
                 "Pulls the joystick right",
                 [this](Arguments& argv, long value) {

            auto &port = (value == ControlPort::PORT1) ? amiga.controlPort1 : amiga.controlPort2;
            port.joystick.trigger(PULL_RIGHT);

        }, i);

        root.add({"joystick", nr, "pull", "up"},
                 "Pulls the joystick up",
                 [this](Arguments& argv, long value) {

            auto &port = (value == ControlPort::PORT1) ? amiga.controlPort1 : amiga.controlPort2;
            port.joystick.trigger(PULL_UP);

        }, i);

        root.add({"joystick", nr, "pull", "down"},
                 "Pulls the joystick down",
                 [this](Arguments& argv, long value) {

            auto &port = (value == ControlPort::PORT1) ? amiga.controlPort1 : amiga.controlPort2;
            port.joystick.trigger(PULL_DOWN);

        }, i);

        root.add({"joystick", nr, "release"},
                 "Release a joystick axis");

        root.add({"joystick", nr, "release", "x"},
                 "Releases the x-axis",
                 [this](Arguments& argv, long value) {

            auto &port = (value == ControlPort::PORT1) ? amiga.controlPort1 : amiga.controlPort2;
            port.joystick.trigger(RELEASE_X);

        }, i);

        root.add({"joystick", nr, "release", "y"},
                 "Releases the y-axis",
                 [this](Arguments& argv, long value) {

            auto &port = (value == ControlPort::PORT1) ? amiga.controlPort1 : amiga.controlPort2;
            port.joystick.trigger(RELEASE_Y);

        }, i);
    }


    //
    // Mouse
    //

    root.newGroup("");

    for (isize i = ControlPort::PORT1; i <= ControlPort::PORT2; i++) {

        string nr = (i == ControlPort::PORT1) ? "1" : "2";

        root.add({"mouse", nr},
                 "Mouse in port " + nr);

        root.add({"mouse", nr, ""},
                 "Displays the current configuration",
                 [this](Arguments& argv, long value) {

            auto &port = (value == ControlPort::PORT1) ? amiga.controlPort1 : amiga.controlPort2;
            retroShell.dumpConfig(port.mouse);

        }, i);

        root.add({"mouse", nr, "set"},
                 "Configures the component");

        root.add({"mouse", nr, "set", "pullup"}, { Arg::boolean },
                 "Enables or disables pull-up resistors",
                 [this](Arguments& argv, long value) {

            auto port = (value == ControlPort::PORT1) ? ControlPort::PORT1 : ControlPort::PORT2;
            amiga.configure(OPT_PULLUP_RESISTORS, port, parseBool(argv));

        }, i);

        root.add({"mouse", nr, "set", "shakedetector"}, { Arg::boolean },
                 "Enables or disables the shake detector",
                 [this](Arguments& argv, long value) {

            auto port = (value == ControlPort::PORT1) ? ControlPort::PORT1 : ControlPort::PORT2;
            amiga.configure(OPT_SHAKE_DETECTION, port, parseBool(argv));

        }, i);

        root.add({"mouse", nr, "set", "velocity"}, { Arg::value },
                 "Sets the horizontal and vertical mouse velocity",
                 [this](Arguments& argv, long value) {

            auto port = (value == ControlPort::PORT1) ? ControlPort::PORT1 : ControlPort::PORT2;
            amiga.configure(OPT_MOUSE_VELOCITY, port, parseNum(argv));

        }, i);

        root.add({"mouse", nr, "press"},
                 "Presses a mouse button");

        root.add({"mouse", nr, "press", "left"},
                 "Presses the left mouse button",
                 [this](Arguments& argv, long value) {

            auto &port = (value == ControlPort::PORT1) ? amiga.controlPort1 : amiga.controlPort2;
            port.mouse.pressAndReleaseLeft();

        }, i);

        root.add({"mouse", nr, "press", "middle"},
                 "Presses the middle mouse button",
                 [this](Arguments& argv, long value) {

            auto &port = (value == ControlPort::PORT1) ? amiga.controlPort1 : amiga.controlPort2;
            port.mouse.pressAndReleaseMiddle();

        }, i);

        root.add({"mouse", nr, "press", "right"},
                 "Presses the right mouse button",
                 [this](Arguments& argv, long value) {

            auto &port = (value == ControlPort::PORT1) ? amiga.controlPort1 : amiga.controlPort2;
            port.mouse.pressAndReleaseRight();

        }, i);
    }


    //
    // Serial port
    //

    root.newGroup("");

    root.add({"serial", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(amiga.serialPort);
    });

    root.add({"serial", "set"},
             "Configures the component");

    root.add({"serial", "set", "device"}, { SerialPortDeviceEnum::argList() },
             "Connects a device",
             [this](Arguments& argv, long value) {

        amiga.configure(OPT_SERIAL_DEVICE, parseEnum <SerialPortDeviceEnum> (argv));
    });


    //
    // Df0, Df1, Df2, Df3
    //

    root.newGroup("");

    for (isize i = 0; i <= 4; i++) {

        string df = i == 4 ? "dfn" : "df" + std::to_string(i);

        if (i >= 0 && i <= 3) {

            root.add({df, ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dumpConfig(*amiga.df[value]);

            }, i);

            if (i >= 1 && i <= 3) {

                root.add({df, "connect"},
                         "Connects the drive",
                         [this](Arguments& argv, long value) {

                    amiga.configure(OPT_DRIVE_CONNECT, value, true);

                }, i);

                root.add({df, "disconnect"},
                         "Disconnects the drive",
                         [this](Arguments& argv, long value) {

                    amiga.configure(OPT_DRIVE_CONNECT, value, false);

                }, i);
            }
            root.add({df, "eject"},
                     "Ejects a floppy disk",
                     [this](Arguments& argv, long value) {

                amiga.df[value]->ejectDisk();

            }, i);

            root.add({df, "insert"}, { Arg::path },
                     "Inserts a floppy disk",
                     [this](Arguments& argv, long value) {
                
                auto path = argv.front();
                amiga.df[value]->swapDisk(path);

            }, i);

            root.add({df, "set"},
                     "Configures the component");

            root.add({df, "set", "model"}, { FloppyDriveTypeEnum::argList() },
                     "Selects the drive model",
                     [this](Arguments& argv, long value) {

                long model = parseEnum <FloppyDriveTypeEnum> (argv);

                if (value == 0 || value > 3) amiga.configure(OPT_DRIVE_TYPE, 0, model);
                if (value == 1 || value > 3) amiga.configure(OPT_DRIVE_TYPE, 1, model);
                if (value == 2 || value > 3) amiga.configure(OPT_DRIVE_TYPE, 2, model);
                if (value == 3 || value > 3) amiga.configure(OPT_DRIVE_TYPE, 3, model);

            }, i);

            root.add({df, "set", "rpm"}, { "rpm" },
                     "Sets the disk rotation speed",
                     [this](Arguments& argv, long value) {

                long rpm = parseNum(argv);

                if (value == 0 || value > 3) amiga.configure(OPT_DRIVE_RPM, 0, rpm);
                if (value == 1 || value > 3) amiga.configure(OPT_DRIVE_RPM, 1, rpm);
                if (value == 2 || value > 3) amiga.configure(OPT_DRIVE_RPM, 2, rpm);
                if (value == 3 || value > 3) amiga.configure(OPT_DRIVE_RPM, 3, rpm);

            }, i);

            root.add({df, "set", "mechanics"}, { Arg::boolean },
                     "Enables or disables the emulation of mechanical delays",
                     [this](Arguments& argv, long value) {

                auto scheme = parseEnum<DriveMechanicsEnum>(argv);

                if (value == 0 || value > 3) amiga.configure(OPT_DRIVE_MECHANICS, 0, scheme);
                if (value == 1 || value > 3) amiga.configure(OPT_DRIVE_MECHANICS, 1, scheme);
                if (value == 2 || value > 3) amiga.configure(OPT_DRIVE_MECHANICS, 2, scheme);
                if (value == 3 || value > 3) amiga.configure(OPT_DRIVE_MECHANICS, 3, scheme);

            }, i);

            root.add({df, "set", "searchpath"}, { Arg::path },
                     "Sets the search path for media files",
                     [this](Arguments& argv, long value) {

                string path = argv.front();

                if (value == 0 || value > 3) df0.setSearchPath(path);
                if (value == 1 || value > 3) df1.setSearchPath(path);
                if (value == 2 || value > 3) df2.setSearchPath(path);
                if (value == 3 || value > 3) df3.setSearchPath(path);

            }, i);

            root.add({df, "set", "swapdelay"}, { Arg::value },
                     "Sets the disk change delay",
                     [this](Arguments& argv, long value) {

                long delay = parseNum(argv);

                if (value == 0 || value > 3) amiga.configure(OPT_DISK_SWAP_DELAY, 0, delay);
                if (value == 1 || value > 3) amiga.configure(OPT_DISK_SWAP_DELAY, 1, delay);
                if (value == 2 || value > 3) amiga.configure(OPT_DISK_SWAP_DELAY, 2, delay);
                if (value == 3 || value > 3) amiga.configure(OPT_DISK_SWAP_DELAY, 3, delay);

            }, i);

            root.add({df, "set", "pan"}, { Arg::value },
                     "Sets the pan for drive sounds",
                     [this](Arguments& argv, long value) {

                long pan = parseNum(argv);

                if (value == 0 || value > 3) amiga.configure(OPT_DRIVE_PAN, 0, pan);
                if (value == 1 || value > 3) amiga.configure(OPT_DRIVE_PAN, 1, pan);
                if (value == 2 || value > 3) amiga.configure(OPT_DRIVE_PAN, 2, pan);
                if (value == 3 || value > 3) amiga.configure(OPT_DRIVE_PAN, 3, pan);

            }, i);

            root.add({df, "audiate"},
                     "Sets the volume of drive sounds");

            root.add({df, "audiate", "insert"}, { Arg::volume },
                     "Makes disk insertions audible",
                     [this](Arguments& argv, long value) {

                long volume = parseNum(argv);

                if (value == 0 || value > 3) amiga.configure(OPT_INSERT_VOLUME, 0, volume);
                if (value == 1 || value > 3) amiga.configure(OPT_INSERT_VOLUME, 1, volume);
                if (value == 2 || value > 3) amiga.configure(OPT_INSERT_VOLUME, 2, volume);
                if (value == 3 || value > 3) amiga.configure(OPT_INSERT_VOLUME, 3, volume);

            }, i);

            root.add({df, "audiate", "eject"}, { Arg::volume },
                     "Makes disk ejections audible",
                     [this](Arguments& argv, long value) {

                long volume = parseNum(argv);

                if (value == 0 || value > 3) amiga.configure(OPT_EJECT_VOLUME, 0, volume);
                if (value == 1 || value > 3) amiga.configure(OPT_EJECT_VOLUME, 1, volume);
                if (value == 2 || value > 3) amiga.configure(OPT_EJECT_VOLUME, 2, volume);
                if (value == 3 || value > 3) amiga.configure(OPT_EJECT_VOLUME, 3, volume);

            }, i);

            root.add({df, "audiate", "step"},  { Arg::volume },
                     "Makes head steps audible",
                     [this](Arguments& argv, long value) {

                long volume = parseNum(argv);

                if (value == 0 || value > 3) amiga.configure(OPT_STEP_VOLUME, 0, volume);
                if (value == 1 || value > 3) amiga.configure(OPT_STEP_VOLUME, 1, volume);
                if (value == 2 || value > 3) amiga.configure(OPT_STEP_VOLUME, 2, volume);
                if (value == 3 || value > 3) amiga.configure(OPT_STEP_VOLUME, 3, volume);

            }, i);

            root.add({df, "audiate", "poll"},  { Arg::volume },
                     "Makes polling clicks audible",
                     [this](Arguments& argv, long value) {

                long volume = parseNum(argv);

                if (value == 0 || value > 3) amiga.configure(OPT_POLL_VOLUME, 0, volume);
                if (value == 1 || value > 3) amiga.configure(OPT_POLL_VOLUME, 1, volume);
                if (value == 2 || value > 3) amiga.configure(OPT_POLL_VOLUME, 2, volume);
                if (value == 3 || value > 3) amiga.configure(OPT_POLL_VOLUME, 3, volume);

            }, i);
        }
    }


    //
    // Hd0, Hd1, Hd2, Hd3
    //

    root.newGroup("");

    for (isize i = 0; i <= 4; i++) {

        string hd = i == 4 ? "hdn" : "hd" + std::to_string(i);

        if (i != 4) {

            root.add({hd, ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dumpConfig(*amiga.hd[value]);

            }, i);

            root.add({hd, "connect"},
                     "Connects the hard drive",
                     [this](Arguments& argv, long value) {

                amiga.configure(OPT_HDC_CONNECT, value, true);

            }, i);

            root.add({hd, "disconnect"},
                     "Disconnects the hard drive",
                     [this](Arguments& argv, long value) {

                amiga.configure(OPT_HDC_CONNECT, value, false);

            }, i);

        }

        root.add({hd, "set"},
                 "Configures the component");
        
        root.add({hd, "set", "pan"}, { Arg::value },
                 "Sets the pan for drive sounds",
                 [this](Arguments& argv, long value) {

            long pan = parseNum(argv);

            if (value == 0 || value > 3) amiga.configure(OPT_HDR_PAN, 0, pan);
            if (value == 1 || value > 3) amiga.configure(OPT_HDR_PAN, 1, pan);
            if (value == 2 || value > 3) amiga.configure(OPT_HDR_PAN, 2, pan);
            if (value == 3 || value > 3) amiga.configure(OPT_HDR_PAN, 3, pan);

        }, i);

        root.add({hd, "audiate"},
                 "Sets the volume of drive sounds");

        root.add({hd, "audiate", "step"}, { Arg::volume },
                 "Makes head steps audible",
                 [this](Arguments& argv, long value) {

            long volume = parseNum(argv);

            if (value == 0 || value > 3) amiga.configure(OPT_HDR_STEP_VOLUME, 0, volume);
            if (value == 1 || value > 3) amiga.configure(OPT_HDR_STEP_VOLUME, 1, volume);
            if (value == 2 || value > 3) amiga.configure(OPT_HDR_STEP_VOLUME, 2, volume);
            if (value == 3 || value > 3) amiga.configure(OPT_HDR_STEP_VOLUME, 3, volume);

        }, i);

        if (i != 4) {

            root.add({hd, "set", "geometry"},  { "<cylinders>", "<heads>", "<sectors>" },
                     "Changes the disk geometry",
                     [this](Arguments& argv, long value) {

                auto c = util::parseNum(argv[0]);
                auto h = util::parseNum(argv[1]);
                auto s = util::parseNum(argv[2]);

                amiga.hd[value]->changeGeometry(c, h, s);

            }, i);
        }
    }


    //
    // Remote server
    //

    root.newGroup("");

    root.add({"server", ""},
             "Displays a server status summary",
             [this](Arguments& argv, long value) {

        retroShell.dump(remoteManager, Category::Status);
    });

    root.add({"server", "serial"},
             "Serial port server");

    root.add({"server", "serial", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dump(remoteManager, Category::Config);
    });

    root.add({"server", "serial", "set"},
             "Configures the component");

    root.add({"server", "serial", "set", "port"}, { Arg::value },
             "Assigns the port number",
             [this](Arguments& argv, long value) {

        remoteManager.serServer.setConfigItem(OPT_SRV_PORT, parseNum(argv));
    });

    root.add({"server", "serial", "set", "verbose"}, { Arg::boolean },
             "Switches verbose mode on or off",
             [this](Arguments& argv, long value) {

        remoteManager.rshServer.setConfigItem(OPT_SRV_VERBOSE, parseBool(argv));
    });

    root.add({"server", "serial", "inspect"},
             "Displays the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dumpDebug(remoteManager.rshServer);
    });

    root.add({"server", "rshell"},
             "Retro shell server");

    root.add({"server", "rshell", "start"},
             "Starts the retro shell server",
             [this](Arguments& argv, long value) {

        remoteManager.rshServer.start();
    });

    root.add({"server", "rshell", "stop"},
             "Stops the retro shell server",
             [this](Arguments& argv, long value) {

        remoteManager.rshServer.stop();
    });

    root.add({"server", "rshell", "disconnect"},
             "Disconnects a client",
             [this](Arguments& argv, long value) {

        remoteManager.rshServer.disconnect();
    });

    root.add({"server", "rshell", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(remoteManager.rshServer);
    });

    root.add({"server", "rshell", "set"},
             "Configures the component");

    root.add({"server", "rshell", "set", "port"}, { Arg::value },
             "Assigns the port number",
             [this](Arguments& argv, long value) {

        remoteManager.rshServer.setConfigItem(OPT_SRV_PORT, parseNum(argv));
    });

    root.add({"server", "rshell", "set", "verbose"}, { Arg::boolean },
             "Switches verbose mode on or off",
             [this](Arguments& argv, long value) {

        remoteManager.rshServer.setConfigItem(OPT_SRV_PORT, parseBool(argv));
    });

    root.add({"server", "rshell", "inspect"},
             "Displays the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dumpDebug(remoteManager.rshServer);
    });

    root.add({"server", "gdb"},
             "GDB server");

    root.add({"server", "gdb", "attach"}, { Arg::process },
             "Attaches the GDB server to a process",
             [this](Arguments& argv, long value) {

        remoteManager.gdbServer.attach(argv.front());
    });

    root.add({"server", "gdb", "detach"},
             "Detaches the GDB server from a process",
             [this](Arguments& argv, long value) {

        remoteManager.gdbServer.detach();
    });

    root.add({"server", "gdb", ""},
             "Displays the current configuration",
             [this](Arguments& argv, long value) {

        retroShell.dumpConfig(remoteManager.gdbServer);
    });

    root.add({"server", "gdb", "set"},
             "Configures the component");

    root.add({"server", "gdb", "set", "port"}, { Arg::value },
             "Assigns the port number",
             [this](Arguments& argv, long value) {

        remoteManager.gdbServer.setConfigItem(OPT_SRV_PORT, parseNum(argv));
    });

    root.add({"server", "gdb", "set", "verbose"}, { Arg::boolean },
             "Switches verbose mode on or off",
             [this](Arguments& argv, long value) {

        remoteManager.gdbServer.setConfigItem(OPT_SRV_VERBOSE, parseBool(argv));
    });

    root.add({"server", "gdb", "inspect"},
             "Displays the internal state",
             [this](Arguments& argv, long value) {

        retroShell.dumpDebug(remoteManager.gdbServer);
    });
}

}
