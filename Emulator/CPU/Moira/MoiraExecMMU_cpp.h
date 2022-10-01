// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

bool mmuDebug;

struct MmuContext {

    u64 addrBits;
    u64 indexBits;
    u8  shiftCnt;

    u32 addr;

    u32 lowerLimit;
    u32 upperLimit;

    u8 idx[5];

    bool wp;
    bool su;

    // DEPRECATED
    [[deprecated]] u32 bitslice(u32 start, u32 length) {

        u64 shifted = u32((u64)addr << start);
        shifted = (shifted << length) >> 32;
        return (u32)shifted;
    };

    u32 getAddrBits() { return u32(addrBits >> 32); }
    u32 getIndexBits() { return u32(indexBits >> 32); }

    u32 nextAddrBits() {

        // Shift the new index into the upper word
        indexBits = (indexBits & 0xFFFFFFFF) << 4;

        // Get the upper word
        auto shift = indexBits >> 32;

        // Remember the total number of shifted bits
        shiftCnt += u8(shift);

        // Shift the new address bits into the upper word
        addrBits = (addrBits & 0xFFFFFFFF) << shift;

        // Return the upper word
        return u32(addrBits >> 32);
    }

    u32 remainingAddrBits() {

        return u32(addrBits) >> shiftCnt;
    }
};

template <Core C, bool write> u32
Moira::translate(u32 addr, u8 fc)
{
    // Only proceed if a MMU capable core is present
    if constexpr (C == C68000 || C == C68010) return addr;
    
    // Only proceed of the selected CPU model has a MMU
    if (!hasMMU()) return addr;
    
    // Only proceed if the MMU is enabled
    if (!(mmu.tc & 0x80000000)) return addr;

    //
    // TODO: LOOKUP IN CACHE
    //
    
    return mmuLookup<C, write>(addr, fc);
}

template <Core C, bool write> u32
Moira::mmuLookup(u32 addr, u8 fc)
{
    MmuContext context { .addr = addr };

    // REMOVE ASAP
    static int tmp = 0;
    mmuDebug = tmp++ < 10 || addr == 0xaff180;

    // Start with the SRP or CRP
    u64 rp = (reg.sr.s && (mmu.tc & 0x02000000)) ? mmu.srp : mmu.crp;

    // Decode the root pointer
    u32 ptr   = u32(rp >> 0)  & 0xFFFFFFF0;
    u32 limit = u32(rp >> 48) & 0x7FFF;

    // Evaluate the limit field
    if (rp & (1LL << 63)) {

        context.lowerLimit = limit;
        context.upperLimit = 0XFFFF;

    } else {

        context.lowerLimit = 0;
        context.upperLimit = limit;
    }

    context.addrBits = addr;
    context.indexBits = mmu.tc << 12;

    // Apply the initial shift (ignore some bits)
    (void)context.nextAddrBits();

    context.idx[0] = u32(mmu.tc >> 16) & 0xF;   // IS  (Initial Shift)
    context.idx[1] = u32(mmu.tc >> 12) & 0xF;   // TIA (Table Index A)
    context.idx[2] = u32(mmu.tc >>  8) & 0xF;   // TIB (Table Index B)
    context.idx[3] = u32(mmu.tc >>  4) & 0xF;   // TIC (Table Index C)
    context.idx[4] = u32(mmu.tc >>  0) & 0xF;   // TID (Table Index D)

    bool fcl = mmu.tc & ( 1 << 24);

    if (mmuDebug) printf("MMU: %s %x (%d %d %d %d %d) [%x,%x]\n",
                         write ? "WRITE" : "READ",
                         addr,
                         context.idx[0], context.idx[1], context.idx[2], context.idx[3], context.idx[4],
                         context.lowerLimit, context.upperLimit);

    // Check the descriptor type
    switch (rp >> 32 & 0x3) {

        case 1:

            /* Early termination. No translation table exists, all memory
             * accesses are calculated by adding the TableA address to the
             * logical address specified.
             */
            if (mmuDebug) printf("Early termination RP -> %x\n", ptr + addr);
            return ptr + addr;

        case 2:
        {
            u32 taddr = rp & 0xFFFFFFF0;

            if (fcl) {

                u32 desc = readMMU32(taddr + 4 * readFC());

                if (mmuDebug) printf("     RP -> FCL %d\n", readFC());

                u32 offset = context.nextAddrBits();

                if (desc & 0x1) {
                    return mmuLookupLong<C, write>('A', desc & 0xFFFFFFF0, offset, context);
                } else {
                    return mmuLookupShort<C, write>('A', desc & 0xFFFFFFF0, offset, context);
                }
            }

            u32 offset = context.nextAddrBits();

            if (mmuDebug) printf("     RP = %08llx -> Short table A[%d]\n", rp, offset);
            return mmuLookupShort<C, write>('A', taddr, offset, context);
        }
        case 3:
        {
            u32 taddr = rp & 0xFFFFFFF0;

            if (fcl) {

                u64 desc = readMMU64(taddr + 8 * readFC());

                if (mmuDebug) printf("     RP -> FCL %d\n", readFC());

                u32 offset = context.nextAddrBits();

                if (desc & 0x10000) {
                    return mmuLookupLong<C, write>('A', desc & 0xFFFFFFF0, offset, context);
                } else {
                    return mmuLookupShort<C, write>('A', desc & 0xFFFFFFF0, offset, context);
                }
            }

            u32 offset = context.nextAddrBits();

            if (mmuDebug) printf("     RP = %016llx -> Long table A[%d]\n", rp, offset);
            return mmuLookupLong<C, write>('A', taddr, offset, context);
        }
        default:

            // TODO: WHICH EXCEPTION? (MMU CONFIG ERROR? BUS ERROR?)
            if (mmuDebug) printf("Invalid RP -> Bus error\n");
            throw BusErrorException();
    }
}

template <Core C, bool write> u32
Moira::mmuLookupShort(char table, u32 taddr, u32 offset, struct MmuContext &c)
{
    // Check offset
    if (offset < c.lowerLimit || offset > c.upperLimit) {

        printf("Short table offset violation: %d [%d;%d]\n", offset, c.lowerLimit, c.upperLimit);
        throw BusErrorException();
    }

    u32 physAddr;
    u32 descriptor = readMMU32(taddr + 4 * offset);

    if (mmuDebug) printf("     %c[%d] = %08x ", table, offset, descriptor);

    switch (descriptor & 0x3) {

        case 0: // Short format invalid descriptor
        {
            if (mmuDebug) printf("Bus error (invalid descriptor)\n");
            throw BusErrorException();
        }
        case 1: // Short format page or early termination descriptor
        {
            // Set U and M bit
            u64 desc2 = descriptor | (1LL << 3);
            if constexpr (write) { desc2 |= (1LL << 4); }
            write16(taddr + 4 * offset + 2, (u16)desc2);

            physAddr = (descriptor & 0xFFFFFF00) + c.remainingAddrBits();

            if (table == 'D') {   // Short format page descriptor

                if (mmuDebug) printf("(short page descriptor) -> %08x\n", physAddr);

            } else {            // Short early termination descriptor

                if (mmuDebug) printf("(short early descriptor) -> %08x\n", physAddr);
            }
            break;
        }
        case 2: // Short format table descriptor or indirect descriptor
        {
            if (table == 'D') { // Indirect descriptor

                taddr = descriptor & 0xFFFFFFFC;

                if (mmuDebug) printf("(short indirect descriptor)\n");

                physAddr = mmuLookupShort<C, write>(table + 1, taddr, 0, c);

            } else {

                // Set U bit
                u64 desc2 = descriptor | (1LL << 3);
                write16(taddr + 4 * offset + 2, (u16)desc2);

                u32 offset = c.nextAddrBits();

                taddr = descriptor & 0xFFFFFFF0;

                if (mmuDebug) printf("(short table descriptor) -> %c[%d]\n", table, offset);

                // Check offset range
                if (offset < c.lowerLimit || offset > c.upperLimit) {

                    if (mmuDebug) printf("     Offset violation %d [%d;%d]\n",
                                         offset, c.lowerLimit, c.upperLimit);
                    throw BusErrorException();
                }

                if constexpr (write) { c.wp |= descriptor & 0x4; }
                c.lowerLimit = 0;
                c.upperLimit = 0xFFFF;

                physAddr = mmuLookupShort<C, write>(table + 1, taddr, offset, c);
            }
            break;
        }
        case 3: // Long format table descriptor
        {
            if (table == 'D') { // Indirect descriptor

                taddr = descriptor & 0xFFFFFFFC;

                if (mmuDebug) printf("(long indirect descriptor)\n");

                physAddr = mmuLookupLong<C, write>(table + 1, taddr, 0, c);

            } else {

                // Set U bit
                u64 desc2 = descriptor | (1LL << 3);
                write16(taddr + 4 * offset + 2, (u16)desc2);

                u32 offset = c.nextAddrBits();

                taddr = descriptor & 0xFFFFFFF0;

                if (mmuDebug) printf("(long table descriptor) -> %c[%d]\n", table, offset);

                // Check offset range
                if (offset < c.lowerLimit || offset > c.upperLimit) {

                    if (mmuDebug) printf("     Offset violation %d [%d;%d]\n",
                                         offset, c.lowerLimit, c.upperLimit);
                    throw BusErrorException();
                }

                if constexpr (write) { c.wp |= descriptor & 0x4; }
                c.lowerLimit = 0;
                c.upperLimit = 0xFFFF;

                physAddr = mmuLookupLong<C, write>(table + 1, taddr, offset, c);
            }
            break;
        }

        default:

            assert(false);
            return 0;
    }

    // Check for write protection
    if constexpr (write) {
        if (c.wp) {

            printf("WRITE PROTECTION VIOLATION\n");
            throw BusErrorException();
        }
    }

    // Check for supervisor protection
    if (c.su && !reg.sr.s) throw BusErrorException();

    return physAddr;
}

template <Core C, bool write> u32
Moira::mmuLookupLong(char table, u32 taddr, u32 offset, struct MmuContext &c)
{
    // Check offset
    if (offset < c.lowerLimit || offset > c.upperLimit) {

        printf("Long table offset violation: %d [%d;%d]\n", offset, c.lowerLimit, c.upperLimit);
        throw BusErrorException();
    }

    u32 physAddr;
    u64 descriptor = readMMU64(taddr + 8 * offset);

    if (mmuDebug) printf("     %c[%d] = %016llx ", table, offset, descriptor);

    if constexpr (write) c.wp |= (descriptor >> 32) & 0x4;
    c.su |= (descriptor >> 32) & 0x100;
    // if (c.su) { printf("Supervisor protection\n"); }

    // Evaluate the limit field
    if (descriptor & (1LL << 63)) {
        c.lowerLimit = u32(descriptor >> 48) & 0x7FFF;
        c.upperLimit = 0xFFFF;
    } else {
        c.lowerLimit = 0;
        c.upperLimit = u32(descriptor >> 48) & 0x7FFF;
    }

    switch ((descriptor >> 32) & 0x3) {

        case 0: // Short format invalid descriptor
        {
            if (mmuDebug) printf("Bus error (invalid descriptor)\n");
            throw BusErrorException();
        }
        case 1: // Long format page or early termination descriptor
        {
            // Set U and M bit
            u64 desc2 = (descriptor >> 32) | (1LL << 3);
            if constexpr (write) { desc2 |= (1LL << 4); }
            write16(taddr + 8 * offset + 2, (u16)desc2);

            physAddr = (descriptor & 0xFFFFFF00) + c.remainingAddrBits();

            if (table == 'D') {   // Long format page descriptor

                if (mmuDebug) printf("(long page descriptor) -> %08x\n", physAddr);

            } else {              // Long early termination descriptor

                if (mmuDebug) printf("(long early descriptor) -> %08x\n", physAddr);
            }
            break;
        }
        default: // Long format table descriptor or indirect descriptor
        {
            if (table == 'D') { // Indirect descriptor

                taddr = descriptor & 0xFFFFFFFC;

                if (mmuDebug) printf("(long indirect descriptor)\n");

                physAddr = mmuLookupLong<C, write>(table, taddr, 0, c);

            } else {

                // Set U bit
                u64 desc2 = (descriptor >> 32) | (1LL << 3);
                write16(taddr + 8 * offset + 2, (u16)desc2);

                u32 offset = c.nextAddrBits();

                taddr = descriptor & 0xFFFFFFF0;

                if (mmuDebug) printf("(long table descriptor) -> %c[%d]\n", table, offset);

                // Check offset range
                if (offset < c.lowerLimit || offset > c.upperLimit) {

                    if (mmuDebug) printf("     Offset violation %d [%d;%d]\n",
                                         offset, c.lowerLimit, c.upperLimit);
                    throw BusErrorException();
                }

                if constexpr (write) { c.wp |= (descriptor >> 32) & 0x4; }
                c.lowerLimit = 0;
                c.upperLimit = 0xFFFF;

                if ((descriptor & 0x3) == 2) {
                    physAddr = mmuLookupShort<C, write>(table + 1, taddr, offset, c);
                } else {
                    physAddr = mmuLookupLong<C, write>(table + 1, taddr, offset, c);
                }
            }
            break;
        }
    }

    // Check for write protection
    if constexpr (write) {
        if (c.wp) {

            printf("WRITE PROTECTION VIOLATION\n");
            throw BusErrorException();
        }
    }

    // Check for supervisor protection
    if (c.su && !reg.sr.s) throw BusErrorException();

    return physAddr;
}

bool
Moira::testTT(u32 ttx, u32 addr, u8 fc, bool rw)
{
    u32 addrBase = ttx >> 24 & 0xFF;
    u32 addrMask = ttx >> 16 & 0xFF;
    auto e       = x_______________ (ttx);
    auto rwb     = ______x_________ (ttx);
    auto rwm     = _______x________ (ttx);
    auto fcBase  = _________xxx____ (ttx);
    auto fcMask  = _____________xxx (ttx);
    
    if (!e) {
        return false;
    }
    if ((addr >> 24 | addrMask) != (addrBase | addrMask)) {
        return false;
    }
    if ((fc | fcMask) != (fcBase | fcMask)) {
        return false;
    }
    if ((rw | rwm) != (rwb | rwm)) {
        return false;
    }
    return true;
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
    
    auto validFC = [&]() {
            return fc() <= 1 || (fc() >= 8); // Binutils checks M68851
    };
    
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
    auto ext = queue.irc;

    // PLOAD: 0010 00x0 000x xxxx
    if ((ext & 0xFDE0) == 0x2000) {

        execPLoad<C, PLOAD, M, S>(opcode);
        return;
    }

    // PFLUSHA: 0010 010x xxxx xxxx
    if ((ext & 0xFE00) == 0x2400) {

        execPFlusha<C, PFLUSHA, M, S>(opcode);
        return;
    }

    // PFLUSH: 001x xx0x xxxx xxxx
    if ((ext & 0xE200) == 0x2000) {

        execPFlush<C, PFLUSH, M, S>(opcode);
        return;
    }

    // PTEST: 100x xxxx xxxx xxxx
    if ((ext & 0xE000) == 0x8000) {

        execPTest<C, PTEST, M, S>(opcode);
        return;
    }

    // PMOVE: 010x xxxx 0000 0000 || 0110 00x0 0000 0000 || 000x xxxx 0000 0000
    if ((ext & 0xE0FF) == 0x4000 || (ext & 0xFDFF) == 0x6000 || (ext & 0xE0FF) == 0x0000) {

        execPMove<C, PMOVE, M, S>(opcode);
        return;
    }

    execIllegal<C, I, M, S>(opcode);
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPFlush(u16 opcode)
{
    AVAILABILITY(C68020)

    debug(MMU_DEBUG, "PFLUSH (68030) not supported yet\n");

    u16 ext   = queue.irc;
    auto reg  = _____________xxx (opcode);
    auto mode = ___xxx__________ (ext);

    // Catch illegal extension words
    if (!isValidExtMMU(I, M, opcode, ext)) {

        execIllegal<C, ILLEGAL, M, S>(opcode);
        return;
    }

    (void)readI<C, Word>();

    if (mode == 0b110) {
        
        u32 ea; u32 data;
        readOp<C68020, M>(reg, &ea, &data);
    }
    
    prefetch<C, POLLIPL>();

    CYCLES_68020(8);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPFlusha(u16 opcode)
{
    AVAILABILITY(C68020)

    debug(MMU_DEBUG, "PFLUSHA (68030) not supported yet\n");

    u16 ext = queue.irc;

    // Catch illegal extension words
    if (!isValidExtMMU(I, M, opcode, ext)) {

        execIllegal<C, ILLEGAL, M, S>(opcode);
        return;
    }

    (void)readI<C, Word>();
    
    prefetch<C, POLLIPL>();

    CYCLES_68020(8);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPFlush40(u16 opcode)
{
    AVAILABILITY(C68020)

    debug(MMU_DEBUG, "PFLUSH (68040) not supported yet\n");

    auto reg  = _____________xxx (opcode);
    auto mode = ___________xx___ (opcode);
    
    printf("Mode = %d\n", mode);

    switch (mode) {

        case 0:
        case 1:
                        
            u32 ea; u32 data;
            readOp<C68020, M>(reg, &ea, &data);
            break;
            
        case 2:
        case 3:

            break;
    }

    prefetch<C, POLLIPL>();

    CYCLES_68020(8);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPLoad(u16 opcode)
{
    AVAILABILITY(C68020)

    u16 ext   = queue.irc;
    auto reg  = _____________xxx (opcode);
    auto rw   = ______x_________ (ext);

    if (rw) {
        debug(MMU_DEBUG, "PLOADR (68030) not supported yet\n");
    } else {
        debug(MMU_DEBUG, "PLOADW (68030) not supported yet\n");
    }
    
    // Catch illegal extension words
    if (!isValidExtMMU(I, M, opcode, ext)) {

        execIllegal<C, ILLEGAL, M, S>(opcode);
        return;
    }

    (void)readI<C, Word>();
        
    u32 ea; u32 data;
    readOp<C68020, M>(reg, &ea, &data);
    
    prefetch<C, POLLIPL>();
    
    CYCLES_68020(8);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPMove(u16 opcode)
{
    AVAILABILITY(C68020)

    u16 ext = queue.irc;
    auto fmt  = xxx_____________ (ext);
    auto preg = ___xxx__________ (ext);
    bool rw   = ______x_________ (ext);

    // Catch illegal extension words
    if (!isValidExtMMU(I, M, opcode, ext)) {

        execIllegal<C, ILLEGAL, M, S>(opcode);
        return;
    }

    (void)readI<C, Word>();

    switch (fmt) {

        case 0:

            switch (preg) {

                case 0b010: execPMove<M>(opcode, REG_TT0, rw); break;
                case 0b011: execPMove<M>(opcode, REG_TT1, rw); break;
            }
            break;

        case 2:

            switch (preg) {

                case 0b000: execPMove<M>(opcode, REG_TC, rw); break;
                case 0b010: execPMove<M>(opcode, REG_SRP, rw); break;
                case 0b011: execPMove<M>(opcode, REG_CRP, rw); break;
            }
            break;

        case 3:

            switch (preg) {

                case 0b000: execPMove<M>(opcode, REG_MMUSR, rw); break;
            }
            break;
    }

    prefetch<C, POLLIPL>();

    CYCLES_68020(8);
    FINALIZE
}

template <Mode M> void
Moira::execPMove(u16 opcode, RegName mmuReg, bool rw)
{
    u32 ea;
    u32 data32;
    u64 data64;

    auto reg  = _____________xxx (opcode);

    if (rw) {

        switch (mmuReg) {

            case REG_MMUSR:

                writeOp<C68020, M, Word>(reg, mmu.mmusr);
                break;

            case REG_TT0:

                writeOp<C68020, M, Long>(reg, mmu.tt0);
                break;

            case REG_TT1:

                writeOp<C68020, M, Long>(reg, mmu.tt1);
                break;

            case REG_TC:

                writeOp<C68020, M, Long>(reg, mmu.tc);
                break;

            case REG_CRP:

                writeOp64<C68020, M>(reg, mmu.crp);
                break;

            case REG_SRP:

                writeOp64<C68020, M>(reg, mmu.srp);
                break;

            default:
                assert(false);
        }

    } else {

        switch (mmuReg) {

            case REG_MMUSR:

                readOp<C68020, M, Word>(reg, &ea, &data32);
                mmu.mmusr = u16(data32);
                break;

            case REG_TT0:

                readOp<C68020, M, Long>(reg, &ea, &data32);
                mmu.tt0 = data32;
                break;

            case REG_TT1:

                readOp<C68020, M, Long>(reg, &ea, &data32);
                mmu.tt1 = data32;
                break;

            case REG_TC:

                readOp<C68020, M, Long>(reg, &ea, &data32);
                mmu.tc = data32;
                break;

            case REG_CRP:

                readOp64<M, Word>(reg, &ea, &data64);
                mmu.crp = data64;
                break;

            case REG_SRP:

                readOp64<M, Word>(reg, &ea, &data64);
                mmu.srp = data64;
                break;

            default:
                assert(false);
        }
    }
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPTest(u16 opcode)
{
    AVAILABILITY(C68020)
    
    u16 ext  = queue.irc;
    auto rg  = _____________xxx (opcode);
    auto fc  = ___________xxxxx (ext);
    auto rw  = ______x_________ (ext);

    // Catch illegal extension words
    if (!isValidExtMMU(I, M, opcode, ext)) {

        execIllegal<C, ILLEGAL, M, S>(opcode);
        return;
    }

    (void)readI<C, Word>();
        
    u32 ea = 0; u32 data;
    readOp<C68020, M, Long>(rg, &ea, &data);
    
    u8 fcode;
    if (fc == 0) { fcode = (u8)reg.sfc; }
    else if (fc == 1) { fcode = (u8)reg.dfc; }
    else if (fc & 0b01000) { fcode = readD(fc & 0b111) & 0b111; }
    else { fcode = fc & 0b111; }
    
    if (testTT(mmu.tt0, ea, fcode, rw) || testTT(mmu.tt1, ea, fcode, rw)) {
        mmu.mmusr |= 0x40;
    } else {
        mmu.mmusr &= ~0x40;
    }
    
    prefetch<C, POLLIPL>();

    CYCLES_68020(8);
    FINALIZE
}

template <Core C, Instr I, Mode M, Size S> void
Moira::execPTest40(u16 opcode)
{
    AVAILABILITY(C68020)
    
    auto rg  = _____________xxx (opcode);
    auto rw  = __________x_____ (opcode);
        
    if (rw) {
        debug(MMU_DEBUG, "PTESTR (68040) not supported yet\n");
    } else {
        debug(MMU_DEBUG, "PTESTW (68040) not supported yet\n");
    }

    u32 ea = 0; u32 data;
    readOp<C68020, M, Long>(rg, &ea, &data);
    
    u8 fcode = u8(reg.dfc);
    if (testTT(mmu.tt0, ea, fcode, rw) || testTT(mmu.tt1, ea, fcode, rw)) {
        mmu.mmusr |= 0x40;
    } else {
        mmu.mmusr &= ~0x40;
    }
    
    prefetch<C, POLLIPL>();

    CYCLES_68020(8);
    FINALIZE
}
