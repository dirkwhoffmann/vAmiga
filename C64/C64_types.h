//  Created by Dirk Hoffmann

#ifndef C64_TYPES_H
#define C64_TYPES_H

#include "CPU_types.h"
#include "Memory_types.h"
#include "TOD_types.h"
#include "CIA_types.h"
#include "VIC_types.h"
#include "SID_types.h"
#include "ControlPort_types.h"
#include "ExpansionPort_types.h"
#include "Cartridge_types.h"
#include "Drive_types.h"
#include "Disk_types.h"
#include "Mouse_types.h"
#include "File_types.h"

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
    SIDModel sid;
    bool sidFilter;
    GlueLogic glue;
    RamInitPattern pattern;
} C64Configuration;

//! @brief    Configurations of standard C64 models
/*! @note     sidFilter should be true for all known configurations. We have
 *            disabled them by default, because the filter emulation is broken
 *            in the currently used reSID code. Once the reSID bug has been
 *            fixed, it should be set to true again.
 */
static const C64Configuration configurations[] = {
    
    // C64 PAL
    { PAL_6569_R3, false, MOS_6526, true, MOS_6581, false, GLUE_DISCRETE, INIT_PATTERN_C64 },
    
    // C64_II_PAL
    { PAL_8565, true, MOS_8521, false, MOS_8580, false, GLUE_CUSTOM_IC, INIT_PATTERN_C64C },
    
    // C64_OLD_PAL
    { PAL_6569_R1, false, MOS_6526, true, MOS_6581, false, GLUE_DISCRETE, INIT_PATTERN_C64 },

    // C64_NTSC
    { NTSC_6567, false, MOS_6526, false, MOS_6581, false, GLUE_DISCRETE, INIT_PATTERN_C64 },

    // C64_II_NTSC
    { NTSC_8562, true, MOS_8521, true, MOS_8580, false, GLUE_CUSTOM_IC, INIT_PATTERN_C64C },
    
    // C64_OLD_NTSC
    { NTSC_6567_R56A, false, MOS_6526, false, MOS_6581, false, GLUE_DISCRETE, INIT_PATTERN_C64 }
};



#endif
