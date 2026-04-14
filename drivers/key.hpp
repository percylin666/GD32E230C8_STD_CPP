#pragma once

#include "gd32e23x.h"

enum class KEY_EVENT {
    NONE,
    PRESSED,
    
    RELEASED,
};

class Key {
private:
    rcu_periph_enum m_periph_clock;
    uint32_t m_gpio_port;
    uint32_t m_gpio_pin;
    FlagStatus m_press_level;

    uint32_t m_press_cunter = 0; // 按键计数器

    KEY_EVENT m_key_event = KEY_EVENT::NONE; // 上一次的按键事件

    // 读取物理电平 (假设按下为低电平 RESET)
    bool isPressed();

public:
    Key(rcu_periph_enum periph_clock, uint32_t gpio_port, uint32_t gpio_pin, FlagStatus press_level = RESET);
    ~Key() {}

    bool scan();
};