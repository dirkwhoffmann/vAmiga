// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

/* Micro-instructions:
 *
 * To keep the implementation flexible, the SlowBlitter is emulated as a
 * micro-programmable device. When a blit is processed, a micro-program is
 * executed that decide on the actions that are performed in a certain Blitter
 * cycle.
 *
 * A micro-program consists of the following micro-instructions:
 *
 *     NOTHING : No action is taken
 *     BUSIDLE : Waits for the bus to be free
 *         BUS : Waits for the bus to be free and allocates it
 *     WRITE_D : Writes back register D hold
 *     FETCH_A : Loads register A new
 *     FETCH_B : Loads register B new
 *     FETCH_C : Loads register C hold
 *      HOLD_A : Loads register A hold
 *      HOLD_B : Loads register B hold
 *      HOLD_D : Loads register D hold
 *        FILL : Run the fill circuitry
 *     BLTDONE : Marks the last instruction and terminates the Blitter
 *      REPEAT : Performs a conditional jump back to instruction 0
 *
 *   FAKEWRITE : Used in fake-execution mode instead of WRITE_D
 */

static const uint16_t NOTHING   = 0b0000'0000'0000'0000;
static const uint16_t BUSIDLE   = 0b0000'0000'0000'0001;
static const uint16_t BUS       = 0b0000'0000'0000'0010;
static const uint16_t WRITE_D   = 0b0000'0000'0000'0100;
static const uint16_t FETCH_A   = 0b0000'0000'0000'1000;
static const uint16_t FETCH_B   = 0b0000'0000'0001'0000;
static const uint16_t FETCH_C   = 0b0000'0000'0010'0000;
static const uint16_t HOLD_A    = 0b0000'0000'0100'0000;
static const uint16_t HOLD_B    = 0b0000'0000'1000'0000;
static const uint16_t HOLD_D    = 0b0000'0001'0000'0000;
static const uint16_t FILL      = 0b0000'0010'0000'0000;
static const uint16_t BLTDONE   = 0b0000'0100'0000'0000;
static const uint16_t REPEAT    = 0b0000'1000'0000'0000;

static const uint16_t FAKEWRITE = 0b0001'0000'0000'0000;

void
Blitter::initSlowBlitter()
{
    /* Micro programs
     *
     * The Copy Blitter micro programs are stored in array
     *
     *   copyBlitInstr[16][2][2][6]
     *
     * For each program, four different versions are stored:
     *
     *   [][0][0][] : Performs a Copy Blit in accuracy level 2
     *   [][0][1][] : Performs a Fill Copy Blit in accuracy level 2
     *   [][1][0][] : Performs a Copy Blit in accuracy level 1
     *   [][1][1][] : Performs a Fill Copy Blit in accuracy level 1
     *
     * Level 2 microprograms operate the bus and all Blitter components.
     * Level 1 microprograms are a stripped down version that operates
     * the bus only. This is what we call "fake execution", because the
     * blit itself has already been carried out by the Fast Blitter.
     *
     * The programs below have been derived from Table 6.2 of the HRM.
     * The published table doesn't seem to be 100% accurate. See the
     * microprograms below for applied modifications.
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
     *
     * The programs below apply of the fill bit is set. They have been derived
     * from the "Errata for the Amiga Hardware Manual" (October 17, 1985).
     * The published table doesn't seem to be 100% accurate. See the
     * microprograms below for applied modifications.
     *
     *           Active
     * BLTCON0  Channels            Cycle sequence
     *    D     A B   D    A0 B0 -- -- A1 B1 D0 -- A2 B2 D1 -- D2
     *    9     A     D    A0 -- -- A1 D0 A2 D1 -- D2
     *    5       B   D    B0 -- -- -- B1 D0 -- -- B2 D1 -- D2
     *    1           D    -- -- -- D0 -- -- D1 -- -- D2
     *
     * For all other BLTCON0 combinations, the fill bit has no effect on timing.
     */
    void (Blitter::*copyBlitInstr[16][2][2][6])(void) = {

        // 0: -- -- | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <BUSIDLE>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <BUSIDLE>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUSIDLE>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUSIDLE>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                }
            }
        },

        // 1: D0 -- D1 -- | -- D2
        // 1: D0 -- -- D1 -- -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <WRITE_D | HOLD_A | HOLD_B | BUS>,
                    &Blitter::exec <HOLD_D | BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <WRITE_D | HOLD_A | HOLD_B | BUS>,
                    &Blitter::exec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::exec <REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <FAKEWRITE | BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <FAKEWRITE | BUS>,
                    &Blitter::exec <BUSIDLE>,
                    &Blitter::exec <REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                }
            }
        },

        // 2: C0 -- C1 -- | -- C2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS>,
                    &Blitter::exec <HOLD_D | BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS>,
                    &Blitter::exec <FILL | HOLD_D | BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                }
            }
        },

        // 3: C0 -- -- C1 D0 -- C2 D1 -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS>,
                    &Blitter::exec <WRITE_D | BUS>,
                    &Blitter::exec <HOLD_D | BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS>,
                    &Blitter::exec <WRITE_D | BUS>,
                    &Blitter::exec <FILL | HOLD_D | BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                }
            }
        },

        // 4: B0 -- -- B1 -- -- | -- B2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_B | BUS>,
                    &Blitter::exec <HOLD_A | HOLD_B | BUSIDLE>,
                    &Blitter::exec <HOLD_D | BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_B | BUS>,
                    &Blitter::exec <HOLD_A | HOLD_B | BUSIDLE>,
                    &Blitter::exec <FILL | HOLD_D | BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUSIDLE>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUSIDLE>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                }
            }
        },

        // 5: B0 -- -- B1 D0 -- B2 D1 -- | -- D2
        // 5: B0 -- -- -- B1 D0 -- -- B2 D1 -- -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_B | BUS>,
                    &Blitter::exec <WRITE_D | HOLD_A | HOLD_B | BUS>,
                    &Blitter::exec <HOLD_D | BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_B | BUS>,
                    &Blitter::exec <WRITE_D | HOLD_A | HOLD_B | BUS>,
                    &Blitter::exec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::exec <REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | BUS>,
                    &Blitter::exec <BUSIDLE>,
                    &Blitter::exec <REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                }
            }
        },

        // 6: B0 C0 -- B1 C1 -- | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_B | HOLD_D | BUS>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_B | FILL | HOLD_D | BUS>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                }
            }
        },

        // 7: B0 C0 -- -- B1 C1 D0 -- B2 C2 D1 -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                    &Blitter::exec <FETCH_C | HOLD_B | BUS>,
                    &Blitter::exec <WRITE_D | HOLD_D | BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                },
                {
                    // Full execution, fill
                    &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                    &Blitter::exec <FETCH_C | HOLD_B | BUS>,
                    &Blitter::exec <WRITE_D | FILL | HOLD_D | BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                }
            }
        },

        // 8: A0 -- A1 -- | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                    &Blitter::exec <HOLD_A | HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D | BUS>,
                    &Blitter::exec <HOLD_A | HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                }
            }
        },

        // 9: A0 -- A1 D0 A2 D1 | -- D2
        // 9: A0 -- -- A1 D0 -- A2 D1 -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                    &Blitter::exec <WRITE_D | HOLD_A | HOLD_B | BUS | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D | BUS>,
                    &Blitter::exec <WRITE_D | HOLD_A | HOLD_B | BUS>,
                    &Blitter::exec <REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | BUS | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | BUS>,
                    &Blitter::exec <REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                }
            }
        },

        // A: A0 C0 A1 C1 A2 C2 | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D | BUS>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                }
            }
        },

        // B: A0 C0 -- A1 C1 D0 A2 C2 D1 | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS>,
                    &Blitter::exec <WRITE_D | REPEAT | BUS>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D | BUS>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | BUS>,
                    &Blitter::exec <WRITE_D | REPEAT | BUS>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | REPEAT | BUS>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | REPEAT | BUS>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                }
            }
        },

        // C: A0 B0 -- A1 B1 -- A2 B2 -- | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                    &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                    &Blitter::exec <HOLD_B  | BUSIDLE | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D | BUS>,
                    &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                    &Blitter::exec <HOLD_B  | BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                }
            }
        },

        // D: A0 B0 -- A1 B1 D0 A2 B2 D1 | -- D2
        // D: A0 B0 -- -- A1 B1 D0 -- A2 B2 D1 -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                    &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                    &Blitter::exec <WRITE_D | HOLD_B | BUS | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D | BUS>,
                    &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                    &Blitter::exec <WRITE_D | HOLD_B | BUS>,
                    &Blitter::exec <REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | BUS | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | BUS>,
                    &Blitter::exec <REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                }
            }
        },

        // E: A0 B0 C0 A1 B1 C1 A2 B2 C2 | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D | BUS>,
                    &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                    &Blitter::exec <FETCH_C | HOLD_B | BUS | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D | BUS>,
                    &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                    &Blitter::exec <FETCH_C | HOLD_B | BUS | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>
                }
            }
        },

        // F: A0 B0 C0 -- A1 B1 C1 D0 A2 B2 C2 D1 | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | BUS>,
                    &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                    &Blitter::exec <FETCH_C | HOLD_B | BUS>,
                    &Blitter::exec <WRITE_D | HOLD_D | BUS | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | BUS>,
                    &Blitter::exec <FETCH_B | HOLD_A | BUS>,
                    &Blitter::exec <FETCH_C | HOLD_B | BUS>,
                    &Blitter::exec <WRITE_D | FILL | HOLD_D | BUS | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <WRITE_D | BUS | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | BUS | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <BUS>,
                    &Blitter::exec <FAKEWRITE | BUS | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
                }
            }
        }
    };

    /* The Line Blitter uses the same micro program in all situations.
     *
     * -- C0 -- -- -- C1 -- D0 -- C2 -- D1 | -- D2   (???)
    */
    void (Blitter::*lineBlitInstr[6])(void) = {

        // Fake execution
        &Blitter::exec <BUSIDLE>,
        &Blitter::exec <BUS>,
        &Blitter::exec <BUSIDLE>,
        &Blitter::exec <FAKEWRITE | BUS | REPEAT>,

        &Blitter::exec <NOTHING>,
        &Blitter::exec <FAKEWRITE | BUS | BLTDONE>
    };

    // Copy all programs over
    assert(sizeof(this->copyBlitInstr) == sizeof(copyBlitInstr));
    memcpy(this->copyBlitInstr, copyBlitInstr, sizeof(copyBlitInstr));

    assert(sizeof(this->lineBlitInstr) == sizeof(lineBlitInstr));
    memcpy(this->lineBlitInstr, lineBlitInstr, sizeof(lineBlitInstr));

    dump();
}

void
Blitter::beginSlowLineBlit()
{
    /* Note: There is no such thing as a slow line Blitter yet. Until such a
     * thing has been implemented, we call the fast Blitter instead.
     */

    // Only call this function is line blit mode
    assert(bltconLINE());

    static bool verbose = true;
    if (verbose) { verbose = false; debug("Fall back to the the fast line Blitter\n"); }

    // Run in accuracy level 1 instead
    beginFastLineBlit(1);
}

void
Blitter::beginSlowCopyBlit()
{
    // Only call this function in copy blit mode
    assert(!bltconLINE());

    static bool verbose = true;
    if (verbose) { verbose = false; debug("Using the slow copy Blitter\n"); }

    // Setup parameters
    if (bltconDESC()) {
        incr = -2;
        ash  = 16 - bltconASH();
        bsh  = 16 - bltconBSH();
        amod = -bltamod;
        bmod = -bltbmod;
        cmod = -bltcmod;
        dmod = -bltdmod;
    } else {
        incr = 2;
        ash  = bltconASH();
        bsh  = bltconBSH();
        amod = bltamod;
        bmod = bltbmod;
        cmod = bltcmod;
        dmod = bltdmod;
    }

    // Set width and height counters
    resetXCounter();
    resetYCounter();

    // Reset registers
    aold = 0;
    bold = 0;

    // Reset the fill carry bit
    fillCarry = !!bltconFCI();

    // Lock pipeline stage D
    lockD = true;

    // Schedule the first execution event
    agnus.scheduleRel<BLT_SLOT>(DMA_CYCLES(1), BLT_COPY_SLOW);

#ifdef SLOW_BLT_DEBUG

    // In debug mode, we execute the whole micro program immediately.
    // This let's us compare checksums with the fast Blitter.
    
    BusOwner owner = agnus->busOwner[agnus->pos.h];

    while (agnus->hasEvent<BLT_SLOT>()) {
        agnus->busOwner[agnus->pos.h] = BUS_NONE;
        serviceEvent(agnus->slot[BLT_SLOT].id);
    }

    agnus->busOwner[agnus->pos.h] = owner;

#endif
}

template <uint16_t instr> void
Blitter::exec()
{
    // Check if the Blitter needs to allocate the bus to proceed
    if ((instr & BUS) && !agnus.allocateBus<BUS_BLITTER>()) {
        // debug("BUS cycle blocked in BLT_EXEC by %d\n", agnus.busOwner[agnus.pos.h]);
        return;
    }

    // Check if the Blitter needs the bus to be free to proceed
    if ((instr & BUSIDLE) && !agnus.busIsFree<BUS_BLITTER>()) {
        // debug("IDLE cycle blocked in BLT_EXEC by %d\n", agnus.busOwner[agnus.pos.h]);
        return;
    }

    bltpc++;

    if (instr & WRITE_D) {

        /* D is not written in the first iteration, because the pipepline needs
         * to ramp up.
         */
        // if (iteration == 0) {
        if (lockD) {

            debug(BLT_DEBUG, "WRITE_D (skipped)\n");
            // lockD = false;

        } else {

            agnus.blitterWrite(bltdpt, dhold);
            check1 = fnv_1a_it32(check1, dhold);
            check2 = fnv_1a_it32(check2, bltdpt);
            debug(BLT_DEBUG, "D: poke(%X), %X (check: %X %X)\n", bltdpt, dhold, check1, check2);

            INC_CHIP_PTR_BY(bltdpt, incr);
            if (--cntD == 0) {
                INC_CHIP_PTR_BY(bltdpt, dmod);
                cntD = bltsizeW;
                fillCarry = !!bltconFCI();
            }
            // if (xCounter == bltsizeW()) INC_CHIP_PTR_BY(dpt, dmod);
        }
    }

    if (instr & FAKEWRITE) {

        // This instruction is only used in fake-execution mode. We simply
        // record some fake data to make the DMA debugger happy.
        assert(agnus.pos.h < HPOS_CNT);
        agnus.busValue[agnus.pos.h] = 0x8888;
    }

    if (instr & FETCH_A) {

        debug(BLT_DEBUG, "FETCH_A\n");

        anew = agnus.blitterRead(bltapt);
        debug(BLT_DEBUG, "    A = peek(%X) = %X\n", bltapt, anew);
        debug(BLT_DEBUG, "    After fetch: A = %X\n", anew);
        INC_CHIP_PTR_BY(bltapt, incr);
        if (--cntA == 0) {
            INC_CHIP_PTR_BY(bltapt, amod);
            cntA = bltsizeW;
        }
    }

    if (instr & FETCH_B) {

        debug(BLT_DEBUG, "FETCH_B\n");

        bnew = agnus.blitterRead(bltbpt);
        debug(BLT_DEBUG, "    B = peek(%X) = %X\n", bltbpt, bnew);
        debug(BLT_DEBUG, "    After fetch: B = %X\n", bnew);
        INC_CHIP_PTR_BY(bltbpt, incr);
        if (--cntB == 0) {
            INC_CHIP_PTR_BY(bltbpt, bmod);
            cntB = bltsizeW;
        }
    }

    if (instr & FETCH_C) {

        debug(BLT_DEBUG, "FETCH_C\n");

        chold = agnus.blitterRead(bltcpt);
        debug(BLT_DEBUG, "    C = peek(%X) = %X\n", bltcpt, chold);
        debug(BLT_DEBUG, "    After fetch: C = %X\n", chold);
        INC_CHIP_PTR_BY(bltcpt, incr);
        if (--cntC == 0) {
            INC_CHIP_PTR_BY(bltcpt, cmod);
            cntC = bltsizeW;
        }
    }

    if (instr & HOLD_A) {

        debug(BLT_DEBUG, "HOLD_A\n");

        debug(BLT_DEBUG, "    After masking with %x (%x,%x) %x\n", mask, bltafwm, bltalwm, anew & mask);

        // Run the barrel shifters on data path A
        debug(BLT_DEBUG, "    ash = %d mask = %X\n", bltconASH(), mask);
        if (bltconDESC()) {
            ahold = HI_W_LO_W(anew & mask, aold) >> ash;
        } else {
            ahold = HI_W_LO_W(aold, anew & mask) >> ash;
        }
        aold = anew & mask;
        debug(BLT_DEBUG, "    After shifting A (%d) A = %x\n", ash, ahold);
    }

    if (instr & HOLD_B) {

        debug(BLT_DEBUG, "HOLD_B\n");

        // Run the barrel shifters on data path B
        debug(BLT_DEBUG, "    bsh = %d\n", bltconBSH());
        if (bltconDESC()) {
            bhold = HI_W_LO_W(bnew, bold) >> bsh;
        } else {
            bhold = HI_W_LO_W(bold, bnew) >> bsh;
        }
        bold = bnew;
        debug(BLT_DEBUG, "    After shifting B (%d) B = %x\n", bsh, bhold);
    }

    if (instr & HOLD_D) {

        /*
        if (lockD) {

            debug(BLT_DEBUG, "HOLD_D (skipped)\n");
            
        } else
        */
        {

            debug(BLT_DEBUG, "HOLD_D\n");

            // Run the minterm logic circuit
            debug(BLT_DEBUG, "    Minterms: ahold = %X bhold = %X chold = %X bltcon0 = %X (hex)\n", ahold, bhold, chold, bltcon0);
            dhold = doMintermLogicQuick(ahold, bhold, chold, bltcon0 & 0xFF);
            assert(dhold == doMintermLogic(ahold, bhold, chold, bltcon0 & 0xFF));

            // Run the fill logic circuitry
            if (instr & FILL) doFill(dhold, fillCarry);

            // Update the zero flag
            if (dhold) bzero = false;
        }
    }

    if (instr & REPEAT) {

        uint16_t newpc = 0;

        debug(BLT_DEBUG, "REPEAT\n");
        iteration++;
        lockD = false;

        if (xCounter > 1) {

            bltpc = newpc;
            decXCounter();

        } else if (yCounter > 1) {

            bltpc = newpc;
            resetXCounter();
            decYCounter();

        } else {

            signalEnd();
        }
    }

    if (instr & BLTDONE) {

        debug(BLT_DEBUG, "BLTDONE\n");
        endBlit();
    }
}

void
Blitter::setXCounter(uint16_t value)
{
    xCounter = value;

    // Set the mask for this iteration
    mask = 0xFFFF;

    // Apply the "first word mask" in the first iteration
    if (xCounter == bltsizeW) mask &= bltafwm;

    // Apply the "last word mask" in the last iteration
    if (xCounter == 1) mask &= bltalwm;
}

void
Blitter::setYCounter(uint16_t value)
{
    yCounter = value;
}

void
Blitter::doBarrelShifterA()
{
    uint16_t masked = anew;

    if (isFirstWord()) masked &= bltafwm;
    if (isLastWord())  masked &= bltalwm;

    if(bltconDESC()){
        ahold = (aold >> (16 - bltconASH())) | (masked << bltconASH());
    }else{
        ahold = (aold << (16 - bltconASH())) | (masked >> bltconASH());
    }
}

void
Blitter::doBarrelShifterB()
{
    if(bltconDESC()) {
        bhold = (bold >> (16 - bltconBSH())) | (bnew << bltconBSH());
    } else {
        bhold = (bold << (16 - bltconBSH())) | (bnew >> bltconBSH());
    }
}
