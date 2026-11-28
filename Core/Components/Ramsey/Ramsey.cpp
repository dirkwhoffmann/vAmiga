// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "Ramsey.h"
#include "Amiga.h"
#include "utl/io.h"

namespace vamiga {

i64
Ramsey::getOption(Opt option) const
{
    switch (option) {

        case Opt::RAMSEY_REVISION:  return (long)config.revision;

        default:
            fatalError;
    }
}

void
Ramsey::checkOption(Opt option, i64 value)
{
    switch (option) {

        case Opt::RAMSEY_REVISION:

            if (!isPoweredOff()) {
                throw CoreError(CoreError::OPT_LOCKED);
            }
            if (!RamseyRevisionEnum::isValid(value)) {
                throw CoreError(CoreError::OPT_INV_ARG, RamseyRevisionEnum::keyList());
            }
            return;

        default:
            throw CoreError(CoreError::OPT_UNSUPPORTED);
    }
}

void
Ramsey::setOption(Opt option, i64 value)
{
    switch (option) {

        case Opt::RAMSEY_REVISION:

            config.revision = (RamseyRevision)value;
            mem.updateMemSrcTables();
            return;

        default:
            fatalError;
    }
}

void
Ramsey::_dump(Category category, std::ostream &os) const
{
    using namespace utl;

    if (category == Category::Config) {

        dumpConfig(os);
    }

    if (category == Category::State || category == Category::Registers) {

        os << tab("Revision");
        os << hex(revisionId()) << std::endl;
        os << tab("Control");
        os << hex(control) << std::endl;
        os << tab("Page mode");
        os << bol(control & 0x01) << std::endl;
        os << tab("Burst mode");
        os << bol(control & 0x02) << std::endl;
        os << tab("Wrap");
        os << bol(control & 0x04) << std::endl;
        os << tab("DRAM size");
        os << ((control & 0x08) ? "1M x 4" : "256K x 4") << std::endl;
        os << tab("Skip mode");
        os << bol(control & 0x10) << std::endl;
        os << tab("Refresh rate");
        os << dec((control >> 5) & 0b11) << std::endl;
        os << tab("Test mode");
        os << bol(control & 0x80) << std::endl;
    }
}

u8
Ramsey::revisionId() const
{
    switch (config.revision) {

        case RamseyRevision::REV04:  return 0x0D;
        case RamseyRevision::REV07:  return 0x0F;

        default:
            return 0x00;
    }
}

u8
Ramsey::spypeek8(u32 addr) const
{
    switch (addr & 0xFFFF) {

        case 0x0003: return control;
        case 0x0043: return revisionId();

        default:
            return 0;
    }
}

void
Ramsey::poke8(u32 addr, u8 value)
{
    logmsg(LOG_RMY, "poke8(%x, %02x)\n", addr, value);

    switch (addr & 0xFFFF) {

        case 0x0003: control = value; break;

        default:
            break;
    }
}

}
