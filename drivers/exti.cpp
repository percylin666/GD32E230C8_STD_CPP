#include "exti.hpp"

Exti::Exti(rcu_periph_enum periph_clock, uint32_t port_source, uint32_t pin_source, exti_trig_type_enum trigger) :
    m_periph_clock(periph_clock), 
    m_port_source(port_source), 
    m_pin_source(pin_source), 
    m_trigger(trigger) {
    init();
}

void Exti::init() {
// 1. 开启时钟
    rcu_periph_clock_enable(m_periph_clock);
    rcu_periph_clock_enable(RCU_CFGCMP);

    // 2. 配置 GPIO (注意：m_port_source 应为 GPIOA/GPIOB 等)
    gpio_mode_set(m_port_source, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, m_pin_source);

    // 3. 获取引脚的索引值 (从 GPIO_PIN_x 转换为 0-15 的整数)
    uint8_t pin_idx = 0;
    for(uint8_t i = 0; i < 16; i++) {
        if(m_pin_source & (1 << i)) {
            pin_idx = i;
            break;
        }
    }

    // 4. 配置 EXTI 源映射
    // 注意：需要将 GPIOA 转换为 EXTI_SOURCE_GPIOA，引脚转换为 EXTI_SOURCE_PINx
    uint8_t port_idx = ((uint32_t)m_port_source - GPIOA) / 0x400; // 简单地址换算
    syscfg_exti_line_config(port_idx, pin_idx);

    // 5. 初始化 EXTI 线 (使用计算出的 pin_idx)
    uint32_t exti_line = (1 << pin_idx); 
    exti_init((exti_line_enum)exti_line, EXTI_INTERRUPT, m_trigger);
    exti_interrupt_flag_clear((exti_line_enum)exti_line);
    exti_interrupt_enable((exti_line_enum)exti_line);

    // 6. 动态配置 NVIC
    if (pin_idx <= 1) {
        nvic_irq_enable(EXTI0_1_IRQn, 1);
    } else if (pin_idx <= 3) {
        nvic_irq_enable(EXTI2_3_IRQn, 1);
    } else {
        nvic_irq_enable(EXTI4_15_IRQn, 1);
    }
}