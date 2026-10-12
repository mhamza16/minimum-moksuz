#ifndef USER_H
#define USER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "minemu/boot.h"

/* User virtual layout (same as examples/user-mode-hello/kernel/main.c). */
#define USER_IMAGE_BASE UINT32_C(0x00400000)
#define USER_STACK_BOTTOM UINT32_C(0x007ff000)
#define USER_STACK_TOP UINT32_C(0x00800000)
#define USER_MODULE_NAME "minimum-user"

bool user_map_system_rom(const struct minemu_boot_info *boot_info);
const struct minemu_module_record *user_find_module(const struct minemu_boot_info *boot_info,
                                                     const char *name, size_t name_length);
bool user_load_module(const struct minemu_boot_info *boot_info,
                      const struct minemu_module_record *module);
bool user_range_ok(uint32_t vaddr, uint32_t length, bool write);
void minemu_enter_user(uint32_t entry_vaddr, uint32_t stack_top) __attribute__((noreturn));

#endif
