#pragma once

#include "gd32e23x.h"
#include "timer.hpp"

enum class RGB_LED_STATUS {
    RED,
    GREEN,
    BLUE
};

class RGBLed {
private:
    // RGB 颜色值，范围 0-500
    uint16_t m_r_value;
    uint16_t m_g_value;
    uint16_t m_b_value;

    Timer m_timer_r = Timer(TIMER0, 1, TIMER_MODE::PWM);
    Timer m_timer_g = Timer(TIMER14, 1, TIMER_MODE::PWM);
    Timer m_timer_b = Timer(TIMER2, 1, TIMER_MODE::PWM);

public:
    RGBLed() : m_r_value(0), m_g_value(0), m_b_value(0) {}
    ~RGBLed() {}

    void setColor(uint16_t r, uint16_t g, uint16_t b);

    void turnOff();
};