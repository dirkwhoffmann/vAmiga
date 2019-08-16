// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
Blitter::startSlowBlitter()
{
    if (bltconLINE()) {

        // Perform a line blit (we fall back to the fast blitter)
        doFastLineBlit();

    } else {

        // Perform a copy blit
        warn("SLOW COPY BLITTER NOT IMPLEMENTED YET\n");

    }
}

void
Blitter::loadMicrocode()
{
    bltpc = 0;

    /* The following code is inspired by Table 6.2 of the HRM:
     *
     *           Active
     * BLTCON0  Channels            Cycle sequence
     *    F     A B C D    A0 B0 C0 -- A1 B1 C1 D0 A2 B2 C2 D1 D2
     *    E     A B C      A0 B0 C0 A1 B1 C1 A2 B2 C2
     *    D     A B   D    A0 B0 -- A1 B1 D0 A2 B2 D1 -- D2
     *    C     A B        A0 B0 -- A1 B1 -- A2 B2
     *    B     A   C D    A0 C0 -- A1 C1 D0 A2 C2 D1 -- D2
     *    A     A   C      A0 C0 A1 C1 A2 C2
     *    9     A     D    A0 -- A1 D0 A2 D1 -- D2
     *    8     A          A0 -- A1 -- A2
     *    7       B C D    B0 C0 -- -- B1 C1 D0 -- B2 C2 D1 -- D2
     *    6       B C      B0 C0 -- B1 C1 -- B2 C2
     *    5       B   D    B0 -- -- B1 D0 -- B2 D1 -- D2
     *    4       B        B0 -- -- B1 -- -- B2
     *    3         C D    C0 -- -- C1 D0 -- C2 D1 -- D2
     *    2         C      C0 -- C1 -- C2
     *    1           D    D0 -- D1 -- D2
     *    0                -- -- -- --
     */

    uint8_t A = !!bltconUSEA();
    uint8_t B = !!bltconUSEB();
    uint8_t C = !!bltconUSEC();
    uint8_t D = !!bltconUSED();

    switch ((A << 3) | (B << 2) | (C << 1) | D) {

        case 0xF: { // A0 B0 C0 -- A1 B1 C1 D0 A2 B2 C2 D1 D2

            uint16_t prog[] = {

                FETCH_A,
                FETCH_B | HOLD_A,
                FETCH_C | HOLD_B,
                HOLD_D,

                FETCH_A,
                FETCH_B | HOLD_A,
                FETCH_C | HOLD_B,
                WRITE_D | HOLD_D | LOOPBACK3,

                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0xE: { // A0 B0 C0 A1 B1 C1 A2 B2 C2

            uint16_t prog[] = {

                FETCH_A,
                FETCH_B | HOLD_A,
                FETCH_C | HOLD_B,

                FETCH_A | HOLD_D,
                FETCH_B | HOLD_A,
                FETCH_C | HOLD_B | LOOPBACK2,

                HOLD_D  | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0xD: { // A0 B0 -- A1 B1 D0 A2 B2 D1 -- D2

            uint16_t prog[] = {

                FETCH_A,
                FETCH_B | HOLD_A,
                HOLD_B,

                FETCH_A | HOLD_D,
                FETCH_B | HOLD_A,
                HOLD_B | WRITE_D | LOOPBACK2,

                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0xC: { // A0 B0 -- A1 B1 -- A2 B2

            uint16_t prog[] = {

                FETCH_A,
                FETCH_B | HOLD_A,
                HOLD_B | LOOPBACK2,
                BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0xB: { // A0 C0 -- A1 C1 D0 A2 C2 D1 -- D2

            uint16_t prog[] = {

                FETCH_A,
                FETCH_C | HOLD_A,
                HOLD_D,

                FETCH_A,
                FETCH_C | HOLD_A,
                WRITE_D | LOOPBACK2,

                BLTIDLE,
                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0xA: { // A0 C0 A1 C1 A2 C2

            uint16_t prog[] = {

                FETCH_A,
                FETCH_C | HOLD_A | LOOPBACK1,
                BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x9: { // A0 -- A1 D0 A2 D1 -- D2

            uint16_t prog[] = {

                FETCH_A,
                HOLD_A,

                FETCH_A,
                WRITE_D | HOLD_A | LOOPBACK1,
                BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x8: { // A0 -- A1 -- A2

            uint16_t prog[] = {

                FETCH_A,
                HOLD_A | LOOPBACK1,
                BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x7: { // B0 C0 -- -- B1 C1 D0 -- B2 C2 D1 -- D2

            uint16_t prog[] = {

                FETCH_B,
                FETCH_C | HOLD_B,
                BLTIDLE,
                BLTIDLE,
                FETCH_B,
                FETCH_C | HOLD_B,
                WRITE_D,
                LOOPBACK3,
                BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x6: { // B0 C0 -- B1 C1 -- B2 C2

            uint16_t prog[] = {

                FETCH_B,
                FETCH_C | HOLD_B,
                BLTIDLE | LOOPBACK2,
                BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x5: { // B0 -- -- B1 D0 -- B2 D1 -- D2

            uint16_t prog[] = {

                FETCH_B,
                HOLD_B,

                HOLD_D,
                FETCH_B,
                WRITE_D | HOLD_B | LOOPBACK2,

                HOLD_D,
                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x4: { // B0 -- -- B1 -- -- B2

            uint16_t prog[] = {

                FETCH_B,
                BLTIDLE,
                BLTIDLE | LOOPBACK2,

                FETCH_B | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x3: { // C0 -- -- C1 D0 -- C2 D1 -- D2

            uint16_t prog[] = {

                FETCH_C,
                BLTIDLE,
                HOLD_D,

                FETCH_C,
                WRITE_D,
                HOLD_D | LOOPBACK2,

                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x2: { // C0 -- C1 -- C2

            uint16_t prog[] = {

                FETCH_C,
                BLTIDLE | LOOPBACK1,
                FETCH_C | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x1: { // D0 -- D1 -- D2

            uint16_t prog[] = {

                WRITE_D,
                HOLD_D | LOOPBACK1,

                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x0: { // -- -- -- --

            uint16_t prog[] = {

                BLTIDLE,

                BLTIDLE,
                BLTIDLE | LOOPBACK1,

                BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        default:

            _dump();
            panic("Unimplemented Blitter configuration\n");
            assert(false);
    }

    debug(BLT_DEBUG, "Microcode loaded\n");
}

void
Blitter::doBarrelShifterA()
{
    uint16_t masked = anew;

    if (isFirstWord()) masked &= bltafwm;
    if (isLastWord())  masked &= bltalwm;

    debug(2, "first = %d last = %d masked = %X\n", isFirstWord(), isLastWord(), masked);

    if(bltconDESC()){
        ahold = (aold >> (16 - bltconASH())) | (masked << bltconASH());
    }else{
        ahold = (aold << (16 - bltconASH())) | (masked >> bltconASH());
    }

    /*
     if (bltDESC()) {
     uint32_t barrelA = HI_W_LO_W(anew & mask, aold);
     ahold = (barrelA >> (16 - bltASH())) & 0xFFFF;
     } else {
     uint32_t barrelA = HI_W_LO_W(aold, anew & mask);
     ahold = (barrelA >> bltASH()) & 0xFFFF;
     }
     */
}

void
Blitter::doBarrelShifterB()
{
    if(bltconDESC()) {
        bhold = (bold >> (16 - bltconBSH())) | (bnew << bltconBSH());
    } else {
        bhold = (bold << (16 - bltconBSH())) | (bnew >> bltconBSH());
    }

    /*
     if (bltDESC()) {
     uint32_t barrelB = HI_W_LO_W(bnew, bold);
     bhold = (barrelB >> (16 - bltBSH())) & 0xFFFF;
     } else {
     uint32_t barrelB = HI_W_LO_W(bold, bnew);
     bhold = (barrelB >> bltBSH()) & 0xFFFF;
     }
     */
}
