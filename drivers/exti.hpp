#pragma once
#include "gd32e23x.h"

class Exti {
private:
    rcu_periph_enum m_periph_clock; // 使能对应 GPIO 端口的时钟，例如 RCU_GPIOA, RCU_GPIOB 等
    
    uint32_t m_port_source; // GPIO 端口来源，例如 GPIOA, GPIOB 等
    uint32_t m_pin_source;  // GPIO 引脚来源，例如 GPIO_PIN_0, GPIO_PIN_1 等
    exti_trig_type_enum m_trigger;     // 触发方式，例如 EXTI_TRIGGER_RISING, EXTI_TRIGGER_FALLING 等

    void init();
public:
    Exti(rcu_periph_enum periph_clock, uint32_t port_source, uint32_t pin_source, exti_trig_type_enum trigger);
    ~Exti() {}
};