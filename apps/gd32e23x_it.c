#include "gd32e23x_it.h"

uint8_t g_systick_flag = 0;
// uint8_t g_timer2_flag = 0;
uint8_t g_ext0_flag = 0; // EXTI 线 0 的标志位

void SysTick_Handler(void) {
    g_systick_flag = 1; // 设置标志位，表示 SysTick 中断发生
}

void EXTI0_1_IRQHandler(void) {
    if (RESET != exti_interrupt_flag_get(EXTI_0)) {
        /* 用户自定义任务 */
        g_ext0_flag = 1;
        exti_interrupt_flag_clear(EXTI_0); // 清除 EXTI 线 0 的中断标志位
    }
}

void EXTI2_3_IRQHandler(void) {
    // 处理 EXTI 线 2 和 3 的中断
}

void EXTI4_15_IRQHandler(void) {
    // 处理 EXTI 线 4 到 15 的中断
}

void TIMER0_IRQHandler(void) {

}

void TIMER2_IRQHandler(void) {
    if (SET == timer_interrupt_flag_get(TIMER2, TIMER_INT_UP)) {
        /* 用户自定义任务 */
        // g_timer2_flag = 1; // 设置标志位，表示 TIMER2 更新中断发生
        timer_interrupt_flag_clear(TIMER2, TIMER_INT_UP);
    }
}
