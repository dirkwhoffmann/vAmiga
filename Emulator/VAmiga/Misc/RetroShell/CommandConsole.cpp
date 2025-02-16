// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "VAmigaConfig.h"
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
CommandConsole::initCommands(RetroShellCmd &root)
{
    Console::initCommands(root);
    
    //
    // Workspace management
    //
    
    root.add({
        
        .tokens = { "workspace" },
        .hidden = releaseBuild,
        .help   = { "Workspace management" }
    });
    
    root.add({
        
        .tokens = { "workspace init" },
        .hidden = releaseBuild,
        .help   = { "First command of a workspace script" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            amiga.initWorkspace();
        }
    });

    root.add({
        
        .tokens = { "workspace activate" },
        .hidden = releaseBuild,
        .help   = { "Last command of a workspace script" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            amiga.activateWorkspace();
        }
    });

    
    //
    // Regression tester
    //
    
    RetroShellCmd::currentGroup = "Regression testing";
    
    root.add({
        
        .tokens = { "regression" },
        .hidden = releaseBuild,
        .help   = { "Runs the regression tester" }
    });
    
    root.add({
        
        .tokens = { "regression", "setup" },
        .args   = { ConfigSchemeEnum::argList() },
        .extra  = { Arg::path, Arg::path },
        .help   = { "Initializes the test environment" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto scheme = ConfigScheme(parseEnum<ConfigSchemeEnum>(argv[0]));
            auto rom = argv.size() > 1 ? argv[1] : "";
            auto ext = argv.size() > 2 ? argv[2] : "";
            
            amiga.regressionTester.prepare(scheme, rom, ext);
        }
    });
    
    root.add({
        
        .tokens = { "regression", "run" },
        .args   = { Arg::path },
        .help   = { "Launches a regression test" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            amiga.regressionTester.run(argv.front());
        }
    });
    
    root.add({
        
        .tokens = { "screenshot" },
        .hidden = releaseBuild,
        .help   = { "Manages screenshots" }
    });
    
    root.add({
        
        .tokens = { "screenshot", "set" },
        .help   = { "Configure the screenshot" }
    });
    
    root.add({
        
        .tokens = { "screenshot", "set", "filename" },
        .args   = { Arg::path },
        .help   = { "Assign the screen shot filename" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            amiga.regressionTester.dumpTexturePath = argv.front();
        }
    });
    
    root.add({
        
        .tokens = { "screenshot", "set", "cutout" },
        .args   = { Arg::value, Arg::value, Arg::value, Arg::value },
        .help   = { "Adjust the texture cutout" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            isize x1 = parseNum(argv[0]);
            isize y1 = parseNum(argv[1]);
            isize x2 = parseNum(argv[2]);
            isize y2 = parseNum(argv[3]);
            
            amiga.regressionTester.x1 = x1;
            amiga.regressionTester.y1 = y1;
            amiga.regressionTester.x2 = x2;
            amiga.regressionTester.y2 = y2;
        }
    });
    
    root.add({
        
        .tokens = { "screenshot", "save" },
        .args   = { Arg::path },
        .help   = { "Saves a screenshot and exits the emulator" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            amiga.regressionTester.dumpTexture(amiga, argv.front());
        }
    });
    
    
    //
    // Components
    //
    
    RetroShellCmd::currentGroup = "Components";
    
    //
    // Components (Amiga)
    //
    
    auto cmd = registerComponent(amiga);
    
    root.add({
        
        .tokens = { cmd, "defaults" },
        .help   = { "Displays the user defaults storage" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(emulator, Category::Defaults);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "power" },
        .args   = { Arg::onoff },
        .help   = { "Switches the Amiga on or off" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            parseOnOff(argv[0]) ? emulator.run() : emulator.powerOff();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "reset" },
        .help   = { "Performs a hard reset" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {

            amiga.hardReset();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "init" },
        .args   = { ConfigSchemeEnum::argList() },
        .help   = { "Initializes the Amiga with a predefined scheme" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto scheme = ConfigScheme(parseEnum <ConfigSchemeEnum> (argv[0]));
            
            emulator.powerOff();
            emulator.set(scheme);
        }
    });
    
    
    //
    // Components (Memory)
    //
    
    cmd = registerComponent(mem);
    
    root.add({
        
        .tokens = { cmd, "load" },
        .help   = { "Load memory contents from a file" }
    });
    
    root.add({
        
        .tokens = { cmd, "load", "rom" },
        .args   = { Arg::path },
        .help   = { "Installs a Kickstart Rom" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            mem.loadRom(argv.front());
        }
    });
    
    root.add({
        
        .tokens = { cmd, "load", "ext" },
        .args   = { Arg::path },
        .help   = { "Installs an extension Rom" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            mem.loadExt(argv.front());
        }
    });
    
    root.add({
        
        .tokens = { cmd, "load", "bin" },
        .args   = { Arg::path, Arg::address },
        .help   = { "Loads a chunk of memory" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            fs::path path(argv[0]);
            mem.debugger.load(path, parseAddr(argv[1]));
        }
    });
    
    root.add({
        
        .tokens = { cmd, "save" },
        .help   = { "Save memory contents to a file" }
    });
    
    root.add({
        
        .tokens = { cmd, "save", "rom" },
        .args   = { Arg::path },
        .help   = { "Saves the Kickstart Rom" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            mem.saveRom(argv[0]);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "save", "ext" },
        .args   = { Arg::path },
        .help   = { "Saves the extension Rom" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            mem.saveExt(argv[0]);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "save", "bin" },
        .args   = { Arg::path, Arg::address, Arg::count },
        .help   = { "Loads a chunk of memory" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            fs::path path(argv[0]);
            mem.debugger.save(path, parseAddr(argv[1]), parseNum(argv[2]));
        }
    });
    
    
    //
    // Components (CPU)
    //
    
    cmd = registerComponent(cpu);
    
    
    //
    // Components (CIA)
    //
    
    cmd = registerComponent(ciaa);
    cmd = registerComponent(ciab);
    
    
    //
    // Components (Agnus)
    //
    
    cmd = registerComponent(agnus);
    
    
    //
    // Components (Blitter)
    //
    
    cmd = registerComponent(blitter);
    
    
    //
    // Components (Denise)
    //
    
    cmd = registerComponent(denise);
    
    
    //
    // Components (Paula)
    //
    
    cmd = registerComponent(paula);
    cmd = registerComponent(diskController);
    
    
    //
    // Components (RTC)
    //
    
    cmd = registerComponent(rtc);
    
    
    //
    // Components (Hard-drive controller)
    //
    
    /*
    cmd = registerComponent(hd0con);
    cmd = registerComponent(hd1con);
    cmd = registerComponent(hd2con);
    cmd = registerComponent(hd3con);
    */
    
    
    //
    // Ports
    //
    
    RetroShellCmd::currentGroup = "Ports";
    
    
    //
    // Ports (Audio port)
    //
    
    cmd = registerComponent(audioPort);
    cmd = registerComponent(audioPort.filter);
    
    
    //
    // Ports (Video port)
    //
    
    cmd = registerComponent(videoPort);
    
    
    //
    // Ports (Serial port)
    //
    
    cmd = registerComponent(serialPort);
    
    root.add({
        
        .tokens = { cmd, "send" },
        .args   = { "<text>" },
        .help   = { "Sends a text to the serial port" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            amiga.serialPort << argv[0];
        }
    });
    
    
    //
    // Peripherals
    //
    
    RetroShellCmd::currentGroup = "Peripherals";
    
    
    //
    // Peripherals (Monitor)
    //
    
    cmd = registerComponent(pixelEngine);
    
    
    //
    // Peripherals (Keyboard)
    //
    
    cmd = registerComponent(keyboard);
    
        root.add({
            
            .tokens = { cmd, "press" },
            .args   = { Arg::value },
            .help   = { "Sends a keycode to the keyboard" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            auto code = KeyCode(parseNum(argv[0]));
            emulator.put(Command(Cmd::KEY_PRESS, KeyCommand { .keycode = code, .delay = 0.0 }));
            emulator.put(Command(Cmd::KEY_RELEASE, KeyCommand { .keycode = code, .delay = 0.5 }));
        }
    });
    
    
    //
    // Peripherals (Joystick)
    //
    
    for (isize i = 0; i <= 1; i++) {
        
        if (i == 0) cmd = registerComponent(controlPort1.joystick);
        if (i == 1) cmd = registerComponent(controlPort2.joystick);
        
        root.add({
                 
            .tokens = { cmd, "press" },
            .args   = { Arg::value },
            .help   = { "Presses a joystick button" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                auto nr = parseNum(argv[0]);
                
                switch (nr) {
                        
                    case 1: port.joystick.trigger(GamePadAction::PRESS_FIRE); break;
                    case 2: port.joystick.trigger(GamePadAction::PRESS_FIRE2); break;
                    case 3: port.joystick.trigger(GamePadAction::PRESS_FIRE3); break;
                        
                    default:
                        throw CoreError(Fault::OPT_INV_ARG, "1...3");
                }
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "unpress" },
            .args   = { Arg::value },
            .help   = { "Releases a joystick button" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                auto nr = parseNum(argv[0]);
                
                switch (nr) {
                        
                    case 1: port.joystick.trigger(GamePadAction::RELEASE_FIRE); break;
                    case 2: port.joystick.trigger(GamePadAction::RELEASE_FIRE2); break;
                    case 3: port.joystick.trigger(GamePadAction::RELEASE_FIRE3); break;
                        
                    default:
                        throw CoreError(Fault::OPT_INV_ARG, "1...3");
                }
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "pull" },
            .help   = { "Pulls the joystick" }
        });
        
        root.add({
            
            .tokens = { cmd, "pull", "left" },
            .help   = { "Pulls the joystick left" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_LEFT);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "pull", "right" },
            .help   = { "Pulls the joystick right" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_RIGHT);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "pull", "up" },
            .help   = { "Pulls the joystick up" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_UP);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "pull", "down" },
            .help   = { "Pulls the joystick down" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_DOWN);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "release" },
            .help   = { "Release a joystick axis" }
        });
        
        root.add({
            
            .tokens = { cmd, "release", "x" },
            .help   = { "Releases the x-axis" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::RELEASE_X);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "release", "y" },
            .help   = { "Releases the y-axis" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::RELEASE_Y);
                
            }, .values = {i}
        });
    }
    
    
    //
    // Peripherals (Mouse)
    //
    
    for (isize i = 0; i <= 1; i++) {
        
        if (i == 0) cmd = registerComponent(controlPort1.mouse);
        if (i == 1) cmd = registerComponent(controlPort2.mouse);
        
        root.add({
            
            .tokens = { cmd, "press" },
            .help   = { "Presses a mouse button" }
        });
        
        root.add({
            
            .tokens = { cmd, "press", "left" },
            .help   = { "Presses the left mouse button" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.mouse.pressAndReleaseLeft();
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "press", "middle" },
            .help   = { "Presses the middle mouse button" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.mouse.pressAndReleaseMiddle();
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "press", "right" },
            .help   = { "Presses the right mouse button" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto &port = (values[0] == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.mouse.pressAndReleaseRight();
                
            }, .values = {i}
        });
    }
    
    
    //
    // Peripherals (Df0, Df1, Df2, Df3)
    //
    
    for (isize i = 0; i <= 3; i++) {
        
        cmd = registerComponent(*df[i]);
        
        if (i >= 1 && i <= 3) {
            
            root.add({
                
                .tokens = { cmd, "connect" },
                .help   = { "Connects the drive" },
                .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                    
                    emulator.set(Opt::DRIVE_CONNECT, true, values);
                    
                }, .values = {i}
            });
            
            root.add({
                
                .tokens = { cmd, "disconnect" },
                .help   = { "Disconnects the drive" },
                .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                    
                    emulator.set(Opt::DRIVE_CONNECT, false, values);
                    
                }, .values = {i}
            });
        }
        
        root.add({
            
            .tokens = { cmd, "eject" },
            .help   = { "Ejects a floppy disk" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                amiga.df[values[0]]->ejectDisk();
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "insert" },
            .args   = { Arg::path },
            .help   = { "Inserts a floppy disk" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto path = argv.front();
                amiga.df[values[0]]->swapDisk(path);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "protect" },
            .help   = { "Enables write protection" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                amiga.df[values[0]]->setFlag(DiskFlags::PROTECTED, true);
                
            }, .values = {i}
        });
 
        root.add({
            
            .tokens = { cmd, "unprotect" },
            .help   = { "Disables write protection" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                amiga.df[values[0]]->setFlag(DiskFlags::PROTECTED, false);
                
            }, .values = {i}
        });
    }
    
    
    //
    // Peripherals (Hd0, Hd1, Hd2, Hd3)
    //
    
    for (isize i = 0; i <= 3; i++) {
        
        cmd = registerComponent(*hd[i]);
        
        root.add({
            
            .tokens = { cmd, "connect" },
            .help   = { "Connects the hard drive" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                emulator.set(Opt::HDC_CONNECT, true, values);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "disconnect" },
            .help   = { "Disconnects the hard drive" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                emulator.set(Opt::HDC_CONNECT, false, values);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "attach" },
            .args   = { Arg::path },
            .help   = { "Attaches a hard drive image" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto path = argv.front();

                // Make sure the hard-drive controller board is plugged in
                emulator.set(Opt::HDC_CONNECT, true, values);

                // Connect the drive
                amiga.hd[values[0]]->init(path);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "geometry" },
            .args   = { "<cylinders>", "<heads>", "<sectors>" },
            .help   = { "Changes the disk geometry" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                auto c = util::parseNum(argv[0]);
                auto h = util::parseNum(argv[1]);
                auto s = util::parseNum(argv[2]);
                
                amiga.hd[values[0]]->changeGeometry(c, h, s);
                
            }, .values = {i}
        });
        
        root.add({
            
            .tokens = { cmd, "protect" },
            .help   = { "Enables write protection" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                amiga.hd[values[0]]->setFlag(DiskFlags::PROTECTED, true);
                
            }, .values = {i}
        });
 
        root.add({
            
            .tokens = { cmd, "unprotect" },
            .help   = { "Disables write protection" },
            .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                
                amiga.hd[values[0]]->setFlag(DiskFlags::PROTECTED, false);
                
            }, .values = {i}
        });
    }
    
    
    //
    // Miscellaneous
    //
    
    RetroShellCmd::currentGroup = "Miscellaneous";
    
    
    //
    // Miscellaneous (Config)
    //
    
    root.add({
        
        .tokens = { "config" },
        .help   = { "Virtual machine configuration" }
    });
    
    root.add({
        
        .tokens = { "config", "" },
        .help   = { "Displays the current configuration" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            std::stringstream ss;
            amiga.exportConfig(ss);
            *this << ss;
        }
    });
    
    root.add({
        
        .tokens = { "config", "diff" },
        .help   = { "Displays the difference to the default configuration" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            std::stringstream ss;
            amiga.exportDiff(ss);
            *this << ss;
        }
    });
    
    
    //
    // Miscellaneous (DMA Debugger)
    //
    
    cmd = registerComponent(dmaDebugger);
    
    root.add({
        
        .tokens = { cmd, "open" },
        .help   = { "Opens the DMA debugger" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            emulator.set(Opt::DMA_DEBUG_ENABLE, true);
        }
    });
    
    root.add({
        
        .tokens = { cmd, "close" },
        .help   = { "Closes the DMA debugger" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            emulator.set(Opt::DMA_DEBUG_ENABLE, false);
        }
    });
    
    
    //
    // Miscellaneous (Logic Analyzer)
    //
    
    cmd = registerComponent(logicAnalyzer);
    
    
    //
    // Miscellaneous (Host)
    //
    
    cmd = registerComponent(host);
    
    root.add({
        
        .tokens = { cmd, "searchpath" },
        .args   = { Arg::path },
        .help   = { "Sets the search path for media files" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            string path = argv.front();
            
            host.setSearchPath(path);
        }
    });
    
    
    //
    // Miscellaneous (Remote server)
    //
    
    
    root.add({
        
        .tokens = { "server" },
        .help   = { "Remote connections" }
    });
    
    root.add({
        
        .tokens = { "server", "" },
        .help   = { "Displays a server status summary" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            dump(remoteManager, Category::Status);
        }
    });
    
    cmd = registerComponent(remoteManager.serServer);
    
    cmd = registerComponent(remoteManager.rshServer);
    
    root.add({
        
        .tokens = { cmd, "start" },
        .help   = { "Starts the retro shell server" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            remoteManager.rshServer.start();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "stop" },
        .help   = { "Stops the retro shell server" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            remoteManager.rshServer.stop();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "disconnect" },
        .help   = { "Disconnects a client" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
                 
                 remoteManager.rshServer.disconnect();
             }
    });
    
    cmd = registerComponent(remoteManager.promServer);
    
    root.add({
        
        .tokens = { cmd, "start" },
        .help   = { "Starts the Prometheus server" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            remoteManager.promServer.start();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "stop" },
        .help   = { "Stops the Prometheus server" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            remoteManager.promServer.stop();
        }
    });
    
    root.add({
        
        .tokens = { cmd, "disconnect" },
        .help   = { "Disconnects a client" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            remoteManager.promServer.disconnect();
        }
    });
    
    cmd = registerComponent(remoteManager.gdbServer);
    
    root.add({
        
        .tokens = { cmd, "attach" },
        .args   = { Arg::process },
        .help   = { "Attaches the GDB server to a process" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            remoteManager.gdbServer.attach(argv.front());
        }
    });
    
    root.add({
        
        .tokens = { cmd, "detach" },
        .help   = { "Detaches the GDB server from a process" },
        .func   = [this] (Arguments& argv, const std::vector<isize> &values) {
            
            remoteManager.gdbServer.detach();
        }
    });
}

}
