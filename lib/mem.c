#include <stdint.h>

#include "panic.h"
#include "bit.h"

#include "mem.h"

void bzero(void *p, size_t sz)
{
    ASSERT(ALIGNED(p, 2)); /* log2(sizeof(uint32_t)) */

    uint32_t *wp = p;
    while (sz >= sizeof(uint32_t)) {
        *wp++ = 0;
        sz -= sizeof(uint32_t);
    }
    uint8_t *bp = (uint8_t *)wp;
    while (sz-- > 0)
        *bp++ = 0;
}

void *memset(void *s, int c, size_t n)
{
    uint8_t *ptr = s;
    for (int i = 0; i < n; ++i)
        ptr[i] = (uint8_t)c;
    return s;
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
    uint8_t *bdest = dest;
    const uint8_t *bsrc = src;
    while (n) {
        *bdest++ = *bsrc++;
        --n;
    }
    return dest;
}

volatile void *vmem_set(volatile void *s, int c, unsigned n)
{
    volatile uint8_t *bs = s;
    while (n--)
        *bs++ = (unsigned char) c;
    return s;
}

volatile void *vmem_cpy(volatile void *restrict dest, void *restrict src,
                        unsigned n)
{
    // assume dest and src are word-aligned
    volatile uint32_t *wd;
    uint32_t *ws;
    volatile uint8_t *bd;
    uint8_t *bs;
    for (wd = dest, ws = src; n >= sizeof(*wd); n -= sizeof(*wd))
        *wd++ = *ws++;
    for (bd = (volatile uint8_t *) wd, bs = (uint8_t *) ws; n > 0; n--)
        *bd++ = *bs++;
    return dest;
}

void *mem_vcpy(void *restrict dest, volatile void *restrict src, unsigned n)
{
    // assume dest and src are word-aligned
    uint32_t *wd;
    volatile uint32_t *ws;
    uint8_t *bd;
    volatile uint8_t *bs;
    for (wd = dest, ws = src; n >= sizeof(*wd); n -= sizeof(*wd))
        *wd++ = *ws++;
    for (bd = (uint8_t *) wd, bs = (volatile uint8_t *) ws; n > 0; n--)
        *bd++ = *bs++;
    return dest;
}
