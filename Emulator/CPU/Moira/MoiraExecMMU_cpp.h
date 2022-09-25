// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

template <Core C, bool write> u32
Moira::translate(u32 addr, u8 fc)
{
    MmuDescriptorType type = MmuDescriptorType(0);

    auto bitslice = [addr](u32 start, u32 length) {
    
        u64 shifted = u32((u64)addr << start);
        shifted = (shifted << length) >> 32;
        return (u32)shifted;
    };

    auto name = [](MmuDescriptorType t) {

        switch (t) {
                
            case ShortTable:    return "Short format table descriptor";
            case ShortEarly:    return "Short format early termination descriptor";
            case ShortPage:     return "Short format page descriptor";
            case ShortInvalid:  return "Short format invalid descriptor";
            case ShortIndirect: return "Short format indirect descriptor";
            case LongTable:     return "Long format table descriptor";
            case LongEarly:     return "Long format early termination descriptor";
            case LongPage:      return "Long format page descriptor";
            case LongInvalid:   return "Long format invalid descriptor";
            case LongIndirect:  return "Long format indirect descriptor";
            default:            return "Unknown table descriptor";
        }
    };

    // REMOVE ASAP
    static int tmp = 0;
    
    // Only proceed if a MMU capable core is present
    if constexpr (C == C68000 || C == C68010) return addr;

    // Only proceed of the selected CPU model has a MMU
    if (!hasMMU()) return addr;

    // Only proceed if the MMU is enabled
    if (!(mmu.tc & 0x80000000)) return addr;

    bool debug = tmp++ < 10;

    // Get the root pointer
    u64 rp = (reg.sr.s && (mmu.tc & 0x02000000)) ? mmu.srp : mmu.crp;
    
    // Decode the root pointer
    u32 ptr   = u32(rp >> 0)      & 0xFFFFFFF0;
    u32 limit = u32(rp >> 48)     & 0x7FFF;
    u32 dt    = u32(rp >> 32)     & 0x3;
    
    u32 idx[5];
    idx[0] = u32(mmu.tc >> 16) & 0xF;   // IS  (Initial Shift)
    idx[1] = u32(mmu.tc >> 12) & 0xF;   // TIA (Table Index A)
    idx[2] = u32(mmu.tc >>  8) & 0xF;   // TIB (Table Index B)
    idx[3] = u32(mmu.tc >>  4) & 0xF;   // TIC (Table Index C)
    idx[4] = u32(mmu.tc >>  0) & 0xF;   // TID (Table Index D)
    bool fcl = mmu.tc & ( 1 << 24);

    if (debug) printf("MMU: Mapping %x %s (%d %d %d %d %d)\n",
                      addr,
                      write ? "(WRITE)" : "(READ)",
                      idx[0], idx[1], idx[2], idx[3], idx[4]);
    
    if (limit) {
        
        if (debug) printf("TODO (1): MMU limit field is not supported yet\n");
        assert(0);
        return addr;
    }
        
    // Evaluate the root pointer
    switch (dt) {
            
        case 0:
            /* This value is not allowed in the root pointer and will cause
             * an MMU exception.
             */
            
            // TODO: WHICH EXCEPTION (MMU CONFIG ERROR? BUS ERROR?)
            throw BusErrorException();

        case 1:
            
            /* Early termination. No translation table exists, all memory
             * accesses are calculated by adding the TableA address to the
             * logical address specified.
             */
            return ptr + addr;
            
        case 2:
            
            type = ShortTable;
            
            if (fcl) {
                
                u32 lword0 = readMMU(ptr + 4 * readFC());
                ptr = lword0 & 0xFFFFFFF0;
                type = (lword0 & 1) ? LongTable : ShortTable;
            }
            break;
            
        case 3:
            
            type = LongTable;
            
            if (fcl) {
                
                u32 lword0 = readMMU(ptr + 8 * readFC());
                u32 lword1 = readMMU(ptr + 8 * readFC() + 4);
                ptr = lword1 & 0xFFFFFFF0;
                type = (lword0 & 1) ? LongTable : ShortTable;
            }
            break;
    }
    
    char table = 'A';
    
    // Traverse the translation table
    u32 pos = idx[0];
    u32 len = idx[1];
    u32 physAddr = 0;
    u32 wp = 0;
    
    for (int i = 0;; i++) {
        
        switch (type) {
                
            case ShortTable:
            {
                u32 offset = bitslice(pos, len);
                u32 lword0 = readMMU(ptr + 4 * offset);
                
                if (debug) printf("ShortTable: %c[%d] = %x\n", table, offset, lword0);
                
                ptr = lword0 & 0xFFFFFFF0;
                wp |= lword0 & 0x4;

                switch (lword0 & 0x3) {
                        
                    case 0:

                        type = ShortInvalid;
                        if (debug) printf("%c[%d]: %s\n", table, offset, name(type));
                        continue;
                        
                    case 1:

                        type = table == 'D' ? ShortPage : ShortEarly;
                        if (debug) printf("%c[%d]: %s\n", table, offset, name(type));
                        continue;
                        
                    case 2:
                        
                        if (table == 'D') {

                            type = ShortIndirect;
                            if (debug) printf("%c[%d]: %s\n", table, offset, name(type));

                        } else {
                            
                            type = ShortTable;
                            if (debug) printf("%c[%d]: %s\n", table, offset, name(type));

                            table++;
                            pos += len;
                            len = idx[i+2];
                        }
                        continue;
                        
                    case 3:

                        if (table == 'D') {

                            type = LongIndirect;
                            if (debug) printf("%c[%d]: %s\n", table, offset, name(type));

                        } else {
                            
                            type = LongTable;
                            if (debug) printf("%c[%d]: %s\n", table, offset, name(type));

                            table++;
                            pos += len;
                            len = idx[i+2];
                        }
                        continue;
                }
                break;
            }
            case ShortEarly:
            {
                physAddr = ptr + bitslice(pos + len, 32 - (pos + len));
                if (debug) printf("ShortEarly: %x -> %x\n", addr, physAddr);
                break;
            }
            case ShortPage:
            {
                physAddr = ptr + bitslice(pos + len, 32 - (pos + len));
                if (debug) printf("ShortPage: %x -> %x\n", addr, physAddr);
                break;
            }
            case ShortInvalid:
            {
                printf("ShortInvalid: %x -> Bus error\n", addr);
                throw BusErrorException();
            }
            case ShortIndirect:
            {
                u32 lword0 = readMMU(ptr);
                ptr = lword0 & 0xFFFFFFF0;
                physAddr = ptr + bitslice(pos + len, 32 - (pos + len));
                if (debug) printf("ShortIndirect: %x -> %x\n", addr, physAddr);
                break;
            }
            case LongTable:
            {
                u32 offset = bitslice(pos, len);
                u32 lword0 = readMMU(ptr + 8 * offset);
                u32 lword1 = readMMU(ptr + 8 * offset + 4);
                
                if (debug) printf("LongTable: %c[%d] = %x %x\n", table, offset, lword0, lword1);
                
                ptr = lword1 & 0xFFFFFFF0;
                if constexpr (write) wp |= lword0 & 0x4;
                
                switch (lword0 & 0x3) {
                        
                    case 0:

                        type = LongInvalid;
                        if (debug) printf("%c[%d]: %s\n", table, offset, name(type));
                        continue;
                        
                    case 1:

                        type = table == 'D' ? LongPage : LongEarly;
                        if (debug) printf("%c[%d]: %s\n", table, offset, name(type));
                        continue;
                        
                    case 2:
                        
                        if (table == 'D') {

                            type = LongIndirect;
                            if (debug) printf("%c[%d]: %s\n", table, offset, name(type));

                        } else {
                            
                            type = LongTable;
                            if (debug) printf("%c[%d]: %s\n", table, offset, name(type));

                            table++;
                            pos += len;
                            len = idx[i+2];
                        }
                        continue;
                        
                    case 3:

                        if (table == 'D') {

                            type = LongIndirect;
                            if (debug) printf("%c[%d]: %s\n", table, offset, name(type));

                        } else {
                            
                            type = LongTable;
                            if (debug) printf("%c[%d]: %s\n", table, offset, name(type));

                            table++;
                            pos += len;
                            len = idx[i+2];
                        }
                        continue;
                }
                break;
            }
            case LongEarly:
            {
                physAddr = ptr + bitslice(pos + len, 32 - (pos + len));
                if (debug) printf("LongEarly: %x -> %x\n", addr, physAddr);
                break;
            }
            case LongPage:
            {
                physAddr = ptr + bitslice(pos + len, 32 - (pos + len));
                if (debug) printf("LongPage: %x -> %x\n", addr, physAddr);
                break;
            }
            case LongInvalid:
            {
                printf("LongInvalid: %x -> Bus error\n", addr);
                throw BusErrorException();
            }
            case LongIndirect:
            {
                u32 lword1 = readMMU(ptr + 4);
                ptr = lword1 & 0xFFFFFFF0;
                physAddr = ptr + bitslice(pos + len, 32 - (pos + len));
                if (debug) printf("LongIndirect: %x -> %x\n", addr, physAddr);
                break;
            }
                
            default:
                assert(false);
        }
        
        // Check write protection status
        if constexpr (write) { if (wp) throw BusErrorException(); }
        
        return physAddr;
    }
    
    assert(false);
    
#if 0
    
        // O L D   C O D E
        
        
    
    u32 entry, entry2 = 0;
    u32 offset, shift;
        
    //
    // Search table A
    //
    
    switch (type) {

        case 0:     // Invalid descriptor type
            
            printf("A: Invalid descriptor\n");
            throw BusErrorException();
            
        case 1:     // Page descriptor (early termination)
            
            if (debug) printf("TODO (2): Page descriptor (early termination)\n");
            assert(0);
            return ptr + addr; // TODO: Do we need to cancel some bits?
            // break;

        case 2:     // Valid table with 'short' entries

            // Read short entry from table A
            offset = 4 * ((addr << is) >> (32 - abits));
            entry = readMMU(ptr + offset);
            if (debug) printf("Table A (%x): Short entry[%d] = %x\n", ptr, offset / 4, entry);
            ptr = entry & 0xFFFFFFF0;
            type = entry & 0x3;
            break;

        case 3:     // Valid table with 'long' entries

            // Read long entry from table A
            offset = 8 * ((addr << is) >> (32 - abits));
            entry = readMMU(ptr + offset);
            entry2 = readMMU(ptr + offset + 4);
            if ((entry2 & 0x3) == 0) debug = true;
            if (debug) printf("Table A (%x): Long entry[%d] = %x %x\n", ptr, offset / 8, entry, entry2);
            ptr = entry & 0xFFFFFFF0;
            type = entry2 & 0x3;
            break;
    }
    
    //
    // Search table B
    //

    switch (type) {

        case 0:     // Invalid descriptor type
            
            printf("B: Invalid descriptor\n");
            throw BusErrorException();

        case 1:     // Page descriptor (early termination)
        {
            shift = is + abits;
            u32 physAddr = ((addr << shift) >> shift) + ptr;
            if (debug) printf("Physical address = %x\n", physAddr);
            // if (addr != physAddr) printf("Mapping %x -> %x\n", addr, physAddr);
            return ((addr << shift) >> shift) + ptr;
        }
        case 2:     // Valid table with 'short' entries

            // Read short entry from table B
            offset = 4 * ((addr << (is + abits)) >> (32 - bbits));
            entry = readMMU(ptr + offset);
            if (debug) printf("Table B: Short entry[%d] = %x\n", offset / 4, entry);
            ptr = entry & 0xFFFFFFF0;
            type = entry & 0x3;
            break;
                        
        case 3:     // Valid table with 'long' entries

            // Read long entry from table B
            offset = 8 * ((addr << (is + abits)) >> (32 - bbits));
            entry = readMMU(ptr + offset);
            entry2 = readMMU(ptr + offset + 4);
            if (debug) printf("Table B: Long entry[%d] = %x %x\n", offset / 8, entry, entry2);
            ptr = entry & 0xFFFFFFF0;
            type = entry2 & 0x3;
            break;
    }
 
    //
    // Search table C
    //

    switch (type) {

        case 0:     // Invalid descriptor type
            
            printf("C: Invalid descriptor\n");
            throw BusErrorException();

        case 1: // Page descriptor (early termination)
            
            shift = is + abits + bbits;
            if (debug) printf("Physical address = %x\n", ((addr << shift) >> shift) + ptr);
            return ((addr << shift) >> shift) + ptr;

        case 2:     // Valid table with 'short' entries

            // Read short entry from table C
            offset = 4 * ((addr << (is + abits + bbits)) >> (32 - cbits));
            entry = readMMU(ptr + offset);
            if (debug) printf("Table C: Short entry[%d] = %x\n", offset / 4, entry);
            ptr = entry & 0xFFFFFFF0;
            type = entry & 0x3;
            break;
                        
        case 3:     // Valid table with 'long' entries

            // Read long entry from table C
            offset = 8 * ((addr << (is + abits + bbits)) >> (32 - cbits));
            entry = readMMU(ptr + offset);
            entry2 = readMMU(ptr + offset + 4);
            if (debug) printf("Table C: Long entry[%d] = %x %x\n", offset / 8, entry, entry2);
            ptr = entry & 0xFFFFFFF0;
            type = entry2 & 0x3;
            break;
    }
    
    switch (type)
    {
        case 0:     // Invalid descriptor type
            
            printf("D: Invalid descriptor\n");
            throw BusErrorException();

        case 1:     // Page descriptor (early termination)
        {
            shift = is + abits + bbits + cbits;
            u32 physAddr = ((addr << shift) >> shift) + ptr;            
            if (debug) printf("Physical address = %x\n", physAddr);
            return physAddr;
        }
        case 2:     // Valid table with 'short' entries

            if (debug) printf("TODO (6): Yet unhandeled table C case.\n");
            assert(0);
            return addr;
            
        case 3:     // Valid table with 'long' entries

            if (debug) printf("TODO (7): Yet unhandeled table C case.\n");
            assert(0);
            return addr;
    }
    
    assert(false);
    return addr;

#endif
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
