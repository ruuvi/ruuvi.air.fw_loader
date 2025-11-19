/**
 * @copyright Ruuvi Innovations Ltd, license BSD-3-Clause.
 */

#include "fwloader_segger_rtt.h"
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#if defined(CONFIG_USE_SEGGER_RTT)
#include <SEGGER_RTT.h>
#endif
#include "fwloader_led_err.h"

LOG_MODULE_DECLARE(fw_loader, LOG_LEVEL_INF);

#if defined(CONFIG_USE_SEGGER_RTT)
#define RTT_DATA_SRAM_NODE DT_NODELABEL(rtt_data)
#define RTT_DATA_SRAM_ADDR DT_REG_ADDR(RTT_DATA_SRAM_NODE)
#define RTT_DATA_SRAM_SIZE DT_REG_SIZE(RTT_DATA_SRAM_NODE)
#endif

#define FWLOADER_ASSERT(test, fmt, ...) \
    do \
    { \
        if (!(test)) \
        { \
            printk("fw_loader: ASSERTION FAIL @ %s:%d\n", __FILE__, __LINE__); \
            printk("\t" fmt "\n", ##__VA_ARGS__); \
            (void)arch_irq_lock(); \
            fwloader_led_err_blink_red_led(NUM_RED_LED_BLINKS_ON_ASSERT); \
            CODE_UNREACHABLE; \
        } \
    } while (false)

void
fwloader_segger_rtt_check_data_location_and_size(void)
{
#if defined(CONFIG_USE_SEGGER_RTT)
    extern uint8_t __rtt_buff_data_start[]; // NOSONAR
    extern uint8_t __rtt_buff_data_end[];   // NOSONAR
    const size_t   rtt_buff_size = (size_t)((uintptr_t)__rtt_buff_data_end - (uintptr_t)__rtt_buff_data_start);
    LOG_INF("fw_loader: RTT data address: %p", __rtt_buff_data_start);
    LOG_INF("fw_loader: RTT data size: 0x%zx", rtt_buff_size);
    FWLOADER_ASSERT(
        (uintptr_t)__rtt_buff_data_start == CONFIG_SRAM_BASE_ADDRESS,
        "__rtt_buff_data_start != CONFIG_SRAM_BASE_ADDRESS, 0x%p != 0x%08" PRIx32,
        __rtt_buff_data_start,
        CONFIG_SRAM_BASE_ADDRESS);
    FWLOADER_ASSERT(
        0 == (rtt_buff_size % 0x1000),
        ""
        "RTT buffer size is not aligned to 4kB, size=0x%zx",
        rtt_buff_size);

    FWLOADER_ASSERT(
        (uintptr_t)__rtt_buff_data_start == RTT_DATA_SRAM_ADDR,
        "__rtt_buff_data_start != RTT_DATA_SRAM_ADDR, 0x%p != 0x%08" PRIx32,
        __rtt_buff_data_start,
        RTT_DATA_SRAM_ADDR);
    FWLOADER_ASSERT(
        rtt_buff_size == RTT_DATA_SRAM_SIZE,
        "__rtt_buff_data_start != RTT_DATA_SRAM_ADDR, 0x%08" PRIx32 " != 0x%08" PRIx32,
        rtt_buff_size,
        RTT_DATA_SRAM_SIZE);
#endif // defined(CONFIG_USE_SEGGER_RTT)
}
