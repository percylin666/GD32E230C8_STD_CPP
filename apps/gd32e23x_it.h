#ifndef GD32E23X_IT_H
#define GD32E23X_IT_H

#include "gd32e23x.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t g_systick_flag; // 声明全局变量，用于标志 SysTick 中断
// extern uint8_t g_timer2_flag; // 声明全局变量，用于标志 TIMER2 更新中断
extern uint8_t g_ext0_flag; // 声明全局变量，用于标志 EXTI 线 0 的中断

void SysTick_Handler(void);
void EXTI0_1_IRQHandler(void);
void EXTI2_3_IRQHandler(void);
void EXTI4_15_IRQHandler(void);

void TIMER0_IRQHandler(void);
void TIMER2_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif // GD32E23X_IT_H