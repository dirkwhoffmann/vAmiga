//  Created by Dirk Hoffmann

#ifndef C64_TYPES_H
#define C64_TYPES_H

#include "Memory_types.h"
#include "TOD_types.h"
#include "CIA_types.h"
#include "VIC_types.h"
#include "ControlPort_types.h"

//! @brief    C64 model
typedef enum {
    C64_PAL,
    C64_II_PAL,
    C64_OLD_PAL,
    C64_NTSC,
    C64_II_NTSC,
    C64_OLD_NTSC,
    C64_CUSTOM
} C64Model;

inline bool isC64Model(C64Model model) {
    return model >= C64_PAL && model <= C64_OLD_NTSC;
}

//! @brief    C64 configuration
typedef struct {
    VICModel vic;
    bool grayDotBug;
    CIAModel cia;
    bool timerBBug;
    bool sidFilter;
    GlueLogic glue;
    RamInitPattern pattern;
} C64Configuration;




#endif
