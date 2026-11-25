// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

// Functions that report which features the selected CPU model supports. This
// file is included by the main translation unit (Moira.cpp) only.

bool
Moira::hasCPI() const
{
    return moira::hasCPI(cpuModel);
}

bool
Moira::hasMMU() const
{
    return mmu.isPresent();
}

bool
Moira::hasFPU() const
{
    return hasInternalFPU() || hasExternalFPU();
}

bool
Moira::hasInternalFPU() const
{
    return cpuModel == CPUModel::M68040;
}

bool
Moira::hasExternalFPU() const
{
    return moira::hasCPI(cpuModel) && cpModel != CPModel::NONE;
}

bool
Moira::isAvailable(CPUModel model, Instr I) const
{
    return availabilityMask(I) & (1 << (int)model);
}

bool
Moira::isAvailable(CPUModel model, Instr I, Mode M, Size S) const
{
    return availabilityMask(I, M, S) & (1 << (int)model);
}

bool
Moira::isAvailable(CPUModel model, Instr I, Mode M, Size S, u16 ext) const
{
    return availabilityMask(I, M, S, ext) & (1 << (int)model);
}

u32
Moira::getAddrMask() const
{
    switch (cpuModel) {

        case CPUModel::M68000: return addrMask<Core::C68000>();
        case CPUModel::M68010: return addrMask<Core::C68010>();
        
        default:
            return addrMask<Core::C68020>();
    }
}

u32
Moira::getCacrMask() const
{
    switch (cpuModel) {

        case CPUModel::M68020: case CPUModel::M68EC020: return 0x0003;
        case CPUModel::M68030: case CPUModel::M68EC030: return 0x3313;

        // Instruction cache enable (bit 15) and data cache enable (bit 31)
        case CPUModel::M68EC040: case CPUModel::M68LC040: case CPUModel::M68040: return 0x80008000;

        default:
            return 0xFFFF;
    }
}

u16
Moira::availabilityMask(Instr I) const
{
    switch (I) {

        case Instr::BKPT: case Instr::MOVEC: case Instr::MOVES: case Instr::MOVEFCCR: case Instr::RTD:

            return AV::M68010_UP;

        case Instr::CALLM: case Instr::RTM:

            return AV::M68020;

        case Instr::cpGEN: case Instr::cpRESTORE: case Instr::cpSAVE: case Instr::cpScc: case Instr::cpTRAPcc:

            return AV::M68020 | AV::M68030;

        case Instr::BFCHG: case Instr::BFCLR: case Instr::BFEXTS: case Instr::BFEXTU: case Instr::BFFFO:
        case Instr::BFINS: case Instr::BFSET: case Instr::BFTST: case Instr::CAS: case Instr::CAS2:
        case Instr::CHK2: case Instr::CMP2: case Instr::DIVL: case Instr::EXTB: case Instr::MULL:
        case Instr::PACK: case Instr::TRAPCC: case Instr::TRAPCS: case Instr::TRAPEQ: case Instr::TRAPGE:
        case Instr::TRAPGT: case Instr::TRAPHI: case Instr::TRAPLE: case Instr::TRAPLS: case Instr::TRAPLT:
        case Instr::TRAPMI: case Instr::TRAPNE: case Instr::TRAPPL: case Instr::TRAPVC: case Instr::TRAPVS:
        case Instr::TRAPF: case Instr::TRAPT: case Instr::UNPK:

            return AV::M68020_UP;

        case Instr::CINV: case Instr::CPUSH: case Instr::MOVE16:

            return AV::M68040;

        case Instr::PFLUSH: case Instr::PFLUSHA: case Instr::PFLUSHAN: case Instr::PFLUSHN: case Instr::PLOAD:
        case Instr::PMOVE: case Instr::PTEST:

            return AV::MMU;

        case Instr::FABS: case Instr::FADD: case Instr::FBcc: case Instr::FCMP: case Instr::FDBcc: case Instr::FDIV:
        case Instr::FMOVE: case Instr::FMOVEM: case Instr::FMUL: case Instr::FNEG: case Instr::FNOP:
        case Instr::FRESTORE: case Instr::FSAVE: case Instr::FScc: case Instr::FSQRT: case Instr::FSUB:
        case Instr::FTRAPcc: case Instr::FTST:

        case Instr::FSABS: case Instr::FDABS: case Instr::FSADD: case Instr::FDADD: case Instr::FSDIV: case Instr::FDDIV:
        case Instr::FSMOVE: case Instr::FDMOVE: case Instr::FSMUL: case Instr::FDMUL: case Instr::FSNEG:
        case Instr::FDNEG: case Instr::FSSQRT: case Instr::FDSQRT: case Instr::FSSUB: case Instr::FDSUB:

            return AV::FPU;

        case Instr::FACOS: case Instr::FASIN: case Instr::FATAN: case Instr::FATANH: case Instr::FCOS: case Instr::FCOSH:
        case Instr::FETOX: case Instr::FETOXM1: case Instr::FGETEXP: case Instr::FGETMAN: case Instr::FINT:
        case Instr::FINTRZ: case Instr::FLOG10: case Instr::FLOG2: case Instr::FLOGN: case Instr::FLOGNP1:
        case Instr::FMOD: case Instr::FMOVECR: case Instr::FREM: case Instr::FSCAL: case Instr::FSGLDIV:
        case Instr::FSGLMUL: case Instr::FSIN: case Instr::FSINCOS: case Instr::FSINH: case Instr::FTAN:
        case Instr::FTANH: case Instr::FTENTOX: case Instr::FTWOTOX:

            return 0; // M6888x only

        default:

            return AV::M68000_UP;
    }
}

u16
Moira::availabilityMask(Instr I, Mode M, Size S) const
{
    u16 mask = availabilityMask(I);

    switch (I) {

        case Instr::CMPI:

            if (isPrgMode(M)) mask &= AV::M68010_UP;
            break;

        case Instr::CHK: case Instr::LINK: case Instr::BRA: case Instr::BHI: case Instr::BLS: case Instr::BCC: case Instr::BCS:
        case Instr::BNE: case Instr::BEQ: case Instr::BVC: case Instr::BVS: case Instr::BPL: case Instr::BMI: case Instr::BGE:
        case Instr::BLT: case Instr::BGT: case Instr::BLE: case Instr::BSR:

            if (S == Long) mask &= AV::M68020_UP;
            break;

        case Instr::TST:

            if (M == Mode(1) || M >= Mode(9)) mask &= AV::M68020_UP;
            break;

        default:

            break;
    }

    return mask;
}

u16 Moira::availabilityMask(Instr I, Mode M, Size S, u16 ext) const
{
    u16 mask = availabilityMask(I);

    switch (I) {

        case Instr::MOVEC:

            switch (ext & 0x0FFF) {

                case 0x000:
                case 0x001:
                case 0x800:
                case 0x801: mask &= AV::M68010_UP; break;
                case 0x002:
                case 0x803:
                case 0x804: mask &=  AV::M68020_UP; break;
                case 0x802: mask &=  AV::M68020 | AV::M68030; break;
                case 0x003:
                case 0x004:
                case 0x005:
                case 0x006:
                case 0x007:
                case 0x805:
                case 0x806:
                case 0x807: mask &= AV::M68040; break;

                default:
                    break;
            }
            break;

        case Instr::MOVES:

            if (ext & 0x7FF) mask = 0;
            break;

        default:
            break;
    }

    return mask;
}

bool
Moira::isValidExt(Instr I, Mode M, u16 op, u32 ext) const
{
    switch (I) {

        case Instr::BFCHG:     return (ext & 0xF000) == 0;
        case Instr::BFCLR:     return (ext & 0xF000) == 0;
        case Instr::BFEXTS:    return (ext & 0x8000) == 0;
        case Instr::BFEXTU:    return (ext & 0x8000) == 0;
        case Instr::BFFFO:     return (ext & 0x8000) == 0;
        case Instr::BFINS:     return (ext & 0x8000) == 0;
        case Instr::BFSET:     return (ext & 0xF000) == 0;
        case Instr::BFTST:     return (ext & 0xF000) == 0;
        case Instr::CAS:       return (ext & 0xFE38) == 0;
        case Instr::CAS2:      return (ext & 0x0E380E38) == 0;
        case Instr::CHK2:      return (ext & 0x07FF) == 0;
        case Instr::CMP2:      return (ext & 0x0FFF) == 0;
        case Instr::MULL:      return (ext & 0x83F8) == 0;
        case Instr::DIVL:      return (ext & 0x83F8) == 0;

        default:
            fatalError;
    }
}
