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
#define VAMIGA_GROUP(x) CommandGroup VAMIGA_GROUP_NAME(__COUNTER__)(root,x); Command::currentGroup = x;

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
            throw ScriptInterruption();
        });
    }
}

void
Interpreter::initSetters(Command &root, const CoreComponent &c)
{
    if (auto cmd = string(c.shellName()); !cmd.empty()) {

        if (auto &options = c.getOptions(); !options.empty()) {

            root.add({cmd, "set"}, "Configure the component");
            for (auto &opt : options) {

                root.add({cmd, "set", OptionEnum::plainkey(opt)},
                         {OptionParser::argList(opt)},
                         OptionEnum::help(opt),
                         [this](Arguments& argv, long value) {

                    emulator.set(Option(HI_WORD(value)), argv[0], { LO_WORD(value) });

                }, HI_W_LO_W(opt, c.objid));
            }
        }
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

            initSetters(root, amiga);

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

            initSetters(root, mem);

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

            initSetters(root, cpu);
        }

        //
        // CIA
        //

        for (isize i = 0; i < 2; i++) {

            cmd = (i == 0) ? ciaa.shellName() : ciab.shellName();
            description = (i == 0) ? ciaa.description() : ciab.description();
            root.add({cmd}, description);

            {   VAMIGA_GROUP("")

                root.add({cmd, ""},
                         "Displays the current configuration",
                         [this](Arguments& argv, long value) {

                    if (value == 0) retroShell.dump(ciaa, Category::Config);
                    if (value == 1) retroShell.dump(ciab, Category::Config);

                }, i);
            }
        }
        
        initSetters(root, ciaa);
        initSetters(root, ciab);


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

            initSetters(root, agnus);
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

            initSetters(root, blitter);
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

            initSetters(root, denise);
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

            initSetters(root, paula.muxer);

            root.add({cmd, "audio", "filter"},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(paula.muxer.filter, Category::Config);
            });

            initSetters(root, paula.muxer.filter);

            root.add({cmd, "dc"},
                     "Disk controller");

            root.add({cmd, "dc", ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(diskController, Category::Config);
            });

            initSetters(root, diskController);
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

            initSetters(root, rtc);
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

            initSetters(root, serialPort);

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

            initSetters(root, dmaDebugger);
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

            initSetters(root, pixelEngine);
        }

        //
        // Keyboard
        //

        root.add({"keyboard"}, "Keyboard");

        {   VAMIGA_GROUP("")

            root.add({"keyboard", ""},
                     "Displays the current configuration",
                     [this](Arguments& argv, long value) {

                retroShell.dump(keyboard, Category::Config);
            });

            initSetters(root, keyboard);

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

            initSetters(root, controlPort1.joystick);
            initSetters(root, controlPort2.joystick);
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

            initSetters(root, controlPort1.mouse);
            initSetters(root, controlPort2.mouse);
        }

        //
        // Df0, Df1, Df2, Df3
        //

        for (isize i = 0; i <= 3; i++) {

            string df = "df" + std::to_string(i);
            root.add({df}, "Floppy drive");

            {   VAMIGA_GROUP("")

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

                root.add({df, "searchpath"}, { Arg::path },
                         "Sets the search path for media files",
                         [this](Arguments& argv, long value) {

                    string path = argv.front();

                    if (value == 0 || value > 3) df0.setSearchPath(path);
                    if (value == 1 || value > 3) df1.setSearchPath(path);
                    if (value == 2 || value > 3) df2.setSearchPath(path);
                    if (value == 3 || value > 3) df3.setSearchPath(path);

                }, i);
            }
        }

        initSetters(root, df0);
        initSetters(root, df1);
        initSetters(root, df2);
        initSetters(root, df3);

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

                root.add({hd, "geometry"},  { "<cylinders>", "<heads>", "<sectors>" },
                         "Changes the disk geometry",
                         [this](Arguments& argv, long value) {

                    auto c = util::parseNum(argv[0]);
                    auto h = util::parseNum(argv[1]);
                    auto s = util::parseNum(argv[2]);

                    amiga.hd[value]->changeGeometry(c, h, s);

                }, i);
            }
        }

        initSetters(root, hd0);
        initSetters(root, hd1);
        initSetters(root, hd2);
        initSetters(root, hd3);
    }

    {   VAMIGA_GROUP("Miscellaneous")

        //
        // Miscellaneous (Host)
        //

        auto cmd = host.shellName();
        auto description = host.description();
        root.add({cmd}, description);

        root.add({cmd, ""},
                 "Displays the current configuration",
                 [this](Arguments& argv, long value) {

            retroShell.dump(host, Category::Config);
        });

        initSetters(root, host);


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

            initSetters(root, remoteManager.serServer);

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

            initSetters(root, remoteManager.rshServer);

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

            initSetters(root, remoteManager.gdbServer);
        }
    }
}
}
