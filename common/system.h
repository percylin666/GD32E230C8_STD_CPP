#ifndef __SYSTEM_H
#define __SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gd32e23x.h"

void sys_delay_us(uint32_t nus);
void sys_delay_ms(uint32_t nms);

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_H */