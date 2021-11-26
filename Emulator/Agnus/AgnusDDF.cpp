// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Agnus.h"

/* Register DDFSTRT and DDFSTOP define the area where the system performs
 * bitplane DMA. From a hardware engineer's point of view, these registers
 * are completely independent of DIWSTRT and DIWSTOP. From a software
 * engineer's point of view they appear closely related though. To get
 * graphics output right, bitplane DMA has to start closely before the
 * display window opens (left border ends) and to stop closely after the
 * display window closes (right border begins).
 * DDFSTRT and DDFSTOP have a resolution of four lowres pixels (unlike
 * DIWSTRT and DIWSTOP which have a resolution of one lores pixels).
 *
 * I haven't found detailed information about the how the DDF logic is
 * implemented in hardware inside Agnus. If you have such information,
 * please let me know. For the time being, I base my implementation on the
 * following assumptions:
 *
 * 1. The four-pixel resolution is achieved by ignoring the two lower bits
 *    in DDFSTRT and DDFSTOP.
 *
 * 2. The actual DMA start position depends solely on DDFSTRT. In hires
 *    mode, the start position always matches DDFSTRT. In lores mode, it
 *    matches DDFSTRT only if DDFSTRT is dividable by 8. Otherwise, the
 *    value is rounded up to the next position dividable by eight (because
 *    the lower two bits are always 0, this is equivalent to adding 4).
 *
 * 3. The actual DMA stop position depends on both DDFSTRT and DDFSTOP.
 *    Hence, if DDFSTRT changes, the stop position needs to be recomputed
 *    even if DDFSTOP hasn't changed.
 *
 * 4. Agnus switches bitplane DMA on and off by constantly comparing the
 *    horizontal raster position with the DMA start and stop positions that
 *    have been computed out of DDFSTRT and DDFSTOP. Hence, if DDFSTRT
 *    changes before DMA is switched on, the changed values takes effect
 *    immediately (i.e., in the same rasterline). If it changes when DMA is
 *    already on, the change takes effect in the next rasterline.
 *
 * 5. The values written to DDFSTRT and DDFSTOP are not clipped if they
 *    describe a position outside the two hardware stops (at 0x18 and 0xD8).
 *    E.g., if a very small value is written to DDFSTRT, Agnus starts
 *    incrementing the bitplane pointers even if the left hardware stop is
 *    not crossed yet. Agnus simply refused to perform DMA until the
 *    hardware stop has been crossed.
 */

#define DDF_EMPTY     0
#define DDF_STRT_STOP 1
#define DDF_STRT_D8   2
#define DDF_18_STOP   3
#define DDF_18_D8     4

void
Agnus::predictDDF()
{
    auto oldLores = ddfLores;
    auto oldHires = ddfHires;
    auto oldState = ddfState;
    
    ddfstrtReached = ddfstrt < HPOS_CNT ? ddfstrt : -1;
    ddfstopReached = ddfstop < HPOS_CNT ? ddfstop : -1;
    
    computeDDFWindow();
    
    if (ddfLores != oldLores || ddfHires != oldHires || ddfState != oldState) {
        
        hsyncActions |= HSYNC_UPDATE_BPL_TABLE; // Update bitplane events
        hsyncActions |= HSYNC_PREDICT_DDF;      // Call this function again
    }
    
    trace(DDF_DEBUG, "predictDDF LORES: %ld %ld\n", ddfLores.strt, ddfLores.stop);
    trace(DDF_DEBUG, "predictDDF HIRES: %ld %ld\n", ddfHires.strt, ddfHires.stop);
}

void
Agnus::computeDDFWindow()
{
    isOCS() ? computeDDFWindowOCS() : computeDDFWindowECS();
}

void
Agnus::computeDDFWindowOCS()
{
    /* To determine the correct data fetch window, we need to distinguish
     * three kinds of DDFSTRT / DDFSTOP values.
     *
     *   0:   small : Value is smaller than the left hardware stop.
     *   1:  medium : Value complies to the specs.
     *   2:   large : Value is larger than HPOS_MAX and thus never reached.
     */
    int strt = (ddfstrtReached < 0) ? 2 : (ddfstrtReached < 0x18) ? 0 : 1;
    int stop = (ddfstopReached < 0) ? 2 : (ddfstopReached < 0x18) ? 0 : 1;

    /* Emulate the special "scan line effect" of the OCS Agnus.
     * If DDFSTRT is set to a small value, DMA is enabled every other row.
     */
    if (ddfstrtReached < 0x18) {
        
        if (ocsEarlyAccessLine == pos.v) {
            ddfLores.compute(ddfstrtReached, ddfstopReached);
            ddfHires.compute(ddfstrtReached, ddfstopReached);
        
        } else {
            
            ddfLores.clear();
            ddfHires.clear();
            ocsEarlyAccessLine = pos.v + 1;
        }
        return;
    }

    /* Nr | DDFSTRT | DDFSTOP | State   || Data Fetch Window   | Next State
     *  --------------------------------------------------------------------
     *  0 | small   | small   | -       || Empty               | DDF_OFF
     *  1 | small   | medium  | -       || [0x18 ; DDFSTOP]    | DDF_OFF
     *  2 | small   | large   | -       || [0x18 ; 0xD8]       | DDF_OFF
     *  3 | medium  | small   | -       || not handled         | DDF_OFF
     *  4 | medium  | medium  | -       || [DDFSTRT ; DDFSTOP] | DDF_OFF
     *  5 | medium  | large   | -       || [DDFSTRT ; 0xD8]    | DDF_OFF
     *  6 | large   | small   | -       || not handled         | DDF_OFF
     *  7 | large   | medium  | -       || not handled         | DDF_OFF
     *  8 | large   | large   | -       || Empty               | DDF_OFF
     */
    const struct { isize interval; } table[9] = {
        
        { DDF_EMPTY     }, // 0
        { DDF_18_STOP   }, // 1
        { DDF_18_D8     }, // 2
        { DDF_EMPTY     }, // 3
        { DDF_STRT_STOP }, // 4
        { DDF_STRT_D8   }, // 5
        { DDF_EMPTY     }, // 6
        { DDF_EMPTY     }, // 7
        { DDF_EMPTY     }  // 8
    };

    isize index = 3*strt + stop;
    switch (table[index].interval) {

        case DDF_EMPTY:
            
            ddfLores.clear();
            ddfHires.clear();
            break;
            
        case DDF_STRT_STOP:
            
            ddfLores.compute(ddfstrtReached, ddfstopReached);
            ddfHires.compute(ddfstrtReached, ddfstopReached);
            break;
            
        case DDF_STRT_D8:
            
            ddfLores.compute(ddfstrtReached, 0xD8);
            ddfHires.compute(ddfstrtReached, 0xD8);
            break;
            
        case DDF_18_STOP:
            
            ddfLores.compute(0x18, ddfstopReached);
            ddfHires.compute(0x18, ddfstopReached);
            break;
            
        case DDF_18_D8:
            
            ddfLores.compute(0x18, 0xD8);
            ddfHires.compute(0x18, 0xD8);
            break;
    }

    trace(DDF_DEBUG, "DDF Window (OCS):  (%ld,%ld) (%ld,%ld)\n",
          ddfLores.strt, ddfHires.strt, ddfLores.stop, ddfHires.stop);
}

void
Agnus::computeDDFWindowECS()
{
    /* To determine the correct data fetch window, we need to distinguish
     * three kinds of DDFSTRT / DDFSTOP values.
     *
     *   0:   small : Value is smaller than the left hardware stop.
     *   1:  medium : Value complies to the specs.
     *   2:   large : Value is larger than HPOS_MAX and thus never reached.
     */
    int strt = (ddfstrtReached < 0) ? 2 : (ddfstrtReached < 0x18) ? 0 : 1;
    int stop = (ddfstopReached < 0) ? 2 : (ddfstopReached < 0x18) ? 0 : 1;

    /* Nr | DDFSTRT | DDFSTOP | State   || Data Fetch Window   | Next State
     *  --------------------------------------------------------------------
     *  0 | small   | small   | DDF_OFF || Empty               | DDF_OFF
     *  1 | small   | small   | DDF_ON  || Empty               | DDF_OFF
     *  2 | small   | medium  | DDF_OFF || [0x18 ; DDFSTOP]    | DDF_OFF
     *  3 | small   | medium  | DDF_ON  || [0x18 ; DDFSTOP]    | DDF_OFF
     *  4 | small   | large   | DDF_OFF || [0x18 ; 0xD8]       | DDF_ON
     *  5 | small   | large   | DDF_ON  || [0x18 ; 0xD8]       | DDF_ON
     *  6 | medium  | small   | DDF_OFF || not handled         | -
     *  7 | medium  | small   | DDF_ON  || not handled         | -
     *  8 | medium  | medium  | DDF_OFF || [DDFSTRT ; DDFSTOP] | DDF_OFF
     *  9 | medium  | medium  | DDF_ON  || [0x18 ; DDFSTOP]    | DDF_OFF
     * 10 | medium  | large   | DDF_OFF || [DDFSTRT ; 0xD8]    | DDF_ON
     * 11 | medium  | large   | DDF_ON  || [0x18 ; 0xD8]       | DDF_ON
     * 12 | large   | small   | DDF_OFF || not handled         | -
     * 13 | large   | small   | DDF_ON  || not handled         | -
     * 14 | large   | medium  | DDF_OFF || not handled         | -
     * 15 | large   | medium  | DDF_ON  || not handled         | -
     * 16 | large   | large   | DDF_OFF || Empty               | DDF_OFF
     * 17 | large   | large   | DDF_ON  || [0x18 ; 0xD8]       | DDF_ON
     */
    const struct { isize interval; DDFState state; } table[18] = {
        
        { DDF_EMPTY ,    DDF_OFF }, // 0
        { DDF_EMPTY ,    DDF_OFF }, // 1
        { DDF_18_STOP ,  DDF_OFF }, // 2
        { DDF_18_STOP ,  DDF_OFF }, // 3
        { DDF_18_D8 ,    DDF_ON  }, // 4
        { DDF_18_D8 ,    DDF_ON  }, // 5
        { DDF_EMPTY ,    DDF_OFF }, // 6
        { DDF_EMPTY ,    DDF_OFF }, // 7
        { DDF_STRT_STOP, DDF_OFF }, // 8
        { DDF_18_STOP ,  DDF_OFF }, // 9
        { DDF_STRT_D8 ,  DDF_ON  }, // 10
        { DDF_18_D8 ,    DDF_ON  }, // 11
        { DDF_EMPTY ,    DDF_OFF }, // 12
        { DDF_EMPTY ,    DDF_OFF }, // 13
        { DDF_EMPTY ,    DDF_OFF }, // 14
        { DDF_EMPTY ,    DDF_OFF }, // 15
        { DDF_EMPTY ,    DDF_OFF }, // 16
        { DDF_18_D8 ,    DDF_ON  }, // 17
    };

    isize index = 6*strt + 2*stop + (ddfState == DDF_ON);
    switch (table[index].interval) {

        case DDF_EMPTY:
            
            ddfLores.clear();
            ddfHires.clear();
            break;
            
        case DDF_STRT_STOP:
            
            ddfLores.compute(ddfstrtReached, ddfstopReached);
            ddfHires.compute(ddfstrtReached, ddfstopReached);
            break;
            
        case DDF_STRT_D8:
            
            ddfLores.compute(ddfstrtReached, 0xD8);
            ddfHires.compute(ddfstrtReached, 0xD8);
            break;
            
        case DDF_18_STOP:
            
            ddfLores.compute(0x18, ddfstopReached);
            ddfHires.compute(0x18, ddfstopReached);
            break;
            
        case DDF_18_D8:
            
            ddfLores.compute(0x18, 0xD8);
            ddfHires.compute(0x18, 0xD8);
            break;
    }
    ddfState = table[index].state;

    trace(DDF_DEBUG, "DDF Window (ECS):  (%ld,%ld) (%ld,%ld)\n",
          ddfLores.strt, ddfHires.strt, ddfLores.stop, ddfHires.stop);
}
