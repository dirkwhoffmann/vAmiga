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

    auto cnd = ___________xxxxx (opcode);
    u32 oldpc = reg.pc;
            
    if (fpu.cpcc(cnd)) {

        u32 disp = queue.irc;

        if (S == Long) {

            readExt<C>();
            disp = disp << 16 | queue.irc;
        }

        u32 newpc = U32_ADD(oldpc, SEXT<S>(disp));

        // Check for address error
        if (misaligned<C>(newpc)) {
            throw AddressError(makeFrame(newpc));
        }

        // Take branch
        reg.pc = newpc;
        fullPrefetch<C, POLL>();

    } else {

        // Fall through to next instruction
        readExt<C>();
        if constexpr (S == Long) readExt<C>();
        prefetch<C>();
    }
    
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFDbcc(u16 opcode)
{
    AVAILABILITY(C68000)

    auto ext = readExt<C,Word>();
    auto cnd = ___________xxxxx (ext);

    if (!fpu.cpcc(cnd)) {

        int dn = _____________xxx(opcode);
        u32 newpc = U32_ADD(reg.pc, (i16)queue.irc);

        bool takeBranch = readD<Word>(dn) != 0;

        // Check for address error
        if (misaligned<C, S>(newpc)) {
            throw AddressError(makeFrame<AE_INC_PC>(newpc, newpc));
        }

        // Decrement loop counter
        writeD<Word>(dn, U32_SUB(readD<Word>(dn), 1));

        // Branch
        if (takeBranch) {
            
            reg.pc = newpc;
            fullPrefetch<C, POLL>();

        } else {

            (void)read<C, MEM_PROG, Word>(reg.pc + 2);
            reg.pc += 2;
            fullPrefetch<C, POLL>();
        }

    } else {

        // Fall through to next instruction
        reg.pc += 2;
        fullPrefetch<C, POLL>();
    }
    
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFGen(u16 opcode)
{
    AVAILABILITY(C68000)

    auto ext = queue.irc;
    auto cod  = xxx_____________(ext);
    auto cmd  = _________xxxxxxx(ext);

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

                case 0x01: execFGeneric<C, FINT, M, S>(opcode); return;
                case 0x02: execFGeneric<C, FSINH, M, S>(opcode); return;
                case 0x03: execFGeneric<C, FINTRZ, M, S>(opcode); return;
                case 0x04: execFGeneric<C, FSQRT, M, S>(opcode); return;
                case 0x06: execFGeneric<C, FLOGNP1, M, S>(opcode); return;
                case 0x08: execFGeneric<C, FETOXM1, M, S>(opcode); return;
                case 0x09: execFGeneric<C, FTANH, M, S>(opcode); return;
                case 0x0A: execFGeneric<C, FATAN, M, S>(opcode); return;
                case 0x0C: execFGeneric<C, FASIN, M, S>(opcode); return;
                case 0x0D: execFGeneric<C, FATANH, M, S>(opcode); return;
                case 0x0E: execFGeneric<C, FSIN, M, S>(opcode); return;
                case 0x0F: execFGeneric<C, FTAN, M, S>(opcode); return;
                case 0x10: execFGeneric<C, FETOX, M, S>(opcode); return;
                case 0x11: execFGeneric<C, FTWOTOX, M, S>(opcode); return;
                case 0x12: execFGeneric<C, FTENTOX, M, S>(opcode); return;
                case 0x14: execFGeneric<C, FLOGN, M, S>(opcode); return;
                case 0x15: execFGeneric<C, FLOG10, M, S>(opcode); return;
                case 0x16: execFGeneric<C, FLOG2, M, S>(opcode); return;
                case 0x18: execFGeneric<C, FABS, M, S>(opcode); return;
                case 0x19: execFGeneric<C, FCOSH, M, S>(opcode); return;
                case 0x1A: execFGeneric<C, FNEG, M, S>(opcode); return;
                case 0x1C: execFGeneric<C, FACOS, M, S>(opcode); return;
                case 0x1D: execFGeneric<C, FCOS, M, S>(opcode); return;
                case 0x1E: execFGeneric<C, FGETEXP, M, S>(opcode); return;
                case 0x1F: execFGeneric<C, FGETMAN, M, S>(opcode); return;
                case 0x20: execFGeneric<C, FDIV, M, S>(opcode); return;
                case 0x21: execFGeneric<C, FMOD, M, S>(opcode); return;
                case 0x22: execFGeneric<C, FADD, M, S>(opcode); return;
                case 0x23: execFGeneric<C, FMUL, M, S>(opcode); return;
                case 0x24: execFGeneric<C, FSGLDIV, M, S>(opcode); return;
                case 0x25: execFGeneric<C, FREM, M, S>(opcode); return;
                case 0x26: execFGeneric<C, FSCAL, M, S>(opcode); return;
                case 0x27: printf("TODO: FSGLMUL\n"); execFGeneric<C, FSGLMUL, M, S>(opcode); return;
                case 0x28: execFGeneric<C, FSUB, M, S>(opcode); return;
                case 0x30:
                case 0x31:
                case 0x32:
                case 0x33:
                case 0x34:
                case 0x35:
                case 0x36:
                case 0x37: execFGeneric<C, FSINCOS, M, S>(opcode); return;
                case 0x38: execFGeneric<C, FCMP, M, S>(opcode); return;
                case 0x3A: execFGeneric<C, FTST, M, S>(opcode); return;
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
    AVAILABILITY(C68000)

    prefetch<C>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFRestore(u16 opcode)
{
    AVAILABILITY(C68000);

    auto n   = _____________xxx (opcode);

    auto ea = computeEA<C68020, M, S>(n);
    auto fmtWord = readM<C, M, Long>(ea);
    auto type = fpu.typeOfFrame(fmtWord);
    // if (M == MODE_PI) U32_INC(reg.a[n], fpu.stateFrameSize(type) + 4);
    if (type == FPU_FRAME_NULL) fpu.reset();
        
    updateAn(M, Size(fpu.stateFrameSize(type) + 4), n);
    prefetch<C>();
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFSave(u16 opcode)
{
    AVAILABILITY(C68000);

    auto rg = _____________xxx (opcode);
    auto ea = computeEA<C68020, M, Long>(rg);

    // Depending on the current state, the FPU creates different frame types
    FpuFrameType type = fpu.inResetState() ? FPU_FRAME_NULL : FPU_FRAME_IDLE;
    auto size = fpu.stateFrameSize(type);

    switch (type) {
            
        case FPU_FRAME_NULL:

            writeM<C68020, M, Long>(ea, fpu.computeFormatWord(FPU_FRAME_NULL));
            updateAn(M, Long, rg);
            break;
            
        case FPU_FRAME_IDLE:
                        
            if (M == MODE_PD) {
                
                updateAn(M, Size(size + 4), rg);

                writeM<C68020, M, Long>(ea, 0x70000000);
                ea -= 4;
                for (isize i = 0; i < (size / 4) - 1; i++) {
                    writeM<C68020, M, Long>(ea, 0x0);
                    ea -= 4;
                }
                writeM<C68020, M, Long>(ea, fpu.computeFormatWord(FPU_FRAME_IDLE));
                
            } else {
             
                writeM<C68020, M, Long>(ea, fpu.computeFormatWord(FPU_FRAME_IDLE));
                ea += 4;
                for (isize i = 0; i < (size / 4) - 1; i++) {
                    writeM<C68020, M, Long>(ea, 0x0);
                    ea += 4;
                }
                writeM<C68020, M, Long>(ea, 0x70000000);
            }
            break;
            
        default:
            fatalError;
    }
            
    prefetch<C>();
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFScc(u16 opcode)
{
    AVAILABILITY(C68000);

    auto ext = readExt<C, Word>();
    auto reg = _____________xxx (opcode);
    auto cnd = ___________xxxxx (ext);

    auto ea = computeEA<C68020, M, Byte>(reg);
    auto data = fpu.cpcc(cnd) ? 0xFF : 0x00;
    writeM<C68020, M, Byte>(ea, data);
    updateAn<M, Byte>(reg);
    prefetch<C>();

    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execFTrapcc(u16 opcode)
{
    AVAILABILITY(C68000)

    auto mod = _____________xxx (opcode);
    auto cnd = ___________xxxxx (queue.irc);
                    
    if (mod == 0b010) { readExt<C, Word>(); }
    if (mod == 0b011) { readExt<C, Long>(); }

    if (fpu.cpcc(cnd)) {

        // Execute exception handler
        readExt<C>();
        execException<C>(EXC_TRAPV);

    } else {

        // Fall through to next instruction
        readExt<C>();
        prefetch<C>();
    }
    
    FINALIZE
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
    fpu.clearExcStatusByte();

    switch (cod) {

        case 0b000:                                 // FMOVE FpFp

            fpu.fpr[dst].load(fpu.fpr[src].val);
            fpu.setConditionCodes(src);
            break;

        case 0b010:

            if (M == MODE_IM) {                     // FMOVE ImFp

                if (src >= 0 && src <= 6) {

                    auto value = readFpuOpIm<M>(FltFormat(src));
                    
                    if (auto nan = fpu.resolveNan(value); nan) {
                        value = *nan;
                    }
                    fpu.fpr[dst].load(value);
                }

            } else {                                // FMOVE EaFp

                auto value = readFpuOp<M>(reg, FltFormat(src));
                
                if (auto nan = fpu.resolveNan(value); nan) {
                    value = *nan;
                }
                fpu.fpr[dst].load(value);
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
    fpu.clearExcStatusByte();

    fpu.fpr[dst].load(fpu.readCR(ofs));
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

            fpu.clearExcStatusByte();

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

            fpu.clearExcStatusByte();

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
    if (!fpu.isSupported<I>()) {
     
        execLineF<C, I, M, S>(opcode);
        FINALIZE
        return;
    }
    
    auto ext = queue.irc;
    auto nr  = _____________xxx (opcode);
    auto src = ___xxx__________ (ext);
    auto dst = ______xxx_______ (ext);
    
    (void)readExt<C,Word>();

    FpuExtended source, result;

    if (ext & 0x4000) {
        source = readFpuOp<M>(nr, FltFormat(src));
    } else {
        source = fpu.fpr[src].val;
    }

    fpu.clearExcStatusByte();

    if (I == FSINCOS) {
        
        auto dst2 = _____________xxx (ext);
        FpuExtended result2;
        
        if (auto nan = fpu.resolveNan(source); nan) {
            
            result2 = *nan;
            result = *nan;
            
        } else {
            
            result2 = fpu.fcos(source);
            result = fpu.fsin(source);
        }
        
        prefetch<C>();
        fpu.fpr[dst2].load(result2);
        fpu.fpr[dst].load(result);
        fpu.setConditionCodes(dst);
        
        FINALIZE
        return;
        
    } else if (FPU::isMonadic<I>()) {
        
        if (auto nan = fpu.resolveNan(source); nan) {
            
            result = *nan;
            
        } else {
            
            switch (I) {
                    
                case FABS:  result = fpu.fabs(source); break;
                case FACOS: result = fpu.facos(source); break;
                case FASIN: result = fpu.fasin(source); break;
                case FATAN: result = fpu.fatan(source); break;
                case FATANH: result = fpu.fatanh(source); break;
                case FCOSH: result = fpu.fcosh(source); break;
                case FETOX: result = fpu.fetox(source); break;
                case FETOXM1: result = fpu.fetoxm1(source); break;
                case FGETEXP: result = fpu.fgetexp(source); break;
                case FGETMAN: result = fpu.fgetman(source); break;
                case FINT: result = fpu.fint(source); break;
                case FINTRZ: result = fpu.fintrz(source); break;
                case FLOG10: result = fpu.flog10(source); break;
                case FLOG2: result = fpu.flog2(source); break;
                case FLOGN: result = fpu.flogn(source); break;
                case FLOGNP1: result = fpu.flognp1(source); break;
                case FNEG:  result = fpu.fneg(source); break;
                case FSIN:  result = fpu.fsin(source); break;
                case FSINH:  result = fpu.fsinh(source); break;
                case FSQRT:  result = fpu.fsqrt(source); break;
                case FTAN:  result = fpu.ftan(source); break;
                case FTANH:  result = fpu.ftanh(source); break;
                case FTENTOX:  result = fpu.ftentox(source); break;
                case FTST: result = fpu.ftst(source); break;
                case FTWOTOX:  result = fpu.ftwotox(source); break;
                    
                default:
                    result = source;
                    break;
            }
        }

    } else {
        
        printf("Dyadic\n");
        auto dest = fpu.fpr[dst].val;
        
        if (auto nan = fpu.resolveNan(source, dest); nan) {
            
            printf("NaN resolved (dyadic)\n");
            result = *nan;
            
        } else {
            
            printf("switch(I)\n");

            switch (I) {
                    
                case FADD: result = fpu.fadd(source, dest); break;
                case FCMP: result = fpu.fcmp(source, dest); break;
                case FDIV: result = fpu.fdiv(source, dest); break;
                case FMOD: result = fpu.fmod(source, dest); break;
                case FMUL: result = fpu.fmul(source, dest); break;
                case FREM: result = fpu.frem(source, dest); break;
                case FSCAL: result = fpu.fscal(source, dest); break;
                case FSGLDIV: result = fpu.fsgldiv(source, dest); break;
                case FSGLMUL: result = fpu.fsglmul(source, dest); break;
                case FSUB: result = fpu.fsub(source, dest); break;
                    
                default:
                    result = source;
                    break;
            }
        }
    }
    
    if (I == FTST || I == FCMP) {
        
        fpu.setConditionCodes(result);
        
    } else {
        
        fpu.fpr[dst].load(result);
        fpu.setConditionCodes(dst);
    }

    prefetch<C>();
    FINALIZE
}
