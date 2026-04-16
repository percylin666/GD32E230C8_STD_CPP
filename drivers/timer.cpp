#include "timer.hpp"

Timer::Timer(uint32_t timer_periph, uint32_t period_ms, TIMER_MODE mode)
    : m_timer_periph(timer_periph), m_period(period_ms), m_mode(mode) {
    init();
}

void Timer::init() const {
    // 1. 先开启时钟并复位
    if (m_timer_periph == TIMER0) rcu_periph_clock_enable(RCU_TIMER0);
    else if (m_timer_periph == TIMER2) rcu_periph_clock_enable(RCU_TIMER2);
    else if (m_timer_periph == TIMER14) rcu_periph_clock_enable(RCU_TIMER14);

    timer_deinit(m_timer_periph);

    // GPIO 配置
    if (m_mode != TIMER_MODE::NORMAL) {
        if (m_timer_periph == TIMER14) {
            rcu_periph_clock_enable(RCU_GPIOB);
            // 重点检查：此处 AF1 对应 TIMER14_CH0
            gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_14); 
            gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_14);
            gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
        }
        else if (m_timer_periph == TIMER2) {
            rcu_periph_clock_enable(RCU_GPIOB);
            // 重点检查：此处 AF1 对应 TIMER2_CH0
            gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_4); 
            gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_4);
            gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
        }
        else if (m_timer_periph == TIMER0) {
            rcu_periph_clock_enable(RCU_GPIOA);
            // 重点检查：此处 AF2 对应 TIMER0_CH0
            gpio_af_set(GPIOA, GPIO_AF_2, GPIO_PIN_8); 
            gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_8);
            gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
        }
    }
    
    timer_parameter_struct timer_initpara;
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler = 72 - 1; // 预分频  
    timer_initpara.alignedmode = TIMER_COUNTER_EDGE; // 边沿对齐模式
    timer_initpara.counterdirection = TIMER_COUNTER_UP; // 向上计数 
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1; // 时钟不分频
    timer_initpara.period = m_period * 1000 - 1; // 自动重装载值
    timer_initpara.repetitioncounter = 0;
    timer_init(m_timer_periph, &timer_initpara);

    if (m_mode != TIMER_MODE::NORMAL) {
        // 输出比较配置
        timer_oc_parameter_struct timer_ocinitpara;
        timer_channel_output_struct_para_init(&timer_ocinitpara);
        timer_ocinitpara.outputstate = TIMER_CCX_ENABLE; // 使能输出比较
        timer_ocinitpara.ocpolarity = TIMER_OC_POLARITY_HIGH; // 输出比较极性
        timer_channel_output_config(m_timer_periph, TIMER_CH_0, &timer_ocinitpara);

        //配置 PWM 模式及占空比等参数
        /* 设置为 PWM 模式 0，占空比 = (500 / 1000) * 100% = 50% */
        timer_channel_output_pulse_value_config(m_timer_periph, TIMER_CH_0, m_compare_value);
        timer_channel_output_mode_config(m_timer_periph, TIMER_CH_0, TIMER_OC_MODE_PWM0);
        timer_channel_output_shadow_config(m_timer_periph, TIMER_CH_0, TIMER_OC_SHADOW_ENABLE);

        if (m_timer_periph == TIMER0 || m_timer_periph == TIMER2 || m_timer_periph == TIMER14) {
            timer_primary_output_config(m_timer_periph, ENABLE);
        }
    }

    if (m_mode == TIMER_MODE::NORMAL) {
        // 正常模式配置
    
        /* 中断配置 */
        timer_interrupt_flag_clear(m_timer_periph, TIMER_INT_UP);
        timer_interrupt_enable(m_timer_periph, TIMER_INT_UP); // 使能更新中断
        
        // 配置 NVIC 中断优先级等
        if (m_timer_periph == TIMER0) {
            nvic_irq_enable(TIMER0_BRK_UP_TRG_COM_IRQn, 1);
        } else if (m_timer_periph == TIMER2) {
            nvic_irq_enable(TIMER2_IRQn, 1);
        } else if (m_timer_periph == TIMER5) {
            nvic_irq_enable(TIMER5_IRQn, 1);
        } else if (m_timer_periph == TIMER13) {
            nvic_irq_enable(TIMER13_IRQn, 1);
        } else if (m_timer_periph == TIMER14) {
            nvic_irq_enable(TIMER14_IRQn, 1);
        } else if (m_timer_periph == TIMER15) {
            nvic_irq_enable(TIMER15_IRQn, 1);
        } else if (m_timer_periph == TIMER16) {
            nvic_irq_enable(TIMER16_IRQn, 1);
        }
    }

    timer_enable(m_timer_periph); // 启动定时器
}

void Timer::setCompareValue(uint32_t compare_value) {
    m_compare_value = compare_value;
    if (m_mode != TIMER_MODE::NORMAL) {
        timer_channel_output_pulse_value_config(m_timer_periph, TIMER_CH_0, m_compare_value);
    }
}

void Timer::start() const {
    // 启动定时器
    timer_enable(m_timer_periph);
}

void Timer::stop() const {
    // 停止定时器
    timer_disable(m_timer_periph);
}

void Timer::reset() const {
    // 重置定时器
    timer_disable(m_timer_periph);
    timer_counter_value_config(m_timer_periph, 0);
    timer_enable(m_timer_periph);
}
