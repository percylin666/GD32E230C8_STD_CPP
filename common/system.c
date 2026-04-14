#include "system.h"

// 实现微秒级延时
void sys_delay_us(uint32_t nus) {
    uint32_t ticks;
    uint32_t tcnt, told, tnow;
    uint32_t reload = SysTick->LOAD; // 读取 SysTick 的重载值

    // 计算延时所需的总 ticks 数
    // SystemCoreClock / 1000000 是 1us 对应的 tick 数
    ticks = nus * (SystemCoreClock / 1000000);
    tcnt = 0;
    told = SysTick->VAL; // 记录当前 SysTick 的计数值

    while (1) {
        tnow = SysTick->VAL; // 读取当前 SysTick 的计数值
        if (tnow != told) {
            if (tnow < told) {
                tcnt += told - tnow; // 正常计数
            } else {
                tcnt += reload - tnow + told; // 发生了计数器重载
            }
            told = tnow; // 更新上次的计数值

            if (tcnt >= ticks) {
                break; // 达到或超过所需的 ticks，退出循环
            }
        }
    }
}

void sys_delay_ms(uint32_t nms) {
    // 实现毫秒级延时
    while (nms--) {
        sys_delay_us(1000);
    }
}