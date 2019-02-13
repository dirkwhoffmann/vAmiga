/*!
 * @file        basic.cpp
 * @author      Dirk W. Hoffmann, www.dirkwhoffmann.de
 * @copyright   Dirk W. Hoffmann. All rights reserved.
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "basic.h"

struct timeval t;
// long tv_base = ((void)gettimeofday(&t,NULL), t.tv_sec);

/*
void translateToUnicode(const char *petscii, uint16_t *unichars, uint16_t base, size_t max)
{
    assert(petscii != NULL);
    assert(unichars != NULL);
    
    unsigned i;
    size_t len = MIN(strlen(petscii), max);
    
    for (i = 0; i < len; i++) {
        unichars[i] = base + (uint16_t)petscii[i];
    }
    unichars[i] = 0;
}

size_t
strlen16(const uint16_t *unichars)
{
    size_t count = 0;
    
    if (unichars)
        while(unichars[count]) count++;
    
    return count;
}
*/





uint8_t 
localTimeSec()
{
	time_t t = time(NULL);
	struct tm *loctime = localtime(&t);
	return (uint8_t)loctime->tm_sec;
}

uint8_t 
localTimeMinute()
{
	time_t t = time(NULL);
	struct tm *loctime = localtime(&t);
	return (uint8_t)loctime->tm_min;
}

uint8_t 
localTimeHour()
{
	time_t t = time(NULL);
	struct tm *loctime = localtime(&t);
	return (uint8_t)loctime->tm_hour;
}

	
void 
sleepMicrosec(unsigned usec)
{		
	if (usec > 0 && usec < 1000000) {
		usleep(usec);
	}
}

int64_t
sleepUntil(uint64_t kernelTargetTime, uint64_t kernelEarlyWakeup)
{
    uint64_t now = mach_absolute_time();
    int64_t jitter;
    
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

uint64_t
fnv_1a(uint8_t *addr, size_t size)
{
    if (addr == NULL || size == 0) return 0;
    
    uint64_t basis = 0xcbf29ce484222325;
    uint64_t prime = 0x100000001b3;
    uint64_t hash = basis;

    for (size_t i = 0; i < size; i++) {
        hash = (hash ^ (uint64_t)addr[i]) * prime;
    }
    
    printf("hash = %lld\n", hash);
    return hash;
}
