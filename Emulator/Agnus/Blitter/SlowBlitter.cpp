// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Blitter.h"
#include "Agnus.h"
#include "Memory.h"
#include "Paula.h"

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
 */

static constexpr u16 NOTHING   = 0b0000'0000'0000'0000;
static constexpr u16 BUSIDLE   = 0b0000'0000'0000'0001;
static constexpr u16 BUS       = 0b0000'0000'0000'0010;
static constexpr u16 WRITE_D   = 0b0000'0000'0000'0100;
static constexpr u16 FETCH_A   = 0b0000'0000'0000'1000;
static constexpr u16 FETCH_B   = 0b0000'0000'0001'0000;
static constexpr u16 FETCH_C   = 0b0000'0000'0010'0000;
static constexpr u16 HOLD_A    = 0b0000'0000'0100'0000;
static constexpr u16 HOLD_B    = 0b0000'0000'1000'0000;
static constexpr u16 HOLD_D    = 0b0000'0001'0000'0000;
static constexpr u16 FILL      = 0b0000'0010'0000'0000;
static constexpr u16 BLTDONE   = 0b0000'0100'0000'0000;
static constexpr u16 REPEAT    = 0b0000'1000'0000'0000;
static constexpr u16 FETCH     = FETCH_A | FETCH_B | FETCH_C;

void
Blitter::initSlowBlitter()
{
    /* Micro programs
     *
     * The Copy Blitter micro programs are stored in array
     *
     *   copyBlitInstr[ABCD][level][fill][]
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
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <BUSIDLE>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <BUSIDLE>,
                    &Blitter::fakeExec <BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <NOTHING>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <BUSIDLE>,
                    &Blitter::fakeExec <BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <NOTHING>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 1:  -- D0 -- D1 | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <HOLD_D | BUSIDLE>,
                    &Blitter::exec <WRITE_D | HOLD_A | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::exec <WRITE_D>,
                    &Blitter::exec <BUSIDLE | HOLD_A | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <WRITE_D | HOLD_A | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <WRITE_D>,
                    &Blitter::fakeExec <BUSIDLE | HOLD_A | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 2: C0 -- C1 -- | -- C2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <HOLD_D | BUSIDLE>,
                    &Blitter::exec <FETCH_C | HOLD_A | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                },
                {   // Full execution, fill
                    &Blitter::exec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::exec <FETCH_C | HOLD_A | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 3: C0 -- -- C1 D0 -- C2 D1 -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <HOLD_D | BUSIDLE>,
                    &Blitter::exec <FETCH_C | HOLD_A>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::exec <FETCH_C | HOLD_A>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 4: B0 -- -- B1 -- -- | -- B2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <HOLD_D | BUSIDLE>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 5: B0 -- -- B1 D0 -- B2 D1 -- | -- D2
        // 5: B0 -- -- -- B1 D0 -- -- B2 D1 -- -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <BUSIDLE | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <WRITE_D | HOLD_B | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <BUSIDLE | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <WRITE_D | HOLD_B>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <BUSIDLE | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <WRITE_D | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <BUSIDLE | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <WRITE_D | HOLD_B>,
                    &Blitter::fakeExec <BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>
                }
            }
        },

        // 6: B0 C0 -- B1 C1 -- | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <BUSIDLE | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <BUSIDLE | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <BUSIDLE | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <BUSIDLE | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 7: B0 C0 -- -- B1 C1 D0 -- B2 C2 D1 -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <BUSIDLE | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>
                },
                {
                    // Full execution, fill
                    &Blitter::exec <BUSIDLE | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <BUSIDLE | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <BUSIDLE | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>
                }
            }
        },

        // 8: A0 -- A1 -- | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <HOLD_A | BUSIDLE | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <HOLD_A | BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <HOLD_A | BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <HOLD_A | BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 9: A0 -- A1 D0 A2 D1 | -- D2
        // 9: A0 -- -- A1 D0 -- A2 D1 -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <WRITE_D | HOLD_A | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | HOLD_A>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | HOLD_A | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | HOLD_A>,
                    &Blitter::fakeExec <BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // A: A0 C0 A1 C1 A2 C2 | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <FETCH_C | HOLD_A | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_C | HOLD_A | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // B: A0 C0 -- A1 C1 D0 A2 C2 D1 | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <FETCH_C | HOLD_A>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_C | HOLD_A>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // C: A0 B0 -- A1 B1 -- A2 B2 -- | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <HOLD_B  | BUSIDLE | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <HOLD_B  | BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <HOLD_B  | BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <HOLD_B  | BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // D: A0 B0 -- A1 B1 D0 A2 B2 D1 | -- D2
        // D: A0 B0 -- -- A1 B1 D0 -- A2 B2 D1 -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <WRITE_D | HOLD_B | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <WRITE_D | HOLD_B>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <WRITE_D | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <WRITE_D | HOLD_B>,
                    &Blitter::fakeExec <BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>
                }
            }
        },

        // E: A0 B0 C0 A1 B1 C1 A2 B2 C2 | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // F: A0 B0 C0 -- A1 B1 C1 D0 A2 B2 C2 D1 | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>
                }
            }
        }
    };
    
    /* The Line Blitter micro programs are stored in array
     *
     *   lineBlitInstr[BC][level][]
     *
     * For each program, two different versions are stored:
     *
     *   [][0][] : Performs a Line Blit in accuracy level 2
     *   [][1][] : Performs a Line Blit in accuracy level 1
     */
    void (Blitter::*lineBlitInstr[4][2][8])(void) = {
        
        // B disabled, C disabled (unusual)
        {
            {   // Full execution
                &Blitter::execLine <BUSIDLE | HOLD_A>,
                &Blitter::execLine <BUSIDLE | HOLD_B>,
                &Blitter::execLine <BUSIDLE | HOLD_D>,
                &Blitter::execLine <BUSIDLE | REPEAT>,
                
                &Blitter::execLine <NOTHING>,
                &Blitter::execLine <BLTDONE>,
                &Blitter::execLine <BLTDONE>,
                &Blitter::execLine <BLTDONE>
            },
            {   // Fake execution
                &Blitter::fakeExecLine <BUSIDLE>,
                &Blitter::fakeExecLine <BUSIDLE>,
                &Blitter::fakeExecLine <BUSIDLE>,
                &Blitter::fakeExecLine <BUSIDLE | REPEAT>,
                
                &Blitter::fakeExecLine <NOTHING>,
                &Blitter::fakeExecLine <BLTDONE>,
                &Blitter::fakeExecLine <BLTDONE>,
                &Blitter::fakeExecLine <BLTDONE>
            }
        },
        
        // B disabled, C enabled (the standard case)
        {
            {   // Full execution
                &Blitter::execLine <BUSIDLE | HOLD_A>,
                &Blitter::execLine <FETCH_C | HOLD_B>,
                &Blitter::execLine <BUSIDLE | HOLD_D>,
                &Blitter::execLine <WRITE_D | REPEAT>,
                
                &Blitter::execLine <NOTHING>,
                &Blitter::execLine <BLTDONE>,
                &Blitter::execLine <BLTDONE>,
                &Blitter::execLine <BLTDONE>
            },
            {   // Fake execution
                &Blitter::fakeExecLine <BUSIDLE>,
                &Blitter::fakeExecLine <BUS>,
                &Blitter::fakeExecLine <BUSIDLE>,
                &Blitter::fakeExecLine <BUS | REPEAT>,
                
                &Blitter::fakeExecLine <NOTHING>,
                &Blitter::fakeExecLine <BLTDONE>,
                &Blitter::fakeExecLine <BLTDONE>,
                &Blitter::fakeExecLine <BLTDONE>
            }
        },
    
        // B enabled, C disabled (unusual)
        {
            {
                // Full execution
                &Blitter::execLine <BUSIDLE | HOLD_A>,
                &Blitter::execLine <FETCH_B>,
                &Blitter::execLine <BUSIDLE | HOLD_B>,
                &Blitter::execLine <BUSIDLE | HOLD_D>,
                &Blitter::execLine <BUS>,
                &Blitter::execLine <BUSIDLE | REPEAT>,
                
                &Blitter::execLine <NOTHING>,
                &Blitter::execLine <BUSIDLE | BLTDONE>
            },
            {
                // Fake execution
                &Blitter::fakeExecLine <BUSIDLE>,
                &Blitter::fakeExecLine <BUS>,
                &Blitter::fakeExecLine <BUSIDLE>,
                &Blitter::fakeExecLine <BUSIDLE>,
                &Blitter::fakeExecLine <BUS>,
                &Blitter::fakeExecLine <BUSIDLE | REPEAT>,

                &Blitter::fakeExecLine <NOTHING>,
                &Blitter::fakeExecLine <BUSIDLE | BLTDONE>
            }
        },
        
        // B enabled, C enabled (unusual)
        {
            {
                // Full execution
                &Blitter::execLine <BUSIDLE | HOLD_A>,
                &Blitter::execLine <FETCH_B>,
                &Blitter::execLine <FETCH_C | HOLD_B>,
                &Blitter::execLine <BUSIDLE | HOLD_D>,
                &Blitter::execLine <BUS>,
                &Blitter::execLine <WRITE_D | REPEAT>,
                
                &Blitter::execLine <NOTHING>,
                &Blitter::execLine <BUSIDLE | BLTDONE>
            },
            {
                // Fake execution
                &Blitter::fakeExecLine <BUSIDLE>,
                &Blitter::fakeExecLine <BUS>,
                &Blitter::fakeExecLine <BUS>,
                &Blitter::fakeExecLine <BUSIDLE>,
                &Blitter::fakeExecLine <BUS>,
                &Blitter::fakeExecLine <BUS | REPEAT>,
                
                &Blitter::fakeExecLine <NOTHING>,
                &Blitter::fakeExecLine <BUSIDLE | BLTDONE>
            }
        }
    };

    // Copy all programs over
    assert(sizeof(this->copyBlitInstr) == sizeof(copyBlitInstr));
    std::memcpy(this->copyBlitInstr, copyBlitInstr, sizeof(copyBlitInstr));

    assert(sizeof(this->lineBlitInstr) == sizeof(lineBlitInstr));
    std::memcpy(this->lineBlitInstr, lineBlitInstr, sizeof(lineBlitInstr));
}

void
Blitter::beginFakeCopyBlit()
{
    // Only call this function in copy mode
    assert(!bltconLINE());

    // Run the fast Blitter
    int nr = ((bltcon0 >> 7) & 0b11110) | !!bltconDESC();
    (this->*blitfunc[nr])();

    // Prepare the slow Blitter
    resetXCounter();
    resetYCounter();
    lockD = true;

    // Schedule the first slow Blitter execution event
    agnus.scheduleRel<SLOT_BLT>(DMA_CYCLES(1), BLT_COPY_FAKE);
}

void
Blitter::beginSlowCopyBlit()
{
    // Only call this function in copy mode
    assert(!bltconLINE());

    // Set width and height counters
    resetXCounter();
    resetYCounter();

    // Reset registers
    aold = 0;
    bold = 0;

    // Reset the fill carry bit
    fillCarry = bltconFCI();

    // Lock pipeline stage D
    lockD = true;

    // Schedule the first slow Blitter execution event
    agnus.scheduleRel<SLOT_BLT>(DMA_CYCLES(1), BLT_COPY_SLOW);

    // In debug mode, we execute the whole micro program immediately.
    // This let's us compare checksums with the FastBlitter.
    if constexpr (SLOW_BLT_DEBUG) {
        
        BusOwner owner = agnus.busOwner[agnus.pos.h];
        agnus.setBLS(false);
        
        while (scheduler.hasEvent<SLOT_BLT>()) {
            agnus.busOwner[agnus.pos.h] = BUS_NONE;
            serviceEvent();
        }
        
        agnus.busOwner[agnus.pos.h] = owner;
    }
}

void
Blitter::beginFakeLineBlit()
{
    // Only call this function in line mode
    assert(bltconLINE());

    // Do the blit
    doFastLineBlit();

    // Prepare the slow Blitter
    resetXCounter();
    resetYCounter();
    lockD = true;
    
    // Schedule the first slow Blitter execution event
    agnus.scheduleRel<SLOT_BLT>(DMA_CYCLES(1), BLT_LINE_FAKE);
}

void
Blitter::beginSlowLineBlit()
{
    // Only call this function is line mode
    assert(bltconLINE());

    // Set width and height counters
    resetXCounter();
    resetYCounter();

    // Reset registers
    aold = 0;
    bold = 0;

    // Unlock pipeline stage D
    lockD = false;

    // Used to detect the first dot in a line
    fillCarry = true;
    
    // Schedule the first slow Blitter execution event
    agnus.scheduleRel<SLOT_BLT>(DMA_CYCLES(1), BLT_LINE_SLOW);

    // In debug mode, we execute the whole micro program immediately.
    // This let's us compare checksums with the FastBlitter.
    if constexpr (SLOW_BLT_DEBUG) {
        
        BusOwner owner = agnus.busOwner[agnus.pos.h];
        agnus.setBLS(false);
        
        while (scheduler.hasEvent<SLOT_BLT>()) {
            agnus.busOwner[agnus.pos.h] = BUS_NONE;
            serviceEvent();
        }
        
        agnus.busOwner[agnus.pos.h] = owner;
    }
}

template <u16 instr> void
Blitter::exec()
{
    bool bus, busidle;
    bool desc = bltconDESC();
    
    // Determine if we need the bus
    if constexpr ((bool)(instr & WRITE_D)) {
        bus     = !lockD;
        busidle = lockD;
    } else {
        bus     = (bool)(instr & (FETCH | BUS));
        busidle = (bool)(instr & BUSIDLE);
    }

    // Trigger Blitter interrupt if this is the termination cycle
    if constexpr ((bool)(instr & BLTDONE)) {
        if (!birq) {
            paula.scheduleIrqRel(INT_BLIT, DMA_CYCLES(1));
            birq = true;
        }
    }
    
    // Allocate the bus if needed
    if (bus && !agnus.allocateBus<BUS_BLITTER>()) return;

    // Check if the Blitter needs a free bus to continue
    if (busidle && !agnus.busIsFree<BUS_BLITTER>()) return;

    bltpc++;

    if constexpr ((bool)(instr & WRITE_D)) {

        // Only proceed if channel D is unlocked
        if (!lockD) {

            agnus.doBlitterDmaWrite(bltdpt, dhold);

            if constexpr (BLT_GUARD) {
                memguard[bltdpt & agnus.ptrMask & mem.chipMask] = 1;
            }

            if constexpr (BLT_CHECKSUM) {
                check1 = util::fnv_1a_it32(check1, dhold);
                check2 = util::fnv_1a_it32(check2, bltdpt);
            }
            trace(BLT_DEBUG, "    D = %X -> %X\n", dhold, bltdpt);
            
            bltdpt = U32_ADD(bltdpt, desc ? -2 : 2);
            if (--cntD == 0) {
                bltdpt = U32_ADD(bltdpt, desc ? -bltdmod : bltdmod);
                cntD = bltsizeH;
                fillCarry = !!bltconFCI();
            }
        }
    }

    if constexpr ((bool)(instr & FETCH_A)) {

        trace(BLT_DEBUG, "FETCH_A\n");

        anew = agnus.doBlitterDmaRead(bltapt);
        trace(BLT_DEBUG, "    A = %X <- %X\n", anew, bltapt);
        
        bltapt = U32_ADD(bltapt, desc ? -2 : 2);
        if (--cntA == 0) {
            bltapt = U32_ADD(bltapt, desc ? -bltamod : bltamod);
            cntA = bltsizeH;
        }
    }

    if constexpr ((bool)(instr & FETCH_B)) {

        trace(BLT_DEBUG, "FETCH_B\n");

        bnew = agnus.doBlitterDmaRead(bltbpt);
        trace(BLT_DEBUG, "    B = %X <- %X\n", bnew, bltbpt);
        
        bltbpt = U32_ADD(bltbpt, desc ? -2 : 2);
        if (--cntB == 0) {
            bltbpt = U32_ADD(bltbpt, desc ? -bltbmod : bltbmod);
            cntB = bltsizeH;
        }
    }

    if constexpr ((bool)(instr & FETCH_C)) {

        trace(BLT_DEBUG, "FETCH_C\n");

        chold = agnus.doBlitterDmaRead(bltcpt);
        trace(BLT_DEBUG, "    C = %X <- %X\n", chold, bltcpt);
        
        bltcpt = U32_ADD(bltcpt, desc ? -2 : 2);
        if (--cntC == 0) {
            bltcpt = U32_ADD(bltcpt, desc ? -bltcmod : bltcmod);
            cntC = bltsizeH;
        }
    }

    if constexpr ((bool)(instr & HOLD_A)) {

        trace(BLT_DEBUG, "HOLD_A\n");

        // Run the barrel shifter on data path A
        ahold = barrelShifter(anew & mask, aold, bltconASH(), desc);
        aold = anew & mask;
    }

    if constexpr ((bool)(instr & HOLD_B)) {

        trace(BLT_DEBUG, "HOLD_B\n");

        // Run the barrel shifter on data path B
        bhold = barrelShifter(bnew, bold, bltconBSH(), desc);
        bold = bnew;
    }

    if constexpr ((bool)(instr & HOLD_D)) {

        trace(BLT_DEBUG, "HOLD_D\n");

        // Run the minterm logic circuit
        dhold = doMintermLogic(ahold, bhold, chold, bltcon0 & 0xFF);

        if constexpr (BLT_DEBUG) {
            assert(dhold == doMintermLogic(ahold, bhold, chold, bltcon0 & 0xFF));
        }

        if (!lockD) {

            // Run the fill logic circuitry
            if constexpr ((bool)(instr & FILL)) doFill(dhold, fillCarry);

            // Update the zero flag
            if (dhold) bzero = false;
        }
        
    }

    if constexpr ((bool)(instr & REPEAT)) {

        u16 newpc = 0;

        trace(BLT_DEBUG, "REPEAT\n");
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

            clearBusyFlag();
        }
    }

    if constexpr ((bool)(instr & BLTDONE)) {

        trace(BLT_DEBUG, "BLTDONE\n");
        endBlit();
    }
}

template <u16 instr> void
Blitter::fakeExec()
{
    bool bus, busidle;

    // Determine if we need the bus
    if constexpr ((bool)(instr & WRITE_D)) {
        bus     = !lockD;
        busidle = lockD;
    } else {
        bus     = (bool)(instr & (FETCH | BUS));
        busidle = (bool)(instr & BUSIDLE);
    }

    // Trigger Blitter interrupt if this is the termination cycle
    if constexpr ((bool)(instr & BLTDONE)) {
        if (!birq) {
            paula.scheduleIrqRel(INT_BLIT, DMA_CYCLES(1));
            birq = true;
        }
    }

    // Allocate the bus if needed
    if (bus && !agnus.allocateBus<BUS_BLITTER>()) return;

    // Check if the Blitter needs a free bus to continue
    if (busidle && !agnus.busIsFree<BUS_BLITTER>()) return;

    bltpc++;

    if constexpr ((bool)(instr & (FETCH | WRITE_D))) {

        // Record some fake data to make the DMA debugger happy
        assert(agnus.pos.h < HPOS_CNT);
        agnus.busValue[agnus.pos.h] = 0x8888;
    }

    if constexpr ((bool)(instr & REPEAT)) {

        u16 newpc = 0;

        trace(BLT_DEBUG, "REPEAT\n");
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
            
            clearBusyFlag();
        }
    }

    if constexpr ((bool)(instr & BLTDONE)) {

        trace(BLT_DEBUG, "BLTDONE\n");
        endBlit();
    }
}

void
Blitter::setXCounter(u16 value)
{
    xCounter = value;

    // Set the mask for this iteration
    mask = 0xFFFF;

    // Apply the "first word mask" in the first iteration
    if (isFirstWord()) mask &= bltafwm;

    // Apply the "last word mask" in the last iteration
    if (isLastWord()) mask &= bltalwm;
}

void
Blitter::setYCounter(u16 value)
{
    yCounter = value;
}

template <u16 instr> void
Blitter::execLine()
{
    bool useC = bltcon0 & BLTCON0_USEC;
    bool sing = bltcon1 & BLTCON1_SING;
    
    bool bus, busidle;
    
    // Determine if we need the bus
    if constexpr ((bool)(instr & WRITE_D)) {
        bus     = true;
        busidle = false;
    } else {
        bus     = (bool)(instr & (FETCH | BUS));
        busidle = (bool)(instr & BUSIDLE);
    }

    // Trigger Blitter interrupt if this is the termination cycle
    if constexpr ((bool)(instr & BLTDONE)) {
        if (!birq) {
            paula.scheduleIrqRel(INT_BLIT, DMA_CYCLES(1));
            birq = true;
        }
    }
    
    // Allocate the bus if needed
    if (bus && !agnus.allocateBus<BUS_BLITTER>()) return;

    // Check if the Blitter needs a free bus to continue
    if (busidle && !agnus.busIsFree<BUS_BLITTER>()) return;

    bltpc++;

    if constexpr ((bool)(instr & WRITE_D)) {
        
        // Only proceed if channel D is unlocked
        if (!lockD) {

            agnus.doBlitterDmaWrite(bltdpt, dhold);

            if constexpr (BLT_GUARD) {
                memguard[bltdpt & agnus.ptrMask & mem.chipMask] = 1;
            }

            if constexpr (BLT_CHECKSUM) {
                check1 = util::fnv_1a_it32(check1, dhold);
                check2 = util::fnv_1a_it32(check2, bltdpt);
            }
        }
    }

    if constexpr ((bool)(instr & FETCH_B)) {

        // Perform channel B DMA
        bnew = agnus.doBlitterDmaRead(bltbpt);
        U32_INC(bltbpt, bltbmod);
    }

    if constexpr ((bool)(instr & FETCH_C)) {

        // Perform channel C DMA
        chold = agnus.doBlitterDmaRead(bltcpt);
    }

    if constexpr ((bool)(instr & HOLD_A)) {

        // Run the barrel shifter on data path A
        ahold = barrelShifter(anew & bltafwm, 0, bltconASH());
    }

    if constexpr ((bool)(instr & HOLD_B)) {

        // Run the barrel shifter on data path B
        bhold = barrelShifter(bnew, bnew, bltconBSH());
        decBSH();
    }

    if constexpr ((bool)(instr & HOLD_D)) {

        // Run the minterm logic circuit
        dhold = doMintermLogic(ahold, (bhold & 1) ? 0xFFFF : 0, chold, bltcon0 & 0xFF);
                
        // Determine if we need to lock the D channel in WRITE_D
        lockD = (sing && !fillCarry) || !useC;

        // Run the line logic circuit
        doLine();

        // Update the zero flag
        if (dhold) bzero = false;
    }

    if constexpr ((bool)(instr & REPEAT)) {

        iteration++;
        lockD = false;

        if (yCounter > 1) {

            bltpc = 0;
            resetXCounter();
            decYCounter();

        } else {
        
            clearBusyFlag();
        }
        
        bltdpt = bltcpt;
    }

    if constexpr ((bool)(instr & BLTDONE)) {

        endBlit();
    }
}

template <u16 instr> void
Blitter::fakeExecLine()
{
    bool bus, busidle;

    // Determine if we need the bus
    if constexpr ((bool)(instr & WRITE_D)) {
        bus     = true;
        busidle = false;
    } else {
        bus     = (bool)(instr & (FETCH | BUS));
        busidle = (bool)(instr & BUSIDLE);
    }
    
    // Trigger Blitter interrupt if this is the termination cycle
    if constexpr ((bool)(instr & BLTDONE)) {
        if (!birq) {
            paula.scheduleIrqRel(INT_BLIT, DMA_CYCLES(1));
            birq = true;
        }
    }
    
    // Allocate the bus if needed
    if (bus && !agnus.allocateBus<BUS_BLITTER>()) return;

    // Check if the Blitter needs a free bus to continue
    if (busidle && !agnus.busIsFree<BUS_BLITTER>()) return;

    bltpc++;

    if constexpr ((bool)(instr & (FETCH | BUS | WRITE_D))) {

        // Record some fake data to make the DMA debugger happy
        assert(agnus.pos.h < HPOS_CNT);
        agnus.busValue[agnus.pos.h] = 0x8888;
    }

    if constexpr ((bool)(instr & REPEAT)) {
        
        iteration++;
        lockD = false;

        if (yCounter > 1) {

            bltpc = 0;
            resetXCounter();
            decYCounter();

        } else {
            
            clearBusyFlag();
        }
    }
    
    if constexpr ((bool)(instr & BLTDONE)) {

        endBlit();
    }
}
