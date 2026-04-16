#include "key.hpp"
#include "system.h"

bool Key::isPressed() const{

    if (m_press_level == RESET) {
        // 按下时为低电平
        return gpio_input_bit_get(m_gpio_port, m_gpio_pin) == RESET;
    }
    else {
        // 按下时为高电平
        return gpio_input_bit_get(m_gpio_port, m_gpio_pin) == SET;
    }
}

Key::Key(rcu_periph_enum periph_clock, uint32_t gpio_port, uint32_t gpio_pin, FlagStatus press_level)
    : m_periph_clock(periph_clock), m_gpio_port(gpio_port), m_gpio_pin(gpio_pin), m_press_level(press_level) {
    // 1. 开启 GPIO 时钟
    rcu_periph_clock_enable(m_periph_clock);

    // 2. 配置 GPIO 为输入模式
    gpio_mode_set(m_gpio_port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, m_gpio_pin);
}

bool Key::scan() {
    // 使用状态机来检测按键事件
    switch (m_key_event)
    {
    case KEY_EVENT::NONE:
        if (isPressed()) {
            m_key_event = KEY_EVENT::PRESSED;
        }
        else {
            m_key_event = KEY_EVENT::NONE;

            return false;
        }
        break;

    case KEY_EVENT::PRESSED:
        if (isPressed()) {
            m_press_cunter++; // 按键持续计数
            m_key_event = KEY_EVENT::PRESSED;
        }
        else {
            if (m_press_cunter < 20) {
                m_key_event = KEY_EVENT::NONE;
                m_press_cunter = 0; // 重置计数器
            }
            else {
                m_key_event = KEY_EVENT::RELEASED;
            }
        }

        break;
        
    case KEY_EVENT::RELEASED:
        m_key_event = KEY_EVENT::NONE;
        m_press_cunter = 0; // 重置计数器
        return true;
    
    default:
        break;
    }

    return false;
}