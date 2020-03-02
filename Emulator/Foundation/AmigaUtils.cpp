// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "AmigaUtils.h"

char *
extractFilename(const char *path)
{
    assert(path != NULL);
    
    const char *pos = strrchr(path, '/');
    return pos ? strdup(pos + 1) : strdup(path);
}

char *
extractSuffix(const char *path)
{
    assert(path != NULL);
    
    const char *pos = strrchr(path, '.');
    return pos ? strdup(pos + 1) : strdup("");
}

char *
extractFilenameWithoutSuffix(const char *path)
{
    assert(path != NULL);
    
    char *result;
    char *filename = extractFilename(path);
    char *suffix   = extractSuffix(filename);
    
    if (strlen(suffix) == 0)
        result = strdup(filename);
    else
        result = strndup(filename, strlen(filename) - strlen(suffix) - 1);
    
    free(filename);
    free(suffix);
    return result;
}

bool
checkFileSuffix(const char *filename, const char *suffix)
{
    assert(filename != NULL);
    assert(suffix != NULL);
    
    if (strlen(suffix) > strlen(filename))
        return false;
    
    filename += (strlen(filename) - strlen(suffix));
    if (strcmp(filename, suffix) == 0)
        return true;
    else
        return false;
}

long
getSizeOfFile(const char *filename)
{
    struct stat fileProperties;
    
    if (filename == NULL)
        return -1;
    
    if (stat(filename, &fileProperties) != 0)
        return -1;
    
    return fileProperties.st_size;
}

bool
checkFileSize(const char *filename, long size)
{
    return checkFileSizeRange(filename, size, size);
}

bool
checkFileSizeRange(const char *filename, long min, long max)
{
    long filesize = getSizeOfFile(filename);
    
    if (filesize == -1)
        return false;
    
    if (min > 0 && filesize < min)
        return false;
    
    if (max > 0 && filesize > max)
        return false;
    
    return true;
}

bool
checkFileHeader(const char *filename, const u8 *header)
{
    int i, c;
    bool result = true;
    FILE *file;
    
    assert(filename != NULL);
    assert(header != NULL);
    
    if ((file = fopen(filename, "r")) == NULL)
        return false;
    
    for (i = 0; header[i] != 0; i++) {
        c = fgetc(file);
        if (c != (int)header[i]) {
            result = false;
            break;
        }
    }
    
    fclose(file);
    return result;
}

bool
matchingFileHeader(const char *path, const u8 *header, size_t length)
{
    assert(path != NULL);
    assert(header != NULL);
    
    bool result = true;
    FILE *file;
    
    if ((file = fopen(path, "r")) == NULL)
        return false;
    
    for (unsigned i = 0; i < length; i++) {
        int c = fgetc(file);
        if (c != (int)header[i]) {
            result = false;
            break;
        }
    }
    
    fclose(file);
    return result;
}


bool
matchingBufferHeader(const u8 *buffer, const u8 *header, size_t length)
{
    assert(buffer != NULL);
    assert(header != NULL);
    
    for (unsigned i = 0; i < length; i++) {
        if (header[i] != buffer[i])
            return false;
    }

    return true;
}

void
sleepMicrosec(unsigned usec)
{
    if (usec > 0 && usec < 1000000) {
        usleep(usec);
    }
}

i64
sleepUntil(u64 kernelTargetTime, u64 kernelEarlyWakeup)
{
    u64 now = mach_absolute_time();
    i64 jitter;
    
    if (now > kernelTargetTime) {
        printf("Too slow\n");
        return 0;
    }
    
    // Sleep
    // printf("Sleeping for %lld\n", kernelTargetTime - kernelEarlyWakeup);
    mach_wait_until(kernelTargetTime - kernelEarlyWakeup);
    
    // Count some sheep to increase precision
    unsigned sheep = 0;
    do {
        jitter = mach_absolute_time() - kernelTargetTime;
        sheep++;
    } while (jitter < 0);
    // printf("Counted %d sheep (%lld)\n", sheep, jitter);
    
    return jitter;
}

u32
fnv_1a_32(const u8 *addr, size_t size)
{
    if (addr == NULL || size == 0) return 0;

    u32 hash = fnv_1a_init32();

    for (size_t i = 0; i < size; i++) {
        hash = fnv_1a_it32(hash, (u32)addr[i]);
    }

    return hash;
}

u64
fnv_1a_64(const u8 *addr, size_t size)
{
    if (addr == NULL || size == 0) return 0;
    
    u64 hash = fnv_1a_init64();
    
    for (size_t i = 0; i < size; i++) {
        hash = fnv_1a_it64(hash, (u64)addr[i]);
    }
    
    return hash;
}

u32 crc32(const u8 *addr, size_t size)
{
    if (addr == NULL || size == 0) return 0;

    u32 result = 0;

    // Setup lookup table
    u32 table[256];
    for(int i = 0; i < 256; i++) table[i] = crc32forByte(i);

    // Compute CRC-32 checksum
     for(int i = 0; i < size; i++)
       result = table[(u8)result ^ addr[i]] ^ result >> 8;

    return result;
}

u32 crc32forByte(u32 r)
{
    for(int j = 0; j < 8; ++j)
        r = (r & 1? 0: (u32)0xEDB88320L) ^ r >> 1;
    return r ^ (u32)0xFF000000L;
}

bool releaseBuild()
{
#ifdef NDEBUG
    return true;
#else
    return false;
#endif
}

const char *regName(u32 addr)
{
    assert(IS_EVEN(addr));

    static const char *name[256] = {

        "BLTDDAT",        "DMACONR",        "VPOSR",
        "VHPOSR",         "DSKDATR",        "JOY0DAT",
        "JOY1DAT",        "CLXDAT",         "ADKCONR",
        "POT0DAT",        "POT1DAT",        "POTGOR",
        "SERDATR",        "DSKBYTR",        "INTENAR",
        "INTREQR",        "DSKPTH",         "DSKPTL",
        "DSKLEN",         "DSKDAT",         "REFPTR",
        "VPOSW",          "VHPOSW",         "COPCON",
        "SERDAT",         "SERPER",         "POTGO",
        "JOYTEST",        "STREQU",         "STRVBL",
        "STRHOR",         "STRLONG",        "BLTCON0",
        "BLTCON1",        "BLTAFWM",        "BLTALWM",
        "BLTCPTH",        "BLTCPTL",        "BLTBPTH",
        "BLTBPTL",        "BLTAPTH",        "BLTAPTL",
        "BLTDPTH",        "BLTDPTL",        "BLTSIZE",
        "BLTCON0L (ECS)", "BLTSIZV (ECS)",  "BLTSIZH (ECS)",
        "BLTCMOD",        "BLTBMOD",        "BLTAMOD",
        "BLTDMOD",        "unused",         "unused",
        "unused",         "unused",         "BLTCDAT",
        "BLTBDAT",        "BLTADAT",        "unused",
        "SPRHDAT (ECS)",  "BPLHDAT (AGA)",  "DENISEID (ECS)",
        "DSKSYNC",        "COP1LCH",        "COP1LCL",
        "COP2LCH",        "COP2LCL",        "COPJMP1",
        "COPJMP2",        "COPINS",         "DIWSTRT",
        "DIWSTOP",        "DDFSTRT",        "DDFSTOP",
        "DMACON",         "CLXCON",         "INTENA",
        "INTREQ",         "ADKCON",         "AUD0PTH",
        "AUD0PTL",        "AUD0LEN",        "AUD0PER",
        "AUD0VOL",        "AUD0DAT",        "unused",
        "UNUSED",         "AUD1PTH",        "AUD1PTL",
        "AUD1LEN",        "AUD1PER",        "AUD1VOL",
        "AUD1DAT",        "unused",         "unused",
        "AUD2PTH",        "AUD2PTL",        "AUD2LEN",
        "AUD2PER",        "AUD2VOL",        "AUD2DAT",
        "unused",         "unused",         "AUD3PTH",
        "AUD3PTL",        "AUD3LEN",        "AUD3PER",
        "AUD3VOL",        "AUD3DAT",        "unused",
        "UNUSED",         "BPL1PTH",        "BPL1PTL",
        "BPL2PTH",        "BPL2PTL",        "BPL3PTH",
        "BPL3PTL",        "BPL4PTH",        "BPL4PTL",
        "BPL5PTH",        "BPL5PTL",        "BPL6PTH",
        "BPL6PTL",        "BPL7PTH (AGA)",  "BPL7PTL (AGA)",
        "BPL8PTH (AGA)",  "BPL8PTL (AGA)",  "BPLCON0",
        "BPLCON1",        "BPLCON2",        "BPLCON3 (ECS)",
        "BPL1MOD",        "BPL2MOD",        "BPLCON4 (AGA)",
        "BPLCON4 (AGA)",  "BPL1DAT",        "BPL2DAT",
        "BPL3DAT",        "BPL4DAT",        "BPL5DAT",
        "BPL6DAT",        "BPL7DAT (AGA)",  "BPL8DAT (AGA)",
        "SPR0PTH",        "SPR0PTL",        "SPR1PTH",
        "SPR1PTL",        "SPR2PTH",        "SPR2PTL",
        "SPR3PTH",        "SPR3PTL",        "SPR4PTH",
        "SPR4PTL",        "SPR5PTH",        "SPR5PTL",
        "SPR6PTH",        "SPR6PTL",        "SPR7PTH",
        "SPR7PTL",        "SPR0POS",        "SPR0CTL",
        "SPR0DATA",       "SPR0DATB",       "SPR1POS",
        "SPR1CTL",        "SPR1DATA",       "SPR1DATB",
        "SPR2POS",        "SPR2CTL",        "SPR2DATA",
        "SPR2DATB",       "SPR3POS",        "SPR3CTL",
        "SPR3DATA",       "SPR3DATB",       "SPR4POS",
        "SPR4CTL",        "SPR4DATA",       "SPR4DATB",
        "SPR5POS",        "SPR5CTL",        "SPR5DATA",
        "SPR5DATB",       "SPR6POS",        "SPR6CTL",
        "SPR6DATA",       "SPR6DATB",       "SPR7POS",
        "SPR7CTL",        "SPR7DATA",       "SPR7DATB",
        "COLOR00",        "COLOR01",        "COLOR02",
        "COLOR03",        "COLOR04",        "COLOR05",
        "COLOR06",        "COLOR07",        "COLOR08",
        "COLOR09",        "COLOR10",        "COLOR11",
        "COLOR12",        "COLOR13",        "COLOR14",
        "COLOR15",        "COLOR16",        "COLOR17",
        "COLOR18",        "COLOR19",        "COLOR20",
        "COLOR21",        "COLOR22",        "COLOR23",
        "COLOR24",        "COLOR25",        "COLOR26",
        "COLOR27",        "COLOR28",        "COLOR29",
        "COLOR30",        "COLOR31",        "HTOTAL (ECS)",
        "HSSTOP (ECS)",   "HBSTRT (ECS)",   "HBSTOP (ECS)",
        "VTOTAL (ECS)",   "VSSTOP (ECS)",   "VBSTRT (ECS)",
        "VBSTOP (ECS)",   "SPRHSTRT (AGA)", "SPRHSTOP (AGA)",
        "BPLHSTRT (AGA)", "BPLHSTOP (AGA)", "HHPOSW (AGA)",
        "HHPOSR (AGA)",   "BEAMCON0 (ECS)", "HSSTRT (ECS)",
        "VSSTRT (ECS)",   "HCENTER (ECS)",  "DIWHIGH (ECS)",
        "BPLHMOD (AGA)",  "SPRHPTH (AGA)",  "SPRHPTL (AGA)",
        "BPLHPTH (AGA)",  "BPLHPTL (AGA)",  "unused",
        "unused",         "unused",         "unused",
        "unused",         "unused",         "FMODE (AGA)",
        "NO-OP"
    };

    return name[(addr >> 1) & 0xFF];
}
