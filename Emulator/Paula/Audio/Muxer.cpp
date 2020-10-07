// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Muxer::Muxer(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("Muxer");
}
    
void
Muxer::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    for (int i = 0; i < 4; i++) {
        
        sampler[i].clear();
        
        /* Some methods assume that the sample buffer is never empty. We assure
         * this by initializing the buffer with a dummy element.
         */
        assert(sampler[i].isEmpty());
        sampler[i].write( TaggedSample { 0, 0 } );
    }
}
