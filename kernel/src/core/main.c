#include "minemu/boot.h"
#include "minemu/irq.h"
#include "minemu/trap.h"
#include "minemu/trace.h"
#include "uart.h"
#include "user.h"

void minemu_kernel_main(const struct minemu_boot_info *boot_info) {
    if ((uintptr_t)boot_info != MINEMU_BOOT_INFO_VADDR ||
        boot_info->magic != MINEMU_BOOT_INFO_MAGIC ||
        boot_info->version != MINEMU_ABI_VERSION ||
        boot_info->size != sizeof(*boot_info) ||
        boot_info->system_rom_base != UINT32_C(0x08000000) ||
        boot_info->direct_map_vaddr != UINT32_C(0xc0000000) ||
        boot_info->direct_map_paddr != UINT32_C(0x40000000) ||
        boot_info->direct_map_size != UINT32_C(0x04000000)) {
        minemu_trace_event(UINT32_C(0xb007bad0));
        minemu_fail_stop();
    }
    if (!user_map_system_rom(boot_info)) {
        minemu_trace_event(UINT32_C(0xb007bad0));
        minemu_fail_stop();
    }
    const struct minemu_module_record *module =
        user_find_module(boot_info, USER_MODULE_NAME, sizeof(USER_MODULE_NAME) - 1U);
    if (module == NULL || !user_load_module(boot_info, module)) {
        minemu_trace_event(UINT32_C(0xb007bad0));
        minemu_fail_stop();
    }
    uart_rx_init();
    minemu_enter_user(module->entry_vaddr, USER_STACK_TOP);
}
