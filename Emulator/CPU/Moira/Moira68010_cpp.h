// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

template<Instr I, Mode M, Size S> void
Moira::execBkpt(u16 opcode)
{
    EXEC_DEBUG

    // TODO
    printf("BKPT: Implementation missing\n");
    breakpointReached(reg.pc0);

    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execRtd(u16 opcode)
{
    EXEC_DEBUG

    // TODO
    printf("RTD: Implementation missing\n");
    breakpointReached(reg.pc0);

    fullPrefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execMovecRcRx(u16 opcode)
{
    EXEC_DEBUG

    auto arg = readI<Word>();
    int dst = xxxx____________(arg);
    int src = arg & 0x0FFF;

    prefetch<POLLIPL>();

    switch(src) {

        case 0x000: reg.r[dst] = reg.sfc & 0b111; break;
        case 0x001: reg.r[dst] = reg.dfc & 0b111; break;
        case 0x800: reg.r[dst] = reg.usp; break;
        case 0x801: reg.r[dst] = reg.vbr; break;

        default:
            execIllegal(opcode);
    }
}

template<Instr I, Mode M, Size S> void
Moira::execMovecRxRc(u16 opcode)
{
    EXEC_DEBUG

    auto arg = readI<Word>();
    int src = xxxx____________(arg);
    int dst = arg & 0x0FFF;

    prefetch<POLLIPL>();

    switch(dst) {

        case 0x000: reg.sfc = reg.r[src] & 0b111; break;
        case 0x001: reg.dfc = reg.r[src] & 0b111; break;
        case 0x800: reg.usp = reg.r[src]; break;
        case 0x801: reg.vbr = reg.r[src]; break;

        default:
            execIllegal(opcode);
    }
}

template<Instr I, Mode M, Size S> void
Moira::execMovesRgEa(u16 opcode)
{
    EXEC_DEBUG

    // TODO
    printf("MovesRgEa: Implementation missing\n");
    breakpointReached(reg.pc0);

    fullPrefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execDbcc68010(u16 opcode)
{
    EXEC_DEBUG

    // printf("execDbcc68010(%x)\n", opcode);
    // softstopReached(reg.pc0);

    sync(2);
    if (!cond<I>()) {

        int dn = _____________xxx(opcode);
        i16 disp = (i16)queue.irc;

        u32 newpc = U32_ADD(reg.pc, disp);

        bool takeBranch = readD<Word>(dn) != 0;

        // Check for address error
        if (misaligned<S>(newpc)) {
            execAddressError(makeFrame(newpc, newpc + 2));
            return;
        }

        // Decrement loop counter
        writeD<Word>(dn, U32_SUB(readD<Word>(dn), 1));

        // Branch
        if (takeBranch) {

            reg.pc = newpc;
            fullPrefetch<POLLIPL>();

            if (loop[queue.ird] && disp == -4) {

                // Enter loop mode
                flags |= CPU_IS_LOOPING;
                queue.irc = opcode;
                // printf("Entering loop mode (IRD: %x IRC: %x)\n", queue.ird, queue.irc);
            }
            return;

        } else {

            (void)readMS <MEM_PROG, Word> (reg.pc + 2);
        }

    } else {

        sync(2);
    }

    // Fall through to next instruction
    reg.pc += 2;
    fullPrefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execDbccLoop(u16 opcode)
{
    EXEC_DEBUG

    sync(2);
    if (!cond<I>()) {

        int dn = _____________xxx(opcode);
        u32 newpc = U32_ADD(reg.pc, -4);

        bool takeBranch = readD<Word>(dn) != 0;

        // Check for address error
        if (misaligned<S>(newpc)) {
            execAddressError(makeFrame(newpc, newpc + 2));
            return;
        }

        // Decrement loop counter
        writeD<Word>(dn, U32_SUB(readD<Word>(dn), 1));

        // Branch
        if (takeBranch) {
            reg.pc = newpc;
            reg.pc0 = reg.pc;
            queue.ird = queue.irc;
            queue.irc = opcode;
            return;
        } else {
            (void)readMS <MEM_PROG, Word> (reg.pc + 2);
        }
    } else {
        sync(2);
    }

    // Fall through to next instruction
    reg.pc += 2;
    fullPrefetch<POLLIPL>();
    flags &= ~CPU_IS_LOOPING;
    // printf("Exiting loop mode (IRD: %x IRC: %x)\n", queue.ird, queue.irc);
}

template<Instr I, Mode M, Size S> void
Moira::execMoveFromSrRg68010(u16 opcode)
{
    EXEC_DEBUG

    SUPERVISOR_MODE_ONLY
    execMoveFromSrRg <I,M,S> (opcode);
}

template<Instr I, Mode M, Size S> void
Moira::execMoveFromSrEa68010(u16 opcode)
{
    EXEC_DEBUG

    SUPERVISOR_MODE_ONLY
    execMoveFromSrEa <I,M,S> (opcode);
}
