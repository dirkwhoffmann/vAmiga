// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

template <Core C, bool write> u32
Moira::translate(u32 addr, u8 fc)
{
    // Only proceed if a MMU capable core is present
    if constexpr (C == C68000 || C == C68010) return addr;

    // Only proceed of the selected CPU model has a MMU
    if (!hasMMU()) return addr;

    // TODO: Translate address
    return addr;
}

bool
Moira::isValidExtMMU(Instr I, Mode M, u16 op, u32 ext)
{
    auto preg  = [ext]() { return ext >> 10 & 0b111;   };
    auto a     = [ext]() { return ext >>  8 & 0b1;     };
    auto mode  = [ext]() { return ext >> 10 & 0b111;   };
    auto mask  = [ext]() { return ext >>  5 & 0b1111;  }; // 68851 mask is 4 bit
    auto reg   = [ext]() { return ext >>  5 & 0b111;   };
    auto fc    = [ext]() { return ext       & 0b11111; };

    auto validFC = [&]() { return fc() <= 1 || (fc() >= 8); }; // Binutils checks M68851

    switch (I) {

        case PFLUSHA:

            return (op & 0xFF) == 0 && mask() == 0 && fc() == 0;

        case PFLUSH:

            // Check mode
            if (mode() != 0b100 && mode() != 0b110) return false;

            // Check EA mode
            if (mode() == 0b110) {
                if (M != MODE_AI && M != MODE_DI && M != MODE_IX && M != MODE_AW && M != MODE_AL) {
                    return false;
                }
            }
            return validFC();

        case PLOAD:

            // Check EA mode
            if (M != MODE_AI && M != MODE_DI && M != MODE_IX && M != MODE_AW && M != MODE_AL) {
                return false;
            }

            return validFC();

        case PMOVE:

            if ((ext & 0x200)) {
                if (M == MODE_DIPC || M == MODE_IXPC || M == MODE_IM) return false;
            }
            if (M == MODE_IP) return false;

            switch (ext >> 13 & 0b111) {

                case 0b000:

                    // Check register field
                    if (preg() != 0b010 && preg() != 0b011) return false;

                    // If memory is written, flushing is mandatory
                    if ((ext & 0x300) == 0x300) return false;
                    return true;

                case 0b010:

                    // If memory is written, flushing is mandatory
                    if ((ext & 0x300) == 0x300) return false;

                    if ((ext & 0x300) == 0) {
                        if (preg() != 0) {
                            if (M == MODE_PI || M == MODE_PD || M == MODE_IM || M == MODE_IP) return false;
                        }
                    }

                    // Check register field (binutils accepts all M68851 registers)
                    if ((ext & 0x100) == 0) {
                        if (preg() != 0) {
                            if (M == MODE_DN || M == MODE_AN) return false;
                        }
                    }
                    return true;

                case 0b011:

                    return true;

                default:
                    return false;
            }
            break;

        case PTEST:

            // When A is 0, reg must be 0
            if (a() == 0 && reg() != 0) return false;

            // Check FC
            if ((fc() & 0b11000) == 0 && (fc() & 0b110) != 0) return false;

            // Check EA mode
            if (M != MODE_AI && M != MODE_DI && M != MODE_IX && M != MODE_AW && M != MODE_AL) return false;

            return true;

        default:
            fatalError;
    }
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPGen(u16 opcode)
{
    printf("TODO: execPGen");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPFlush(u16 opcode)
{
    printf("TODO: execPFlush");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPFlusha(u16 opcode)
{
    printf("TODO: execPFlusha");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPFlush40(u16 opcode)
{
    printf("TODO: execPFlush40");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPLoad(u16 opcode)
{
    printf("TODO: execPLoad");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPMove(u16 opcode)
{
    printf("TODO: execPMove");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPTest(u16 opcode)
{
    printf("TODO: execPTest");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPTest40(u16 opcode)
{
    printf("TODO: execPTest40");
}
