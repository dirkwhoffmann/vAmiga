/// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "vaconfig.h"
#include "Paula.h"

namespace vamiga {

template <isize nr> void
StateMachine<nr>::serviceEvent()
{
    assert(agnus.id[SLOT_CH0+nr] == CHX_PERFIN);

    logmsg(LOG_AUD, "CHX_PERFIN state = %ld\n", state);

    switch (state) {

        case 0b010:

            move_010_011();
            return;

        case 0b011:

            (AUDxON() || !AUDxIP()) ? move_011_010() : move_011_000();
            return;

        default:
            fatalError;
    }
}

template void StateMachine<0>::serviceEvent();
template void StateMachine<1>::serviceEvent();
template void StateMachine<2>::serviceEvent();
template void StateMachine<3>::serviceEvent();

}
