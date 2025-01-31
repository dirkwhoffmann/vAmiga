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
CommandConsole::initCommands(RetroShellCmd &root)
{
    Console::initCommands(root);
    
    //
    // Regression tester
    //
    
    RetroShellCmd::currentGroup = "Regression testing";
    
    root.add({
        
        .tokens         = {"regression"},
        .hidden         = releaseBuild,
        .help           = "Runs the regression tester"
    });
    
    root.add({
        
        .tokens         = {"regression", "setup"},
        .requiredArgs   = { ConfigSchemeEnum::argList() },
        .optionalArgs   = { Arg::path, Arg::path },
        .help           = "Initializes the test environment",
        .func           = [this](Arguments& argv, i64 value) {
            
            auto scheme = ConfigScheme(parseEnum<ConfigSchemeEnum>(argv[0]));
            auto rom = argv.size() > 1 ? argv[1] : "";
            auto ext = argv.size() > 2 ? argv[2] : "";
            
            amiga.regressionTester.prepare(scheme, rom, ext);
        }
    });
    
    root.add({
        
        .tokens         = {"regression", "run"},
        .requiredArgs   = { Arg::path },
        .help           = "Launches a regression test",
        .func           = [this](Arguments& argv, i64 value) {
            
            amiga.regressionTester.run(argv.front());
        }
    });
    
    root.add({
        
        .tokens         = {"screenshot"},
        .hidden         = releaseBuild,
        .help           = "Manages screenshots"
    });
    
    root.add({
        
        .tokens         = {"screenshot", "set"},
        .help           = "Configure the screenshot"
    });
    
    root.add({
        
        .tokens         = {"screenshot", "set", "filename"},
        .requiredArgs   = { Arg::path },
        .help           = "Assign the screen shot filename",
        .func           = [this](Arguments& argv, i64 value) {
            
            amiga.regressionTester.dumpTexturePath = argv.front();
        }
    });
    
    root.add({
        
        .tokens         = {"screenshot", "set", "cutout"},
        .requiredArgs   = { Arg::value, Arg::value, Arg::value, Arg::value },
        .help           = "Adjust the texture cutout",
        .func           = [this](Arguments& argv, i64 value) {
            
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
        
        .tokens         = {"screenshot", "save"},
        .requiredArgs   = { Arg::path },
        .help           = "Saves a screenshot and exits the emulator",
        .func           = [this](Arguments& argv, i64 value) {
            
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
        
        .tokens         = {cmd, "defaults"},
        .help           = "Displays the user defaults storage",
        .func           = [this](Arguments& argv, i64 value) {
            
            dump(emulator, Category::Defaults);
        }
    });
    
    root.add({
        
        .tokens         = {cmd, "power"},
        .requiredArgs   = { Arg::onoff },
        .help           = "Switches the Amiga on or off",
        .func           = [this](Arguments& argv, i64 value) {
            
            parseOnOff(argv[0]) ? emulator.run() : emulator.powerOff();
        }
    });
    
    root.add({
        
        .tokens         = {cmd, "reset"},
        .help           = "Performs a hard reset",
        .func           = [this](Arguments& argv, i64 value) {
            
            amiga.hardReset();
        }
    });
    
    root.add({
        
        .tokens         = {cmd, "init"},
        .requiredArgs   = { ConfigSchemeEnum::argList() },
        .help           = "Initializes the Amiga with a predefined scheme",
        .func           = [this](Arguments& argv, i64 value) {
            
            auto scheme = ConfigScheme(parseEnum <ConfigSchemeEnum> (argv[0]));
            
            emulator.powerOff();
            emulator.set(scheme);
        }
    });
    
    
    //
    // Components (Memory)
    //
    
    cmd = registerComponent(mem);
    
    root.add({cmd, "load"},
             "Load memory contents from a file");
    
    root.add({cmd, "load", "rom"}, { Arg::path },
             "Installs a Kickstart Rom",
             [this](Arguments& argv, i64 value) {
        
        mem.loadRom(argv.front());
    });
    
    root.add({cmd, "load", "ext"}, { Arg::path },
             "Installs an extension Rom",
             [this](Arguments& argv, i64 value) {
        
        mem.loadExt(argv.front());
    });
    
    root.add({cmd, "load", "bin"}, { Arg::path, Arg::address },
             "Loads a chunk of memory",
             [this](Arguments& argv, i64 value) {
        
        fs::path path(argv[0]);
        mem.debugger.load(path, parseAddr(argv[1]));
    });
    
    root.add({cmd, "save"},
             "Save memory contents to a file");
    
    root.add({cmd, "save", "rom"}, { Arg::path },
             "Saves the Kickstart Rom",
             [this](Arguments& argv, i64 value) {
        
        mem.saveRom(argv[0]);
    });
    
    root.add({cmd, "save", "ext"}, { Arg::path },
             "Saves the extension Rom",
             [this](Arguments& argv, i64 value) {
        
        mem.saveExt(argv[0]);
    });
    
    root.add({cmd, "save", "bin"}, { Arg::path, Arg::address, Arg::count },
             "Loads a chunk of memory",
             [this](Arguments& argv, i64 value) {
        
        fs::path path(argv[0]);
        mem.debugger.save(path, parseAddr(argv[1]), parseNum(argv[2]));
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
    // Components (Serial port)
    //
    
    cmd = registerComponent(serialPort);
    
    root.add({cmd, "send"}, { "<text>" },
             "Sends a text to the serial port",
             [this](Arguments& argv, i64 value) {
        
        amiga.serialPort << argv[0];
    });
    
    
    //
    // Components (DMA Debugger)
    //
    
    cmd = registerComponent(dmaDebugger);
    
    root.add({cmd, "open"},
             "Opens the DMA debugger",
             [this](Arguments& argv, i64 value) {
        
        emulator.set(Opt::DMA_DEBUG_ENABLE, true);
    });
    
    root.add({cmd, "close"},
             "Closes the DMA debugger",
             [this](Arguments& argv, i64 value) {
        
        emulator.set(Opt::DMA_DEBUG_ENABLE, false);
    });
    
    
    //
    // Components (Logic Analyzer)
    //
    
    cmd = registerComponent(logicAnalyzer);
    
    //
    // Ports
    //
    
    RetroShellCmd::currentGroup = "Ports";
    
    //
    // Ports (Audio port)
    //
    
    cmd = registerComponent(audioPort);
    cmd = registerComponent(audioPort.filter);
    // cmd = registerComponent(audioPort.filter, root / cmd);
    
    
    //
    // Ports (Video port)
    //
    
    cmd = registerComponent(videoPort);
    
    
    ///
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
        
        .tokens         = {cmd, "press"},
        .requiredArgs   = { Arg::value },
        .help           = "Sends a keycode to the keyboard",
        .func           = [this](Arguments& argv, i64 value) {
            
            auto code = KeyCode(parseNum(argv[0]));
            emulator.put(Command(Cmd::KEY_PRESS, KeyCommand { .keycode = code, .delay = 0.0 }));
            emulator.put(Command(Cmd::KEY_RELEASE, KeyCommand { .keycode = code, .delay = 0.5 }));
        }
    });
    
    
    //
    // Peripherals (Joystick)
    //
    
    for (isize i = 0; i <= 1; i++) {
        
        auto joystick =
        (i == 0) ? cmd = registerComponent(controlPort1.joystick) :
        (i == 1) ? cmd = registerComponent(controlPort2.joystick) : "???";
        
        root.add({
                 
            .tokens         = {joystick, "press"},
            .requiredArgs   = { Arg::value },
            .help           = "Presses a joystick button",
            .func           = [this](Arguments& argv, i64 value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                auto nr = parseNum(argv[0]);
                
                switch (nr) {
                        
                    case 1: port.joystick.trigger(GamePadAction::PRESS_FIRE); break;
                    case 2: port.joystick.trigger(GamePadAction::PRESS_FIRE2); break;
                    case 3: port.joystick.trigger(GamePadAction::PRESS_FIRE3); break;
                        
                    default:
                        throw VAException(VAError::OPT_INV_ARG, "1...3");
                }
            }, .value = i
        });
        
        root.add({
            
            .tokens         = {joystick, "unpress"},
            .requiredArgs   = { Arg::value },
            .help           = "Releases a joystick button",
            .func           = [this](Arguments& argv, i64 value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                auto nr = parseNum(argv[0]);
                
                switch (nr) {
                        
                    case 1: port.joystick.trigger(GamePadAction::RELEASE_FIRE); break;
                    case 2: port.joystick.trigger(GamePadAction::RELEASE_FIRE2); break;
                    case 3: port.joystick.trigger(GamePadAction::RELEASE_FIRE3); break;
                        
                    default:
                        throw VAException(VAError::OPT_INV_ARG, "1...3");
                }
            }, .value = i
        });
        
        root.add({
            
            .tokens         = {joystick, "pull"},
            .help           = "Pulls the joystick"
        });
        
        root.add({
            
            .tokens         = {joystick, "pull", "left"},
            .help           = "Pulls the joystick left",
            .func           = [this](Arguments& argv, i64 value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_LEFT);
                
            }, .value = i
        });
        
        root.add({
            
            .tokens         = {joystick, "pull", "right"},
            .help           = "Pulls the joystick right",
            .func           = [this](Arguments& argv, i64 value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_RIGHT);
                
            }, .value = i
        });
        
        root.add({
            
            .tokens         = {joystick, "pull", "up"},
            .help           = "Pulls the joystick up",
            .func           = [this](Arguments& argv, i64 value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_UP);
                
            }, .value = i
        });
        
        root.add({
            
            .tokens         = {joystick, "pull", "down"},
            .help           = "Pulls the joystick down",
            .func           = [this](Arguments& argv, i64 value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::PULL_DOWN);
                
            }, .value = i
        });
        
        root.add({
            
            .tokens         = {joystick, "release"},
            .help           = "Release a joystick axis"
        });
        
        root.add({
            
            .tokens         = {joystick, "release", "x"},
            .help           = "Releases the x-axis",
            .func           = [this](Arguments& argv, i64 value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::RELEASE_X);
                
            }, .value = i
        });
        
        root.add({
            
            .tokens         = {joystick, "release", "y"},
            .help           = "Releases the y-axis",
            .func           = [this](Arguments& argv, i64 value) {
                
                auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
                port.joystick.trigger(GamePadAction::RELEASE_Y);
                
            }, .value = i
        });
    }
    
    
    //
    // Peripherals (Mouse)
    //
    
    for (isize i = 0; i <= 1; i++) {
        
        if (i == 0) cmd = registerComponent(controlPort1.mouse);
        if (i == 1) cmd = registerComponent(controlPort2.mouse);
        
        root.add({cmd, "press"},
                 "Presses a mouse button");
        
        root.add({cmd, "press", "left"},
                 "Presses the left mouse button",
                 [this](Arguments& argv, i64 value) {
            
            auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
            port.mouse.pressAndReleaseLeft();
            
        }, i);
        
        root.add({cmd, "press", "middle"},
                 "Presses the middle mouse button",
                 [this](Arguments& argv, i64 value) {
            
            auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
            port.mouse.pressAndReleaseMiddle();
            
        }, i);
        
        root.add({cmd, "press", "right"},
                 "Presses the right mouse button",
                 [this](Arguments& argv, i64 value) {
            
            auto &port = (value == 0) ? amiga.controlPort1 : amiga.controlPort2;
            port.mouse.pressAndReleaseRight();
            
        }, i);
    }
    
    
    //
    // Peripherals (Df0, Df1, Df2, Df3)
    //
    
    for (isize i = 0; i <= 3; i++) {
        
        cmd = registerComponent(*df[i]);
        
        if (i >= 1 && i <= 3) {
            
            root.add({cmd, "connect"},
                     "Connects the drive",
                     [this](Arguments& argv, i64 value) {
                
                emulator.set(Opt::DRIVE_CONNECT, true, { value });
                
            }, i);
            
            root.add({cmd, "disconnect"},
                     "Disconnects the drive",
                     [this](Arguments& argv, i64 value) {
                
                emulator.set(Opt::DRIVE_CONNECT, false, { value });
                
            }, i);
        }
        
        root.add({cmd, "eject"},
                 "Ejects a floppy disk",
                 [this](Arguments& argv, i64 value) {
            
            amiga.df[value]->ejectDisk();
            
        }, i);
        
        root.add({cmd, "insert"}, { Arg::path },
                 "Inserts a floppy disk",
                 [this](Arguments& argv, i64 value) {
            
            auto path = argv.front();
            amiga.df[value]->swapDisk(path);
            
        }, i);
        
        root.add({cmd, "searchpath"}, { Arg::path },
                 "Sets the search path for media files",
                 [this](Arguments& argv, i64 value) {
            
            string path = argv.front();
            
            if (value == 0 || value > 3) df0.setSearchPath(path);
            if (value == 1 || value > 3) df1.setSearchPath(path);
            if (value == 2 || value > 3) df2.setSearchPath(path);
            if (value == 3 || value > 3) df3.setSearchPath(path);
            
        }, i);
    }
    
    
    //
    // Peripherals (Hd0, Hd1, Hd2, Hd3)
    //
    
    for (isize i = 0; i <= 3; i++) {
        
        cmd = registerComponent(*hd[i]);
        
        root.add({cmd, "connect"},
                 "Connects the hard drive",
                 [this](Arguments& argv, i64 value) {
            
            emulator.set(Opt::HDC_CONNECT, true, {value});
            
        }, i);
        
        root.add({cmd, "disconnect"},
                 "Disconnects the hard drive",
                 [this](Arguments& argv, i64 value) {
            
            emulator.set(Opt::HDC_CONNECT, false, {value});
            
        }, i);
        
        root.add({cmd, "attach"}, { Arg::path },
                 "Attaches a hard drive image",
                 [this](Arguments& argv, i64 value) {
            
            auto path = argv.front();
            amiga.hd[value]->init(path);
            
        }, i);
        
        root.add({cmd, "geometry"},  { "<cylinders>", "<heads>", "<sectors>" },
                 "Changes the disk geometry",
                 [this](Arguments& argv, i64 value) {
            
            auto c = util::parseNum(argv[0]);
            auto h = util::parseNum(argv[1]);
            auto s = util::parseNum(argv[2]);
            
            amiga.hd[value]->changeGeometry(c, h, s);
            
        }, i);
    }
    
    
    //
    // Miscellaneous
    //
    
    RetroShellCmd::currentGroup = "Miscellaneous";
    
    //
    // Miscellaneous (Diff)
    //
    
    root.add({"config"},
             "Virtual machine configuration");
    
    root.add({"config", ""},
             "Displays the current configuration",
             [this](Arguments& argv, i64 value) {
        
        std::stringstream ss;
        amiga.exportConfig(ss);
        *this << ss;
    });
    
    root.add({"config", "diff"},
             "Displays the difference to the default configuration",
             [this](Arguments& argv, i64 value) {
        
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
             [this](Arguments& argv, i64 value) {
        
        dump(remoteManager, Category::Status);
    });
    
    cmd = registerComponent(remoteManager.serServer);
    
    cmd = registerComponent(remoteManager.rshServer);
    
    root.add({cmd, "start"},
             "Starts the retro shell server",
             [this](Arguments& argv, i64 value) {
        
        remoteManager.rshServer.start();
    });
    
    root.add({cmd, "stop"},
             "Stops the retro shell server",
             [this](Arguments& argv, i64 value) {
        
        remoteManager.rshServer.stop();
    });
    
    root.add({cmd, "disconnect"},
             "Disconnects a client",
             [this](Arguments& argv, i64 value) {
        
        remoteManager.rshServer.disconnect();
    });
    
    cmd = registerComponent(remoteManager.promServer);
    
    root.add({cmd, "start"},
             "Starts the Prometheus server",
             [this](Arguments& argv, i64 value) {
        
        remoteManager.promServer.start();
    });
    
    root.add({cmd, "stop"},
             "Stops the Prometheus server",
             [this](Arguments& argv, i64 value) {
        
        remoteManager.promServer.stop();
    });
    
    root.add({cmd, "disconnect"},
             "Disconnects a client",
             [this](Arguments& argv, i64 value) {
        
        remoteManager.promServer.disconnect();
    });
    
    cmd = registerComponent(remoteManager.gdbServer);
    
    root.add({cmd, "attach"}, { Arg::process },
             "Attaches the GDB server to a process",
             [this](Arguments& argv, i64 value) {
        
        remoteManager.gdbServer.attach(argv.front());
    });
    
    root.add({cmd, "detach"},
             "Detaches the GDB server from a process",
             [this](Arguments& argv, i64 value) {
        
        remoteManager.gdbServer.detach();
    });
}

}
