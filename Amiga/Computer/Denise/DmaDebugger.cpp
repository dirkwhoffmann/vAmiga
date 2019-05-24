// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

DmaDebugger::DmaDebugger()
{
    setDescription("DmaDebugger");

    setColor(BUS_DISK, RgbColor::yellow);
    setColor(BUS_AUDIO, RgbColor::blue);
    setColor(BUS_SPRITE, RgbColor::magenta);
    setColor(BUS_BITPLANE, RgbColor::red);
    setColor(BUS_BLITTER, RgbColor::green);
    setColor(BUS_COPPER, RgbColor::blue);
}

bool
DmaDebugger::isVisualized(BusOwner owner)
{
    return true;
}

void
DmaDebugger::setVisualized(BusOwner owner, bool value)
{

}

uint32_t
DmaDebugger::getColor(BusOwner owner)
{
    return 0;
}

void
DmaDebugger::setColor(BusOwner owner, RgbColor color)
{
    assert(isBusOwner(owner));

    float weight[4] = { 0.3, 0.2, 0.1, 0.0 };

    for (int i = 0; i < 4; i++) {

        debugColor[owner][i] = color.shade(weight[i]);
    }
}

float
DmaDebugger::getOpacity()
{
    return 1.0;
}

void
DmaDebugger::setOpacity(float value)
{

}

void
DmaDebugger::computeOverlay()
{
    BusOwner *owners = amiga->agnus.busOwner;
    int *ptr = amiga->denise.pixelAddr(0);

    for (int i = 0; i < HPOS_COUNT; i++) {

        GpuColor bg = GpuColor(ptr[i]);
        BusOwner owner = owners[i];
        owners[i] = BUS_NONE;

        switch (owner) {

            case BUS_DISK:
            case BUS_AUDIO:
            case BUS_SPRITE:
            case BUS_BITPLANE:
            case BUS_BLITTER:
            case BUS_COPPER:
                break;

            default:
                ptr += 2;
                continue;

        }

        int data = rand() % 4;
        GpuColor col = bg.mix(debugColor[owner][data], 0.3);
        *ptr++ = col.rawValue;

        data = rand() % 4;
        col = bg.mix(debugColor[owner][data], 0.3);
        *ptr++ = col.rawValue;
    }
}
