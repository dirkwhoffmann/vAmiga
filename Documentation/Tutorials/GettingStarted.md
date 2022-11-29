# Getting started 

## Obtaining 

The recommended way to download Moira is to clone the project directly from GitHub:

````bash
git clone https://github.com/dirkwhoffmann/Moira.git
cd Moira 
````

Older versions can be downloaded from the [Moira release page](https://github.com/dirkwhoffmann/Moira/releases).  

## Configuring 

All compile-time options are configured in a single file called `MoiraConfig.h`. For this tutorial, there is no need to change any of the default settings. However, you may already want to peek inside the file to get an idea about available options:

````bash
cat Moira/MoiraConfig.h
````

All available options are explained in detail in a seperate section.

## Compiling 

The easies way to compile Moira is via `CMake`. Simply create a build directory, invoke `CMake` to setup the build-chain, and call `make` afterwards:

````bash
mkdir build
cd build
cmake ..
make
````

## Running 

If build as explained above, an application called `testRunner` is created. The application iterates over all opcodes, runs Moira and Musashi in parallel, and compares the outcome. If everything works as expected, the application terminates after a few minutes. If a disrepancy is detected, an error rerport is generated. In such a case, please file a bug on GitHub. Every help in ruling out remaining bugs is highly appreciated.

````text
Moira CPU tester. (C) Dirk W. Hoffmann, 2019 - 2022

The test program runs Moira agains Musashi with randomly generated data.

    Test rounds : 1
    Random seed : 442
     Exec range : (opcode >= 0x0000 && opcode <= 0xEFFF)
     Dasm range : (opcode >= 0x0000 && opcode <= 0xFFFF)

Round 1:

68000 CPU ................................ PASSED  (Moira: 0.65s  Musashi: 0.89s)
68010 CPU ................................ PASSED  (Moira: 1.31s  Musashi: 1.78s)
EC020 CPU ................................ PASSED  (Moira: 1.99s  Musashi: 2.70s)
68020 CPU ................................ PASSED  (Moira: 2.67s  Musashi: 3.62s)
EC030 CPU ................................ PASSED  (Moira: 3.34s  Musashi: 4.53s)
68030 CPU ................................ PASSED  (Moira: 4.02s  Musashi: 5.45s)
68030 MMU ................................ PASSED  (Moira: 4.02s  Musashi: 5.45s)
EC040 CPU ................................ PASSED  (Moira: 4.69s  Musashi: 6.35s)
LC040 CPU ................................ PASSED  (Moira: 5.35s  Musashi: 7.26s)
LC040 MMU ................................ PASSED  (Moira: 5.35s  Musashi: 7.26s)
68040 CPU ................................ PASSED  (Moira: 6.04s  Musashi: 8.18s)
68040 MMU ................................ PASSED  (Moira: 6.04s  Musashi: 8.18s)
68040 FPU ................................ PASSED  (Moira: 6.04s  Musashi: 8.18s)

All tests completed
````

Please note that not all test rounds execute instructions. This happens only in the first four rounds, when a 68000, 68010, 68EC020, or 68020 is emulated. For all other CPU models, only the disassembler is invoked. 
