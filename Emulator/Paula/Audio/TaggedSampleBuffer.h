// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _TAGGED_SAMPLE_BUFFER_H
#define _TAGGED_SAMPLE_BUFFER_H

#include "Utils.h"
#include "Buffers.h"

/* This buffer type is used to temporarily store the generated sound samples as
 * they are produced by the state machine. Note that the state machine doesn't
 * output samples at a constant sampling rate. Instead, a new sound sample is
 * generated whenever the period counter underflows. To preserve this timing
 * information, each sample is tagged by the cycle it was produced.
 */

// struct TaggedSampleBuffer : RingBuffer <TaggedSample, 0x20000> {
struct TaggedSampleBuffer : RingBuffer <TaggedSample, 256> {

};

#endif

