// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

// Assembles an instruction handler name
#define EXEC_HANDLER(func,C,I,M,S) &Moira::exec##func<C,I,M,S>
#define DASM_HANDLER(func,I,M,S) &Moira::dasm##func<I,M,S>

// Registers an instruction handler
#if ENABLE_DASM == true
#define REGISTER_DASM(id,name,I,M,S) if (regDasm) dasm[id] = DASM_HANDLER(name,I,M,S);
#else
#define REGISTER_DASM(id,name,I,M,S) { }
#endif

#if BUILD_INSTR_INFO_TABLE == true
#define REGISTER_INFO(id,name,I,M,S) info[id] = InstrInfo {I,M,S};
#else
#define REGISTER_INFO(id,name,I,M,S) { }
#endif

#define CIMS(id,name,I,M,S) { \
exec[id] = EXEC_HANDLER(name,C,I,M,S); \
REGISTER_DASM(id,name,I,M,S) \
REGISTER_INFO(id,name,I,M,S) \
}

#define CIMSloop(id,name,I,M,S) { \
assert(loop[id] == nullptr); \
loop[id] = EXEC_HANDLER(name,Core::C68010,I##_LOOP,M,S); \
}

// Registers an instruction in one of the standard instruction formats:
//
//     ____ ____ ____ ____
//     ____ ____ ____ _XXX    XXX : Operand parameter (Register number etc.)
//     ____ ____ ____ XXXX      S : Size information  (Word or Long)
//     ____ ____ XXXX XXXX     SS : Size information  (Byte or Word or Long)
//     ____ XXX_ ____ _XXX
//     ____ XXX_ XXXX XXXX
//     ____ ____ __MM MXXX
//     ____ XXX_ __MM MXXX
//     ____ XXX_ SS__ _XXX
//     ____ ____ SSMM MXXX
//     ____ XXX_ SSMM MXXX
//     ____ XXXS __MM MXXX
//     __SS ____ __MM MXXX
//     __SS XXX_ __MM MXXX

#define ________________(op,I,M,S,f,func) { func(op, f, I, M, S); }

#define _____________XXX(op,I,M,S,f,func) { \
for (int j = 0; j < 8; j++) func((op) | j, f, I, M, S); }

#define ____________XXXX(op,I,M,S,f,func) { \
for (int j = 0; j < 16; j++) func((op) | j, f, I, M, S); }

#define ___________XXXXX(op,I,M,S,f,func) { \
for (int j = 0; j < 32; j++) func((op) | j, f, I, M, S); }

#define __________XXXXXX(op,I,M,S,f,func) { \
for (int j = 0; j < 64; j++) func((op) | j, f, I, M, S); }

#define ________XXXXXXXX(op,I,M,S,f,func) { \
for (int j = 0; j < 256; j++) func((op) | j, f, I, M, S); }

#define ____XXXXXXXXXXXX(op,I,M,S,f,func) { \
for (int j = 0; j < 4096; j++) func((op) | j, f, I, M, S); }

#define XXXXXXXXXXXXXXXX(I,M,S,f,func) { \
for (int j = 0; j < 65536; j++) func(j, f, I, M, S); }

#define __________XXX___(op,I,M,S,f,func) { \
for (int j = 0; j < 8; j++) func((op) | j << 3, f, I, M, S); }

#define ____XXX______XXX(op,I,M,S,f,func) { \
for (int i = 0; i < 8; i++) _____________XXX((op) | i << 9, I, M, S, f, func); }

#define ____XXX___XXX___(op,I,M,S,f,func) { \
for (int i = 0; i < 8; i++) __________XXX___((op) | i << 9, I, M, S, f, func); }

#define ____XXX___XXXXXX(op,I,M,S,f,func) { \
for (int i = 0; i < 8; i++) __________XXXXXX((op) | i << 9, I, M, S, f, func); }

#define ____XXX_XXXXXXXX(op,I,M,S,f,func) { \
for (int i = 0; i < 8; i++) ________XXXXXXXX((op) | i << 9, I, M, S, f, func); }

#define __________MMMXXX(op,I,m,S,f,func) { \
for (int j = 0; j < 8; j++) { \
if ((m) & 0b100000000000) func((op) | 0 << 3 | j, f, I,  (Mode)0, S); \
if ((m) & 0b010000000000) func((op) | 1 << 3 | j, f, I,  (Mode)1, S); \
if ((m) & 0b001000000000) func((op) | 2 << 3 | j, f, I,  (Mode)2, S); \
if ((m) & 0b000100000000) func((op) | 3 << 3 | j, f, I,  (Mode)3, S); \
if ((m) & 0b000010000000) func((op) | 4 << 3 | j, f, I,  (Mode)4, S); \
if ((m) & 0b000001000000) func((op) | 5 << 3 | j, f, I,  (Mode)5, S); \
if ((m) & 0b000000100000) func((op) | 6 << 3 | j, f, I,  (Mode)6, S); \
} \
if ((m) & 0b000000010000) func((op) | 7 << 3 | 0, f, I,  (Mode)7, S); \
if ((m) & 0b000000001000) func((op) | 7 << 3 | 1, f, I,  (Mode)8, S); \
if ((m) & 0b000000000100) func((op) | 7 << 3 | 2, f, I,  (Mode)9, S); \
if ((m) & 0b000000000010) func((op) | 7 << 3 | 3, f, I, (Mode)10, S); \
if ((m) & 0b000000000001) func((op) | 7 << 3 | 4, f, I, (Mode)11, S); }

#define ____XXX___MMMXXX(op,I,m,S,f,func) { \
for (int i = 0; i < 8; i++) __________MMMXXX((op) | i << 9, I, m, S, f, func) }

#define ____XXX_SS___XXX(op,I,M,s,f,func) { \
if ((s) & 0b100) ____XXX______XXX((op) | 2 << 6, I, M, Long, f, func); \
if ((s) & 0b010) ____XXX______XXX((op) | 1 << 6, I, M, Word, f, func); \
if ((s) & 0b001) ____XXX______XXX((op) | 0 << 6, I, M, Byte, f, func); }

#define ________SSMMMXXX(op,I,m,s,f,func) { \
if ((s) & 0b100) __________MMMXXX((op) | 2 << 6, I, m, Long, f, func); \
if ((s) & 0b010) __________MMMXXX((op) | 1 << 6, I, m, Word, f, func); \
if ((s) & 0b001) __________MMMXXX((op) | 0 << 6, I, m, Byte, f, func); }

#define ____XXX_SSMMMXXX(op,I,m,s,f,func) { \
if ((s) & 0b100) ____XXX___MMMXXX((op) | 2 << 6, I, m, Long, f, func); \
if ((s) & 0b010) ____XXX___MMMXXX((op) | 1 << 6, I, m, Word, f, func); \
if ((s) & 0b001) ____XXX___MMMXXX((op) | 0 << 6, I, m, Byte, f, func); }

#define ____XXXS__MMMXXX(op,I,m,s,f,func) { \
if ((s) & 0b100) ____XXX___MMMXXX((op) | 1 << 8, I, m, Long, f, func); \
if ((s) & 0b010) ____XXX___MMMXXX((op) | 0 << 8, I, m, Word, f, func); \
if ((s) & 0b001) assert(false); }

#define _____SS___MMMXXX(op,I,m,s,f,func) { \
if ((s) & 0b100) __________MMMXXX((op) | 2 << 9, I, m, Long, f, func); \
if ((s) & 0b010) __________MMMXXX((op) | 3 << 9, I, m, Word, f, func); \
if ((s) & 0b001) __________MMMXXX((op) | 1 << 9, I, m, Byte, f, func); }

#define __SS______MMMXXX(op,I,m,s,f,func) { \
if ((s) & 0b100) __________MMMXXX((op) | 2 << 12, I, m, Long, f, func); \
if ((s) & 0b010) __________MMMXXX((op) | 3 << 12, I, m, Word, f, func); \
if ((s) & 0b001) __________MMMXXX((op) | 1 << 12, I, m, Byte, f, func); }

#define __SSXXX___MMMXXX(op,I,m,s,f,func) { \
if ((s) & 0b100) ____XXX___MMMXXX((op) | 2 << 12, I, m, Long, f, func); \
if ((s) & 0b010) ____XXX___MMMXXX((op) | 3 << 12, I, m, Word, f, func); \
if ((s) & 0b001) ____XXX___MMMXXX((op) | 1 << 12, I, m, Byte, f, func); }


static constexpr u16
parse(const char *s, int sum = 0)
{
    return
    *s == ' ' ? parse(s + 1, sum) :
    *s == '-' ? parse(s + 1, sum << 1) :
    *s == '0' ? parse(s + 1, sum << 1) :
    *s == '1' ? parse(s + 1, (sum << 1) + 1) : (u16)sum;
}

void
Moira::createJumpTable(Model cpuModel, Model dasmModel)
{
    auto core = [&](Model model) {
        
        switch (model) {
                
            case Model::M68000: return Core::C68000;
            case Model::M68010: return Core::C68010;
                
            default:
                return Core::C68020;
        }
    };

    Core cpuCore = core(cpuModel);
    Core dasmCore = core(dasmModel);

    // Register handlers based on the dasm model
    if (dasmCore == Core::C68000) createJumpTable<Core::C68000>(dasmModel, true);
    if (dasmCore == Core::C68010) createJumpTable<Core::C68010>(dasmModel, true);
    if (dasmCore == Core::C68020) createJumpTable<Core::C68020>(dasmModel, true);

    // If both models differ, overwrite the exec handlers
    if (cpuModel != dasmModel) {

        if (cpuCore == Core::C68000) createJumpTable<Core::C68000>(cpuModel, false);
        if (cpuCore == Core::C68010) createJumpTable<Core::C68010>(cpuModel, false);
        if (cpuCore == Core::C68020) createJumpTable<Core::C68020>(cpuModel, false);
    }
}

template <Core C> void
Moira::createJumpTable(Model model, bool regDasm)
{
    u16 opcode;

    //
    // Start with clean tables
    //

    XXXXXXXXXXXXXXXX(ILLEGAL, Mode::MODE_IP, (Size)0, Illegal, CIMS)

    for (int i = 0; i < 0x10000; i++) {
        loop[i] = nullptr;
    }


    // Unimplemented instructions
    //
    //       Format: 1010 ---- ---- ---- (Line A instructions)
    //               1111 ---- ---- ---- (Line F instructions)

    opcode = parse("1010 ---- ---- ----");
    ____XXXXXXXXXXXX(opcode, LINE_A, Mode::MODE_IP, (Size)0, LineA, CIMS)

    opcode = parse("1111 ---- ---- ----");
    ____XXXXXXXXXXXX(opcode, LINE_F, Mode::MODE_IP, (Size)0, LineF, CIMS)


    // ABCD
    //
    //       Syntax: (1) ABCD Dx,Dy
    //               (2) ABCD -(Ax),-(Ay)
    //         Size: Byte

    // Dx,Dy
    opcode = parse("1100 ---1 0000 0---");
    ____XXX______XXX(opcode, ABCD, Mode::MODE_DN, Byte, AbcdRg, CIMS)

    // -(Ax),-(Ay)
    opcode = parse("1100 ---1 0000 1---");
    ____XXX______XXX(opcode, ABCD, Mode::MODE_PD, Byte, AbcdEa, CIMS)
    ____XXX______XXX(opcode, ABCD, Mode::MODE_PD, Byte, AbcdEa, CIMSloop)


    // ADD
    //
    //       Syntax: (1) ADD <ea>,Dy
    //               (2) ADD Dx,<ea>
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)  X   X   X   X   X   X   X   X   X   X

    opcode = parse("1101 ---0 ---- ----");
    ____XXX_SSMMMXXX(opcode, ADD, 0b101111111111, Byte,        AddEaRg, CIMS)
    ____XXX_SSMMMXXX(opcode, ADD, 0b111111111111, Word | Long, AddEaRg, CIMS)
    ____XXX_SSMMMXXX(opcode, ADD, 0b001110000000, Byte,        AddEaRg, CIMSloop)
    ____XXX_SSMMMXXX(opcode, ADD, 0b001110000000, Word | Long, AddEaRg, CIMSloop)


    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1101 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, ADD, 0b001111111000, Byte | Word | Long, AddRgEa, CIMS)
    ____XXX_SSMMMXXX(opcode, ADD, 0b001110000000, Byte | Word | Long, AddRgEa, CIMSloop)


    // ADDA
    //
    //       Syntax: ADDA <ea>,Ay
    //         Size: Word, Longword
    //
    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X   X   X   X   X   X   X   X   X   X   X   X

    opcode = parse("1101 ---- 11-- ----");
    ____XXXS__MMMXXX(opcode, ADDA, 0b111111111111, Word | Long, Adda, CIMS)
    ____XXXS__MMMXXX(opcode, ADDA, 0b001110000000, Word | Long, Adda, CIMSloop)


    // ADDI
    //
    //       Syntax: ADDI #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 0110 ---- ----");
    ________SSMMMXXX(opcode, ADDI, 0b100000000000, Byte | Word | Long, AddiRg, CIMS)
    ________SSMMMXXX(opcode, ADDI, 0b001111111000, Byte | Word | Long, AddiEa, CIMS)


    // ADDQ
    //
    //       Syntax: ADDQ #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)  X   X   X   X   X   X   X

    opcode = parse("0101 ---0 ---- ----");
    ____XXX_SSMMMXXX(opcode, ADDQ, 0b100000000000, Byte | Word | Long, AddqDn, CIMS)
    ____XXX_SSMMMXXX(opcode, ADDQ, 0b010000000000, Word | Long,        AddqAn, CIMS)
    ____XXX_SSMMMXXX(opcode, ADDQ, 0b001111111000, Byte | Word | Long, AddqEa, CIMS)


    // ADDX
    //
    //       Syntax: (1) ADDX Dx,Dy
    //               (2) ADDX -(Ax),-(Ay)
    //         Size: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1101 ---1 --00 0---");
    ____XXX_SS___XXX(opcode, ADDX, Mode::MODE_DN, Byte | Word | Long, AddxRg, CIMS)

    // -(Ax),-(Ay)
    opcode = parse("1101 ---1 --00 1---");
    ____XXX_SS___XXX(opcode, ADDX, Mode::MODE_PD, Byte | Word | Long, AddxEa, CIMS)
    ____XXX_SS___XXX(opcode, ADDX, Mode::MODE_PD, Byte | Word | Long, AddxEa, CIMSloop)


    // AND
    //
    //       Syntax: (1) AND <ea>,Dy
    //               (2) AND Dx,<ea>
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("1100 ---0 ---- ----");
    ____XXX_SSMMMXXX(opcode, AND, 0b101111111111, Byte | Word | Long, AndEaRg, CIMS)
    ____XXX_SSMMMXXX(opcode, AND, 0b001110000000, Byte | Word | Long, AndEaRg, CIMSloop)


    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1100 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, AND, 0b001111111000, Byte | Word | Long, AndRgEa, CIMS)
    ____XXX_SSMMMXXX(opcode, AND, 0b001110000000, Byte | Word | Long, AndRgEa, CIMSloop)


    // ANDI
    //
    //       Syntax: ANDI #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 0010 ---- ----");
    ________SSMMMXXX(opcode, ANDI, 0b100000000000, Byte | Word | Long, AndiRg, CIMS)
    ________SSMMMXXX(opcode, ANDI, 0b001111111000, Byte | Word | Long, AndiEa, CIMS)


    // ANDI to CCR
    //
    //       Syntax: ANDI #<data>,CCR
    //         Size: Byte
    //

    opcode = parse("0000 0010 0011 1100");
    ________________(opcode, ANDICCR, Mode::MODE_IM, Byte, Andiccr, CIMS)


    // ANDI to SR
    //
    //       Syntax: ANDI #<data>,SR
    //         Size: Byte
    //

    opcode = parse("0000 0010 0111 1100");
    ________________(opcode, ANDISR, Mode::MODE_IM, Word, Andisr, CIMS)


    // ASL, ASR
    //
    //       Syntax: (1) ASx Dx,Dy
    //               (2) ASx #<data>,Dy
    //               (3) ASx <ea>
    //         Size: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1110 ---1 --10 0---");
    ____XXX_SS___XXX(opcode, ASL, Mode::MODE_DN,  Byte | Word | Long, ShiftRg, CIMS)

    opcode = parse("1110 ---0 --10 0---");
    ____XXX_SS___XXX(opcode, ASR, Mode::MODE_DN,  Byte | Word | Long, ShiftRg, CIMS)

    // #<data>,Dy
    opcode = parse("1110 ---1 --00 0---");
    ____XXX_SS___XXX(opcode, ASL, Mode::MODE_IM, Byte | Word | Long, ShiftIm, CIMS)

    opcode = parse("1110 ---0 --00 0---");
    ____XXX_SS___XXX(opcode, ASR, Mode::MODE_IM, Byte | Word | Long, ShiftIm, CIMS)

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1110 0001 11-- ----");
    __________MMMXXX(opcode, ASL, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, ASL, 0b001110000000, Word, ShiftEa, CIMSloop)

    opcode = parse("1110 0000 11-- ----");
    __________MMMXXX(opcode, ASR, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, ASR, 0b001110000000, Word, ShiftEa, CIMSloop)


    // Bcc
    //
    //       Syntax: Bcc <label>
    //         Size: Word

    // Dn,<label>

    opcode = parse("0110 ---- ---- ----");
    ________________(opcode | 0x000, BRA, Mode::MODE_IP, Word, Bra, CIMS)
    ________________(opcode | 0x200, BHI, Mode::MODE_IP, Word, Bcc, CIMS)
    ________________(opcode | 0x300, BLS, Mode::MODE_IP, Word, Bcc, CIMS)
    ________________(opcode | 0x400, BCC, Mode::MODE_IP, Word, Bcc, CIMS)
    ________________(opcode | 0x500, BCS, Mode::MODE_IP, Word, Bcc, CIMS)
    ________________(opcode | 0x600, BNE, Mode::MODE_IP, Word, Bcc, CIMS)
    ________________(opcode | 0x700, BEQ, Mode::MODE_IP, Word, Bcc, CIMS)
    ________________(opcode | 0x800, BVC, Mode::MODE_IP, Word, Bcc, CIMS)
    ________________(opcode | 0x900, BVS, Mode::MODE_IP, Word, Bcc, CIMS)
    ________________(opcode | 0xA00, BPL, Mode::MODE_IP, Word, Bcc, CIMS)
    ________________(opcode | 0xB00, BMI, Mode::MODE_IP, Word, Bcc, CIMS)
    ________________(opcode | 0xC00, BGE, Mode::MODE_IP, Word, Bcc, CIMS)
    ________________(opcode | 0xD00, BLT, Mode::MODE_IP, Word, Bcc, CIMS)
    ________________(opcode | 0xE00, BGT, Mode::MODE_IP, Word, Bcc, CIMS)
    ________________(opcode | 0xF00, BLE, Mode::MODE_IP, Word, Bcc, CIMS)

    for (int i = 1; i <= 0xFF; i++) {

        ________________(opcode | 0x000 | i, BRA, Mode::MODE_IP, Byte, Bra, CIMS)
        ________________(opcode | 0x200 | i, BHI, Mode::MODE_IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x300 | i, BLS, Mode::MODE_IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x400 | i, BCC, Mode::MODE_IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x500 | i, BCS, Mode::MODE_IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x600 | i, BNE, Mode::MODE_IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x700 | i, BEQ, Mode::MODE_IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x800 | i, BVC, Mode::MODE_IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x900 | i, BVS, Mode::MODE_IP, Byte, Bcc, CIMS)
        ________________(opcode | 0xA00 | i, BPL, Mode::MODE_IP, Byte, Bcc, CIMS)
        ________________(opcode | 0xB00 | i, BMI, Mode::MODE_IP, Byte, Bcc, CIMS)
        ________________(opcode | 0xC00 | i, BGE, Mode::MODE_IP, Byte, Bcc, CIMS)
        ________________(opcode | 0xD00 | i, BLT, Mode::MODE_IP, Byte, Bcc, CIMS)
        ________________(opcode | 0xE00 | i, BGT, Mode::MODE_IP, Byte, Bcc, CIMS)
        ________________(opcode | 0xF00 | i, BLE, Mode::MODE_IP, Byte, Bcc, CIMS)
    }

    if constexpr (C >= Core::C68020) {

        ________________(opcode | 0x0FF, BRA, Mode::MODE_IP, Long, Bra, CIMS)
        ________________(opcode | 0x2FF, BHI, Mode::MODE_IP, Long, Bcc, CIMS)
        ________________(opcode | 0x3FF, BLS, Mode::MODE_IP, Long, Bcc, CIMS)
        ________________(opcode | 0x4FF, BCC, Mode::MODE_IP, Long, Bcc, CIMS)
        ________________(opcode | 0x5FF, BCS, Mode::MODE_IP, Long, Bcc, CIMS)
        ________________(opcode | 0x6FF, BNE, Mode::MODE_IP, Long, Bcc, CIMS)
        ________________(opcode | 0x7FF, BEQ, Mode::MODE_IP, Long, Bcc, CIMS)
        ________________(opcode | 0x8FF, BVC, Mode::MODE_IP, Long, Bcc, CIMS)
        ________________(opcode | 0x9FF, BVS, Mode::MODE_IP, Long, Bcc, CIMS)
        ________________(opcode | 0xAFF, BPL, Mode::MODE_IP, Long, Bcc, CIMS)
        ________________(opcode | 0xBFF, BMI, Mode::MODE_IP, Long, Bcc, CIMS)
        ________________(opcode | 0xCFF, BGE, Mode::MODE_IP, Long, Bcc, CIMS)
        ________________(opcode | 0xDFF, BLT, Mode::MODE_IP, Long, Bcc, CIMS)
        ________________(opcode | 0xEFF, BGT, Mode::MODE_IP, Long, Bcc, CIMS)
        ________________(opcode | 0xFFF, BLE, Mode::MODE_IP, Long, Bcc, CIMS)
    }


    // BFCHG, BFCLR, BFINS, BFSET
    //
    //       Syntax: BFxxx <ea> {offset:width}
    //         Size: Unsized

    //               -------------------------------------------------
    // <ea> {:}      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X           X   X   X   X

    if constexpr (C >= Core::C68020) {

        opcode = parse("1110 1010 11-- ----");
        __________MMMXXX(opcode, BFCHG, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, BFCHG, 0b001001111000, Long, BitFieldEa, CIMS)

        opcode = parse("1110 1100 11-- ----");
        __________MMMXXX(opcode, BFCLR, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, BFCLR, 0b001001111000, Long, BitFieldEa, CIMS)

        opcode = parse("1110 1111 11-- ----");
        __________MMMXXX(opcode, BFINS, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, BFINS, 0b001001111000, Long, BitFieldEa, CIMS)

        opcode = parse("1110 1110 11-- ----");
        __________MMMXXX(opcode, BFSET, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, BFSET, 0b001001111000, Long, BitFieldEa, CIMS)
    }


    // BFEXTS, BFEXTU, BFFFO, BFTST
    //
    //       Syntax: BFxxx <ea> {offset:width}
    //         Size: Unsized

    //               -------------------------------------------------
    // <ea> {:}      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X           X   X   X   X   X   X   X

    if constexpr (C >= Core::C68020) {

        opcode = parse("1110 1011 11-- ----");
        __________MMMXXX(opcode, BFEXTS, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, BFEXTS, 0b001001111110, Long, BitFieldEa, CIMS)

        opcode = parse("1110 1001 11-- ----");
        __________MMMXXX(opcode, BFEXTU, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, BFEXTU, 0b001001111110, Long, BitFieldEa, CIMS)

        opcode = parse("1110 1101 11-- ----");
        __________MMMXXX(opcode, BFFFO, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, BFFFO, 0b001001111110, Long, BitFieldEa, CIMS)

        opcode = parse("1110 1000 11-- ----");
        __________MMMXXX(opcode, BFTST, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, BFTST, 0b001001111110, Long, BitFieldEa, CIMS)
    }


    // BCHG, BCLR
    //
    //       Syntax: (1) BCxx Dn,<ea>
    //               (2) BCxx #<data>,<ea>
    //         Size: Byte, Longword

    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 ---1 01-- ----");
    ____XXX___MMMXXX(opcode, BCHG, 0b100000000000, Byte, BitDxDy, CIMS)
    ____XXX___MMMXXX(opcode, BCHG, 0b001111111000, Byte, BitDxEa, CIMS)

    opcode = parse("0000 ---1 10-- ----");
    ____XXX___MMMXXX(opcode, BCLR, 0b100000000000, Byte, BitDxDy, CIMS)
    ____XXX___MMMXXX(opcode, BCLR, 0b001111111000, Byte, BitDxEa, CIMS)

    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 1000 01-- ----");
    __________MMMXXX(opcode, BCHG, 0b100000000000, Byte, BitImDy, CIMS)
    __________MMMXXX(opcode, BCHG, 0b001111111000, Byte, BitImEa, CIMS)

    opcode = parse("0000 1000 10-- ----");
    __________MMMXXX(opcode, BCLR, 0b100000000000, Byte, BitImDy, CIMS)
    __________MMMXXX(opcode, BCLR, 0b001111111000, Byte, BitImEa, CIMS)


    // BKPT (68010+)
    //
    //       Syntax: BKPT #<vector>
    //        Sizes: Unsized

    if constexpr (C >= Core::C68010) {

        opcode = parse("0100 1000 0100 1---");
        _____________XXX(opcode, BKPT, Mode::MODE_IP, Long, Bkpt, CIMS)
    }


    // BSET
    //
    //       Syntax: (1) BSET Dx,<ea>
    //               (2) BSET #<data>,<ea>
    //         Size: Byte, Longword

    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 ---1 11-- ----");
    ____XXX___MMMXXX(opcode, BSET, 0b100000000000, Byte, BitDxDy, CIMS)
    ____XXX___MMMXXX(opcode, BSET, 0b001111111000, Byte, BitDxEa, CIMS)

    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 1000 11-- ----");
    __________MMMXXX(opcode, BSET, 0b100000000000, Byte, BitImDy, CIMS)
    __________MMMXXX(opcode, BSET, 0b001111111000, Byte, BitImEa, CIMS)


    // BSR
    //
    //       Syntax: (1) BSR <label>
    //         Size: Byte, Word

    opcode = parse("0110 0001 ---- ----");
    ________________(opcode, BSR, Mode::MODE_IP, Word, Bsr, CIMS)

    for (int i = 1; i <= 0xFF; i++) {
        ________________(opcode | i, BSR, Mode::MODE_IP, Byte, Bsr, CIMS)
    }

    if constexpr (C >= Core::C68020) {
        ________________(opcode | 0xFF, BSR, Mode::MODE_IP, Long, Bsr, CIMS)
    }


    // BTST
    //
    //       Syntax: (1) BTST Dx,<ea>
    //               (2) BTST #<data>,<ea>
    //         Size: Byte, Longword

    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X?

    opcode = parse("0000 ---1 00-- ----");
    ____XXX___MMMXXX(opcode, BTST, 0b100000000000, Byte, BitDxDy, CIMS)
    ____XXX___MMMXXX(opcode, BTST, 0b001111111111, Byte, BitDxEa, CIMS)

    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X

    opcode = parse("0000 1000 00-- ----");
    __________MMMXXX(opcode, BTST, 0b100000000000, Byte, BitImDy, CIMS)
    __________MMMXXX(opcode, BTST, 0b001111111110, Byte, BitImEa, CIMS)


    // CALLM
    //
    //       Syntax: CALLM #<data>,<ea>
    //         Size: Unsized

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X           X   X   X   X   X   X

    if constexpr (C >= Core::C68020) {

        if (model == Model::M68EC020 || model == Model::M68020) {

            opcode = parse("0000 0110 11-- ----");
            __________MMMXXX(opcode, CALLM, 0b001001111110, Long, Callm, CIMS)
        }
    }


    // CAS, CAS2
    //
    //       Syntax: CAS Dc,Du,<ea>
    //               CAS2 Dc1:Dc2,Du1:Du2,(Rn1):(Rn2)
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X   X   X

    if constexpr (C >= Core::C68020) {

        // CAS
        opcode = parse("0000 1010 11-- ----");
        __________MMMXXX(opcode, CAS, 0b001111111000, Byte, Cas, CIMS)
        opcode = parse("0000 1100 11-- ----");
        __________MMMXXX(opcode, CAS, 0b001111111000, Word, Cas, CIMS)
        opcode = parse("0000 1110 11-- ----");
        __________MMMXXX(opcode, CAS, 0b001111111000, Long, Cas, CIMS)

        // CAS2
        opcode = parse("0000 1100 1111 1100");
        ________________(opcode, CAS2, Mode::MODE_IM, Word, Cas2, CIMS)

        opcode = parse("0000 1110 1111 1100");
        ________________(opcode, CAS2, Mode::MODE_IM, Long, Cas2, CIMS)
    }


    // CHK
    //
    //       Syntax: CHK <ea>,Dy
    //         Size: Word, (Longword)

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("0100 ---1 10-- ----");
    ____XXX___MMMXXX(opcode, CHK, 0b101111111111, Word, Chk, CIMS)

    if constexpr (C >= Core::C68020) {

        opcode = parse("0100 ---1 00-- ----");
        ____XXX___MMMXXX(opcode, CHK, 0b101111111111, Long, Chk, CIMS)
    }


    // CHK2 / CMP2
    //
    //       Syntax: CHK2 <ea>,Dy
    //         Size: Word

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X           X   X   X   X   X   X

    if constexpr (C >= Core::C68020) {

        opcode = parse("0000 0000 11-- ----");
        __________MMMXXX(opcode, CHK2, 0b001001111110, Byte, ChkCmp2, CIMS)

        opcode = parse("0000 0010 11-- ----");
        __________MMMXXX(opcode, CHK2, 0b001001111110, Word, ChkCmp2, CIMS)

        opcode = parse("0000 0100 11-- ----");
        __________MMMXXX(opcode, CHK2, 0b001001111110, Long, ChkCmp2, CIMS)
    }


    // CLR
    //
    //       Syntax: CLR <ea>
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0100 0010 ---- ----");
    ________SSMMMXXX(opcode, CLR, 0b101111111000, Byte | Word | Long, Clr, CIMS)
    ________SSMMMXXX(opcode, CLR, 0b001110000000, Byte | Word | Long, Clr, CIMSloop)


    // CMP
    //
    //       Syntax: CMP <ea>,Dy
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)  X   X   X   X   X   X   X   X   X   X

    opcode = parse("1011 ---0 ---- ----");
    ____XXX_SSMMMXXX(opcode, CMP, 0b101111111111, Byte,        Cmp, CIMS)
    ____XXX_SSMMMXXX(opcode, CMP, 0b111111111111, Word | Long, Cmp, CIMS)
    ____XXX_SSMMMXXX(opcode, CMP, 0b001110000000, Byte | Word | Long, Cmp, CIMSloop)


    // CMPA
    //
    //       Syntax: CMPA <ea>,Ay
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X   X   X   X   X   X   X   X   X   X   X   X

    opcode = parse("1011 ---- 11-- ----");
    ____XXXS__MMMXXX(opcode, CMPA, 0b111111111111, Word | Long, Cmpa, CIMS)
    ____XXXS__MMMXXX(opcode, CMPA, 0b001110000000, Word | Long, Cmpa, CIMSloop)


    // CMPI
    //
    //       Syntax: CMPI #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 1100 ---- ----");
    ________SSMMMXXX(opcode, CMPI, 0b100000000000, Byte | Word | Long, CmpiRg, CIMS)
    ________SSMMMXXX(opcode, CMPI, 0b001111111000, Byte | Word | Long, CmpiEa, CIMS)

    if constexpr (C >= Core::C68010) {

        ________SSMMMXXX(opcode, CMPI, 0b000000000110, Byte | Word | Long, CmpiEa, CIMS)
    }


    // CMPM
    //
    //       Syntax: (1) CMPM (Ax)+,(Ay)+
    //         Size: Byte, Word, Longword

    // (Ax)+,(Ay)+
    opcode = parse("1011 ---1 --00 1---");
    ____XXX_SS___XXX(opcode, CMPM, Mode::MODE_PI, Byte | Word | Long, Cmpm, CIMS)
    ____XXX_SS___XXX(opcode, CMPM, Mode::MODE_PI, Byte | Word | Long, Cmpm, CIMSloop)


    // DBcc
    //
    //       Syntax: DBcc Dn,<label>
    //         Size: Word

    // Dn,<label>
    opcode = parse("0101 ---- 1100 1---");
    _____________XXX(opcode | 0x000, DBT,  Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x100, DBF,  Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x200, DBHI, Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x300, DBLS, Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x400, DBCC, Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x500, DBCS, Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x600, DBNE, Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x700, DBEQ, Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x800, DBVC, Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x900, DBVS, Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0xA00, DBPL, Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0xB00, DBMI, Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0xC00, DBGE, Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0xD00, DBLT, Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0xE00, DBGT, Mode::MODE_IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0xF00, DBLE, Mode::MODE_IP, Word, Dbcc, CIMS)

    _____________XXX(opcode | 0x000, DBT,  Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x100, DBF,  Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x200, DBHI, Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x300, DBLS, Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x400, DBCC, Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x500, DBCS, Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x600, DBNE, Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x700, DBEQ, Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x800, DBVC, Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x900, DBVS, Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0xA00, DBPL, Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0xB00, DBMI, Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0xC00, DBGE, Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0xD00, DBLT, Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0xE00, DBGT, Mode::MODE_IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0xF00, DBLE, Mode::MODE_IP, Word, Dbcc, CIMSloop)


    // DIVS, DIVU, DIVL
    //
    //       Syntax: DIVx <ea>,Dy
    //               DIVx.L <ea>,Dq         (68020+)
    //               DIVx.L <ea>,Dr:Dq      (68020+)
    //               DIVxL.L <ea>,Dr:Dq     (68020+)
    //
    //               -------------------------------------------------
    // <ea>,Dn       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("1000 ---1 11-- ----");
    ____XXX___MMMXXX(opcode, DIVS, 0b101111111111, Word, Divs, CIMS)

    opcode = parse("1000 ---0 11-- ----");
    ____XXX___MMMXXX(opcode, DIVU, 0b101111111111, Word, Divu, CIMS)

    if constexpr (C >= Core::C68020) {

        opcode = parse("0100 1100 01-- ----");
        __________MMMXXX(opcode, DIVL, 0b101111111111, Long, Divl, CIMS)
    }


    // EOR
    //
    //       Syntax: EOR Dx,<ea>
    //        Sizes: Byte, Word, Longword

    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("1011 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, EOR, 0b101111111000, Byte | Word | Long, AndRgEa, CIMS)
    ____XXX_SSMMMXXX(opcode, EOR, 0b001110000000, Byte | Word | Long, AndRgEa, CIMSloop)


    // EORI
    //
    //       Syntax: EORI #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 1010 ---- ----");
    ________SSMMMXXX(opcode, EORI, 0b100000000000, Byte | Word | Long, AndiRg, CIMS)
    ________SSMMMXXX(opcode, EORI, 0b001111111000, Byte | Word | Long, AndiEa, CIMS)


    // EORI to CCR
    //
    //       Syntax: EORI #<data>,CCR
    //         Size: Byte
    //

    opcode = parse("0000 1010 0011 1100");
    ________________(opcode, EORICCR, Mode::MODE_IM, Byte, Andiccr, CIMS)


    // EORI to SR
    //
    //       Syntax: EORI #<data>,SR
    //         Size: Byte
    //

    opcode = parse("0000 1010 0111 1100");
    ________________(opcode, EORISR, Mode::MODE_IM, Word, Andisr, CIMS)


    // EXG
    //
    //       Syntax: EXG Dx,Dy
    //               EXG Ax,Dy
    //               EXG Ax,Ay
    //         Size: Longword

    opcode = parse("1100 ---1 0100 0---");
    ____XXX______XXX(opcode, EXG, Mode::MODE_IP, Long, ExgDxDy, CIMS)

    opcode = parse("1100 ---1 1000 1---");
    ____XXX______XXX(opcode, EXG, Mode::MODE_IP, Long, ExgAxDy, CIMS)

    opcode = parse("1100 ---1 0100 1---");
    ____XXX______XXX(opcode, EXG, Mode::MODE_IP, Long, ExgAxAy, CIMS)


    // EXT
    //
    //       Syntax: EXT Dx
    //        Sizes: Word, Longword

    opcode = parse("0100 1000 --00 0---");
    _____________XXX(opcode | 2 << 6, EXT, Mode::MODE_DN, Word, Ext, CIMS)
    _____________XXX(opcode | 3 << 6, EXT, Mode::MODE_DN, Long, Ext, CIMS)


    // EXTB
    //
    //       Syntax: EXTB Dx
    //        Sizes: Longword

    if constexpr (C >= Core::C68020) {

        opcode = parse("0100 1001 --00 0---");
        _____________XXX(opcode | 3 << 6, EXTB, Mode::MODE_DN, Long, Extb, CIMS)
    }


    // LINK
    //
    //       Syntax: LINK An,#<displacement>
    //        Sizes: Word, (Longword)

    opcode = parse("0100 1110 0101 0---");
    _____________XXX(opcode, LINK, Mode::MODE_IP, Word, Link, CIMS)

    if constexpr (C >= Core::C68020) {

        opcode = parse("0100 1000 0000 1---");
        _____________XXX(opcode, LINK, Mode::MODE_IP, Long, Link, CIMS)
    }


    // JMP
    //
    //       Syntax: JMP <ea>
    //        Sizes: Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X           X   X   X   X   X   X

    opcode = parse("0100 1110 11-- ----");
    __________MMMXXX(opcode, JMP, 0b001001111110, Long, Jmp, CIMS)


    // JSR
    //
    //       Syntax: JSR <ea>
    //        Sizes: Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X           X   X   X   X   X   X

    opcode = parse("0100 1110 10-- ----");
    __________MMMXXX(opcode, JSR, 0b001001111110, Long, Jsr, CIMS)


    // LEA
    //
    //       Syntax: LEA <ea>,Ay
    //        Sizes: Longword

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X           X   X   X   X   X   X

    opcode = parse("0100 ---1 11-- ----");
    ____XXX___MMMXXX(opcode, LEA, 0b001001111110, Long, Lea, CIMS)


    // LSL, LSR
    //
    //       Syntax: (1) LSx Dx,Dy
    //               (2) LSx #<data>,Dy
    //               (3) LSx <ea>
    //        Sizes: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1110 ---1 --10 1---");
    ____XXX_SS___XXX(opcode, LSL, Mode::MODE_DN, Byte | Word | Long, ShiftRg, CIMS)

    opcode = parse("1110 ---0 --10 1---");
    ____XXX_SS___XXX(opcode, LSR, Mode::MODE_DN, Byte | Word | Long, ShiftRg, CIMS)

    // #<data>,Dy
    opcode = parse("1110 ---1 --00 1---");
    ____XXX_SS___XXX(opcode, LSL, Mode::MODE_IM, Byte | Word | Long, ShiftIm, CIMS)

    opcode = parse("1110 ---0 --00 1---");
    ____XXX_SS___XXX(opcode, LSR, Mode::MODE_IM, Byte | Word | Long, ShiftIm, CIMS)

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1110 0011 11-- ----");
    __________MMMXXX(opcode, LSL, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, LSL, 0b001110000000, Word, ShiftEa, CIMSloop)

    opcode = parse("1110 0010 11-- ----");
    __________MMMXXX(opcode, LSR, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, LSR, 0b001110000000, Word, ShiftEa, CIMSloop)


    // MOVE
    //
    //       Syntax: MOVE <ea>,<e>
    //        Sizes: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)   X   X   X   X   X   X   X   X   X   X

    //               -------------------------------------------------
    // <e>           | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    // <ea>,Dy
    opcode = parse("00-- ---0 00-- ----");
    __SSXXX___MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move0, CIMS)
    __SSXXX___MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move0, CIMS)

    // <ea>,(Ay)
    opcode = parse("00-- ---0 10-- ----");
    __SSXXX___MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move2, CIMS)
    __SSXXX___MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move2, CIMS)
    __SSXXX___MMMXXX(opcode, MOVE, 0b101110000000, Byte,        Move2, CIMSloop)
    __SSXXX___MMMXXX(opcode, MOVE, 0b111110000000, Word | Long, Move2, CIMSloop)

    // <ea>,(Ay)+
    opcode = parse("00-- ---0 11-- ----");
    __SSXXX___MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move3, CIMS)
    __SSXXX___MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move3, CIMS)
    __SSXXX___MMMXXX(opcode, MOVE, 0b101110000000, Byte,        Move3, CIMSloop)
    __SSXXX___MMMXXX(opcode, MOVE, 0b111110000000, Word | Long, Move3, CIMSloop)

    // <ea>,-(Ay)
    opcode = parse("00-- ---1 00-- ----");
    __SSXXX___MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move4, CIMS)
    __SSXXX___MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move4, CIMS)
    __SSXXX___MMMXXX(opcode, MOVE, 0b101110000000, Byte,        Move4, CIMSloop)
    __SSXXX___MMMXXX(opcode, MOVE, 0b111110000000, Word | Long, Move4, CIMSloop)

    // <ea>,(d,Ay)
    opcode = parse("00-- ---1 01-- ----");
    __SSXXX___MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move5, CIMS)
    __SSXXX___MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move5, CIMS)

    // <ea>,(d,Ay,Xi)
    opcode = parse("00-- ---1 10-- ----");
    __SSXXX___MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move6, CIMS)
    __SSXXX___MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move6, CIMS)

    // <ea>,ABS.w
    opcode = parse("00-- 0001 11-- ----");
    __SS______MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move7, CIMS)
    __SS______MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move7, CIMS)

    // <ea>,ABS.l
    opcode = parse("00-- 0011 11-- ----");
    __SS______MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move8, CIMS)
    __SS______MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move8, CIMS)


    // MOVEA
    //
    //       Syntax: MOVEA <ea>,Ay
    //        Sizes: Word, Longword

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X   X   X   X   X   X   X   X   X   X   X   X

    opcode = parse("001- ---0 01-- ----");

    ____XXX___MMMXXX(opcode | 0 << 12, MOVEA, 0b111111111111, Long, Movea, CIMS)
    ____XXX___MMMXXX(opcode | 1 << 12, MOVEA, 0b111111111111, Word, Movea, CIMS)


    // MOVEC
    //
    //       Syntax: MOVEC Rc,Rx
    //               MOVEC Rx,Rc
    //        Sizes: Longword

    if constexpr (C >= Core::C68010) {

        opcode = parse("0100 1110 0111 101-");
        ________________(opcode | 0, MOVEC, Mode::MODE_IP, Long, MovecRcRx, CIMS)
        ________________(opcode | 1, MOVEC, Mode::MODE_IP, Long, MovecRxRc, CIMS)
    }


    // MOVEM
    //
    //       Syntax: MOVEM <ea>,<register list>
    //               MOVEM <register list>,<ea>
    //        Sizes: Word, Longword

    //               -------------------------------------------------
    // <ea>,<list>   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X       X   X   X   X   X   X

    //               -------------------------------------------------
    // <list>,<ea>   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X       X   X   X   X   X

    opcode = parse("0100 1100 1--- ----");
    __________MMMXXX(opcode | 0 << 6, MOVEM, 0b001101111110, Word, MovemEaRg, CIMS)
    __________MMMXXX(opcode | 1 << 6, MOVEM, 0b001101111110, Long, MovemEaRg, CIMS)

    opcode = parse("0100 1000 1--- ----");
    __________MMMXXX(opcode | 0 << 6, MOVEM, 0b001011111000, Word, MovemRgEa, CIMS)
    __________MMMXXX(opcode | 1 << 6, MOVEM, 0b001011111000, Long, MovemRgEa, CIMS)


    // MOVEP
    //
    //       Syntax: MOVEP Dx,(d,Ay)
    //               MOVEP (d,Ay),Dx
    //         Size: Word, Longword

    // MOVEP Dx,(d,Ay)
    opcode = parse("0000 ---1 1-00 1---");
    ____XXX______XXX(opcode | 0 << 6, MOVEP, Mode::MODE_DI, Word, MovepDxEa, CIMS)
    ____XXX______XXX(opcode | 1 << 6, MOVEP, Mode::MODE_DI, Long, MovepDxEa, CIMS)

    // MOVEP (d,Ay),Dx
    opcode = parse("0000 ---1 0-00 1---");
    ____XXX______XXX(opcode | 0 << 6, MOVEP, Mode::MODE_DI, Word, MovepEaDx, CIMS)
    ____XXX______XXX(opcode | 1 << 6, MOVEP, Mode::MODE_DI, Long, MovepEaDx, CIMS)


    // MOVEQ
    //
    //       Syntax: MOVEQ #<data>,Dn
    //        Sizes: Longword

    // #<data>,Dn
    opcode = parse("0111 ---0 ---- ----");
    ____XXX_XXXXXXXX(opcode, MOVEQ, Mode::MODE_IM, Long, Moveq, CIMS)


    // MOVES
    //
    //       Syntax: MOVES Dx,<ea>
    //        Sizes: Byte, Word, Longword

    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    if constexpr (C >= Core::C68010) {

        opcode = parse("0000 1110 ---- ----");
        ________SSMMMXXX(opcode, MOVES, 0b001111111000, Byte | Word | Long, Moves, CIMS)
    }


    // MOVE from CCR
    //
    //       Syntax: MOVE CCR,<ea>
    //         Size: Word
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    if constexpr (C >= Core::C68010) {

        opcode = parse("0100 0010 11-- ----");
        __________MMMXXX(opcode, MOVEFCCR, 0b100000000000, Word, MoveCcrRg, CIMS)
        __________MMMXXX(opcode, MOVEFCCR, 0b001111111000, Word, MoveCcrEa, CIMS)
    }


    // MOVE to CCR
    //
    //       Syntax: MOVE <ea>,CCR
    //         Size: Word
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("0100 0100 11-- ----");
    __________MMMXXX(opcode, MOVETCCR, 0b101111111111, Word, MoveToCcr, CIMS)


    // MOVE from SR
    //
    //       Syntax: MOVE SR,<ea>
    //         Size: Word
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0100 0000 11-- ----");

    __________MMMXXX(opcode, MOVEFSR, 0b100000000000, Word, MoveSrRg, CIMS)
    __________MMMXXX(opcode, MOVEFSR, 0b001111111000, Word, MoveSrEa, CIMS)


    // MOVE to SR
    //
    //       Syntax: MOVE <ea>,SR
    //         Size: Word
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("0100 0110 11-- ----");
    __________MMMXXX(opcode, MOVETSR, 0b101111111111, Word, MoveToSr, CIMS)


    // MOVEUSP
    //
    //       Syntax: MOVE USP,An
    //               MOVE An,USP
    //        Sizes: Longword

    opcode = parse("0100 1110 0110 ----");
    _____________XXX(opcode | 1 << 3, MOVEUSP, Mode::MODE_IP, Long, MoveUspAn, CIMS)
    _____________XXX(opcode | 0 << 3, MOVEUSP, Mode::MODE_IP, Long, MoveAnUsp, CIMS)


    // MULS, MULU
    //
    //       Syntax: MULx <ea>,Dy
    //        Sizes: Word x Word -> Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("1100 ---1 11-- ----");
    ____XXX___MMMXXX(opcode, MULS, 0b101111111111, Word, Muls, CIMS)

    opcode = parse("1100 ---0 11-- ----");
    ____XXX___MMMXXX(opcode, MULU, 0b101111111111, Word, Mulu, CIMS)

    if constexpr (C >= Core::C68020) {

        opcode = parse("0100 1100 00-- ----");
        __________MMMXXX(opcode, MULL, 0b101111111111, Long, Mull, CIMS)
    }


    // NBCD
    //
    //       Syntax: NBCD <ea>
    //        Sizes: Byte

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0100 1000 00-- ----");
    __________MMMXXX(opcode, NBCD, 0b100000000000, Byte, NbcdRg, CIMS)
    __________MMMXXX(opcode, NBCD, 0b001111111000, Byte, NbcdEa, CIMS)
    __________MMMXXX(opcode, NBCD, 0b001110000000, Byte, NbcdEa, CIMSloop)


    // NEG, NEGX, NOT
    //
    //       Syntax: Nxx <ea>
    //        Sizes: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0100 0100 ---- ----");
    ________SSMMMXXX(opcode, NEG, 0b100000000000, Byte | Word | Long, NegRg, CIMS)
    ________SSMMMXXX(opcode, NEG, 0b001111111000, Byte | Word | Long, NegEa, CIMS)
    ________SSMMMXXX(opcode, NEG, 0b001110000000, Byte | Word | Long, NegEa, CIMSloop)

    opcode = parse("0100 0000 ---- ----");
    ________SSMMMXXX(opcode, NEGX, 0b100000000000, Byte | Word | Long, NegRg, CIMS)
    ________SSMMMXXX(opcode, NEGX, 0b001111111000, Byte | Word | Long, NegEa, CIMS)
    ________SSMMMXXX(opcode, NEGX, 0b001110000000, Byte | Word | Long, NegEa, CIMSloop)

    opcode = parse("0100 0110 ---- ----");
    ________SSMMMXXX(opcode, NOT, 0b100000000000, Byte | Word | Long, NegRg, CIMS)
    ________SSMMMXXX(opcode, NOT, 0b001111111000, Byte | Word | Long, NegEa, CIMS)
    ________SSMMMXXX(opcode, NOT, 0b001110000000, Byte | Word | Long, NegEa, CIMSloop)


    // NOP
    //
    //       Syntax: NOP
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0001");
    ________________(opcode, NOP, Mode::MODE_IP, Long, Nop, CIMS)


    // OR
    //
    //       Syntax: OR <ea>,Dy
    //               OR Dx,<ea>
    //        Sizes: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("1000 ---0 ---- ----");
    ____XXX_SSMMMXXX(opcode, OR, 0b101111111111, Byte | Word | Long, AndEaRg, CIMS)
    ____XXX_SSMMMXXX(opcode, OR, 0b001110000000, Byte | Word | Long, AndEaRg, CIMSloop)


    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("1000 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, OR, 0b001111111000, Byte | Word | Long, AndRgEa, CIMS)
    ____XXX_SSMMMXXX(opcode, OR, 0b001110000000, Byte | Word | Long, AndRgEa, CIMSloop)


    // ORI
    //
    //       Syntax: ORI #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 0000 ---- ----");
    ________SSMMMXXX(opcode, ORI, 0b100000000000, Byte | Word | Long, AndiRg, CIMS)
    ________SSMMMXXX(opcode, ORI, 0b001111111000, Byte | Word | Long, AndiEa, CIMS)


    // ORI to CCR
    //
    //       Syntax: ORI #<data>,CCR
    //         Size: Byte
    //

    opcode = parse("0000 0000 0011 1100");
    ________________(opcode, ORICCR, Mode::MODE_IM, Byte, Andiccr, CIMS)


    // ORI to SR
    //
    //       Syntax: ORI #<data>,SR
    //         Size: Byte
    //

    opcode = parse("0000 0000 0111 1100");
    ________________(opcode, ORISR, Mode::MODE_IM, Word, Andisr, CIMS)


    // ROL, ROR, ROXL, ROXR
    //
    //       Syntax: (1) ROxx Dx,Dy
    //               (2) ROxx #<data>,Dy
    //               (3) ROxx <ea>
    //        Sizes: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1110 ---1 --11 1---");
    ____XXX_SS___XXX(opcode, ROL, Mode::MODE_DN, Byte | Word | Long, ShiftRg, CIMS)

    opcode = parse("1110 ---0 --11 1---");
    ____XXX_SS___XXX(opcode, ROR, Mode::MODE_DN, Byte | Word | Long, ShiftRg, CIMS)

    opcode = parse("1110 ---1 --11 0---");
    ____XXX_SS___XXX(opcode, ROXL, Mode::MODE_DN, Byte | Word | Long, ShiftRg, CIMS)

    opcode = parse("1110 ---0 --11 0---");
    ____XXX_SS___XXX(opcode, ROXR, Mode::MODE_DN, Byte | Word | Long, ShiftRg, CIMS)

    // #<data>,Dy
    opcode = parse("1110 ---1 --01 1---");
    ____XXX_SS___XXX(opcode, ROL, Mode::MODE_IM, Byte | Word | Long, ShiftIm, CIMS)

    opcode = parse("1110 ---0 --01 1---");
    ____XXX_SS___XXX(opcode, ROR, Mode::MODE_IM, Byte | Word | Long, ShiftIm, CIMS)

    opcode = parse("1110 ---1 --01 0---");
    ____XXX_SS___XXX(opcode, ROXL, Mode::MODE_IM, Byte | Word | Long, ShiftIm, CIMS)

    opcode = parse("1110 ---0 --01 0---");
    ____XXX_SS___XXX(opcode, ROXR, Mode::MODE_IM, Byte | Word | Long, ShiftIm, CIMS)

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1110 0111 11-- ----");
    __________MMMXXX(opcode, ROL, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, ROL, 0b001110000000, Word, ShiftEa, CIMSloop)

    opcode = parse("1110 0110 11-- ----");
    __________MMMXXX(opcode, ROR, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, ROR, 0b001110000000, Word, ShiftEa, CIMSloop)

    opcode = parse("1110 0101 11-- ----");
    __________MMMXXX(opcode, ROXL, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, ROXL, 0b001110000000, Word, ShiftEa, CIMSloop)

    opcode = parse("1110 0100 11-- ----");
    __________MMMXXX(opcode, ROXR, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, ROXR, 0b001110000000, Word, ShiftEa, CIMSloop)


    // PACK
    //
    //       Syntax: PACK -(Ax),-(Ay),#<adjustment>
    //               PACK DX,Dy,#<adjustment>
    //        Sizes: Unsized

    if constexpr (C >= Core::C68020) {

        opcode = parse("1000 ---1 0100 0---");
        ____XXX______XXX(opcode, PACK, Mode::MODE_DN, Word, PackDn, CIMS)

        opcode = parse("1000 ---1 0100 1---");
        ____XXX______XXX(opcode, PACK, Mode::MODE_PD, Word, PackPd, CIMS)
    }


    // PEA
    //
    //       Syntax: PEA <ea>,Ay
    //        Sizes: Longword

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X           X   X   X   X   X   X

    opcode = parse("0100 1000 01-- ----");
    __________MMMXXX(opcode, PEA, 0b001001111110, Long, Pea, CIMS)


    // RESET
    //
    //       Syntax: RESET
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0000");
    ________________(opcode, RESET, Mode::MODE_IP, Long, Reset, CIMS)


    // RTD
    //
    //       Syntax: RTD
    //        Sizes: Unsized

    if constexpr (C >= Core::C68010) {

        opcode = parse("0100 1110 0111 0100");
        ________________(opcode, RTD, Mode::MODE_IP, Long, Rtd, CIMS)
    }


    // RTE
    //
    //       Syntax: RTE
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0011");
    ________________(opcode, RTE, Mode::MODE_IP, Long, Rte, CIMS)


    // RTM
    //
    //       Syntax: RTM Rn
    //        Sizes: Unsized

    if constexpr (C >= Core::C68020) {

        if (model == Model::M68EC020 || model == Model::M68020) {

            opcode = parse("0000 0110 1100 ----");
            ____________XXXX(opcode, RTM, Mode::MODE_IP, Long, Rtm, CIMS)
        }
    }


    // RTR
    //
    //       Syntax: RTR
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0111");
    ________________(opcode, RTR, Mode::MODE_IP, Long, Rtr, CIMS)


    // RTS
    //
    //       Syntax: RTS
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0101");
    ________________(opcode, RTS, Mode::MODE_IP, Long, Rts, CIMS)


    // SBCD
    //
    //       Syntax: (1) SBCD Dx,Dy
    //               (2) SBCD -(Ax),-(Ay)
    //         Size: Byte

    // Dx,Dy
    opcode = parse("1000 ---1 0000 0---");
    ____XXX______XXX(opcode, SBCD, Mode::MODE_DN, Byte, AbcdRg, CIMS)

    // -(Ax),-(Ay)
    opcode = parse("1000 ---1 0000 1---");
    ____XXX______XXX(opcode, SBCD, Mode::MODE_PD, Byte, AbcdEa, CIMS)
    ____XXX______XXX(opcode, SBCD, Mode::MODE_PD, Byte, AbcdEa, CIMSloop)


    // Scc
    //
    //       Syntax: Scc <ea>
    //         Size: Word

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0101 ---- 11-- ----");
    __________MMMXXX(opcode | 0x000, ST,  0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x100, SF,  0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x200, SHI, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x300, SLS, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x400, SCC, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x500, SCS, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x600, SNE, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x700, SEQ, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x800, SVC, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x900, SVS, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0xA00, SPL, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0xB00, SMI, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0xC00, SGE, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0xD00, SLT, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0xE00, SGT, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0xF00, SLE, 0b100000000000, Byte, SccRg, CIMS)

    __________MMMXXX(opcode | 0x000, ST,  0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x100, SF,  0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x200, SHI, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x300, SLS, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x400, SCC, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x500, SCS, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x600, SNE, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x700, SEQ, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x800, SVC, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x900, SVS, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0xA00, SPL, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0xB00, SMI, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0xC00, SGE, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0xD00, SLT, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0xE00, SGT, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0xF00, SLE, 0b001111111000, Byte, SccEa, CIMS)

    // STOP
    //
    //       Syntax: STOP
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0010");
    ________________(opcode, STOP, Mode::MODE_IP, Word, Stop, CIMS)


    // SUB
    //
    //       Syntax: (1) SUB <ea>,Dy
    //               (2) SUB Dx,<ea>
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)  X   X   X   X   X   X   X   X   X   X

    opcode = parse("1001 ---0 ---- ----");
    ____XXX_SSMMMXXX(opcode, SUB, 0b101111111111, Byte,        AddEaRg, CIMS)
    ____XXX_SSMMMXXX(opcode, SUB, 0b111111111111, Word | Long, AddEaRg, CIMS)
    ____XXX_SSMMMXXX(opcode, SUB, 0b001110000000, Byte,        AddEaRg, CIMSloop)
    ____XXX_SSMMMXXX(opcode, SUB, 0b001110000000, Word | Long, AddEaRg, CIMSloop)


    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1001 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, SUB, 0b001111111000, Byte | Word | Long, AddRgEa, CIMS)
    ____XXX_SSMMMXXX(opcode, SUB, 0b001110000000, Byte | Word | Long, AddRgEa, CIMSloop)


    // SUBA
    //
    //       Syntax: SUBA <ea>,Ay
    //         Size: Word, Longword
    //
    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X   X   X   X   X   X   X   X   X   X   X   X

    opcode = parse("1001 ---- 11-- ----");
    ____XXXS__MMMXXX(opcode, SUBA, 0b111111111111, Word | Long, Adda, CIMS)
    ____XXXS__MMMXXX(opcode, SUBA, 0b001110000000, Word | Long, Adda, CIMSloop)


    // SUBI
    //
    //       Syntax: SUBI #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 0100 ---- ----");
    ________SSMMMXXX(opcode, SUBI, 0b100000000000, Byte | Word | Long, AddiRg, CIMS)
    ________SSMMMXXX(opcode, SUBI, 0b001111111000, Byte | Word | Long, AddiEa, CIMS)


    // SUBQ
    //
    //       Syntax: SUBQ #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)  X   X   X   X   X   X   X

    opcode = parse("0101 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, SUBQ, 0b100000000000, Byte | Word | Long, AddqDn, CIMS)
    ____XXX_SSMMMXXX(opcode, SUBQ, 0b010000000000, Word | Long,        AddqAn, CIMS)
    ____XXX_SSMMMXXX(opcode, SUBQ, 0b001111111000, Byte | Word | Long, AddqEa, CIMS)


    // SUBX
    //
    //       Syntax: (1) SUBX Dx,Dy
    //               (2) SUBX -(Ax),-(Ay)
    //         Size: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1001 ---1 --00 0---");
    ____XXX_SS___XXX(opcode, SUBX, Mode::MODE_DN, Byte | Word | Long, AddxRg, CIMS)

    // -(Ax),-(Ay)
    opcode = parse("1001 ---1 --00 1---");
    ____XXX_SS___XXX(opcode, SUBX, Mode::MODE_PD, Byte | Word | Long, AddxEa, CIMS)
    ____XXX_SS___XXX(opcode, SUBX, Mode::MODE_PD, Byte | Word | Long, AddxEa, CIMSloop)


    // SWAP
    //
    //       Syntax: SWAP Dn
    //         Size: Word

    opcode = parse("0100 1000 0100 0---");
    _____________XXX(opcode, SWAP, Mode::MODE_DN, Word, Swap, CIMS)


    // TAS
    //
    //       Syntax: TAS <ea>
    //         Size: Byte

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0100 1010 11-- ----");
    __________MMMXXX(opcode, TAS, 0b100000000000, Byte, TasRg, CIMS)
    __________MMMXXX(opcode, TAS, 0b001111111000, Byte, TasEa, CIMS)


    // TRAP
    //
    //       Syntax: TRAP #<vector>
    //        Sizes: Unsized

    opcode = parse("0100 1110 0100 ----");
    ____________XXXX(opcode, TRAP, Mode::MODE_IP, Long, Trap, CIMS)


    // TRAPcc
    //
    //       Syntax: TRAPcc #<vector>
    //        Sizes: Unsized

    if constexpr (C >= Core::C68020) {

        opcode = parse("0101 ---- 1111 1100");
        ________________(opcode | 0x000, TRAPT,  Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x100, TRAPF,  Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x200, TRAPHI, Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x300, TRAPLS, Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x400, TRAPCC, Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x500, TRAPCS, Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x600, TRAPNE, Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x700, TRAPEQ, Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x800, TRAPVC, Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x900, TRAPVS, Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0xA00, TRAPPL, Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0xB00, TRAPMI, Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0xC00, TRAPGE, Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0xD00, TRAPLT, Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0xE00, TRAPGT, Mode::MODE_IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0xF00, TRAPLE, Mode::MODE_IP, Byte, Trapcc, CIMS)

        opcode = parse("0101 ---- 1111 1010");
        ________________(opcode | 0x000, TRAPT,  Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x100, TRAPF,  Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x200, TRAPHI, Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x300, TRAPLS, Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x400, TRAPCC, Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x500, TRAPCS, Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x600, TRAPNE, Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x700, TRAPEQ, Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x800, TRAPVC, Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x900, TRAPVS, Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0xA00, TRAPPL, Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0xB00, TRAPMI, Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0xC00, TRAPGE, Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0xD00, TRAPLT, Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0xE00, TRAPGT, Mode::MODE_IP, Word, Trapcc, CIMS)
        ________________(opcode | 0xF00, TRAPLE, Mode::MODE_IP, Word, Trapcc, CIMS)

        opcode = parse("0101 ---- 1111 1011");
        ________________(opcode | 0x000, TRAPT,  Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x100, TRAPF,  Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x200, TRAPHI, Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x300, TRAPLS, Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x400, TRAPCC, Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x500, TRAPCS, Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x600, TRAPNE, Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x700, TRAPEQ, Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x800, TRAPVC, Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x900, TRAPVS, Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0xA00, TRAPPL, Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0xB00, TRAPMI, Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0xC00, TRAPGE, Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0xD00, TRAPLT, Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0xE00, TRAPGT, Mode::MODE_IP, Long, Trapcc, CIMS)
        ________________(opcode | 0xF00, TRAPLE, Mode::MODE_IP, Long, Trapcc, CIMS)
    }


    // TRAPV
    //
    //       Syntax: TRAPV
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0110");
    ________________(opcode, TRAPV, Mode::MODE_IP, Long, Trapv, CIMS)


    // TST
    //
    //       Syntax: TST <ea>
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)  X   X   X   X   X   X   X  (X) (X) (X)

    opcode = parse("0100 1010 ---- ----");
    ________SSMMMXXX(opcode, TST, 0b101111111000, Byte | Word | Long, Tst, CIMS)
    ________SSMMMXXX(opcode, TST, 0b001110000000, Byte | Word | Long, Tst, CIMSloop)

    if constexpr (C >= Core::C68020) {

        ________SSMMMXXX(opcode, TST, 0b000000000111, Byte, Tst, CIMS)
        ________SSMMMXXX(opcode, TST, 0b010000000111, Word | Long, Tst, CIMS)
    }


    // UNLK
    //
    //       Syntax: UNLK An
    //        Sizes: Word

    opcode = parse("0100 1110 0101 1---");
    _____________XXX(opcode, UNLK, Mode::MODE_IP, Word, Unlk, CIMS)


    // UNPK
    //
    //       Syntax: UNPK -(Ax),-(Ay),#<adjustment>
    //               UNPK DX,Dy,#<adjustment>
    //        Sizes: Unsized

    if constexpr (C >= Core::C68020) {

        opcode = parse("1000 ---1 1000 0---");
        ____XXX______XXX(opcode, UNPK, Mode::MODE_DN, Word, UnpkDn, CIMS)

        opcode = parse("1000 ---1 1000 1---");
        ____XXX______XXX(opcode, UNPK, Mode::MODE_PD, Word, UnpkPd, CIMS)
    }

    //
    // Line-F area
    //

    if constexpr (C >= Core::C68020) {

        //
        // Coprocessor interface
        //

        if (hasCPI()) {

            opcode = parse("1111 ---0 10-- ----");
            ____XXX___XXXXXX(opcode, cpBcc, Mode::MODE_IP, Word, CpBcc, CIMS)

            opcode = parse("1111 ---0 11-- ----");
            ____XXX___XXXXXX(opcode, cpBcc, Mode::MODE_IP, Long, CpBcc, CIMS)

            opcode = parse("1111 ---0 00-- ----");
            ____XXX___XXXXXX(opcode, cpGEN, Mode::MODE_IP, (Size)0, CpGen, CIMS)

            opcode = parse("1111 ---1 01-- ----");
            ____XXX___MMMXXX(opcode, cpRESTORE, 0b001101111110, Word, CpRestore, CIMS)

            opcode = parse("1111 ---1 00-- ----");
            ____XXX___MMMXXX(opcode, cpSAVE, 0b001011111000, Word, CpSave, CIMS)

            opcode = parse("1111 ---0 0111 1---");
            ____XXX______XXX(opcode, cpTRAPcc, Mode::MODE_IP, Word, CpTrapcc, CIMS)

            opcode = parse("1111 ---0 01-- ----");
            ____XXX___MMMXXX(opcode, cpScc, 0b101111111000, Byte, CpScc, CIMS)

            opcode = parse("1111 ---0 0100 1---");
            ____XXX______XXX(opcode, cpDBcc, Mode::MODE_IP, (Size)0, CpDbcc, CIMS)
        }

        
        //
        // Memory management unit
        //

        if (model == Model::M68030) {

            opcode = parse("1111 0000 00-- ----");
            __________XXXXXX(opcode, cpGEN, Mode::MODE_IP, Unsized, PGen, CIMS)
            __________MMMXXX(opcode, cpGEN, 0b111111111111, Unsized, PGen, CIMS)
        }

        if (model == Model::M68040 || model == Model::M68LC040) {

            opcode = parse("1111 0101 000- ----");
            ___________XXXXX(opcode, PFLUSH, Mode::MODE_AI, Unsized, PFlush40, CIMS)

            opcode = parse("1111 0101 0100 1---");
            _____________XXX(opcode, PTEST, Mode::MODE_AI, Unsized, PTest40, CIMS)

            opcode = parse("1111 0101 0110 1---");
            _____________XXX(opcode, PTEST, Mode::MODE_AI, Unsized, PTest40, CIMS)
        }


        //
        // 68040 instructions
        //

        if (model == Model::M68040 || model == Model::M68EC040 || model == Model::M68LC040) {


            //
            // CINV
            //

            opcode = parse("1111 0100 --0- ----");
            for (int i = 0; i < 4; i++) {
                ___________XXXXX(opcode | i << 6, CINV, Mode::MODE_AI, Unsized, Cinv, CIMS)
            }


            //
            // CPUSH
            //

            opcode = parse("1111 0100 --1- ----");
            for (int i = 0; i < 4; i++) {
                ___________XXXXX(opcode | i << 6, CPUSH, Mode::MODE_AI, Unsized, Cpush, CIMS)
            }


            //
            // MOVE16
            //

            opcode = parse("1111 0110 0010 0---");
            _____________XXX(opcode, MOVE16, Mode::MODE_IP, Unsized, Move16PiPi, CIMS)

            opcode = parse("1111 0110 0000 0---");
            _____________XXX(opcode, MOVE16, Mode::MODE_IP, Unsized, Move16PiAl, CIMS)

            opcode = parse("1111 0110 0000 1---");
            _____________XXX(opcode, MOVE16, Mode::MODE_IP, Unsized, Move16AlPi, CIMS)

            opcode = parse("1111 0110 0001 0---");
            _____________XXX(opcode, MOVE16, Mode::MODE_IP, Unsized, Move16AiAl, CIMS)

            opcode = parse("1111 0110 0001 1---");
            _____________XXX(opcode, MOVE16, Mode::MODE_IP, Unsized, Move16AlAi, CIMS)
        }
    }

    //
    // Floating-Point Unit
    //

    if (model == Model::M68040) {

        opcode = parse("1111 0010 100- ----");
        ___________XXXXX(opcode, FBcc, Mode::MODE_IP, Word, FBcc, CIMS)

        opcode = parse("1111 0010 110- ----");
        ___________XXXXX(opcode, FBcc, Mode::MODE_IP, Long, FBcc, CIMS)

        opcode = parse("1111 0010 00-- ----");
        __________XXXXXX(opcode, cpGEN, Mode::MODE_IP, Unsized, FGen, CIMS)
        __________MMMXXX(opcode, cpGEN, 0b111111111111, Unsized, FGen, CIMS)

        opcode = parse("1111 0011 01-- ----");
        __________MMMXXX(opcode, FRESTORE, 0b001101111110, Word, FRestore, CIMS)

        opcode = parse("1111 0011 00-- ----");
        __________MMMXXX(opcode, FSAVE, 0b001011111000, Word, FSave, CIMS)

        opcode = parse("1111 0010 0111 1---");
        ________________(opcode | 0b010, FTRAPcc, Mode::MODE_IP, Word, FTrapcc, CIMS)
        ________________(opcode | 0b011, FTRAPcc, Mode::MODE_IP, Long, FTrapcc, CIMS)
        ________________(opcode | 0b100, FTRAPcc, Mode::MODE_IP, Unsized, FTrapcc, CIMS)

        opcode = parse("1111 0010 01-- ----");
        __________MMMXXX(opcode, FScc, 0b101111111000, Byte, FScc, CIMS)

        opcode = parse("1111 0010 0100 1---");
        _____________XXX(opcode, FDBcc, Mode::MODE_IP, Word, FDbcc, CIMS)
    }
}
