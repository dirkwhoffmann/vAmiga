// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _REGISTER_INC
#define _REGISTER_INC

struct RegisterChange
{
    uint32_t addr;
    uint16_t value;
    int16_t pixel;

    template <class T>
    void applyToItems(T& worker)
    {
        worker

        & addr
        & value
        & pixel;
    }
};

struct ChangeHistory
{
    RegisterChange change[256];
    int count;

    template <class T>
    void applyToItems(T& worker)
    {
        worker

        & change
        & count;
    }

    // Deletes all recorded register changes
    void init() { count = 0; }

    // Records a register change
    void recordChange(uint32_t addr, uint16_t value, int16_t pixel)
    {
        int nr = count++;
        assert(nr < 256);

        // Add new entry
        change[nr].addr = addr;
        change[nr].value = value;
        change[nr].pixel = pixel;

        // Keep the list sorted
        while (nr > 0 && change[nr].pixel < change[nr-1].pixel) {

            swap(change[nr], change[nr - 1]);
            nr--;
        }
    }
};

#endif
