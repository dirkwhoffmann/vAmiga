// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Frame.h"

isize
Frame::numLines() const
{
    switch (type) {

        case LINE_PAL:  return lof ? 313 : 312;
        default:        return lof ? 263 : 262;
    }
}

isize
Frame::lastLine() const
{
    switch (type) {

        case LINE_PAL:  return lof ? 312 : 311;
        default:        return lof ? 262 : 261;
    }
}

isize
Frame::prevNumLines() const
{
    switch (type) {

        case LINE_PAL:  return prevlof ? 313 : 312;
        default:        return prevlof ? 263 : 262;
    }
}

isize
Frame::prevLastLine() const
{
    switch (type) {

        case LINE_PAL:  return prevlof ? 312 : 311;
        default:        return prevlof ? 262 : 261;
    }
}
