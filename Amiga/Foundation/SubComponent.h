// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _SUB_COMPONENT_INC
#define _SUB_COMPONENT_INC

#include "HardwareComponent.h"

//
// Forward declarations of all sub components
//

class Memory;
class CPU;
class CIAA;
class CIAB;
class Agnus;
class Denise;
class Paula;
class ZorroManager;
class ControlPort;

/* Base class for all sub components
 * This class provides references to all Amiga components
 */
class SubComponent : public HardwareComponent {

protected:

    Amiga& amiga;
    Agnus& agnus;

public:

    SubComponent(Amiga& ref);
};

#endif
