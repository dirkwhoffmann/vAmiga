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

    // By default, visualize all DMA channels
    for (unsigned i = 0; i < BUS_OWNER_COUNT; i++) {
        visualize[i] = true;
    }
    visualize[BUS_NONE] = false;

    // Assign default colors
    setColor(BUS_CPU,      RgbColor((uint8_t)0xFF, 0xFF, 0xFF));
    setColor(BUS_REFRESH,  RgbColor((uint8_t)0xFF, 0x00, 0x00));
    setColor(BUS_DISK,     RgbColor((uint8_t)0x00, 0xFF, 0x00));
    setColor(BUS_AUDIO,    RgbColor((uint8_t)0xFF, 0x00, 0xFF));
    setColor(BUS_BITPLANE, RgbColor((uint8_t)0x00, 0xFF, 0xFF));
    setColor(BUS_SPRITE,   RgbColor((uint8_t)0x00, 0x80, 0xFF));
    setColor(BUS_COPPER,   RgbColor((uint8_t)0xFF, 0xFF, 0x00));
    setColor(BUS_BLITTER,  RgbColor((uint8_t)0xFF, 0x80, 0x00));
}

DMADebuggerInfo
DmaDebugger::getInfo()
{
    DMADebuggerInfo result;

    pthread_mutex_lock(&lock);

    result.enabled = enabled;
    result.opacity = opacity;
    result.overlay = overlay; 

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

void
DmaDebugger::setEnabled(bool value)
{
    if (!enabled && value) {
        enabled = true;
        amiga->putMessage(MSG_DMA_DEBUG_ON);
    }
    if (enabled && !value) {
        enabled = false;
        amiga->putMessage(MSG_DMA_DEBUG_OFF);
    }
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
    // Only proceed if DMA debugging has been turned on
    if (!enabled) return;

    BusOwner *owners = amiga->agnus.busOwner;
    uint16_t *values = amiga->agnus.busValue;
    int *ptr = amiga->denise.pixelEngine.pixelAddr(0);

    for (int i = 0; i < HPOS_CNT; i++, ptr += 4) {

        BusOwner owner = owners[i];

        // Check for the easy case: Nothing to overlay
        if (!visualize[owner]) {

            if (overlay) {
                ptr[0] = GpuColor(ptr[0]).shade(1.0 - opacity).rawValue;
                ptr[1] = GpuColor(ptr[1]).shade(1.0 - opacity).rawValue;
                ptr[2] = GpuColor(ptr[2]).shade(1.0 - opacity).rawValue;
                ptr[3] = GpuColor(ptr[3]).shade(1.0 - opacity).rawValue;
            }
            continue;
        }

        // The not so easy case: Overlay DMA pixels
        GpuColor col0 = debugColor[owner][(values[i] & 0xC000) >> 14];
        GpuColor col1 = debugColor[owner][(values[i] & 0x0C00) >> 10];
        GpuColor col2 = debugColor[owner][(values[i] & 0x00C0) >> 6];
        GpuColor col3 = debugColor[owner][(values[i] & 0x000C) >> 2];

        if (!overlay) {
                col0 = col0.mix(GpuColor(ptr[0]), opacity);
                col1 = col1.mix(GpuColor(ptr[1]), opacity);
                col2 = col2.mix(GpuColor(ptr[2]), opacity);
                col3 = col3.mix(GpuColor(ptr[3]), opacity);
        }

        ptr[0] = col0.rawValue;
        ptr[1] = col1.rawValue;
        ptr[2] = col2.rawValue;
        ptr[3] = col3.rawValue;
    }
}

void
DmaDebugger::vSyncHandler()
{
    // Only proceed if the debugger is enabled
    if (!enabled) return;

    // Clear old data in the next frame's VBLANK area
    int *ptr = amiga->denise.pixelEngine.frameBuffer->data;
    for (int row = 0; row < VBLANK_CNT; row++) {
        for (int col = 0; col <= LAST_PIXEL; col++) {
            ptr[row * HPIXELS + col] = PixelEngine::rgbaVBlank;
        }
    }
}

