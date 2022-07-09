// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

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
                printf("Entering loop mode (IRD: %x IRC: %x)\n", queue.ird, queue.irc);
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

    // printf("loopDbcc(%x) IRD = %x IRC = %x\n", opcode, queue.ird, queue.irc);

    sync(2);
    if (!cond<I>()) {

        int dn = _____________xxx(opcode);
        u32 newpc = U32_ADD(reg.pc, -4);

        bool takeBranch = readD<Word>(dn) != 0;
        // printf("takeBranch = %d\n", takeBranch);

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
            // fullPrefetch<POLLIPL>();
            queue.ird = queue.irc;
            queue.irc = opcode;
            // printf("Repeating loop (%d): IRD = %x IRC = %x\n", readD<Word>(dn), opcode, queue.ird, queue.irc);
            return;
        } else {
            // printf("Exiting loop PC = %x\n", reg.pc);
            (void)readMS <MEM_PROG, Word> (reg.pc + 2);
        }
    } else {
        sync(2);
    }

    // Fall through to next instruction
    reg.pc += 2;
    fullPrefetch<POLLIPL>();
    flags &= ~CPU_IS_LOOPING;
    printf("Exiting loop mode (IRD: %x IRC: %x)\n", queue.ird, queue.irc);
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
