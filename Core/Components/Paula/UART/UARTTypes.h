// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------
/// @file

#pragma once

#include "BasicTypes.h"

namespace vamiga {

/// State of the UART.
typedef struct
{
    u16 serper;                             ///< Serial period register (SERPER)
    isize baudRate;                         ///< Baud rate
    u16 receiveBuffer;                      ///< Receive buffer
    u16 receiveShiftReg;                    ///< Receive shift register
    u16 transmitBuffer;                     ///< Transmit buffer
    u16 transmitShiftReg;                   ///< Transmit shift register
}
UARTInfo;

}
