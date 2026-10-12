#ifndef IOCTL_H
#define IOCTL_H

#include <stdint.h>

int32_t sys_ioctl(int32_t fd, uint32_t op, uint32_t arg0, uint32_t arg1);

#endif
