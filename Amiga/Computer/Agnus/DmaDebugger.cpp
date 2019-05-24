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

    for (unsigned i = 0; i < BUS_OWNER_COUNT; i++) {

        visualize[i] = true;
        setColor((BusOwner)i, i % 7);
    }
}

DMADebuggerInfo
DmaDebugger::getInfo()
{
    DMADebuggerInfo result;

    pthread_mutex_lock(&lock);

    result.enabled = enabled;
    result.opacity = opacity;

    for (uint8_t i = 0; i < BUS_OWNER_COUNT; i++) {

        RgbColor color = getColor((BusOwner)i);
        result.visualize[i] = visualize[i];
        result.colorRGB[i][0] = color.r;
        result.colorRGB[i][1] = color.g;
        result.colorRGB[i][2] = color.b;
    }

    pthread_mutex_unlock(&lock);

    return result;
}

bool
DmaDebugger::isVisualized(BusOwner owner)
{
    return true;
}

void
DmaDebugger::setVisualized(BusOwner owner, bool value)
{
    assert(isBusOwner(owner));
    visualize[owner] = value;
}

RgbColor
DmaDebugger::getColor(BusOwner owner)
{
    assert(isBusOwner(owner));
    return debugColor[owner][4];
}

void
DmaDebugger::setColor(BusOwner owner, uint8_t nr)
{
    assert(isBusOwner(owner));
    assert(nr < 7);

    const RgbColor color[7] = {
        RgbColor(1.0,0.4,0.4),
        RgbColor(1.0,1.0,0.4),
        RgbColor(0.4,1.0,0.4),
        RgbColor(0.4,1.0,1.0),
        RgbColor(0.4,0.4,1.0),
        RgbColor(1.0,0.4,1.0),
        RgbColor(0.7,0.7,0.7)
    };

    setColor(owner, color[nr]);
}


void
DmaDebugger::setColor(BusOwner owner, RgbColor color)
{
    assert(isBusOwner(owner));

    // Store the original color at an unused location
    debugColor[owner][4] = color;

    // Compute the color variants that are used for drawing
    debugColor[owner][0] = color.shade(0.3);
    debugColor[owner][1] = color.shade(0.1);
    debugColor[owner][2] = color.tint(0.1);
    debugColor[owner][3] = color.tint(0.3);
}

void
DmaDebugger::setColor(BusOwner owner, double r, double g, double b)
{
    assert(isBusOwner(owner));
    setColor(owner, RgbColor(r, g, b));
}

double
DmaDebugger::getOpacity()
{
    return opacity;
}

void
DmaDebugger::setOpacity(double value)
{
    assert(value >= 0.0 && value <= 1.0);
    opacity = value;
}

void
DmaDebugger::computeOverlay()
{
    // Only proceed if the debugger is enabled
    if (!enabled) return;

    BusOwner *owners = amiga->agnus.busOwner;
    GpuColor col;
    int *ptr = amiga->denise.pixelAddr(0);

    for (int i = 0; i < HPOS_COUNT; i++, ptr += 2) {

        int data = rand() % 16;
        int data1 = (data >> 2) & 0x3;
        int data2 = data & 0x3;

        BusOwner owner = owners[i];
        owners[i] = BUS_NONE;

        switch (owner) {

            case BUS_DISK:
            case BUS_AUDIO:
            case BUS_SPRITE:
            case BUS_BITPLANE:
            case BUS_BLITTER:
            case BUS_COPPER:

                if (visualize[owner]) {

                    col = GpuColor(ptr[0]).mix(debugColor[owner][data1], opacity);
                    ptr[0] = col.rawValue;

                    col = GpuColor(ptr[1]).mix(debugColor[owner][data2], opacity);
                    ptr[1] = col.rawValue;
                }
                break;

            default:
                break;
        }
    }
}
