// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FRAME_INC
#define _FRAME_INC

struct Frame
{
    // Frame count
    i64 nr;
    
    // Indicates if this frame is drawn in interlace mode
    bool interlaced;

    // The long frame flipflop
    bool lof;
    
    // The number of rasterlines in the current frame
    // TODO: MAKE IT A COMPUTED VALUE ONCE THE LACE FLIP FLOP IS HERE
    i16 numLines;


    template <class T>
    void applyToItems(T& worker)
    {
        worker

        & nr
        & interlaced
        & lof
        & numLines;
    }

    // Advances one frame
    void next(bool lace)
    {
        nr++;
        
        // Update the long frame flipflop
        if ((interlaced = lace)) { lof = !lof; } else { lof = true; }
        
        // Determine if the next frame is a long or a short frame
        numLines = lof ? 313 : 312;
        
     }
    
};

#endif
