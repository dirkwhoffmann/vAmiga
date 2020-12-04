// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DATABASE_H
#define _DATABASE_H

#include "FileTypes.h"

//
// Bootblock signatures
//

typedef struct
{
    BootBlockIdentifier id;
    BootBlockType type;
    const char *name;
    u16 signature[14];
}
BBRecord;

const BBRecord bbRecord[] = {
    
    {
        BB_KICK_1_3, BB_STANDARD,
        "AmigaDOS Standard 1.x Bootblock",
        { 4,170,6,36,22,103,40,100,48,97,17,174,1,79 }
    },
    {
        BB_KICK_2_0, BB_STANDARD,
        "AmigaDOS Standard - 2.0 FFS",
        { 3,1,4,227,29,233,68,108,506,0,760,0,44,255 }
    },
    {
        BB_SCA_VIRUS, BB_VIRUS,
        "SCA Virus",
        { 800,101,822,97,900,115,841,71,217,231,9,72,435,7 }
    },
    {
        BB_BYTE_BANDIT_VIRUS, BB_VIRUS,
        "Byte Bandit 1 Virus",
        { 18,114,25,66,32,66,35,100,335,252,513,196,639,188 }
    }
};

const BBRecord *bbSeek(const u8 *data);
BootBlockIdentifier bbID(const u8 *data);
BootBlockType bbType(const u8 *data);
const char *bbName(const u8 *data);

const BBRecord *bbSeek(const u8 *data)
{
    size_t i,j;
    
    for (i = 0; i < sizeof(bbRecord) / sizeof(BBRecord); i++) {

        for (j = 0; i < 7; j++) {

            u16 pos = bbRecord[i].signature[2*j];
            u16 val = bbRecord[i].signature[2*j + 1];
            if (data[pos] != val) break;
        }
        if (j == 7) return &bbRecord[i];
    }
    return nullptr;
}

BootBlockIdentifier bbID(const u8 *data)
{
    const BBRecord *rec = bbSeek(data);
    return rec ? rec->id : BB_UNKNOWN;
}

BootBlockType bbType(const u8 *data)
{
    const BBRecord *rec = bbSeek(data);
    return rec ? rec->type : BB_STANDARD;
}

const char *bbName(const u8 *data)
{
    const BBRecord *rec = bbSeek(data);
    return rec ? rec->name : "";
}

#endif
