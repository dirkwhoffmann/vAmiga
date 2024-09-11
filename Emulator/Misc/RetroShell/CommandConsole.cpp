// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Console.h"
#include "Emulator.h"
#include "Option.h"

namespace vamiga {

void
CommandConsole::_pause()
{
    
}

string
CommandConsole::getPrompt()
{
    return "vAmiga% ";
}

void
CommandConsole::welcome()
{
    storage << "vAmiga RetroShell ";
    remoteManager.rshServer << "vAmiga RetroShell Remote Server ";
    *this << Amiga::build() << '\n';
    *this << '\n';
    *this << "Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de" << '\n';
    *this << "https://github.com/dirkwhoffmann/vAmiga" << '\n';
    *this << '\n';
    
    printHelp();
    *this << '\n';
}

void
CommandConsole::printHelp()
{
    storage << "Type 'help' or press 'TAB' twice for help.\n";
    storage << "Type '.' or press 'SHIFT+RETURN' to enter debug mode.";
    
    remoteManager.rshServer << "Type 'help' for help.\n";
    remoteManager.rshServer << "Type '.' to enter debug mode.";
    
    *this << '\n';
}

void
CommandConsole::pressReturn(bool shift)
{
    if (!shift && input.empty()) {
        
        printHelp();
        
    } else {
        
        Console::pressReturn(shift);
    }
}

void
CommandConsole::initCommands(Command &root)
{
    Console::initCommands(root);
    
    {   Command::currentGroup = "Regression testing";
        
        root.add({"regression"}, debugBuild ? "Runs the regression tester" : "");
        
        {   Command::currentGroup = "";
            
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
        
        root.add({"screenshot"}, debugBuild ? "Manages screenshots" : "");
        
        {   Command::currentGroup = "";
            
            root.add({"screenshot", "set"},
                     "Configure the screenshot");
            
            root.add({"screenshot", "set", "filename"}, { Arg::path },
                     "Assign the screen shot filename",
                     [this](Arguments& argv, long value) {
                
                amiga.regressionTester.dumpTexturePath = argv.front();
            });
            
            root.add({"screenshot", "set", "cutout"}, { Arg::value, Arg::value, Arg::value, Arg::value },
                     "Adjust the texture cutout",
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
    
    {   Command::currentGroup = "Components";
        
        //
        // Amiga
        //
        
        auto cmd = registerComponent(amiga);
        
        root.add({cmd, "defaults"},
                 "Displays the user defaults storage",
                 [this](Arguments& argv, long value) {
            
            dump(emulator, Category::Defaults);
        });
        
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
        
        
        //
        // Memory
        //
        
        cmd = registerComponent(mem);
        
        root.add({cmd, "load"},
                 "Load memory contents from a file");
        
        root.add({cmd, "load", "rom"}, { Arg::path },
                 "Installs a Kickstart Rom",
                 [this](Arguments& argv, long value) {
            
            mem.loadRom(argv.front());
        });
        
        root.add({cmd, "load", "ext"}, { Arg::path },
                 "Installs an extension Rom",
                 [this](Arguments& argv, long value) {
            
            mem.loadExt(argv.front());
        });
        
        root.add({cmd, "load", "bin"}, { Arg::path, Arg::address },
                 "Loads a chunk of memory",
                 [this](Arguments& argv, long value) {
            
            fs::path path(argv[0]);
            mem.debugger.load(path, parseAddr(argv[1]));
        });
        
        root.add({cmd, "save"},
                 "Save memory contents to a file");
        
        root.add({cmd, "save", "rom"}, { Arg::path },
                 "Saves the Kickstart Rom",
                 [this](Arguments& argv, long value) {
            
            mem.saveRom(argv[0]);
        });
        
        root.add({cmd, "save", "ext"}, { Arg::path },
                 "Saves the extension Rom",
                 [this](Arguments& argv, long value) {
            
            mem.saveExt(argv[0]);
        });
        
        root.add({cmd, "save", "bin"}, { Arg::path, Arg::address, Arg::count },
                 "Loads a chunk of memory",
                 [this](Arguments& argv, long value) {
            
            fs::path path(argv[0]);
            mem.debugger.save(path, parseAddr(argv[1]), parseNum(argv[2]));
        });
        
        
        //
        // CPU
        //
        
        cmd = registerComponent(cpu);
        
        
        //
        // CIA
        //
        
        cmd = registerComponent(ciaa);
        cmd = registerComponent(ciab);
        
        
        //
        // Agnus
        //
        
        cmd = registerComponent(agnus);
        
        
        //
        // Blitter
        //
        
        cmd = registerComponent(blitter);
        
        
        //
        // Denise
        //
        
        cmd = registerComponent(denise);
        
        
        //
        // Paula
        //
        
        cmd = registerComponent(paula);
        cmd = registerComponent(diskController, root / cmd);
        
        
        //
        // RTC
        //
        
        cmd = registerComponent(rtc);
        
        
        //
        // Serial port
        //
        
        cmd = registerComponent(serialPort);
        
        root.add({cmd, "send"}, { "<text>" },
                 "Sends a text to the serial port",
                 [this](Arguments& argv, long value) {
            
            amiga.serialPort << argv[0];
        });
        
        
        //
        // DMA Debugger
        //
        
        cmd = registerComponent(dmaDebugger);
        
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
        
        
        Command::currentGroup = "Ports";
        
        //
        // Audio port
        //
        
        cmd = registerComponent(audioPort);
        cmd = registerComponent(audioPort.filter, root / cmd);
        
        
        //
        // Video port
        //
        
        cmd = registerComponent(videoPort);
        
        
        Command::currentGroup = "Peripherals";
        
        //
        // Monitor
        //
        
        cmd = registerComponent(pixelEngine);
        
        
        //
        // Keyboard
        //
        
        cmd = registerComponent(keyboard);
        
        root.add({cmd, "press"}, { Arg::value },
                 "Sends a keycode to the keyboard",
                 [this](Arguments& argv, long value) {
            
            auto code = KeyCode(parseNum(argv[0]));
            emulator.put(Cmd(CMD_KEY_PRESS, KeyCmd { .keycode = code, .delay = 0.0 }));
            emulator.put(Cmd(CMD_KEY_RELEASE, KeyCmd { .keycode = code, .delay = 0.5 }));
        });
        
        
        //
        // Joystick
        //
        
        for (isize i = 0; i <= 1; i++) {
            
            if (i == 0) cmd = registerComponent(controlPort1.joystick);
            if (i == 1) cmd = registerComponent(controlPort2.joystick);
            
            root.add({cmd, "press"}, { Arg::value },
                     "Presses a joystick button",
                     [this](Arguments& argv, long value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                auto nr = parseNum(argv[0]);
                
                switch (nr) {
                        
                    case 1: port.joystick.trigger(PRESS_FIRE); break;
                    case 2: port.joystick.trigger(PRESS_FIRE2); break;
                    case 3: port.joystick.trigger(PRESS_FIRE3); break;
                        
                    default:
                        throw Error(VAERROR_OPT_INV_ARG, "1...3");
                }
                
            }, i);
            
            root.add({cmd, "unpress"}, { Arg::value },
                     "Releases a joystick button",
                     [this](Arguments& argv, long value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                auto nr = parseNum(argv[0]);
                
                switch (nr) {
                        
                    case 1: port.joystick.trigger(RELEASE_FIRE); break;
                    case 2: port.joystick.trigger(RELEASE_FIRE2); break;
                    case 3: port.joystick.trigger(RELEASE_FIRE3); break;
                        
                    default:
                        throw Error(VAERROR_OPT_INV_ARG, "1...3");
                }
                
            }, i);
            
            root.add({cmd, "pull"},
                     "Pulls the joystick");
            
            root.add({cmd, "pull", "left"},
                     "Pulls the joystick left",
                     [this](Arguments& argv, long value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(PULL_LEFT);
                
            }, i);
            
            root.add({cmd, "pull", "right"},
                     "Pulls the joystick right",
                     [this](Arguments& argv, long value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(PULL_RIGHT);
                
            }, i);
            
            root.add({cmd, "pull", "up"},
                     "Pulls the joystick up",
                     [this](Arguments& argv, long value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(PULL_UP);
                
            }, i);
            
            root.add({cmd, "pull", "down"},
                     "Pulls the joystick down",
                     [this](Arguments& argv, long value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(PULL_DOWN);
                
            }, i);
            
            root.add({cmd, "release"},
                     "Release a joystick axis");
            
            root.add({cmd, "release", "x"},
                     "Releases the x-axis",
                     [this](Arguments& argv, long value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(RELEASE_X);
                
            }, i);
            
            root.add({cmd, "release", "y"},
                     "Releases the y-axis",
                     [this](Arguments& argv, long value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(RELEASE_Y);
                
            }, i);
        }
        
        
        //
        // Mouse
        //
        
        for (isize i = 0; i <= 1; i++) {
            
            if (i == 0) cmd = registerComponent(controlPort1.mouse);
            if (i == 1) cmd = registerComponent(controlPort2.mouse);
            
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
        
        
        //
        // Df0, Df1, Df2, Df3
        //
        
        for (isize i = 0; i <= 3; i++) {
            
            cmd = registerComponent(*df[i]);
            
            if (i >= 1 && i <= 3) {
                
                root.add({cmd, "connect"},
                         "Connects the drive",
                         [this](Arguments& argv, long value) {
                    
                    emulator.set(OPT_DRIVE_CONNECT, true, { value });
                    
                }, i);
                
                root.add({cmd, "disconnect"},
                         "Disconnects the drive",
                         [this](Arguments& argv, long value) {
                    
                    emulator.set(OPT_DRIVE_CONNECT, false, { value });
                    
                }, i);
            }
            
            root.add({cmd, "eject"},
                     "Ejects a floppy disk",
                     [this](Arguments& argv, long value) {
                
                amiga.df[value]->ejectDisk();
                
            }, i);
            
            root.add({cmd, "insert"}, { Arg::path },
                     "Inserts a floppy disk",
                     [this](Arguments& argv, long value) {
                
                auto path = argv.front();
                amiga.df[value]->swapDisk(path);
                
            }, i);
            
            root.add({cmd, "searchpath"}, { Arg::path },
                     "Sets the search path for media files",
                     [this](Arguments& argv, long value) {
                
                string path = argv.front();
                
                if (value == 0 || value > 3) df0.setSearchPath(path);
                if (value == 1 || value > 3) df1.setSearchPath(path);
                if (value == 2 || value > 3) df2.setSearchPath(path);
                if (value == 3 || value > 3) df3.setSearchPath(path);
                
            }, i);
        }
        
        
        //
        // Hd0, Hd1, Hd2, Hd3
        //
        
        for (isize i = 0; i <= 3; i++) {
            
            cmd = registerComponent(*hd[i]);
            
            if (i != 4) {
                
                root.add({cmd, ""},
                         "Displays the current configuration",
                         [this](Arguments& argv, long value) {
                    
                    dump(*amiga.hd[value], Category::Config);
                    
                }, i);
                
                root.add({cmd, "connect"},
                         "Connects the hard drive",
                         [this](Arguments& argv, long value) {
                    
                    emulator.set(OPT_HDC_CONNECT, true, {value});
                    
                }, i);
                
                root.add({cmd, "disconnect"},
                         "Disconnects the hard drive",
                         [this](Arguments& argv, long value) {
                    
                    emulator.set(OPT_HDC_CONNECT, false, {value});
                    
                }, i);
                
            }
            
            root.add({cmd, "geometry"},  { "<cylinders>", "<heads>", "<sectors>" },
                     "Changes the disk geometry",
                     [this](Arguments& argv, long value) {
                
                auto c = util::parseNum(argv[0]);
                auto h = util::parseNum(argv[1]);
                auto s = util::parseNum(argv[2]);
                
                amiga.hd[value]->changeGeometry(c, h, s);
                
            }, i);
        }
        
        
        //
        // Miscellaneous
        //
        
        Command::currentGroup = "Miscellaneous";
        
        //
        // Miscellaneous (Diff)
        //
        
        root.add({"config"},
                 "Virtual machine configuration");
        
        root.add({"config", ""},
                 "Displays the current configuration",
                 [this](Arguments& argv, long value) {
            
            std::stringstream ss;
            amiga.exportConfig(ss);
            *this << ss;
        });
        
        root.add({"config", "diff"},
                 "Displays the difference to the default configuration",
                 [this](Arguments& argv, long value) {
            
            std::stringstream ss;
            amiga.exportDiff(ss);
            *this << ss;
        });
        
        
        //
        // Miscellaneous (Host)
        //
        
        cmd = registerComponent(host);
                
        
        //
        // Miscellaneous (Remote server)
        //
        

        root.add({"server"}, "Remote connections");

        root.add({"server", ""},
                 "Displays a server status summary",
                 [this](Arguments& argv, long value) {
            
            dump(remoteManager, Category::Status);
        });
        
        cmd = registerComponent(remoteManager.serServer, root / "server");
        cmd = registerComponent(remoteManager.rshServer, root / "server");
        
        root.add({"server", cmd, "start"},
                 "Starts the retro shell server",
                 [this](Arguments& argv, long value) {
            
            remoteManager.rshServer.start();
        });
        
        root.add({"server", cmd, "stop"},
                 "Stops the retro shell server",
                 [this](Arguments& argv, long value) {
            
            remoteManager.rshServer.stop();
        });
        
        root.add({"server", cmd, "disconnect"},
                 "Disconnects a client",
                 [this](Arguments& argv, long value) {
            
            remoteManager.rshServer.disconnect();
        });
        
        root.add({"server", cmd, ""},
                 "Displays the current configuration",
                 [this](Arguments& argv, long value) {
            
            dump(remoteManager.rshServer, Category::Config);
        });
        
        cmd = registerComponent(remoteManager.gdbServer, root / "server");
        
        root.add({"server", cmd, "attach"}, { Arg::process },
                 "Attaches the GDB server to a process",
                 [this](Arguments& argv, long value) {
            
            remoteManager.gdbServer.attach(argv.front());
        });
        
        root.add({"server", cmd, "detach"},
                 "Detaches the GDB server from a process",
                 [this](Arguments& argv, long value) {
            
            remoteManager.gdbServer.detach();
        });
        
        root.add({"server", cmd, ""},
                 "Displays the current configuration",
                 [this](Arguments& argv, long value) {
            
            dump(remoteManager.gdbServer, Category::Config);
        });
    }
}

}
