// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

template <Core C, Instr I, Mode M, Size S> void
Moira::execFBcc(u16 opcode)
{
    execLineF<C, I, M, S>(opcode);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFDbcc(u16 opcode)
{
    execLineF<C, I, M, S>(opcode);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFGen(u16 opcode)
{
    AVAILABILITY(C68000)

    auto ext = queue.irc;
    auto cod  = xxx_____________(ext);
    auto cmd  = _________xxxxxxx(ext);

    // readExt<C>();

    if (M == MODE_AN) {
        if (ext & 0x4000) { execLineF<C, I, M, S>(opcode); return; }
    }
    if (M == MODE_IP) {
        if (cod == 0b010) { execLineF<C, I, M, S>(opcode); return; }
    }

    // Catch FMOVE instructions
    switch (cod) {

        case 0b010:

            if ((ext & 0xFC00) == 0x5C00) { execFMovecr<C, FMOVECR, M, S>(opcode); return; }
            [[fallthrough]];

        case 0b000:

            if (cmd != 0x00 && cmd != 0x40 && cmd != 0x44) break;
            [[fallthrough]];

        case 0b011:

            execFMove<C, FMOVE, M, S>(opcode);
            return;

        case 0b101:
        case 0b100:
        case 0b110:
        case 0b111:

            printf("Calling execFMovem\n");
            execFMovem<C, FMOVEM, M, S>(opcode);
            // breakpointReached(reg.pc0);
            return;
    }

    // Catch all other instructions
    switch (cod) {

        case 0b010:
        case 0b000:

            switch (cmd) {

                case 0x01: printf("TODO: FINT\n"); return; // dasmFGeneric<FINT, M, S>(str, addr, op); return;
                case 0x02: printf("TODO: FSINH\n"); return; // dasmFGeneric<FSINH, M, S>(str, addr, op); return;
                case 0x03: printf("TODO: FINTRZ\n"); return; // dasmFGeneric<FINTRZ, M, S>(str, addr, op); return;
                case 0x04: printf("TODO: FSQRT\n"); return; // dasmFGeneric<FSQRT, M, S>(str, addr, op); return;
                case 0x06: printf("TODO: FLOGNP1\n"); return; // dasmFGeneric<FLOGNP1, M, S>(str, addr, op); return;
                case 0x08: printf("TODO: FETOXM1\n"); return; // dasmFGeneric<FETOXM1, M, S>(str, addr, op); return;
                case 0x09: printf("TODO: FTANH\n"); return; // dasmFGeneric<FTANH, M, S>(str, addr, op); return;
                case 0x0A: printf("TODO: FASIN\n"); return; // dasmFGeneric<FASIN, M, S>(str, addr, op); return;
                case 0x0D: printf("TODO: FATANH\n"); return; // dasmFGeneric<FATANH, M, S>(str, addr, op); return;
                case 0x0E: printf("TODO: FSIN\n"); return; // dasmFGeneric<FSIN, M, S>(str, addr, op); return;
                case 0x0F: printf("TODO: FTAN\n"); return; // dasmFGeneric<FTAN, M, S>(str, addr, op); return;
                case 0x10: printf("TODO: FETOX\n"); return; // dasmFGeneric<FETOX, M, S>(str, addr, op); return;
                case 0x11: printf("TODO: FTWOTOX\n"); return; // dasmFGeneric<FTWOTOX, M, S>(str, addr, op); return;
                case 0x12: printf("TODO: FTENTOX\n"); return; // dasmFGeneric<FTENTOX, M, S>(str, addr, op); return;
                case 0x14: printf("TODO: FLOGN\n"); return; // dasmFGeneric<FLOGN, M, S>(str, addr, op); return;
                case 0x15: printf("TODO: FLOG10\n"); return; // dasmFGeneric<FLOG10, M, S>(str, addr, op); return;
                case 0x16: printf("TODO: FLOG2\n"); return; // dasmFGeneric<FLOG2, M, S>(str, addr, op); return;
                case 0x18: printf("TODO: FABS\n"); return; // dasmFGeneric<FABS, M, S>(str, addr, op); return;
                case 0x19: printf("TODO: FCOSH\n"); return; // dasmFGeneric<FCOSH, M, S>(str, addr, op); return;
                case 0x1A: printf("TODO: FNEG\n"); return; // dasmFGeneric<FNEG, M, S>(str, addr, op); return;
                case 0x1C: printf("TODO: FACOS\n"); return; // dasmFGeneric<FACOS, M, S>(str, addr, op); return;
                case 0x1D: printf("TODO: FCOS\n"); return; // dasmFGeneric<FCOS, M, S>(str, addr, op); return;
                case 0x1E: printf("TODO: FGETEXP\n"); return; // dasmFGeneric<FGETEXP, M, S>(str, addr, op); return;
                case 0x1F: printf("TODO: FGETMAN\n"); return; // dasmFGeneric<FGETMAN, M, S>(str, addr, op); return;
                case 0x20: printf("TODO: FDIV\n"); return; // dasmFGeneric<FDIV, M, S>(str, addr, op); return;
                case 0x21: printf("TODO: FMOD\n"); return; // dasmFGeneric<FMOD, M, S>(str, addr, op); return;
                case 0x22: printf("TODO: FADD\n"); return; // dasmFGeneric<FADD, M, S>(str, addr, op); return;
                case 0x23: printf("TODO: FMUL\n"); return; // dasmFGeneric<FMUL, M, S>(str, addr, op); return;
                case 0x24: printf("TODO: FSGLDIV\n"); return; // dasmFGeneric<FSGLDIV, M, S>(str, addr, op); return;
                case 0x25: printf("TODO: FREM\n"); return; // dasmFGeneric<FREM, M, S>(str, addr, op); return;
                case 0x26: printf("TODO: FSCAL\n"); return; // dasmFGeneric<FSCAL, M, S>(str, addr, op); return;
                case 0x27: printf("TODO: FSGLMUL\n"); return; // dasmFGeneric<FSGLMUL, M, S>(str, addr, op); return;
                case 0x28: printf("TODO: FSUB\n"); return; // dasmFGeneric<FSUB, M, S>(str, addr, op); return;
                case 0x30: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x31: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x32: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x33: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x34: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x35: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x36: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x37: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x38: printf("TODO: FCMP\n"); return; // dasmFGeneric<FCMP, M, S>(str, addr, op); return;
                case 0x3A: printf("TODO: FTST\n"); return; // dasmFGeneric3<FTST, M, S>(str, addr, op); return;
                case 0x41: printf("TODO: FSSQRT\n"); return; // dasmFGeneric<FSSQRT, M, S>(str, addr, op); return;
                case 0x45: printf("TODO: FDSQRT\n"); return; // dasmFGeneric<FDSQRT, M, S>(str, addr, op); return;
                case 0x58: printf("TODO: FSABS\n"); return; // dasmFGeneric<FSABS, M, S>(str, addr, op); return;
                case 0x5A: printf("TODO: FSNEG\n"); return; // dasmFGeneric<FSNEG, M, S>(str, addr, op); return;
                case 0x5C: printf("TODO: FDABS\n"); return; // dasmFGeneric<FDABS, M, S>(str, addr, op); return;
                case 0x5E: printf("TODO: FDNEG\n"); return; // dasmFGeneric<FDNEG, M, S>(str, addr, op); return;
                case 0x60: printf("TODO: FSDIV\n"); return; // dasmFGeneric<FSDIV, M, S>(str, addr, op); return;
                case 0x62: printf("TODO: FSADD\n"); return; // dasmFGeneric<FSADD, M, S>(str, addr, op); return;
                case 0x63: printf("TODO: FSMUL\n"); return; // dasmFGeneric<FSMUL, M, S>(str, addr, op); return;
                case 0x64: printf("TODO: FDDIV\n"); return; // dasmFGeneric<FDDIV, M, S>(str, addr, op); return;
                case 0x66: printf("TODO: FDADD\n"); return; // dasmFGeneric<FDADD, M, S>(str, addr, op); return;
                case 0x67: printf("TODO: FDMUL\n"); return; // dasmFGeneric<FDMUL, M, S>(str, addr, op); return;
                case 0x68: printf("TODO: FSSUB\n"); return; // dasmFGeneric<FSSUB, M, S>(str, addr, op); return;
                case 0x6C: printf("TODO: FDSUB\n"); return; // dasmFGeneric<FDSUB, M, S>(str, addr, op); return;
            }
            break;
    }

    execLineF<C, I, M, S>(opcode);

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFNop(u16 opcode)
{
    execLineF<C, I, M, S>(opcode);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFRestore(u16 opcode)
{
    execLineF<C, I, M, S>(opcode);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFSave(u16 opcode)
{
    execLineF<C, I, M, S>(opcode);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFScc(u16 opcode)
{
    execLineF<C, I, M, S>(opcode);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFTrapcc(u16 opcode)
{
    execLineF<C, I, M, S>(opcode);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFMove(u16 opcode)
{
    AVAILABILITY(C68000);

    auto ext = readExt<C, Word>();
    auto reg = _____________xxx (opcode);
    auto cod = xxx_____________ (ext);
    auto src = ___xxx__________ (ext);
    auto dst = ______xxx_______ (ext);
    auto fac = _________xxxxxxx (ext);

    // Catch illegal extension words
    if (!fpu.isValidExt(I, M, opcode, ext)) {

        execLineF<C, I, M, S>(opcode);
        return;
    }

    switch (cod) {

        case 0b000:

            printf("FMOVE: cod = 000\n");
            /*
             if (fac == 0x40) str << Ins<FSMOVE>{} << Ffmt{2};
             else if (fac == 0x44) str << Ins<FDMOVE>{} << Ffmt{2};
             else str << Ins<I>{} << Ffmt{2};

             str << str.tab << Fp(src) << Sep{} << Fp(dst);
             */
            break;

        case 0b010:

            printf("FMOVE: cod = 010\n");
            /*
             if (fac == 0x40) str << Ins<FSMOVE>{} << Ffmt{src};
             else if (fac == 0x44) str << Ins<FDMOVE>{} << Ffmt{src};
             else str << Ins<I>{} << Ffmt{src};
             */

            if (M == MODE_IM) {

                u64 val;

                switch (src) {

                    case 0: // Long-Word Integer

                        printf("FMOVE IM: Long-Word Integer\n");
                        /*
                         val = dasmIncRead<Long>(addr);
                         str << str.tab << Ims<Long>(u32(val)) << Sep{} << Fp(dst);
                         */
                        break;

                    case 1: // Single precision

                        printf("FMOVE IM: Single precision\n");
                        /*
                         val = dasmIncRead<Long>(addr);
                         str << str.tab << "#<fixme>" << Sep{} << Fp(dst);
                         */
                        break;

                    case 2: // Double precision

                        printf("FMOVE IM: Double precision (fallthrough)\n");
                        break;

                    case 3: // Packed-Decimal Real

                        printf("FMOVE IM: Single precision\n");
                        /*
                         val = dasmIncRead<Long>(addr);
                         dasmIncRead<Long>(addr);
                         dasmIncRead<Long>(addr); // Why???
                         str << str.tab << "#<fixme>" << Sep{} << Fp(dst);
                         */
                        break;

                    case 5: // Double-precision real

                        printf("FMOVE IM: Double-precision real\n");
                        /*
                         val = dasmIncRead<Long>(addr);
                         dasmIncRead<Long>(addr);
                         str << str.tab << "#<fixme>" << Sep{} << Fp(dst);
                         */
                        break;

                    case 6: // Byte Integer

                        printf("FMOVE IM: Byte Integer\n");
                        /*
                         val = dasmIncRead<Word>(addr);
                         str << str.tab << Ims<Byte>(u32(val)) << Sep{} << Fp(dst);
                         */
                        break;

                    default:

                        printf("FMOVE IM: Default\n");
                }
            } else {

                printf("FMOVE (MODE != IM)\n");
            }
            break;

        case 0b011:

            switch (src) {

                case 0b011:

                    printf("FMOVE (0b011, 0b011)\n");
                    /*
                     str << Ins<I>{} << Ffmt{src} << str.tab << Fp(dst) << Sep{} << Op<M, Long>(reg, addr);
                     str << "{" << Ims<Byte>(i8(fac << 1) >> 1) << "}";
                     */
                    break;

                case 0b111:

                    printf("FMOVE (0b011, 0b111)\n");
                    /*
                     str << Ins<I>{} << Ffmt{3} << str.tab << Fp{dst} << Sep{} << Op<M, Long>(reg, addr);
                     str << Sep{} << Dn(fac >> 4);
                     */
                    break;

                default:

                    printf("FMOVE (0b011, default)\n");
                    u32 data = softfloat::floatx80_to_int32(fpu.fpr[dst].raw);
                    writeOp<C, M, Word>(reg, data);

                    /*
                     str << Ins<I>{} << Ffmt{src} << str.tab << Fp{dst} << Sep{} << Op<M, Long>(reg, addr);
                     */
                    break;
            }
            break;
    }

    prefetch<C>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFMovecr(u16 opcode)
{
    AVAILABILITY(C68000);

    auto ext = readExt<C,Word>();
    auto dst = ______xxx_______ (ext);
    auto ofs = _________xxxxxxx (ext);

    // Catch illegal extension words
    if (!fpu.isValidExt(I, M, opcode, ext)) {

        execLineF<C, I, M, S>(opcode);
        return;
    }

    printf("execFMovecr %x -> %d\n", ofs, dst);

    switch (ofs)
    {
        case 0x0:   fpu.setFPR(dst, 0x4000, 0xc90fdaa22168c235); break; // pi
        case 0xb:   fpu.setFPR(dst, 0x3ffd, 0x9a209a84fbcff798); break; // log10(2)
        case 0xc:   fpu.setFPR(dst, 0x4000, 0xadf85458a2bb4a9b); break; // e
        case 0xd:   fpu.setFPR(dst, 0x3fff, 0xb8aa3b295c17f0bc); break; // log2(e)
        case 0xe:   fpu.setFPR(dst, 0x3ffd, 0xde5bd8a937287195); break; // log10(e)
        case 0xf:   fpu.setFPR(dst, 0x0000, 0x0000000000000000); break; // 0.0
        case 0x30:  fpu.setFPR(dst, 0x3ffe, 0xb17217f7d1cf79ac); break;  // ln(2)
        case 0x31:  fpu.setFPR(dst, 0x4000, 0x935d8dddaaa8ac17); break;   // ln(10)
        case 0x32:  fpu.setFPR(dst, 0x0000, 0x0000000000000001); break;   // 10^0
        case 0x33:  fpu.setFPR(dst, 0x0000, 0x000000000000000A); break;   // 10^1
        case 0x34:  fpu.setFPR(dst, 0x0000, 0x0000000000000064); break;   // 10^2

        default:
            printf("Unknown FPU ROM constant (%02x)\n", ofs);
            fpu.setFPR(dst, 0, 0);
            break;
    }

    prefetch<C>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFMovem(u16 opcode)
{
    // auto old = addr;
    auto ext = queue.irc; //  dasmIncRead(addr);
    auto reg = _____________xxx (opcode);
    auto cod = xxx_____________ (ext);
    auto mod = ___xx___________ (ext);
    auto rrr = _________xxx____ (ext);
    auto lll = ___xxx__________ (ext);

    printf("execFMovem\n");

    if (!fpu.isValidExt(I, M, opcode, ext)) {

        execLineF<C, I, M, S>(opcode);
        return;
    }

    switch (cod) {

        case 0b100: // Ea to Cntrl

            if (lll == 0 || lll == 1 || lll == 2 || lll == 4) {

                (void)readExt<C,Word>();

                u32 ea, data;
                readOp<C, M, Long, STD_AE_FRAME>(reg, &ea, &data);

                if (lll & 1) { fpu.fpiar = data; }
                if (lll & 2) { fpu.fpsr = data; }
                if (lll & 4) { fpu.fpcr = data; }

                if (lll & 1) { printf("FPIAR = %d\n", data); }
                if (lll & 2) { printf("FPSR = %d\n", data); }
                if (lll & 4) { printf("FPCR = %d\n", data); }

                prefetch<C>();
                return;

            } else {
                printf("TODO: FMOVEM Ea -> Cntrl\n");
            }
            break;

        case 0b101: // Cntrl to Ea

            if (lll == 0 || lll == 1 || lll == 2 || lll == 4) {

                printf("FMOVE Cntrl -> Ea\n");

                (void)readExt<C,Word>();

                u32 data = 0;
                if (lll & 1) { data = fpu.fpiar; }
                if (lll & 2) { data = fpu.fpsr; }
                if (lll & 4) { data = fpu.fpcr; }

                writeOp<C, M, Long>(reg, data);

                prefetch<C>();
                return;

            } else {
                printf("TODO: FMOVEM Cntrl -> Ea\n");
            }
            break;

        case 0b110: // Memory to FPU

            switch (mod) {

                case 0b00: // Static list, predecrement addressing

                    printf("TODO: MEM -> FPU (Static list, predecrement addressing)\n");
                    break;

                case 0b01: // Dynamic list, predecrement addressing

                    printf("TODO: MEM -> FPU (Dynamic list, predecrement addressing)\n");
                    break;

                case 0b10: // Static list, postincrement addressing

                    printf("TODO: MEM -> FPU (Static list, postincrement addressing)\n");
                    break;

                case 0b11: // Dynamic list, postincrement addressing

                    printf("TODO: MEM -> FPU (Dynamic list, postincrement addressing)\n");
                    break;
            }
            break;

        case 0b111: // FPU to memory

            switch (mod) {

                case 0b00: // Static list, predecrement addressing

                    printf("TODO: FPU -> MEM (Static list, predecrement addressing)\n");
                    break;

                case 0b01: // Dynamic list, predecrement addressing

                    printf("TODO: FPU -> MEM (Dynamic list, predecrement addressing)\n");
                    break;

                case 0b10: // Static list, postincrement addressing

                    printf("TODO: FPU -> MEM (Static list, postincrement addressing)\n");
                    break;

                case 0b11: // Dynamic list, postincrement addressing

                    printf("TODO: FPU -> MEM (Dynamic list, postincrement addressing)\n");
                    break;
            }
            break;
    }

    execLineF<C, I, M, S>(opcode);
}
