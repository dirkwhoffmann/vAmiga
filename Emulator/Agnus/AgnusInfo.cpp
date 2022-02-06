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
#include "IOUtils.h"

void
Agnus::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category & dump::Config) {
    
        os << tab("Chip Revison");
        os << AgnusRevisionEnum::key(config.revision) << std::endl;
        os << tab("Slow Ram mirror");
        os << bol(config.slowRamMirror) << std::endl;
    }

    if (category & dump::State) {
        
        os << tab("Clock");
        os << dec(clock) << std::endl;
        os << tab("Frame");
        os << dec(frame.nr) << std::endl;
        os << tab("LOF");
        os << dec(frame.lof) << std::endl;
        os << tab("LOF in previous frame");
        os << dec(frame.prevlof) << std::endl;
        os << tab("Beam position");
        os << "(" << dec(pos.v) << "," << dec(pos.h) << ")" << std::endl;
        os << tab("Latched position");
        os << "(" << dec(latchedPos.v) << "," << dec(latchedPos.h) << ")" << std::endl;
        os << tab("scrollOdd");
        os << dec(scrollOdd) << std::endl;
        os << tab("scrollEven");
        os << dec(scrollEven) << std::endl;
        os << tab("BLS signal");
        os << bol(bls) << std::endl;
        
        sequencer.dump(dump::State, os);
    }

    if (category & dump::Registers) {
        
        sequencer.dump(dump::Registers, os);
        
        os << tab("DMACON");
        os << hex(dmacon) << std::endl;
                
        os << tab("BPLCON0, BPLCON1");
        os << hex(bplcon0) << ' ' << hex(bplcon1) << ' ' << std::endl;

        os << tab("BPL1MOD, BPL2MOD");
        os << dec(bpl1mod) << ' ' << dec(bpl2mod) << ' ' << std::endl;
    
        os << tab("BPL0PT - BPL2PT");
        os << hex(bplpt[0]) << ' ' << hex(bplpt[1]) << ' ';
        os << hex(bplpt[2]) << ' ' << ' ' << std::endl;
        os << tab("BPL3PT - BPL5PT");
        os << hex(bplpt[3]) << ' ' << hex(bplpt[4]) << ' ';
        os << hex(bplpt[5]) << std::endl;

        os << tab("SPR0PT - SPR3PT");
        os << hex(sprpt[0]) << ' ' << hex(sprpt[1]) << ' ';
        os << hex(sprpt[2]) << ' ' << hex(sprpt[3]) << ' ' << std::endl;
        os << tab("SPR4PT - SPR7PT");
        os << hex(sprpt[4]) << ' ' << hex(sprpt[5]) << ' ';
        os << hex(sprpt[5]) << ' ' << hex(sprpt[7]) << ' ' << std::endl;

        os << tab("AUD0PT - AUD3PT");
        os << hex(audpt[0]) << ' ' << hex(audpt[1]) << ' ';
        os << hex(audpt[2]) << ' ' << hex(audpt[3]) << ' ' << std::endl;

        os << tab("DSKPT");
        os << hex(dskpt) << std::endl;
    }
    
    if (category & dump::Bus) {
        
        for (isize i = 0; i < HPOS_CNT; i++) {
            
            isize cycle = (i / 6) + (i % 6) * ((HPOS_CNT + 1) / 6);
            
            string key = std::to_string(cycle) + ":";
            os << std::left << std::setw(5) << key;

            string value = BusOwnerEnum::key(busOwner[cycle]);
            os << std::left << std::setw(8) << value;
            
            if (i % 6 == 5) { os << std::endl; } else { os << "  "; }
        }
        os << std::endl;
    }
    
    if (category & dump::Dma) {
        
        sequencer.dump(dump::Dma, os);
    }

    if (category & dump::Signals) {
        
        sequencer.dump(dump::Signals, os);
    }
}

void
Agnus::_inspect() const
{
    SYNCHRONIZED
    
    info.vpos     = pos.v;
    info.hpos     = pos.h;
    
    info.dmacon   = dmacon;
    info.bplcon0  = bplcon0;
    info.ddfstrt  = sequencer.ddfstrt;
    info.ddfstop  = sequencer.ddfstop;
    info.diwstrt  = sequencer.diwstrt;
    info.diwstop  = sequencer.diwstop;
    
    info.bpl1mod  = bpl1mod;
    info.bpl2mod  = bpl2mod;
    info.bltamod  = blitter.bltamod;
    info.bltbmod  = blitter.bltbmod;
    info.bltcmod  = blitter.bltcmod;
    info.bltdmod  = blitter.bltdmod;
    info.bltcon0  = blitter.bltcon0;
    info.bls      = bls;
    
    info.coppc0   = copper.coppc0 & ptrMask;
    info.dskpt    = dskpt & ptrMask;
    info.bltpt[0] = blitter.bltapt & ptrMask;
    info.bltpt[1] = blitter.bltbpt & ptrMask;
    info.bltpt[2] = blitter.bltcpt & ptrMask;
    info.bltpt[3] = blitter.bltdpt & ptrMask;
    for (isize i = 0; i < 6; i++) info.bplpt[i] = bplpt[i] & ptrMask;
    for (isize i = 0; i < 4; i++) info.audpt[i] = audpt[i] & ptrMask;
    for (isize i = 0; i < 4; i++) info.audlc[i] = audlc[i] & ptrMask;
    for (isize i = 0; i < 8; i++) info.sprpt[i] = sprpt[i] & ptrMask;
}

void
Agnus::clearStats()
{
    stats = { };
}

void
Agnus::updateStats()
{
    constexpr double w = 0.5;
    
    double copperUsage = stats.usage[BUS_COPPER];
    double blitterUsage = stats.usage[BUS_BLITTER];
    double diskUsage = stats.usage[BUS_DISK];
    
    double audioUsage =
    stats.usage[BUS_AUD0] +
    stats.usage[BUS_AUD1] +
    stats.usage[BUS_AUD2] +
    stats.usage[BUS_AUD3];

    double spriteUsage =
    stats.usage[BUS_SPRITE0] +
    stats.usage[BUS_SPRITE1] +
    stats.usage[BUS_SPRITE2] +
    stats.usage[BUS_SPRITE3] +
    stats.usage[BUS_SPRITE4] +
    stats.usage[BUS_SPRITE5] +
    stats.usage[BUS_SPRITE6] +
    stats.usage[BUS_SPRITE7];
    
    double bitplaneUsage =
    stats.usage[BUS_BPL1] +
    stats.usage[BUS_BPL2] +
    stats.usage[BUS_BPL3] +
    stats.usage[BUS_BPL4] +
    stats.usage[BUS_BPL5] +
    stats.usage[BUS_BPL6];

    stats.copperActivity = w * stats.copperActivity + (1 - w) * copperUsage;
    stats.blitterActivity = w * stats.blitterActivity + (1 - w) * blitterUsage;
    stats.diskActivity = w * stats.diskActivity + (1 - w) * diskUsage;
    stats.audioActivity = w * stats.audioActivity + (1 - w) * audioUsage;
    stats.spriteActivity = w * stats.spriteActivity + (1 - w) * spriteUsage;
    stats.bitplaneActivity = w * stats.bitplaneActivity + (1 - w) * bitplaneUsage;
    
    for (isize i = 0; i < BUS_COUNT; i++) stats.usage[i] = 0;
}
