// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

bool
Moira::isValidExtFPU(Instr I, Mode M, u16 op, u32 ext) const
{
    auto cod  = xxx_____________ (ext);
    auto mode = ___xx___________ (ext);
    auto fmt  = ___xxx__________ (ext);
    auto lst  = ___xxx__________ (ext);
    auto cmd  = _________xxxxxxx (ext);

    switch (I) {

        case Instr::FDBcc:
        case Instr::FScc:
        case Instr::FTRAPcc:

            return (ext & 0xFFE0) == 0;

        case Instr::FMOVECR:

            return (op & 0x3F) == 0;

        case Instr::FMOVE:

            switch (cod) {

                case 0b010:

                    if (M == Mode::IP) break;
                    return true;

                case 0b000:

                    if (cmd == 0 && cod == 0 && (op & 0x3F)) break;
                    return true;

                case 0b011:

                    if (fmt != 0b011 && fmt != 0b111 && (ext & 0x7F)) break;

                    if (M == Mode::DN) {
                        if (fmt == 0b010 || fmt == 0b011 || fmt == 0b101 || fmt == 0b111) break;
                    }
                    if (M == Mode::AN) {
                        if (fmt == 0b011 || fmt == 0b111) break;
                    }
                    if (M == Mode::DIPC || M == Mode::IXPC || M == Mode::IM || M == Mode::IP) {
                        break;
                    } else {
                        if (fmt == 0b111 && (ext & 0xF)) break;
                    }

                    return true;
            }

        case Instr::FMOVEM:

            switch (cod) {

                case 0b101:
                {

                    if (ext & 0x3FF) break;

                    if (M == Mode::DN || M == Mode::AN) {
                        if (lst != 0b000 && lst != 0b001 && lst != 0b010 && lst != 0b100) break;
                    }
                    if (M == Mode::DIPC || M == Mode::IXPC || M == Mode::IM || M == Mode::IP) {
                        break;
                    }
                    return true;
                }
                case 0b100:

                    if (ext & 0x3FF) break;
                    if (M == Mode::IP) break;
                    return true;

                case 0b110:
                case 0b111:

                    if (ext & 0x0700) break;
                    if (mode == 3 && (ext & 0x8F)) break;

                    if (M == Mode::DN || M == Mode::AN) {
                        break;
                    }
                    if (M == Mode::DIPC || M == Mode::IXPC || M == Mode::IM || M == Mode::IP) {
                        break;
                    }
                    if (M == Mode::AI) {
                        if (mode == 0 || mode == 1) break;
                    }
                    if (M == Mode::PI) {
                        if (mode == 0 || mode == 1 || cod == 0b111) break;
                    }
                    if (M == Mode::PD) {
                        if (cod == 0b110) break;
                        if (cod == 0b111 && (mode == 1) && (ext & 0x8F)) break;
                        if (cod == 0b111 && (mode == 2 || mode == 3)) break;
                    }
                    if (M == Mode::DI || M == Mode::IX || M == Mode::AW || M == Mode::AL) {
                        if (mode == 0 || mode == 1) break;
                    }
                    return true;
            }
            return false;

        default:
            fatalError;
    }
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFBcc(u16 opcode)
{
    throw std::runtime_error("Attempt to execute an unsupported FPU instruction.");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFDbcc(u16 opcode)
{
    throw std::runtime_error("Attempt to execute an unsupported FPU instruction.");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFGen(u16 opcode)
{
    throw std::runtime_error("Attempt to execute an unsupported FPU instruction.");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFNop(u16 opcode)
{
    throw std::runtime_error("Attempt to execute an unsupported FPU instruction.");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFRestore(u16 opcode)
{
    throw std::runtime_error("Attempt to execute an unsupported FPU instruction.");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFSave(u16 opcode)
{
    throw std::runtime_error("Attempt to execute an unsupported FPU instruction.");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFScc(u16 opcode)
{
    throw std::runtime_error("Attempt to execute an unsupported FPU instruction.");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFTrapcc(u16 opcode)
{
    throw std::runtime_error("Attempt to execute an unsupported FPU instruction.");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFMove(u16 opcode)
{
    throw std::runtime_error("Attempt to execute an unsupported FPU instruction.");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFMovecr(u16 opcode)
{
    throw std::runtime_error("Attempt to execute an unsupported FPU instruction.");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFMovem(u16 opcode)
{
    throw std::runtime_error("Attempt to execute an unsupported FPU instruction.");
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFGeneric(u16 opcode)
{
    throw std::runtime_error("Attempt to execute an unsupported FPU instruction.");
}
