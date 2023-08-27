// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

template <Core C, Instr I, Mode M, Size S> void
Moira::execFBcc(u16 opcode)
{
    AVAILABILITY(C68000)

    fpu.clearFPSR();

    execLineF<C, I, M, S>(opcode);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFDbcc(u16 opcode)
{
    AVAILABILITY(C68000)

    fpu.clearFPSR();

    execLineF<C, I, M, S>(opcode);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFGen(u16 opcode)
{
    AVAILABILITY(C68000)

    auto ext = queue.irc;
    auto cod  = xxx_____________(ext);
    auto cmd  = _________xxxxxxx(ext);

    // printf("execFGen (I = %d M = %d S = %d)\n", I, M, S);

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

            execFMovem<C, FMOVEM, M, S>(opcode);
            return;
    }

    // Catch all other instructions
    switch (cod) {

        case 0b010:
        case 0b000:

            switch (cmd) {

                case 0x01: printf("TODO: FINT\n"); execFGeneric<C, FINT, M, S>(opcode); return;
                case 0x02: printf("TODO: FSINH\n"); execFGeneric<C, FSINH, M, S>(opcode); return;
                case 0x03: printf("TODO: FINTRZ\n"); execFGeneric<C, FINTRZ, M, S>(opcode); return;
                case 0x04: printf("TODO: FSQRT\n"); execFGeneric<C, FSQRT, M, S>(opcode); return;
                case 0x06: printf("TODO: FLOGNP1\n"); execFGeneric<C, FLOGNP1, M, S>(opcode); return;
                case 0x08: printf("TODO: FETOXM1\n"); execFGeneric<C, FETOXM1, M, S>(opcode); return;
                case 0x09: printf("TODO: FTANH\n"); execFGeneric<C, FTANH, M, S>(opcode); return;
                case 0x0A: printf("FASIN\n"); execFGeneric<C, FASIN, M, S>(opcode); return;
                case 0x0D: printf("TODO: FATANH\n"); execFGeneric<C, FATANH, M, S>(opcode); return;
                case 0x0E: printf("FSIN\n"); execFGeneric<C, FSIN, M, S>(opcode); return;
                case 0x0F: printf("TODO: FTAN\n"); execFGeneric<C, FTAN, M, S>(opcode); return;
                case 0x10: printf("TODO: FETOX\n"); execFGeneric<C, FETOX, M, S>(opcode); return;
                case 0x11: printf("TODO: FTWOTOX\n"); execFGeneric<C, FTWOTOX, M, S>(opcode); return;
                case 0x12: printf("TODO: FTENTOX\n"); execFGeneric<C, FTENTOX, M, S>(opcode); return;
                case 0x14: printf("TODO: FLOGN\n"); execFGeneric<C, FLOGN, M, S>(opcode); return;
                case 0x15: printf("TODO: FLOG10\n"); execFGeneric<C, FLOG10, M, S>(opcode); return;
                case 0x16: printf("TODO: FLOG2\n"); execFGeneric<C, FLOG2, M, S>(opcode); return;
                case 0x18: printf("FABS\n"); execFGeneric<C, FABS, M, S>(opcode); return;
                case 0x19: printf("TODO: FCOSH\n"); execFGeneric<C, FCOSH, M, S>(opcode); return;
                case 0x1A: printf("FNEG\n"); execFGeneric<C, FNEG, M, S>(opcode); return;
                case 0x1C: printf("FACOS\n"); execFGeneric<C, FACOS, M, S>(opcode); return;
                case 0x1D: printf("TODO: FCOS\n"); execFGeneric<C, FCOS, M, S>(opcode); return;
                case 0x1E: printf("TODO: FGETEXP\n"); execFGeneric<C, FGETEXP, M, S>(opcode); return;
                case 0x1F: printf("TODO: FGETMAN\n"); execFGeneric<C, FGETMAN, M, S>(opcode); return;
                case 0x20: printf("TODO: FDIV\n"); execFGeneric<C, FDIV, M, S>(opcode); return;
                case 0x21: printf("TODO: FMOD\n"); execFGeneric<C, FMOD, M, S>(opcode); return;
                case 0x22: printf("TODO: FADD\n"); execFGeneric<C, FADD, M, S>(opcode); return;
                case 0x23: printf("TODO: FMUL\n"); execFGeneric<C, FMUL, M, S>(opcode); return;
                case 0x24: printf("TODO: FSGLDIV\n"); execFGeneric<C, FSGLDIV, M, S>(opcode); return;
                case 0x25: printf("TODO: FREM\n"); execFGeneric<C, FREM, M, S>(opcode); return;
                case 0x26: printf("TODO: FSCAL\n"); execFGeneric<C, FSCAL, M, S>(opcode); return;
                case 0x27: printf("TODO: FSGLMUL\n"); execFGeneric<C, FSGLMUL, M, S>(opcode); return;
                case 0x28: printf("TODO: FSUB\n"); execFGeneric<C, FSUB, M, S>(opcode); return;
                case 0x30: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x31: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x32: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x33: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x34: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x35: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x36: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x37: printf("TODO: FSINCOS\n"); return; // dasmFGeneric2<FSINCOS, M, S>(str, addr, op); return;
                case 0x38: printf("TODO: FCMP\n"); execFGeneric<C, FCMP, M, S>(opcode); return;
                case 0x3A: printf("TODO: FTST\n"); return; // dasmFGeneric3<FTST, M, S>(str, addr, op); return;
                case 0x41: printf("TODO: FSSQRT\n"); execFGeneric<C, FSSQRT, M, S>(opcode); return;
                case 0x45: printf("TODO: FDSQRT\n"); execFGeneric<C, FDSQRT, M, S>(opcode); return;
                case 0x58: printf("TODO: FSABS\n"); execFGeneric<C, FSABS, M, S>(opcode); return;
                case 0x5A: printf("TODO: FSNEG\n"); execFGeneric<C, FSNEG, M, S>(opcode); return;
                case 0x5C: printf("TODO: FDABS\n"); execFGeneric<C, FDABS, M, S>(opcode); return;
                case 0x5E: printf("TODO: FDNEG\n"); execFGeneric<C, FDNEG, M, S>(opcode); return;
                case 0x60: printf("TODO: FSDIV\n"); execFGeneric<C, FSDIV, M, S>(opcode); return;
                case 0x62: printf("TODO: FSADD\n"); execFGeneric<C, FSADD, M, S>(opcode); return;
                case 0x63: printf("TODO: FSMUL\n"); execFGeneric<C, FSMUL, M, S>(opcode); return;
                case 0x64: printf("TODO: FDDIV\n"); execFGeneric<C, FDDIV, M, S>(opcode); return;
                case 0x66: printf("TODO: FDADD\n"); execFGeneric<C, FDADD, M, S>(opcode); return;
                case 0x67: printf("TODO: FDMUL\n"); execFGeneric<C, FDMUL, M, S>(opcode); return;
                case 0x68: printf("TODO: FSSUB\n"); execFGeneric<C, FSSUB, M, S>(opcode); return;
                case 0x6C: printf("TODO: FDSUB\n"); execFGeneric<C, FDSUB, M, S>(opcode); return;
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

    // Clear the status register
    fpu.clearFPSR();

    switch (cod) {

        case 0b000:                                 // FMOVE FpFp

            fpu.fpr[dst].set(fpu.fpr[src].val);
            fpu.setConditionCodes(src);
            break;

        case 0b010:

            if (M == MODE_IM) {                     // FMOVE ImFp

                if (src >= 0 && src <= 6) {

                    auto value = readFpuOpIm<M>(FltFormat(src));
                    fpu.fpr[dst].set(value);
                }

            } else {                                // FMOVE EaFp

                auto value = readFpuOp<M>(reg, FltFormat(src));
                fpu.fpr[dst].set(value);
            }
            fpu.setConditionCodes(dst);
            break;

        case 0b011:                                 // FMOVE FpEa

            if (src == 0b011 || src == 0b111) {     // P{#k} || P{Dn}

                // Get the k-factor, either directly or from a register
                int k = (src == 0b011) ? fac : readD(fac >> 4);

                // The k-factor is a sign-extended 7-bit value
                k = i8(k | (k & 0x40) << 1);

                writeFpuOp<M>(reg, fpu.fpr[dst], FLT_PACKED, k);
                break;
            }

            writeFpuOp<M>(reg, fpu.fpr[dst], FltFormat(src));
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

    // Clear the status register
    fpu.clearFPSR();

    fpu.fpr[dst].set(fpu.readCR(ofs));
    fpu.setConditionCodes(dst);

    prefetch<C>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFMovem(u16 opcode)
{
    AVAILABILITY(C68000);

    auto ext = queue.irc;
    auto reg = _____________xxx (opcode);
    auto cod = xxx_____________ (ext);
    auto mod = ___xx___________ (ext);
    auto lll = ___xxx__________ (ext);

    if (!MIMIC_MUSASHI) {

        if (!fpu.isValidExt(I, M, opcode, ext)) {

            execLineF<C, I, M, S>(opcode);
            return;
        }
    }

    auto oldfpsr = fpu.fpsr;    // TODO: IS THIS STILL NEEDED?

    switch (cod) {              // FMOVEM Ea,CR

        case 0b100:
        {
            (void)readExt<C,Word>();

            u32 ea, data;

            if (lll & 4) {      // -> FPCR

                readOp<C, M, Long, STD_AE_FRAME>(reg, &ea, &data);
                U32_INC(ea, 4);
                fpu.setFPCR(data);
            }
            if (lll & 2) {      // -> FPSR

                readOp<C, M, Long, STD_AE_FRAME>(reg, &ea, &data);
                U32_INC(ea, 4);
                fpu.setFPSR(data);
                oldfpsr = fpu.getFPSR();
            }
            if (lll & 1) {      // -> FPIAR

                readOp<C, M, Long, STD_AE_FRAME>(reg, &ea, &data);
                U32_INC(ea, 4);
                fpu.setFPIAR(data);
            }
            prefetch<C>();
            break;
        }
        case 0b101:             // FMOVEM CR,Ea
        {
            (void)readExt<C,Word>();

            if constexpr (M == MODE_PD) {

                u32 data = 0;
                auto ea = computeEA<C, M, Long>(reg);

                if (lll & 1) {  // -> FPCR

                    data = fpu.getFPIAR();
                    writeOp<C, M, Long>(reg, ea, data);
                    updateAn<M, Long>(reg);
                    U32_DEC(ea, 4);
                }
                if (lll & 2) {  // -> FPSR

                    data = oldfpsr;
                    writeOp<C, M, Long>(reg, ea, data);
                    updateAn<M, Long>(reg);
                    U32_DEC(ea, 4);
                }
                if (lll & 4) {  // -> FPIAR

                    data = fpu.getFPCR();
                    writeOp<C, M, Long>(reg, ea, data);
                    updateAn<M, Long>(reg);
                    U32_DEC(ea, 4);
                }

            } else if constexpr (M != MODE_IM && M != MODE_IP) {

                u32 data = 0;
                auto ea = computeEA<C, M, Long>(reg);

                if (lll & 4) {  // -> FPCR

                    data = fpu.getFPCR();
                    writeOp<C, M, Long>(reg, ea, data);
                    updateAn<M, Long>(reg);
                    U32_INC(ea, 4);
                }
                if (lll & 2) {  // -> FPSR

                    data = oldfpsr;
                    writeOp<C, M, Long>(reg, ea, data);
                    updateAn<M, Long>(reg);
                    U32_INC(ea, 4);
                }
                if (lll & 1) {  // -> FPIAR

                    data = fpu.getFPIAR();
                    writeOp<C, M, Long>(reg, ea, data);
                    updateAn<M, Long>(reg);
                    U32_INC(ea, 4);
                }
            }
            prefetch<C>();
            break;
        }
        case 0b110:             // FMOVEM Ea,Fp

            fpu.clearFPSR();

            switch (mod) {

                case 0b10:      // Static list, postincrement addressing
                {
                    auto reglist = ________xxxxxxxx (ext);
                    (void)readExt<C,Word>();

                    auto ea = computeEA<C, M, Extended>(reg);

                    for (isize i = 0; i < 8; i++) {

                        if (reglist & (0x80 >> i)) {

                            auto val = readFpuOpEa<M>(reg, ea, FLT_EXTENDED);
                            fpu.fpr[i].val = val;
                            U32_INC(ea, 12);
                        }
                    }
                    prefetch<C>();
                    break;
                }
                case 0b11:      // Dynamic list, postincrement addressing
                {
                    auto reglist = getD(_________xxx____ (ext)) ;
                    (void)readExt<C,Word>();

                    auto ea = computeEA<C, M, Extended>(reg);

                    for (isize i = 0; i < 8; i++) {

                        if (reglist & (0x80 >> i)) {

                            auto val = readFpuOpEa<M>(reg, ea, FLT_EXTENDED);
                            fpu.fpr[i].val = val;
                            U32_INC(ea, 12);
                        }
                    }
                    prefetch<C>();
                    break;
                }
                default:
                    fatalError;
            }
            break;

        case 0b111:             // FMOVEM Fp,Ea

            fpu.clearFPSR();

            switch (mod) {

                case 0b00:      // Static list, predecrement addressing
                {
                    auto reglist = ________xxxxxxxx (ext);
                    (void)readExt<C,Word>();

                    auto ea = computeEA<C, M, Extended>(reg);

                    for (isize i = 8; i >= 0; i--) {

                        if (reglist & (0x01 << i)) {

                            writeFpuOp<M, FPU_FMOVEM>(reg, ea, fpu.fpr[i], FLT_EXTENDED);
                            U32_DEC(ea, 12);
                        }
                    }
                    prefetch<C>();
                    break;
                }
                case 0b01:      // Dynamic list, predecrement addressing
                {
                    auto reglist = getD(_________xxx____ (ext)) ;
                    (void)readExt<C,Word>();

                    auto ea = computeEA<C, M, Extended>(reg);

                    for (isize i = 8; i >= 0; i--) {

                        if (reglist & (0x01 << i)) {

                            writeFpuOp<M, FPU_FMOVEM>(reg, ea, fpu.fpr[i], FLT_EXTENDED);
                            U32_DEC(ea, 12);
                        }
                    }
                    prefetch<C>();
                    break;
                }
                case 0b10:      // Static list, postincrement addressing
                {
                    auto reglist = ________xxxxxxxx (ext);
                    (void)readExt<C,Word>();

                    auto ea = computeEA<C, M, Extended>(reg);

                    for (isize i = 0; i < 8; i++) {

                        if (reglist & (0x80 >> i)) {

                            writeFpuOp<M, FPU_FMOVEM>(reg, ea, fpu.fpr[i], FLT_EXTENDED);
                            U32_INC(ea, 12);
                        }
                    }
                    prefetch<C>();
                    break;
                }
                case 0b11:      // Dynamic list, postincrement addressing

                    auto reglist = getD(_________xxx____ (ext)) ;
                    (void)readExt<C,Word>();

                    auto ea = computeEA<C, M, Extended>(reg);

                    for (isize i = 0; i < 8; i++) {

                        if (reglist & (0x80 >> i)) {

                            writeFpuOp<M, FPU_FMOVEM>(reg, ea, fpu.fpr[i], FLT_EXTENDED);
                            U32_INC(ea, 12);
                        }
                    }
                    prefetch<C>();
                    break;
            }
            break;
    }

    fpu.fpsr = oldfpsr;
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFGeneric(u16 opcode)
{
    AVAILABILITY(C68000);

    // Filter out unavailable instructions
    if (fpu.getModel() == FPU_68040) {

        switch (I) {

            case FACOS:
            case FASIN:
            case FATANH:
            case FCOS:
            case FCOSH:
            case FETOX:
            case FETOXM1:
            case FGETEXP:
            case FGETMAN:
            // case FINT:
            case FINTRZ:
            case FLOG10:
            case FLOG2:
            case FLOGN:
            case FLOGNP1:
            case FMOD:
            case FREM:
            case FSCAL:
            case FSIN:
            case FSINCOS:
            case FSINH:
            case FTAN:
            case FTANH:
            case FTENTOX:
            case FTWOTOX:

                execLineF<C, I, M, S>(opcode);
                FINALIZE
                return;

            default:
                break;
        }
    }

    auto ext = queue.irc;
    auto reg = _____________xxx (opcode);
    auto src = ___xxx__________ (ext);
    auto dst = ______xxx_______ (ext);
    (void)readExt<C,Word>();

    FpuExtended source, result;

    if (ext & 0x4000) {

        if (M == MODE_IM) {

            switch (src) {

                case 0: // Long-Word Integer

                    /*
                    val = dasmIncRead<Long>(addr);
                    str << Ins<I>{} << Ffmt{src} << str.tab << Ims<Long>(u32(val));
                    */
                    printf("execFGeneric: Long-Word Integer: TODO\n");
                    break;

                case 1: // Single precision

                    /*
                    val = dasmIncRead<Long>(addr);
                    str << Ins<I>{} << Ffmt{src} << str.tab << "#<fixme>";
                    */
                    printf("execFGeneric: Single precision: TODO\n");
                    break;

                case 2: // Double precision

                    printf("execFGeneric: Double precision: TODO\n");
                    break;

                case 3: // Packed-Decimal Real

                    /*
                    val = dasmIncRead<Long>(addr);
                    dasmIncRead<Long>(addr);
                    dasmIncRead<Long>(addr); // Why???
                    str << Ins<I>{} << Ffmt{src} << str.tab << "#<fixme>";
                    */
                    printf("execFGeneric: Packed-Decimal Real: TODO\n");
                    break;

                case 5: // Double-precision real

                    /*
                    val = dasmIncRead<Long>(addr);
                    dasmIncRead<Long>(addr);
                    str << Ins<I>{} << Ffmt{src} << str.tab << "#<fixme>";
                    */
                    printf("execFGeneric: Double-precision real: TODO\n");
                    break;

                case 6: // Byte Integer
                    /*
                    val = dasmIncRead<Word>(addr);
                    str << Ins<I>{} << Ffmt{src} << str.tab << Ims<Byte>(u32(val));
                    */
                    printf("execFGeneric: Byte Integer: TODO\n");
                    break;

                default:

                    /*
                    str << Ins<I>{} << Ffmt{src} << str.tab << Op<M, Word>(reg, addr);
                    */
                    printf("execFGeneric: MODE_M Default: TODO\n");
            }
        } else {
            /*
            str << Ins<I>{} << Ffmt{src} << str.tab << Op<M, Long>(reg, addr);
            */
            printf("execFGeneric: ext & 0x4000 == 1: TODO\n");
        }

    } else {
        // str << Ins<I>{} << Ffmt{2} << str.tab << Fp{src};
        // printf("execFGeneric: ext & 0x4000 == 0: TODO\n");
        source = fpu.fpr[src].val;
    }

    switch (I) {

        case FABS:  result = fpu.fabs(source); break;
        case FACOS: result = fpu.facos(source); break;
        case FASIN: result = fpu.fasin(source); break;

        case FNEG:  result = fpu.fneg(source); break;

        case FSIN:  result = fpu.fsin(source); break;

        default:
            result = source;
            break;
    }

    prefetch<C>();
    fpu.fpr[dst].set(result);
    fpu.setConditionCodes(dst);

    FINALIZE
}
