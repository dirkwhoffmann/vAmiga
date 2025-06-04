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
#if MOIRA_ENABLE_DASM == true
#define REGISTER_DASM(id,name,I,M,S) if (regDasm) dasm[id] = DASM_HANDLER(name,I,M,S);
#else
#define REGISTER_DASM(id,name,I,M,S) { }
#endif

#if MOIRA_BUILD_INSTR_INFO_TABLE == true
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

    XXXXXXXXXXXXXXXX(Instr::ILLEGAL, Mode::IP, (Size)0, Illegal, CIMS)

    for (int i = 0; i < 0x10000; i++) {
        loop[i] = nullptr;
    }


    // Unimplemented instructions
    //
    //       Format: 1010 ---- ---- ---- (Line A instructions)
    //               1111 ---- ---- ---- (Line F instructions)

    opcode = parse("1010 ---- ---- ----");
    ____XXXXXXXXXXXX(opcode, Instr::LINE_A, Mode::IP, (Size)0, LineA, CIMS)

    opcode = parse("1111 ---- ---- ----");
    ____XXXXXXXXXXXX(opcode, Instr::LINE_F, Mode::IP, (Size)0, LineF, CIMS)


    // ABCD
    //
    //       Syntax: (1) ABCD Dx,Dy
    //               (2) ABCD -(Ax),-(Ay)
    //         Size: Byte

    // Dx,Dy
    opcode = parse("1100 ---1 0000 0---");
    ____XXX______XXX(opcode, Instr::ABCD, Mode::DN, Byte, AbcdRg, CIMS)

    // -(Ax),-(Ay)
    opcode = parse("1100 ---1 0000 1---");
    ____XXX______XXX(opcode, Instr::ABCD, Mode::PD, Byte, AbcdEa, CIMS)
    ____XXX______XXX(opcode, Instr::ABCD, Mode::PD, Byte, AbcdEa, CIMSloop)


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
    ____XXX_SSMMMXXX(opcode, Instr::ADD, 0b101111111111, Byte,        AddEaRg, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::ADD, 0b111111111111, Word | Long, AddEaRg, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::ADD, 0b001110000000, Byte,        AddEaRg, CIMSloop)
    ____XXX_SSMMMXXX(opcode, Instr::ADD, 0b001110000000, Word | Long, AddEaRg, CIMSloop)


    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1101 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, Instr::ADD, 0b001111111000, Byte | Word | Long, AddRgEa, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::ADD, 0b001110000000, Byte | Word | Long, AddRgEa, CIMSloop)


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
    ____XXXS__MMMXXX(opcode, Instr::ADDA, 0b111111111111, Word | Long, Adda, CIMS)
    ____XXXS__MMMXXX(opcode, Instr::ADDA, 0b001110000000, Word | Long, Adda, CIMSloop)


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
    ________SSMMMXXX(opcode, Instr::ADDI, 0b100000000000, Byte | Word | Long, AddiRg, CIMS)
    ________SSMMMXXX(opcode, Instr::ADDI, 0b001111111000, Byte | Word | Long, AddiEa, CIMS)


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
    ____XXX_SSMMMXXX(opcode, Instr::ADDQ, 0b100000000000, Byte | Word | Long, AddqDn, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::ADDQ, 0b010000000000, Word | Long,        AddqAn, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::ADDQ, 0b001111111000, Byte | Word | Long, AddqEa, CIMS)


    // ADDX
    //
    //       Syntax: (1) ADDX Dx,Dy
    //               (2) ADDX -(Ax),-(Ay)
    //         Size: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1101 ---1 --00 0---");
    ____XXX_SS___XXX(opcode, Instr::ADDX, Mode::DN, Byte | Word | Long, AddxRg, CIMS)

    // -(Ax),-(Ay)
    opcode = parse("1101 ---1 --00 1---");
    ____XXX_SS___XXX(opcode, Instr::ADDX, Mode::PD, Byte | Word | Long, AddxEa, CIMS)
    ____XXX_SS___XXX(opcode, Instr::ADDX, Mode::PD, Byte | Word | Long, AddxEa, CIMSloop)


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
    ____XXX_SSMMMXXX(opcode, Instr::AND, 0b101111111111, Byte | Word | Long, AndEaRg, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::AND, 0b001110000000, Byte | Word | Long, AndEaRg, CIMSloop)


    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1100 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, Instr::AND, 0b001111111000, Byte | Word | Long, AndRgEa, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::AND, 0b001110000000, Byte | Word | Long, AndRgEa, CIMSloop)


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
    ________SSMMMXXX(opcode, Instr::ANDI, 0b100000000000, Byte | Word | Long, AndiRg, CIMS)
    ________SSMMMXXX(opcode, Instr::ANDI, 0b001111111000, Byte | Word | Long, AndiEa, CIMS)


    // ANDI to CCR
    //
    //       Syntax: ANDI #<data>,CCR
    //         Size: Byte
    //

    opcode = parse("0000 0010 0011 1100");
    ________________(opcode, Instr::ANDICCR, Mode::IM, Byte, Andiccr, CIMS)


    // ANDI to SR
    //
    //       Syntax: ANDI #<data>,SR
    //         Size: Byte
    //

    opcode = parse("0000 0010 0111 1100");
    ________________(opcode, Instr::ANDISR, Mode::IM, Word, Andisr, CIMS)


    // ASL, Instr::ASR
    //
    //       Syntax: (1) ASx Dx,Dy
    //               (2) ASx #<data>,Dy
    //               (3) ASx <ea>
    //         Size: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1110 ---1 --10 0---");
    ____XXX_SS___XXX(opcode, Instr::ASL, Mode::DN,  Byte | Word | Long, ShiftRg, CIMS)

    opcode = parse("1110 ---0 --10 0---");
    ____XXX_SS___XXX(opcode, Instr::ASR, Mode::DN,  Byte | Word | Long, ShiftRg, CIMS)

    // #<data>,Dy
    opcode = parse("1110 ---1 --00 0---");
    ____XXX_SS___XXX(opcode, Instr::ASL, Mode::IM, Byte | Word | Long, ShiftIm, CIMS)

    opcode = parse("1110 ---0 --00 0---");
    ____XXX_SS___XXX(opcode, Instr::ASR, Mode::IM, Byte | Word | Long, ShiftIm, CIMS)

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1110 0001 11-- ----");
    __________MMMXXX(opcode, Instr::ASL, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, Instr::ASL, 0b001110000000, Word, ShiftEa, CIMSloop)

    opcode = parse("1110 0000 11-- ----");
    __________MMMXXX(opcode, Instr::ASR, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, Instr::ASR, 0b001110000000, Word, ShiftEa, CIMSloop)


    // Bcc
    //
    //       Syntax: Bcc <label>
    //         Size: Word

    // Dn,<label>

    opcode = parse("0110 ---- ---- ----");
    ________________(opcode | 0x000, Instr::BRA, Mode::IP, Word, Bra, CIMS)
    ________________(opcode | 0x200, Instr::BHI, Mode::IP, Word, Bcc, CIMS)
    ________________(opcode | 0x300, Instr::BLS, Mode::IP, Word, Bcc, CIMS)
    ________________(opcode | 0x400, Instr::BCC, Mode::IP, Word, Bcc, CIMS)
    ________________(opcode | 0x500, Instr::BCS, Mode::IP, Word, Bcc, CIMS)
    ________________(opcode | 0x600, Instr::BNE, Mode::IP, Word, Bcc, CIMS)
    ________________(opcode | 0x700, Instr::BEQ, Mode::IP, Word, Bcc, CIMS)
    ________________(opcode | 0x800, Instr::BVC, Mode::IP, Word, Bcc, CIMS)
    ________________(opcode | 0x900, Instr::BVS, Mode::IP, Word, Bcc, CIMS)
    ________________(opcode | 0xA00, Instr::BPL, Mode::IP, Word, Bcc, CIMS)
    ________________(opcode | 0xB00, Instr::BMI, Mode::IP, Word, Bcc, CIMS)
    ________________(opcode | 0xC00, Instr::BGE, Mode::IP, Word, Bcc, CIMS)
    ________________(opcode | 0xD00, Instr::BLT, Mode::IP, Word, Bcc, CIMS)
    ________________(opcode | 0xE00, Instr::BGT, Mode::IP, Word, Bcc, CIMS)
    ________________(opcode | 0xF00, Instr::BLE, Mode::IP, Word, Bcc, CIMS)

    for (int i = 1; i <= 0xFF; i++) {

        ________________(opcode | 0x000 | i, Instr::BRA, Mode::IP, Byte, Bra, CIMS)
        ________________(opcode | 0x200 | i, Instr::BHI, Mode::IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x300 | i, Instr::BLS, Mode::IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x400 | i, Instr::BCC, Mode::IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x500 | i, Instr::BCS, Mode::IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x600 | i, Instr::BNE, Mode::IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x700 | i, Instr::BEQ, Mode::IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x800 | i, Instr::BVC, Mode::IP, Byte, Bcc, CIMS)
        ________________(opcode | 0x900 | i, Instr::BVS, Mode::IP, Byte, Bcc, CIMS)
        ________________(opcode | 0xA00 | i, Instr::BPL, Mode::IP, Byte, Bcc, CIMS)
        ________________(opcode | 0xB00 | i, Instr::BMI, Mode::IP, Byte, Bcc, CIMS)
        ________________(opcode | 0xC00 | i, Instr::BGE, Mode::IP, Byte, Bcc, CIMS)
        ________________(opcode | 0xD00 | i, Instr::BLT, Mode::IP, Byte, Bcc, CIMS)
        ________________(opcode | 0xE00 | i, Instr::BGT, Mode::IP, Byte, Bcc, CIMS)
        ________________(opcode | 0xF00 | i, Instr::BLE, Mode::IP, Byte, Bcc, CIMS)
    }

    if constexpr (C >= Core::C68020) {

        ________________(opcode | 0x0FF, Instr::BRA, Mode::IP, Long, Bra, CIMS)
        ________________(opcode | 0x2FF, Instr::BHI, Mode::IP, Long, Bcc, CIMS)
        ________________(opcode | 0x3FF, Instr::BLS, Mode::IP, Long, Bcc, CIMS)
        ________________(opcode | 0x4FF, Instr::BCC, Mode::IP, Long, Bcc, CIMS)
        ________________(opcode | 0x5FF, Instr::BCS, Mode::IP, Long, Bcc, CIMS)
        ________________(opcode | 0x6FF, Instr::BNE, Mode::IP, Long, Bcc, CIMS)
        ________________(opcode | 0x7FF, Instr::BEQ, Mode::IP, Long, Bcc, CIMS)
        ________________(opcode | 0x8FF, Instr::BVC, Mode::IP, Long, Bcc, CIMS)
        ________________(opcode | 0x9FF, Instr::BVS, Mode::IP, Long, Bcc, CIMS)
        ________________(opcode | 0xAFF, Instr::BPL, Mode::IP, Long, Bcc, CIMS)
        ________________(opcode | 0xBFF, Instr::BMI, Mode::IP, Long, Bcc, CIMS)
        ________________(opcode | 0xCFF, Instr::BGE, Mode::IP, Long, Bcc, CIMS)
        ________________(opcode | 0xDFF, Instr::BLT, Mode::IP, Long, Bcc, CIMS)
        ________________(opcode | 0xEFF, Instr::BGT, Mode::IP, Long, Bcc, CIMS)
        ________________(opcode | 0xFFF, Instr::BLE, Mode::IP, Long, Bcc, CIMS)
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
        __________MMMXXX(opcode, Instr::BFCHG, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, Instr::BFCHG, 0b001001111000, Long, BitFieldEa, CIMS)

        opcode = parse("1110 1100 11-- ----");
        __________MMMXXX(opcode, Instr::BFCLR, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, Instr::BFCLR, 0b001001111000, Long, BitFieldEa, CIMS)

        opcode = parse("1110 1111 11-- ----");
        __________MMMXXX(opcode, Instr::BFINS, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, Instr::BFINS, 0b001001111000, Long, BitFieldEa, CIMS)

        opcode = parse("1110 1110 11-- ----");
        __________MMMXXX(opcode, Instr::BFSET, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, Instr::BFSET, 0b001001111000, Long, BitFieldEa, CIMS)
    }


    // BFEXTS, Instr::BFEXTU, Instr::BFFFO, Instr::BFTST
    //
    //       Syntax: BFxxx <ea> {offset:width}
    //         Size: Unsized

    //               -------------------------------------------------
    // <ea> {:}      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X           X   X   X   X   X   X   X

    if constexpr (C >= Core::C68020) {

        opcode = parse("1110 1011 11-- ----");
        __________MMMXXX(opcode, Instr::BFEXTS, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, Instr::BFEXTS, 0b001001111110, Long, BitFieldEa, CIMS)

        opcode = parse("1110 1001 11-- ----");
        __________MMMXXX(opcode, Instr::BFEXTU, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, Instr::BFEXTU, 0b001001111110, Long, BitFieldEa, CIMS)

        opcode = parse("1110 1101 11-- ----");
        __________MMMXXX(opcode, Instr::BFFFO, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, Instr::BFFFO, 0b001001111110, Long, BitFieldEa, CIMS)

        opcode = parse("1110 1000 11-- ----");
        __________MMMXXX(opcode, Instr::BFTST, 0b100000000000, Long, BitFieldDn, CIMS)
        __________MMMXXX(opcode, Instr::BFTST, 0b001001111110, Long, BitFieldEa, CIMS)
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
    ____XXX___MMMXXX(opcode, Instr::BCHG, 0b100000000000, Byte, BitDxDy, CIMS)
    ____XXX___MMMXXX(opcode, Instr::BCHG, 0b001111111000, Byte, BitDxEa, CIMS)

    opcode = parse("0000 ---1 10-- ----");
    ____XXX___MMMXXX(opcode, Instr::BCLR, 0b100000000000, Byte, BitDxDy, CIMS)
    ____XXX___MMMXXX(opcode, Instr::BCLR, 0b001111111000, Byte, BitDxEa, CIMS)

    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 1000 01-- ----");
    __________MMMXXX(opcode, Instr::BCHG, 0b100000000000, Byte, BitImDy, CIMS)
    __________MMMXXX(opcode, Instr::BCHG, 0b001111111000, Byte, BitImEa, CIMS)

    opcode = parse("0000 1000 10-- ----");
    __________MMMXXX(opcode, Instr::BCLR, 0b100000000000, Byte, BitImDy, CIMS)
    __________MMMXXX(opcode, Instr::BCLR, 0b001111111000, Byte, BitImEa, CIMS)


    // BKPT (68010+)
    //
    //       Syntax: BKPT #<vector>
    //        Sizes: Unsized

    if constexpr (C >= Core::C68010) {

        opcode = parse("0100 1000 0100 1---");
        _____________XXX(opcode, Instr::BKPT, Mode::IP, Long, Bkpt, CIMS)
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
    ____XXX___MMMXXX(opcode, Instr::BSET, 0b100000000000, Byte, BitDxDy, CIMS)
    ____XXX___MMMXXX(opcode, Instr::BSET, 0b001111111000, Byte, BitDxEa, CIMS)

    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 1000 11-- ----");
    __________MMMXXX(opcode, Instr::BSET, 0b100000000000, Byte, BitImDy, CIMS)
    __________MMMXXX(opcode, Instr::BSET, 0b001111111000, Byte, BitImEa, CIMS)


    // BSR
    //
    //       Syntax: (1) BSR <label>
    //         Size: Byte, Word

    opcode = parse("0110 0001 ---- ----");
    ________________(opcode, Instr::BSR, Mode::IP, Word, Bsr, CIMS)

    for (int i = 1; i <= 0xFF; i++) {
        ________________(opcode | i, Instr::BSR, Mode::IP, Byte, Bsr, CIMS)
    }

    if constexpr (C >= Core::C68020) {
        ________________(opcode | 0xFF, Instr::BSR, Mode::IP, Long, Bsr, CIMS)
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
    ____XXX___MMMXXX(opcode, Instr::BTST, 0b100000000000, Byte, BitDxDy, CIMS)
    ____XXX___MMMXXX(opcode, Instr::BTST, 0b001111111111, Byte, BitDxEa, CIMS)

    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X

    opcode = parse("0000 1000 00-- ----");
    __________MMMXXX(opcode, Instr::BTST, 0b100000000000, Byte, BitImDy, CIMS)
    __________MMMXXX(opcode, Instr::BTST, 0b001111111110, Byte, BitImEa, CIMS)


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
            __________MMMXXX(opcode, Instr::CALLM, 0b001001111110, Long, Callm, CIMS)
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
        __________MMMXXX(opcode, Instr::CAS, 0b001111111000, Byte, Cas, CIMS)
        opcode = parse("0000 1100 11-- ----");
        __________MMMXXX(opcode, Instr::CAS, 0b001111111000, Word, Cas, CIMS)
        opcode = parse("0000 1110 11-- ----");
        __________MMMXXX(opcode, Instr::CAS, 0b001111111000, Long, Cas, CIMS)

        // CAS2
        opcode = parse("0000 1100 1111 1100");
        ________________(opcode, Instr::CAS2, Mode::IM, Word, Cas2, CIMS)

        opcode = parse("0000 1110 1111 1100");
        ________________(opcode, Instr::CAS2, Mode::IM, Long, Cas2, CIMS)
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
    ____XXX___MMMXXX(opcode, Instr::CHK, 0b101111111111, Word, Chk, CIMS)

    if constexpr (C >= Core::C68020) {

        opcode = parse("0100 ---1 00-- ----");
        ____XXX___MMMXXX(opcode, Instr::CHK, 0b101111111111, Long, Chk, CIMS)
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
        __________MMMXXX(opcode, Instr::CHK2, 0b001001111110, Byte, ChkCmp2, CIMS)

        opcode = parse("0000 0010 11-- ----");
        __________MMMXXX(opcode, Instr::CHK2, 0b001001111110, Word, ChkCmp2, CIMS)

        opcode = parse("0000 0100 11-- ----");
        __________MMMXXX(opcode, Instr::CHK2, 0b001001111110, Long, ChkCmp2, CIMS)
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
    ________SSMMMXXX(opcode, Instr::CLR, 0b101111111000, Byte | Word | Long, Clr, CIMS)
    ________SSMMMXXX(opcode, Instr::CLR, 0b001110000000, Byte | Word | Long, Clr, CIMSloop)


    // CMP
    //
    //       Syntax: CMP <ea>,Dy
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)  X   X   X   X   X   X   X   X   X   X

    opcode = parse("1011 ---0 ---- ----");
    ____XXX_SSMMMXXX(opcode, Instr::CMP, 0b101111111111, Byte,        Cmp, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::CMP, 0b111111111111, Word | Long, Cmp, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::CMP, 0b001110000000, Byte | Word | Long, Cmp, CIMSloop)


    // CMPA
    //
    //       Syntax: CMPA <ea>,Ay
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X   X   X   X   X   X   X   X   X   X   X   X

    opcode = parse("1011 ---- 11-- ----");
    ____XXXS__MMMXXX(opcode, Instr::CMPA, 0b111111111111, Word | Long, Cmpa, CIMS)
    ____XXXS__MMMXXX(opcode, Instr::CMPA, 0b001110000000, Word | Long, Cmpa, CIMSloop)


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
    ________SSMMMXXX(opcode, Instr::CMPI, 0b100000000000, Byte | Word | Long, CmpiRg, CIMS)
    ________SSMMMXXX(opcode, Instr::CMPI, 0b001111111000, Byte | Word | Long, CmpiEa, CIMS)

    if constexpr (C >= Core::C68010) {

        ________SSMMMXXX(opcode, Instr::CMPI, 0b000000000110, Byte | Word | Long, CmpiEa, CIMS)
    }


    // CMPM
    //
    //       Syntax: (1) CMPM (Ax)+,(Ay)+
    //         Size: Byte, Word, Longword

    // (Ax)+,(Ay)+
    opcode = parse("1011 ---1 --00 1---");
    ____XXX_SS___XXX(opcode, Instr::CMPM, Mode::PI, Byte | Word | Long, Cmpm, CIMS)
    ____XXX_SS___XXX(opcode, Instr::CMPM, Mode::PI, Byte | Word | Long, Cmpm, CIMSloop)


    // DBcc
    //
    //       Syntax: DBcc Dn,<label>
    //         Size: Word

    // Dn,<label>
    opcode = parse("0101 ---- 1100 1---");
    _____________XXX(opcode | 0x000, Instr::DBT,  Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x100, Instr::DBF,  Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x200, Instr::DBHI, Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x300, Instr::DBLS, Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x400, Instr::DBCC, Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x500, Instr::DBCS, Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x600, Instr::DBNE, Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x700, Instr::DBEQ, Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x800, Instr::DBVC, Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0x900, Instr::DBVS, Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0xA00, Instr::DBPL, Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0xB00, Instr::DBMI, Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0xC00, Instr::DBGE, Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0xD00, Instr::DBLT, Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0xE00, Instr::DBGT, Mode::IP, Word, Dbcc, CIMS)
    _____________XXX(opcode | 0xF00, Instr::DBLE, Mode::IP, Word, Dbcc, CIMS)

    _____________XXX(opcode | 0x000, Instr::DBT,  Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x100, Instr::DBF,  Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x200, Instr::DBHI, Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x300, Instr::DBLS, Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x400, Instr::DBCC, Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x500, Instr::DBCS, Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x600, Instr::DBNE, Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x700, Instr::DBEQ, Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x800, Instr::DBVC, Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0x900, Instr::DBVS, Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0xA00, Instr::DBPL, Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0xB00, Instr::DBMI, Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0xC00, Instr::DBGE, Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0xD00, Instr::DBLT, Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0xE00, Instr::DBGT, Mode::IP, Word, Dbcc, CIMSloop)
    _____________XXX(opcode | 0xF00, Instr::DBLE, Mode::IP, Word, Dbcc, CIMSloop)


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
    ____XXX___MMMXXX(opcode, Instr::DIVS, 0b101111111111, Word, Divs, CIMS)

    opcode = parse("1000 ---0 11-- ----");
    ____XXX___MMMXXX(opcode, Instr::DIVU, 0b101111111111, Word, Divu, CIMS)

    if constexpr (C >= Core::C68020) {

        opcode = parse("0100 1100 01-- ----");
        __________MMMXXX(opcode, Instr::DIVL, 0b101111111111, Long, Divl, CIMS)
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
    ____XXX_SSMMMXXX(opcode, Instr::EOR, 0b101111111000, Byte | Word | Long, AndRgEa, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::EOR, 0b001110000000, Byte | Word | Long, AndRgEa, CIMSloop)


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
    ________SSMMMXXX(opcode, Instr::EORI, 0b100000000000, Byte | Word | Long, AndiRg, CIMS)
    ________SSMMMXXX(opcode, Instr::EORI, 0b001111111000, Byte | Word | Long, AndiEa, CIMS)


    // EORI to CCR
    //
    //       Syntax: EORI #<data>,CCR
    //         Size: Byte
    //

    opcode = parse("0000 1010 0011 1100");
    ________________(opcode, Instr::EORICCR, Mode::IM, Byte, Andiccr, CIMS)


    // EORI to SR
    //
    //       Syntax: EORI #<data>,SR
    //         Size: Byte
    //

    opcode = parse("0000 1010 0111 1100");
    ________________(opcode, Instr::EORISR, Mode::IM, Word, Andisr, CIMS)


    // EXG
    //
    //       Syntax: EXG Dx,Dy
    //               EXG Ax,Dy
    //               EXG Ax,Ay
    //         Size: Longword

    opcode = parse("1100 ---1 0100 0---");
    ____XXX______XXX(opcode, Instr::EXG, Mode::IP, Long, ExgDxDy, CIMS)

    opcode = parse("1100 ---1 1000 1---");
    ____XXX______XXX(opcode, Instr::EXG, Mode::IP, Long, ExgAxDy, CIMS)

    opcode = parse("1100 ---1 0100 1---");
    ____XXX______XXX(opcode, Instr::EXG, Mode::IP, Long, ExgAxAy, CIMS)


    // EXT
    //
    //       Syntax: EXT Dx
    //        Sizes: Word, Longword

    opcode = parse("0100 1000 --00 0---");
    _____________XXX(opcode | 2 << 6, Instr::EXT, Mode::DN, Word, Ext, CIMS)
    _____________XXX(opcode | 3 << 6, Instr::EXT, Mode::DN, Long, Ext, CIMS)


    // EXTB
    //
    //       Syntax: EXTB Dx
    //        Sizes: Longword

    if constexpr (C >= Core::C68020) {

        opcode = parse("0100 1001 --00 0---");
        _____________XXX(opcode | 3 << 6, Instr::EXTB, Mode::DN, Long, Extb, CIMS)
    }


    // LINK
    //
    //       Syntax: LINK An,#<displacement>
    //        Sizes: Word, (Longword)

    opcode = parse("0100 1110 0101 0---");
    _____________XXX(opcode, Instr::LINK, Mode::IP, Word, Link, CIMS)

    if constexpr (C >= Core::C68020) {

        opcode = parse("0100 1000 0000 1---");
        _____________XXX(opcode, Instr::LINK, Mode::IP, Long, Link, CIMS)
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
    __________MMMXXX(opcode, Instr::JMP, 0b001001111110, Long, Jmp, CIMS)


    // JSR
    //
    //       Syntax: JSR <ea>
    //        Sizes: Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X           X   X   X   X   X   X

    opcode = parse("0100 1110 10-- ----");
    __________MMMXXX(opcode, Instr::JSR, 0b001001111110, Long, Jsr, CIMS)


    // LEA
    //
    //       Syntax: LEA <ea>,Ay
    //        Sizes: Longword

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X           X   X   X   X   X   X

    opcode = parse("0100 ---1 11-- ----");
    ____XXX___MMMXXX(opcode, Instr::LEA, 0b001001111110, Long, Lea, CIMS)


    // LSL, LSR
    //
    //       Syntax: (1) LSx Dx,Dy
    //               (2) LSx #<data>,Dy
    //               (3) LSx <ea>
    //        Sizes: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1110 ---1 --10 1---");
    ____XXX_SS___XXX(opcode, Instr::LSL, Mode::DN, Byte | Word | Long, ShiftRg, CIMS)

    opcode = parse("1110 ---0 --10 1---");
    ____XXX_SS___XXX(opcode, Instr::LSR, Mode::DN, Byte | Word | Long, ShiftRg, CIMS)

    // #<data>,Dy
    opcode = parse("1110 ---1 --00 1---");
    ____XXX_SS___XXX(opcode, Instr::LSL, Mode::IM, Byte | Word | Long, ShiftIm, CIMS)

    opcode = parse("1110 ---0 --00 1---");
    ____XXX_SS___XXX(opcode, Instr::LSR, Mode::IM, Byte | Word | Long, ShiftIm, CIMS)

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1110 0011 11-- ----");
    __________MMMXXX(opcode, Instr::LSL, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, Instr::LSL, 0b001110000000, Word, ShiftEa, CIMSloop)

    opcode = parse("1110 0010 11-- ----");
    __________MMMXXX(opcode, Instr::LSR, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, Instr::LSR, 0b001110000000, Word, ShiftEa, CIMSloop)


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
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b101111111111, Byte,        Move0, CIMS)
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b111111111111, Word | Long, Move0, CIMS)

    // <ea>,(Ay)
    opcode = parse("00-- ---0 10-- ----");
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b101111111111, Byte,        Move2, CIMS)
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b111111111111, Word | Long, Move2, CIMS)
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b101110000000, Byte,        Move2, CIMSloop)
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b111110000000, Word | Long, Move2, CIMSloop)

    // <ea>,(Ay)+
    opcode = parse("00-- ---0 11-- ----");
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b101111111111, Byte,        Move3, CIMS)
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b111111111111, Word | Long, Move3, CIMS)
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b101110000000, Byte,        Move3, CIMSloop)
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b111110000000, Word | Long, Move3, CIMSloop)

    // <ea>,-(Ay)
    opcode = parse("00-- ---1 00-- ----");
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b101111111111, Byte,        Move4, CIMS)
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b111111111111, Word | Long, Move4, CIMS)
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b101110000000, Byte,        Move4, CIMSloop)
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b111110000000, Word | Long, Move4, CIMSloop)

    // <ea>,(d,Ay)
    opcode = parse("00-- ---1 01-- ----");
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b101111111111, Byte,        Move5, CIMS)
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b111111111111, Word | Long, Move5, CIMS)

    // <ea>,(d,Ay,Xi)
    opcode = parse("00-- ---1 10-- ----");
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b101111111111, Byte,        Move6, CIMS)
    __SSXXX___MMMXXX(opcode, Instr::MOVE, 0b111111111111, Word | Long, Move6, CIMS)

    // <ea>,ABS.w
    opcode = parse("00-- 0001 11-- ----");
    __SS______MMMXXX(opcode, Instr::MOVE, 0b101111111111, Byte,        Move7, CIMS)
    __SS______MMMXXX(opcode, Instr::MOVE, 0b111111111111, Word | Long, Move7, CIMS)

    // <ea>,ABS.l
    opcode = parse("00-- 0011 11-- ----");
    __SS______MMMXXX(opcode, Instr::MOVE, 0b101111111111, Byte,        Move8, CIMS)
    __SS______MMMXXX(opcode, Instr::MOVE, 0b111111111111, Word | Long, Move8, CIMS)


    // MOVEA
    //
    //       Syntax: MOVEA <ea>,Ay
    //        Sizes: Word, Longword

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X   X   X   X   X   X   X   X   X   X   X   X

    opcode = parse("001- ---0 01-- ----");

    ____XXX___MMMXXX(opcode | 0 << 12, Instr::MOVEA, 0b111111111111, Long, Movea, CIMS)
    ____XXX___MMMXXX(opcode | 1 << 12, Instr::MOVEA, 0b111111111111, Word, Movea, CIMS)


    // MOVEC
    //
    //       Syntax: MOVEC Rc,Rx
    //               MOVEC Rx,Rc
    //        Sizes: Longword

    if constexpr (C >= Core::C68010) {

        opcode = parse("0100 1110 0111 101-");
        ________________(opcode | 0, Instr::MOVEC, Mode::IP, Long, MovecRcRx, CIMS)
        ________________(opcode | 1, Instr::MOVEC, Mode::IP, Long, MovecRxRc, CIMS)
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
    __________MMMXXX(opcode | 0 << 6, Instr::MOVEM, 0b001101111110, Word, MovemEaRg, CIMS)
    __________MMMXXX(opcode | 1 << 6, Instr::MOVEM, 0b001101111110, Long, MovemEaRg, CIMS)

    opcode = parse("0100 1000 1--- ----");
    __________MMMXXX(opcode | 0 << 6, Instr::MOVEM, 0b001011111000, Word, MovemRgEa, CIMS)
    __________MMMXXX(opcode | 1 << 6, Instr::MOVEM, 0b001011111000, Long, MovemRgEa, CIMS)


    // MOVEP
    //
    //       Syntax: MOVEP Dx,(d,Ay)
    //               MOVEP (d,Ay),Dx
    //         Size: Word, Longword

    // MOVEP Dx,(d,Ay)
    opcode = parse("0000 ---1 1-00 1---");
    ____XXX______XXX(opcode | 0 << 6, Instr::MOVEP, Mode::DI, Word, MovepDxEa, CIMS)
    ____XXX______XXX(opcode | 1 << 6, Instr::MOVEP, Mode::DI, Long, MovepDxEa, CIMS)

    // MOVEP (d,Ay),Dx
    opcode = parse("0000 ---1 0-00 1---");
    ____XXX______XXX(opcode | 0 << 6, Instr::MOVEP, Mode::DI, Word, MovepEaDx, CIMS)
    ____XXX______XXX(opcode | 1 << 6, Instr::MOVEP, Mode::DI, Long, MovepEaDx, CIMS)


    // MOVEQ
    //
    //       Syntax: MOVEQ #<data>,Dn
    //        Sizes: Longword

    // #<data>,Dn
    opcode = parse("0111 ---0 ---- ----");
    ____XXX_XXXXXXXX(opcode, Instr::MOVEQ, Mode::IM, Long, Moveq, CIMS)


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
        ________SSMMMXXX(opcode, Instr::MOVES, 0b001111111000, Byte | Word | Long, Moves, CIMS)
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
        __________MMMXXX(opcode, Instr::MOVEFCCR, 0b100000000000, Word, MoveCcrRg, CIMS)
        __________MMMXXX(opcode, Instr::MOVEFCCR, 0b001111111000, Word, MoveCcrEa, CIMS)
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
    __________MMMXXX(opcode, Instr::MOVETCCR, 0b101111111111, Word, MoveToCcr, CIMS)


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

    __________MMMXXX(opcode, Instr::MOVEFSR, 0b100000000000, Word, MoveSrRg, CIMS)
    __________MMMXXX(opcode, Instr::MOVEFSR, 0b001111111000, Word, MoveSrEa, CIMS)


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
    __________MMMXXX(opcode, Instr::MOVETSR, 0b101111111111, Word, MoveToSr, CIMS)


    // MOVEUSP
    //
    //       Syntax: MOVE USP,An
    //               MOVE An,USP
    //        Sizes: Longword

    opcode = parse("0100 1110 0110 ----");
    _____________XXX(opcode | 1 << 3, Instr::MOVEUSP, Mode::IP, Long, MoveUspAn, CIMS)
    _____________XXX(opcode | 0 << 3, Instr::MOVEUSP, Mode::IP, Long, MoveAnUsp, CIMS)


    // MULS, MULU
    //
    //       Syntax: MULx <ea>,Dy
    //        Sizes: Word x Word -> Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("1100 ---1 11-- ----");
    ____XXX___MMMXXX(opcode, Instr::MULS, 0b101111111111, Word, Muls, CIMS)

    opcode = parse("1100 ---0 11-- ----");
    ____XXX___MMMXXX(opcode, Instr::MULU, 0b101111111111, Word, Mulu, CIMS)

    if constexpr (C >= Core::C68020) {

        opcode = parse("0100 1100 00-- ----");
        __________MMMXXX(opcode, Instr::MULL, 0b101111111111, Long, Mull, CIMS)
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
    __________MMMXXX(opcode, Instr::NBCD, 0b100000000000, Byte, NbcdRg, CIMS)
    __________MMMXXX(opcode, Instr::NBCD, 0b001111111000, Byte, NbcdEa, CIMS)
    __________MMMXXX(opcode, Instr::NBCD, 0b001110000000, Byte, NbcdEa, CIMSloop)


    // NEG, NEGX, NOT
    //
    //       Syntax: Nxx <ea>
    //        Sizes: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0100 0100 ---- ----");
    ________SSMMMXXX(opcode, Instr::NEG, 0b100000000000, Byte | Word | Long, NegRg, CIMS)
    ________SSMMMXXX(opcode, Instr::NEG, 0b001111111000, Byte | Word | Long, NegEa, CIMS)
    ________SSMMMXXX(opcode, Instr::NEG, 0b001110000000, Byte | Word | Long, NegEa, CIMSloop)

    opcode = parse("0100 0000 ---- ----");
    ________SSMMMXXX(opcode, Instr::NEGX, 0b100000000000, Byte | Word | Long, NegRg, CIMS)
    ________SSMMMXXX(opcode, Instr::NEGX, 0b001111111000, Byte | Word | Long, NegEa, CIMS)
    ________SSMMMXXX(opcode, Instr::NEGX, 0b001110000000, Byte | Word | Long, NegEa, CIMSloop)

    opcode = parse("0100 0110 ---- ----");
    ________SSMMMXXX(opcode, Instr::NOT, 0b100000000000, Byte | Word | Long, NegRg, CIMS)
    ________SSMMMXXX(opcode, Instr::NOT, 0b001111111000, Byte | Word | Long, NegEa, CIMS)
    ________SSMMMXXX(opcode, Instr::NOT, 0b001110000000, Byte | Word | Long, NegEa, CIMSloop)


    // NOP
    //
    //       Syntax: NOP
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0001");
    ________________(opcode, Instr::NOP, Mode::IP, Long, Nop, CIMS)


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
    ____XXX_SSMMMXXX(opcode, Instr::OR, 0b101111111111, Byte | Word | Long, AndEaRg, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::OR, 0b001110000000, Byte | Word | Long, AndEaRg, CIMSloop)


    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("1000 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, Instr::OR, 0b001111111000, Byte | Word | Long, AndRgEa, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::OR, 0b001110000000, Byte | Word | Long, AndRgEa, CIMSloop)


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
    ________SSMMMXXX(opcode, Instr::ORI, 0b100000000000, Byte | Word | Long, AndiRg, CIMS)
    ________SSMMMXXX(opcode, Instr::ORI, 0b001111111000, Byte | Word | Long, AndiEa, CIMS)


    // ORI to CCR
    //
    //       Syntax: ORI #<data>,CCR
    //         Size: Byte
    //

    opcode = parse("0000 0000 0011 1100");
    ________________(opcode, Instr::ORICCR, Mode::IM, Byte, Andiccr, CIMS)


    // ORI to SR
    //
    //       Syntax: ORI #<data>,SR
    //         Size: Byte
    //

    opcode = parse("0000 0000 0111 1100");
    ________________(opcode, Instr::ORISR, Mode::IM, Word, Andisr, CIMS)


    // ROL, ROR, ROXL, ROXR
    //
    //       Syntax: (1) ROxx Dx,Dy
    //               (2) ROxx #<data>,Dy
    //               (3) ROxx <ea>
    //        Sizes: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1110 ---1 --11 1---");
    ____XXX_SS___XXX(opcode, Instr::ROL, Mode::DN, Byte | Word | Long, ShiftRg, CIMS)

    opcode = parse("1110 ---0 --11 1---");
    ____XXX_SS___XXX(opcode, Instr::ROR, Mode::DN, Byte | Word | Long, ShiftRg, CIMS)

    opcode = parse("1110 ---1 --11 0---");
    ____XXX_SS___XXX(opcode, Instr::ROXL, Mode::DN, Byte | Word | Long, ShiftRg, CIMS)

    opcode = parse("1110 ---0 --11 0---");
    ____XXX_SS___XXX(opcode, Instr::ROXR, Mode::DN, Byte | Word | Long, ShiftRg, CIMS)

    // #<data>,Dy
    opcode = parse("1110 ---1 --01 1---");
    ____XXX_SS___XXX(opcode, Instr::ROL, Mode::IM, Byte | Word | Long, ShiftIm, CIMS)

    opcode = parse("1110 ---0 --01 1---");
    ____XXX_SS___XXX(opcode, Instr::ROR, Mode::IM, Byte | Word | Long, ShiftIm, CIMS)

    opcode = parse("1110 ---1 --01 0---");
    ____XXX_SS___XXX(opcode, Instr::ROXL, Mode::IM, Byte | Word | Long, ShiftIm, CIMS)

    opcode = parse("1110 ---0 --01 0---");
    ____XXX_SS___XXX(opcode, Instr::ROXR, Mode::IM, Byte | Word | Long, ShiftIm, CIMS)

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1110 0111 11-- ----");
    __________MMMXXX(opcode, Instr::ROL, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, Instr::ROL, 0b001110000000, Word, ShiftEa, CIMSloop)

    opcode = parse("1110 0110 11-- ----");
    __________MMMXXX(opcode, Instr::ROR, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, Instr::ROR, 0b001110000000, Word, ShiftEa, CIMSloop)

    opcode = parse("1110 0101 11-- ----");
    __________MMMXXX(opcode, Instr::ROXL, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, Instr::ROXL, 0b001110000000, Word, ShiftEa, CIMSloop)

    opcode = parse("1110 0100 11-- ----");
    __________MMMXXX(opcode, Instr::ROXR, 0b001111111000, Word, ShiftEa, CIMS)
    __________MMMXXX(opcode, Instr::ROXR, 0b001110000000, Word, ShiftEa, CIMSloop)


    // PACK
    //
    //       Syntax: PACK -(Ax),-(Ay),#<adjustment>
    //               PACK DX,Dy,#<adjustment>
    //        Sizes: Unsized

    if constexpr (C >= Core::C68020) {

        opcode = parse("1000 ---1 0100 0---");
        ____XXX______XXX(opcode, Instr::PACK, Mode::DN, Word, PackDn, CIMS)

        opcode = parse("1000 ---1 0100 1---");
        ____XXX______XXX(opcode, Instr::PACK, Mode::PD, Word, PackPd, CIMS)
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
    __________MMMXXX(opcode, Instr::PEA, 0b001001111110, Long, Pea, CIMS)


    // RESET
    //
    //       Syntax: RESET
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0000");
    ________________(opcode, Instr::RESET, Mode::IP, Long, Reset, CIMS)


    // RTD
    //
    //       Syntax: RTD
    //        Sizes: Unsized

    if constexpr (C >= Core::C68010) {

        opcode = parse("0100 1110 0111 0100");
        ________________(opcode, Instr::RTD, Mode::IP, Long, Rtd, CIMS)
    }


    // RTE
    //
    //       Syntax: RTE
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0011");
    ________________(opcode, Instr::RTE, Mode::IP, Long, Rte, CIMS)


    // RTM
    //
    //       Syntax: RTM Rn
    //        Sizes: Unsized

    if constexpr (C >= Core::C68020) {

        if (model == Model::M68EC020 || model == Model::M68020) {

            opcode = parse("0000 0110 1100 ----");
            ____________XXXX(opcode, Instr::RTM, Mode::IP, Long, Rtm, CIMS)
        }
    }


    // RTR
    //
    //       Syntax: RTR
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0111");
    ________________(opcode, Instr::RTR, Mode::IP, Long, Rtr, CIMS)


    // RTS
    //
    //       Syntax: RTS
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0101");
    ________________(opcode, Instr::RTS, Mode::IP, Long, Rts, CIMS)


    // SBCD
    //
    //       Syntax: (1) SBCD Dx,Dy
    //               (2) SBCD -(Ax),-(Ay)
    //         Size: Byte

    // Dx,Dy
    opcode = parse("1000 ---1 0000 0---");
    ____XXX______XXX(opcode, Instr::SBCD, Mode::DN, Byte, AbcdRg, CIMS)

    // -(Ax),-(Ay)
    opcode = parse("1000 ---1 0000 1---");
    ____XXX______XXX(opcode, Instr::SBCD, Mode::PD, Byte, AbcdEa, CIMS)
    ____XXX______XXX(opcode, Instr::SBCD, Mode::PD, Byte, AbcdEa, CIMSloop)


    // Scc
    //
    //       Syntax: Scc <ea>
    //         Size: Word

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0101 ---- 11-- ----");
    __________MMMXXX(opcode | 0x000, Instr::ST,  0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x100, Instr::SF,  0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x200, Instr::SHI, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x300, Instr::SLS, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x400, Instr::SCC, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x500, Instr::SCS, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x600, Instr::SNE, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x700, Instr::SEQ, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x800, Instr::SVC, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0x900, Instr::SVS, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0xA00, Instr::SPL, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0xB00, Instr::SMI, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0xC00, Instr::SGE, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0xD00, Instr::SLT, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0xE00, Instr::SGT, 0b100000000000, Byte, SccRg, CIMS)
    __________MMMXXX(opcode | 0xF00, Instr::SLE, 0b100000000000, Byte, SccRg, CIMS)

    __________MMMXXX(opcode | 0x000, Instr::ST,  0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x100, Instr::SF,  0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x200, Instr::SHI, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x300, Instr::SLS, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x400, Instr::SCC, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x500, Instr::SCS, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x600, Instr::SNE, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x700, Instr::SEQ, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x800, Instr::SVC, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0x900, Instr::SVS, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0xA00, Instr::SPL, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0xB00, Instr::SMI, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0xC00, Instr::SGE, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0xD00, Instr::SLT, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0xE00, Instr::SGT, 0b001111111000, Byte, SccEa, CIMS)
    __________MMMXXX(opcode | 0xF00, Instr::SLE, 0b001111111000, Byte, SccEa, CIMS)

    // STOP
    //
    //       Syntax: STOP
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0010");
    ________________(opcode, Instr::STOP, Mode::IP, Word, Stop, CIMS)


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
    ____XXX_SSMMMXXX(opcode, Instr::SUB, 0b101111111111, Byte,        AddEaRg, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::SUB, 0b111111111111, Word | Long, AddEaRg, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::SUB, 0b001110000000, Byte,        AddEaRg, CIMSloop)
    ____XXX_SSMMMXXX(opcode, Instr::SUB, 0b001110000000, Word | Long, AddEaRg, CIMSloop)


    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1001 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, Instr::SUB, 0b001111111000, Byte | Word | Long, AddRgEa, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::SUB, 0b001110000000, Byte | Word | Long, AddRgEa, CIMSloop)


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
    ____XXXS__MMMXXX(opcode, Instr::SUBA, 0b111111111111, Word | Long, Adda, CIMS)
    ____XXXS__MMMXXX(opcode, Instr::SUBA, 0b001110000000, Word | Long, Adda, CIMSloop)


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
    ________SSMMMXXX(opcode, Instr::SUBI, 0b100000000000, Byte | Word | Long, AddiRg, CIMS)
    ________SSMMMXXX(opcode, Instr::SUBI, 0b001111111000, Byte | Word | Long, AddiEa, CIMS)


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
    ____XXX_SSMMMXXX(opcode, Instr::SUBQ, 0b100000000000, Byte | Word | Long, AddqDn, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::SUBQ, 0b010000000000, Word | Long,        AddqAn, CIMS)
    ____XXX_SSMMMXXX(opcode, Instr::SUBQ, 0b001111111000, Byte | Word | Long, AddqEa, CIMS)


    // SUBX
    //
    //       Syntax: (1) SUBX Dx,Dy
    //               (2) SUBX -(Ax),-(Ay)
    //         Size: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1001 ---1 --00 0---");
    ____XXX_SS___XXX(opcode, Instr::SUBX, Mode::DN, Byte | Word | Long, AddxRg, CIMS)

    // -(Ax),-(Ay)
    opcode = parse("1001 ---1 --00 1---");
    ____XXX_SS___XXX(opcode, Instr::SUBX, Mode::PD, Byte | Word | Long, AddxEa, CIMS)
    ____XXX_SS___XXX(opcode, Instr::SUBX, Mode::PD, Byte | Word | Long, AddxEa, CIMSloop)


    // SWAP
    //
    //       Syntax: SWAP Dn
    //         Size: Word

    opcode = parse("0100 1000 0100 0---");
    _____________XXX(opcode, Instr::SWAP, Mode::DN, Word, Swap, CIMS)


    // TAS
    //
    //       Syntax: TAS <ea>
    //         Size: Byte

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0100 1010 11-- ----");
    __________MMMXXX(opcode, Instr::TAS, 0b100000000000, Byte, TasRg, CIMS)
    __________MMMXXX(opcode, Instr::TAS, 0b001111111000, Byte, TasEa, CIMS)


    // TRAP
    //
    //       Syntax: TRAP #<vector>
    //        Sizes: Unsized

    opcode = parse("0100 1110 0100 ----");
    ____________XXXX(opcode, Instr::TRAP, Mode::IP, Long, Trap, CIMS)


    // TRAPcc
    //
    //       Syntax: TRAPcc #<vector>
    //        Sizes: Unsized

    if constexpr (C >= Core::C68020) {

        opcode = parse("0101 ---- 1111 1100");
        ________________(opcode | 0x000, Instr::TRAPT,  Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x100, Instr::TRAPF,  Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x200, Instr::TRAPHI, Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x300, Instr::TRAPLS, Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x400, Instr::TRAPCC, Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x500, Instr::TRAPCS, Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x600, Instr::TRAPNE, Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x700, Instr::TRAPEQ, Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x800, Instr::TRAPVC, Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0x900, Instr::TRAPVS, Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0xA00, Instr::TRAPPL, Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0xB00, Instr::TRAPMI, Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0xC00, Instr::TRAPGE, Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0xD00, Instr::TRAPLT, Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0xE00, Instr::TRAPGT, Mode::IP, Byte, Trapcc, CIMS)
        ________________(opcode | 0xF00, Instr::TRAPLE, Mode::IP, Byte, Trapcc, CIMS)

        opcode = parse("0101 ---- 1111 1010");
        ________________(opcode | 0x000, Instr::TRAPT,  Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x100, Instr::TRAPF,  Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x200, Instr::TRAPHI, Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x300, Instr::TRAPLS, Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x400, Instr::TRAPCC, Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x500, Instr::TRAPCS, Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x600, Instr::TRAPNE, Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x700, Instr::TRAPEQ, Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x800, Instr::TRAPVC, Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0x900, Instr::TRAPVS, Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0xA00, Instr::TRAPPL, Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0xB00, Instr::TRAPMI, Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0xC00, Instr::TRAPGE, Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0xD00, Instr::TRAPLT, Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0xE00, Instr::TRAPGT, Mode::IP, Word, Trapcc, CIMS)
        ________________(opcode | 0xF00, Instr::TRAPLE, Mode::IP, Word, Trapcc, CIMS)

        opcode = parse("0101 ---- 1111 1011");
        ________________(opcode | 0x000, Instr::TRAPT,  Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x100, Instr::TRAPF,  Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x200, Instr::TRAPHI, Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x300, Instr::TRAPLS, Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x400, Instr::TRAPCC, Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x500, Instr::TRAPCS, Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x600, Instr::TRAPNE, Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x700, Instr::TRAPEQ, Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x800, Instr::TRAPVC, Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0x900, Instr::TRAPVS, Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0xA00, Instr::TRAPPL, Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0xB00, Instr::TRAPMI, Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0xC00, Instr::TRAPGE, Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0xD00, Instr::TRAPLT, Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0xE00, Instr::TRAPGT, Mode::IP, Long, Trapcc, CIMS)
        ________________(opcode | 0xF00, Instr::TRAPLE, Mode::IP, Long, Trapcc, CIMS)
    }


    // TRAPV
    //
    //       Syntax: TRAPV
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0110");
    ________________(opcode, Instr::TRAPV, Mode::IP, Long, Trapv, CIMS)


    // TST
    //
    //       Syntax: TST <ea>
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)  X   X   X   X   X   X   X  (X) (X) (X)

    opcode = parse("0100 1010 ---- ----");
    ________SSMMMXXX(opcode, Instr::TST, 0b101111111000, Byte | Word | Long, Tst, CIMS)
    ________SSMMMXXX(opcode, Instr::TST, 0b001110000000, Byte | Word | Long, Tst, CIMSloop)

    if constexpr (C >= Core::C68020) {

        ________SSMMMXXX(opcode, Instr::TST, 0b000000000111, Byte, Tst, CIMS)
        ________SSMMMXXX(opcode, Instr::TST, 0b010000000111, Word | Long, Tst, CIMS)
    }


    // UNLK
    //
    //       Syntax: UNLK An
    //        Sizes: Word

    opcode = parse("0100 1110 0101 1---");
    _____________XXX(opcode, Instr::UNLK, Mode::IP, Word, Unlk, CIMS)


    // UNPK
    //
    //       Syntax: UNPK -(Ax),-(Ay),#<adjustment>
    //               UNPK DX,Dy,#<adjustment>
    //        Sizes: Unsized

    if constexpr (C >= Core::C68020) {

        opcode = parse("1000 ---1 1000 0---");
        ____XXX______XXX(opcode, Instr::UNPK, Mode::DN, Word, UnpkDn, CIMS)

        opcode = parse("1000 ---1 1000 1---");
        ____XXX______XXX(opcode, Instr::UNPK, Mode::PD, Word, UnpkPd, CIMS)
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
            ____XXX___XXXXXX(opcode, Instr::cpBcc, Mode::IP, Word, CpBcc, CIMS)

            opcode = parse("1111 ---0 11-- ----");
            ____XXX___XXXXXX(opcode, Instr::cpBcc, Mode::IP, Long, CpBcc, CIMS)

            opcode = parse("1111 ---0 00-- ----");
            ____XXX___XXXXXX(opcode, Instr::cpGEN, Mode::IP, (Size)0, CpGen, CIMS)

            opcode = parse("1111 ---1 01-- ----");
            ____XXX___MMMXXX(opcode, Instr::cpRESTORE, 0b001101111110, Word, CpRestore, CIMS)

            opcode = parse("1111 ---1 00-- ----");
            ____XXX___MMMXXX(opcode, Instr::cpSAVE, 0b001011111000, Word, CpSave, CIMS)

            opcode = parse("1111 ---0 0111 1---");
            ____XXX______XXX(opcode, Instr::cpTRAPcc, Mode::IP, Word, CpTrapcc, CIMS)

            opcode = parse("1111 ---0 01-- ----");
            ____XXX___MMMXXX(opcode, Instr::cpScc, 0b101111111000, Byte, CpScc, CIMS)

            opcode = parse("1111 ---0 0100 1---");
            ____XXX______XXX(opcode, Instr::cpDBcc, Mode::IP, (Size)0, CpDbcc, CIMS)
        }

        
        //
        // Memory management unit
        //

        if (model == Model::M68030) {

            opcode = parse("1111 0000 00-- ----");
            __________XXXXXX(opcode, Instr::cpGEN, Mode::IP, Unsized, PGen, CIMS)
            __________MMMXXX(opcode, Instr::cpGEN, 0b111111111111, Unsized, PGen, CIMS)
        }

        if (model == Model::M68040 || model == Model::M68LC040) {

            opcode = parse("1111 0101 000- ----");
            ___________XXXXX(opcode, Instr::PFLUSH, Mode::AI, Unsized, PFlush40, CIMS)

            opcode = parse("1111 0101 0100 1---");
            _____________XXX(opcode, Instr::PTEST, Mode::AI, Unsized, PTest40, CIMS)

            opcode = parse("1111 0101 0110 1---");
            _____________XXX(opcode, Instr::PTEST, Mode::AI, Unsized, PTest40, CIMS)
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
                ___________XXXXX(opcode | i << 6, Instr::CINV, Mode::AI, Unsized, Cinv, CIMS)
            }


            //
            // CPUSH
            //

            opcode = parse("1111 0100 --1- ----");
            for (int i = 0; i < 4; i++) {
                ___________XXXXX(opcode | i << 6, Instr::CPUSH, Mode::AI, Unsized, Cpush, CIMS)
            }


            //
            // MOVE16
            //

            opcode = parse("1111 0110 0010 0---");
            _____________XXX(opcode, Instr::MOVE16, Mode::IP, Unsized, Move16PiPi, CIMS)

            opcode = parse("1111 0110 0000 0---");
            _____________XXX(opcode, Instr::MOVE16, Mode::IP, Unsized, Move16PiAl, CIMS)

            opcode = parse("1111 0110 0000 1---");
            _____________XXX(opcode, Instr::MOVE16, Mode::IP, Unsized, Move16AlPi, CIMS)

            opcode = parse("1111 0110 0001 0---");
            _____________XXX(opcode, Instr::MOVE16, Mode::IP, Unsized, Move16AiAl, CIMS)

            opcode = parse("1111 0110 0001 1---");
            _____________XXX(opcode, Instr::MOVE16, Mode::IP, Unsized, Move16AlAi, CIMS)
        }
    }

    //
    // Floating-Point Unit
    //

    if (model == Model::M68040) {

        opcode = parse("1111 0010 100- ----");
        ___________XXXXX(opcode, Instr::FBcc, Mode::IP, Word, FBcc, CIMS)

        opcode = parse("1111 0010 110- ----");
        ___________XXXXX(opcode, Instr::FBcc, Mode::IP, Long, FBcc, CIMS)

        opcode = parse("1111 0010 00-- ----");
        __________XXXXXX(opcode, Instr::cpGEN, Mode::IP, Unsized, FGen, CIMS)
        __________MMMXXX(opcode, Instr::cpGEN, 0b111111111111, Unsized, FGen, CIMS)

        opcode = parse("1111 0011 01-- ----");
        __________MMMXXX(opcode, Instr::FRESTORE, 0b001101111110, Word, FRestore, CIMS)

        opcode = parse("1111 0011 00-- ----");
        __________MMMXXX(opcode, Instr::FSAVE, 0b001011111000, Word, FSave, CIMS)

        opcode = parse("1111 0010 0111 1---");
        ________________(opcode | 0b010, Instr::FTRAPcc, Mode::IP, Word, FTrapcc, CIMS)
        ________________(opcode | 0b011, Instr::FTRAPcc, Mode::IP, Long, FTrapcc, CIMS)
        ________________(opcode | 0b100, Instr::FTRAPcc, Mode::IP, Unsized, FTrapcc, CIMS)

        opcode = parse("1111 0010 01-- ----");
        __________MMMXXX(opcode, Instr::FScc, 0b101111111000, Byte, FScc, CIMS)

        opcode = parse("1111 0010 0100 1---");
        _____________XXX(opcode, Instr::FDBcc, Mode::IP, Word, FDbcc, CIMS)
    }
}
