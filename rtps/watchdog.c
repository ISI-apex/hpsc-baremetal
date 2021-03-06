#include <stdint.h>

#include "console.h"
#include "panic.h"
#include "gic.h"
#include "hwinfo.h"
#include "arm.h"
#include "subsys.h"
#include "wdt.h"

#include "watchdog.h"

static struct wdt *wdt;
static struct wdt **wdt_ptr;

static void handle_timeout(struct wdt *wdt, unsigned stage, void *arg)
{
    printf("watchdog: expired\r\n");
    // nothing to do: main loop will return from WFI/WFE and kick
}

int watchdog_init(struct wdt **wdt_ptr)
{
    volatile unsigned expired_stage = 0;

    // Each timer can be tested only from its associated core
    unsigned core = self_core_id();
    ASSERT(core < RTPS_R52_NUM_CORES);

    uintptr_t base = core ?
        WDT_RTPS_R52_1_RTPS_BASE : WDT_RTPS_R52_0_RTPS_BASE;

    wdt = wdt_create_target("RTPS_R52", base,
                            handle_timeout, (void *)&expired_stage);
    if (!wdt)
        return 1;
    *wdt_ptr = wdt;

    wdt_enable(wdt);

    gic_int_enable(PPI_IRQ__WDT, GIC_IRQ_TYPE_PPI, GIC_IRQ_CFG_LEVEL);
    return 0;
}

void watchdog_deinit()
{
    ASSERT(wdt);
    // NOTE: order: ISR might be called during destroy if IRQ is not disabled
    gic_int_disable(PPI_IRQ__WDT, GIC_IRQ_TYPE_PPI);
    wdt_destroy(wdt);
    *wdt_ptr = NULL;
}

void watchdog_kick()
{
    ASSERT(wdt);
    wdt_kick(wdt);
}
