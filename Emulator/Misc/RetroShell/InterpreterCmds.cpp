// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Interpreter.h"
#include "Emulator.h"
#include "Option.h"

namespace vamiga {

#define VAMIGA_CONCAT(x,y) x##y
#define VAMIGA_GROUP_NAME(x) VAMIGA_CONCAT(group_,x)
#define VAMIGA_GROUP(x) CommandGroup VAMIGA_GROUP_NAME(__COUNTER__)(root,x);

void
Interpreter::initCommons(Command &root)
{
    //
    // Common commands
    //

    {   VAMIGA_GROUP("Shell commands");

        root.add({"."},
                 "Enter or exit the debugger",
                 [this](Arguments& argv, long value) {

            retroShell.clear();
            switchInterpreter();
            retroShell.welcome();
        });

        root.add({"clear"},
                 "Clear the console window",
                 [this](Arguments& argv, long value) {

            retroShell.clear();
        });

        root.add({"close"},
                 "Hide the console window",
                 [this](Arguments& argv, long value) {

            msgQueue.put(MSG_CONSOLE_CLOSE);
        });

        root.add({"help"}, { }, {Arg::command},
                 "Print usage information",
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
                 "Process a command script",
                 [this](Arguments& argv, long value) {

            auto stream = std::ifstream(argv.front());
            if (!stream.is_open()) throw Error(ERROR_FILE_NOT_FOUND, argv.front());
            retroShell.execScript(stream);
        });

        root.add({"wait"}, {Arg::value, Arg::seconds},
                 "", // Pause the execution of a command script",
                 [this](Arguments& argv, long value) {

            auto seconds = parseNum(argv[0]);
            agnus.scheduleRel<SLOT_RSH>(SEC(seconds), RSH_WAKEUP);
            throw ScriptInterruption(seconds);
        });
    }
}

void
Interpreter::initCommandShell(Command &root)
{
    initCommons(root);

    {   VAMIGA_GROUP("Regression testing")

        root.add({"regression"},    "Runs the regression tester");

        {   VAMIGA_GROUP("");

            root.add({"regression", "setup"}, { ConfigSchemeEnum::argList() }, { Arg::path, Arg::path },
                     "Initializes the test environment",
                     [this](Arguments& argv, long value) {

                auto scheme = parseEnum<ConfigSchemeEnum>(argv[0]);
                auto rom = argv.size() > 1 ? argv[1] : "";
                auto ext = argv.size() > 2 ? argv[2] : "";

                amiga.regressionTester.prepare(scheme, rom, ext);
            });

            root.add({"regression", "run"}, { Arg::path },
                     "Launches a regression test",
                     [this](Arguments& argv, long value) {

                amiga.regressionTester.run(argv.front());
            });
        }

        root.add({"screenshot"},    "Manages screenshots");

        {   VAMIGA_GROUP("")

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

                isize x1 = parseNum(argv[0]);
                isize y1 = parseNum(argv[1]);
                isize x2 = parseNum(argv[2]);
                isize y2 = parseNum(argv[3]);

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
        }
    }

    {   VAMIGA_GROUP("Controlling components")

        //
        // Amiga
        //

        auto cmd = amiga.shellName();
        auto description = amiga.description();
        root.add({cmd}, description);

        {   VAMIGA_GROUP("")

            root.add({cmd, ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(amiga, Category::Config);
            });

            root.add({cmd, "defaults"},
                     "Displays the user defaults storage",
                     [this](Arguments& argv, long value) {

                retroShell.dump(amiga, Category::Defaults);
            });

            root.add({cmd, "set"}, "Configure the component");
            for (auto &opt : amiga.getOptions()) {

                root.add({cmd, "set", OptionEnum::plainkey(opt)},
                         {OptionParser::argList(opt)},
                         OptionEnum::help(opt),
                         [this](Arguments& argv, long opt) {

                    emulator.set(Option(opt), argv[0]);

                }, opt);
            }

            root.add({cmd, "power"}, { Arg::onoff },
                     "Switches the Amiga on or off",
                     [this](Arguments& argv, long value) {

                parseOnOff(argv[0]) ? emulator.run() : emulator.powerOff();
            });

            root.add({cmd, "reset"},
                     "Performs a hard reset",
                     [this](Arguments& argv, long value) {

                amiga.hardReset();
            });

            root.add({cmd, "init"}, { ConfigSchemeEnum::argList() },
                     "Initializes the Amiga with a predefined scheme",
                     [this](Arguments& argv, long value) {

                auto scheme = parseEnum <ConfigSchemeEnum> (argv[0]);

                // Don't call this function from within RetroShell. It'll call
                // initialize() which will reinitialize RetroShell while it is running.
                // This causes a crash on gcc.
                // amiga.revertToFactorySettings();
                emulator.powerOff();
                emulator.set(scheme);
            });
        }

        //
        // Memory
        //

        cmd = mem.shellName();
        description = mem.description();
        root.add({cmd}, description);

        {   VAMIGA_GROUP("")

            root.add({cmd, ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(mem, Category::Config);
            });

            root.add({cmd, "set"}, "Configure the component");
            for (auto &opt : mem.getOptions()) {

                root.add({cmd, "set", OptionEnum::plainkey(opt)},
                         {OptionParser::argList(opt)},
                         OptionEnum::help(opt),
                         [this](Arguments& argv, long opt) {

                    emulator.set(Option(opt), argv[0]);

                }, opt);
            }

            root.add({cmd, "load"},
                     "Installs a Rom image");

            root.add({cmd, "load", "rom"}, { Arg::path },
                     "Installs a Kickstart Rom",
                     [this](Arguments& argv, long value) {

                amiga.mem.loadRom(argv.front());
            });

            root.add({cmd, "load", "extrom"}, { Arg::path },
                     "Installs a Rom extension",
                     [this](Arguments& argv, long value) {

                amiga.mem.loadExt(argv.front());
            });
        }

        //
        // CPU
        //

        cmd = cpu.shellName();
        description = cpu.description();
        root.add({cmd}, description);

        {   VAMIGA_GROUP("")

            root.add({cmd, ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(cpu, Category::Config);
            });

            root.add({cmd, "set"}, "Configure the component");
            for (auto &opt : cpu.getOptions()) {

                root.add({cmd, "set", OptionEnum::plainkey(opt)},
                         {OptionParser::argList(opt)},
                         OptionEnum::help(opt),
                         [this](Arguments& argv, long opt) {

                    emulator.set(Option(opt), argv[0]);

                }, opt);
            }
        }

        //
        // CIA
        //

        for (isize i = 0; i < 2; i++) {

            auto cmd = (i == 0) ? ciaa.shellName() : ciab.shellName();
            auto description = (i == 0) ? ciaa.description() : ciab.description();
            root.add({cmd}, description);

            {   VAMIGA_GROUP("")

                root.add({cmd, ""},
                         "Displays the current configuration",
                         [this](Arguments& argv, long value) {

                    if (value == 0) retroShell.dump(ciaa, Category::Config);
                    if (value == 1) retroShell.dump(ciab, Category::Config);

                }, i);

                root.add({cmd, "set"}, "Configures the component");

                for (auto &opt : ciaa.getOptions()) {

                    root.add({cmd, "set", OptionEnum::plainkey(opt)},
                             {OptionParser::argList(opt)},
                             OptionEnum::help(opt),
                             [this](Arguments& argv, long value) {

                        emulator.set(Option(HI_WORD(value)), argv[0], { LO_WORD(value) });

                    }, HI_W_LO_W(opt, i));
                }
            }
        }

        //
        // Agnus
        //

        cmd = agnus.shellName();
        description = agnus.description();
        root.add({cmd}, description);

        {   VAMIGA_GROUP("")

            root.add({cmd, ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(agnus, Category::Config);
            });

            root.add({cmd, "set"}, "Configure the component");
            for (auto &opt : agnus.getOptions()) {

                root.add({cmd, "set", OptionEnum::plainkey(opt)},
                         {OptionParser::argList(opt)},
                         OptionEnum::help(opt),
                         [this](Arguments& argv, long opt) {

                    emulator.set(Option(opt), argv[0]);

                }, opt);
            }
        }

        //
        // Blitter
        //

        cmd = blitter.shellName();
        description = blitter.description();
        root.add({cmd}, description);

        {   VAMIGA_GROUP("")

            root.add({cmd, ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(blitter, Category::Config);
            });

            root.add({cmd, "set"}, "Configure the component");
            for (auto &opt : blitter.getOptions()) {

                root.add({cmd, "set", OptionEnum::plainkey(opt)},
                         {OptionParser::argList(opt)},
                         OptionEnum::help(opt),
                         [this](Arguments& argv, long opt) {

                    emulator.set(Option(opt), argv[0]);

                }, opt);
            }
        }

        //
        // Denise
        //

        cmd = denise.shellName();
        description = denise.description();
        root.add({cmd}, description);

        {   VAMIGA_GROUP("")

            root.add({cmd, ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(denise, Category::Config);
            });

            root.add({cmd, "set"}, "Configure the component");
            for (auto &opt : denise.getOptions()) {

                root.add({cmd, "set", OptionEnum::plainkey(opt)},
                         {OptionParser::argList(opt)},
                         OptionEnum::help(opt),
                         [this](Arguments& argv, long opt) {

                    emulator.set(Option(opt), argv[0]);

                }, opt);
            }
        }

        //
        // Paula
        //

        cmd = paula.shellName();
        description = paula.description();
        root.add({cmd}, description);

        {   VAMIGA_GROUP("")

            root.add({cmd, "audio"},
                     "Audio unit");

            root.add({cmd, "audio", ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(paula.muxer, Category::Config);
            });

            root.add({cmd, "audio", "filter"},
                     "Displays the current filter configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(paula.muxer.filter, Category::Config);
            });

            root.add({cmd, "audio", "filter", "set"}, "Configure the component");
            for (auto &opt : paula.muxer.filter.getOptions()) {

                root.add({cmd, "audio", "filter", "set", OptionEnum::plainkey(opt)},
                         {OptionParser::argList(opt)},
                         OptionEnum::help(opt),
                         [this](Arguments& argv, long opt) {

                    emulator.set(Option(opt), argv[0]);

                }, opt);
            }

            root.add({cmd, "audio", "set"}, "Configure the component");
            for (auto &opt : paula.muxer.getOptions()) {

                root.add({cmd, "audio", "set", OptionEnum::plainkey(opt)},
                         {OptionParser::argList(opt)},
                         OptionEnum::help(opt),
                         [this](Arguments& argv, long opt) {

                    emulator.set(Option(opt), argv[0]);

                }, opt);
            }

            root.add({cmd, "dc"},
                     "Disk controller");

            root.add({cmd, "dc", ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(diskController, Category::Config);
            });

            root.add({cmd, "dc", "set"}, "Configure the component");
            for (auto &opt : paula.diskController.getOptions()) {

                root.add({cmd, "dc", "set", OptionEnum::plainkey(opt)},
                         {OptionParser::argList(opt)},
                         OptionEnum::help(opt),
                         [this](Arguments& argv, long opt) {

                    emulator.set(Option(opt), argv[0]);

                }, opt);
            }
        }

        //
        // RTC
        //

        root.add({"rtc"},           "Real-time clock");

        {   VAMIGA_GROUP("")

            root.add({"rtc", ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(rtc, Category::Config);
            });

            root.add({"rtc", "set"},
                     "Configures the component");

            root.add({"rtc", "set", "revision"}, { RTCRevisionEnum::argList() },
                     "Selects the emulated chip model",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_RTC_MODEL, parseEnum <RTCRevisionEnum> (argv[0]));
            });
        }

        //
        // Serial port
        //

        root.add({"serial"},        "Serial port");

        {   VAMIGA_GROUP("")

            root.add({"serial", ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(serialPort, Category::Config);
            });

            root.add({"serial", "set"},
                     "Configures the component");

            root.add({"serial", "set", "device"}, { SerialPortDeviceEnum::argList() },
                     "Connects a device",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_SER_DEVICE, parseEnum <SerialPortDeviceEnum> (argv[0]));
            });

            root.add({"serial", "set", "verbose"}, { Arg::boolean },
                     "Enables or disables communication tracking",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_SER_VERBOSE, parseBool(argv[0]));
            });

            root.add({"serial", "send"}, { "<text>" },
                     "Sends a text to the serial port",
                     [this](Arguments& argv, long value) {

                amiga.serialPort << argv[0];
            });
        }

        //
        // DMA Debugger
        //

        root.add({"dmadebugger"},   "DMA Debugger");

        {   VAMIGA_GROUP("")

            root.add({"dmadebugger", "open"},
                     "Opens the DMA debugger",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_DMA_DEBUG_ENABLE, true);
            });

            root.add({"dmadebugger", "close"},
                     "Closes the DMA debugger",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_DMA_DEBUG_ENABLE, false);
            });

            root.add({"dmadebugger", "copper"}, { Arg::onoff },
                     "Turns Copper DMA visualization on or off",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_DMA_DEBUG_CHANNEL0, parseOnOff(argv[0]));
            });

            root.add({"dmadebugger", "blitter"}, { Arg::onoff },
                     "Turns Blitter DMA visualization on or off",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_DMA_DEBUG_CHANNEL1, parseOnOff(argv[0]));
            });

            root.add({"dmadebugger", "disk"}, { Arg::onoff },
                     "Turns Disk DMA visualization on or off",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_DMA_DEBUG_CHANNEL2, parseOnOff(argv[0]));
            });

            root.add({"dmadebugger", "audio"}, { Arg::onoff },
                     "Turns Audio DMA visualization on or off",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_DMA_DEBUG_CHANNEL3, parseOnOff(argv[0]));
            });

            root.add({"dmadebugger", "sprites"}, { Arg::onoff },
                     "Turns Sprite DMA visualization on or off",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_DMA_DEBUG_CHANNEL4, parseOnOff(argv[0]));
            });

            root.add({"dmadebugger", "bitplanes"}, { Arg::onoff },
                     "Turns Bitplane DMA visualization on or off",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_DMA_DEBUG_CHANNEL5, parseOnOff(argv[0]));
            });

            root.add({"dmadebugger", "cpu"}, { Arg::onoff },
                     "Turns CPU bus usage visualization on or off",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_DMA_DEBUG_CHANNEL6, parseOnOff(argv[0]));
            });

            root.add({"dmadebugger", "refresh"}, { Arg::onoff },
                     "Turn memory refresh visualization on or off",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_DMA_DEBUG_CHANNEL7, parseOnOff(argv[0]));
            });
        }
    }

    {   VAMIGA_GROUP("Controlling peripherals")

        //
        // Monitor
        //

        root.add({"monitor"},       "Amiga monitor");

        {   VAMIGA_GROUP("")

            root.add({"monitor", ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(pixelEngine, Category::Config);
            });

            root.add({"monitor", "set"},
                     "Configures the component");

            root.add({"monitor", "set", "palette"}, { PaletteEnum::argList() },
                     "Selects the color palette",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_PALETTE, parseEnum <PaletteEnum> (argv[0]));
            });

            root.add({"monitor", "set", "brightness"}, { Arg::value },
                     "Adjusts the brightness of the Amiga texture",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_BRIGHTNESS, parseNum(argv[0]));
            });

            root.add({"monitor", "set", "contrast"}, { Arg::value },
                     "Adjusts the contrast of the Amiga texture",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_CONTRAST, parseNum(argv[0]));
            });

            root.add({"monitor", "set", "saturation"}, { Arg::value },
                     "Adjusts the saturation of the Amiga texture",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_SATURATION, parseNum(argv[0]));
            });
        }

        //
        // Keyboard
        //

        root.add({"keyboard"},      "Keyboard");

        {   VAMIGA_GROUP("")

            root.add({"keyboard", ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(keyboard, Category::Config);
            });

            root.add({"keyboard", "set"},
                     "Configures the component");

            root.add({"keyboard", "set", "accuracy"}, { Arg::value },
                     "Determines the emulation accuracy level",
                     [this](Arguments& argv, long value) {

                emulator.set(OPT_ACCURATE_KEYBOARD, parseBool(argv[0]));
            });

            root.add({"keyboard", "press"}, { Arg::value },
                     "Sends a keycode to the keyboard",
                     [this](Arguments& argv, long value) {

                keyboard.autoType(KeyCode(parseNum(argv[0])));
            });
        }

        //
        // Joystick
        //

        root.add({"joystick"},      "Joystick");

        {   VAMIGA_GROUP("")

            for (isize i = 0; i <= 1; i++) {

                string nr = (i == 0) ? "1" : "2";

                root.add({"joystick", nr},
                         "Joystick in port " + nr);

                root.add({"joystick", nr, ""},
                         "Displays the current configuration",
                         [this](Arguments& argv, long value) {

                    auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                    retroShell.dump(port.joystick, Category::Config);

                }, i);

                root.add({"joystick", nr, "set"},
                         "Configures the component");

                root.add({"joystick", nr, "set", "autofire"}, { Arg::boolean },
                         "Enables or disables auto-fire mode",
                         [this](Arguments& argv, long value) {

                    auto port = (value == 0) ? 0 : 1;
                    emulator.set(OPT_AUTOFIRE, port, { parseBool(argv[0]) });

                }, i);

                root.add({"joystick", nr, "set", "bullets"},  { Arg::value },
                         "Sets the number of bullets per auto-fire shot",
                         [this](Arguments& argv, long value) {

                    auto port = (value == 0) ? 0 : 1;
                    emulator.set(OPT_AUTOFIRE_BULLETS, port, { parseNum(argv[0]) });

                }, i);

                root.add({"joystick", nr, "set", "delay"}, { Arg::value },
                         "Configures the auto-fire delay",
                         [this](Arguments& argv, long value) {

                    auto port = (value == 0) ? 0 : 1;
                    emulator.set(OPT_AUTOFIRE_DELAY, port, { parseNum(argv[0]) });

                }, i);

                root.add({"joystick", nr, "press"}, { Arg::value },
                         "Presses a joystick button",
                         [this](Arguments& argv, long value) {

                    auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                    auto nr = parseNum(argv[0]);

                    switch (nr) {

                        case 1: port.joystick.trigger(PRESS_FIRE); break;
                        case 2: port.joystick.trigger(PRESS_FIRE2); break;
                        case 3: port.joystick.trigger(PRESS_FIRE3); break;

                        default:
                            throw Error(ERROR_OPT_INV_ARG, "1...3");
                    }

                }, i);

                root.add({"joystick", nr, "unpress"}, { Arg::value },
                         "Releases a joystick button",
                         [this](Arguments& argv, long value) {

                    auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                    auto nr = parseNum(argv[0]);

                    switch (nr) {

                        case 1: port.joystick.trigger(RELEASE_FIRE); break;
                        case 2: port.joystick.trigger(RELEASE_FIRE2); break;
                        case 3: port.joystick.trigger(RELEASE_FIRE3); break;

                        default:
                            throw Error(ERROR_OPT_INV_ARG, "1...3");
                    }

                }, i);

                root.add({"joystick", nr, "pull"},
                         "Pulls the joystick");

                root.add({"joystick", nr, "pull", "left"},
                         "Pulls the joystick left",
                         [this](Arguments& argv, long value) {

                    auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                    port.joystick.trigger(PULL_LEFT);

                }, i);

                root.add({"joystick", nr, "pull", "right"},
                         "Pulls the joystick right",
                         [this](Arguments& argv, long value) {

                    auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                    port.joystick.trigger(PULL_RIGHT);

                }, i);

                root.add({"joystick", nr, "pull", "up"},
                         "Pulls the joystick up",
                         [this](Arguments& argv, long value) {

                    auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                    port.joystick.trigger(PULL_UP);

                }, i);

                root.add({"joystick", nr, "pull", "down"},
                         "Pulls the joystick down",
                         [this](Arguments& argv, long value) {

                    auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                    port.joystick.trigger(PULL_DOWN);

                }, i);

                root.add({"joystick", nr, "release"},
                         "Release a joystick axis");

                root.add({"joystick", nr, "release", "x"},
                         "Releases the x-axis",
                         [this](Arguments& argv, long value) {

                    auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                    port.joystick.trigger(RELEASE_X);

                }, i);

                root.add({"joystick", nr, "release", "y"},
                         "Releases the y-axis",
                         [this](Arguments& argv, long value) {

                    auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                    port.joystick.trigger(RELEASE_Y);

                }, i);
            }
        }

        //
        // Mouse
        //

        root.add({"mouse"}, "Mouse");

        {   VAMIGA_GROUP("")

            for (isize i = 0; i <= 1; i++) {

                auto cmd = (i == 0) ?
                controlPort1.mouse.shellName() : controlPort2.mouse.shellName();

                string nr = (i == 0) ? "1" : "2";

                root.add({cmd},
                         "Mouse in port " + nr);

                root.add({cmd, ""},
                         "Displays the current configuration",
                         [this](Arguments& argv, long value) {

                    auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                    retroShell.dump(port.mouse, Category::Config);

                }, i);

                root.add({cmd, "set"},
                         "Configures the component");

                root.add({cmd, "set", "pullup"}, { Arg::boolean },
                         "Enables or disables pull-up resistors",
                         [this](Arguments& argv, long value) {

                    auto port = (value == 0) ? 0 : 1;
                    emulator.set(OPT_PULLUP_RESISTORS, port, { parseBool(argv[0]) });

                }, i);

                root.add({cmd, "set", "shakedetector"}, { Arg::boolean },
                         "Enables or disables the shake detector",
                         [this](Arguments& argv, long value) {

                    auto port = (value == 0) ? 0 : 1;
                    emulator.set(OPT_SHAKE_DETECTION, port, { parseBool(argv[0]) });

                }, i);

                root.add({cmd, "set", "velocity"}, { Arg::value },
                         "Sets the horizontal and vertical mouse velocity",
                         [this](Arguments& argv, long value) {

                    auto port = (value == 0) ? 0 : 1;
                    emulator.set(OPT_MOUSE_VELOCITY, port, { parseNum(argv[0]) });

                }, i);

                root.add({cmd, "press"},
                         "Presses a mouse button");

                root.add({cmd, "press", "left"},
                         "Presses the left mouse button",
                         [this](Arguments& argv, long value) {

                    auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                    port.mouse.pressAndReleaseLeft();

                }, i);

                root.add({cmd, "press", "middle"},
                         "Presses the middle mouse button",
                         [this](Arguments& argv, long value) {

                    auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                    port.mouse.pressAndReleaseMiddle();

                }, i);

                root.add({cmd, "press", "right"},
                         "Presses the right mouse button",
                         [this](Arguments& argv, long value) {

                    auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                    port.mouse.pressAndReleaseRight();

                }, i);
            }
        }

        //
        // Df0, Df1, Df2, Df3
        //

        for (isize i = 0; i <= 4; i++) {

            string df = i == 4 ? "dfn" : "df" + std::to_string(i);
            root.add({df}, "Floppy drive");

            {   VAMIGA_GROUP("")

                if (i >= 0 && i <= 3) {

                    root.add({df, ""},
                             "Displays the current configuration",
                             [this](Arguments& argv, long value) {

                        retroShell.dump(*amiga.df[value], Category::Config);

                    }, i);

                    if (i >= 1 && i <= 3) {

                        root.add({df, "connect"},
                                 "Connects the drive",
                                 [this](Arguments& argv, long value) {

                            emulator.set(OPT_DRIVE_CONNECT, true, { value });

                        }, i);

                        root.add({df, "disconnect"},
                                 "Disconnects the drive",
                                 [this](Arguments& argv, long value) {

                            emulator.set(OPT_DRIVE_CONNECT, false, { value });

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

                        long model = parseEnum <FloppyDriveTypeEnum> (argv[0]);

                        if (value == 0 || value > 3) emulator.set(OPT_DRIVE_TYPE, model, {0});
                        if (value == 1 || value > 3) emulator.set(OPT_DRIVE_TYPE, model, {1});
                        if (value == 2 || value > 3) emulator.set(OPT_DRIVE_TYPE, model, {2});
                        if (value == 3 || value > 3) emulator.set(OPT_DRIVE_TYPE, model, {3});

                    }, i);

                    root.add({df, "set", "rpm"}, { "rpm" },
                             "Sets the disk rotation speed",
                             [this](Arguments& argv, long value) {

                        long rpm = parseNum(argv[0]);

                        if (value == 0 || value > 3) emulator.set(OPT_DRIVE_RPM, rpm, {0});
                        if (value == 1 || value > 3) emulator.set(OPT_DRIVE_RPM, rpm, {1});
                        if (value == 2 || value > 3) emulator.set(OPT_DRIVE_RPM, rpm, {2});
                        if (value == 3 || value > 3) emulator.set(OPT_DRIVE_RPM, rpm, {3});

                    }, i);

                    root.add({df, "set", "mechanics"}, { Arg::boolean },
                             "Enables or disables the emulation of mechanical delays",
                             [this](Arguments& argv, long value) {

                        auto scheme = parseEnum<DriveMechanicsEnum>(argv[0]);

                        if (value == 0 || value > 3) emulator.set(OPT_DRIVE_MECHANICS, scheme, {0});
                        if (value == 1 || value > 3) emulator.set(OPT_DRIVE_MECHANICS, scheme, {1});
                        if (value == 2 || value > 3) emulator.set(OPT_DRIVE_MECHANICS, scheme, {2});
                        if (value == 3 || value > 3) emulator.set(OPT_DRIVE_MECHANICS, scheme, {3});

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

                        long delay = parseNum(argv[0]);

                        if (value == 0 || value > 3) emulator.set(OPT_DISK_SWAP_DELAY, delay, {0});
                        if (value == 1 || value > 3) emulator.set(OPT_DISK_SWAP_DELAY, delay, {1});
                        if (value == 2 || value > 3) emulator.set(OPT_DISK_SWAP_DELAY, delay, {2});
                        if (value == 3 || value > 3) emulator.set(OPT_DISK_SWAP_DELAY, delay, {3});

                    }, i);

                    root.add({df, "set", "pan"}, { Arg::value },
                             "Sets the pan for drive sounds",
                             [this](Arguments& argv, long value) {

                        long pan = parseNum(argv[0]);

                        if (value == 0 || value > 3) emulator.set(OPT_DRIVE_PAN, pan, {0});
                        if (value == 1 || value > 3) emulator.set(OPT_DRIVE_PAN, pan, {1});
                        if (value == 2 || value > 3) emulator.set(OPT_DRIVE_PAN, pan, {2});
                        if (value == 3 || value > 3) emulator.set(OPT_DRIVE_PAN, pan, {3});

                    }, i);

                    root.add({df, "audiate"},
                             "Sets the volume of drive sounds");

                    root.add({df, "audiate", "insert"}, { Arg::volume },
                             "Makes disk insertions audible",
                             [this](Arguments& argv, long value) {

                        long volume = parseNum(argv[0]);

                        if (value == 0 || value > 3) emulator.set(OPT_INSERT_VOLUME, volume, {0});
                        if (value == 1 || value > 3) emulator.set(OPT_INSERT_VOLUME, volume, {1});
                        if (value == 2 || value > 3) emulator.set(OPT_INSERT_VOLUME, volume, {2});
                        if (value == 3 || value > 3) emulator.set(OPT_INSERT_VOLUME, volume, {3});

                    }, i);

                    root.add({df, "audiate", "eject"}, { Arg::volume },
                             "Makes disk ejections audible",
                             [this](Arguments& argv, long value) {

                        long volume = parseNum(argv[0]);

                        if (value == 0 || value > 3) emulator.set(OPT_EJECT_VOLUME, volume, {0});
                        if (value == 1 || value > 3) emulator.set(OPT_EJECT_VOLUME, volume, {1});
                        if (value == 2 || value > 3) emulator.set(OPT_EJECT_VOLUME, volume, {2});
                        if (value == 3 || value > 3) emulator.set(OPT_EJECT_VOLUME, volume, {3});

                    }, i);

                    root.add({df, "audiate", "step"},  { Arg::volume },
                             "Makes head steps audible",
                             [this](Arguments& argv, long value) {

                        long volume = parseNum(argv[0]);

                        if (value == 0 || value > 3) emulator.set(OPT_STEP_VOLUME, volume, {0});
                        if (value == 1 || value > 3) emulator.set(OPT_STEP_VOLUME, volume, {1});
                        if (value == 2 || value > 3) emulator.set(OPT_STEP_VOLUME, volume, {2});
                        if (value == 3 || value > 3) emulator.set(OPT_STEP_VOLUME, volume, {3});

                    }, i);

                    root.add({df, "audiate", "poll"},  { Arg::volume },
                             "Makes polling clicks audible",
                             [this](Arguments& argv, long value) {

                        long volume = parseNum(argv[0]);

                        if (value == 0 || value > 3) emulator.set(OPT_POLL_VOLUME, volume, {0});
                        if (value == 1 || value > 3) emulator.set(OPT_POLL_VOLUME, volume, {1});
                        if (value == 2 || value > 3) emulator.set(OPT_POLL_VOLUME, volume, {2});
                        if (value == 3 || value > 3) emulator.set(OPT_POLL_VOLUME, volume, {3});

                    }, i);
                }
            }
        }

        //
        // Hd0, Hd1, Hd2, Hd3
        //

        for (isize i = 0; i <= 4; i++) {

            string hd = i == 4 ? "hdn" : "hd" + std::to_string(i);
            root.add({hd}, "Floppy drive");

            {   VAMIGA_GROUP("")

                if (i != 4) {

                    root.add({hd, ""},
                             "Displays the current configuration",
                             [this](Arguments& argv, long value) {

                        retroShell.dump(*amiga.hd[value], Category::Config);

                    }, i);

                    root.add({hd, "connect"},
                             "Connects the hard drive",
                             [this](Arguments& argv, long value) {

                        emulator.set(OPT_HDC_CONNECT, true, {value});

                    }, i);

                    root.add({hd, "disconnect"},
                             "Disconnects the hard drive",
                             [this](Arguments& argv, long value) {

                        emulator.set(OPT_HDC_CONNECT, false, {value});

                    }, i);

                }

                root.add({hd, "set"},
                         "Configures the component");

                root.add({hd, "set", "pan"}, { Arg::value },
                         "Sets the pan for drive sounds",
                         [this](Arguments& argv, long value) {

                    long pan = parseNum(argv[0]);

                    if (value == 0 || value > 3) emulator.set(OPT_HDR_PAN, pan, {0});
                    if (value == 1 || value > 3) emulator.set(OPT_HDR_PAN, pan, {1});
                    if (value == 2 || value > 3) emulator.set(OPT_HDR_PAN, pan, {2});
                    if (value == 3 || value > 3) emulator.set(OPT_HDR_PAN, pan, {3});

                }, i);

                root.add({hd, "audiate"},
                         "Sets the volume of drive sounds");

                root.add({hd, "audiate", "step"}, { Arg::volume },
                         "Makes head steps audible",
                         [this](Arguments& argv, long value) {

                    long volume = parseNum(argv[0]);

                    if (value == 0 || value > 3) emulator.set(OPT_HDR_STEP_VOLUME, volume, {0});
                    if (value == 1 || value > 3) emulator.set(OPT_HDR_STEP_VOLUME, volume, {1});
                    if (value == 2 || value > 3) emulator.set(OPT_HDR_STEP_VOLUME, volume, {2});
                    if (value == 3 || value > 3) emulator.set(OPT_HDR_STEP_VOLUME, volume, {3});

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
        }
    }

    {   VAMIGA_GROUP("Miscellaneous")

        //
        // Remote server
        //

        root.add({"server"},        "Remote connections");

        {   VAMIGA_GROUP("");

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

                remoteManager.serServer.setOption(OPT_SRV_PORT, parseNum(argv[0]));
            });

            root.add({"server", "serial", "set", "verbose"}, { Arg::boolean },
                     "Switches verbose mode on or off",
                     [this](Arguments& argv, long value) {

                remoteManager.rshServer.setOption(OPT_SRV_VERBOSE, parseBool(argv[0]));
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

                retroShell.dump(remoteManager.rshServer, Category::Config);
            });

            root.add({"server", "rshell", "set"},
                     "Configures the component");

            root.add({"server", "rshell", "set", "port"}, { Arg::value },
                     "Assigns the port number",
                     [this](Arguments& argv, long value) {

                remoteManager.rshServer.setOption(OPT_SRV_PORT, parseNum(argv[0]));
            });

            root.add({"server", "rshell", "set", "verbose"}, { Arg::boolean },
                     "Switches verbose mode on or off",
                     [this](Arguments& argv, long value) {

                remoteManager.rshServer.setOption(OPT_SRV_PORT, parseBool(argv[0]));
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

                retroShell.dump(remoteManager.gdbServer, Category::Config);
            });

            root.add({"server", "gdb", "set"},
                     "Configures the component");

            root.add({"server", "gdb", "set", "port"}, { Arg::value },
                     "Assigns the port number",
                     [this](Arguments& argv, long value) {

                remoteManager.gdbServer.setOption(OPT_SRV_PORT, parseNum(argv[0]));
            });

            root.add({"server", "gdb", "set", "verbose"}, { Arg::boolean },
                     "Switches verbose mode on or off",
                     [this](Arguments& argv, long value) {

                remoteManager.gdbServer.setOption(OPT_SRV_VERBOSE, parseBool(argv[0]));
            });
        }
    }
}
}
